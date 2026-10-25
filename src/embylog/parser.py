import elftools
import pathlib

def load_elf(elf: pathlib.Path):
    elftools.elf