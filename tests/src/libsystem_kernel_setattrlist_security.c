/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * setattrlist() with ATTR_CMN_EXTENDED_SECURITY (a file's ACL), ATTR_CMN_UUID and ATTR_CMN_GRPUUID, as a file installer
 * uses them to clear an ACL, and what a request that is not applied must not do:
 *  - an ACL without entries (or KAUTH_FILESEC_NOACL) and null GUIDs are accepted: a file without an ACL or GUIDs is
 *    what they ask for;
 *  - a malformed request (wrong magic number, an empty or too short reference, a reference that does not lie in the
 *    buffer, more entries than the data holds, a buffer too small for the reference) is EINVAL; a GUID that is not null
 *    is EPERM (macOS 26.7.3, not as root);
 *  - an ACL with entries is either applied (macOS) or refused with ENOTSUP (Darling keeps no ACLs): a success must be
 *    visible afterwards in getattrlist(ATTR_CMN_EXTENDED_SECURITY), a failure must not change the file;
 *  - a request that fails changes nothing of what else was asked for with it (the access mask);
 *  - an attribute that is not applied is not reported as set: the creation time is either set (and then read back as
 *    set) or refused.
 * Every expected value was observed on macOS 26.7.3; the program also passes there.
 */
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/attr.h>
#include <sys/stat.h>
#include <unistd.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s (errno %d)\n", __LINE__, #cond, errno); failures++; } } while (0)

#define KAUTH_FILESEC_MAGIC_VALUE 0x012CC16DU
#define NOACL 0xffffffffU

// the value of ATTR_CMN_EXTENDED_SECURITY: a reference, then struct kauth_filesec (magic, owner GUID, group GUID,
// ACL entry count, ACL flags, entries of 24 bytes)
struct request
{
	attrreference_t ref;
	unsigned char filesec[44 + 24 * 2];
};

static void fill(struct request* r, uint32_t magic, uint32_t entries, uint32_t length)
{
	memset(r, 0, sizeof(*r));
	r->ref.attr_dataoffset = sizeof(attrreference_t);
	r->ref.attr_length = length;
	memcpy(r->filesec, &magic, 4);
	memcpy(r->filesec + 36, &entries, 4);
	if (entries != NOACL && entries > 0)
	{
		// a permit entry for a null GUID
		uint32_t flags = 0x2, rights = 0x2;
		memcpy(r->filesec + 44 + 16, &flags, 4);
		memcpy(r->filesec + 44 + 20, &rights, 4);
	}
}

static int set_security(const char* path, const struct request* r, size_t size)
{
	struct attrlist al;
	memset(&al, 0, sizeof(al));
	al.bitmapcount = ATTR_BIT_MAP_COUNT;
	al.commonattr = ATTR_CMN_EXTENDED_SECURITY;
	errno = 0;
	return setattrlist(path, &al, (void*) r, size, 0);
}

// whether getattrlist reports an ACL
static int has_acl(const char* path)
{
	struct attrlist al;
	unsigned char buf[1024];
	memset(&al, 0, sizeof(al));
	al.bitmapcount = ATTR_BIT_MAP_COUNT;
	al.commonattr = ATTR_CMN_RETURNED_ATTRS | ATTR_CMN_EXTENDED_SECURITY;
	if (getattrlist(path, &al, buf, sizeof(buf), 0) != 0)
		return -1;
	uint32_t returned_common;
	memcpy(&returned_common, buf + 4, 4);
	return (returned_common & ATTR_CMN_EXTENDED_SECURITY) != 0;
}

int main(void)
{
	char dir[] = "/tmp/setattrlist-security-XXXXXX";
	char file[256];
	struct request r;
	struct attrlist al;

	CHECK(mkdtemp(dir) != NULL);
	snprintf(file, sizeof(file), "%s/file", dir);
	close(open(file, O_CREAT | O_WRONLY, 0644));
	CHECK(has_acl(file) == 0);

	// no ACL: accepted
	fill(&r, KAUTH_FILESEC_MAGIC_VALUE, NOACL, 44);
	CHECK(set_security(file, &r, 8 + 44) == 0);
	fill(&r, KAUTH_FILESEC_MAGIC_VALUE, 0, 44);
	CHECK(set_security(file, &r, 8 + 44) == 0);

	// malformed
	fill(&r, 0x1234, NOACL, 44);
	CHECK(set_security(file, &r, 8 + 44) == -1 && errno == EINVAL);
	fill(&r, KAUTH_FILESEC_MAGIC_VALUE, NOACL, 0);
	CHECK(set_security(file, &r, 8) == -1 && errno == EINVAL);
	fill(&r, KAUTH_FILESEC_MAGIC_VALUE, NOACL, 20);
	CHECK(set_security(file, &r, 8 + 44) == -1 && errno == EINVAL);
	fill(&r, KAUTH_FILESEC_MAGIC_VALUE, NOACL, 44);
	CHECK(set_security(file, &r, 8 + 20) == -1 && errno == EINVAL);
	CHECK(set_security(file, &r, 4) == -1 && errno == EINVAL);
	fill(&r, KAUTH_FILESEC_MAGIC_VALUE, NOACL, 44);
	r.ref.attr_dataoffset = 4000;
	CHECK(set_security(file, &r, 8 + 44) == -1 && errno == EINVAL);
	fill(&r, KAUTH_FILESEC_MAGIC_VALUE, 3, 44);
	CHECK(set_security(file, &r, 8 + 44) == -1 && errno == EINVAL);

	// an ACL with an entry: applied (and then reported), or refused with ENOTSUP and nothing changed
	fill(&r, KAUTH_FILESEC_MAGIC_VALUE, 1, 44 + 24);
	int rv = set_security(file, &r, 8 + 44 + 24);
	if (rv == 0)
		CHECK(has_acl(file) == 1);
	else
	{
		CHECK(rv == -1 && errno == ENOTSUP);
		CHECK(has_acl(file) == 0);
	}
	// an empty ACL is accepted whatever the file has, and a file that has an ACL ends up without
	fill(&r, KAUTH_FILESEC_MAGIC_VALUE, NOACL, 44);
	CHECK(set_security(file, &r, 8 + 44) == 0);
	CHECK(has_acl(file) == 0);

	// GUIDs: null accepted, any other refused
	{
		unsigned char guid[16];
		memset(&al, 0, sizeof(al));
		al.bitmapcount = ATTR_BIT_MAP_COUNT;
		memset(guid, 0, sizeof(guid));
		al.commonattr = ATTR_CMN_UUID;
		CHECK(setattrlist(file, &al, guid, sizeof(guid), 0) == 0);
		al.commonattr = ATTR_CMN_GRPUUID;
		CHECK(setattrlist(file, &al, guid, sizeof(guid), 0) == 0);
		for (int i = 0; i < 16; i++)
			guid[i] = 0x40 + i;
		if (geteuid() != 0)
		{
			al.commonattr = ATTR_CMN_UUID;
			CHECK(setattrlist(file, &al, guid, sizeof(guid), 0) == -1 && errno == EPERM);
			al.commonattr = ATTR_CMN_GRPUUID;
			CHECK(setattrlist(file, &al, guid, sizeof(guid), 0) == -1 && errno == EPERM);
		}
	}

	// together with the access mask: applied when everything is acceptable, nothing changed when not
	{
		struct { uint32_t mode; struct request r; } both;
		struct stat st;
		memset(&al, 0, sizeof(al));
		al.bitmapcount = ATTR_BIT_MAP_COUNT;
		al.commonattr = ATTR_CMN_ACCESSMASK | ATTR_CMN_EXTENDED_SECURITY;
		CHECK(chmod(file, 0644) == 0);

		both.mode = 0600;
		fill(&both.r, KAUTH_FILESEC_MAGIC_VALUE, NOACL, 44);
		// the reference's offset is from the reference itself: it follows the mode
		both.r.ref.attr_dataoffset = 8;
		CHECK(setattrlist(file, &al, &both, 4 + 8 + 44, 0) == 0);
		CHECK(stat(file, &st) == 0 && (st.st_mode & 07777) == 0600);

		both.mode = 0640;
		fill(&both.r, 0x1234, NOACL, 44);
		both.r.ref.attr_dataoffset = 8;
		errno = 0;
		CHECK(setattrlist(file, &al, &both, 4 + 8 + 44, 0) == -1 && errno == EINVAL);
		CHECK(stat(file, &st) == 0 && (st.st_mode & 07777) == 0600);
	}

	// an attribute that is not applied is not reported as set
	{
		struct timespec ts = { 1000000000, 0 };
		struct { struct timespec t; } crtime = { ts };
		struct attrlist get;
		unsigned char got[64];
		memset(&al, 0, sizeof(al));
		al.bitmapcount = ATTR_BIT_MAP_COUNT;
		al.commonattr = ATTR_CMN_CRTIME;
		rv = setattrlist(file, &al, &crtime, sizeof(crtime), 0);
		if (rv == 0)
		{
			memset(&get, 0, sizeof(get));
			get.bitmapcount = ATTR_BIT_MAP_COUNT;
			get.commonattr = ATTR_CMN_CRTIME;
			struct timespec read_back;
			CHECK(getattrlist(file, &get, got, sizeof(got), 0) == 0);
			// the value follows the length field, which does not align it
			memcpy(&read_back, got + 4, sizeof(read_back));
			CHECK(read_back.tv_sec == ts.tv_sec);
		}
		else
			CHECK(errno == EINVAL || errno == ENOTSUP);
	}

	unlink(file);
	rmdir(dir);
	printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
	return failures != 0;
}
