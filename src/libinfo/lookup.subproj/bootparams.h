/*
 * Copyright (c) 1999-2009 Apple Inc.  All rights reserved.
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
 * Bootparams lookup routines
 *
 * Copyright 1997
 * Apple Computer Inc.
 */

#ifndef _BOOTPARAMS_H_
#define _BOOTPARAMS_H_

#include <sys/param.h>
#include <sys/cdefs.h>

/*
 * Structures returned by bootparams calls.
 */
struct bootparamsent {
	char *bp_name;			/* name of host */
	char **bp_bootparams;	/* bootparams list */
};

__BEGIN_DECLS
void bootparams_endent __P((void));
struct bootparamsent *bootparams_getbyname __P((const char *));
struct bootparamsent *bootparams_getent __P((void));
void bootparams_setent __P((void));
__END_DECLS

#endif /* !_BOOTPARAMS_H_ */
