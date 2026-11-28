/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * The volume attributes of getattrlist() and fgetattrlist() that describe the mount that holds an object:
 * ATTR_VOL_FSTYPE, MOUNTPOINT, MOUNTFLAGS, MOUNTEDDEVICE, MOUNTEXTFLAGS, FSTYPENAME, FSSUBTYPE and OWNER (the
 * attributes statfs_ext() asks for, and MOUNTPOINT|UUID|FSTYPENAME as installers ask for them together).
 *  - They are the fields of statfs() for the same path: the type, the mount point, the mount flags, the mounted
 *    device, the file system's name, its subtype and the owner, and the extended mount flags of struct statfs.
 *  - The result has the layout of macOS 26.7.3: a length, then (with ATTR_CMN_RETURNED_ATTRS) the returned attributes,
 *    of which the volume ones are the requested ones without ATTR_VOL_INFO, then the fixed-size values in the order of
 *    the bits, a string as a reference (offset from the reference itself, length with the NUL) to text that follows
 *    the fixed part, padded to 4 bytes. The length field counts what was written.
 *  - Any object names its volume (a file, a directory, a descriptor), the answer is the same for all, a request
 *    without ATTR_CMN_RETURNED_ATTRS gives the same values, and the attributes can be asked for together with
 *    ATTR_VOL_UUID and ATTR_VOL_NAME.
 *  - A bit outside ATTR_VOL_VALIDMASK (0xF0FFFFFF) is EINVAL.
 *  - On macOS 10.14.4 these attributes are EINVAL; the program is meant for macOS 11 and later, as Darling is.
 */
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/attr.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef ATTR_VOL_MOUNTEXTFLAGS
#define ATTR_VOL_MOUNTEXTFLAGS 0x00080000
#define ATTR_VOL_FSTYPENAME 0x00100000
#define ATTR_VOL_FSSUBTYPE 0x00200000
#define ATTR_VOL_OWNER 0x00400000
#endif

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s (errno %d)\n", __LINE__, #cond, errno); failures++; } } while (0)

#define ALL_STATFS_ATTRS (ATTR_VOL_FSTYPE | ATTR_VOL_MOUNTPOINT | ATTR_VOL_MOUNTFLAGS | ATTR_VOL_MOUNTEDDEVICE | \
	ATTR_VOL_MOUNTEXTFLAGS | ATTR_VOL_FSTYPENAME | ATTR_VOL_FSSUBTYPE | ATTR_VOL_OWNER)

struct vol_values
{
	uint32_t length;
	uint32_t returned_volattr;
	uint32_t fstype, mountflags, mountextflags, fssubtype, owner;
	char mountpoint[1100], device[1100], fstypename[64];
	uint32_t uuid_present;
	unsigned char uuid[16];
	char name[300];
	uint32_t end_of_fixed;     // offset of the first byte after the fixed part
	uint32_t end_of_text;      // offset after the last text byte, padded
};

// parses `buf` (from getattrlist for volattr `want` with or without RETURNED_ATTRS) into `v`; 0 if it is well formed
static int parse(const unsigned char* buf, size_t size, uint32_t want, int returned_attrs, struct vol_values* v)
{
	memset(v, 0, sizeof(*v));
	if (size < 4)
		return -1;
	memcpy(&v->length, buf, 4);
	if (v->length > size)
		return -1;
	size_t at = 4;
	if (returned_attrs)
	{
		uint32_t returned[5];
		memcpy(returned, buf + at, sizeof(returned));
		at += sizeof(returned);
		v->returned_volattr = returned[1];
		want &= returned[1];
	}
	size_t text_end = 0;
	for (uint32_t bit = 1; bit; bit <<= 1)
	{
		if (!(want & bit) || bit == ATTR_VOL_INFO)
			continue;
		const unsigned char* p = buf + at;
		switch (bit)
		{
			case ATTR_VOL_FSTYPE: memcpy(&v->fstype, p, 4); at += 4; break;
			case ATTR_VOL_MOUNTFLAGS: memcpy(&v->mountflags, p, 4); at += 4; break;
			case ATTR_VOL_MOUNTEXTFLAGS: memcpy(&v->mountextflags, p, 4); at += 4; break;
			case ATTR_VOL_FSSUBTYPE: memcpy(&v->fssubtype, p, 4); at += 4; break;
			case ATTR_VOL_OWNER: memcpy(&v->owner, p, 4); at += 4; break;
			case ATTR_VOL_UUID: memcpy(v->uuid, p, 16); v->uuid_present = 1; at += 16; break;
			case ATTR_VOL_MOUNTPOINT:
			case ATTR_VOL_MOUNTEDDEVICE:
			case ATTR_VOL_FSTYPENAME:
			case ATTR_VOL_NAME:
			{
				int32_t offset;
				uint32_t length;
				memcpy(&offset, p, 4);
				memcpy(&length, p + 4, 4);
				long text = (long) at + offset;
				if (text < 0 || (size_t) text + length > v->length || length == 0 || buf[text + length - 1] != 0 ||
					strlen((const char*) buf + text) + 1 != length || (text & 3))
					return -1;
				char* out = bit == ATTR_VOL_MOUNTPOINT ? v->mountpoint : bit == ATTR_VOL_MOUNTEDDEVICE ? v->device :
					bit == ATTR_VOL_FSTYPENAME ? v->fstypename : v->name;
				memcpy(out, buf + text, length);
				size_t end = ((size_t) text + length + 3) & ~(size_t) 3;
				if (end > text_end)
					text_end = end;
				at += 8;
				break;
			}
			default:
				return -1;
		}
	}
	v->end_of_fixed = at;
	v->end_of_text = text_end;
	return 0;
}

