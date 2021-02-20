/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
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

#ifndef _SCNETWORKINTERFACEPROVIDER_H
#define _SCNETWORKINTERFACEPROVIDER_H

/*
 * Modification History
 *
 * January 17, 2018		Dieter Siegmund (dieter@apple.com)
 * - initial revision
 */

/*
 * SCNetworkInterfaceProvider.h
 */


#include <os/availability.h>
#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SCNetworkConfiguration.h>

__BEGIN_DECLS

typedef CF_ENUM(uint32_t, SCNetworkInterfaceProviderEvent) {
	kSCNetworkInterfaceProviderEventActivationRequested = 1,
	kSCNetworkInterfaceProviderEventActivationNoLongerRequested = 2,
};

typedef struct CF_BRIDGED_TYPE(id) __SCNetworkInterfaceProvider *
SCNetworkInterfaceProviderRef;

/*!
	@typedef SCNetworkInterfaceProviderEventHandler
	@discussion Event handler callback to process SCNetworkInterfaceProvider
	events.
	@param event The event to handle.
	@param event_data The event data, always NULL currently.
 */
typedef void
(^SCNetworkInterfaceProviderEventHandler)(SCNetworkInterfaceProviderEvent event,
					  CFDictionaryRef event_data);

/*!
	@function SCNetworkInterfaceProviderCreate
	@discussion  Create an interface provider for a single network
	interface. The interface provider processes the events on the
	interface and takes actions based on the specific event.
	After calling this function, activate the event handler by calling
	SCNetworkInterfaceProviderSetEventHandler() followed by
	SCNetworkInterfaceProviderResume().
	Calling CFRelease() will free resources and deactivate the
	SCNetworkInterfaceProvider callback.
	@param interfaceType The kSCNetworkInterfaceType that the interface
	provider handles e.g. kSCNetworkInterfaceTypeCellular.
	@param interfaceName The name of the network interface, e.g. "pdp_ip0".
	@param options NULL for now.
	@result A non-NULL SCNetworkInterfaceProviderRef if the interface
	provider was successfully registered, NULL otherwise.
 */
SCNetworkInterfaceProviderRef
SCNetworkInterfaceProviderCreate(CFStringRef interfaceType,
				 CFStringRef interfaceName,
				 CFDictionaryRef options)
     API_AVAILABLE(macos(10.14), ios(12.0));

/*!
	@function SCNetworkInterfaceProviderSetEventHandler
	@discussion  Set the event handler to process events for the
	SCNetworkInterfaceProvider object.
	@param provider The SCNetworkInterfaceProvider to set the callback for.
	@param handler The event handler to process events. Invoking this
	function more than once or with a NULL handler is not valid.
 */
void
SCNetworkInterfaceProviderSetEventHandler(SCNetworkInterfaceProviderRef provider,
					  SCNetworkInterfaceProviderEventHandler handler)
     API_AVAILABLE(macos(10.14), ios(12.0));

/*!
	@function SCNetworkInterfaceProviderResume
	@discussion  Activate the interface provider so that its event handler
	will get called.
	@param provider The provider object to enable events on.
 */
void
SCNetworkInterfaceProviderResume(SCNetworkInterfaceProviderRef provider)
     API_AVAILABLE(macos(10.14), ios(12.0));

__END_DECLS
#endif /* _SCNETWORKINTERFACEPROVIDER_H */
