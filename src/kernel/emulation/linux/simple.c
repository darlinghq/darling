#include "base.h"
#include "simple.h"
#include <stdarg.h>
#include <stddef.h>
#include <linux-syscalls/linux.h>
#include <lkm/api.h>
#include "mach/lkm.h"
#include "signal/kill.h"
#include <sys/signal.h>
#include <darlingserver/rpc.h>

extern char* memchr(char* buf, int c, __SIZE_TYPE__ n);

__attribute__ ((visibility ("default")))
int __simple_strlen(const char* text)
{
	int len = 0;
	while (*text++)
		len++;
	return len;
}

static inline int abs(int n)
{
	return (n < 0) ? -n : n;
}

__attribute__ ((visibility ("default")))
int __simple_vsnprintf(char* buf, size_t max_length, const char* format, va_list vl)
{
	size_t offset = 0;
	while (*format)
	{
		if (*format == '%')
		{
			enum format_size {
				format_size_default,
				format_size_short_short,
				format_size_short,
				format_size_long,
				format_size_long_long,
				format_size_max,
				format_size_size_type,
				format_size_ptrdiff,
			};

			enum format_size format_size = format_size_default;

			format++;
			if (!*format)
				break;

			switch (*format) {
				case 'l':
					format++;
					if (*format == 'l') {
						format++;
						format_size = format_size_long_long;
					} else {
						format_size = format_size_long;
					}
					break;

				case 'h':
					format++;
					if (*format == 'h') {
						format++;
						format_size = format_size_short;
					} else {
						format_size = format_size_short_short;
					}
					break;

				case 'j':
					format++;
					format_size = format_size_max;
					break;

				case 'z':
					format++;
					format_size = format_size_size_type;
					break;

				case 't':
					format++;
					format_size = format_size_ptrdiff;
					break;
			}

			switch (*format)
			{
				case '%':
					if (offset < max_length)
						buf[offset] = '%';
					offset++;
					break;
				case 's':
				{
					const char* str = va_arg(vl, const char*);
					if (!str)
						str = "(null)";

					while (*str)
					{
						if (offset < max_length)
							buf[offset] = *str;
						offset++;
						str++;
					}
					break;
				}
				case 'd':
				{
					intmax_t num = 0;
					char temp[20];
					int count = 0;

					switch (format_size) {
						case format_size_short:
						case format_size_short_short:
						case format_size_default:
							num = va_arg(vl, int);
							break;
						case format_size_long:
							num = va_arg(vl, long int);
							break;
						case format_size_long_long:
							num = va_arg(vl, long long int);
							break;
						case format_size_max:
							num = va_arg(vl, intmax_t);
							break;
						case format_size_size_type:
							num = va_arg(vl, size_t);
							break;
						case format_size_ptrdiff:
							num = va_arg(vl, ptrdiff_t);
							break;
					}

					if (num < 0)
					{
						if (offset < max_length)
							buf[offset] = '-';
						offset++;
						num = -num;
					}

					do
					{
						temp[count++] = '0' + abs(num % 10);
						num /= 10;
					}
					while (num > 0);
					
					while (count--)
					{
						if (offset < max_length)
							buf[offset] = temp[count];
						offset++;
					}

					break;
				}
				case 'u':
				{
					uintmax_t num = 0;
					char temp[20];
					int count = 0;

					switch (format_size) {
						case format_size_short:
						case format_size_short_short:
						case format_size_default:
							num = va_arg(vl, unsigned int);
							break;
						case format_size_long:
							num = va_arg(vl, unsigned long int);
							break;
						case format_size_long_long:
							num = va_arg(vl, unsigned long long int);
							break;
						case format_size_max:
							num = va_arg(vl, uintmax_t);
							break;
						case format_size_size_type:
							num = va_arg(vl, size_t);
							break;
						case format_size_ptrdiff:
							num = va_arg(vl, ptrdiff_t);
							break;
					}

					do
					{
						temp[count++] = '0' + (num % 10);
						num /= 10;
					}
					while (num > 0);
					
					while (count--)
					{
						if (offset < max_length)
							buf[offset] = temp[count];
						offset++;
					}

					break;
				}
				case 'p':
				case 'x':
				{
					uintmax_t num = 0;
					char temp[40];
					int count = 0;

					if (*format == 'p') {
						num = va_arg(vl, void*);
					} else {
						switch (format_size) {
							case format_size_short:
							case format_size_short_short:
							case format_size_default:
								num = va_arg(vl, unsigned int);
								break;
							case format_size_long:
								num = va_arg(vl, unsigned long int);
								break;
							case format_size_long_long:
								num = va_arg(vl, unsigned long long int);
								break;
							case format_size_max:
								num = va_arg(vl, uintmax_t);
								break;
							case format_size_size_type:
								num = va_arg(vl, size_t);
								break;
							case format_size_ptrdiff:
								num = va_arg(vl, ptrdiff_t);
								break;
						}
					}

					if (*format == 'p')
					{
						if (offset < max_length)
							buf[offset] = '0';
						offset++;
						if (offset < max_length)
							buf[offset] = 'x';
						offset++;
					}

					do
					{
						int c = (num % 16);

						if (c < 10)
							temp[count++] = '0' + c;
						else
							temp[count++] = 'A' + (c - 10);
						num /= 16;
					}
					while (num > 0);
					
					while (count--)
					{
						if (offset < max_length)
							buf[offset] = temp[count];
						offset++;
					}

					break;

				}

				case 'o':
				{
					uintmax_t num = 0;
					char temp[16];
					int count = 0;

					switch (format_size) {
						case format_size_short:
						case format_size_short_short:
						case format_size_default:
							num = va_arg(vl, unsigned int);
							break;
						case format_size_long:
							num = va_arg(vl, unsigned long int);
							break;
						case format_size_long_long:
							num = va_arg(vl, unsigned long long int);
							break;
						case format_size_max:
							num = va_arg(vl, uintmax_t);
							break;
						case format_size_size_type:
							num = va_arg(vl, size_t);
							break;
						case format_size_ptrdiff:
							num = va_arg(vl, ptrdiff_t);
							break;
					}

					do
					{
						temp[count++] = '0' + (num % 8);
						num /= 8;
					}
					while (num > 0);

					while (count--)
					{
						if (offset < max_length)
							buf[offset] = temp[count];
						offset++;
					}

					break;
				}

				case 'c':
				{
					int num = va_arg(vl, int);
					if (offset < max_length)
						buf[offset] = (char)num;
					offset++;
					break;
				}
			}

			format++;
		}
		else
		{
			if (offset < max_length)
				buf[offset] = *format;
			offset++;
			format++;
		}
	}

	if (offset < max_length)
	{
		buf[offset] = '\0';
	}
	else
	{
		buf[max_length - 1] = '\0';
	}

	return offset;
}

