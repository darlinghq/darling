#include <stdarg.h>

#ifndef LINUX_DEBUG_H
#define LINUX_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

void __simple_printf(const char* format, ...) __attribute__((format(printf, 1, 2)));
int __simple_sprintf(char* buffer, const char* format, ...) __attribute__((format(printf, 2, 3)));
int __simple_vsprintf(char* buf, const char* format, va_list vl) __attribute__((format(printf, 2, 0)));
int __simple_strlen(const char* str);

unsigned long long __simple_atoi(const char* str, const char** endp);
unsigned long long __simple_atoi16(const char* str, const char** endp);

#ifdef __cplusplus
}
#endif

#endif

