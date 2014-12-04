/*
 * Copyright (c) 2000, 2002 - 2008 Apple Inc. All rights reserved.
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
/*-
 * Copyright (c) 1990, 1993
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
 *	@(#)stdlib.h	8.5 (Berkeley) 5/19/95
 */

#ifndef _STDLIB_H_
#define _STDLIB_H_

#include <Availability.h>

#include <_types.h>
#if !defined(_ANSI_SOURCE)
#include <sys/wait.h>
#if (!defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE))
#include <alloca.h>
#endif /* (!_POSIX_C_SOURCE || _DARWIN_C_SOURCE) */
#endif /* !_ANSI_SOURCE */

#ifndef	_SIZE_T
#define	_SIZE_T
/* DO NOT REMOVE THIS COMMENT: fixincludes needs to see:
 * _GCC_SIZE_T */
typedef	__darwin_size_t		size_t;
#endif

#if !defined(_ANSI_SOURCE) && (!defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE))
#ifndef _CT_RUNE_T
#define _CT_RUNE_T
typedef	__darwin_ct_rune_t	ct_rune_t;
#endif

#ifndef _RUNE_T
#define _RUNE_T
typedef __darwin_rune_t   	rune_t;
#endif
#endif	/* !_ANSI_SOURCE && (!_POSIX_C_SOURCE || _DARWIN_C_SOURCE) */

#ifndef	__cplusplus
#ifndef	_WCHAR_T
#define	_WCHAR_T
typedef	__darwin_wchar_t	wchar_t;
#endif	/* _WCHAR_T */
#endif	/* __cplusplus */

typedef struct {
	int quot;		/* quotient */
	int rem;		/* remainder */
} div_t;

typedef struct {
	long quot;		/* quotient */
	long rem;		/* remainder */
} ldiv_t;

#if !__DARWIN_NO_LONG_LONG
typedef struct {
	long long quot;
	long long rem;
} lldiv_t;
#endif /* !__DARWIN_NO_LONG_LONG */

#ifndef NULL
#define NULL __DARWIN_NULL
#endif /* ! NULL */

#define	EXIT_FAILURE	1
#define	EXIT_SUCCESS	0

#define	RAND_MAX	0x7fffffff

#ifdef _USE_EXTENDED_LOCALES_
#include <_xlocale.h>
#endif /* _USE_EXTENDED_LOCALES_ */

#ifndef MB_CUR_MAX
#ifdef _USE_EXTENDED_LOCALES_
#define	MB_CUR_MAX	(___mb_cur_max())
#ifndef MB_CUR_MAX_L
#define	MB_CUR_MAX_L(x)	(___mb_cur_max_l(x))
#endif /* !MB_CUR_MAX_L */
#else /* !_USE_EXTENDED_LOCALES_ */
extern int __mb_cur_max;
#define	MB_CUR_MAX	__mb_cur_max
#endif /* _USE_EXTENDED_LOCALES_ */
#endif /* MB_CUR_MAX */

#if !defined(_ANSI_SOURCE) && (!defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)) \
    && defined(_USE_EXTENDED_LOCALES_) && !defined(MB_CUR_MAX_L)
#define	MB_CUR_MAX_L(x)	(___mb_cur_max_l(x))
#endif
//Begin-Libc
/* f must be a literal string */
#define LIBC_ABORT(f,...)	abort_report_np("%s:%s:%u: " f, __FILE__, __func__, __LINE__, ## __VA_ARGS__)
//End-Libc

__BEGIN_DECLS
void	 abort(void) __dead2;
//Begin-Libc
__private_extern__
void	 abort_report_np(const char *, ...) __dead2 __printflike(1, 2);
//End-Libc
int	 abs(int) __pure2;
int	 atexit(void (*)(void));
double	 atof(const char *);
int	 atoi(const char *);
long	 atol(const char *);
#if !__DARWIN_NO_LONG_LONG
long long
	 atoll(const char *);
