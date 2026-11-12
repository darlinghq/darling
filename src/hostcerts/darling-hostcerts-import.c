// Copyright (C) 2026 Lubos Dolezel
//
// darling-hostcerts-import makes the host's CA store the source of truth for the
// guest's system trust anchors.
//
// The anchors that SecTrust evaluates against (trustd's system anchor source) are
// read from the index and table files of
// /System/Library/Security/Certificates.bundle: certsIndex.data maps the SHA-1 of
// a certificate's normalized subject to an offset into certsTable.data, which
// holds the DER certificates. This tool
//
//   1. finds the host's PEM bundle under /Volumes/SystemRoot and parses it;
//   2. writes new certsIndex.data and certsTable.data for exactly the
//      certificates of that bundle into the bundle's Resources directory (in the
//      guest's overlay of /System, beside the shipped files), so a root that the
//      host does not trust is not trusted by the guest, and one the host adds is;
//   3. without a usable host bundle, restores the shipped anchors (copies of the
//      files that Darling's build generated, kept in /usr/libexec/darling-hostcerts
//      so that they cannot be confused with older installed files), but only if an
//      earlier run replaced them; otherwise it leaves the shipped files alone;
//   4. stops trustd when the files changed, because trustd reads the anchors once
//      at start; the next request starts it again with the new anchors;
//   5. creates /Library/Keychains/System.keychain with an empty password if it
//      does not exist (nothing is imported into it and no trust settings are
//      written, so the com.apple.ocspd trust settings service is not needed).
//
// Only the index and table are replaced; the other files of the bundle (EV roots,
// block lists, ...) stay as shipped. The files are written to a temporary name and
// renamed, one after the other, so a running trustd keeps a consistent mapping of
// the old data; it is only the short window between the two renames in which a
// freshly started trustd could pair a new index with an old table, which is why the
// tool runs at load, before most clients. The anchors are produced from the
// bundle's content on every run and compared with the files, so a changed host
// bundle is picked up at the next load and an unchanged one writes nothing. A
// state file (/var/db/org.darlinghq.hostcerts.plist) records only that the shipped
// files were replaced and from which bundle.
//
// The org.darlinghq.hostcerts launchd job runs it once when the system is loaded
// (run `launchctl start org.darlinghq.hostcerts` to apply a change of the host
// bundle sooner). Nothing happens when the ImportHostCertificates preference of
// the org.darlinghq.security domain is false. Failures are logged and never
// prevent the guest from starting; the tool exits 0 unless it was given a bad
// option.
//
// Options (for administrators and the tests):
//   --bundle FILE    use this PEM file instead of searching for the host's (no
//                    ownership check); a file that cannot be read counts as "no
//                    host bundle"
//   --shipped DIR    directory holding the shipped certsIndex.data and
//                    certsTable.data
//   --output DIR     write the index and table here instead of the bundle's
//                    Resources directory (trustd is not stopped then)
//   --keychain FILE  create this keychain instead of the System keychain
//   --state FILE     keep the state in this file
//   --force          write the files even if they already have this content

#include <CommonCrypto/CommonDigest.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <os/log.h>
#include <spawn.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

extern char** environ;

#define HOST_ROOT "/Volumes/SystemRoot"
#define ANCHOR_DIRECTORY "/System/Library/Security/Certificates.bundle/Contents/Resources"
#define SHIPPED_DIRECTORY "/usr/libexec/darling-hostcerts"
#define SYSTEM_KEYCHAIN_PATH "/Library/Keychains/System.keychain"
#define STATE_PATH "/var/db/org.darlinghq.hostcerts.plist"
#define LOG_PATH "/var/log/darling-hostcerts.log"
#define LAUNCHCTL_PATH "/bin/launchctl"
#define TRUSTD_LABEL "com.apple.trustd"
#define SECURITY_DOMAIN CFSTR("org.darlinghq.security")
#define PREFERENCE_KEY CFSTR("ImportHostCertificates")

#define INDEX_NAME "certsIndex.data"
#define TABLE_NAME "certsTable.data"

#define INDEX_RECORD_SIZE 24 // SHA-1 of the subject content and a 4-byte table offset
#define MAX_FILE_SIZE (16 * 1024 * 1024)
#define MAX_CERTIFICATES 4096
#define STATE_VERSION 2

