import asyncio
import pathlib
import typing as typ

import watchfiles

from embylog import elf_parser

NOT_LOADED = sentinel("NOT_LOADED")
U = typ.TypeVar("U")


async def elf_watcher(
    elf_path: pathlib.Path, stop_event: None | asyncio.Event
) -> typ.AsyncIterator[None]:
    async for changes in watchfiles.awatch(elf_path, stop_event=stop_event):
        yield None


class WatchedElf:
    elf_path: pathlib.Path
    watcher_task: None | typ.Tuple[asyncio.Event, asyncio.Task[None]]

    _parser: NOT_LOADED | elf_parser.ByteIntParser
    _block: NOT_LOADED | elf_parser.RawMessageBlockFetcher
    _lookup: NOT_LOADED | elf_parser.StringLookup
    _dwarf: NOT_LOADED | elf_parser.AddrLookup | None

    def __init__(
        self,
        elf: pathlib.Path,
    ) -> None:
        self.elf_path = elf
        self._parser = NOT_LOADED
        self._block = NOT_LOADED
        self._lookup = NOT_LOADED
        self._dwarf = NOT_LOADED

    async def load_elf(self):
        self._parser, self._block, self._lookup, self._dwarf = await asyncio.to_thread(
            elf_parser.load_elf, self.elf_path
        )

    async def start_watcher(self):
        if self.watcher_task is None or self.watcher_task[0].is_set():
            if self.watcher_task:
                await self.stop_watcher()
            event = asyncio.Event()
            self.watcher_task = (
                event,
                asyncio.create_task(
                    self._watcher(event), name=f"WatchedElf({self.elf_path})"
                ),
            )

    async def stop_watcher(self):
        if self.watcher_task is not None:
            self.watcher_task[0].set()
            await self.watcher_task[1]

    @property
    def parser(self) -> elf_parser.ByteIntParser:
        return self._check_loaded(self._parser)

    @property
    def block(self) -> elf_parser.RawMessageBlockFetcher:
        return self._check_loaded(self._block)

    @property
    def lookup(self) -> elf_parser.StringLookup:
        return self._check_loaded(self._lookup)

    @property
    def dwarf(self) -> elf_parser.AddrLookup | None:
        return self._check_loaded(self._dwarf)

    async def _watcher(self, event: asyncio.Event):
        try:
            if self.watcher_task is None or self.watcher_task[0] != event:
                # Watched task changed before this function was executed, so return immediately
                event.set()
                return

            async for _ in watchfiles.awatch(self.elf_path, stop_event=event):
                if self.watcher_task is None or self.watcher_task[0] != event:
                    break
                await self.load_elf()
        finally:
            event.set()

    def _check_loaded(self, value: NOT_LOADED | U) -> U:
        if value is NOT_LOADED:
            raise ValueError("ELS")
        return value
