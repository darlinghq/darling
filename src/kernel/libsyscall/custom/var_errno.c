
#if defined(__x86_64__) || defined(__i386__)
__asm__(".section .bss\n"
                ".global darwin_errno\n"
                ".hidden darwin_errno\n"
                ".symver darwin_errno, errno@DARWIN\n"
                ".comm darwin_errno, 4\n"
                "darwin_errno:\n"
                    ".word 0\n");
#else
#	warning No darwin_errno definition!
#endif
