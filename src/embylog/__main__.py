import io
import pathlib
import typing as typ

from elftools.elf.elffile import ELFFile
from machofile import UniversalMachO

from embylog.elf_parser import load_elf

filename = "./b.out"

parser, block = load_elf(pathlib.Path(filename))

# 00000000: 4200 f2ff ffff ffff ffff 4243 0142 4303  B.........BC.BC.
# 00000010: 4243 0242 4300 4200 1100 0000 0000 0000  BC.BC.B.........
# 00000020: 4243 0142 4303 4243 0242 4300            BC.BC.BC.BC.
input = bytes.fromhex("""
    4200 f2ff ffff ffff ffff 4243 0142 4303
    4243 0242 4300 4200 1100 0000 0000 0000
    4243 0142 4303 4243 0242 4300
""")

file = io.BytesIO(input)


def find_message_start_magic(file: typ.BinaryIO) -> bytes:
    byte = file.read(1)
    read_bytes = bytearray()
    while True:
        if byte == b"\x42":
            byte = file.read(1)
            if byte == b"\x00":
                return bytes(read_bytes)
            else:
                # Isn't the message start magic byte; but the newly loaded byte could be.
                read_bytes += byte
                continue
        else:
            read_bytes += byte
            byte = file.read(1)


print(find_message_start_magic(file))


def parse_message(file: typ.BinaryIO):
    message = find_message_start_magic(file)
    msg_file = io.BytesIO(message)
    msg_offset = parser.read_sstype(msg_file)
    message = block.find_message_from_offset(msg_offset)


print(parse_message(file))
print()
