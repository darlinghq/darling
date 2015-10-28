#include "./sys/eventfd.h"
#include "../errno.h"
#include "../base.h"
#include <asm/unistd.h>

extern long cerror(int __err);
extern long write(int fd, const void *buf, unsigned long count);

VISIBLE
int eventfd_write (int __fd, eventfd_t __value)
{
	return write(__fd, &__value, sizeof(eventfd_t)) == sizeof(eventfd_t)
		? 0 : -1;
}

