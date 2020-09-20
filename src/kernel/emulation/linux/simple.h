#ifndef LINUX_DEBUG_H
#define LINUX_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

void __simple_printf(const char* format, ...);
void __simple_kprintf(const char* format, ...);
void __simple_fprintf(int fd, const char* format, ...);
void __simple_sprintf(char *buffer, const char* format, ...);
int __simple_strlen(const char* str);

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

