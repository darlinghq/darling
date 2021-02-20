/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
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
 */

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcomma"

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)findfp.c	8.2 (Berkeley) 1/4/94";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/stdio/findfp.c,v 1.34 2009/12/05 19:31:38 ed Exp $");

#include <TargetConditionals.h>

#include <sys/param.h>
#include <machine/atomic.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libkern/OSAtomic.h>
#include <errno.h>

#include <pthread.h>
#include <spinlock.h>

#include "libc_private.h"
#include "local.h"
#include "glue.h"

pthread_once_t	__sdidinit;

#if !TARGET_OS_EMBEDDED
#define	NDYNAMIC 10		/* add ten more whenever necessary */
#else
#define	NDYNAMIC 1		/* add one at a time on embedded */
#endif

#define	std(flags, file) {		\
	._flags = (flags),		\
	._file = (file),		\
	._cookie = __sF + (file),	\
	._close = __sclose,		\
	._read = __sread,		\
	._seek = __sseek,		\
	._write = __swrite,		\
	._extra = __sFX + file,         \
}
  /* set counted */
#define __sFXInit3      {.fl_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER, .counted = 1}

static int __scounted;		/* streams counted against STREAM_MAX */
static int __stream_max;

#if !TARGET_OS_EMBEDDED
/* usual and usual_extra are data pigs. See 7929728. For embedded we should
 * always allocate dynamically, and probably should for desktop too. */
				/* the usual - (stdin + stdout + stderr) */
static FILE usual[FOPEN_MAX - 3];
static struct __sFILEX usual_extra[FOPEN_MAX - 3];
static struct glue uglue = { NULL, FOPEN_MAX - 3, usual };
#endif /* !TARGET_OS_EMBEDDED */

static struct __sFILEX __sFX[3] = {__sFXInit3, __sFXInit3, __sFXInit3};

/*
 * We can't make this 'static' due to binary compatibility concerns.
 * This also means we cannot change the sizeof(FILE) and must continue to
 * use the __sFILEX stuff to add to FILE.
 */
FILE __sF[3] = {
	std(__SRD, STDIN_FILENO),
	std(__SWR, STDOUT_FILENO),
	std(__SWR|__SNBF, STDERR_FILENO)
};

FILE *__stdinp = &__sF[0];
FILE *__stdoutp = &__sF[1];
FILE *__stderrp = &__sF[2];

#if !TARGET_OS_EMBEDDED
struct glue __sglue = { &uglue, 3, __sF };
static struct glue *lastglue = &uglue;
#else
struct glue __sglue = { NULL, 3, __sF };
static struct glue *lastglue = &__sglue;
#endif

static struct glue *	moreglue(int);

static pthread_mutex_t filelist_lock = PTHREAD_MUTEX_INITIALIZER;
#define FILELIST_LOCK()	do { pthread_mutex_lock(&filelist_lock); } while(0)
#define FILELIST_UNLOCK()	do { pthread_mutex_unlock(&filelist_lock); } while(0)

static struct glue *
moreglue(int n)
{
	struct glue *g;
	FILE *p;
	struct __sFILEX *fx;
	size_t align;

	align = __alignof__( (struct { FILE f; struct __sFILEX s; }){} );
	g = (struct glue *)malloc(sizeof(*g) + align + n * sizeof(FILE) +
	    n * sizeof(struct __sFILEX));
	if (g == NULL)
		return (NULL);
	p = (FILE *)roundup((uintptr_t)(g + 1), align);
	fx = (struct __sFILEX *)&p[n];
	g->next = NULL;
	g->niobs = n;
	g->iobs = p;
        
	while (--n >= 0) {
		bzero(p, sizeof(*p));
		p->_extra = fx;
		INITEXTRA(p);
		p++, fx++;
	}
	return (g);
}

/*
 * Find a free FILE for fopen et al.
 */
