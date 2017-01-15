#ifndef _READLINE_H
#define _READLINE_H

struct rdline_buffer
{
	char buffer[1024];
	unsigned int usage, next;
};

void _readline_init(struct rdline_buffer* buf);
const char* _readline(int fd, struct rdline_buffer* buf);

#endif

