/*
 * Copyright (c) 2000, 2007, 2010 Apple Inc. All rights reserved.
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
 *	@(#)string.h	8.1 (Berkeley) 6/2/93
 */

#ifndef _STRING_H_
#define	_STRING_H_

#include <_types.h>

#include <sys/cdefs.h>
#include <Availability.h>

#ifndef	_SIZE_T
#define	_SIZE_T
typedef	__darwin_size_t		size_t;
#endif

#ifndef NULL
#define NULL __DARWIN_NULL
#endif /* ! NULL */



/* ANSI-C */

__BEGIN_DECLS
void	*memchr(const void *, int, size_t);
int	 memcmp(const void *, const void *, size_t);
void	*memcpy(void *, const void *, size_t);
void	*memmove(void *, const void *, size_t);
void	*memset(void *, int, size_t);
char	*strcat(char *, const char *);
char	*strchr(const char *, int);
int	 strcmp(const char *, const char *);
int	 strcoll(const char *, const char *);
char	*strcpy(char *, const char *);
size_t	 strcspn(const char *, const char *);
//Begin-Libc
#ifndef LIBC_ALIAS_STRERROR
//End-Libc
char	*strerror(int) __DARWIN_ALIAS(strerror);
//Begin-Libc
#else /* LIBC_ALIAS_STRERROR */
char	*strerror(int) LIBC_ALIAS(strerror);
#endif /* !LIBC_ALIAS_STRERROR */
//End-Libc
size_t	 strlen(const char *);
char	*strncat(char *, const char *, size_t);
int	 strncmp(const char *, const char *, size_t);
char	*strncpy(char *, const char *, size_t);
char	*strpbrk(const char *, const char *);
char	*strrchr(const char *, int);
size_t	 strspn(const char *, const char *);
char	*strstr(const char *, const char *);
char	*strtok(char *, const char *);
size_t	 strxfrm(char *, const char *, size_t);
__END_DECLS



/* Additional functionality provided by:
 * POSIX.1c-1995,
 * POSIX.1i-1995,
 * and the omnibus ISO/IEC 9945-1: 1996
 */

#if __DARWIN_C_LEVEL >= 199506L
__BEGIN_DECLS
char	*strtok_r(char *, const char *, char **);
__END_DECLS
#endif /* __DARWIN_C_LEVEL >= 199506L */



/* Additional functionality provided by:
 * POSIX.1-2001
 */

#if __DARWIN_C_LEVEL >= 200112L
__BEGIN_DECLS
int	 strerror_r(int, char *, size_t);
char	*strdup(const char *);
void	*memccpy(void *, const void *, int, size_t);
__END_DECLS
#endif /* __DARWIN_C_LEVEL >= 200112L */



/* Additional functionality provided by:
 * POSIX.1-2008
 */

#if __DARWIN_C_LEVEL >= 200809L
__BEGIN_DECLS
char	*stpcpy(char *, const char *);
char    *stpncpy(char *, const char *, size_t) __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_4_3);
char	*strndup(const char *, size_t) __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_4_3);
size_t   strnlen(const char *, size_t) __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_4_3);
char	*strsignal(int sig);
__END_DECLS
#endif /* __DARWIN_C_LEVEL >= 200809L */



/* Darwin extensions */

#if __DARWIN_C_LEVEL >= __DARWIN_C_FULL
#ifndef _SSIZE_T
#define _SSIZE_T
typedef __darwin_ssize_t	ssize_t;
#endif

__BEGIN_DECLS
void	*memmem(const void *, size_t, const void *, size_t) __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_4_3);
void     memset_pattern4(void *, const void *, size_t) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_3_0);
void     memset_pattern8(void *, const void *, size_t) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_3_0);
void     memset_pattern16(void *, const void *, size_t) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_3_0);

char	*strcasestr(const char *, const char *);
char	*strnstr(const char *, const char *, size_t);
size_t	 strlcat(char *, const char *, size_t);
size_t	 strlcpy(char *, const char *, size_t);
void	 strmode(int, char *);
char	*strsep(char **, const char *);

/* SUS places swab() in unistd.h.  It is listed here for source compatibility */
void	 swab(const void * __restrict, void * __restrict, ssize_t);
__END_DECLS

/* Some functions historically defined in string.h were placed in strings.h
 * by SUS.  We are using "strings.h" instead of <strings.h> to avoid an issue
 * where /Developer/Headers/FlatCarbon/Strings.h could be included instead on
 * case-insensitive file systems.
 */
#include "strings.h"
#endif /* __DARWIN_C_LEVEL >= __DARWIN_C_FULL */


#ifdef _USE_EXTENDED_LOCALES_
#include <xlocale/_string.h>
#endif /* _USE_EXTENDED_LOCALES_ */

#if defined (__GNUC__) && _FORTIFY_SOURCE > 0 && !defined (__cplusplus)
/* Security checking functions.  */
#include <secure/_string.h>
#endif

#endif /* _STRING_H_ */
