/*
 * Copyright (c) 2009 Apple Inc. All rights reserved.
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

#if !defined(__OPENDIRECTORY_CFODSESSION__)
#define __OPENDIRECTORY_CFODSESSION__

#include <CFOpenDirectory/CFOpenDirectory.h>

/*!
    @const      kODSessionDefault
    @abstract   is the default type of ODSessionRef used if there is no need to create a specific reference
    @discussion is the default type of ODSessionRef used if there is no need to create a specific reference
*/
CF_EXPORT
ODSessionRef kODSessionDefault __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

__BEGIN_DECLS

/*!
    @function	ODSessionGetTypeID
    @abstract   Standard GetTypeID function support for CF-based objects
    @discussion Returns the typeID for ODSession objects
    @result     a valid CFTypeID for the ODSession object
*/
CF_EXPORT
CFTypeID ODSessionGetTypeID(void) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODSessionCreate
    @abstract   Creates an ODSession object to be passed to ODNode functions
    @discussion Creates an ODSession object to be passed to ODNode functions.
    @param      allocator a memory allocator to use for this object
    @param      options a CFDictionary of options associated with this ODSession.  This is typically NULL
                unless caller needs to proxy to another host.  
 
                If proxy is required then a dictionary with keys should be:
                            Key                             Value
                    kODSessionProxyAddress        CFString(hostname or IP)
                    kODSessionProxyPort           CFNumber(IP port, should not be set as it will default)
                    kODSessionProxyUsername       CFString(username)
                    kODSessionProxyPassword       CFString(password)
    @param      error an optional CFErrorRef reference for error details
    @result     a valid ODSessionRef object or NULL if it cannot be created. Pass reference to CFErrorRef to
                get error details
*/
CF_EXPORT
ODSessionRef ODSessionCreate(CFAllocatorRef allocator, CFDictionaryRef options, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODSessionCopyNodeNames
    @abstract   Returns the node names that are registered on this ODSession
    @discussion Returns the node names that are registered on this ODSession
    @param      allocator a memory allocator to use for this object
    @param      session an ODSessionRef, either kODSessionDefault or a valid ODSessionRef can be passed
    @param      error an optional CFErrorRef reference for error details
    @result     a valid CFArrayRef of node names that can be opened on the session reference
*/
CF_EXPORT
CFArrayRef ODSessionCopyNodeNames(CFAllocatorRef allocator, ODSessionRef session, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

__END_DECLS

#endif /* !__OPENDIRECTORY_CFODSESSION__ */
