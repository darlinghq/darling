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

/*
 * Modification History
 *
 * October 31, 2000		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */


#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <dlfcn.h>

#include "dy_framework.h"


#pragma mark -
#pragma mark IOKit.framework APIs

static void *
__loadIOKit(void) {
	static void *image = NULL;
	if (NULL == image) {
		const char	*framework		= "/System/Library/Frameworks/IOKit.framework/IOKit";
		struct stat	statbuf;
		const char	*suffix			= getenv("DYLD_IMAGE_SUFFIX");
		char		path[MAXPATHLEN];

		strlcpy(path, framework, sizeof(path));
		if (suffix) strlcat(path, suffix, sizeof(path));
		if (0 <= stat(path, &statbuf)) {
			image = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
		} else {
			image = dlopen(framework, RTLD_LAZY | RTLD_LOCAL);
		}
	}
	return (void *)image;
}


__private_extern__ CFMutableDictionaryRef
_IOBSDNameMatching(mach_port_t masterPort, uint32_t options, const char *bsdName)
{
	#undef IOBSDNameMatching
	static typeof (IOBSDNameMatching) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadIOKit();
		if (image) dyfunc = dlsym(image, "IOBSDNameMatching");
	}
	return dyfunc ? dyfunc(masterPort, options, bsdName) : NULL;
}


__private_extern__ io_object_t
_IOIteratorNext(io_iterator_t iterator)
{
	#undef IOIteratorNext
	static typeof (IOIteratorNext) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadIOKit();
		if (image) dyfunc = dlsym(image, "IOIteratorNext");
	}
	return dyfunc ? dyfunc(iterator) : 0;
}


__private_extern__ kern_return_t
_IOMasterPort(mach_port_t bootstrapPort, mach_port_t *masterPort)
{
	#undef IOMasterPort
	static typeof (IOMasterPort) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadIOKit();
		if (image) dyfunc = dlsym(image, "IOMasterPort");
	}
	return dyfunc ? dyfunc(bootstrapPort, masterPort) : KERN_FAILURE;
}


__private_extern__ boolean_t
_IOObjectConformsTo(io_object_t object, const io_name_t className)
{
	#undef IOObjectConformsTo
	static typeof (IOObjectConformsTo) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadIOKit();
		if (image) dyfunc = dlsym(image, "IOObjectConformsTo");
	}
	return dyfunc ? dyfunc(object, className) : FALSE;
}


__private_extern__ boolean_t
_IOObjectGetClass(io_object_t object, io_name_t className)
{
	#undef IOObjectGetClass
	static typeof (IOObjectGetClass) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadIOKit();
		if (image) dyfunc = dlsym(image, "IOObjectGetClass");
	}
	return dyfunc ? dyfunc(object, className) : FALSE;
}


__private_extern__ kern_return_t
_IOObjectRelease(io_object_t object)
{
	#undef IOObjectRelease
	static typeof (IOObjectRelease) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadIOKit();
		if (image) dyfunc = dlsym(image, "IOObjectRelease");
	}
	return dyfunc ? dyfunc(object) : KERN_FAILURE;
}


#if	!TARGET_OS_IPHONE

__private_extern__ IOReturn
_IOPMConnectionAcknowledgeEvent(IOPMConnection myConnection, IOPMConnectionMessageToken token)
{
	#undef IOPMConnectionAcknowledgeEvent
	static typeof (IOPMConnectionAcknowledgeEvent) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadIOKit();
		if (image) dyfunc = dlsym(image, "IOPMConnectionAcknowledgeEvent");
		}
	return dyfunc ? dyfunc(myConnection, token) : kIOReturnError;
}


__private_extern__ IOReturn
_IOPMConnectionCreate(CFStringRef myName, IOPMCapabilityBits interests, IOPMConnection *newConnection)
{
	#undef IOPMConnectionCreate
	static typeof (IOPMConnectionCreate) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadIOKit();
		if (image) dyfunc = dlsym(image, "IOPMConnectionCreate");
	}
	return dyfunc ? dyfunc(myName, interests, newConnection) : kIOReturnError;
}


__private_extern__ IOPMCapabilityBits
_IOPMConnectionGetSystemCapabilities(void)
{
	#undef IOPMConnectionGetSystemCapabilities
	static typeof (IOPMConnectionGetSystemCapabilities) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadIOKit();
		if (image) dyfunc = dlsym(image, "IOPMConnectionGetSystemCapabilities");
	}
	return dyfunc ? dyfunc() : kIOPMSleepWakeInterest;
}


