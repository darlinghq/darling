#ifdef __x86_64__
__asm__(".section .bss\n"
		".global __darwin_environ\n"
		".symver __darwin_environ, environ@DARWIN\n"
		"__darwin_environ:\n"
		    ".quad 0\n");
#else
__asm__(".section .bss\n"
		".global __darwin_environ\n"
		".symver __darwin_environ, environ@DARWIN\n"
		"__darwin_environ:\n"
		    ".word 0\n");
#endif