// Where distributions keep the PEM bundle of their trusted CAs, relative to the
// host's root. When a path is a symbolic link with an absolute target, the target
// would be looked up in the guest, so the usual targets are listed as well.
static const char* const kBundlePaths[] = {
	"/etc/ssl/certs/ca-certificates.crt",                  // Debian, Ubuntu, Arch, Alpine, Gentoo
	"/etc/pki/tls/certs/ca-bundle.crt",                    // Fedora, RHEL
	"/etc/pki/ca-trust/extracted/pem/tls-ca-bundle.pem",   // Fedora, RHEL (target of the above)
	"/etc/ssl/ca-bundle.pem",                              // openSUSE
	"/var/lib/ca-certificates/ca-bundle.pem",              // openSUSE (target of the above)
	"/etc/ssl/cert.pem",                                   // Alpine, Void, macOS-style
};

static int g_logFD = -1;

// Messages go to os_log and, because Darling has no log store to read them back
// from, also to LOG_PATH.
static void hc_log(os_log_type_t type, const char* format, ...) __attribute__((format(printf, 2, 3)));
static void hc_log(os_log_type_t type, const char* format, ...)
{
	char message[1024];
	va_list args;

	va_start(args, format);
	vsnprintf(message, sizeof(message), format, args);
	va_end(args);

	os_log_with_type(OS_LOG_DEFAULT, type, "%{public}s", message);

	if (g_logFD == -1)
		g_logFD = open(LOG_PATH, O_WRONLY | O_APPEND | O_CREAT | O_NOFOLLOW | O_CLOEXEC, 0644);
	if (g_logFD != -1)
	{
		char stamp[32] = "";
		char line[sizeof(message) + 64];
		time_t now = time(NULL);
		struct tm tm;

		if (localtime_r(&now, &tm) != NULL)
			strftime(stamp, sizeof(stamp), "%Y-%m-%d %H:%M:%S", &tm);

		int length = snprintf(line, sizeof(line), "%s darling-hostcerts-import[%d]: %s\n", stamp, getpid(), message);
		if (length > 0)
		{
			if ((size_t) length >= sizeof(line))
				length = sizeof(line) - 1;
			(void) write(g_logFD, line, length);
		}
	}
}

//
// files
//

// Reads a regular file of at most MAX_FILE_SIZE bytes. With checkOwner, the file is
// only used if root owns it and neither its group nor others can write it, because
// everything in the host's bundle becomes a trust anchor of the guest.
static CFDataRef read_file(const char* path, bool checkOwner)
{
	int fd = open(path, O_RDONLY | O_CLOEXEC);
	struct stat st;
	UInt8* bytes = NULL;
	CFDataRef data = NULL;

	if (fd == -1)
		return NULL;
	if (fstat(fd, &st) != 0 || !S_ISREG(st.st_mode) || st.st_size <= 0 || st.st_size > MAX_FILE_SIZE)
		goto out;
	if (checkOwner && (st.st_uid != 0 || (st.st_mode & (S_IWGRP | S_IWOTH)) != 0))
	{
		hc_log(OS_LOG_TYPE_ERROR, "%s is not owned by root or is writable by others; not used", path);
		goto out;
	}

	bytes = malloc(st.st_size);
	if (bytes == NULL)
		goto out;

	for (off_t done = 0; done < st.st_size; )
	{
		ssize_t n = read(fd, bytes + done, st.st_size - done);
		if (n <= 0)
		{
			free(bytes);
			bytes = NULL;
			goto out;
		}
		done += n;
	}
	data = CFDataCreateWithBytesNoCopy(NULL, bytes, st.st_size, kCFAllocatorMalloc);
	bytes = NULL;

out:
	free(bytes);
	close(fd);
	return data;
}

// Writes the file under a temporary name and renames it over the target, so a
// reader sees either the old or the new content.
static bool write_file_atomically(const char* path, const UInt8* bytes, CFIndex length)
{
	char temp[1024];
	int fd;
	bool ok = false;

	snprintf(temp, sizeof(temp), "%s.%d", path, getpid());
	fd = open(temp, O_WRONLY | O_CREAT | O_TRUNC | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0644);
	if (fd == -1)
		return false;
	// the mode is the one the readers (trustd runs as _trustd) need, whatever the umask
	if (fchmod(fd, 0644) == 0)
	{
		CFIndex done = 0;

		while (done < length)
		{
			ssize_t n = write(fd, bytes + done, length - done);
			if (n <= 0)
				break;
			done += n;
		}
		if (done == length && fsync(fd) == 0)
			ok = rename(temp, path) == 0;
	}
	close(fd);
	if (!ok)
		unlink(temp);
	return ok;
}

