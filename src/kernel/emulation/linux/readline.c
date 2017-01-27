#include "readline.h"
#include <stddef.h>
#include "unistd/read.h"

extern void *memmove(void *dest, const void *src, __SIZE_TYPE__ n);
extern char* memchr(char* buf, int c, __SIZE_TYPE__ n);

const char* _readline(int fd, struct rdline_buffer* buf)
{
	if (buf->next != 0)
	{
		memmove(buf->buffer, buf->buffer + buf->next, buf->usage - buf->next);
		buf->usage -= buf->next;
		buf->next = 0;
	}

	int eof = 0;
	while (1)
	{
		if (buf->usage > 0)
		{
			char* p = (char*) memchr(buf->buffer, '\n', buf->usage);
			
			if (p)
			{
				*p = '\0';
				buf->next = p + 1 - buf->buffer;
				return buf->buffer;
			}
			else if (eof)
			{
				buf->buffer[buf->usage] = '\0'; // this is OK, because we never fully fill the buffer
				buf->usage = 0;
				return buf->buffer;
			}
		}
		else if (eof)
			return NULL;

		int count = sys_read_nocancel(fd, buf->buffer + buf->usage, sizeof(buf->buffer) - buf->usage - 1);
		if (count < 0)
			return NULL;
		else if (count == 0)
			eof = 1;
		buf->usage += count;
	}
}

void _readline_init(struct rdline_buffer* buf)
{
	buf->usage = 0;
	buf->next = 0;
}

