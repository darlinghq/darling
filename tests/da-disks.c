// Copyright (C) 2026 Lubos Dolezel
// DiskArbitration disk lookups and descriptions for a mounted disk image, against what macOS 26.7.3 gives (see
// diskarbitration.md):
//  - DADiskCreateFromVolumePath finds a volume by its exact mount point (with or without a trailing slash); a file
//    in it, a path that is not a mount point and a missing path give NULL; "/" has a disk;
//  - the volume's disk is a leaf "diskNs1" whose whole disk is "diskN", with the description keys macOS gives a
//    disk image;
//  - DADiskCreateFromBSDName accepts any name (also with "/dev/"), and the description of an unknown one is NULL;
//  - disks for the same name are equal but distinct objects, and every description is a new dictionary.
//
// Usage: da-disks <image mount point> <its volume name> [<other mount point> <its volume name>]...
//        da-disks --unmount <mount point>        (unmount(2), for the mount point cleanup case)
//        da-disks --name <mount point>            (prints the BSD name of the volume)
// Link with -framework DiskArbitration -framework CoreFoundation. The program also passes on macOS.
#include <CoreFoundation/CoreFoundation.h>
#include <DiskArbitration/DiskArbitration.h>
#include <errno.h>
#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <sys/mount.h>

static int failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL: %s (line %d)\n", #c, __LINE__); failures++; } } while (0)

static DADiskRef disk_for_path(DASessionRef session, const char* path, Boolean directory)
{
	CFURLRef url = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8*) path, strlen(path), directory);
	DADiskRef disk = DADiskCreateFromVolumePath(NULL, session, url);
	CFRelease(url);
	return disk;
}

static int matches(const char* string, const char* pattern)
{
	regex_t re;
	if (regcomp(&re, pattern, REG_EXTENDED | REG_NOSUB) != 0)
		return 0;
	int result = regexec(&re, string, 0, NULL, 0) == 0;
	regfree(&re);
	return result;
}

static int has_string(CFDictionaryRef d, CFStringRef key, const char* value)
{
	CFTypeRef v = CFDictionaryGetValue(d, key);
	if (v == NULL || CFGetTypeID(v) != CFStringGetTypeID())
		return 0;
	CFStringRef expected = CFStringCreateWithCString(NULL, value, kCFStringEncodingUTF8);
	int result = CFEqual(v, expected);
	CFRelease(expected);
	return result;
}

static int has_bool(CFDictionaryRef d, CFStringRef key, Boolean value)
{
	CFTypeRef v = CFDictionaryGetValue(d, key);
	return v != NULL && CFGetTypeID(v) == CFBooleanGetTypeID() && CFBooleanGetValue(v) == value;
}

static long long number(CFDictionaryRef d, CFStringRef key)
{
	CFTypeRef v = CFDictionaryGetValue(d, key);
	long long result = -1;
	if (v != NULL && CFGetTypeID(v) == CFNumberGetTypeID())
		CFNumberGetValue(v, kCFNumberLongLongType, &result);
	return result;
}

static int is_type(CFDictionaryRef d, CFStringRef key, CFTypeID type)
{
	CFTypeRef v = CFDictionaryGetValue(d, key);
	return v != NULL && CFGetTypeID(v) == type;
}

// the keys every disk-image disk has (whole disk and volume alike)
static void check_image_media(CFDictionaryRef d, const char* name, int unit)
{
	CHECK(has_string(d, kDADiskDescriptionDeviceModelKey, "Disk Image"));
	CHECK(has_string(d, kDADiskDescriptionDeviceProtocolKey, "Virtual Interface"));
	CHECK(has_string(d, kDADiskDescriptionDeviceVendorKey, "Apple"));
	CHECK(has_string(d, kDADiskDescriptionMediaBSDNameKey, name));
	CHECK(number(d, kDADiskDescriptionMediaBSDUnitKey) == unit);
	CHECK(number(d, kDADiskDescriptionMediaBlockSizeKey) == 512);
	CHECK(number(d, kDADiskDescriptionMediaSizeKey) > 0);
	CHECK(has_bool(d, kDADiskDescriptionMediaEjectableKey, true));
	CHECK(has_bool(d, kDADiskDescriptionMediaRemovableKey, true));
	CHECK(has_bool(d, kDADiskDescriptionMediaWritableKey, false));
	CHECK(has_string(d, kDADiskDescriptionMediaKindKey, "IOMedia"));
	CHECK(has_bool(d, kDADiskDescriptionVolumeNetworkKey, false));
	CHECK(is_type(d, CFSTR("DAAppearanceTime"), CFNumberGetTypeID()));
}

