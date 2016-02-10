/*
 * Copyright (c) 2006, 2008, 2009, 2011, 2012 Apple Inc. All rights reserved.
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

#ifndef _DNS_CONFIGURATION_H
#define _DNS_CONFIGURATION_H

#include <TargetConditionals.h>
#include <sys/cdefs.h>
#include <CoreFoundation/CoreFoundation.h>


#define DNS_CONFIGURATION_SCOPED_QUERY_KEY	CFSTR("__SCOPED_QUERY__")


__BEGIN_DECLS

void	dns_configuration_init		(CFBundleRef		bundle);


#if	!TARGET_OS_IPHONE
void	dns_configuration_monitor	(SCDynamicStoreRef	store,
					 SCDynamicStoreCallBack	callout);
#endif	// !TARGET_OS_IPHONE

Boolean	dns_configuration_set		(CFDictionaryRef	defaultResolver,
					 CFDictionaryRef	services,
					 CFArrayRef		serviceOrder,
					 CFArrayRef		multicastResolvers,
					 CFArrayRef		privateResolvers);

__END_DECLS

#endif /* _DNS_CONFIGURATION_H */

