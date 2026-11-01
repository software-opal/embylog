import asyncio
import enum
import typing as typ

import anyio


class MsgType(enum.Enum):
    PREAMBLE = None
    MESSAGE = b"\x00"
    SYSTICK = b"\x01"
    HEXDUMP = b"\x02"
    ADDRESS = b"\x03"


async def iterate_messages(
    file: anyio.AsyncFile, stop_event: asyncio.Event
) -> typ.AsyncIterator[typ.Tuple[MsgType, bytes]]:
    async with asyncio.TaskGroup() as tg, file:
        tg.create_task(cancel_on_stop(tg, stop_event))
        async for msg in do_iterate_messages(file, tg, stop_event):
            yield msg


async def cancel_on_stop(tg: asyncio.TaskGroup, stop_event: asyncio.Event):
    await stop_event.wait()
    tg.cancel()


async def read_byte(
    file: anyio.AsyncFile,
    tg: asyncio.TaskGroup,
) -> None | bytes:
    try:
        return await tg.create_task(file.read(1))
    except BlockingIOError:
        return None


async def do_iterate_messages(
    file: anyio.AsyncFile, tg: asyncio.TaskGroup, stop_event: asyncio.Event
) -> typ.AsyncIterator[typ.Tuple[MsgType, bytes]]:
    read_bytes = bytearray()
    type = MsgType.PREAMBLE
    while not stop_event.is_set():
        byte1 = await read_byte(file, tg)

        if byte1 == None or byte1 == b"":
            yield (type, bytes(read_bytes))
        elif byte1 != b"\x42":
            read_bytes += byte1
        else:
            byte2 = await read_byte(file, tg)
            if byte2 == None or byte2 == b"":
                yield (type, bytes(read_bytes))
            elif byte2 == b"\x43":
                read_bytes += b"\x42"
            elif byte2 in MsgType:
                if len(read_bytes) > 0:
                    yield (type, bytes(read_bytes))
                    read_bytes = bytearray()
                type = MsgType(byte2)
            else:
                read_bytes += byte1
                read_bytes += byte2
