#include "DADisk.h"
#include <stddef.h>
#include <string.h>
#include <CoreFoundation/CFRuntime.h>
#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFURL.h>
#include <CoreFoundation/CFDictionary.h>
#include "../CFF/CFStringConst.h"

CONST_STRING_DECL(kDADiskDescriptionVolumeKindKey, "kDADiskDescriptionVolumeKindKey");
CONST_STRING_DECL(kDADiskDescriptionVolumeMountableKey, "kDADiskDescriptionVolumeMountableKey");
CONST_STRING_DECL(kDADiskDescriptionVolumeNameKey, "kDADiskDescriptionVolumeNameKey");
CONST_STRING_DECL(kDADiskDescriptionVolumeNetworkKey, "kDADiskDescriptionVolumeNetworkKey");
CONST_STRING_DECL(kDADiskDescriptionVolumePathKey, "kDADiskDescriptionVolumePathKey");
CONST_STRING_DECL(kDADiskDescriptionVolumeUUIDKey, "kDADiskDescriptionVolumeUUIDKey");

CONST_STRING_DECL(kDADiskDescriptionMediaBlockSizeKey, "kDADiskDescriptionMediaBlockSizeKey");
CONST_STRING_DECL(kDADiskDescriptionMediaBSDMajorKey, "kDADiskDescriptionMediaBSDMajorKey");
CONST_STRING_DECL(kDADiskDescriptionMediaBSDMinorKey, "kDADiskDescriptionMediaBSDMinorKey");
CONST_STRING_DECL(kDADiskDescriptionMediaBSDNameKey, "kDADiskDescriptionMediaBSDNameKey");
CONST_STRING_DECL(kDADiskDescriptionMediaBSDUnitKey, "kDADiskDescriptionMediaBSDUnitKey");
CONST_STRING_DECL(kDADiskDescriptionMediaContentKey, "kDADiskDescriptionMediaContentKey");
CONST_STRING_DECL(kDADiskDescriptionMediaEjectableKey, "kDADiskDescriptionMediaEjectableKey");
CONST_STRING_DECL(kDADiskDescriptionMediaIconKey, "kDADiskDescriptionMediaIconKey");
CONST_STRING_DECL(kDADiskDescriptionMediaKindKey, "kDADiskDescriptionMediaKindKey");
CONST_STRING_DECL(kDADiskDescriptionMediaLeafKey, "kDADiskDescriptionMediaLeafKey");
CONST_STRING_DECL(kDADiskDescriptionMediaNameKey, "kDADiskDescriptionMediaNameKey");
CONST_STRING_DECL(kDADiskDescriptionMediaPathKey, "kDADiskDescriptionMediaPathKey");
CONST_STRING_DECL(kDADiskDescriptionMediaRemovableKey, "kDADiskDescriptionMediaRemovableKey");
CONST_STRING_DECL(kDADiskDescriptionMediaSizeKey, "kDADiskDescriptionMediaSizeKey");
CONST_STRING_DECL(kDADiskDescriptionMediaTypeKey, "kDADiskDescriptionMediaTypeKey");
CONST_STRING_DECL(kDADiskDescriptionMediaUUIDKey, "kDADiskDescriptionMediaUUIDKey");
CONST_STRING_DECL(kDADiskDescriptionMediaWholeKey, "kDADiskDescriptionMediaWholeKey");
CONST_STRING_DECL(kDADiskDescriptionMediaWritableKey, "kDADiskDescriptionMediaWritableKey");

CONST_STRING_DECL(kDADiskDescriptionDeviceGUIDKey, "kDADiskDescriptionDeviceGUIDKey");
CONST_STRING_DECL(kDADiskDescriptionDeviceInternalKey, "kDADiskDescriptionDeviceInternalKey");
CONST_STRING_DECL(kDADiskDescriptionDeviceModelKey, "kDADiskDescriptionDeviceModelKey");
CONST_STRING_DECL(kDADiskDescriptionDevicePathKey, "kDADiskDescriptionDevicePathKey");
CONST_STRING_DECL(kDADiskDescriptionDeviceProtocolKey, "kDADiskDescriptionDeviceProtocolKey");
CONST_STRING_DECL(kDADiskDescriptionDeviceRevisionKey, "kDADiskDescriptionDeviceRevisionKey");                                                                                                                                              
CONST_STRING_DECL(kDADiskDescriptionDeviceUnitKey, "kDADiskDescriptionDeviceUnitKey");                                                                                                                                                      
CONST_STRING_DECL(kDADiskDescriptionDeviceVendorKey, "kDADiskDescriptionDeviceVendorKey");                                                                                                                                                  
                                                                                                                                                                                                                                            
