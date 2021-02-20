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

/*!
    @header     CFOpenDirectoryPriv
    @abstract   Private functions and constants
    @discussion Contains private functions and constants.  These functions can
                change or go away at any time as they are not public API.
*/

#ifndef __CFOPENDIRECTORYPRIV_H
#define __CFOPENDIRECTORYPRIV_H

#include <CFOpenDirectory/CFOpenDirectory.h>
#include <CFOpenDirectory/CFOpenDirectoryConstantsPriv.h>
#include <DirectoryService/DirServicesTypes.h>

#include <CFOpenDirectory/CFODTrigger.h>

/* 8239461 */
CF_EXPORT const ODRecordType kODRecordTypeUserAuthenticationData __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

/* 8394219 */
CF_EXPORT const ODAttributeType kODAttributeTypeKerberosACL __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);
CF_EXPORT const ODAttributeType kODAttributeTypeKerberosKeySet __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);
CF_EXPORT const ODAttributeType kODAttributeTypeKerberosPrincipalACL __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);
CF_EXPORT const ODAttributeType kODAttributeTypeKerberosPrincipalAliases __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);
CF_EXPORT const ODAttributeType kODAttributeTypeKerberosPrincipalName __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);
CF_EXPORT const ODAttributeType kODAttributeTypeKerberosTicketPolicy __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

/*!
    @const      kODAuthenticationTypeNodeNativeRetainCredential
    @abstract   The plug-in should determine which specific authentication method to use.
    @discussion Identical to kODAuthenticationTypeNodeNativeClearTextOK, except that it
				retains the authentication for future calls.

				Authentication array has following items in order:
					user name in UTF8 encoding,
					password in UTF8 encoding
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeNodeNativeRetainCredential __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

/*!
    @const      kODAuthenticationTypeNodePPS
    @abstract   PPS authentication.
    @discussion PPS authentication.
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeNodePPS __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

__BEGIN_DECLS

CF_EXPORT bool
ODNodeVerifyCredentialsExtended(ODNodeRef node, ODRecordType recordType, ODAuthenticationType authType, CFArrayRef authItems,
								CFArrayRef *authItemsOut, ODContextRef *context, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

CF_EXPORT bool
ODNodeCopyCredentials(ODNodeRef node, ODRecordType *recordType, CFStringRef *username, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

/*!
    @function   ODNodeCopyRecordAuthenticationData
    @abstract   Private call to copy the UserAuthenticationData record corresponding to a given user record
    @discussion 
    @param      node ldapi:// node containing the needed UserAuthenticationData record
    @param      record the user record
    @param      error error details
    @result     a valid ODSessionRef or NULL if failure occurs and ODGetError() can be called for detailed error
*/
CF_EXPORT
ODRecordRef ODNodeCopyRecordAuthenticationData(ODNodeRef node, ODRecordRef record, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

/*!
	@function	ODSessionNodeNameIsLocal
	@abstract	Private call to determine if a given nodename refers to a "local" node.
	@discussion	Currently returns true only for /BSD/local and nodes beginning with "/Local/"
	@param		session Session from which the node was opened. May be NULL.
	@param		nodename Name of node, e.g. "/Local/Default".
	@result		true if the node name refers to a "local" node, otherwise false.
*/
CF_EXPORT
bool ODSessionNodeNameIsLocal(ODSessionRef session, CFStringRef nodename) __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

/*!
    @function   ODNodeCreateWithDSRef
    @abstract   Private call to create an object from existing DS references
    @discussion Recovery if DS invalidates the references, or something fails outside of the plugins ability cannot
                be recovered for DSProxy tDirReference types.  Other types will re-open a DS Reference and re-open the
                node if the references become invalid.  That does mean the ODNode will "own" the reference at that point.  
                If the flag inCloseOnRelease is set to true, then the last release on ODNodeRef will cause the 
                tDirNodeReference and tDirReference to be closed, otherwise, it's up to the caller to close the references
                accordingly (except after the failure/reopen).
    @param      inAllocator the CFAllocatorRef to use
    @param      inDirRef the tDirReference to use.  An ODSessionRef will be created internally for this reference.
    @param      inNodeRef the existing tDirNodeReference to use.
    @param      inCloseOnRelease a bool true or false signifying whether the ODNodeRef and underlying ODSessionRef should
                close the references after the last release.
    @result     a valid ODNodeRef or NULL if failure occurs and ODGetError() can be called for detailed error
*/
CF_EXPORT
ODNodeRef ODNodeCreateWithDSRef(CFAllocatorRef inAllocator, tDirReference inDirRef, tDirNodeReference inNodeRef, 
								bool inCloseOnRelease) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);


