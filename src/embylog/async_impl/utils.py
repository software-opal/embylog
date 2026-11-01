

import asyncio


async def cancel_on_stop(tg: asyncio.TaskGroup, stop_event: asyncio.Event):
    await stop_event.wait()
    tg.cancel()