CONST_STRING_DECL(kDADiskDescriptionBusNameKey, "kDADiskDescriptionBusNameKey");                                                                                                                                                            
CONST_STRING_DECL(kDADiskDescriptionBusPathKey, "kDADiskDescriptionBusPathKey");                                                                                                                                                            

struct _DADisk
{
  CFRuntimeBase _parent;
  DASessionRef session;
  CFStringRef path;
};

static CFTypeID _kDADiskTypeID = 0;
static void DADiskFinalize(CFTypeRef cf);
static Boolean DADiskEqual(CFTypeRef cf1, CFTypeRef cf2);
static CFHashCode DADiskHash(CFTypeRef cf);
static DADiskRef DADiskCreateCopy (CFAllocatorRef allocator, DADiskRef disk);

static CFRuntimeClass DADiskClass = {
  0,
  "DADisk",
  NULL,
  (CFTypeRef (*)(CFAllocatorRef, CFTypeRef)) DADiskCreateCopy,
  DADiskFinalize,
  DADiskEqual,
  DADiskHash,
  NULL,
  NULL
};

__attribute__((constructor))
void
DADiskInitialize (void)
{
	_kDADiskTypeID = _CFRuntimeRegisterClass (&DADiskClass);
}

#define DADISK_SIZE sizeof(struct _DADisk) - sizeof(CFRuntimeBase)

static void
DADiskFinalize (CFTypeRef cf)
{
	DADiskRef disk = (DADiskRef) cf;
	CFRelease(disk->session);
	CFRelease(disk->path);
}

static Boolean
DADiskEqual (CFTypeRef cf1, CFTypeRef cf2)
{
	DADiskRef d1 = (DADiskRef) cf1;
	DADiskRef d2 = (DADiskRef) cf2;
	return CFStringCompare(d1->path, d2->path, 0) == kCFCompareEqualTo;
}

static CFHashCode
DADiskHash (CFTypeRef cf)
{
	return CFHash(((DADiskRef) cf)->path);
}

static DADiskRef
DADiskCreateCopy (CFAllocatorRef allocator, DADiskRef disk)
{
	CFStringRef path;
	DADiskRef new;
	
	new = (DADiskRef) _CFRuntimeCreateInstance (allocator, _kDADiskTypeID,
												DADISK_SIZE, 0);
	new->path = (CFStringRef) CFRetain(disk->path);
	new->session = (DASessionRef) CFRetain(disk->session);
	
	return new;
}

CFTypeID DADiskGetTypeID(void)
{
	return _kDADiskTypeID;
}

DADiskRef DADiskCreateFromBSDName(CFAllocatorRef allocator,
		DASessionRef session, const char* name)
{
	CFStringRef path;
	DADiskRef new;
	
	path = CFStringCreateWithCString(allocator, name, kCFStringEncodingUTF8);
	
	new = (DADiskRef) _CFRuntimeCreateInstance (allocator, _kDADiskTypeID,
												DADISK_SIZE, 0);
	new->path = path;
	new->session = (DASessionRef) CFRetain(session);
	
	return new;
}

DADiskRef DADiskCreateFromIOMedia(CFAllocatorRef allocator,
		DASessionRef session, io_service_t media)
{
	return DADiskCreateFromBSDName(allocator, session, "/dummy");
}

DADiskRef DADiskCreateFromVolumePath(CFAllocatorRef allocator,
		DASessionRef session, CFURLRef url)
{
	CFStringRef path;
	DADiskRef new;
	
	new = (DADiskRef) _CFRuntimeCreateInstance (allocator, _kDADiskTypeID,
												DADISK_SIZE, 0);
	new->path = (CFStringRef) CFRetain(CFURLGetString(url));
	new->session = (DASessionRef) CFRetain(session);
	
	return new;
}

const char* DADiskGetBSDName(DADiskRef disk)
{
	return disk->path;
}

io_service_t DADiskCopyIOMedia(DADiskRef disk)
{
	return 0;
}

CFDictionaryRef DADiskCopyDescription(DADiskRef disk)
{
	return CFDictionaryCreate(NULL, NULL, NULL, 0,
			&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
}

DADiskRef DADiskCopyWholeDisk(DADiskRef disk)
{
	return (DADiskRef) CFRetain(disk);
}
