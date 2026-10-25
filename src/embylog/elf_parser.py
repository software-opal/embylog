import pathlib
import typing as typ

from elftools.elf.elffile import ELFFile

from .msg_lookup import ByteIntParser, RawMessageBlockFetcher


def load_elf(elf: pathlib.Path) -> typ.Tuple[ByteIntParser, RawMessageBlockFetcher]:
    with elf.open("rb") as f:
        elffile = ELFFile(f)
        parser = ByteIntParser(elffile.little_endian, elffile.elfclass)
        section = elffile.get_section_by_name(".emby_log")
        print(section.data())
        if section is not None:
            block = RawMessageBlockFetcher(section.data())
            return (parser, block)
    raise ValueError("THOEUTHOUHOUTHOEUTH")