__private_extern__ IOReturn
_IOPMConnectionRelease(IOPMConnection myConnection)
{
	#undef IOPMConnectionRelease
	static typeof (IOPMConnectionRelease) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadIOKit();
		if (image) dyfunc = dlsym(image, "IOPMConnectionRelease");
	}
	return dyfunc ? dyfunc(myConnection) : kIOReturnError;
}


__private_extern__ void
_IOPMConnectionSetDispatchQueue(IOPMConnection myConnection, dispatch_queue_t myQueue)
{
	#undef IOPMConnectionSetDispatchQueue
	static typeof (IOPMConnectionSetDispatchQueue) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadIOKit();
		if (image) dyfunc = dlsym(image, "IOPMConnectionSetDispatchQueue");
	}
	if (dyfunc) {
		dyfunc(myConnection, myQueue);
	}
	return;
}


__private_extern__ IOReturn
_IOPMConnectionSetNotification(IOPMConnection myConnection, void *param, IOPMEventHandlerType handler)
{
	#undef IOPMConnectionSetNotification
	static typeof (IOPMConnectionSetNotification) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadIOKit();
		if (image) dyfunc = dlsym(image, "IOPMConnectionSetNotification");
	}
	return dyfunc ? dyfunc(myConnection, param, handler) : kIOReturnError;
}

#endif	// !TARGET_OS_IPHONE


__private_extern__ CFTypeRef
_IORegistryEntryCreateCFProperty(io_registry_entry_t entry, CFStringRef key, CFAllocatorRef allocator, IOOptionBits options)
{
	#undef IORegistryEntryCreateCFProperty
	static typeof (IORegistryEntryCreateCFProperty) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadIOKit();
		if (image) dyfunc = dlsym(image, "IORegistryEntryCreateCFProperty");
	}
	return dyfunc ? dyfunc(entry, key, allocator, options) : NULL;
}


__private_extern__ kern_return_t
_IORegistryEntryCreateCFProperties(io_registry_entry_t entry, CFMutableDictionaryRef *properties, CFAllocatorRef allocator, IOOptionBits options)
{
	#undef IORegistryEntryCreateCFProperties
	static typeof (IORegistryEntryCreateCFProperties) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadIOKit();
		if (image) dyfunc = dlsym(image, "IORegistryEntryCreateCFProperties");
	}
	return dyfunc ? dyfunc(entry, properties, allocator, options) : KERN_FAILURE;
}


__private_extern__ kern_return_t
_IORegistryEntryCreateIterator(mach_port_t masterPort, const io_name_t plane, IOOptionBits options, io_iterator_t *iterator)
{
	#undef IORegistryEntryCreateIterator
	static typeof (IORegistryEntryCreateIterator) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadIOKit();
		if (image) dyfunc = dlsym(image, "IORegistryEntryCreateIterator");
	}
	return dyfunc ? dyfunc(masterPort, plane, options, iterator) : KERN_FAILURE;
}


__private_extern__ kern_return_t
_IORegistryEntryGetLocationInPlane(io_registry_entry_t entry, const io_name_t plane, io_name_t location)
{
#undef IORegistryEntryGetLocationInPlane
	static typeof (IORegistryEntryGetLocationInPlane) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadIOKit();
		if (image) dyfunc = dlsym(image, "IORegistryEntryGetLocationInPlane");
	}
	return dyfunc ? dyfunc(entry, plane, location) : KERN_FAILURE;
}


__private_extern__ kern_return_t
_IORegistryEntryGetName(io_registry_entry_t entry, io_name_t name)
{
	#undef IORegistryEntryGetName
	static typeof (IORegistryEntryGetName) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadIOKit();
		if (image) dyfunc = dlsym(image, "IORegistryEntryGetName");
	}
	return dyfunc ? dyfunc(entry, name) : KERN_FAILURE;
}


__private_extern__ kern_return_t
_IORegistryEntryGetNameInPlane(io_registry_entry_t entry, const io_name_t plane, io_name_t name)
{
	#undef IORegistryEntryGetNameInPlane
	static typeof (IORegistryEntryGetNameInPlane) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadIOKit();
		if (image) dyfunc = dlsym(image, "IORegistryEntryGetNameInPlane");
	}
	return dyfunc ? dyfunc(entry, plane, name) : KERN_FAILURE;
}


