/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "DAModel.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/attr.h>
#include <sys/file.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <unistd.h>
#include <uuid/uuid.h>

// The description keys (the values of the kDADiskDescription*Key constants).
#define KEY_APPEARANCE_TIME CFSTR("DAAppearanceTime")
#define KEY_DEVICE_INTERNAL CFSTR("DADeviceInternal")
#define KEY_DEVICE_MODEL    CFSTR("DADeviceModel")
#define KEY_DEVICE_PROTOCOL CFSTR("DADeviceProtocol")
#define KEY_DEVICE_VENDOR   CFSTR("DADeviceVendor")
#define KEY_MEDIA_BSD_NAME  CFSTR("DAMediaBSDName")
#define KEY_MEDIA_BSD_UNIT  CFSTR("DAMediaBSDUnit")
#define KEY_MEDIA_BLOCK     CFSTR("DAMediaBlockSize")
#define KEY_MEDIA_CONTENT   CFSTR("DAMediaContent")
#define KEY_MEDIA_EJECTABLE CFSTR("DAMediaEjectable")
#define KEY_MEDIA_KIND      CFSTR("DAMediaKind")
#define KEY_MEDIA_LEAF      CFSTR("DAMediaLeaf")
#define KEY_MEDIA_NAME      CFSTR("DAMediaName")
#define KEY_MEDIA_REMOVABLE CFSTR("DAMediaRemovable")
#define KEY_MEDIA_SIZE      CFSTR("DAMediaSize")
#define KEY_MEDIA_WHOLE     CFSTR("DAMediaWhole")
#define KEY_MEDIA_WRITABLE  CFSTR("DAMediaWritable")
#define KEY_VOLUME_KIND     CFSTR("DAVolumeKind")
#define KEY_VOLUME_MOUNTABLE CFSTR("DAVolumeMountable")
#define KEY_VOLUME_NAME     CFSTR("DAVolumeName")
#define KEY_VOLUME_NETWORK  CFSTR("DAVolumeNetwork")
#define KEY_VOLUME_PATH     CFSTR("DAVolumePath")
#define KEY_VOLUME_TYPE     CFSTR("DAVolumeType")
#define KEY_VOLUME_UUID     CFSTR("DAVolumeUUID")

// The GPT partition type of an HFS+ volume (the leaf's media content macOS reports).
#define HFS_PLUS_PARTITION_TYPE "48465300-0000-11AA-AA11-00306543ECAC"

static DAMedia* g_media;

DAMedia* DAModelFirst(void)
{
	return g_media;
}

void DAModelName(const DAMedia* media, bool whole, char* name, size_t size)
{
	if (whole)
		snprintf(name, size, "disk%d", media->unit);
	else
		snprintf(name, size, "disk%ds1", media->unit);
}

DAMedia* DAModelFindName(const char* name, bool* whole)
{
	if (strncmp(name, "/dev/", 5) == 0)
		name += 5;

	for (DAMedia* media = g_media; media != NULL; media = media->next)
	{
		char candidate[32];
		DAModelName(media, true, candidate, sizeof(candidate));
		if (strcmp(candidate, name) == 0)
		{
			*whole = true;
			return media;
		}
		DAModelName(media, false, candidate, sizeof(candidate));
		if (strcmp(candidate, name) == 0)
		{
			*whole = false;
			return media;
		}
	}
	return NULL;
}

DAMedia* DAModelFindMountPoint(const char* path)
{
	for (DAMedia* media = g_media; media != NULL; media = media->next)
	{
		if (media->mounted && strcmp(media->mountPoint, path) == 0)
			return media;
	}
	return NULL;
}

static void setString(CFMutableDictionaryRef d, CFStringRef key, const char* value)
{
	CFStringRef string = CFStringCreateWithCString(NULL, value, kCFStringEncodingUTF8);
	if (string == NULL)
		return;
	CFDictionarySetValue(d, key, string);
	CFRelease(string);
}

static void setBool(CFMutableDictionaryRef d, CFStringRef key, bool value)
{
	CFDictionarySetValue(d, key, value ? kCFBooleanTrue : kCFBooleanFalse);
}

static void setNumber(CFMutableDictionaryRef d, CFStringRef key, long long value)
{
	CFNumberRef number = CFNumberCreate(NULL, kCFNumberLongLongType, &value);
	CFDictionarySetValue(d, key, number);
	CFRelease(number);
}

