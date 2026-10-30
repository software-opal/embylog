SECTIONS
{
    .embylog 0 : {
        # Define the starting address into `extern size_t __embylog_start`
        PROVIDE(__embylog_start = .);
        
        # TODO: Report the pointer alignment. should this be a struct?
        # BYTE(ALIGNOF(.embylog));

        # Tell --gc-sections to keep these unrefernced symbols.        
        KEEP(*(.embylog));
    }
}