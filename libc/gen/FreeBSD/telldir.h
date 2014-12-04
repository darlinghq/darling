/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Copyright (c) 2000
 * 	Daniel Eischen.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
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
 * $FreeBSD: src/lib/libc/gen/telldir.h,v 1.3 2008/05/05 14:05:23 kib Exp $
 */

#ifndef _TELLDIR_H_
#define	_TELLDIR_H_

#include <sys/queue.h>

/*
 * One of these structures is malloced to describe the current directory
 * position each time telldir is called. It records the current magic
 * cookie returned by getdirentries and the offset within the buffer
 * associated with that return value.
 */
struct ddloc {
	LIST_ENTRY(ddloc) loc_lqe; /* entry in list */
	long	loc_index;	/* key associated with structure */
#if __DARWIN_64_BIT_INO_T
	__darwin_off_t	loc_seek;	/* returned by lseek */
#else /* !__DARWIN_64_BIT_INO_T */
	long	loc_seek;	/* magic cookie returned by getdirentries */
#endif /* __DARWIN_64_BIT_INO_T */
	long	loc_loc;	/* offset of entry in buffer */
};

/*
 * One of these structures is malloced for each DIR to record telldir
 * positions.
 */
struct _telldir {
	LIST_HEAD(, ddloc) td_locq; /* list of locations */
	long	td_loccnt;	/* index of entry for sequential readdir's */
#if __DARWIN_64_BIT_INO_T
	__darwin_off_t seekoff;	/* 64-bit seek offset */
#endif /* __DARWIN_64_BIT_INO_T */
};

#if __DARWIN_64_BIT_INO_T
size_t		__getdirentries64(int fd, void *buf, size_t bufsize, __darwin_off_t *basep);
#endif /* __DARWIN_64_BIT_INO_T */
struct dirent	*_readdir_unlocked(DIR *, int) __DARWIN_INODE64(_readdir_unlocked);
void 		_reclaim_telldir(DIR *);
void 		_seekdir(DIR *, long) __DARWIN_ALIAS_I(_seekdir);
long		telldir(DIR *) __DARWIN_ALIAS_I(telldir);

#endif
