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

#ifndef _S_NETWORK_INFORMATION_SERVER_H
#define _S_NETWORK_INFORMATION_SERVER_H

#include <sys/cdefs.h>
#include <stdbool.h>
#include <mach/mach.h>
#include <CoreFoundation/CoreFoundation.h>
#include <network_information.h>

typedef void (^_nwi_sync_handler_t)(Boolean inSync);

__BEGIN_DECLS

void
load_NetworkInformation		(CFBundleRef		bundle,
				 SCLoggerRef		logger,
				 Boolean		*bundleVerbose,
				 _nwi_sync_handler_t	syncHandler);

void
_nwi_state_signature		(nwi_state_t		state,
				 unsigned char		*signature,
				 size_t			signature_len);

_Bool
_nwi_state_store		(nwi_state_t		state);

__END_DECLS

#endif	/* !_S_NETWORK_INFORMATION_SERVER_H */
