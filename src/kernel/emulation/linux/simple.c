#include "base.h"
#include "simple.h"
#include <stdarg.h>
#include <stddef.h>
#include <asm/unistd.h>

void __simple_vsprintf(char* buf, const char* format, va_list vl);

int __simple_strlen(const char* text)
{
	int len = 0;
	while (*text++)
		len++;
	return len;
}

void __simple_vsprintf(char* buf, const char* format, va_list vl)
{
	while (*format)
	{
		if (*format == '%')
		{
			format++;
			if (!*format)
				break;
			
			switch (*format)
			{
				case '%':
					*buf++ = '%';
					break;
				case 's':
				{
					const char* str = va_arg(vl, const char*);
					if (!str)
						str = "(null)";

					while (*str)
					{
						*buf++ = *str;
						str++;
					}
					break;
				}
				case 'd':
				{
					int num = va_arg(vl, int);
					char temp[16];
					int count = 0;

					if (num < 0)
						*buf++ = '-';

					do
					{
						temp[count++] = '0' + (num % 10);
						num /= 10;
					}
					while (num > 0);
					
					while (count--)
						*buf++ = temp[count];

					break;
				}
				case 'p':
				case 'x':
				{
					unsigned long num = va_arg(vl, unsigned long);
					char temp[40];
					int count = 0;

					if (*format == 'p')
					{
						*buf++ = '0';
						*buf++ = 'x';
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
						*buf++ = temp[count];

					break;

				}
			}

			format++;
		}
		else
		{
			*buf++ = *format;
			format++;
		}
	}

	*buf = 0;
}

void __simple_printf(const char* format, ...)
{
	char buffer[512];
	va_list vl;

	va_start(vl, format);
	__simple_vsprintf(buffer, format, vl);
	va_end(vl);

	LINUX_SYSCALL3(__NR_write, 1, buffer, __simple_strlen(buffer));
}

void __simple_sprintf(char *buffer, const char* format, ...)
{
	va_list vl;

	va_start(vl, format);
	__simple_vsprintf(buffer, format, vl);
	va_end(vl);
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
extern void *memchr(const void *s, int c, __SIZE_TYPE__ n);
extern long sys_read(int fd, void* buf, __SIZE_TYPE__ n);

#ifndef min
#	define min(a,b) ((a) < (b) ? (a) : (b))
#endif

void __simple_readline_init(struct simple_readline_buf* buf)
{
	buf->used = 0;
}

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

