/*
 * Copyright (c) 2012, 2013, 2016, 2017 Apple Inc. All rights reserved.
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

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFRuntime.h>

#ifndef SC_LOG_HANDLE
#define	SC_LOG_HANDLE	__log_SCNetworkConnection
#endif	// SC_LOG_HANDLE
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>
#include <SystemConfiguration/SCValidation.h>


__BEGIN_DECLS

os_log_t	__log_SCNetworkConnection(void);
void		__SCNetworkConnectionForceOnDemandConfigurationRefresh(void);
char *		__SCNetworkConnectionGetControllerPortName(void);

__END_DECLS

#endif	/* _SCNETWORKCONNECTIONINTERNAL_H */
