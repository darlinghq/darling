/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * Copyright 1996 1995 by Open Software Foundation, Inc. 1997 1996 1995 1994 1993 1992 1991  
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 * 
 */
/*
 * MkLinux
 */

/*
 * POSIX Threads - IEEE 1003.1c
 */

#ifndef _POSIX_PTHREAD_SPINLOCK_H
#define _POSIX_PTHREAD_SPINLOCK_H

#include <mach/mach.h>
#define __APPLE_API_PRIVATE
#include <machine/cpu_capabilities.h>

#ifndef __POSIX_LIB__
#define __POSIX_LIB__
#endif

#include "pthread_machdep.h"		/* Machine-dependent definitions. */

/* Number of times to spin when the lock is unavailable and we are on a
   multiprocessor.  On a uniprocessor we yield the processor immediately.  */
#define	MP_SPIN_TRIES	1000
extern int _spin_tries;
extern int __is_threaded;

/* Internal mutex locks for data structures */
#define TRY_LOCK(v) (!__is_threaded || _spin_lock_try((pthread_lock_t *)&(v)))

/* _DO_SPINLOCK_LOCK() takes a (pthread_lock_t *) */
#define _DO_SPINLOCK_LOCK(v) _spin_lock(v)

/* _DO_SPINLOCK_UNLOCK() takes a (pthread_lock_t *) */
#define _DO_SPINLOCK_UNLOCK(v)	_spin_unlock(v)

/* LOCK() takes a (pthread_lock_t) */
#define LOCK(v)								\
    do {								\
	if (__is_threaded) {						\
	    _DO_SPINLOCK_LOCK((pthread_lock_t *)&(v));			\
	}								\
    } while (0)

/* UNLOCK() takes a (pthread_lock_t) */
#define UNLOCK(v)							\
    do {								\
	if (__is_threaded) {						\
	    _DO_SPINLOCK_UNLOCK((pthread_lock_t *)&(v));		\
	}								\
    } while (0)

/* Prototypes. */

/* Functions defined in machine-dependent files. */
extern void _spin_lock(pthread_lock_t *lockp);
extern int _spin_lock_try(pthread_lock_t *lockp);
extern void _spin_unlock(pthread_lock_t *lockp);

#endif /* _POSIX_PTHREAD_SPINLOCK_H */
