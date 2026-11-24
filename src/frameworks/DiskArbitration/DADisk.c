#include <DiskArbitration/DiskArbitration.h>
#include <stddef.h>
#include <string.h>
#include <pthread.h>
#include <CoreFoundation/CFRuntime.h>
#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFURL.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CoreFoundation.h>
#include <limits.h>
#include <stdlib.h>

#include "DAPrivate.h"
#include "DAProtocol.h"

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
CONST_STRING_DECL(kDADiskDescriptionAppearanceTimeKey, "DAAppearanceTime");
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
	CFRelease(disk->name);
}

// Disks are equal when they have the same BSD name.
static Boolean
DADiskEqual (CFTypeRef cf1, CFTypeRef cf2)
{
	DADiskRef d1 = (DADiskRef) cf1;
	DADiskRef d2 = (DADiskRef) cf2;
	return CFEqual(d1->name, d2->name);
}

static CFHashCode
DADiskHash (CFTypeRef cf)
{
	return CFHash(((DADiskRef) cf)->name);
}

static DADiskRef DADiskCreateWithName(CFAllocatorRef allocator, DASessionRef session, const char* name)
{
	DADiskRef new;

	if (session == NULL || name == NULL)
		return NULL;
	if (strncmp(name, "/dev/", 5) == 0)
		name += 5;
	if (strlen(name) >= sizeof(new->bsdName))
		return NULL;

	new = (DADiskRef) _CFRuntimeCreateInstance (allocator, _kDADiskTypeID,
												DADISK_SIZE, 0);
	if (new == NULL)
		return NULL;
	strcpy(new->bsdName, name);
	new->name = CFStringCreateWithCString(allocator, name, kCFStringEncodingUTF8);
	new->session = (DASessionRef) CFRetain(session);
	return new;
}

static DADiskRef
DADiskCreateCopy (CFAllocatorRef allocator, DADiskRef disk)
{
	return DADiskCreateWithName(allocator, disk->session, disk->bsdName);
}

CFTypeID DADiskGetTypeID(void)
{
	return _kDADiskTypeID;
}

// Any name gives a disk; one that names no disk has no description.
DADiskRef DADiskCreateFromBSDName(CFAllocatorRef allocator,
		DASessionRef session, const char* name)
{
	return DADiskCreateWithName(allocator, session, name);
}

DADiskRef DADiskCreateFromIOMedia(CFAllocatorRef allocator,
		DASessionRef session, io_service_t media)
{
	return NULL;
}

// Only the exact mount point of a volume gives a disk (a trailing slash does not matter).
DADiskRef DADiskCreateFromVolumePath(CFAllocatorRef allocator,
		DASessionRef session, CFURLRef url)
{
	char path[PATH_MAX], resolved[PATH_MAX], name[64];

	if (session == NULL || url == NULL)
		return NULL;
	if (!CFURLGetFileSystemRepresentation(url, true, (UInt8*) path, sizeof(path)))
		return NULL;
	if (realpath(path, resolved) == NULL)
		return NULL;
	if (!_DAClientLookupPath(session, resolved, name, sizeof(name)))
		return NULL;
	return DADiskCreateWithName(allocator, session, name);
}

const char* DADiskGetBSDName(DADiskRef disk)
{
	return disk->bsdName;
}

io_service_t DADiskCopyIOMedia(DADiskRef disk)
{
	return 0;
}

CFDictionaryRef DADiskCopyDescription(DADiskRef disk)
{
	return _DAClientCopyDescription(disk->session, disk->bsdName);
}

// The whole disk of "diskNsM" is "diskN"; NULL if it does not exist.
DADiskRef DADiskCopyWholeDisk(DADiskRef disk)
{
	char whole[sizeof(disk->bsdName)];
	char* slice;

	strcpy(whole, disk->bsdName);
	slice = strchr(whole + 4, 's');
	if (strncmp(whole, "disk", 4) == 0 && slice != NULL)
		*slice = 0;

	CFDictionaryRef description = _DAClientCopyDescription(disk->session, whole);
	if (description == NULL)
		return NULL;
	CFRelease(description);
	return DADiskCreateWithName(CFGetAllocator(disk), disk->session, whole);
}

typedef void (*DADiskOperationCallback)(DADiskRef disk, DADissenterRef dissenter, void *context);

// QUIRK: for a disk object made from a BSD name that names no disk, the claim, eject and
// unmount operations call their completion callback before returning, with a dissenter of
// status kDAReturnBadArgument and no status string. The documentation only says the callback is
// called once the operation completes. Evidence: black-box observation on macOS 26.7.3 (tier 2;
// the callback ran before the call returned, on the calling thread, whether or not the session
// was scheduled on a run loop or dispatch queue).
// Test: tests/src/diskarbitration_operations.c (quirk_*)
static void DADiskFailOperation(DADiskRef disk, DADiskOperationCallback callback, void *context)
{
	if (!callback)
		return;
	DADissenterRef dissenter = DADissenterCreate(CFGetAllocator(disk), kDAReturnBadArgument, NULL);
	callback(disk, dissenter, context);
	if (dissenter)
		CFRelease(dissenter);
}

static bool DADiskExists(DADiskRef disk)
{
	CFDictionaryRef description = _DAClientCopyDescription(disk->session, disk->bsdName);
	if (description == NULL)
		return false;
	CFRelease(description);
	return true;
}

// diskarbitrationd does not take claims yet, so every claim fails as for a disk that does not exist. A disk object
// counts as claimed only after a successful claim, so never here.
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

void DADiskUnmount(DADiskRef disk, DADiskUnmountOptions options, DADiskUnmountCallback callback, void* context)
{
	if (!DADiskExists(disk))
		DADiskFailOperation(disk, (DADiskOperationCallback) callback, context);
	else
		_DASessionRequest(disk->session, DA_OP_UNMOUNT, disk->bsdName, options, callback, context);
}

void DADiskEject(DADiskRef disk, DADiskEjectOptions options, DADiskEjectCallback callback, void* context)
{
	if (!DADiskExists(disk))
		DADiskFailOperation(disk, (DADiskOperationCallback) callback, context);
	else
		_DASessionRequest(disk->session, DA_OP_EJECT, disk->bsdName, options, callback, context);
}

// A volume that is mounted is busy, as on macOS. Mounting one again is not supported: an image volume needs the
// disk image layer, which DiskArbitration does not use.
void DADiskMountWithArguments(DADiskRef disk, CFURLRef path, DADiskMountOptions options, DADiskMountCallback callback,
	void* context, CFStringRef arguments[])
{
	CFDictionaryRef description = DADiskCopyDescription(disk);
	bool mounted = description != NULL && CFDictionaryContainsKey(description, kDADiskDescriptionVolumePathKey);
	if (description != NULL)
		CFRelease(description);
	_DASessionComplete(disk->session, disk->bsdName, callback, context,
		mounted ? kDAReturnBusy : kDAReturnUnsupported, NULL);
}

void DADiskMount(DADiskRef disk, CFURLRef path, DADiskMountOptions options, DADiskMountCallback callback,
	void* context)
{
	DADiskMountWithArguments(disk, path, options, callback, context, NULL);
}
