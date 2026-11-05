import pathlib
import typing as typ

from elftools.elf.sections import Section
from elftools.elf.elffile import ELFFile
from elftools.dwarf.dwarfinfo import DWARFInfo

from .msg_lookup import ByteIntParser, RawMessageBlockFetcher


class p_int(int):
    """Wrapper for int where __str__ will do a lookup in .text as if it was a pointer.
    Usefule for string modulus '%' formatting:
    https://docs.python.org/3.12/library/stdtypes.html#printf-style-string-formatting
    """

    _strings = None

    def __new__(cls, data=None, strings: StringLookup | None = None):
        obj = super(p_int, cls).__new__(cls, data)
        obj._strings = strings
        return obj
    
    def __str__(self):
        return self._strings.lookup(self)

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

    def addr2line(self, address: int) -> typ.Tuple[str, int] | None:
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
        return None
        

class Embylog:
    def __init__(self, parser: ByteIntParser, fetcher: RawMessageBlockFetcher, strings: StringLookup, addrs: AddrLookup | None):
        self._parser = parser
        self._fetcher = fetcher
        self._strings = strings
        self._addrs = addrs

    @property
    def byteorder(self):
        return self._parser.byteorder

    @property
    def pointer_bytes(self):
        return self._parser.pointer_bytes

    def get_string(self, pointer: int) -> str:
        """Looks up the pointer in .text and returns the string."""
        self._strings.lookup(pointer)

    def get_log(self, index: int) -> typ.Optional[str]:
        return self._fetcher.find_message_from_offset(index * self._parser.pointer_bytes)

    def addr2line(self, address: int) -> typ.Tuple[str, int] | None:
        if self._addrs is None:
            return None
        return self._addrs.addr2line(address) 

    @classmethod
    def load_elf(cls, elf: pathlib.Path) -> Embylog:
        with elf.open("rb") as f:
            elffile = ELFFile(f)
            dwarf = None
            parser = ByteIntParser(elffile.little_endian, elffile.elfclass)

            section = elffile.get_section_by_name(".embylog")
            if section is None:
                raise ValueError("Failed to get .embylog section")
            
            text = elffile.get_section_by_name(".text")
            if text is None:
                raise ValueError("Failed to get .text section")

            if elffile.has_dwarf_info():
                dwarf = AddrLookup(elffile.get_dwarf_info())
            
            block = RawMessageBlockFetcher(section.data())
            return cls(parser, block, StringLookup(text), dwarf)


