#ifndef DEBUG_H
#define	DEBUG_H
#include <linux/printk.h>

#ifdef DARLING_DEBUG
#	define debug_msg(fmt, ...) printk(KERN_DEBUG "Darling Mach: " fmt, ##__VA_ARGS__)
#else
#	define debug_msg(fmt, ...)
#endif

#endif	/* DEBUG_H */

