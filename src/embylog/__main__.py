import io
import os
import pathlib
import typing as typ

from enum import Enum

from elftools.elf.elffile import ELFFile
from machofile import UniversalMachO

from embylog.elf_parser import load_elf

filename = "../../build/my-app.elf"
trace_log = "../../build/trace.bin"

parser, block, strings, addr2line = load_elf(pathlib.Path(filename))

# 00000000: 4200 f2ff ffff ffff ffff 4243 0142 4303  B.........BC.BC.
# 00000010: 4243 0242 4300 4200 1100 0000 0000 0000  BC.BC.B.........
# 00000020: 4243 0142 4303 4243 0242 4300            BC.BC.BC.BC.

file = pathlib.Path(trace_log).open('r+b')

class MsgType(Enum):
    MESSAGE = b'\x00'
    SYSTICK = b'\x01'
    HEXDUMP = b'\x02'
    ADDRESS = b'\x03'

def find_message_start_magic(file: typ.BinaryIO) -> typ.Tuple[MsgType, bytes] | None:
    type = None
    
    read_bytes = bytearray()
    while True:
        byte = file.read(1)
        if byte == None or byte == b'':
            return (type, bytes(read_bytes))

        if byte != b'\x42':
            read_bytes += byte
            continue

        byte = file.read(1)
        if byte == None:
            return (type, bytes(read_bytes))
        
        if type != None:
            if byte == b'\x43':
                read_bytes += b'\x42'
            elif len(read_bytes) > 0:
                break
        
        try:
            type = MsgType(byte)
            if len(read_bytes) > 0:
                break
        except ValueError:
            read_bytes += byte

    file.seek(-2, io.SEEK_CUR)
    # print(f"file: 0x{file.tell():X}")
    return (type, bytes(read_bytes))
        

_, preamble = find_message_start_magic(file)
if preamble != None:
    print(preamble.decode('utf-8'))

class p_int(int):
    """Wrapper for int where __str__ will do a lookup in .text as if it was a pointer.
    Usefule for string modulus '%' formatting:
    https://docs.python.org/3.12/library/stdtypes.html#printf-style-string-formatting
    """
    def __new__(cls, data=None):
        obj = super(p_int, cls).__new__(cls, data)
        return obj
    
    def __str__(self):
        return strings.lookup(self)

systick = 0
address = ""
width = os.get_terminal_size().columns
while True:
    type, trace = find_message_start_magic(file)
    if len(trace) == 0:
        break

    # print((type, trace.hex()))

    if type == MsgType.SYSTICK:
        old_systick = systick
        systick = int.from_bytes(trace[:4], parser.byteorder)
        if systick > (old_systick + 10):
            print(f"{systick: 8}ms | {'':-<{width - 13}}")
        continue

    if type == MsgType.HEXDUMP:
        for chunk in range(0, len(trace), 16):
            print(f"{systick: 8}ms | {chunk:08X}: {trace[chunk:][:16].hex(' ', 2)}")
        continue

    if type == MsgType.ADDRESS:
        address = int.from_bytes(trace[:4], parser.byteorder)
        if addr2line is None:
            address = f"PC:0x{address:08x}"
        else:
            filename, line = addr2line.lookup(address)
            address = f"({filename:}:{line:})"
        continue

    if type == MsgType.MESSAGE:
        msg_offset = parser.read_sstype(io.BytesIO(trace))
        # print(f"Offset: {msg_offset}(0x{msg_offset * parser.pointer_bytes:08X})")
        
        message = block.find_message_from_offset(msg_offset * parser.pointer_bytes)
        params = [p_int.from_bytes(trace[p:][:4], parser.byteorder) for p in range(4, len(trace), 4)]

        # print(f"offset: 0x{msg_offset:04X} -> {message.decode("utf-8")}\n\t", params)
        try:
            line = f"{systick: 8}ms | {message.decode("utf-8") % tuple(params)}"
            print(f"{line:{width - len(address)}}{address}")
        except TypeError:
            print(f"Could not format message {message} with params {params}. file offset 0x{file.tell():X}")
        continue

    