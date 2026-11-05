import dataclasses
import io
import typing as typ

from enum import Enum

EMBY_LOG_ESCAPE_SYMBOL = b"\x42"


class MsgType(Enum):
    MESSAGE = b'\x00'
    SYSTICK = b'\x01'
    HEXDUMP = b'\x02'
    ADDRESS = b'\x03'

class ByteIntParser:

    byteorder: typ.Literal["little", "big"]

    def __init__(self, little_endian: bool, pointer_bits: int):
        assert pointer_bits == 32 or pointer_bits == 64
        self.little_endian = little_endian
        self.byteorder = "little" if little_endian else "big"
        self.pointer_bits = pointer_bits
        self.pointer_bytes = pointer_bits // 8

    def read_byte(self, file: typ.BinaryIO) -> bytes:
        byte = file.read(1)
        if byte == EMBY_LOG_ESCAPE_SYMBOL:
            file.read(1)
            return EMBY_LOG_ESCAPE_SYMBOL
        return byte

    def read_signed(self, file: typ.BinaryIO, byte_count: int) -> int:
        d = b''.join([self.read_byte(file) for _ in range(byte_count)])
        return int.from_bytes(d, byteorder=self.byteorder, signed=True)

    def read_unsigned(self, file: typ.BinaryIO, byte_count: int) -> int:
        d = b''.join([self.read_byte(file) for _ in range(byte_count)])
        return int.from_bytes(d, byteorder=self.byteorder)

    # (value, read_length)
    def read_sstype(self, file: typ.BinaryIO) -> int:
        return self.read_signed(file, self.pointer_bytes)

    def read_stype(self, file: typ.BinaryIO) -> int:
        return self.read_unsigned(file, self.pointer_bytes)


class RawMessageBlockFetcher:

    __message_cache: typ.Dict[int, typ.Tuple[int, int]]

    def __init__(self, raw_bytes: bytes):
        self.raw_bytes = raw_bytes
        self.__marker_offset = None
        self.__message_cache = {}

    @property
    def marker_offset(self):
        if self.__marker_offset is None:
            # self.__marker_offset = self.raw_bytes.find(b"[EnbyLog] Offset\x00")
            self.__marker_offset = 0

        return self.__marker_offset

    def find_message_from_offset(self, offset_from_marker: int) -> typ.Optional[str]:
        if offset_from_marker < 0 or (self.marker_offset + offset_from_marker) > len(self.raw_bytes):
            raise ValueError(f"Offset {offset_from_marker} falls outside of .embylog section ({len(self.raw_bytes)})")
        
        if offset_from_marker not in self.__message_cache:
            # offset_from_marker = message_ptr - MARKER_ptr
            raw_offset = self.marker_offset + offset_from_marker
            end = self.raw_bytes.find(b"\0", raw_offset)
            # print(f"raw_offset: {raw_offset}, end: {end}")
            self.__message_cache[offset_from_marker] = (raw_offset, end)

        raw_offset, end = self.__message_cache[offset_from_marker]
        if (end - 1) < 0:
            return None
        
        return self.raw_bytes[raw_offset:end].decode('utf-8')

class EmbylogTrace:
    def __init__(self, file: typ.BinaryIO):
        self._file = file

    def messages(self) -> typ.Iterator[typ.Tuple[MsgType, bytes]]:
        while True:
            type = None
            read_bytes = bytearray()
            while True:
                byte = self._file.read(1)
                if byte == None or byte == b'':
                    return type, bytes(read_bytes)

                if byte != b'\x42':
                    read_bytes += byte
                    continue

                byte = self._file.read(1)
                if byte == None or byte == b'':
                    return type, bytes(read_bytes)

                
                if type != None:
                    if byte == b'\x43':
                        read_bytes += b'\x42'
                    elif len(read_bytes) > 0:
                        break
                
                try:
                    type = MsgType(byte)
                    # Special case for when there was data before a valid start-of-frame.
                    if len(read_bytes) > 0:
                        type = None
                        break
                except ValueError:
                    read_bytes += byte

            self._file.seek(-2, io.SEEK_CUR)
            # print(f"file: 0x{file.tell():X}")
            yield type, bytes(read_bytes)