static int volume_info(const char* path, int fd, uint32_t volattr, int returned_attrs, struct vol_values* v, int* err)
{
	struct attrlist al;
	unsigned char buf[8192];
	memset(&al, 0, sizeof(al));
	memset(buf, 0xEE, sizeof(buf));
	al.bitmapcount = ATTR_BIT_MAP_COUNT;
	al.commonattr = returned_attrs ? ATTR_CMN_RETURNED_ATTRS : 0;
	al.volattr = ATTR_VOL_INFO | volattr;
	errno = 0;
	int rv = fd >= 0 ? fgetattrlist(fd, &al, buf, sizeof(buf), 0) : getattrlist(path, &al, buf, sizeof(buf), 0);
	if (err)
		*err = errno;
	if (rv != 0)
		return -1;
	if (parse(buf, sizeof(buf), al.volattr, returned_attrs, v) != 0)
		return -2;
	return 0;
}

static void check_matches_statfs(const char* path, const struct vol_values* v)
{
	struct statfs fs;
	CHECK(statfs(path, &fs) == 0);
	CHECK(v->fstype == fs.f_type);
	CHECK(strcmp(v->mountpoint, fs.f_mntonname) == 0);
	CHECK(v->mountflags == fs.f_flags);
	CHECK(strcmp(v->device, fs.f_mntfromname) == 0);
	CHECK(v->mountextflags == fs.f_flags_ext);
	CHECK(strcmp(v->fstypename, fs.f_fstypename) == 0);
	CHECK(v->fssubtype == fs.f_fssubtype);
	CHECK(v->owner == fs.f_owner);
}

