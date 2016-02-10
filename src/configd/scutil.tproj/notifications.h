/*
 * Copyright (c) 2000, 2001, 2004, 2011 Apple Inc. All rights reserved.
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
 * June 1, 2001			Allan Nathanson <ajn@apple.com>
 * - public API conversion
 *
 * November 9, 2000		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#ifndef _NOTIFICATIONS_H
#define _NOTIFICATIONS_H

#include <sys/cdefs.h>

__BEGIN_DECLS

void	storeCallback		(SCDynamicStoreRef	store,
				 CFArrayRef		changedKeys,
				 void			*info);

void	do_notify_list		(int argc, char **argv);
void	do_notify_add		(int argc, char **argv);
void	do_notify_remove	(int argc, char **argv);
void	do_notify_changes	(int argc, char **argv);
void	do_notify_watch		(int argc, char **argv);
void	do_notify_wait		(int argc, char **argv);
void	do_notify_signal	(int argc, char **argv);
void	do_notify_file		(int argc, char **argv);
void	do_notify_cancel	(int argc, char **argv);

__END_DECLS

#endif /* !_NOTIFICATIONS_H */