#endif /* !__DARWIN_NO_LONG_LONG */
void	*bsearch(const void *, const void *, size_t,
	    size_t, int (*)(const void *, const void *));
void	*calloc(size_t, size_t);
div_t	 div(int, int) __pure2;
void	 exit(int) __dead2;
void	 free(void *);
char	*getenv(const char *);
long	 labs(long) __pure2;
ldiv_t	 ldiv(long, long) __pure2;
#if !__DARWIN_NO_LONG_LONG
long long
	 llabs(long long);
lldiv_t	 lldiv(long long, long long);
#endif /* !__DARWIN_NO_LONG_LONG */
void	*malloc(size_t);
int	 mblen(const char *, size_t);
size_t	 mbstowcs(wchar_t * __restrict , const char * __restrict, size_t);
int	 mbtowc(wchar_t * __restrict, const char * __restrict, size_t);
int 	 posix_memalign(void **, size_t, size_t) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_3_0);
void	 qsort(void *, size_t, size_t,
	    int (*)(const void *, const void *));
int	 rand(void);
void	*realloc(void *, size_t);
void	 srand(unsigned);
double	 strtod(const char *, char **) __DARWIN_ALIAS(strtod);
float	 strtof(const char *, char **) __DARWIN_ALIAS(strtof);
long	 strtol(const char *, char **, int);
long double
	 strtold(const char *, char **);
#if !__DARWIN_NO_LONG_LONG
long long 
	 strtoll(const char *, char **, int);
#endif /* !__DARWIN_NO_LONG_LONG */
unsigned long
	 strtoul(const char *, char **, int);
#if !__DARWIN_NO_LONG_LONG
unsigned long long
	 strtoull(const char *, char **, int);
#endif /* !__DARWIN_NO_LONG_LONG */
//Begin-Libc
#ifndef LIBC_ALIAS_SYSTEM
//End-Libc
int	 system(const char *) __DARWIN_ALIAS_C(system);
//Begin-Libc
#else /* LIBC_ALIAS_SYSTEM */
int	 system(const char *) LIBC_ALIAS_C(system);
#endif /* !LIBC_ALIAS_SYSTEM */
//End-Libc
size_t	 wcstombs(char * __restrict, const wchar_t * __restrict, size_t);
int	 wctomb(char *, wchar_t);

