import argparse
import os
import pathlib
import typing as typ

from elftools.elf.elffile import ELFFile
from machofile import UniversalMachO

from embylog.elf_parser import Embylog, p_int
from embylog.msg_lookup import EmbylogTrace, MsgType


parser = argparse.ArgumentParser()
parser.add_argument('-p', '--preamble', 
                    action='store_true', default=False,
                    help='Print out any data in the trace before the first valid embylog trace,')
parser.add_argument('-e', '--elf', 
                    required=True, 
                    help='Elf binary containing the .embylog and .text sections')
parser.add_argument('trace', # ToDo: can we make this stdin as well?
                    help='binary tracelog containing trace messages ids and respectice parameters.')

args = parser.parse_args()

embylog = Embylog.load_elf(pathlib.Path(args.elf))
tracelog = EmbylogTrace(pathlib.Path(args.trace).open('r+b')).messages()

systick = 0
address = ""
width = os.get_terminal_size().columns
for type, trace in tracelog:
    if  type == None and args.preamble:
        print(trace.decode('utf-8'))
        args.preamble = False
        continue
    # print((type, trace.hex()))

    if type == MsgType.SYSTICK:
        old_systick = systick
        systick = int.from_bytes(trace[:4], embylog.byteorder)
        if systick > (old_systick + 10):
            print(f"{systick: 8}ms | {'':-<{width - 13}}")
        continue

    if type == MsgType.HEXDUMP:
        for chunk in range(0, len(trace), 16):
            print(f"{systick: 8}ms | {chunk:08X}: {trace[chunk:][:16].hex(' ', 2)}")
        continue

    if type == MsgType.ADDRESS:
        address = int.from_bytes(trace[:4], embylog.byteorder)
        addr2line = embylog.addr2line(address)
        if addr2line is None:
            address = f"PC:0x{address:08x}"
        else:
            filename, line = addr2line
            address = f"({filename:}:{line:})"
        continue

    if type == MsgType.MESSAGE:
        params = [p_int(int.from_bytes(trace[p:][:4], embylog.byteorder), embylog._strings) for p in range(0, len(trace), 4)]
        
        msg_index = params[0]
        message = embylog.get_log(msg_index)

        # print(f"offset: 0x{msg_index:04X} -> {message}\n\t", params)
        try:
            line = f"{systick: 8}ms | {message % tuple(params[1:])}"
            print(f"{line:{width - len(address)}}{address}")
            address = ""
        except TypeError:
            print(f"Could not format message {message} with params {params[1:]}.")
        continue

    