#ifndef _DARLING_EMULATION_LINUX_STRING_H_
#define _DARLING_EMULATION_LINUX_STRING_H_

#include <stddef.h>

extern void* memset(void* s, int c, size_t n);
extern char* strncpy(char* dest, const char* src, size_t n);
extern int strncmp(const char* str1, const char* str2, size_t n);

#endif // _DARLING_EMULATION_LINUX_STRING_H_
