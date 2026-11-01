


import asyncio
import pathlib

import anyio

from embylog.async_impl import elf_watcher, stream_reader, utils


def main() -> None:
    print("Hello from embylog!")
    asyncio.run(main_loop('1', '2'))


async def main_loop(f: pathlib.Path, i: pathlib.Path):
    async with asyncio.TaskGroup() as tg:
        stop_event = asyncio.Event()
        x = elf_watcher.WatchedElf(f)
        file_opener = anyio.open_file(i)
        tg.create_task(utils.cancel_on_stop(tg, stop_event))
        await asyncio.gather(x.load_elf(), x.start_watcher(), file_opener)
        async with await file_opener as file:
            async for type, msg in stream_reader.iterate_messages(file, stop_event):
                print(type, msg)