__attribute__ ((visibility ("default")))
int __simple_vsprintf(char* buf, const char* format, va_list vl)
{
	return __simple_vsnprintf(buf, SIZE_MAX, format, vl);
}

__attribute__ ((visibility ("default")))
void __simple_printf(const char* format, ...)
{
	char buffer[512];
	va_list vl;

	va_start(vl, format);
	__simple_vsnprintf(buffer, sizeof(buffer), format, vl);
	va_end(vl);

	LINUX_SYSCALL3(__NR_write, 1, buffer, __simple_strlen(buffer));
}

__attribute__ ((visibility ("default")))
void __simple_kprintf(const char* format, ...)
{
	va_list vl;
	va_start(vl, format);
	__simple_vkprintf(format, vl);
	va_end(vl);
}

__attribute__ ((visibility ("default")))
void __simple_fprintf(int fd, const char* format, ...)
{
	char buffer[512];
	va_list vl;

	va_start(vl, format);
	__simple_vsnprintf(buffer, sizeof(buffer), format, vl);
	va_end(vl);

	LINUX_SYSCALL3(__NR_write, fd, buffer, __simple_strlen(buffer));
}

__attribute__ ((visibility ("default")))
void __simple_vprintf(const char* format, va_list args)
{
	char buffer[512];
	__simple_vsnprintf(buffer, sizeof(buffer), format, args);
	LINUX_SYSCALL3(__NR_write, 1, buffer, __simple_strlen(buffer));
}

