/*
 * Copyright (c) 2010 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * "Portions Copyright (c) 2004 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.0 (the 'License').  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License."
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

/*
 * Thread-aware utmpx SPI
 */
#ifndef _UTMPX_THREAD_H_
#define _UTMPX_THREAD_H_

#include <utmpx.h>

struct _utmpx; /* forward reference */
typedef struct _utmpx *utmpx_t;

__BEGIN_DECLS
int		 _closeutx(utmpx_t);
void    	 _endutxent(utmpx_t);
struct utmpx	*_getutxent(utmpx_t);
struct utmpx	*_getutxid(utmpx_t, const struct utmpx *);
struct utmpx	*_getutxline(utmpx_t, const struct utmpx *);
utmpx_t		 _openutx(const char *);
struct utmpx	*_pututxline(utmpx_t, const struct utmpx *);
void    	 _setutxent(utmpx_t);
int		 _utmpxname(utmpx_t, const char *);
__END_DECLS

#endif /* !_UTMPX_THREAD_H_ */
