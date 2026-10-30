import pathlib
import typing as typ

from elftools.elf.sections import Section
from elftools.elf.elffile import ELFFile

from .msg_lookup import ByteIntParser, RawMessageBlockFetcher


class StringLookup:
    __cache: typ.Dict[int, typ.Tuple[int, int]]
    __addr: int
    __end: int
    def __init__(self, section: Section):
        self.__data = section.data()
        self.__cache = {}
        self.__addr = section.header['sh_addr']
        self.__end = self.__addr + section.header['sh_size']

    def in_range(self, addr: int) -> bool:
        return addr >= self.__addr and addr < self.__end

    def lookup(self, addr: int) -> str | None:
        if not self.in_range(addr):
            return None
        
        if addr not in self.__cache:
            start = addr - self.__addr
            d = self.__data[start:]
            end = d.find(b'\0')
            # print(f"found string 0x{addr:08X}({start}:{end})")
            self.__cache[addr] = (start, start + end)

        start, end = self.__cache[addr]
        return self.__data[start:end].decode('utf-8')
        
        

def load_elf(elf: pathlib.Path) -> typ.Tuple[ByteIntParser, RawMessageBlockFetcher, StringLookup]:
    with elf.open("rb") as f:
        elffile = ELFFile(f)
        parser = ByteIntParser(elffile.little_endian, elffile.elfclass)
        print(f"size_t: {parser.pointer_bytes}, endian: {parser.byteorder}")
        text = elffile.get_section_by_name(".text")
        if text is None:
            raise ValueError("Failed to get .text section")
        section = elffile.get_section_by_name(".embylog")
        # print(section.data())
        if section is not None:
            block = RawMessageBlockFetcher(section.data())
            return (parser, block, StringLookup(text))
    raise ValueError("THOEUTHOUHOUTHOEUTH")
