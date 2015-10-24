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
 * Mail alias lookup routines
 * Copyright (c) 1989 by NeXT, Inc.
 */

#ifndef _ALIAS_H_
#define _ALIAS_H_

struct aliasent {
	char		*alias_name;
	unsigned	alias_members_len;
	char		**alias_members;
	int			alias_local;
};

#include <sys/cdefs.h>

__BEGIN_DECLS
void alias_setent __P((void));
struct aliasent *alias_getent __P((void));
void alias_endent __P((void));
struct aliasent *alias_getbyname __P((const char *));
__END_DECLS

#endif /* !_ALIAS_H_ */
