#include <DiskArbitration/DiskArbitration.h>
#include <stddef.h>
#include <string.h>
#include <pthread.h>
#include <CoreFoundation/CFRuntime.h>
#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFURL.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CoreFoundation.h>

#define CONST_STRING_DECL(name, value) const CFStringRef name = CFSTR(value);

CONST_STRING_DECL(kDADiskDescriptionVolumeKindKey, "DAVolumeKind");
CONST_STRING_DECL(kDADiskDescriptionVolumeMountableKey, "DAVolumeMountable");
CONST_STRING_DECL(kDADiskDescriptionVolumeNameKey, "DAVolumeName");
CONST_STRING_DECL(kDADiskDescriptionVolumeNetworkKey, "DAVolumeNetwork");
CONST_STRING_DECL(kDADiskDescriptionVolumePathKey, "DAVolumePath");
CONST_STRING_DECL(kDADiskDescriptionVolumeTypeKey, "DAVolumeType");
CONST_STRING_DECL(kDADiskDescriptionVolumeUUIDKey, "DAVolumeUUID");

CONST_STRING_DECL(kDADiskDescriptionMediaBlockSizeKey, "DAMediaBlockSize");
CONST_STRING_DECL(kDADiskDescriptionMediaBSDMajorKey, "DAMediaBSDMajor");
CONST_STRING_DECL(kDADiskDescriptionMediaBSDMinorKey, "DAMediaBSDMinor");
CONST_STRING_DECL(kDADiskDescriptionMediaBSDNameKey, "DAMediaBSDName");
CONST_STRING_DECL(kDADiskDescriptionMediaBSDUnitKey, "DAMediaBSDUnit");
CONST_STRING_DECL(kDADiskDescriptionMediaContentKey, "DAMediaContent");
CONST_STRING_DECL(kDADiskDescriptionMediaEjectableKey, "DAMediaEjectable");
CONST_STRING_DECL(kDADiskDescriptionMediaIconKey, "DAMediaIcon");
CONST_STRING_DECL(kDADiskDescriptionMediaKindKey, "DAMediaKind");
CONST_STRING_DECL(kDADiskDescriptionMediaLeafKey, "DAMediaLeaf");
CONST_STRING_DECL(kDADiskDescriptionMediaNameKey, "DAMediaName");
CONST_STRING_DECL(kDADiskDescriptionMediaPathKey, "DAMediaPath");
CONST_STRING_DECL(kDADiskDescriptionMediaRemovableKey, "DAMediaRemovable");
CONST_STRING_DECL(kDADiskDescriptionMediaSizeKey, "DAMediaSize");
CONST_STRING_DECL(kDADiskDescriptionMediaTypeKey, "DAMediaType");
CONST_STRING_DECL(kDADiskDescriptionMediaUUIDKey, "DAMediaUUID");
CONST_STRING_DECL(kDADiskDescriptionMediaWholeKey, "DAMediaWhole");
CONST_STRING_DECL(kDADiskDescriptionMediaWritableKey, "DAMediaWritable");
CONST_STRING_DECL(kDADiskDescriptionMediaEncryptedKey, "DAMediaEncrypted");
CONST_STRING_DECL(kDADiskDescriptionMediaEncryptionDetailKey, "DAMediaEncryptionDetail");

CONST_STRING_DECL(kDADiskDescriptionDeviceGUIDKey, "DADeviceGUID");
CONST_STRING_DECL(kDADiskDescriptionDeviceInternalKey, "DADeviceInternal");
CONST_STRING_DECL(kDADiskDescriptionDeviceModelKey, "DADeviceModel");
CONST_STRING_DECL(kDADiskDescriptionDevicePathKey, "DADevicePath");
CONST_STRING_DECL(kDADiskDescriptionDeviceProtocolKey, "DADeviceProtocol");
CONST_STRING_DECL(kDADiskDescriptionDeviceRevisionKey, "DADeviceRevision");                                                                                                                                              
CONST_STRING_DECL(kDADiskDescriptionDeviceUnitKey, "DADeviceUnit");                                                                                                                                                      
CONST_STRING_DECL(kDADiskDescriptionDeviceVendorKey, "DADeviceVendor");                                                                                                                                                  
CONST_STRING_DECL(kDADiskDescriptionDeviceTDMLockedKey, "DADeviceTDMLocked");

CONST_STRING_DECL(kDADiskDescriptionBusNameKey, "DABusName");                                                                                                                                                            
CONST_STRING_DECL(kDADiskDescriptionBusPathKey, "DABusPath");                                                                                                                                                            

