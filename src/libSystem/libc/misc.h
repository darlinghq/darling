#ifndef LIBC_MISC_H
#define LIBC_MISC_H
#include <setjmp.h>

extern "C"
{

const char* locale_charset();

// atexit() is strange, it seems to be always inlined and cannot be resolved with dlsym()
int __darwin_atexit(void (*function)(void));

void* __darwin_malloc(size_t size);

}

#endif