FILE *
__sfp(int count)
{
	FILE	*fp;
	int	n;
	struct glue *g;

	pthread_once(&__sdidinit, __sinit);

	if (count) {
		int32_t new = OSAtomicIncrement32(&__scounted);
		if (new > __stream_max) {
			if (new > (__stream_max = sysconf(_SC_STREAM_MAX))){
				OSAtomicDecrement32(&__scounted);
				errno = EMFILE;
				return NULL;
			}
		}
	}
	/*
	 * The list must be locked because a FILE may be updated.
	 */
	FILELIST_LOCK();
	for (g = &__sglue; g != NULL; g = g->next) {
		for (fp = g->iobs, n = g->niobs; --n >= 0; fp++)
			if (fp->_flags == 0)
				goto found;
	}
	FILELIST_UNLOCK();	/* don't hold lock while malloc()ing. */
	if ((g = moreglue(NDYNAMIC)) == NULL)
		return (NULL);
	FILELIST_LOCK();		/* reacquire the lock */
	lastglue->next = g; /* atomically append glue to list */
	lastglue = g;		/* not atomic; only accessed when locked */
	fp = g->iobs;
found:
	fp->_flags = 1;		/* reserve this slot; caller sets real flags */
	FILELIST_UNLOCK();
	
	/* _flags = 1 means the FILE* is in use, and this thread owns the object while it is being initialized */
	fp->_p = NULL;		/* no current pointer */
	fp->_w = 0;		/* nothing to read or write */
	fp->_r = 0;
	fp->_bf._base = NULL;	/* no buffer */
	fp->_bf._size = 0;
	fp->_lbfsize = 0;	/* not line buffered */
	fp->_file = -1;		/* no file */
/*	fp->_cookie = <any>; */	/* caller sets cookie, _read/_write etc */
	fp->_ub._base = NULL;	/* no ungetc buffer */
	fp->_ub._size = 0;
	fp->_lb._base = NULL;	/* no line buffer */
	fp->_lb._size = 0;
/*	fp->_lock = NULL; */	/* once set always set (reused) */
	INITEXTRA(fp);
	fp->_extra->counted = count ? 1 : 0;
	return (fp);
}

/*
 * Mark as free and update count as needed
 */
__private_extern__ void
__sfprelease(FILE *fp)
{
	if (fp->_counted) {
		OSAtomicDecrement32(&__scounted);
		fp->_counted = 0;
	}
	
	pthread_mutex_destroy(&fp->_extra->fl_mutex);
	
	/* Make sure nobody else is enumerating the list while we clear the "in use" _flags field. */
	FILELIST_LOCK();
	fp->_flags = 0;
	FILELIST_UNLOCK();
}

/*
 * XXX.  Force immediate allocation of internal memory.  Not used by stdio,
 * but documented historically for certain applications.  Bad applications.
 */
__warn_references(f_prealloc, 
	"warning: this program uses f_prealloc(), which is not recommended.");

void
f_prealloc(void)
{
	struct glue *g;
	int n;

	n = getdtablesize() - FOPEN_MAX + 20;		/* 20 for slop. */
	/*
	 * It should be safe to walk the list without locking it;
	 * new nodes are only added to the end and none are ever
	 * removed.
	 */
	for (g = &__sglue; (n -= g->niobs) > 0 && g->next; g = g->next)
		/* void */;
	if ((n > 0) && ((g = moreglue(n)) != NULL)) {
		FILELIST_LOCK();
		lastglue->next = g;
		lastglue = g;
		FILELIST_UNLOCK();
	}
}

/*
 * exit() calls _cleanup() through *__cleanup, set whenever we
 * open or buffer a file.  This chicanery is done so that programs
 * that do not use stdio need not link it all in.
 *
 * The name `_cleanup' is, alas, fairly well known outside stdio.
 */
void
_cleanup(void)
{
	/* (void) _fwalk(fclose); */
	(void) _fwalk(__sflush);		/* `cheating' */
}

/*
 * __sinit() is called whenever stdio's internal variables must be set up.
 */
void
__sinit(void)
{
#if !TARGET_OS_EMBEDDED
	int i;
#endif

	/* Make sure we clean up on exit. */
	__cleanup = _cleanup;		/* conservative */
	__stream_max = sysconf(_SC_STREAM_MAX);
	__scounted = 3;			/* std{in,out,err} already exists */

#if !TARGET_OS_EMBEDDED
	/* Set _extra for the usual suspects. */
	for (i = 0; i < FOPEN_MAX - 3; i++) {
		usual[i]._extra = &usual_extra[i];
		INITEXTRA(&usual[i]);
	}
#endif
}
#pragma clang diagnostic pop
