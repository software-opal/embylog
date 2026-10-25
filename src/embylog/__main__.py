from elftools.elf.elffile import ELFFile

filename = './b.out'
with open(filename, 'rb') as f:
    elffile = ELFFile(f)
    # elfclass is a public attribute of ELFFile, read from its header
    print(f'{filename}: elfclass is {elffile.elfclass}: {elffile.little_endian}')

    for section in elffile.iter_sections():
        if section.name.startswith('.debug'):
            print('  ' + section.name)

    if elffile.has_dwarf_info():
        dwarfinfo = elffile.get_dwarf_info()
        for CU in dwarfinfo.iter_CUs():
            # cu_offset is a public attribute of CU
            # address_size is part of the CU header
            print('  CU at offset 0x{:x}. address_size is {}'.format(
                CU.cu_offset, CU['address_size']))