__private_extern__ kern_return_t
_IORegistryEntryGetParentEntry(io_registry_entry_t entry, const io_name_t plane, io_registry_entry_t *parent)
{
	#undef IORegistryEntryGetParentEntry
	static typeof (IORegistryEntryGetParentEntry) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadIOKit();
		if (image) dyfunc = dlsym(image, "IORegistryEntryGetParentEntry");
	}
	return dyfunc ? dyfunc(entry, plane, parent) : KERN_FAILURE;
}


__private_extern__ kern_return_t
_IORegistryEntryGetPath(io_registry_entry_t entry, const io_name_t plane, io_string_t path)
{
	#undef IORegistryEntryGetPath
	static typeof (IORegistryEntryGetPath) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadIOKit();
		if (image) dyfunc = dlsym(image, "IORegistryEntryGetPath");
	}
	return dyfunc ? dyfunc(entry, plane, path) : KERN_FAILURE;
}


__private_extern__ kern_return_t
_IORegistryEntryGetRegistryEntryID(io_registry_entry_t entry, uint64_t *entryID)
{
	#undef IORegistryEntryGetRegistryEntryID
	static typeof (IORegistryEntryGetRegistryEntryID) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadIOKit();
		if (image) dyfunc = dlsym(image, "IORegistryEntryGetRegistryEntryID");
	}
	return dyfunc ? dyfunc(entry, entryID) : KERN_FAILURE;
}


__private_extern__ CFTypeRef
_IORegistryEntrySearchCFProperty(io_registry_entry_t entry, const io_name_t plane, CFStringRef key, CFAllocatorRef allocator, IOOptionBits options)
{
	#undef IORegistryEntrySearchCFProperty
	static typeof (IORegistryEntrySearchCFProperty) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadIOKit();
		if (image) dyfunc = dlsym(image, "IORegistryEntrySearchCFProperty");
	}
	return dyfunc ? dyfunc(entry, plane, key, allocator, options) : NULL;
}


__private_extern__ kern_return_t
_IOServiceGetMatchingServices(mach_port_t masterPort, CFDictionaryRef matching, io_iterator_t *existing)
{
	#undef IOServiceGetMatchingServices
	static typeof (IOServiceGetMatchingServices) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadIOKit();
		if (image) dyfunc = dlsym(image, "IOServiceGetMatchingServices");
	}
	return dyfunc ? dyfunc(masterPort, matching, existing) : KERN_FAILURE;
}


__private_extern__ CFMutableDictionaryRef
_IOServiceMatching(const char *name)
{
	#undef IOServiceMatching
	static typeof (IOServiceMatching) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadIOKit();
		if (image) dyfunc = dlsym(image, "IOServiceMatching");
	}
	return dyfunc ? dyfunc(name) : NULL;
}

#pragma mark -
#pragma mark Security.framework APIs

static void *
__loadSecurity(void) {
	static void *image = NULL;
	if (NULL == image) {
		const char	*framework		= "/System/Library/Frameworks/Security.framework/Security";
		struct stat	statbuf;
		const char	*suffix			= getenv("DYLD_IMAGE_SUFFIX");
		char		path[MAXPATHLEN];

		strlcpy(path, framework, sizeof(path));
		if (suffix) strlcat(path, suffix, sizeof(path));
		if (0 <= stat(path, &statbuf)) {
			image = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
		} else {
			image = dlopen(framework, RTLD_LAZY | RTLD_LOCAL);
		}
	}
	return (void *)image;
}

