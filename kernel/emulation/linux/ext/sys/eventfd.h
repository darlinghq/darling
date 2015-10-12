#ifndef EXT_EVENTFD_H
#define EXT_EVENTFD_H
#include <stdint.h>

#ifndef __THROW
#	define __THROW
#endif

// #define eventfd			__linux_eventfd
#define eventfd_read	__linux_eventfd_read
#define eventfd_write	__linux_eventfd_write

typedef uint64_t eventfd_t;

extern int __linux_eventfd (int __count, int __flags) __THROW;

static inline int eventfd (int __count, int __flags)
{
	return __linux_eventfd(__count, __flags);
}

extern int eventfd_read (int __fd, eventfd_t *__value);
extern int eventfd_write (int __fd, eventfd_t __value);

#endif

