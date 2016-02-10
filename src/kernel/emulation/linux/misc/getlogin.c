#include "getlogin.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include <stddef.h>

extern char *getenv(const char *name);
extern unsigned long strlcpy(char* dst, const char* src, unsigned long size);

long sys_getlogin(char* buf, unsigned int len)
{
	int ret;
	char* e;

	e = getenv("USER");
	if (e != NULL)
	{
		return strlcpy(buf, e, len);
	}
	else
	{
		*buf = '\0';
		return 0;
	}
}

