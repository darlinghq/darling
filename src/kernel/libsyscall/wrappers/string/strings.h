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
 *	@(#)strings.h	8.1 (Berkeley) 6/2/93
 */

#ifndef _STRINGS_H_
#define _STRINGS_H_

#include <_types.h>
#include <sys/_types/_size_t.h>

void    *memmove(void *, const void *, size_t);
void    *memset(void *, int, size_t);
int      strcmp(const char *, const char *);
int      strncmp(const char *, const char *, size_t);
char    *strcpy(char *, const char *);
size_t   strlen(const char *);
size_t   strnlen(const char *, size_t);
size_t   strlcpy(char *, const char *, size_t);
size_t   strlcat(char *, const char *, size_t);
char    *strsep(char **, const char *);

void     bcopy(const void *, void *, size_t);
void     bzero(void *, size_t);
void     __bzero(void *, size_t);
char    *index(const char *, int);
char    *strchr(const char *, int);

#include "string.h"

void    *_libkernel_memmove(void *, const void *, size_t);
void    *_libkernel_memset(void *, int, size_t);
int      _libkernel_strcmp(const char *, const char *);
char    *_libkernel_strcpy(char *, const char *);
size_t   _libkernel_strnlen(const char *, size_t);
size_t   _libkernel_strlen(const char *);
size_t   _libkernel_strlcpy(char *, const char *, size_t);
void     _libkernel_bzero(void *, size_t);
char    *_libkernel_strchr(const char *, int);

#endif  /* _STRINGS_H_ */
