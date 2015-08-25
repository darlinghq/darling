#include "base.h"
#include "debug.h"
#include <stdarg.h>
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

