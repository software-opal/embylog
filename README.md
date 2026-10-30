# EmbyLog

Logging for all your embies

## Dev setup

```shell
uv sync
```

## Implementing the library

Logging is called via:
- `embylogf(message, ...)` -- Which uses C-style formatting strings (think `printf`)
- `embylog_hexdump(byte_arr, length)` -- Writes the bytes indicating they're to be shown as a hex dump

The following functions must be implemented:
- `void embylog_transmit_array(const uint8_t *bytes, size_t length)` -- Writes bytes to your preferred output stream.

The following functions can be implemented to extend functionality:
- `size_t embylog_systick()` -- Returns the current time. This is transmitted whenever a log is transmitted and it has changed since the last log message. Nominally milliseconds. Return `0` to disable.