static void make_parent_directories(const char* path)
{
	char dir[1024];

	snprintf(dir, sizeof(dir), "%s", path);
	for (char* slash = strchr(dir + 1, '/'); slash != NULL; slash = strchr(slash + 1, '/'))
	{
		*slash = '\0';
		(void) mkdir(dir, 0755);
		*slash = '/';
	}
}

//
// PEM parsing
//

static int base64_value(unsigned char c)
{
	if (c >= 'A' && c <= 'Z') return c - 'A';
	if (c >= 'a' && c <= 'z') return c - 'a' + 26;
	if (c >= '0' && c <= '9') return c - '0' + 52;
	if (c == '+') return 62;
	if (c == '/') return 63;
	return -1;
}

// Decodes base64 text; white space is ignored, anything else invalid fails.
static CFDataRef base64_decode(const char* text, size_t length)
{
	UInt8* out = malloc(length / 4 * 3 + 3);
	size_t o = 0;
	uint32_t accumulator = 0;
	int bits = 0;
	bool padded = false;

	if (out == NULL)
		return NULL;

	for (size_t i = 0; i < length; i++)
	{
		unsigned char c = text[i];
		int v;

		if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
			continue;
		if (c == '=')
		{
			padded = true;
			continue;
		}
		v = base64_value(c);
		if (v < 0 || padded)
		{
			free(out);
			return NULL;
		}
		accumulator = (accumulator << 6) | v;
		bits += 6;
		if (bits >= 8)
		{
			bits -= 8;
			out[o++] = (accumulator >> bits) & 0xff;
			accumulator &= (1u << bits) - 1;
		}
	}

	if (o == 0)
	{
		free(out);
		return NULL;
	}
	return CFDataCreateWithBytesNoCopy(NULL, out, o, kCFAllocatorMalloc);
}

//
// the set of anchors: certificates without duplicates, ordered by their SHA-256
// digest so that the same input always gives the same files
//

typedef struct
{
	CFDataRef digest;
	SecCertificateRef cert;
} anchor_t;

typedef struct
{
	anchor_t* items;
	size_t count;
	size_t capacity;
} anchors_t;

static CFDataRef sha256(const UInt8* bytes, CFIndex length)
{
	UInt8 digest[CC_SHA256_DIGEST_LENGTH];

	CC_SHA256(bytes, (CC_LONG) length, digest);
	return CFDataCreate(NULL, digest, sizeof(digest));
}

static void anchors_add(anchors_t* anchors, SecCertificateRef cert)
{
	CFDataRef der = SecCertificateCopyData(cert);
	CFDataRef digest;

	if (der == NULL)
		return;
	digest = sha256(CFDataGetBytePtr(der), CFDataGetLength(der));
	CFRelease(der);
	if (digest == NULL)
		return;

	for (size_t i = 0; i < anchors->count; i++)
	{
		if (CFEqual(anchors->items[i].digest, digest))
		{
			CFRelease(digest);
			return;
		}
	}
	if (anchors->count == MAX_CERTIFICATES)
	{
		CFRelease(digest);
		return;
	}
	if (anchors->count == anchors->capacity)
	{
		size_t capacity = anchors->capacity != 0 ? anchors->capacity * 2 : 256;
		anchor_t* items = realloc(anchors->items, capacity * sizeof(anchor_t));

		if (items == NULL)
		{
			CFRelease(digest);
			return;
		}
		anchors->items = items;
		anchors->capacity = capacity;
	}
	anchors->items[anchors->count].digest = digest;
	anchors->items[anchors->count].cert = (SecCertificateRef) CFRetain(cert);
	anchors->count++;
}

static int compare_anchors(const void* a, const void* b)
{
	const anchor_t* x = a;
	const anchor_t* y = b;

	return memcmp(CFDataGetBytePtr(x->digest), CFDataGetBytePtr(y->digest), CC_SHA256_DIGEST_LENGTH);
}

