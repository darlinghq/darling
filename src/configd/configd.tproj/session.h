/*
 * Copyright (c) 2000, 2001, 2005-2007, 2009-2012, 2014 Apple Inc. All rights reserved.
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
 * March 24, 2000		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#ifndef _S_SESSION_H
#define _S_SESSION_H

#include <sys/cdefs.h>
#include <Availability.h>
#include <TargetConditionals.h>


/*
 * SCDynamicStore write access entitlement
 *
 *   Key   : "com.apple.SystemConfiguration.SCDynamicStore-write-access"
 *   Value : Boolean
 *             TRUE == allow SCDynamicStore write access for this process
 *
 *           Dictionary
 *             Key   : "keys"
 *             Value : <array> of CFString with write access allowed for
 *                     each SCDynamicStore key matching the string(s)
 *
 *             Key   : "patterns"
 *             Value : <array> of CFString with write access allowed for
 *                     each SCDynamicStore key matching the regex pattern(s)
 */
#define	kSCWriteEntitlementName	CFSTR("com.apple.SystemConfiguration.SCDynamicStore-write-access")


typedef	enum { NO = 0, YES, UNKNOWN } lazyBoolean;


/* Per client server state */
typedef struct {

	/* mach port used as the key to this session */
	mach_port_t		key;

	/* mach port associated with this session */
	CFMachPortRef		serverPort;
	CFRunLoopSourceRef	serverRunLoopSource;

	/* data associated with this "open" session */
	SCDynamicStoreRef	store;

	/* credentials associated with this "open" session */
	uid_t			callerEUID;

	/* Mach security audit trailer for evaluating credentials */
	audit_token_t		auditToken;

	/* root access credential associated with this "open" session */
	lazyBoolean		callerRootAccess;

	/*
	 * write access entitlement associated with this "open" session
	 *
	 *   kCFNull		caller entitlements unknown (need to fetch)
	 *   NULL		no entitlement
	 *   CFBoolean		true/false
	 *   CFDictionary	"keys"     = CFArray[writable keys]
	 *			"patterns" = CFArray[writable patterns]
	 */
	CFTypeRef		callerWriteEntitlement;

} serverSession, *serverSessionRef;

__BEGIN_DECLS

serverSessionRef	getSession	(mach_port_t	server);

serverSessionRef	tempSession	(mach_port_t	server,
					 CFStringRef	name,
					 audit_token_t	auditToken);

serverSessionRef	addSession	(mach_port_t	server,
					 CFStringRef	(*copyDescription)(const void *info));

void			cleanupSession	(mach_port_t	server);

void			listSessions	(FILE		*f);

Boolean			hasRootAccess	(serverSessionRef	session);

Boolean			hasWriteAccess	(serverSessionRef	session,
					 CFStringRef		key);

Boolean			hasPathAccess	(serverSessionRef	session,
					 const char		*path);

__END_DECLS

#endif /* !_S_SESSION_H */
