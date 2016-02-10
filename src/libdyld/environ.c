#ifdef __x86_64__
__asm__(".section .bss\n"
		".type __darwin_environ,@common\n"
		".size __darwin_environ, 8\n"
		".global __darwin_environ\n"
		".symver __darwin_environ, environ@DARWIN\n"
		"//.comm __darwin_environ, 8\n"
		"__darwin_environ:\n"
		    ".quad 0\n");
#else
__asm__(".section .bss\n"
		".global __darwin_environ\n"
		".symver __darwin_environ, environ@DARWIN\n"
		"//.comm __darwin_environ, 4\n"
		"__darwin_environ:\n"
		    ".word 0\n");
#endif

