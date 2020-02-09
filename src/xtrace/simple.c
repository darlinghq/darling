#include "simple.h"
#include <stdarg.h>
#include <stddef.h>

int __simple_vsprintf(char* buf, const char* format, va_list vl);
extern char* memchr(char* buf, int c, __SIZE_TYPE__ n);

// We cannot call standard write(), because it would loop back to xtrace
extern int __write_for_xtrace(int fd, const void* mem, __SIZE_TYPE__ count);

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

static inline void print_num(char** buf, long long num)
{
	char temp[128];
	int count = 0;

	if (num < 0)
	{
		*(*buf)++ = '-';
		num = -num;
	}

	do
	{
		temp[count++] = '0' + (num % 10);
		num /= 10;
	}
	while (num > 0);

	while (count--)
		*(*buf)++ = temp[count];
}

int __simple_vsprintf(char* buf, const char* format, va_list vl)
{
	const char* initial_buf = buf;
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
				case 'c':
					*buf++ = (char) va_arg(vl, int);
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
					print_num(&buf, num);
					break;
				}
				case 'u':
				{
					unsigned num = va_arg(vl, unsigned);
					print_num(&buf, num);
					break;
				}
				case 'l':
				{
					format++;
					if (*format == 'd')
					{
						long num = va_arg(vl, long);
						print_num(&buf, num);
					}
					else if (*format == 'u')
					{
						unsigned long num = va_arg(vl, unsigned long);
						print_num(&buf, num);
					}
					else if (*format == 'l')
					{
						format++;
						if (*format == 'd')
						{
							long long num = va_arg(vl, long long);
							print_num(&buf, num);
						}
						else if (*format == 'u')
						{
							unsigned long long num = va_arg(vl, unsigned long long);
							char temp[128];
							int count = 0;
							do
							{
								temp[count++] = '0' + (num % 10);
								num /= 10;
							}
							while (num > 0);

							while (count--)
								*buf++ = temp[count];
						}
					}
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
							temp[count++] = 'a' + (c - 10);
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
	return buf - initial_buf;
}

__attribute__ ((visibility ("default")))
void __simple_printf(const char* format, ...)
{
	char buffer[512];
	va_list vl;

	va_start(vl, format);
	__simple_vsprintf(buffer, format, vl);
	va_end(vl);

	__write_for_xtrace(2, buffer, __simple_strlen(buffer));
}

int __simple_sprintf(char *buffer, const char* format, ...)
{
	va_list vl;
	int res;

	va_start(vl, format);
	res = __simple_vsprintf(buffer, format, vl);
	va_end(vl);

	return res;
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