static void check_volume(DASessionRef session, const char* mp, const char* volume_name)
{
	printf("volume %s\n", mp);
	DADiskRef disk = disk_for_path(session, mp, true);
	CHECK(disk != NULL);
	if (disk == NULL)
		return;

	const char* name = DADiskGetBSDName(disk);
	CHECK(name != NULL && matches(name, "^disk[0-9]+s1$"));
	if (name == NULL)
		return;
	printf("  disk %s\n", name);
	int unit = -1;
	sscanf(name, "disk%d", &unit);

	char with_slash[1024];
	snprintf(with_slash, sizeof(with_slash), "%s/", mp);
	DADiskRef again = disk_for_path(session, with_slash, true);
	CHECK(again != NULL && CFEqual(again, disk) && again != disk);
	if (again)
		CFRelease(again);

	CFDictionaryRef d = DADiskCopyDescription(disk);
	CHECK(d != NULL);
	if (d != NULL) {
		check_image_media(d, name, unit);
		CHECK(has_string(d, kDADiskDescriptionMediaContentKey, "48465300-0000-11AA-AA11-00306543ECAC"));
		CHECK(has_string(d, kDADiskDescriptionMediaNameKey, "disk image"));
		CHECK(has_bool(d, kDADiskDescriptionMediaWholeKey, false));
		CHECK(has_bool(d, kDADiskDescriptionMediaLeafKey, true));
		CHECK(has_string(d, kDADiskDescriptionVolumeKindKey, "hfs"));
		CHECK(has_bool(d, kDADiskDescriptionVolumeMountableKey, true));
		CHECK(has_string(d, kDADiskDescriptionVolumeNameKey, volume_name));
		CHECK(has_string(d, kDADiskDescriptionVolumeTypeKey, "Mac OS Extended"));
		CHECK(is_type(d, kDADiskDescriptionVolumeUUIDKey, CFUUIDGetTypeID()));

		CFURLRef path = CFDictionaryGetValue(d, kDADiskDescriptionVolumePathKey);
		CHECK(path != NULL && CFGetTypeID(path) == CFURLGetTypeID());
		if (path != NULL && CFGetTypeID(path) == CFURLGetTypeID()) {
			CFURLRef expected = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8*) mp, strlen(mp), true);
			CHECK(CFEqual(path, expected));
			CFRelease(expected);
		}

		CFDictionaryRef d2 = DADiskCopyDescription(disk);
		CHECK(d2 != NULL && d2 != d);
		if (d2)
			CFRelease(d2);
		CFRelease(d);
	}

	DADiskRef whole = DADiskCopyWholeDisk(disk);
	CHECK(whole != NULL);
	if (whole != NULL) {
		char whole_name[32];
		snprintf(whole_name, sizeof(whole_name), "disk%d", unit);
		CHECK(DADiskGetBSDName(whole) != NULL && strcmp(DADiskGetBSDName(whole), whole_name) == 0);
		CFDictionaryRef w = DADiskCopyDescription(whole);
		CHECK(w != NULL);
		if (w != NULL) {
			check_image_media(w, whole_name, unit);
			CHECK(has_bool(w, kDADiskDescriptionMediaWholeKey, true));
			CHECK(has_bool(w, kDADiskDescriptionMediaLeafKey, false));
			CHECK(has_bool(w, kDADiskDescriptionVolumeMountableKey, false));
			CHECK(CFDictionaryGetValue(w, kDADiskDescriptionVolumePathKey) == NULL);
			CHECK(CFDictionaryGetValue(w, kDADiskDescriptionVolumeNameKey) == NULL);
			CHECK(CFDictionaryGetValue(w, kDADiskDescriptionVolumeKindKey) == NULL);
			CFRelease(w);
		}
		CFRelease(whole);
	}

	char dev[64];
	snprintf(dev, sizeof(dev), "/dev/%s", name);
	DADiskRef by_dev = DADiskCreateFromBSDName(NULL, session, dev);
	CHECK(by_dev != NULL && DADiskGetBSDName(by_dev) != NULL && strcmp(DADiskGetBSDName(by_dev), name) == 0);
	CHECK(by_dev != NULL && CFEqual(by_dev, disk) && by_dev != disk);
	if (by_dev)
		CFRelease(by_dev);

	char inside[1024];
	snprintf(inside, sizeof(inside), "%s/hello.txt", mp);
	DADiskRef file = disk_for_path(session, inside, false);
	CHECK(file == NULL);
	if (file)
		CFRelease(file);

	CFRelease(disk);
}

int main(int argc, char** argv)
{
	if (argc == 3 && strcmp(argv[1], "--unmount") == 0) {
		if (unmount(argv[2], 0) != 0) {
			perror("unmount");
			return 1;
		}
		return 0;
	}

	DASessionRef session = DASessionCreate(NULL);
	CHECK(session != NULL);
	if (session == NULL)
		return 1;

	if (argc == 3 && strcmp(argv[1], "--name") == 0) {
		DADiskRef disk = disk_for_path(session, argv[2], true);
		if (disk == NULL)
			return 1;
		printf("%s\n", DADiskGetBSDName(disk));
		return 0;
	}

	if (argc < 3 || (argc - 1) % 2 != 0) {
		fprintf(stderr, "usage: %s <mount point> <volume name>...\n", argv[0]);
		return 2;
	}

	for (int i = 1; i + 1 < argc; i += 2)
		check_volume(session, argv[i], argv[i + 1]);

	DADiskRef none = disk_for_path(session, "/nonexistent", true);
	CHECK(none == NULL);
	none = disk_for_path(session, "/tmp", true);
	CHECK(none == NULL);

	DADiskRef root = disk_for_path(session, "/", true);
	CHECK(root != NULL);
	if (root != NULL) {
		CFDictionaryRef r = DADiskCopyDescription(root);
		CHECK(r != NULL && has_bool(r, kDADiskDescriptionMediaLeafKey, true));
		if (r)
			CFRelease(r);
		CFRelease(root);
	}

	DADiskRef unknown = DADiskCreateFromBSDName(NULL, session, "disk99");
	CHECK(unknown != NULL);
	if (unknown != NULL) {
		CHECK(DADiskGetBSDName(unknown) != NULL && strcmp(DADiskGetBSDName(unknown), "disk99") == 0);
		CHECK(DADiskCopyDescription(unknown) == NULL);
		CHECK(DADiskCopyWholeDisk(unknown) == NULL);
		CFRelease(unknown);
	}

	CFRelease(session);
	printf("%s\n", failures ? "FAILED" : "PASSED");
	return failures != 0;
}