// The volume's name and UUID, as getattrlist gives them; false if they cannot be read.
static bool volumeAttributes(const char* mountPoint, char* name, size_t nameSize, char* uuid)
{
	struct attrlist list = {
		.bitmapcount = ATTR_BIT_MAP_COUNT,
		.volattr = ATTR_VOL_INFO | ATTR_VOL_NAME | ATTR_VOL_UUID,
	};
	struct
	{
		uint32_t length;
		attrreference_t name;
		uuid_t uuid;
		char data[1024];
	} __attribute__((packed)) reply;

	if (getattrlist(mountPoint, &list, &reply, sizeof(reply), 0) != 0)
		return false;
	if (reply.name.attr_length == 0)
		return false;

	snprintf(name, nameSize, "%s", (const char*) &reply.name + reply.name.attr_dataoffset);
	uuid_unparse_upper(reply.uuid, uuid);
	return true;
}

// The keys the whole disk and the leaf both have.
static void describeMedia(CFMutableDictionaryRef d, const DAMedia* media, const struct statfs* volume, bool whole)
{
	char name[32];
	DAModelName(media, whole, name, sizeof(name));

	if (media->root)
	{
		setBool(d, KEY_DEVICE_INTERNAL, true);
	}
	else
	{
		setString(d, KEY_DEVICE_MODEL, "Disk Image");
		setString(d, KEY_DEVICE_PROTOCOL, "Virtual Interface");
		setString(d, KEY_DEVICE_VENDOR, "Apple");
	}
	setString(d, KEY_MEDIA_BSD_NAME, name);
	setNumber(d, KEY_MEDIA_BSD_UNIT, media->unit);
	setNumber(d, KEY_MEDIA_BLOCK, 512);
	setNumber(d, KEY_MEDIA_SIZE, (long long) volume->f_blocks * volume->f_bsize);
	setBool(d, KEY_MEDIA_EJECTABLE, !media->root);
	setBool(d, KEY_MEDIA_REMOVABLE, !media->root);
	setBool(d, KEY_MEDIA_WRITABLE, !(volume->f_flags & MNT_RDONLY));
	setString(d, KEY_MEDIA_KIND, "IOMedia");
	setBool(d, KEY_MEDIA_WHOLE, whole);
	setBool(d, KEY_MEDIA_LEAF, !whole);
	setBool(d, KEY_VOLUME_NETWORK, false);
}

static CFMutableDictionaryRef describeWhole(const DAMedia* media, const struct statfs* volume)
{
	CFMutableDictionaryRef d = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks);
	describeMedia(d, media, volume, true);
	setBool(d, KEY_VOLUME_MOUNTABLE, false);
	return d;
}

static CFMutableDictionaryRef describeLeaf(const DAMedia* media, const struct statfs* volume)
{
	CFMutableDictionaryRef d = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks);
	char name[1024], uuid[37];
	bool hfs = strcmp(volume->f_fstypename, "hfs") == 0;

	describeMedia(d, media, volume, false);
	if (!media->root)
		setString(d, KEY_MEDIA_NAME, "disk image");
	if (hfs)
	{
		setString(d, KEY_MEDIA_CONTENT, HFS_PLUS_PARTITION_TYPE);
		setString(d, KEY_VOLUME_TYPE, "Mac OS Extended");
	}
	setString(d, KEY_VOLUME_KIND, volume->f_fstypename);
	setBool(d, KEY_VOLUME_MOUNTABLE, true);
	setString(d, KEY_VOLUME_PATH, media->mountPoint);
	if (volumeAttributes(media->mountPoint, name, sizeof(name), uuid))
	{
		setString(d, KEY_VOLUME_NAME, name);
		setString(d, KEY_VOLUME_UUID, uuid);
	}
	return d;
}

