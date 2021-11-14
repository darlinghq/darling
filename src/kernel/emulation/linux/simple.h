#ifndef LINUX_DEBUG_H
#define LINUX_DEBUG_H

#include <stdarg.h>
#include <stddef.h>

// everything in this header can be used outside of libsystem_kernel

#ifdef __cplusplus
extern "C" {
#endif

void __simple_printf(const char* format, ...) __attribute__((format(printf, 1, 2)));
void __simple_kprintf(const char* format, ...) __attribute__((format(printf, 1, 2)));
void __simple_fprintf(int fd, const char* format, ...) __attribute__((format(printf, 2, 3)));
int __simple_sprintf(char *buffer, const char* format, ...) __attribute__((format(printf, 2, 3)));
int __simple_strlen(const char* str);
int __simple_vsprintf(char* buf, const char* format, va_list vl) __attribute__((format(printf, 2, 0)));
int __simple_vsnprintf(char* buffer, size_t max_length, const char* format, va_list args) __attribute__((format(printf, 3, 0)));
int __simple_snprintf(char* buffer, size_t max_length, const char* format, ...) __attribute__((format(printf, 3, 4)));
void __simple_vprintf(const char* format, va_list args) __attribute__((format(printf, 1, 0)));
void __simple_vkprintf(const char* format, va_list args) __attribute__((format(printf, 1, 0)));
void __simple_vfprintf(int fd, const char* format, va_list args) __attribute__((format(printf, 2, 0)));

unsigned long long __simple_atoi(const char* str, const char** endp);
unsigned long long __simple_atoi16(const char* str, const char** endp);

// TODO: Remove this in favor of readline in readline.c?
struct simple_readline_buf
{
	char buf[512];
	int used;
};
void __simple_readline_init(struct simple_readline_buf* buf);
char* __simple_readline(int fd, struct simple_readline_buf* buf, char* out, int max_out);

#ifdef __cplusplus
};
#endif

#endif

