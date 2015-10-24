#ifndef _PTWQ_POSIX_PLATFORM_H
#define _PTWQ_POSIX_PLATFORM_H 1

/* Workaround to get visibility for _SC_NPROCESSORS_ONLN on FreeBSD */
#define __BSD_VISIBLE 1

#include <sys/resource.h>
#include <sys/queue.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <pthread.h>

#ifdef __sun
# include <sys/loadavg.h>
#endif

/* GCC atomic builtins. 
 * See: http://gcc.gnu.org/onlinedocs/gcc-4.1.0/gcc/Atomic-Builtins.html 
 */
#ifdef __sun
# include <atomic.h>
# define atomic_inc      atomic_inc_32
# define atomic_dec      atomic_dec_32
# define atomic_and      atomic_and_uint_nv
# define atomic_or      atomic_or_uint_nv
#else
# define atomic_inc(p)   __sync_add_and_fetch((p), 1)
# define atomic_dec(p)   __sync_sub_and_fetch((p), 1)
# define atomic_and(p,v)   __sync_and_and_fetch((p), (v))
# define atomic_or(p,v)   __sync_or_and_fetch((p), (v))
#endif

#ifdef MAKE_STATIC
# define CONSTRUCTOR
#else
# define CONSTRUCTOR __attribute__ ((constructor))
#endif
#define VISIBLE     __attribute__((visibility("default")))


#endif  /* _PTWQ_POSIX_PLATFORM_H */
