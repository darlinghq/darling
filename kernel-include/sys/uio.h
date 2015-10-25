/*
 * Copyright (c) 2000-2008 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 * 
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
/* Copyright (c) 1995 NeXT Computer, Inc. All Rights Reserved */
/*
 * Copyright (c) 1982, 1986, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
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
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
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
 *	@(#)uio.h	8.5 (Berkeley) 2/22/94
 */

#ifndef _SYS_UIO_H_
#define	_SYS_UIO_H_

#include <sys/cdefs.h>
#include <sys/_types.h>

/*
 * [XSI] The ssize_t and size_t types shall be defined as described
 * in <sys/types.h>.
 */
#ifndef _SIZE_T
#define _SIZE_T
typedef __darwin_size_t		size_t;
#endif

#ifndef	_SSIZE_T
#define	_SSIZE_T
typedef	__darwin_ssize_t	ssize_t;
#endif

/*
 * [XSI] Structure whose address is passed as the second parameter to the
 * readv() and writev() functions.
 */
#ifndef _STRUCT_IOVEC
#define	_STRUCT_IOVEC
struct iovec {
	void *   iov_base;	/* [XSI] Base address of I/O memory region */
	size_t	 iov_len;	/* [XSI] Size of region iov_base points to */
};
#endif


#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
/*
 * IO direction for uio_t.
 *	UIO_READ - data moves into iovec(s) associated with uio_t
 *	UIO_WRITE - data moves out of iovec(s) associated with uio_t
 */
enum uio_rw { UIO_READ, UIO_WRITE };
#endif

#ifdef KERNEL

/*
 * XXX This all really wants a uio_internal.h
 */

#include <sys/kernel_types.h>


/*
 * user / kernel address space type flags.
 * WARNING - make sure to check when adding flags!  Be sure new flags
 * don't overlap the definitions in uio_internal.h
 * NOTES - 
 *	UIO_USERSPACE is equivalent to UIO_USERSPACE32, but UIO_USERSPACE32
 *		is preferred.  UIO_USERSPACE remains for backwards compatibility.
 *	UIO_SYSSPACE is equivalent to UIO_SYSSPACE32, but UIO_SYSSPACE
 *		is preferred.
 */
enum uio_seg {
	UIO_USERSPACE 		= 0,	/* kernel address is virtual,  to/from user virtual */
	UIO_SYSSPACE 		= 2,	/* kernel address is virtual,  to/from system virtual */
	UIO_USERSPACE32 	= 5,	/* kernel address is virtual,  to/from user 32-bit virtual */
	UIO_USERSPACE64 	= 8,	/* kernel address is virtual,  to/from user 64-bit virtual */
	UIO_SYSSPACE32 		= 11	/* deprecated */
};

#define UIO_SEG_IS_USER_SPACE( a_uio_seg )  \
	( (a_uio_seg) == UIO_USERSPACE64 || (a_uio_seg) == UIO_USERSPACE32 || \
	  (a_uio_seg) == UIO_USERSPACE )


__BEGIN_DECLS

/*
 * uio_create - create an uio_t.
 * 	Space is allocated to hold up to a_iovcount number of iovecs.  The uio_t
 *	is not fully initialized until all iovecs are added using uio_addiov calls.
 *	a_iovcount is the maximum number of iovecs you may add.
 */
uio_t uio_create( int a_iovcount,		/* max number of iovecs */
				  off_t a_offset,		/* current offset */
				  int a_spacetype,		/* type of address space */
				  int a_iodirection );	/* read or write flag */

/*
 * uio_reset - reset an uio_t.
 * 	Reset the given uio_t to initial values.  The uio_t is not fully initialized
 * 	until all iovecs are added using uio_add_ov calls.
 *	The a_iovcount value passed in the uio_create is the maximum number of 
 *	iovecs you may add.
 */
void uio_reset( uio_t a_uio,
				off_t a_offset,			/* current offset */
				int a_spacetype,		/* type of address space */
				int a_iodirection );	/* read or write flag */

/*
 * uio_duplicate - allocate a new uio and make a copy of the given uio_t.
 *	may return NULL.
 */
uio_t uio_duplicate( uio_t a_uio );


/*
 * uio_free - free a uio_t allocated via uio_create. 
 */
void uio_free( uio_t a_uio );

/*
 * uio_addiov - add an iovec to the given uio_t.  You may call this up to
 * 	the a_iovcount number that was passed to uio_create. 
 *	returns 0 if add was successful else non zero.
 */
int uio_addiov( uio_t a_uio, user_addr_t a_baseaddr, user_size_t a_length );

/*
 * uio_getiov - get iovec data associated with the given uio_t.  Use
 *  a_index to iterate over each iovec (0 to (uio_iovcnt(uio_t) - 1)).
 *  a_baseaddr_p and a_length_p may be NULL.
 * 	returns -1 when a_index is out of range or invalid uio_t.
 *	returns 0 when data is returned.
 */
int uio_getiov( uio_t a_uio, 
                 int a_index, 
                 user_addr_t * a_baseaddr_p, 
                 user_size_t * a_length_p );

/*
 * uio_update - update the given uio_t for a_count of completed IO.
 *	This call adjusts decrements the current iovec length and residual IO,
 *	and increments the current iovec base address and offset value. 
 */
void uio_update( uio_t a_uio, user_size_t a_count );

/*
 * uio_resid - return the residual IO value for the given uio_t
 */
user_ssize_t uio_resid( uio_t a_uio );

/*
 * uio_setresid - set the residual IO value for the given uio_t
 */
void uio_setresid( uio_t a_uio, user_ssize_t a_value );

/*
 * uio_iovcnt - return count of active iovecs for the given uio_t
 */
int uio_iovcnt( uio_t a_uio );

/*
 * uio_offset - return the current offset value for the given uio_t
 */
off_t uio_offset( uio_t a_uio );

/*
 * uio_setoffset - set the current offset value for the given uio_t
 */
void uio_setoffset( uio_t a_uio, off_t a_offset );

/*
 * uio_rw - return the read / write flag for the given uio_t
 */
int uio_rw( uio_t a_uio );

/*
 * uio_setrw - set the read / write flag for the given uio_t
 */
void uio_setrw( uio_t a_uio, int a_value );

/*
 * uio_isuserspace - return non zero value if the address space 
 * flag is for a user address space (could be 32 or 64 bit).
 */
int uio_isuserspace( uio_t a_uio );

/*
 * uio_curriovbase - return the base address of the current iovec associated 
 *	with the given uio_t.  May return 0.
 */
user_addr_t uio_curriovbase( uio_t a_uio );

/*
 * uio_curriovlen - return the length value of the current iovec associated 
 *	with the given uio_t.
 */
user_size_t uio_curriovlen( uio_t a_uio );

/*
 * Limits
 */
#define UIO_MAXIOV	1024		/* max 1K of iov's */
#define UIO_SMALLIOV	8		/* 8 on stack, else malloc */

extern int uiomove(const char * cp, int n, struct uio *uio);
extern int uiomove64(const __uint64_t cp, int n, struct uio *uio);
__END_DECLS

#endif /* KERNEL */

#ifndef	KERNEL

__BEGIN_DECLS
ssize_t	readv(int, const struct iovec *, int) __DARWIN_ALIAS_C(readv);
ssize_t	writev(int, const struct iovec *, int) __DARWIN_ALIAS_C(writev);
__END_DECLS
#endif /* !KERNEL */

#endif /* !_SYS_UIO_H_ */
