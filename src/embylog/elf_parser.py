import pathlib
import typing as typ

from elftools.elf.sections import Section
from elftools.elf.elffile import ELFFile
from elftools.dwarf.dwarfinfo import DWARFInfo

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

class AddrLookup:
    def __init__(self, dwarf: DWARFInfo):
        self.__dwarf = dwarf

    def lookup(self, address: int) -> typ.Tuple[str, int]:
        """Copied from: https://github.com/eliben/pyelftools/blob/e40a070838548c1552297d1e5cb88f48fe96455f/examples/dwarf_decode_address.py#L72"""
        for cu in self.__dwarf.iter_CUs():
            lines = self.__dwarf.line_program_for_CU(cu)
            offset = 1 if lines.header.version < 5 else 0
            previous = None
            for line in lines.get_entries():
                if line.state is None:
                    continue

                if previous and previous.address <= address < line.state.address:
                    return (lines['file_entry'][previous.file - offset].name.decode('utf-8'), previous.line)

                previous = line.state if not line.state.end_sequence else None
        return ("", 0)
        
        

def load_elf(elf: pathlib.Path) -> typ.Tuple[ByteIntParser, RawMessageBlockFetcher, StringLookup, AddrLookup| None]:
    with elf.open("rb") as f:
        elffile = ELFFile(f)
        dwarf = None
        parser = ByteIntParser(elffile.little_endian, elffile.elfclass)
        print(f"size_t: {parser.pointer_bytes}, endian: {parser.byteorder}")
        
        text = elffile.get_section_by_name(".text")
        if text is None:
            raise ValueError("Failed to get .text section")

        if elffile.has_dwarf_info():
            dwarf = AddrLookup(elffile.get_dwarf_info())

        section = elffile.get_section_by_name(".embylog")
        if section is None:
            raise ValueError("THOEUTHOUHOUTHOEUTH")
        
        block = RawMessageBlockFetcher(section.data())
        return (parser, block, StringLookup(text), dwarf)
