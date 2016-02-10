#ifndef _DADISK_H_
#define _DADISK_H_

#include <CoreFoundation/CoreFoundation.h>
#include "DASession.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern const CFStringRef kDADiskDescriptionVolumeKindKey;
extern const CFStringRef kDADiskDescriptionVolumeMountableKey;
extern const CFStringRef kDADiskDescriptionVolumeNameKey;
extern const CFStringRef kDADiskDescriptionVolumeNetworkKey;
extern const CFStringRef kDADiskDescriptionVolumePathKey;
extern const CFStringRef kDADiskDescriptionVolumeUUIDKey;

extern const CFStringRef kDADiskDescriptionMediaBlockSizeKey;
extern const CFStringRef kDADiskDescriptionMediaBSDMajorKey;
extern const CFStringRef kDADiskDescriptionMediaBSDMinorKey;
extern const CFStringRef kDADiskDescriptionMediaBSDNameKey;
extern const CFStringRef kDADiskDescriptionMediaBSDUnitKey;
extern const CFStringRef kDADiskDescriptionMediaContentKey;
extern const CFStringRef kDADiskDescriptionMediaEjectableKey;
extern const CFStringRef kDADiskDescriptionMediaIconKey;
extern const CFStringRef kDADiskDescriptionMediaKindKey;
extern const CFStringRef kDADiskDescriptionMediaLeafKey;
extern const CFStringRef kDADiskDescriptionMediaNameKey;
extern const CFStringRef kDADiskDescriptionMediaPathKey;
extern const CFStringRef kDADiskDescriptionMediaRemovableKey;
extern const CFStringRef kDADiskDescriptionMediaSizeKey;
extern const CFStringRef kDADiskDescriptionMediaTypeKey;
extern const CFStringRef kDADiskDescriptionMediaUUIDKey;
extern const CFStringRef kDADiskDescriptionMediaWholeKey;
extern const CFStringRef kDADiskDescriptionMediaWritableKey;

extern const CFStringRef kDADiskDescriptionDeviceGUIDKey;
extern const CFStringRef kDADiskDescriptionDeviceInternalKey;
extern const CFStringRef kDADiskDescriptionDeviceModelKey;
extern const CFStringRef kDADiskDescriptionDevicePathKey;
extern const CFStringRef kDADiskDescriptionDeviceProtocolKey;
extern const CFStringRef kDADiskDescriptionDeviceRevisionKey;
extern const CFStringRef kDADiskDescriptionDeviceUnitKey;
extern const CFStringRef kDADiskDescriptionDeviceVendorKey;

extern const CFStringRef kDADiskDescriptionBusNameKey;
extern const CFStringRef kDADiskDescriptionBusPathKey;

struct _DADisk;
typedef struct _DADisk* DADiskRef;

typedef int io_service_t; // hack

CFTypeID DADiskGetTypeID(void);

DADiskRef DADiskCreateFromBSDName(CFAllocatorRef allocator, DASessionRef session, const char* name);
DADiskRef DADiskCreateFromIOMedia(CFAllocatorRef allocator, DASessionRef session, io_service_t media);
DADiskRef DADiskCreateFromVolumePath(CFAllocatorRef allocator, DASessionRef session, CFURLRef path);
const char* DADiskGetBSDName(DADiskRef disk);
io_service_t DADiskCopyIOMedia(DADiskRef disk);
CFDictionaryRef DADiskCopyDescription(DADiskRef disk);
DADiskRef DADiskCopyWholeDisk(DADiskRef disk);
	
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
