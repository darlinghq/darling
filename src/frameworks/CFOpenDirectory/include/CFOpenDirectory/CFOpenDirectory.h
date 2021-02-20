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

#if !defined(__OPENDIRECTORY_CFOPENDIRECTORY__)
#define __OPENDIRECTORY_CFOPENDIRECTORY__ 1

#include <CoreFoundation/CoreFoundation.h>

/*!
    @typedef    ODContextRef
    @abstract   Opaque reference for the ODContext object
*/
typedef const struct __ODContext *ODContextRef;

/*!
    @typedef    ODNodeRef
    @abstract   Opaque reference for the ODNode object
*/
typedef struct __ODNode *ODNodeRef;

/*!
    @typedef    ODQueryRef
    @abstract   Opaque reference for the ODQuery object
*/
typedef struct __ODQuery *ODQueryRef;

/*!
    @typedef    ODRecordRef
    @abstract   Opaque reference for the ODRecord object
*/
typedef struct __ODRecord *ODRecordRef;

/*!
    @typedef	ODSessionRef
    @abstract   Opaque reference for ODSession object
*/
typedef struct __ODSession *ODSessionRef;

/*!
    @const      kODErrorDomainFramework
    @abstract   the error domain for OpenDirectory.framework details
    @discussion the error domain for OpenDirectory.framework details
*/
CF_EXPORT
const CFStringRef kODErrorDomainFramework __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

#include <CFOpenDirectory/CFOpenDirectoryConstants.h>

#include <CFOpenDirectory/CFODContext.h>
#include <CFOpenDirectory/CFODNode.h>
#include <CFOpenDirectory/CFODQuery.h>
#include <CFOpenDirectory/CFODRecord.h>
#include <CFOpenDirectory/CFODSession.h>

#endif /* ! __OPENDIRECTORY_CFOPENDIRECTORY__ */
