# EmbyLog

Logging for all your embies.

This library and companion python module will defer all formatted log messages to something that's got more free-time on their hands.

All messages invoked by `embylogf()` will be placed in a `.embylog` section in the ELF binary. 
Any strings passed as parameters are only kept as pointers; The `embylog` python module will look up the pointer inside the `.text` section. 

Heavily inspired by [Deferred logging from scratch - The Gistre Blog](https://blog.gistre.epita.fr/posts/sofiane.meftah-2025-06-15-deferred_logging/). Thank you!

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

```c
#include <embylog.c>

// (optional) Return the current systick. (Currently the unit is fixed to milliseconds.)
size_t embylog_systick() {
  return systick;
}

// Transmit a single byte to the output channel (UART, RTT, USB-Serial, etc...)
void embylog_transmit(const uint8_t x) {
    SEGGER_RTT_PutChar(0, x);
}

// (optional) Transmit an entire buffer to the output channel (UART, RTT, USB-Serial, etc...)
// Done this way in case there's an performance improvement by using a DMA.
void embylog_transmit_array(const uint8_t *v, size_t length) {
    SEGGER_RTT_Write(0, v, length);
}

// ...

int main() {
    SEGGER_RTT_printf(0, "Oh look, RTT! 👋\n");

    embylogf("This %s will be formatted in python instead of on the target", "message");
}
```

In your linker script, `INCLUDE embylog.ld`. It is self-contained and only ensures `.embylog` input sections are preserved in in the `.embylog` output section.

For example:
```ld
INCLUDE embylog/ld/embylog.ld

MEMORY
{
 ram (rwx) : ORIGIN = 0x20000000, LENGTH = 36K
 rom (rx) : ORIGIN = 0x08000000, LENGTH = 64K
}

SECTIONS
{
 .text : {
    # ...
 } >rom
}
```

## Example log format


```
> xxd rtt-trace.bin
00000000: 5345 4747 4552 204a 2d4c 696e 6b20 5639  SEGGER J-Link V9
00000010: 2e38 3020 2d20 5265 616c 2074 696d 6520  .80 - Real time 
00000020: 7465 726d 696e 616c 206f 7574 7075 740d  terminal output.
00000030: 0a53 4547 4745 5220 4a2d 4c69 6e6b 2045  .SEGGER J-Link E
00000040: 4455 2056 382e 302c 2053 4e3d xxxx xxxx  DU V8.0, SN=xxxx
00000050: xxxx xxxx xx0d 0a50 726f 6365 7373 3a20  xxxxx..Process: 
00000060: 4a4c 696e 6b47 4442 5365 7276 6572 434c  JLinkGDBServerCL
00000070: 4578 650d 0a4f 6820 6c6f 6f6b 2c20 5254  Exe..Oh look, RT
00000080: 5421 20f0 9f91 8b0a 4203 d306 0008 4200  T! .....B.....B.
00000090: 0d00 0000                                ....
```

```
> uv run python -m embylog
size_t: 4, endian: little
SEGGER J-Link V9.80 - Real time terminal output
SEGGER J-Link EDU V8.0, SN=xxxxxxxxx
Process: JLinkGDServerCLExe
Oh look, RTT! 👋

       0ms | Hello!                                                                                                      (main.c:20)
```