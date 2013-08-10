#include "visibility.h"
#include <sys/ioctl.h>
#include <linux/kd.h>

/* GNUstep defines these as static const, Apple actually exports them */

#ifdef __x86_64__
#	define CGFloat double
#else
#	define CGFloat float
#endif

struct FLPair
{
	CGFloat a, b;
};

DARLING_VISIBLE const struct FLPair NSZeroPoint = { 0, 0 };
DARLING_VISIBLE const struct FLPair NSZeroSize = { 0, 0 };
DARLING_VISIBLE const struct FLPair NSZeroRect = { 0, 0 };

DARLING_VISIBLE void NSBeep()
{
	int ms = 5000;
	int freq = 440;
	int fd = 0;
	
	ioctl(fd, KDMKTONE, (ms<<16 | 1193180/freq));
}
