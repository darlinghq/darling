/*
 * Copyright (c) 2010-2012 Apple Inc. All rights reserved.
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
 * Modification History
 *
 * March 1, 2010			Christophe Allie <callie@apple.com>
 * - initial revision
 */

#ifndef _NC_H
#define _NC_H

#include <sys/cdefs.h>
#include <SystemConfiguration/SystemConfiguration.h>

extern CFStringRef		username;
extern CFStringRef		password;
extern CFStringRef		sharedsecret;

__BEGIN_DECLS

int find_nc_cmd(char *cmd);
void do_nc_cmd(char *cmd, int argc, char **argv, Boolean watch);

__END_DECLS

#endif /* !_NC_H */