__attribute__ ((visibility ("default")))
void __simple_vkprintf(const char* format, va_list args)
{
	char buffer[512];
	__simple_vsnprintf(buffer, sizeof(buffer), format, args);
	dserver_rpc_kprintf(buffer, __simple_strlen(buffer));
}

__attribute__ ((visibility ("default")))
void __simple_vfprintf(int fd, const char* format, va_list args)
{
	char buffer[512];
	__simple_vsnprintf(buffer, sizeof(buffer), format, args);
	LINUX_SYSCALL3(__NR_write, fd, buffer, __simple_strlen(buffer));
}

__attribute__ ((visibility ("default")))
int __simple_sprintf(char *buffer, const char* format, ...)
{
	va_list vl;

	va_start(vl, format);
	int ret = __simple_vsprintf(buffer, format, vl);
	va_end(vl);

	return ret;
}

__attribute__ ((visibility ("default")))
int __simple_snprintf(char* buffer, size_t max_length, const char* format, ...)
{
	va_list args;
	int ret;

	va_start(args, format);
	ret = __simple_vsnprintf(buffer, max_length, format, args);
	va_end(args);

	return ret;
}

#ifdef isdigit
#undef isdigit
#endif

static int isdigit(char c)
{
	return c >= '0' && c <= '9';
}
static int isdigit16(char c)
{
	if (isdigit(c))
		return 1;
	if (c >= 'a' && c <= 'f')
		return 1;
	if (c >= 'A' && c <= 'F')
		return 1;
	return 0;
}

__attribute__ ((visibility ("default")))
unsigned long long __simple_atoi(const char* str, const char** endp)
{
	unsigned long long value = 0;

	while (isdigit(*str))
	{
		value *= 10;
		value += *str - '0';
		str++;
	}
	if (endp)
		*endp = str;

	return value;
}

__attribute__ ((visibility ("default")))
unsigned long long __simple_atoi16(const char* str, const char** endp)
{
	unsigned long long value = 0;

	while (isdigit16(*str))
	{
		value *= 16;

		if (*str >= '0' && *str <= '9')
			value += *str - '0';
		else if (*str >= 'a' && *str <= 'f')
			value += 10 + (*str - 'a');
		else if (*str >= 'A' && *str < 'F')
			value += 10 + (*str - 'A');

		str++;
	}
	if (endp)
		*endp = str;

	return value;
}

extern void *memmove(void *dest, const void *src, __SIZE_TYPE__ n);
extern void *memcpy(void *dest, const void *src, __SIZE_TYPE__ n);
extern long sys_read(int fd, void* buf, __SIZE_TYPE__ n);

#ifndef min
#	define min(a,b) ((a) < (b) ? (a) : (b))
#endif

__attribute__ ((visibility ("default")))
void __simple_readline_init(struct simple_readline_buf* buf)
{
	buf->used = 0;
}

__attribute__ ((visibility ("default")))
char* __simple_readline(int fd, struct simple_readline_buf* buf, char* out, int max_out)
{
	char* nl = NULL;
	int len;

find_nl:
	if (buf->used == 0 || (nl = memchr(buf->buf, '\n', buf->used)) == NULL)
	{
		// Read some more
		int rd;

		if (sizeof(buf->buf) == buf->used)
		{
			nl = buf->buf + buf->used;
			goto have_nl;
		}

		rd = sys_read(fd, buf->buf + buf->used, sizeof(buf->buf) - buf->used);
		if (rd == 0)
		{
			if (!buf->used)
				return NULL;

			nl = buf->buf + buf->used;
			goto have_nl;
		}
		else
		{
			buf->used += rd;
			goto find_nl;
		}
	}

have_nl:
	if (nl == buf->buf)
		return NULL;

	len = min(max_out - 1, nl - buf->buf);
	memcpy(out, buf->buf, len);
	out[len] = 0;

	buf->used -= len;
	if (buf->used)
		buf->used--; // remove NL
	if (buf->used)
		memmove(buf->buf, nl+1, buf->used);

	return out;
}

__attribute__ ((visibility ("default")))
void __simple_abort(void) {
	sys_kill(0, SIGABRT, 1);
	__builtin_unreachable();
};
