#ifndef DEBUG_H
#define	DEBUG_H
#include <linux/printk.h>
#include <linux/slab.h>

#ifdef DARLING_DEBUG
#	define debug_msg(fmt, ...) printk(KERN_DEBUG "Darling Mach: " fmt, ##__VA_ARGS__)
#else
#	define debug_msg(fmt, ...)
#endif

//#define DARLING_MEM_DEBUG // better use slub_debug
#ifdef DARLING_MEM_DEBUG
#	define kmalloc dbg_kmalloc
#	define kfree dbg_kfree
void* dbg_kmalloc(size_t size, int flags);
void dbg_kfree(const void* mem);
#endif

#endif	/* DEBUG_H */

