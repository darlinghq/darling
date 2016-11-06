#ifndef LINUX_DEBUG_H
#define LINUX_DEBUG_H

void __simple_printf(const char* format, ...);
void __simple_sprintf(char *buffer, const char* format, ...);
int __simple_strlen(const char* str);

struct simple_readline_buf
{
	char buf[512];
	int used;
};
void __simple_readline_init(struct simple_readline_buf* buf);
char* __simple_readline(int fd, struct simple_readline_buf* buf, char* out, int max_out);

#endif