CFArrayRef kDADiskDescriptionWatchVolumeName = NULL;
CFArrayRef kDADiskDescriptionWatchVolumePath = NULL;
CFDictionaryRef kDADiskDescriptionMatchMediaUnformatted = NULL;
CFDictionaryRef kDADiskDescriptionMatchMediaWhole = NULL;
CFDictionaryRef kDADiskDescriptionMatchVolumeMountable = NULL;
CFDictionaryRef kDADiskDescriptionMatchVolumeUnrecognized = NULL;

static CFDictionaryRef DADiskCreateMatch(CFStringRef key, CFTypeRef value)
{
	return CFDictionaryCreate(kCFAllocatorDefault, (const void**) &key,
			(const void**) &value, 1, &kCFTypeDictionaryKeyCallBacks,
			&kCFTypeDictionaryValueCallBacks);
}

static void DADiskInitializeMatchObjects(void)
{
	const void* name = kDADiskDescriptionVolumeNameKey;
	const void* path = kDADiskDescriptionVolumePathKey;
	int zero = 0;
	CFNumberRef size = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &zero);

	kDADiskDescriptionWatchVolumeName = CFArrayCreate(kCFAllocatorDefault, &name, 1, &kCFTypeArrayCallBacks);
	kDADiskDescriptionWatchVolumePath = CFArrayCreate(kCFAllocatorDefault, &path, 1, &kCFTypeArrayCallBacks);

	// Contents as observed on macOS 27.0.1: unformatted media match a
	// zero media size, unrecognized volumes a false mountable flag.
	kDADiskDescriptionMatchMediaUnformatted = DADiskCreateMatch(kDADiskDescriptionMediaSizeKey, size);
	kDADiskDescriptionMatchMediaWhole = DADiskCreateMatch(kDADiskDescriptionMediaWholeKey, kCFBooleanTrue);
	kDADiskDescriptionMatchVolumeMountable = DADiskCreateMatch(kDADiskDescriptionVolumeMountableKey, kCFBooleanTrue);
	kDADiskDescriptionMatchVolumeUnrecognized = DADiskCreateMatch(kDADiskDescriptionVolumeMountableKey, kCFBooleanFalse);

	CFRelease(size);
}

// QUIRK: the predefined match and watch objects stay NULL until the
// process creates its first session, and are filled in then. Observed on
// macOS 27.0.1 (tier 2, black-box: each object read before and after
// DASessionCreate()); not documented, but every documented use passes them
// to a DARegister*Callback() function, which needs a session.
// Test: tests/src/diskarbitration_match.c
__attribute__((visibility("hidden")))
void _DADiskInitializeMatchObjects(void)
{
	static pthread_once_t once = PTHREAD_ONCE_INIT;
	pthread_once(&once, DADiskInitializeMatchObjects);
}

struct __DADisk
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

#define DADISK_SIZE sizeof(struct __DADisk) - sizeof(CFRuntimeBase)

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
	return CFStringGetCStringPtr(disk->path,kCFStringEncodingUTF8);
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

// There is no disk arbitration daemon, so no disk exists and every operation on a disk fails.
// QUIRK: for a disk object made from a BSD name that names no disk, the claim, eject and
// unmount operations call their completion callback before returning, with a dissenter of
// status kDAReturnBadArgument and no status string. The documentation only says the callback is
// called once the operation completes. Evidence: black-box observation on macOS 26.7.3 (tier 2;
// the callback ran before the call returned, on the calling thread, whether or not the session
// was scheduled on a run loop or dispatch queue). A disk object counts as claimed only after a
// successful claim, so never here.
// Test: tests/src/diskarbitration_operations.c (quirk_*)
typedef void (*DADiskOperationCallback)(DADiskRef disk, DADissenterRef dissenter, void *context);

static void DADiskFailOperation(DADiskRef disk, DADiskOperationCallback callback, void *context)
{
	if (!callback)
		return;
	DADissenterRef dissenter = DADissenterCreate(CFGetAllocator(disk), kDAReturnBadArgument, NULL);
	callback(disk, dissenter, context);
	if (dissenter)
		CFRelease(dissenter);
}

void DADiskClaim(DADiskRef disk, DADiskClaimOptions options,
		DADiskClaimReleaseCallback release, void *releaseContext,
		DADiskClaimCallback callback, void *callbackContext)
{
	DADiskFailOperation(disk, (DADiskOperationCallback) callback, callbackContext);
}

Boolean DADiskIsClaimed(DADiskRef disk)
{
	return false;
}

void DADiskUnclaim(DADiskRef disk)
{
	// Nothing is claimed, so there is nothing to release
}

void DADiskEject(DADiskRef disk, DADiskEjectOptions options,
		DADiskEjectCallback callback, void *context)
{
	DADiskFailOperation(disk, (DADiskOperationCallback) callback, context);
}

void DADiskUnmount(DADiskRef disk, DADiskUnmountOptions options,
		DADiskUnmountCallback callback, void *context)
{
	DADiskFailOperation(disk, (DADiskOperationCallback) callback, context);
}