// Adds the certificates of a PEM text. Text outside the markers is ignored, as
// in a bundle with comments; a block that is not a certificate that Security
// accepts is skipped.
static void anchors_add_pem(anchors_t* anchors, CFDataRef pem)
{
	static const char begin[] = "-----BEGIN CERTIFICATE-----";
	static const char end[] = "-----END CERTIFICATE-----";
	const char* p = (const char*) CFDataGetBytePtr(pem);
	const char* limit = p + CFDataGetLength(pem);

	while (anchors->count < MAX_CERTIFICATES)
	{
		const char* b = memmem(p, limit - p, begin, sizeof(begin) - 1);
		const char* e;
		CFDataRef der;

		if (b == NULL)
			break;
		b += sizeof(begin) - 1;
		e = memmem(b, limit - b, end, sizeof(end) - 1);
		if (e == NULL)
			break;
		p = e + sizeof(end) - 1;

		der = base64_decode(b, e - b);
		if (der != NULL)
		{
			SecCertificateRef cert = SecCertificateCreateWithData(NULL, der);

			if (cert != NULL)
			{
				anchors_add(anchors, cert);
				CFRelease(cert);
			}
			CFRelease(der);
		}
	}
}

static void anchors_finish(anchors_t* anchors)
{
	qsort(anchors->items, anchors->count, sizeof(anchor_t), compare_anchors);
}

static void anchors_free(anchors_t* anchors)
{
	for (size_t i = 0; i < anchors->count; i++)
	{
		CFRelease(anchors->items[i].digest);
		CFRelease(anchors->items[i].cert);
	}
	free(anchors->items);
	memset(anchors, 0, sizeof(*anchors));
}

//
// where the anchors come from
//

// The host's bundle: the first usable file of the usual locations (or the one
// given with --bundle). Returns false when there is none or it has no
// certificates.
static bool load_host_anchors(const char* explicitPath, anchors_t* anchors, CFDataRef* digest, char* usedPath, size_t usedPathSize)
{
	CFDataRef data = NULL;

	if (explicitPath != NULL)
	{
		data = read_file(explicitPath, false);
		snprintf(usedPath, usedPathSize, "%s", explicitPath);
	}
	else
	{
		for (size_t i = 0; i < sizeof(kBundlePaths) / sizeof(kBundlePaths[0]) && data == NULL; i++)
		{
			snprintf(usedPath, usedPathSize, HOST_ROOT "%s", kBundlePaths[i]);
			data = read_file(usedPath, true);
		}
	}
	if (data == NULL)
		return false;

	anchors_add_pem(anchors, data);
	if (anchors->count == 0)
	{
		hc_log(OS_LOG_TYPE_ERROR, "%s contains no certificates; not used", usedPath);
		CFRelease(data);
		return false;
	}
	*digest = sha256(CFDataGetBytePtr(data), CFDataGetLength(data));
	CFRelease(data);
	anchors_finish(anchors);
	return true;
}

//
// the anchor files
//

static void append_le32(CFMutableDataRef data, uint32_t value)
{
	UInt8 bytes[4] = { value & 0xff, (value >> 8) & 0xff, (value >> 16) & 0xff, (value >> 24) & 0xff };

	CFDataAppendBytes(data, bytes, sizeof(bytes));
}

// The content of the normalized subject name: the SEQUENCE returned by Security
// without its tag and length, which is what the lookup hashes.
static bool subject_content(CFDataRef sequence, const UInt8** content, CFIndex* length)
{
	const UInt8* b = CFDataGetBytePtr(sequence);
	CFIndex size = CFDataGetLength(sequence);
	CFIndex header;

	if (size < 2 || b[0] != 0x30)
		return false;
	if (b[1] < 0x80)
		header = 2;
	else
	{
		CFIndex octets = b[1] & 0x7f;

		if (octets == 0 || octets > 4)
			return false;
		header = 2 + octets;
	}
	if (header > size)
		return false;
	*content = b + header;
	*length = size - header;
	return true;
}

