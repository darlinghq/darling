/*
 * Copyright (c) 2012, 2013 Apple Inc. All rights reserved.
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

#ifndef _SCNETWORKCONNECTIONINTERNAL_H
#define _SCNETWORKCONNECTIONINTERNAL_H

#include <sys/cdefs.h>


__BEGIN_DECLS

void		__SCNetworkConnectionForceOnDemandConfigurationRefresh(void);
char *		__SCNetworkConnectionGetControllerPortName(void);
CFDictionaryRef	__SCNetworkConnectionCopyTokenParameters(SCNetworkConnectionRef connection);

__END_DECLS

#endif	/* _SCNETWORKCONNECTIONINTERNAL_H */
