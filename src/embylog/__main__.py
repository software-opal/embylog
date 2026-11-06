import argparse
import colorama
import os
import pathlib
import sys
import struct
import typing as typ

from elftools.elf.elffile import ELFFile
from machofile import UniversalMachO

from embylog.elf_parser import Embylog, p_int
from embylog.msg_lookup import EmbylogTrace, MsgType, Level


parser = argparse.ArgumentParser()
parser.add_argument('-p', '--preamble', 
                    action='store_true', default=False,
                    help='Print out any data in the trace before the first valid embylog trace.')
parser.add_argument('-w', '--watch', 
                    action='store_true', default=False,
                    help='Watch the ' + colorama.Fore.YELLOW + colorama.Style.BRIGHT + 'trace' + colorama.Style.RESET_ALL + ' input file for changes. Implied when <stdin> is used.')
parser.add_argument('-e', '--elf', 
                    type=pathlib.Path,
                    required=True, 
                    help='Elf binary containing the .embylog and .text sections.')
parser.add_argument('trace', # ToDo: can we make this stdin as well?
                    type=pathlib.Path,
                    nargs='?',
                    help='Binary file containing embylog trace message ids and respective parameters.')

args = parser.parse_args()

embylog = Embylog.load_elf(args.elf)

input = None
if args.trace is None:
    input =  sys.stdin.detach()
else:
    input = args.trace.open('r+b')

with input:
    tracelog = EmbylogTrace(input, watch=args.watch).messages()

    systick = 0
    address = ""
    width = os.get_terminal_size().columns
    for type, trace in tracelog:
        if  type == None and args.preamble:
            print(trace.decode('utf-8'))
            args.preamble = False
            continue

        if type == MsgType.SYSTICK:
            old_systick = systick
            systick = int.from_bytes(trace[:4], embylog.byteorder)
            if systick > (old_systick + 10):
                print(f"{'':-<{width}}")
                print(colorama.Cursor.UP() + colorama.Cursor.BACK(1000) + f"{systick: 5}ms | ")
            continue
        else:
            print(f"{'': <{width}}{colorama.Cursor.BACK(1000)}", end='')

        if type == MsgType.ADDRESS:
            address = int.from_bytes(trace[:4], embylog.byteorder)
            addr2line = embylog.addr2line(address)
            if addr2line is None:
                address = f"PC:0x{address:08x}"
            else:
                filename, line = addr2line
                address = f"({filename:}:{line:})"
            continue

        line = colorama.Style.RESET_ALL + f"{systick: 5}ms | "

        if type == MsgType.HEXDUMP:
            for chunk in range(0, len(trace), 16):
                printable = "".join([chr(c) if 32 <= c <= 127 else '.' for c in trace[chunk:][:16]])
                print(f"{line}{colorama.Fore.BLUE}H: {chunk:04X}: {trace[chunk:][:16].hex(' ', 2):41}{printable}{colorama.Style.RESET_ALL}")
            continue

        if type == MsgType.MESSAGE:

            try:
                params = [p_int(int.from_bytes(trace[p:][:4], embylog.byteorder), embylog._strings) for p in range(4, len(trace), 4)]
                
                level, param_count, msg_index = struct.unpack("<BBH", trace[:4])
                message = embylog.get_log(msg_index)

                message = message.replace('%p', '0x%08x')

                if param_count != len(params):
                    print(colorama.Fore.YELLOW + f"Warning, expected {param_count} parameters, got {len(params)} instead" + colorama.Style.RESET_ALL);

                # print(f"offset: 0x{msg_index:04X} -> {message}\n\t", params)
                try:
                    level = Level(level)
                except ValueError:
                    level = Level.NONE

                if level == level.TRACE:
                    line += colorama.Fore.CYAN + colorama.Style.DIM + 'T: '
                elif level == level.DEBUG:
                    line += colorama.Fore.CYAN + 'D: '
                elif level == level.INFO:
                    line += colorama.Fore.WHITE+ 'I: '
                elif level == level.WARNING:
                    line += colorama.Fore.YELLOW + 'W: '
                elif level == level.ERROR:
                    line += colorama.Fore.RED + 'E: '
                elif level == level.FATAL:
                    line += colorama.Back.RED + colorama.Style.BRIGHT + colorama.Fore.YELLOW + 'F: '
                
                line += message % tuple(params) + colorama.Style.RESET_ALL
                print(line)

                if address:
                    print(colorama.Cursor.FORWARD(1000) + colorama.Cursor.UP() + colorama.Cursor.BACK(len(address) - 1) + address)
                    address = ""
            except struct.error:
                print(colorama.Fore.YELLOW + f"Malformed trace {trace.hex()}" + colorama.Style.RESET_ALL)
            except TypeError:
                print(colorama.Fore.YELLOW + f"Could not format message {message} with params {params[1:]}." + colorama.Style.RESET_ALL)
            continue

        