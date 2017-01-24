/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
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
 * Copyright (c) 1988 NeXT, Inc.
 *
 * HISTORY
 *  04-May-90 Created
 */

#ifndef __MONITOR_HEADER__
#define __MONITOR_HEADER__

#include <sys/cdefs.h>
#include <Availability.h>

__BEGIN_DECLS

__OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_0,__MAC_10_11,__IPHONE_2_0,__IPHONE_9_0, "Monitor is no longer supported.")
__WATCHOS_PROHIBITED
extern void monstartup (char *lowpc, char *highpc);

__OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_0,__MAC_10_11,__IPHONE_2_0,__IPHONE_9_0, "Monitor is no longer supported.")
__WATCHOS_PROHIBITED
extern void monitor (char *lowpc, char *highpc, char *buf, int bufsiz, int cntsiz);

__OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_0,__MAC_10_11,__IPHONE_2_0,__IPHONE_9_0, "Monitor is no longer supported.")
__WATCHOS_PROHIBITED
extern void moncontrol (int mode);

__OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_0,__MAC_10_11,__IPHONE_2_0,__IPHONE_9_0, "Monitor is no longer supported.")
__WATCHOS_PROHIBITED
extern void monoutput (const char *filename);

__OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_0,__MAC_10_11,__IPHONE_2_0,__IPHONE_9_0, "Monitor is no longer supported.")
__WATCHOS_PROHIBITED
extern void moninit (void);

__OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_0,__MAC_10_11,__IPHONE_2_0,__IPHONE_9_0, "Monitor is no longer supported.")
__WATCHOS_PROHIBITED
extern void monreset (void);

__END_DECLS

#endif	/* __MONITOR_HEADER__ */
