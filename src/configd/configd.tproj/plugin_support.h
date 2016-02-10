/*
 * Copyright (c) 2000, 2001, 2005, 2006 Apple Computer, Inc. All rights reserved.
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
 * May 26, 2000		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#ifndef _S_PLUGIN_SUPPORT_H
#define _S_PLUGIN_SUPPORT_H

#include <sys/cdefs.h>

extern CFRunLoopRef	plugin_runLoop;

__BEGIN_DECLS

void	plugin_init	(void);
void	plugin_exec	(void		*arg);
Boolean	plugin_term	(int		*status);

__END_DECLS

#endif /* !_S_PLUGIN_SUPPORT_H */
