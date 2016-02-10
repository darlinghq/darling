/*
 * Copyright (c) 2002-2008, 2010-2014 Apple Inc. All rights reserved.
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


#ifndef _DY_FRAMEWORK_H
#define _DY_FRAMEWORK_H

#include <Availability.h>
#include <TargetConditionals.h>
#include <sys/cdefs.h>
#include <mach/mach.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#if	!TARGET_OS_IPHONE
#include <IOKit/pwr_mgt/IOPMLibPrivate.h>
#endif	// !TARGET_OS_IPHONE
#include <Security/Security.h>
#include <Security/SecCertificatePriv.h>
#include <Security/SecItem.h>	// only needed for Mac OS X 10.6[.x]


__BEGIN_DECLS

#pragma mark -
#pragma mark IOKit.framework APIs

CFMutableDictionaryRef
_IOBSDNameMatching			(
					mach_port_t		masterPort,
					uint32_t		options,
					const char		*bsdName
					);
#define IOBSDNameMatching _IOBSDNameMatching

io_object_t
_IOIteratorNext				(
					io_iterator_t		iterator
					);
#define IOIteratorNext _IOIteratorNext

kern_return_t
_IOMasterPort				(
					mach_port_t		bootstrapPort,
					mach_port_t		*masterPort
					);
#define IOMasterPort _IOMasterPort

boolean_t
_IOObjectConformsTo			(
					io_object_t		object,
					const io_name_t		className
					);
#define IOObjectConformsTo _IOObjectConformsTo

boolean_t
_IOObjectGetClass			(
					io_object_t		object,
					io_name_t		className
					);
#define IOObjectGetClass _IOObjectGetClass

kern_return_t
_IOObjectRelease			(
					io_object_t		object
					);
#define IOObjectRelease _IOObjectRelease

#if	!TARGET_OS_IPHONE

IOReturn
_IOPMConnectionAcknowledgeEvent		(
					IOPMConnection			myConnection,
					IOPMConnectionMessageToken	token
					);
#define IOPMConnectionAcknowledgeEvent _IOPMConnectionAcknowledgeEvent

IOReturn
_IOPMConnectionCreate			(
					CFStringRef		myName,
					IOPMCapabilityBits	interests,
					IOPMConnection		*newConnection
					);
#define IOPMConnectionCreate _IOPMConnectionCreate

IOPMCapabilityBits
_IOPMConnectionGetSystemCapabilities	(void);
#define IOPMConnectionGetSystemCapabilities _IOPMConnectionGetSystemCapabilities

IOReturn
_IOPMConnectionRelease			(
					IOPMConnection		myConnection
					);
#define IOPMConnectionRelease _IOPMConnectionRelease

void
_IOPMConnectionSetDispatchQueue		(
					IOPMConnection		myConnection,
					dispatch_queue_t	myQueue
					);
#define IOPMConnectionSetDispatchQueue _IOPMConnectionSetDispatchQueue

IOReturn
_IOPMConnectionSetNotification		(
					IOPMConnection		myConnection,
					void			*param,
					IOPMEventHandlerType	handler
					);
#define IOPMConnectionSetNotification _IOPMConnectionSetNotification

#endif	// !TARGET_OS_IPHONE

CFTypeRef
_IORegistryEntryCreateCFProperty	(
					io_registry_entry_t	entry,
					CFStringRef		key,
					CFAllocatorRef		allocator,
					IOOptionBits		options
					);
#define IORegistryEntryCreateCFProperty _IORegistryEntryCreateCFProperty

kern_return_t
_IORegistryEntryCreateCFProperties	(
					io_registry_entry_t	entry,
					CFMutableDictionaryRef	*properties,
					CFAllocatorRef		allocator,
					IOOptionBits		options
					);
#define IORegistryEntryCreateCFProperties _IORegistryEntryCreateCFProperties

kern_return_t
_IORegistryEntryCreateIterator		(
					io_registry_entry_t	entry,
					const io_name_t		plane,
					IOOptionBits		options,
					io_iterator_t		*iterator
					);
#define IORegistryEntryCreateIterator _IORegistryEntryCreateIterator

kern_return_t
_IORegistryEntryGetLocationInPlane	(
					io_registry_entry_t     entry,
					const io_name_t         plane,
					io_name_t               location
					);
#define	IORegistryEntryGetLocationInPlane _IORegistryEntryGetLocationInPlane

kern_return_t
_IORegistryEntryGetName			(
					io_registry_entry_t	entry,
					io_name_t               name
					);
#define	IORegistryEntryGetName _IORegistryEntryGetName

kern_return_t
_IORegistryEntryGetNameInPlane		(
					io_registry_entry_t     entry,
					const io_name_t         plane,
					io_name_t               name
					);
#define	IORegistryEntryGetNameInPlane _IORegistryEntryGetNameInPlane

kern_return_t
_IORegistryEntryGetParentEntry		(
					io_registry_entry_t	entry,
					const io_name_t		plane,
					io_registry_entry_t	*parent
					);
#define IORegistryEntryGetParentEntry _IORegistryEntryGetParentEntry

kern_return_t
_IORegistryEntryGetPath			(
					io_registry_entry_t	entry,
					const io_name_t		plane,
					io_string_t		path
					);
#define IORegistryEntryGetPath _IORegistryEntryGetPath

kern_return_t
_IORegistryEntryGetRegistryEntryID	(
					io_registry_entry_t	entry,
					uint64_t		*entryID
					);
#define IORegistryEntryGetRegistryEntryID _IORegistryEntryGetRegistryEntryID

CFTypeRef
_IORegistryEntrySearchCFProperty	(
					io_registry_entry_t     entry,
					const io_name_t         plane,
					CFStringRef             key,
					CFAllocatorRef          allocator,
					IOOptionBits            options
					) CF_RETURNS_RETAINED;
#define IORegistryEntrySearchCFProperty _IORegistryEntrySearchCFProperty

kern_return_t
_IOServiceGetMatchingServices		(
					mach_port_t		masterPort,
					CFDictionaryRef		matching,
					io_iterator_t		*existing
					);
#define IOServiceGetMatchingServices _IOServiceGetMatchingServices

CFMutableDictionaryRef
_IOServiceMatching			(
					const char		*name
					);
#define IOServiceMatching _IOServiceMatching

#pragma mark -
#pragma mark Security.framework APIs

#if	!TARGET_OS_IPHONE

CFTypeRef _kSecAttrService();
#define kSecAttrService _kSecAttrService()

CFTypeRef _kSecClass();
#define kSecClass _kSecClass()

CFTypeRef _kSecClassGenericPassword();
#define kSecClassGenericPassword _kSecClassGenericPassword()

CFTypeRef _kSecMatchLimit();
#define kSecMatchLimit _kSecMatchLimit()

CFTypeRef _kSecMatchLimitAll();
#define kSecMatchLimitAll _kSecMatchLimitAll()

CFTypeRef _kSecMatchSearchList();
#define kSecMatchSearchList _kSecMatchSearchList()

CFTypeRef _kSecReturnRef();
#define kSecReturnRef _kSecReturnRef()

CFTypeRef _kSecGuestAttributePid();
#define kSecGuestAttributePid _kSecGuestAttributePid()

CFTypeRef _kSecCodeInfoIdentifier();
#define kSecCodeInfoIdentifier _kSecCodeInfoIdentifier()

CFTypeRef _kSecCodeInfoUnique();
#define kSecCodeInfoUnique _kSecCodeInfoUnique()

OSStatus
_AuthorizationMakeExternalForm		(
					AuthorizationRef		authorization,
					AuthorizationExternalForm	*extForm
					);
#define AuthorizationMakeExternalForm _AuthorizationMakeExternalForm

OSStatus
_SecAccessCreate			(
					CFStringRef			descriptor,
					CFArrayRef			trustedlist,
					SecAccessRef			*accessRef
					);
#define SecAccessCreate _SecAccessCreate

#if	(__MAC_OS_X_VERSION_MIN_REQUIRED < 1070)
OSStatus
_SecAccessCreateFromOwnerAndACL		(
					const CSSM_ACL_OWNER_PROTOTYPE	*owner,
					uint32				aclCount,
					const CSSM_ACL_ENTRY_INFO	*acls,
					SecAccessRef			*accessRef
					);
#define SecAccessCreateFromOwnerAndACL _SecAccessCreateFromOwnerAndACL
#else	// (__MAC_OS_X_VERSION_MIN_REQUIRED < 1070)
SecAccessRef
_SecAccessCreateWithOwnerAndACL		(
					uid_t				userId,
					gid_t				groupId,
					SecAccessOwnerType		ownerType,
					CFArrayRef			acls,
					CFErrorRef			*error
					);
#define SecAccessCreateWithOwnerAndACL _SecAccessCreateWithOwnerAndACL
#endif	// (__MAC_OS_X_VERSION_MIN_REQUIRED < 1070)

OSStatus
_SecItemCopyMatching			(
					CFDictionaryRef			query,
					CFTypeRef			*result
					);
#define SecItemCopyMatching _SecItemCopyMatching

OSStatus
_SecKeychainCopyDomainDefault		(
					SecPreferencesDomain		domain,
					SecKeychainRef			*keychain
					);
#define SecKeychainCopyDomainDefault _SecKeychainCopyDomainDefault

OSStatus
_SecKeychainOpen			(
					const char			*pathName,
					SecKeychainRef			*keychain
					);
#define SecKeychainOpen _SecKeychainOpen

OSStatus
_SecKeychainSetDomainDefault		(
					SecPreferencesDomain		domain,
					SecKeychainRef			keychain
					);
#define SecKeychainSetDomainDefault _SecKeychainSetDomainDefault

OSStatus
_SecKeychainItemCopyContent		(
					SecKeychainItemRef		itemRef,
					SecItemClass			*itemClass,
					SecKeychainAttributeList	*attrList,
					UInt32				*length,
					void				**outData
					);
#define SecKeychainItemCopyContent _SecKeychainItemCopyContent

OSStatus
_SecKeychainItemCreateFromContent	(
					SecItemClass			itemClass,
					SecKeychainAttributeList	*attrList,
					UInt32				length,
					const void			*data,
					SecKeychainRef			keychainRef,
					SecAccessRef			initialAccess,
					SecKeychainItemRef		*itemRef
					);
#define SecKeychainItemCreateFromContent _SecKeychainItemCreateFromContent

OSStatus
_SecKeychainItemDelete			(
					SecKeychainItemRef		itemRef
					);
#define SecKeychainItemDelete _SecKeychainItemDelete

OSStatus
_SecKeychainItemFreeContent		(
					SecKeychainAttributeList	*attrList,
					void				*data
					);
#define SecKeychainItemFreeContent _SecKeychainItemFreeContent

OSStatus
_SecKeychainItemModifyContent		(
					SecKeychainItemRef		itemRef,
					const SecKeychainAttributeList	*attrList,
					UInt32				length,
					const void			*data
					);
#define SecKeychainItemModifyContent _SecKeychainItemModifyContent


OSStatus
_SecTrustedApplicationCreateFromPath	(
					const char			*path,
					SecTrustedApplicationRef	*app
					);
#define SecTrustedApplicationCreateFromPath _SecTrustedApplicationCreateFromPath

#else	// TARGET_OS_IPHONE

CFStringRef _kSecPropertyKeyValue();
#define kSecPropertyKeyValue _kSecPropertyKeyValue()

CFStringRef _kSecPropertyKeyLabel();
#define kSecPropertyKeyLabel _kSecPropertyKeyLabel()

CFArrayRef
_SecCertificateCopyProperties		(
					SecCertificateRef		certRef
					);
#define SecCertificateCopyProperties _SecCertificateCopyProperties

#endif	// TARGET_OS_IPHONE

SecCertificateRef
_SecCertificateCreateWithData		(
					CFAllocatorRef			allocator,
					CFDataRef			data
					);
#define SecCertificateCreateWithData _SecCertificateCreateWithData






__END_DECLS

#endif	// _DY_FRAMEWORK_H