// Builds the two files in the layout trustd reads: the index has one 24-byte
// record per certificate (SHA-1 of the normalized subject content, offset in the
// table, little endian); each table record is its own length (8 more than the
// padded certificate), the certificate's length and the DER certificate padded
// with 0xff to a multiple of 8.
static void build_anchor_files(const anchors_t* anchors, CFMutableDataRef index, CFMutableDataRef table, size_t* used)
{
	*used = 0;
	for (size_t i = 0; i < anchors->count; i++)
	{
		CFDataRef der = SecCertificateCopyData(anchors->items[i].cert);
		CFDataRef subject = SecCertificateCopyNormalizedSubjectSequence(anchors->items[i].cert);
		const UInt8* content;
		CFIndex contentLength;
		UInt8 hash[CC_SHA1_DIGEST_LENGTH];

		if (der == NULL || subject == NULL || !subject_content(subject, &content, &contentLength))
		{
			hc_log(OS_LOG_TYPE_ERROR, "skipping a certificate whose subject cannot be read");
		}
		else
		{
			CFIndex derLength = CFDataGetLength(der);
			CFIndex padded = (derLength + 7) / 8 * 8;
			UInt8 pad[8] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

			CC_SHA1(content, (CC_LONG) contentLength, hash);
			CFDataAppendBytes(index, hash, sizeof(hash));
			append_le32(index, (uint32_t) CFDataGetLength(table));

			append_le32(table, (uint32_t) (padded + 8));
			append_le32(table, (uint32_t) derLength);
			CFDataAppendBytes(table, CFDataGetBytePtr(der), derLength);
			CFDataAppendBytes(table, pad, padded - derLength);
			(*used)++;
		}
		if (der != NULL)
			CFRelease(der);
		if (subject != NULL)
			CFRelease(subject);
	}
}

static bool file_has_content(const char* path, CFDataRef content)
{
	CFDataRef current = read_file(path, false);
	bool same = current != NULL && CFEqual(current, content);

	if (current != NULL)
		CFRelease(current);
	return same;
}

//
// state
//

static CFDictionaryRef load_state(const char* path)
{
	CFDataRef data = read_file(path, false);
	CFPropertyListRef plist = NULL;

	if (data == NULL)
		return NULL;
	plist = CFPropertyListCreateWithData(NULL, data, kCFPropertyListImmutable, NULL, NULL);
	CFRelease(data);
	if (plist != NULL && CFGetTypeID(plist) != CFDictionaryGetTypeID())
	{
		CFRelease(plist);
		plist = NULL;
	}
	return plist;
}

// The source ("host" or "shipped") that an earlier run wrote the anchor files
// from, or NULL if there was none (the shipped files are still in use).
static CFStringRef state_source(CFDictionaryRef state)
{
	CFTypeRef version, source;
	SInt32 v = 0;

	if (state == NULL)
		return NULL;
	version = CFDictionaryGetValue(state, CFSTR("Version"));
	source = CFDictionaryGetValue(state, CFSTR("Source"));
	if (version != NULL && CFGetTypeID(version) == CFNumberGetTypeID())
		CFNumberGetValue(version, kCFNumberSInt32Type, &v);
	if (v == STATE_VERSION && source != NULL && CFGetTypeID(source) == CFStringGetTypeID())
		return source;
	return NULL;
}

static void save_state(const char* path, const char* source, CFDataRef bundleDigest, size_t count)
{
	int32_t version = STATE_VERSION;
	int32_t number = (int32_t) count;
	CFNumberRef versionNumber = CFNumberCreate(NULL, kCFNumberSInt32Type, &version);
	CFNumberRef countNumber = CFNumberCreate(NULL, kCFNumberSInt32Type, &number);
	CFStringRef sourceString = CFStringCreateWithCString(NULL, source, kCFStringEncodingUTF8);
	CFMutableDictionaryRef state = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFDataRef data;

	CFDictionarySetValue(state, CFSTR("Version"), versionNumber);
	CFDictionarySetValue(state, CFSTR("Source"), sourceString);
	CFDictionarySetValue(state, CFSTR("Certificates"), countNumber);
	if (bundleDigest != NULL)
		CFDictionarySetValue(state, CFSTR("BundleDigest"), bundleDigest);

	data = CFPropertyListCreateData(NULL, state, kCFPropertyListXMLFormat_v1_0, 0, NULL);
	make_parent_directories(path);
	if (data == NULL || !write_file_atomically(path, CFDataGetBytePtr(data), CFDataGetLength(data)))
		hc_log(OS_LOG_TYPE_ERROR, "cannot write %s: %s", path, strerror(errno));
	if (data != NULL)
		CFRelease(data);
	CFRelease(state);
	CFRelease(sourceString);
	CFRelease(countNumber);
	CFRelease(versionNumber);
}

//
// trustd and the System keychain
//