#define	SECURITY_FRAMEWORK_EXTERN(t, s)				\
	__private_extern__ t					\
	_ ## s()						\
	{							\
		static t	*dysym = NULL;			\
		if (!dysym) {					\
			void *image = __loadSecurity();		\
			if (image) dysym = dlsym(image, #s );	\
		}						\
		return (dysym != NULL) ? *dysym : NULL;		\
	}

#if	!TARGET_OS_IPHONE
SECURITY_FRAMEWORK_EXTERN(CFTypeRef, kSecAttrService)
SECURITY_FRAMEWORK_EXTERN(CFTypeRef, kSecClass)
SECURITY_FRAMEWORK_EXTERN(CFTypeRef, kSecClassGenericPassword)
SECURITY_FRAMEWORK_EXTERN(CFTypeRef, kSecMatchLimit)
SECURITY_FRAMEWORK_EXTERN(CFTypeRef, kSecMatchLimitAll)
SECURITY_FRAMEWORK_EXTERN(CFTypeRef, kSecMatchSearchList)
SECURITY_FRAMEWORK_EXTERN(CFTypeRef, kSecReturnRef)
SECURITY_FRAMEWORK_EXTERN(CFTypeRef, kSecGuestAttributePid)
SECURITY_FRAMEWORK_EXTERN(CFTypeRef, kSecCodeInfoIdentifier)
SECURITY_FRAMEWORK_EXTERN(CFTypeRef, kSecCodeInfoUnique)

__private_extern__ OSStatus
_AuthorizationMakeExternalForm(AuthorizationRef authorization, AuthorizationExternalForm *extForm)
{
	#undef AuthorizationMakeExternalForm
	static typeof (AuthorizationMakeExternalForm) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadSecurity();
		if (image) dyfunc = dlsym(image, "AuthorizationMakeExternalForm");
	}
	return dyfunc ? dyfunc(authorization, extForm) : -1;
}

__private_extern__ OSStatus
_SecAccessCreate(CFStringRef descriptor, CFArrayRef trustedlist, SecAccessRef *accessRef)
{
	#undef SecAccessCreate
	static typeof (SecAccessCreate) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadSecurity();
		if (image) dyfunc = dlsym(image, "SecAccessCreate");
	}
	return dyfunc ? dyfunc(descriptor, trustedlist, accessRef) : -1;
}

#if	(__MAC_OS_X_VERSION_MIN_REQUIRED < 1070)
__private_extern__ OSStatus
_SecAccessCreateFromOwnerAndACL(const CSSM_ACL_OWNER_PROTOTYPE *owner, uint32 aclCount, const CSSM_ACL_ENTRY_INFO *acls, SecAccessRef *accessRef)
{
	#undef SecAccessCreateFromOwnerAndACL
	static typeof (SecAccessCreateFromOwnerAndACL) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadSecurity();
		if (image) dyfunc = dlsym(image, "SecAccessCreateFromOwnerAndACL");
	}
	return dyfunc ? dyfunc(owner, aclCount, acls, accessRef) : -1;
}
#else	// (__MAC_OS_X_VERSION_MIN_REQUIRED < 1070)
__private_extern__ SecAccessRef
_SecAccessCreateWithOwnerAndACL(uid_t userId, gid_t groupId, SecAccessOwnerType ownerType, CFArrayRef acls, CFErrorRef *error)
{
#undef SecAccessCreateWithOwnerAndACL
	static typeof (SecAccessCreateWithOwnerAndACL) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadSecurity();
		if (image) dyfunc = dlsym(image, "SecAccessCreateWithOwnerAndACL");
	}
	return dyfunc ? dyfunc(userId, groupId, ownerType, acls, error) : NULL;
}
#endif	// (__MAC_OS_X_VERSION_MIN_REQUIRED < 1070)

__private_extern__ OSStatus
_SecItemCopyMatching(CFDictionaryRef query, CFTypeRef *result)
{
#undef SecItemCopyMatching
	static typeof (SecItemCopyMatching) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadSecurity();
		if (image) dyfunc = dlsym(image, "SecItemCopyMatching");
	}
	return dyfunc ? dyfunc(query, result) : -1;
}

__private_extern__ OSStatus
_SecKeychainCopyDomainDefault(SecPreferencesDomain domain, SecKeychainRef *keychain)
{
	#undef SecKeychainCopyDomainDefault
	static typeof (SecKeychainCopyDomainDefault) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadSecurity();
		if (image) dyfunc = dlsym(image, "SecKeychainCopyDomainDefault");
	}
	return dyfunc ? dyfunc(domain, keychain) : -1;
}

__private_extern__ OSStatus
_SecKeychainOpen(const char *pathName, SecKeychainRef *keychain)
{
	#undef SecKeychainOpen
	static typeof (SecKeychainOpen) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadSecurity();
		if (image) dyfunc = dlsym(image, "SecKeychainOpen");
	}
	return dyfunc ? dyfunc(pathName, keychain) : -1;
}