/*!
    @function   ODSessionCreateWithDSRef
    @abstract   Private call to create an object from existing DS references
    @discussion Recovery if DS invalidates the references, or something fails outside of the plugins ability cannot
                be recovered for DSProxy tDirReference types.  If the flag inCloseOnRelease is set to true, then the last release 
                on ODSessionRef will cause the tDirReference to be closed, otherwise, it's up to the caller to close the references
                accordingly (except after the failure/reopen).
    @param      inAllocator the CFAllocatorRef to use
    @param      inDirRef the tDirReference to use.  An ODSessionRef will be created internally for this reference.
    @param      inCloseOnRelease a bool true or false signifying whether the ODNodeRef and underlying ODSessionRef should
                close the references after the last release.
    @result     a valid ODSessionRef or NULL if failure occurs and ODGetError() can be called for detailed error
*/
CF_EXPORT
ODSessionRef ODSessionCreateWithDSRef(CFAllocatorRef inAllocator, tDirReference inDirRef, bool inCloseOnRelease) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODSessionGetDSRef
    @abstract   Returns the internal tDirReference used by the APIs
    @discussion This ref is only guaranteed as long as the ref is usable.  It should not be saved and only used with caution.
                This ref can be closed by the higher-level APIs if ODSessionRefs are deleted, become invalid, etc.
    @param      inSessionRef the ODSessionRef to extract the tDirReference from
    @result     the current tDirReference of the session
*/
CF_EXPORT
tDirReference ODSessionGetDSRef(ODSessionRef inSessionRef) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODNodeGetDSRef
    @abstract   Returns the internal tDirNodeReference used by the APIs
    @discussion This ref is only guaranteed as long as the ref is usable.  It should not be saved and only used with caution.
                This ref can be closed by the higher-level APIs if ODNodeRefs are deleted, become invalid, etc.
    @param      inNodeRef the ODNodeRef to extract the tDirNodeReference from
    @result     the current tDirNodeReference of the node connection
*/
CF_EXPORT
tDirNodeReference ODNodeGetDSRef(ODNodeRef inNodeRef) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function	ODConvertToLegacyErrorCode
    @abstract   Converts error code to legacy error
    @discussion Converts the error code to legacy error code.
    @param      inCode is the error code returned by retrieving error code CFErrorGetCode() or [Error code]
    @result     the legacy style error code
*/
CF_EXPORT
tDirStatus ODConvertToLegacyErrorCode(CFIndex code) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function	ODRecordContainsMemberRefresh
	@abstract   Will use membership APIs to resolve group membership based on Group and Member record combination ignoring the cache
	@discussion Will use membership APIs to resolve group membership based on Group and Member record combination ignoring the cache.
				This API does not check attributes values directly, instead uses system APIs to deal with nested
				memberships.
	@param      inGroupRef an ODRecordRef of the group to be checked for membership
	@param      inMemberRef an ODRecordRef of the member to be checked against the group
	@param      outError an optional CFErrorRef reference for error details
	@result     returns true or false depending on result
*/
CF_EXPORT
bool ODRecordContainsMemberRefresh(ODRecordRef inGroupRef, ODRecordRef inMemberRef, CFErrorRef *outError) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

__END_DECLS

#endif