// trustd maps the anchors when it starts; stopping the job makes the next request
// start a trustd that reads the new files. Harmless when it is not running.
static void stop_trustd(void)
{
	char* argv[] = { LAUNCHCTL_PATH, "stop", TRUSTD_LABEL, NULL };
	pid_t pid;
	int status = 0;

	if (posix_spawn(&pid, LAUNCHCTL_PATH, NULL, NULL, argv, environ) != 0)
	{
		hc_log(OS_LOG_TYPE_ERROR, "cannot run %s: %s", LAUNCHCTL_PATH, strerror(errno));
		return;
	}
	if (waitpid(pid, &status, 0) != pid || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
		hc_log(OS_LOG_TYPE_DEFAULT, "launchctl stop %s did not report success (the job may not be running)", TRUSTD_LABEL);
}

// Creates the keychain when the file does not exist. A keychain made here has an
// empty password, so nobody has to find one. macOS unlocks the System keychain
// with a key kept in /var/db/SystemKey, which Darling does not provide.
static void ensure_keychain(const char* path)
{
	SecKeychainRef keychain = NULL;
	OSStatus status;
	struct stat st;

	if (stat(path, &st) == 0)
		return;

	make_parent_directories(path);
	status = SecKeychainCreate(path, 0, "", false, NULL, &keychain);
	if (status != errSecSuccess)
	{
		hc_log(OS_LOG_TYPE_ERROR, "SecKeychainCreate(%s) failed: %d", path, (int) status);
		return;
	}
	hc_log(OS_LOG_TYPE_DEFAULT, "created %s", path);
	CFRelease(keychain);
}

//
// preferences
//

static bool import_enabled(void)
{
	// kCFPreferencesAnyUser with kCFPreferencesCurrentHost is the system-wide domain,
	// /Library/Preferences/<domain>.plist (any user and any host is its Network
	// subdirectory)
	CFPropertyListRef value = CFPreferencesCopyValue(PREFERENCE_KEY, SECURITY_DOMAIN, kCFPreferencesAnyUser, kCFPreferencesCurrentHost);
	bool enabled = true;

	if (value != NULL)
	{
		if (CFGetTypeID(value) == CFBooleanGetTypeID())
			enabled = CFBooleanGetValue(value);
		else
			hc_log(OS_LOG_TYPE_ERROR, "%s is not a boolean; ignored", "ImportHostCertificates");
		CFRelease(value);
	}
	return enabled;
}

//
// main
//

// Writes the two files into the directory, unless it already has exactly this
// content (or the content is empty). Returns true when files were written;
// *failed is set when the content is unusable or a write failed.
static bool write_anchor_pair(const char* directory, CFDataRef index, CFDataRef table, bool force, bool* failed)
{
	char indexPath[1024], tablePath[1024];
	bool written = false;

	*failed = false;
	snprintf(indexPath, sizeof(indexPath), "%s/%s", directory, INDEX_NAME);
	snprintf(tablePath, sizeof(tablePath), "%s/%s", directory, TABLE_NAME);

	if (CFDataGetLength(index) == 0 || CFDataGetLength(table) == 0)
	{
		hc_log(OS_LOG_TYPE_ERROR, "no usable certificate; the anchors are left as they are");
		*failed = true;
	}
	else if (!force && file_has_content(indexPath, index) && file_has_content(tablePath, table))
		;
	else
	{
		make_parent_directories(indexPath);
		// the table first: the index points into it
		if (write_file_atomically(tablePath, CFDataGetBytePtr(table), CFDataGetLength(table))
			&& write_file_atomically(indexPath, CFDataGetBytePtr(index), CFDataGetLength(index)))
			written = true;
		else
		{
			hc_log(OS_LOG_TYPE_ERROR, "cannot write the anchor files in %s: %s", directory, strerror(errno));
			*failed = true;
		}
	}
	return written;
}

// Writes the anchor files for the certificates (see write_anchor_pair).
static bool update_anchor_files(const char* directory, const anchors_t* anchors, bool force, size_t* used, bool* failed)
{
	CFMutableDataRef index = CFDataCreateMutable(NULL, 0);
	CFMutableDataRef table = CFDataCreateMutable(NULL, 0);
	bool written;

	build_anchor_files(anchors, index, table, used);
	written = write_anchor_pair(directory, index, table, force, failed);
	CFRelease(index);
	CFRelease(table);
	return written;
}

// Puts back the anchor files that Darling's build generated from its shipped
// certificates. They are installed as a pair in a directory only this tool uses
// (nothing else is ever written there, so no stale file can be picked up), as the
// guest's copy of the bundle's own files is masked by the files written here.
static bool restore_shipped_files(const char* shippedDirectory, const char* directory, bool force, size_t* used, bool* failed)
{
	char indexPath[1024], tablePath[1024];
	CFDataRef index, table;
	bool written = false;

	snprintf(indexPath, sizeof(indexPath), "%s/%s", shippedDirectory, INDEX_NAME);
	snprintf(tablePath, sizeof(tablePath), "%s/%s", shippedDirectory, TABLE_NAME);
	index = read_file(indexPath, false);
	table = read_file(tablePath, false);
	*used = 0;
	*failed = true;
	if (index == NULL || table == NULL)
		hc_log(OS_LOG_TYPE_ERROR, "no host bundle and no shipped anchor files in %s; the anchors are left as they are", shippedDirectory);
	else
	{
		*used = CFDataGetLength(index) / INDEX_RECORD_SIZE;
		written = write_anchor_pair(directory, index, table, force, failed);
	}
	if (index != NULL)
		CFRelease(index);
	if (table != NULL)
		CFRelease(table);
	return written;
}

int main(int argc, char** argv)
{
	const char* bundlePath = NULL;
	const char* shippedDirectory = SHIPPED_DIRECTORY;
	const char* outputDirectory = ANCHOR_DIRECTORY;
	const char* keychainPath = SYSTEM_KEYCHAIN_PATH;
	const char* statePath = STATE_PATH;
	bool force = false;
	char usedPath[512] = "";
	anchors_t anchors = { NULL, 0, 0 };
	CFDataRef bundleDigest = NULL;
	CFDictionaryRef oldState = NULL;
	const char* source = NULL;
	bool host;

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--force") == 0)
			force = true;
		else if (strcmp(argv[i], "--bundle") == 0 && i + 1 < argc)
			bundlePath = argv[++i];
		else if (strcmp(argv[i], "--shipped") == 0 && i + 1 < argc)
			shippedDirectory = argv[++i];
		else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc)
			outputDirectory = argv[++i];
		else if (strcmp(argv[i], "--keychain") == 0 && i + 1 < argc)
			keychainPath = argv[++i];
		else if (strcmp(argv[i], "--state") == 0 && i + 1 < argc)
			statePath = argv[++i];
		else
		{
			fprintf(stderr, "usage: %s [--force] [--bundle FILE] [--shipped DIR] [--output DIR] [--keychain FILE] [--state FILE]\n", argv[0]);
			return EX_USAGE;
		}
	}

	if (!import_enabled())
	{
		hc_log(OS_LOG_TYPE_DEFAULT, "ImportHostCertificates is off; nothing to do");
		return EX_OK;
	}

	oldState = load_state(statePath);
	host = load_host_anchors(bundlePath, &anchors, &bundleDigest, usedPath, sizeof(usedPath));
	if (host)
		source = "host";
	else if (state_source(oldState) != NULL)
		source = "shipped"; // the host bundle is gone: back to what Darling ships
	else
		hc_log(OS_LOG_TYPE_DEFAULT, "no usable host certificate bundle; the shipped anchors stay");

	if (source != NULL)
	{
		size_t used = 0;
		bool failed;
		bool written = host
			? update_anchor_files(outputDirectory, &anchors, force, &used, &failed)
			: restore_shipped_files(shippedDirectory, outputDirectory, force, &used, &failed);

		if (!failed)
		{
			hc_log(OS_LOG_TYPE_DEFAULT, "%s anchors%s%s: %zu certificates, %s", source, host ? " from " : "", host ? usedPath : "", used,
				written ? "files updated" : "files already current");
			CFStringRef recorded = state_source(oldState);
			CFStringRef current = CFStringCreateWithCString(NULL, source, kCFStringEncodingUTF8);

			if (written || recorded == NULL || !CFEqual(recorded, current))
				save_state(statePath, source, bundleDigest, used);
			CFRelease(current);
			if (written && strcmp(outputDirectory, ANCHOR_DIRECTORY) == 0)
				stop_trustd();
		}
	}

	ensure_keychain(keychainPath);

	anchors_free(&anchors);
	if (bundleDigest != NULL)
		CFRelease(bundleDigest);
	if (oldState != NULL)
		CFRelease(oldState);
	return EX_OK;
}