__private_extern__ OSStatus
_SecKeychainSetDomainDefault(SecPreferencesDomain domain, SecKeychainRef keychain)
{
	#undef SecKeychainSetDomainDefault
	static typeof (SecKeychainSetDomainDefault) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadSecurity();
		if (image) dyfunc = dlsym(image, "SecKeychainSetDomainDefault");
	}
	return dyfunc ? dyfunc(domain, keychain) : -1;
}

__private_extern__ OSStatus
_SecKeychainItemCopyContent(SecKeychainItemRef itemRef, SecItemClass *itemClass, SecKeychainAttributeList *attrList, UInt32 *length, void **outData)
{
	#undef SecKeychainItemCopyContent
	static typeof (SecKeychainItemCopyContent) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadSecurity();
		if (image) dyfunc = dlsym(image, "SecKeychainItemCopyContent");
	}
	return dyfunc ? dyfunc(itemRef, itemClass, attrList, length, outData) : -1;
}

__private_extern__ OSStatus
_SecKeychainItemCreateFromContent(SecItemClass itemClass, SecKeychainAttributeList *attrList, UInt32 length, const void *data, SecKeychainRef keychainRef, SecAccessRef initialAccess, SecKeychainItemRef *itemRef)
{
	#undef SecKeychainItemCreateFromContent
	static typeof (SecKeychainItemCreateFromContent) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadSecurity();
		if (image) dyfunc = dlsym(image, "SecKeychainItemCreateFromContent");
	}
	return dyfunc ? dyfunc(itemClass, attrList, length, data, keychainRef, initialAccess, itemRef) : -1;
}

__private_extern__ OSStatus
_SecKeychainItemDelete(SecKeychainItemRef itemRef)
{
	#undef SecKeychainItemDelete
	static typeof (SecKeychainItemDelete) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadSecurity();
		if (image) dyfunc = dlsym(image, "SecKeychainItemDelete");
	}
	return dyfunc ? dyfunc(itemRef) : -1;
}

__private_extern__ OSStatus
_SecKeychainItemFreeContent(SecKeychainAttributeList *attrList, void *data)
{
	#undef SecKeychainItemFreeContent
	static typeof (SecKeychainItemFreeContent) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadSecurity();
		if (image) dyfunc = dlsym(image, "SecKeychainItemFreeContent");
	}
	return dyfunc ? dyfunc(attrList, data) : -1;
}

__private_extern__ OSStatus
_SecKeychainItemModifyContent(SecKeychainItemRef itemRef, const SecKeychainAttributeList *attrList, UInt32 length, const void *data)
{
	#undef SecKeychainItemModifyContent
	static typeof (SecKeychainItemModifyContent) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadSecurity();
		if (image) dyfunc = dlsym(image, "SecKeychainItemModifyContent");
	}
	return dyfunc ? dyfunc(itemRef, attrList, length, data) : -1;
}


__private_extern__ OSStatus
_SecTrustedApplicationCreateFromPath(const char *path, SecTrustedApplicationRef *app)
{
	#undef SecTrustedApplicationCreateFromPath
	static typeof (SecTrustedApplicationCreateFromPath) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadSecurity();
		if (image) dyfunc = dlsym(image, "SecTrustedApplicationCreateFromPath");
	}
	return dyfunc ? dyfunc(path, app) : -1;
}

#else	// TARGET_OS_IPHONE

SECURITY_FRAMEWORK_EXTERN(CFStringRef, kSecPropertyKeyValue)
SECURITY_FRAMEWORK_EXTERN(CFStringRef, kSecPropertyKeyLabel)

__private_extern__ CFArrayRef
_SecCertificateCopyProperties(SecCertificateRef certRef)
{
	#undef SecCertificateCopyProperties
	static typeof (SecCertificateCopyProperties) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadSecurity();
		if (image) dyfunc = dlsym(image, "SecCertificateCopyProperties");
	}
	return dyfunc ? dyfunc(certRef) : NULL;
}

#endif	// TARGET_OS_IPHONE

__private_extern__ SecCertificateRef
_SecCertificateCreateWithData(CFAllocatorRef allocator, CFDataRef data)
{
	#undef SecCertificateCreateWithData
	static typeof (SecCertificateCreateWithData) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadSecurity();
		if (image) dyfunc = dlsym(image, "SecCertificateCreateWithData");
	}
	return dyfunc ? dyfunc(allocator, data) : NULL;
}