// Replaces a description with a new one, keeping the appearance time; the keys that changed go to *changed
// (NULL if none).
static void updateDescription(CFMutableDictionaryRef* current, CFMutableDictionaryRef fresh, CFMutableArrayRef* changed)
{
	CFTypeRef time = CFDictionaryGetValue(*current, KEY_APPEARANCE_TIME);
	if (time != NULL)
		CFDictionarySetValue(fresh, KEY_APPEARANCE_TIME, time);

	*changed = NULL;
	CFIndex count = CFDictionaryGetCount(*current) + CFDictionaryGetCount(fresh);
	const void** keys = malloc(sizeof(void*) * (count + 1));
	CFIndex oldCount = CFDictionaryGetCount(*current);
	CFDictionaryGetKeysAndValues(*current, keys, NULL);
	CFDictionaryGetKeysAndValues(fresh, keys + oldCount, NULL);

	for (CFIndex i = 0; i < count; i++)
	{
		CFTypeRef before = CFDictionaryGetValue(*current, keys[i]);
		CFTypeRef after = CFDictionaryGetValue(fresh, keys[i]);
		if ((before == NULL) != (after == NULL) || (before != NULL && !CFEqual(before, after)))
		{
			if (*changed == NULL)
				*changed = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
			if (!CFArrayContainsValue(*changed, CFRangeMake(0, CFArrayGetCount(*changed)), keys[i]))
				CFArrayAppendValue(*changed, keys[i]);
		}
	}
	free(keys);

	CFRelease(*current);
	*current = fresh;
}

static void stampAppearance(CFMutableDictionaryRef d)
{
	CFAbsoluteTime now = CFAbsoluteTimeGetCurrent();
	CFNumberRef number = CFNumberCreate(NULL, kCFNumberDoubleType, &now);
	CFDictionarySetValue(d, KEY_APPEARANCE_TIME, number);
	CFRelease(number);
}

static int freeUnit(void)
{
	for (int unit = 1;; unit++)
	{
		bool taken = false;
		for (DAMedia* media = g_media; media != NULL && !taken; media = media->next)
			taken = media->unit == unit;
		if (!taken)
			return unit;
	}
}

static void report(void (*callback)(void*, const char*, CFDictionaryRef), void* context, const DAMedia* media,
	bool whole)
{
	char name[32];
	if (callback == NULL)
		return;
	DAModelName(media, whole, name, sizeof(name));
	callback(context, name, whole ? media->whole : media->leaf);
}

static void reportChange(const DAModelEvents* events, const DAMedia* media, bool whole, CFArrayRef keys)
{
	char name[32];
	if (keys == NULL)
		return;
	if (events != NULL && events->changed != NULL)
	{
		DAModelName(media, whole, name, sizeof(name));
		events->changed(events->context, name, keys, whole ? media->whole : media->leaf);
	}
	CFRelease(keys);
}

static void appear(const struct statfs* volume, bool root, const DAModelEvents* events)
{
	DAMedia* media = calloc(1, sizeof(DAMedia));
	if (media == NULL)
		return;

	static unsigned long lastId;
	media->id = ++lastId;
	media->unit = root ? 0 : freeUnit();
	media->root = root;
	media->mounted = true;
	media->seen = true;
	media->owner = volume->f_owner;
	strlcpy(media->mountPoint, volume->f_mntonname, sizeof(media->mountPoint));
	strlcpy(media->source, volume->f_mntfromname, sizeof(media->source));
	media->whole = describeWhole(media, volume);
	media->leaf = describeLeaf(media, volume);
	stampAppearance(media->whole);
	stampAppearance(media->leaf);

	DAMedia** tail = &g_media;
	while (*tail != NULL)
		tail = &(*tail)->next;
	*tail = media;

	if (events != NULL)
	{
		report(events->appeared, events->context, media, false);
		report(events->appeared, events->context, media, true);
	}
}

static void refresh(DAMedia* media, const struct statfs* volume, const DAModelEvents* events)
{
	CFMutableArrayRef wholeKeys, leafKeys;

	media->owner = volume->f_owner;
	updateDescription(&media->whole, describeWhole(media, volume), &wholeKeys);
	updateDescription(&media->leaf, describeLeaf(media, volume), &leafKeys);
	reportChange(events, media, true, wholeKeys);
	reportChange(events, media, false, leafKeys);
}

// The leaf's volume is no longer mounted: it loses its mount point.
static void volumeGone(DAMedia* media, const DAModelEvents* events)
{
	media->mounted = false;
	if (!CFDictionaryContainsKey(media->leaf, KEY_VOLUME_PATH))
		return;
	CFDictionaryRemoveValue(media->leaf, KEY_VOLUME_PATH);

	CFMutableArrayRef keys = CFArrayCreateMutable(NULL, 1, &kCFTypeArrayCallBacks);
	CFArrayAppendValue(keys, KEY_VOLUME_PATH);
	reportChange(events, media, false, keys);
}