#ifndef _ANSI_SOURCE
void	_Exit(int) __dead2;
long	 a64l(const char *);
double	 drand48(void);
char	*ecvt(double, int, int *__restrict, int *__restrict); /* LEGACY */
double	 erand48(unsigned short[3]); 
char	*fcvt(double, int, int *__restrict, int *__restrict); /* LEGACY */
char	*gcvt(double, int, char *); /* LEGACY */
int	 getsubopt(char **, char * const *, char **);
int	 grantpt(int);
#if __DARWIN_UNIX03
char	*initstate(unsigned, char *, size_t); /* no  __DARWIN_ALIAS needed */
#else /* !__DARWIN_UNIX03 */
char	*initstate(unsigned long, char *, long);
#endif /* __DARWIN_UNIX03 */
long	 jrand48(unsigned short[3]);
char	*l64a(long);
void	 lcong48(unsigned short[7]);
long	 lrand48(void);
char	*mktemp(char *);
int	 mkstemp(char *);
long	 mrand48(void); 
long	 nrand48(unsigned short[3]);
int	 posix_openpt(int);
char	*ptsname(int);
//Begin-Libc
#ifndef LIBC_ALIAS_PUTENV
//End-Libc
int	 putenv(char *) __DARWIN_ALIAS(putenv);
//Begin-Libc
#else /* LIBC_ALIAS_PUTENV */
int	 putenv(char *) LIBC_ALIAS(putenv);
#endif /* !LIBC_ALIAS_PUTENV */
//End-Libc
long	 random(void);
int	 rand_r(unsigned *);
//Begin-Libc
#ifdef __LIBC__
#ifndef LIBC_ALIAS_REALPATH
char	*realpath(const char * __restrict, char * __restrict) __DARWIN_EXTSN(realpath);
#else /* LIBC_ALIAS_REALPATH */
#ifdef VARIANT_DARWINEXTSN
char	*realpath(const char * __restrict, char * __restrict) LIBC_EXTSN(realpath);
#else /* !VARIANT_DARWINEXTSN */
char	*realpath(const char * __restrict, char * __restrict) LIBC_ALIAS(realpath);
#endif /* VARIANT_DARWINEXTSN */
#endif /* !LIBC_ALIAS_REALPATH */
#else /* !__LIBC__ */
//End-Libc
#if (__DARWIN_UNIX03 && !defined(_POSIX_C_SOURCE)) || defined(_DARWIN_C_SOURCE) || defined(_DARWIN_BETTER_REALPATH)
char	*realpath(const char * __restrict, char * __restrict) __DARWIN_EXTSN(realpath);
#else /* (!__DARWIN_UNIX03 || _POSIX_C_SOURCE) && !_DARWIN_C_SOURCE && !_DARWIN_BETTER_REALPATH */
char	*realpath(const char * __restrict, char * __restrict) __DARWIN_ALIAS(realpath);
#endif /* (__DARWIN_UNIX03 && _POSIX_C_SOURCE) || _DARWIN_C_SOURCE || _DARWIN_BETTER_REALPATH */
//Begin-Libc
#endif /* __LIBC__ */
//End-Libc
unsigned short
	*seed48(unsigned short[3]);
//Begin-Libc
#ifndef LIBC_ALIAS_SETENV
//End-Libc
int	 setenv(const char *, const char *, int) __DARWIN_ALIAS(setenv);
//Begin-Libc
#else /* LIBC_ALIAS_SETENV */
int	 setenv(const char *, const char *, int) LIBC_ALIAS(setenv);
#endif /* !LIBC_ALIAS_SETENV */
//End-Libc
#if __DARWIN_UNIX03
//Begin-Libc
#ifndef LIBC_ALIAS_SETKEY
//End-Libc
void	 setkey(const char *) __DARWIN_ALIAS(setkey);
//Begin-Libc
#else /* LIBC_ALIAS_SETKEY */
void	 setkey(const char *) LIBC_ALIAS(setkey);
#endif /* !LIBC_ALIAS_SETKEY */
//End-Libc
#else /* !__DARWIN_UNIX03 */
int	 setkey(const char *);
#endif /* __DARWIN_UNIX03 */
char	*setstate(const char *);
void	 srand48(long);
#if __DARWIN_UNIX03
void	 srandom(unsigned);
#else /* !__DARWIN_UNIX03 */
void	 srandom(unsigned long);
#endif /* __DARWIN_UNIX03 */
int	 unlockpt(int);
#if __DARWIN_UNIX03
//Begin-Libc
#ifndef LIBC_ALIAS_UNSETENV
//End-Libc
int	 unsetenv(const char *) __DARWIN_ALIAS(unsetenv);
//Begin-Libc
#else /* LIBC_ALIAS_UNSETENV */
int	 unsetenv(const char *) LIBC_ALIAS(unsetenv);
#endif /* !LIBC_ALIAS_UNSETENV */
//End-Libc
#else /* !__DARWIN_UNIX03 */
void	 unsetenv(const char *);
#endif /* __DARWIN_UNIX03 */
#endif	/* !_ANSI_SOURCE */

#if !defined(_ANSI_SOURCE) && (!defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE))
#include <machine/types.h>

#ifndef _DEV_T
typedef	__darwin_dev_t	dev_t;
#define _DEV_T
#endif

#ifndef	_MODE_T
typedef	__darwin_mode_t	mode_t;
#define _MODE_T
#endif

u_int32_t
	 arc4random(void);
