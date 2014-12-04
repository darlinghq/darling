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
 * Copyright (c) 1998 John Birrell <jb@cimlogic.com.au>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by John Birrell.
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY JOHN BIRRELL AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: src/lib/libc/include/spinlock.h,v 1.5 2002/03/21 22:47:52 obrien Exp $
 *
 * Lock definitions used in both libc and libpthread.
 *
 */

#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_
#ifdef __APPLE__
#include <pthread_spinlock.h>

typedef pthread_lock_t spinlock_t;

#define	_SPINLOCK_INITIALIZER	LOCK_INITIALIZER

#define	_SPINLOCK(_lck)							\
    do {								\
	_DO_SPINLOCK_LOCK(_lck);					\
    } while (0)

#define _SPINUNLOCK(_lck)						\
    do {								\
	_DO_SPINLOCK_UNLOCK(_lck);					\
    } while (0)

#else /* ! __APPLE__ */
#include <sys/cdefs.h>
#include <sys/types.h>

/*
 * Lock structure with room for debugging information.
 */
typedef struct {
	volatile long	access_lock;
	volatile long	lock_owner;
	volatile char	*fname;
	volatile int	lineno;
} spinlock_t;

#define	_SPINLOCK_INITIALIZER	{ 0, 0, 0, 0 }

#define _SPINUNLOCK(_lck)	(_lck)->access_lock = 0
#ifdef	_LOCK_DEBUG
#define	_SPINLOCK(_lck)		_spinlock_debug(_lck, __FILE__, __LINE__)
#else
#define	_SPINLOCK(_lck)		_spinlock(_lck)
#endif

/*
 * Thread function prototype definitions:
 */
__BEGIN_DECLS
long	_atomic_lock(volatile long *);
void	_spinlock(spinlock_t *);
void	_spinlock_debug(spinlock_t *, char *, int);
__END_DECLS

#endif /* __APPLE__ */

#endif /* _SPINLOCK_H_ */
