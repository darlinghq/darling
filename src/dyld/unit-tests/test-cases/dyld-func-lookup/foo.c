/*
 * Copyright (c) 2005-2009 Apple Inc. All rights reserved.
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

#include <stdbool.h>
#include <Availability.h>


// _dyld_func_lookup is only available in 10.5 and earlier
#if defined(__MAC_OS_X_VERSION_MIN_REQUIRED) && (__MAC_OS_X_VERSION_MIN_REQUIRED <= __MAC_10_5)

extern bool _dyld_func_lookup(const char* dyld_func_name, void** address);



bool check_dyld_func_lookup()
{
	void* address;
	
	// dlopen does not exist, something is wrong
	if ( ! _dyld_func_lookup("__dyld_dlopen", &address) )
		return false;
		
	// if a garbage string returns true, something is wrong
	if ( _dyld_func_lookup("blablah", &address) )
		return false;
	
	// looks good
	return true;
}

#endif