void	 arc4random_addrandom(unsigned char * /*dat*/, int /*datlen*/);
void	 arc4random_buf(void * /*buf*/, size_t /*nbytes*/) __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_4_3);
void	 arc4random_stir(void);
u_int32_t
	 arc4random_uniform(u_int32_t /*upper_bound*/) __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_4_3);
#ifdef __BLOCKS__
int	 atexit_b(void (^)(void)) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_3_2);
void	*bsearch_b(const void *, const void *, size_t,
	    size_t, int (^)(const void *, const void *)) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_3_2);
#endif /* __BLOCKS__ */

	 /* getcap(3) functions */
char	*cgetcap(char *, const char *, int);
int	 cgetclose(void);
int	 cgetent(char **, char **, const char *);
int	 cgetfirst(char **, char **);
int	 cgetmatch(const char *, const char *);
int	 cgetnext(char **, char **);
int	 cgetnum(char *, const char *, long *);
int	 cgetset(const char *);
int	 cgetstr(char *, const char *, char **);
int	 cgetustr(char *, const char *, char **);

int	 daemon(int, int) __DARWIN_1050(daemon) __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_0, __MAC_10_5, __IPHONE_2_0, __IPHONE_2_0);
char	*devname(dev_t, mode_t);
char	*devname_r(dev_t, mode_t, char *buf, int len);
char	*getbsize(int *, long *);
int	 getloadavg(double [], int);
const char
	*getprogname(void);

int	 heapsort(void *, size_t, size_t,
	    int (*)(const void *, const void *));
#ifdef __BLOCKS__
int	 heapsort_b(void *, size_t, size_t,
	    int (^)(const void *, const void *)) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_3_2);
#endif /* __BLOCKS__ */
int	 mergesort(void *, size_t, size_t,
	    int (*)(const void *, const void *));
#ifdef __BLOCKS__
int	 mergesort_b(void *, size_t, size_t,
	    int (^)(const void *, const void *)) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_3_2);
#endif /* __BLOCKS__ */
void	 psort(void *, size_t, size_t,
	    int (*)(const void *, const void *)) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_3_2);
#ifdef __BLOCKS__
void	 psort_b(void *, size_t, size_t,
	    int (^)(const void *, const void *)) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_3_2);
#endif /* __BLOCKS__ */
void	 psort_r(void *, size_t, size_t, void *,
	    int (*)(void *, const void *, const void *))  __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_3_2);
#ifdef __BLOCKS__
void	 qsort_b(void *, size_t, size_t,
	    int (^)(const void *, const void *)) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_3_2);
#endif /* __BLOCKS__ */
void	 qsort_r(void *, size_t, size_t, void *,
	    int (*)(void *, const void *, const void *));
int	 radixsort(const unsigned char **, int, const unsigned char *,
	    unsigned);
void	 setprogname(const char *);
int	 sradixsort(const unsigned char **, int, const unsigned char *,
	    unsigned);
void	 sranddev(void);
void	 srandomdev(void);
void	*reallocf(void *, size_t);
#if !__DARWIN_NO_LONG_LONG
long long
	 strtoq(const char *, char **, int);
unsigned long long
	 strtouq(const char *, char **, int);
#endif /* !__DARWIN_NO_LONG_LONG */
extern char *suboptarg;		/* getsubopt(3) external variable */
void	*valloc(size_t);
#endif	/* !_ANSI_SOURCE && !_POSIX_SOURCE */

/* Poison the following routines if -fshort-wchar is set */
#if !defined(__cplusplus) && defined(__WCHAR_MAX__) && __WCHAR_MAX__ <= 0xffffU
#pragma GCC poison mbstowcs mbtowc wcstombs wctomb
#endif
__END_DECLS

#ifdef _USE_EXTENDED_LOCALES_
#include <xlocale/_stdlib.h>
#endif /* _USE_EXTENDED_LOCALES_ */

#endif /* _STDLIB_H_ */