static void destroy(DAMedia* media, const DAModelEvents* events)
{
	DAMedia** link = &g_media;
	while (*link != media)
		link = &(*link)->next;
	*link = media->next;

	if (events != NULL)
	{
		report(events->disappeared, events->context, media, true);
		report(events->disappeared, events->context, media, false);
	}
	CFRelease(media->whole);
	CFRelease(media->leaf);
	free(media);
}

// A disk image's volume: mounted from a file (an absolute path outside /dev) by a file system that names its macOS
// kind, as darling-dmg does through the kernel layer ("hfs"). Another FUSE file system with a file as its source
// reports its Linux type ("fuse" or "fuse.<subtype>") and is not a disk.
static bool isImageVolume(const struct statfs* volume)
{
	return volume->f_mntfromname[0] == '/' && strncmp(volume->f_mntfromname, "/dev/", 5) != 0
		&& strncmp(volume->f_fstypename, "fuse", 4) != 0;
}

void DAModelRescan(const DAModelEvents* events)
{
	int count = getfsstat(NULL, 0, MNT_NOWAIT);
	if (count < 0)
		return;

	// room for mounts made between the two calls
	int capacity = count + 16;
	struct statfs* volumes = calloc(capacity, sizeof(struct statfs));
	if (volumes == NULL)
		return;
	count = getfsstat(volumes, capacity * sizeof(struct statfs), MNT_NOWAIT);
	if (count < 0)
	{
		free(volumes);
		return;
	}

	for (DAMedia* media = g_media; media != NULL; media = media->next)
		media->seen = false;

	for (int i = 0; i < count; i++)
	{
		const struct statfs* volume = &volumes[i];
		bool root = strcmp(volume->f_mntonname, "/") == 0;
		bool image = !root && isImageVolume(volume);
		if (!root && !image)
			continue;

		DAMedia* media = DAModelFindMountPoint(volume->f_mntonname);
		if (media != NULL && strcmp(media->source, volume->f_mntfromname) == 0)
		{
			media->seen = true;
			refresh(media, volume, events);
		}
		else if (media == NULL)
		{
			appear(volume, root, events);
		}
	}

	for (DAMedia* media = g_media, *next; media != NULL; media = next)
	{
		next = media->next;
		if (!media->mounted || media->seen)
			continue;

		char mountPoint[MAXPATHLEN];
		bool root = media->root;
		strlcpy(mountPoint, media->mountPoint, sizeof(mountPoint));
		volumeGone(media, events);
		destroy(media, events);
		if (!root)
			DAModelRemoveMountPoint(mountPoint);
	}

	// A volume mounted where a vanished one was is found by the next pass, now that the old disk is gone.
	for (int i = 0; i < count; i++)
	{
		const struct statfs* volume = &volumes[i];
		bool image = strcmp(volume->f_mntonname, "/") != 0 && isImageVolume(volume);
		if (image && DAModelFindMountPoint(volume->f_mntonname) == NULL)
			appear(volume, false, events);
	}

	free(volumes);
}

void DAModelUnmounted(DAMedia* media, const DAModelEvents* events)
{
	char mountPoint[MAXPATHLEN];
	strlcpy(mountPoint, media->mountPoint, sizeof(mountPoint));
	volumeGone(media, events);
	if (!media->root)
		DAModelRemoveMountPoint(mountPoint);
}

void DAModelRemove(DAMedia* media, const DAModelEvents* events)
{
	destroy(media, events);
}

void DAModelRemoveMountPoint(const char* mountPoint)
{
	static const char volumes[] = "/Volumes/";
	const char* name = mountPoint + sizeof(volumes) - 1;

	if (strncmp(mountPoint, volumes, sizeof(volumes) - 1) != 0 || name[0] == 0 || strchr(name, '/') != NULL)
		return;

	int fd = open("/Volumes", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
	if (fd < 0)
		return;
	while (flock(fd, LOCK_EX) != 0 && errno == EINTR)
		;

	struct stat st;
	struct statfs sfs;
	bool mounted = statfs(mountPoint, &sfs) == 0 && strcmp(sfs.f_mntonname, mountPoint) == 0;
	if (!mounted && lstat(mountPoint, &st) == 0 && S_ISDIR(st.st_mode))
		rmdir(mountPoint); // fails, as it should, unless the directory is empty

	close(fd);
}
