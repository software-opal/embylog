import dataclasses
import typing as typ

EMBY_LOG_ESCAPE_SYMBOL = b"\x42"


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

    def find_message_from_offset(self, offset_from_marker: int) -> typ.Optional[bytes]:
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
        return self.raw_bytes[raw_offset:end]