int main(void)
{
	struct vol_values v, w;
	int err;
	char dir[] = "/tmp/getattrlist-volume-XXXXXX";
	char file[256];

	CHECK(mkdtemp(dir) != NULL);
	snprintf(file, sizeof(file), "%s/file", dir);
	close(open(file, O_CREAT | O_WRONLY, 0644));

	// the attributes of the root volume, and of the volume that holds a directory and a file
	const char* paths[] = { "/", dir, file };
	for (unsigned i = 0; i < sizeof(paths) / sizeof(paths[0]); i++)
	{
		CHECK(volume_info(paths[i], -1, ALL_STATFS_ATTRS, 1, &v, &err) == 0);
		// ATTR_VOL_INFO is not among the returned attributes
		CHECK(v.returned_volattr == ALL_STATFS_ATTRS);
		CHECK(v.length >= v.end_of_fixed && v.length >= v.end_of_text);
		CHECK(v.length == (v.end_of_text > v.end_of_fixed ? v.end_of_text : v.end_of_fixed));
		CHECK(v.mountpoint[0] == '/' && v.fstypename[0] != 0 && v.device[0] != 0);
		check_matches_statfs(paths[i], &v);

		// the same without ATTR_CMN_RETURNED_ATTRS
		CHECK(volume_info(paths[i], -1, ALL_STATFS_ATTRS, 0, &w, &err) == 0);
		CHECK(w.fstype == v.fstype && w.mountflags == v.mountflags && w.mountextflags == v.mountextflags &&
			w.fssubtype == v.fssubtype && w.owner == v.owner && strcmp(w.mountpoint, v.mountpoint) == 0 &&
			strcmp(w.device, v.device) == 0 && strcmp(w.fstypename, v.fstypename) == 0);
		CHECK(w.length == v.length - 20);

		// through a descriptor
		int fd = open(paths[i], O_RDONLY);
		CHECK(fd >= 0);
		CHECK(volume_info(NULL, fd, ALL_STATFS_ATTRS, 1, &w, &err) == 0);
		CHECK(w.length == v.length && strcmp(w.mountpoint, v.mountpoint) == 0 && strcmp(w.fstypename, v.fstypename) == 0 &&
			w.mountflags == v.mountflags && w.fssubtype == v.fssubtype);
		close(fd);
	}

	// each attribute on its own
	{
		uint32_t bits[] = { ATTR_VOL_FSTYPE, ATTR_VOL_MOUNTPOINT, ATTR_VOL_MOUNTFLAGS, ATTR_VOL_MOUNTEDDEVICE,
			ATTR_VOL_MOUNTEXTFLAGS, ATTR_VOL_FSTYPENAME, ATTR_VOL_FSSUBTYPE, ATTR_VOL_OWNER };
		struct vol_values all;
		CHECK(volume_info("/", -1, ALL_STATFS_ATTRS, 1, &all, &err) == 0);
		for (unsigned i = 0; i < sizeof(bits) / sizeof(bits[0]); i++)
		{
			CHECK(volume_info("/", -1, bits[i], 1, &v, &err) == 0);
			CHECK(v.returned_volattr == bits[i]);
			CHECK((bits[i] != ATTR_VOL_FSTYPE || v.fstype == all.fstype) &&
				(bits[i] != ATTR_VOL_MOUNTFLAGS || v.mountflags == all.mountflags) &&
				(bits[i] != ATTR_VOL_MOUNTEXTFLAGS || v.mountextflags == all.mountextflags) &&
				(bits[i] != ATTR_VOL_FSSUBTYPE || v.fssubtype == all.fssubtype) &&
				(bits[i] != ATTR_VOL_OWNER || v.owner == all.owner) &&
				(bits[i] != ATTR_VOL_MOUNTPOINT || strcmp(v.mountpoint, all.mountpoint) == 0) &&
				(bits[i] != ATTR_VOL_MOUNTEDDEVICE || strcmp(v.device, all.device) == 0) &&
				(bits[i] != ATTR_VOL_FSTYPENAME || strcmp(v.fstypename, all.fstypename) == 0));
		}
	}

	// together with the volume's UUID and name: MOUNTPOINT|UUID|FSTYPENAME, and the strings of NAME among them
	{
		CHECK(volume_info("/", -1, ATTR_VOL_MOUNTPOINT | ATTR_VOL_UUID | ATTR_VOL_FSTYPENAME, 1, &v, &err) == 0);
		CHECK(v.returned_volattr == (ATTR_VOL_MOUNTPOINT | ATTR_VOL_UUID | ATTR_VOL_FSTYPENAME));
		CHECK(v.uuid_present);
		CHECK(strcmp(v.mountpoint, "/") == 0);
		struct statfs fs;
		CHECK(statfs("/", &fs) == 0 && strcmp(v.fstypename, fs.f_fstypename) == 0);
		CHECK(volume_info("/", -1, ATTR_VOL_MOUNTPOINT | ATTR_VOL_UUID | ATTR_VOL_FSTYPENAME, 0, &w, &err) == 0);
		CHECK(strcmp(w.fstypename, v.fstypename) == 0 && memcmp(w.uuid, v.uuid, 16) == 0);

		CHECK(volume_info("/", -1, ALL_STATFS_ATTRS | ATTR_VOL_NAME | ATTR_VOL_UUID, 1, &v, &err) == 0);
		CHECK(v.name[0] != 0 && v.uuid_present);
		struct vol_values only_name;
		CHECK(volume_info("/", -1, ATTR_VOL_NAME, 1, &only_name, &err) == 0);
		CHECK(strcmp(v.name, only_name.name) == 0);
		check_matches_statfs("/", &v);
	}

	// undefined bits
	{
		struct attrlist al;
		unsigned char buf[1024];
		memset(&al, 0, sizeof(al));
		al.bitmapcount = ATTR_BIT_MAP_COUNT;
		al.volattr = ATTR_VOL_INFO | 0x01000000;
		errno = 0;
		CHECK(getattrlist("/", &al, buf, sizeof(buf), 0) == -1 && errno == EINVAL);
		al.volattr = ATTR_VOL_INFO | 0x08000000;
		errno = 0;
		CHECK(getattrlist("/", &al, buf, sizeof(buf), 0) == -1 && errno == EINVAL);
	}

	// a missing path
	{
		struct attrlist al;
		unsigned char buf[1024];
		memset(&al, 0, sizeof(al));
		al.bitmapcount = ATTR_BIT_MAP_COUNT;
		al.volattr = ATTR_VOL_INFO | ATTR_VOL_FSTYPENAME;
		errno = 0;
		CHECK(getattrlist("/tmp/getattrlist-volume-missing", &al, buf, sizeof(buf), 0) == -1 && errno == ENOENT);
	}

	unlink(file);
	rmdir(dir);
	printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
	return failures != 0;
}
