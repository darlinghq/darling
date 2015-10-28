#include "secure.h"
#include <syslog.h>
#include <sys/sysctl.h>
#include <sys/param.h>
#include <unistd.h>
#include <stdint.h>

extern void __abort(void) __dead2;
extern void __chk_fail (void) __attribute__((__noreturn__));

void __chk_fail_overflow (void) __attribute__((__noreturn__))
{
	__chk_fail();
}

void __chk_fail_overlap (void) __attribute__((__noreturn__))
{
  const char message[] = "[%d] detected illegal buffer overlap";

  syslog(LOG_CRIT, message, getpid());

  __abort();
}

void __chk_overlap (const void *a, size_t an, const void *b, size_t bn)
{
	if (((uintptr_t)a) <= ((uintptr_t)b)+bn && ((uintptr_t)b) <= ((uintptr_t)a)+an)
		__chk_fail_overlap();
}

