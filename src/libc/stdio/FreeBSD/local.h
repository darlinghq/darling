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
 *
 *	@(#)local.h	8.3 (Berkeley) 7/3/94
 * $FreeBSD: src/lib/libc/stdio/local.h,v 1.33 2008/05/05 16:03:52 jhb Exp $
 */

#include <sys/cdefs.h>
#include "xlocale_private.h"
#include "xprintf_private.h"
#include <sys/types.h>	/* for off_t */
#include <pthread.h>
#include <limits.h>
#include <string.h>
#include <wchar.h>

/*
 * Information local to this implementation of stdio,
 * in particular, macros and private variables.
 */

extern int	_sread(FILE *, char *, int);
extern int	_swrite(FILE *, char const *, int);
extern fpos_t	_sseek(FILE *, fpos_t, int);
extern int	_ftello(FILE *, fpos_t *);
extern int	_fseeko(FILE *, off_t, int, int);
extern int	_vasprintf(printf_comp_t __restrict, printf_domain_t __restrict,
		char ** __restrict, locale_t __restrict,
		const char * __restrict, __va_list);
extern int	_vdprintf(printf_comp_t __restrict, printf_domain_t __restrict,
		int, locale_t __restrict, const char * __restrict, va_list);
extern int	_vsnprintf(printf_comp_t __restrict, printf_domain_t __restrict,
		char * __restrict, size_t n, locale_t __restrict,
		const char * __restrict, __va_list);

extern int	__fflush(FILE *fp);
extern void	__fcloseall(void);
extern wint_t	__fgetwc(FILE *, locale_t);
extern wint_t	__fputwc(wchar_t, FILE *, locale_t);
extern int	__sflush(FILE *);
extern FILE	*__sfp(int);
extern void	__sfprelease(FILE *);	/* mark free and update count as needed */
extern int	__slbexpand(FILE *, size_t);
extern int	__srefill(FILE *);
extern int	__srefill0(FILE *);
extern int	__srefill1(FILE *);
extern int	__sread(void *, char *, int);
extern int	__swrite(void *, char const *, int);
extern fpos_t	__sseek(void *, fpos_t, int);
extern int	__sclose(void *);
extern void	__sinit(void);
extern void	_cleanup(void);
extern void	(*__cleanup)(void);
extern void	__smakebuf(FILE *);
extern int	__swhatbuf(FILE *, size_t *, int *);
extern int	_fwalk(int (*)(FILE *));
extern int	__svfscanf_l(FILE *, locale_t, const char *, __va_list);
extern int	__swsetup(FILE *);
extern int	__sflags(const char *, int *);
extern int	__ungetc(int, FILE *);
extern wint_t	__ungetwc(wint_t, FILE *, locale_t);
extern int	__vfprintf(FILE *, locale_t, const char *, __va_list);
extern int	__vfscanf(FILE *, const char *, __va_list);
extern int	__vfwprintf(FILE *, locale_t, const wchar_t *, __va_list);
extern int	__vfwscanf(FILE * __restrict, locale_t, const wchar_t * __restrict,
		    __va_list);
extern size_t	__fread(void * __restrict buf, size_t size, size_t count,
		FILE * __restrict fp);
extern pthread_once_t	__sdidinit;


/* hold a buncha junk that would grow the ABI */
struct __sFILEX {
	unsigned char	*up;	/* saved _p when _p is doing ungetc data */
	pthread_mutex_t	fl_mutex;	/* used for MT-safety */
	int		orientation:2;	/* orientation for fwide() */
	int		counted:1;	/* stream counted against STREAM_MAX */
	mbstate_t	mbstate;	/* multibyte conversion state */
};

#define _up 		_extra->up
#define _fl_mutex	_extra->fl_mutex
#define _orientation	_extra->orientation
#define _mbstate	_extra->mbstate
#define _counted	_extra->counted



#define	INITEXTRA(fp) do { \
	(fp)->_extra->up = NULL; \
	(fp)->_extra->fl_mutex = (pthread_mutex_t)PTHREAD_RECURSIVE_MUTEX_INITIALIZER; \
	(fp)->_extra->orientation = 0; \
	memset(&(fp)->_extra->mbstate, 0, sizeof(mbstate_t)); \
	(fp)->_extra->counted = 0; \
} while(0);

/*
 * Prepare the given FILE for writing, and return 0 iff it
 * can be written now.  Otherwise, return EOF and set errno.
 */
#define	prepwrite(fp) \
 	((((fp)->_flags & __SWR) == 0 || \
 	    ((fp)->_bf._base == NULL && ((fp)->_flags & __SSTR) == 0)) && \
	 __swsetup(fp))

/*
 * Test whether the given stdio file has an active ungetc buffer;
 * release such a buffer, without restoring ordinary unread data.
 */
#define	HASUB(fp) ((fp)->_ub._base != NULL)
#define	FREEUB(fp) { \
	if ((fp)->_ub._base != (fp)->_ubuf) \
		free((char *)(fp)->_ub._base); \
	(fp)->_ub._base = NULL; \
}

/*
 * test for an fgetln() buffer.
 */
#define	HASLB(fp) ((fp)->_lb._base != NULL)
#define	FREELB(fp) { \
	free((char *)(fp)->_lb._base); \
	(fp)->_lb._base = NULL; \
}

/*
 * Set the orientation for a stream. If o > 0, the stream has wide-
 * orientation. If o < 0, the stream has byte-orientation.
 */
#define	ORIENT(fp, o)	do {				\
	if ((fp)->_orientation == 0)			\
		(fp)->_orientation = (o);		\
} while (0)
