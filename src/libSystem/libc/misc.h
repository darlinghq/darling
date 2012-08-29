#ifndef LIBC_MISC_H
#define LIBC_MISC_H
#include <setjmp.h>

extern "C"
{

const char* locale_charset();
int __darwin_sigsetjmp(sigjmp_buf b, int savesigs);

}

#endif

