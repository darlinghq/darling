
		# _myinit sets up TLV content
		.thread_init_func
    #if __LP64__
		.quad	_myinit
    #else
		.long	_myinit
    #endif

.subsections_via_symbols
