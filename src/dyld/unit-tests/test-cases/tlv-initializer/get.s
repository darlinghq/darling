
#if __x86_64__

    # _myinit sets up TLV content
    .thread_init_func
    .quad	_myinit

#endif

#if __i386__

    # _myinit sets up TLV content
    .thread_init_func
    .long	_myinit

#endif

.subsections_via_symbols
