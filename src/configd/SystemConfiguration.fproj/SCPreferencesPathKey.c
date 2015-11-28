/*
 * Copyright (c) 2001, 2004, 2005, 2010 Apple Inc. All rights reserved.
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
 * October 29, 2001		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#include <SystemConfiguration/SystemConfiguration.h>

#include <stdarg.h>

__private_extern__ CFStringRef
SCPreferencesPathKeyCreate(CFAllocatorRef	allocator,
			   CFStringRef		fmt,
			   ...)
{
	va_list		args;
	CFStringRef	result;

	va_start(args, fmt);
	result = CFStringCreateWithFormatAndArguments(allocator,
						      NULL,
						      fmt,
						      args);
	va_end(args);

	return result;
}


__private_extern__ CFStringRef
SCPreferencesPathKeyCreateNetworkServices(CFAllocatorRef	allocator)
{
	/*
	 * create "/NetworkServices"
	 */
	return CFStringCreateWithFormat(allocator,
					NULL,
					CFSTR("/%@"),
					kSCPrefNetworkServices);
}


__private_extern__ CFStringRef
SCPreferencesPathKeyCreateNetworkServiceEntity(CFAllocatorRef	allocator,
					       CFStringRef	service,
					       CFStringRef	entity)
{
	CFStringRef path;

	if (entity == NULL) {
		/*
		 * create "/NetworkServices/service-id"
		 */
		path = CFStringCreateWithFormat(allocator,
						NULL,
						CFSTR("/%@/%@"),
						kSCPrefNetworkServices,
						service);
	} else {
		/*
		 * create "/NetworkServices/service-id/entity"
		 */
		path = CFStringCreateWithFormat(allocator,
						NULL,
						CFSTR("/%@/%@/%@"),
						kSCPrefNetworkServices,
						service,
						entity);
	}

	return path;
}


__private_extern__ CFStringRef
SCPreferencesPathKeyCreateSets(CFAllocatorRef	allocator)
{
	/*
	 * create "/Sets"
	 */
	return (CFStringCreateWithFormat(allocator,
					 NULL,
					 CFSTR("/%@"),
					 kSCPrefSets));
}


__private_extern__ CFStringRef
SCPreferencesPathKeyCreateSet(CFAllocatorRef	allocator,
			      CFStringRef       set)
{
	/*
	 * create "/Sets/set-id"
	 */
	return (CFStringCreateWithFormat(allocator,
					 NULL,
					 CFSTR("/%@/%@"),
					 kSCPrefSets,
					 set));
}


__private_extern__ CFStringRef
SCPreferencesPathKeyCreateSetNetworkGlobalEntity(CFAllocatorRef	allocator,
					      CFStringRef	set,
					      CFStringRef	entity)
{
	/*
	 * create "/Sets/set-id/Network/Global/entity"
	 */
	return CFStringCreateWithFormat(allocator,
					NULL,
					CFSTR("/%@/%@/%@/%@/%@"),
					kSCPrefSets,
					set,
					kSCCompNetwork,
					kSCCompGlobal,
					entity);
}


__private_extern__ CFStringRef
SCPreferencesPathKeyCreateSetNetworkInterfaceEntity(CFAllocatorRef	allocator,
						 CFStringRef	set,
						 CFStringRef	ifname,
						 CFStringRef	entity)
{
	/*
	 * create "/Sets/set-id/Network/Interface/interface-name/entity"
	 */
	return CFStringCreateWithFormat(allocator,
					NULL,
					CFSTR("/%@/%@/%@/%@/%@/%@"),
					kSCPrefSets,
					set,
					kSCCompNetwork,
					kSCCompInterface,
					ifname,
					entity);
}


__private_extern__ CFStringRef
SCPreferencesPathKeyCreateSetNetworkService(CFAllocatorRef	allocator,
					    CFStringRef		set,
					    CFStringRef		service)
{
	CFStringRef path;

	if (service == NULL) {
		/*
		 * create "/Sets/set-id/Network/Service"
		 */
		path = CFStringCreateWithFormat(allocator,
						NULL,
						CFSTR("/%@/%@/%@/%@"),
						kSCPrefSets,
						set,
						kSCCompNetwork,
						kSCCompService);
	} else {
		/*
		 * create "/Sets/set-id/Network/Service/service-id"
		 */
		path = CFStringCreateWithFormat(allocator,
						NULL,
						CFSTR("/%@/%@/%@/%@/%@"),
						kSCPrefSets,
						set,
						kSCCompNetwork,
						kSCCompService,
						service);
	}

	return path;
}


__private_extern__ CFStringRef
SCPreferencesPathKeyCreateSetNetworkServiceEntity(CFAllocatorRef	allocator,
						  CFStringRef		set,
						  CFStringRef		service,
						  CFStringRef		entity)
{
	CFStringRef path;

	if (entity == NULL) {
		/*
		 * create "/Sets/set-id/Network/Service/service-id"
		 */
		path = CFStringCreateWithFormat(allocator,
						NULL,
						CFSTR("/%@/%@/%@/%@/%@"),
						kSCPrefSets,
						set,
						kSCCompNetwork,
						kSCCompService,
						service);
	} else {
		/*
		 * create "/Sets/set-id/Network/Service/service-id/entity"
		 */
		path = CFStringCreateWithFormat(allocator,
						NULL,
						CFSTR("/%@/%@/%@/%@/%@/%@"),
						kSCPrefSets,
						set,
						kSCCompNetwork,
						kSCCompService,
						service,
						entity);
	}

	return path;
}
