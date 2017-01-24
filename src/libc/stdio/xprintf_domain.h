/*
 * Copyright (c) 2012 Apple Inc. All rights reserved.
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

#ifndef __XPRINTF_DOMAIN_H_
#define __XPRINTF_DOMAIN_H_

#include <sys/cdefs.h>
#include <printf.h>
#include <pthread.h>

#define	PRINTF_TBL_FIRST	'!'
#define	PRINTF_TBL_LAST		'~'
#define PRINTF_TBL_SIZE		(PRINTF_TBL_LAST - PRINTF_TBL_FIRST + 1)

#define printf_tbl_index(x)	((x) - PRINTF_TBL_FIRST)
#define printf_tbl_in_range(x)	((x) >= PRINTF_TBL_FIRST && (x) <= PRINTF_TBL_LAST)

enum {
    PRINTF_DOMAIN_UNUSED = 0,
    PRINTF_DOMAIN_GLIBC_API,
    PRINTF_DOMAIN_FBSD_API,
    PRINTF_DOMAIN_FLAG,
};
#define printf_domain_fbsd_api(d,x)	((d)->type[x] == PRINTF_DOMAIN_FBSD_API)
#define printf_domain_flag(d,x)		((d)->type[x] == PRINTF_DOMAIN_FLAG)
#define printf_domain_glibc_api(d,x)	((d)->type[x] == PRINTF_DOMAIN_GLIBC_API)
#define printf_domain_unused(d,x)	((d)->type[x] == PRINTF_DOMAIN_UNUSED)

struct _printf_tbl {
    printf_arginfo_function *arginfo;
    void *render; /* either typedef printf_function or printf_render */
    void *context;
};
struct _printf_domain {
    pthread_rwlock_t rwlock;
    char type[PRINTF_TBL_SIZE];
    struct _printf_tbl tbl[PRINTF_TBL_SIZE];
};

__BEGIN_DECLS
__END_DECLS

#endif /* __XPRINTF_DOMAIN_H_ */
