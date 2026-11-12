// Copyright (C) 2026 Lubos Dolezel
//
// darling-hostcerts-import: the host's PEM bundle becomes the set of system
// anchors. The tool writes certsIndex.data and certsTable.data (the files trustd
// reads from Certificates.bundle); the test points it at a temporary directory and
// checks their layout (index records of the SHA-1 of the normalized subject
// content and a table offset; table records with length, DER certificate and 0xff
// padding to 8 bytes), that exactly the host bundle's certificates are in them
// and none of the shipped ones, that an unchanged bundle writes nothing, that a
// changed one is picked up, that without a host bundle the shipped certificates
// come back (only after an earlier replacement), and that duplicates and
// unparseable blocks are handled. The certificates are throw-away self-signed test
// CAs (EC P-256, valid until 2126, generated with OpenSSL; no private key is
// kept); the expected index hashes were computed independently with Python over
// the subject names. A temporary keychain and state file are used and removed
// afterwards; the System keychain and the real bundle are not touched.
//
// usage: hostcerts_import [path of darling-hostcerts-import]
// (default /usr/libexec/darling-hostcerts-import)
#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#include <stdbool.h>
#include <stdint.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

extern char** environ;

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static const char kCA1[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIBqjCCAU+gAwIBAgIUDW4GOqzEwu4YUkB+w2wycaa372AwCgYIKoZIzj0EAwIw\n"
	"ITEfMB0GA1UEAwwWRGFybGluZyBIb3N0IENBIFRlc3QgMTAgFw0yNjExMTIyMDQ5\n"
	"MzJaGA8yMTI2MTAxOTIwNDkzMlowITEfMB0GA1UEAwwWRGFybGluZyBIb3N0IENB\n"
	"IFRlc3QgMTBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABM77Trz1vOzrwEwx7ug5\n"
	"IL6VYYyx3qRsMlXW1iZjLnA8wlqMtuSNj8ls9ppI/6w0B6BD5ujR7QoUFsjIv8tT\n"
	"EPGjYzBhMB0GA1UdDgQWBBSfgFSpwZ+2kpm1W7MjYye+vLiwdTAfBgNVHSMEGDAW\n"
	"gBSfgFSpwZ+2kpm1W7MjYye+vLiwdTAPBgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB\n"
	"/wQEAwIBBjAKBggqhkjOPQQDAgNJADBGAiEA2FiCLNZ0Lq/wx6XfdUxdIwbls5Uo\n"
	"+mwFSkACmknOb7cCIQDQWaqESP8VYv5f+cVN4BZddTT2+f1aTmSoJf7XdqvU1g==\n"
	"-----END CERTIFICATE-----\n"
;

static const char kCA2[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIBqTCCAU+gAwIBAgIUCJANdlKB7hYqqcN0scPK+FfvBWswCgYIKoZIzj0EAwIw\n"
	"ITEfMB0GA1UEAwwWRGFybGluZyBIb3N0IENBIFRlc3QgMjAgFw0yNjExMTIyMDQ5\n"
	"MzJaGA8yMTI2MTAxOTIwNDkzMlowITEfMB0GA1UEAwwWRGFybGluZyBIb3N0IENB\n"
	"IFRlc3QgMjBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABNOrM4OsF9CMN3ZL7Np1\n"
	"zCWhlRg0qzFx845lyvWZRiUfOhAL2fvwOe9n5p8jF3B5Uoz0Q5CQjqgIxXtaUfpL\n"
	"uLOjYzBhMB0GA1UdDgQWBBT8cdsQPXERZ12FNGG4/3LWCgp3DDAfBgNVHSMEGDAW\n"
	"gBT8cdsQPXERZ12FNGG4/3LWCgp3DDAPBgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB\n"
	"/wQEAwIBBjAKBggqhkjOPQQDAgNIADBFAiEAvMZIYYEjNQj2j73P40UpAMiC9K2S\n"
	"edM5NzM5F9ieto0CIAbTdG7DyOaYpMlBR0Cp4yFE+JL4NXZQ6rbqgBnpuI5j\n"
	"-----END CERTIFICATE-----\n"
;

static const char kCA3[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIBqDCCAU+gAwIBAgIUZEDymirFeIRmwmJ1OKnaU9kFULAwCgYIKoZIzj0EAwIw\n"
	"ITEfMB0GA1UEAwwWRGFybGluZyBIb3N0IENBIFRlc3QgMzAgFw0yNjExMTIyMDQ5\n"
	"MzJaGA8yMTI2MTAxOTIwNDkzMlowITEfMB0GA1UEAwwWRGFybGluZyBIb3N0IENB\n"
	"IFRlc3QgMzBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABCZllAt4hTDlL5Hpi+OR\n"
	"sWPtz7FyPe5OMCL/8mNbzCmAWceI5JcGZAAaGpu9a/26/f0bYvmNykjxRSobFSbF\n"
	"e7OjYzBhMB0GA1UdDgQWBBSwuuMjmLBzj53J6tMSWUcn2jTT4jAfBgNVHSMEGDAW\n"
	"gBSwuuMjmLBzj53J6tMSWUcn2jTT4jAPBgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB\n"
	"/wQEAwIBBjAKBggqhkjOPQQDAgNHADBEAiBSm9BBpPFfBq5wZ0GR4Xxm+2RjTZKM\n"
	"t/1rcoMQ763xYwIgeeVE0z76rjLObd+zN/zcLKytgeP+5iYE8gMb9yiRmEI=\n"
	"-----END CERTIFICATE-----\n"
;

static char g_tool[512] = "/usr/libexec/darling-hostcerts-import";
static char g_dir[256];
static char g_bundle[300], g_missing[300], g_shipped[300], g_output[300], g_keychain[300], g_state[300];
static char g_index[400], g_table[400];

// SHA-1 of the normalized subject content of each test CA (the index keys)
static const char* const kHash1 = "f670aecb3479643c2702b50cea1fee04200a0452";
static const char* const kHash2 = "4ccf02ad1333151b0f7f96b70e5b96e2c3eb61a4";
static const char* const kHash3 = "92584b5193267fa78c12474953fc03e599661059";

static void write_file(const char* path, const char* a, const char* b, const char* c)
{
	FILE* f = fopen(path, "w");

	if (f == NULL) {
		printf("FAIL: cannot write %s\n", path);
		g_failures++;
		return;
	}
	fputs(a, f);
	fputs(b, f);
	fputs(c, f);
	fclose(f);
}

static int run_tool(bool force, const char* bundle)
{
	char* argv[20];
	int n = 0, status = -1;
	pid_t pid;

	argv[n++] = g_tool;
	if (force)
		argv[n++] = "--force";
	argv[n++] = "--bundle";
	argv[n++] = (char*) bundle;
	argv[n++] = "--shipped";
	argv[n++] = g_shipped;
	argv[n++] = "--output";
	argv[n++] = g_output;
	argv[n++] = "--keychain";
	argv[n++] = g_keychain;
	argv[n++] = "--state";
	argv[n++] = g_state;
	argv[n] = NULL;

	if (posix_spawn(&pid, g_tool, NULL, NULL, argv, environ) != 0)
		return -1;
	if (waitpid(pid, &status, 0) != pid)
		return -1;
	return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

static unsigned char* read_all(const char* path, size_t* length)
{
	FILE* f = fopen(path, "rb");
	unsigned char* buffer;
	long size;

	*length = 0;
	if (f == NULL)
		return NULL;
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);
	buffer = malloc(size > 0 ? size : 1);
	if (buffer != NULL && fread(buffer, 1, size, f) == (size_t) size)
		*length = size;
	else {
		free(buffer);
		buffer = NULL;
	}
	fclose(f);
	return buffer;
}

static uint32_t le32(const unsigned char* p)
{
	return p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t) p[3] << 24);
}

typedef struct {
	char hash[41];
	char name[64];
} anchor_info;

// Decodes the output files. Returns the number of anchors (-1: files missing
// or malformed), checking the layout on the way.
static int read_anchors(anchor_info* out, int max)
{
	size_t indexLength, tableLength;
	unsigned char* index = read_all(g_index, &indexLength);
	unsigned char* table = read_all(g_table, &tableLength);
	int count = -1;

	if (index == NULL || table == NULL || indexLength % 24 != 0)
		goto done;
	count = 0;
	for (size_t i = 0; i < indexLength; i += 24) {
		uint32_t offset = le32(index + i + 20);
		uint32_t recordLength, derLength;
		SecCertificateRef cert;
		CFDataRef der;
		CFStringRef summary;

		if (count >= max || (size_t) offset + 8 > tableLength) {
			count = -1;
			goto done;
		}
		recordLength = le32(table + offset);
		derLength = le32(table + offset + 4);
		if (recordLength != (derLength + 7) / 8 * 8 + 8 || (size_t) offset + 8 + (recordLength - 8) > tableLength) {
			printf("FAIL: bad record lengths %u %u\n", recordLength, derLength);
			g_failures++;
			count = -1;
			goto done;
		}
		for (uint32_t j = derLength; j < recordLength - 8; j++)
			CHECK(table[offset + 8 + j] == 0xff);

		for (int k = 0; k < 20; k++)
			snprintf(out[count].hash + 2 * k, 3, "%02x", index[i + k]);
		der = CFDataCreate(NULL, table + offset + 8, derLength);
		cert = SecCertificateCreateWithData(NULL, der);
		CFRelease(der);
		out[count].name[0] = '\0';
		if (cert != NULL) {
			summary = SecCertificateCopySubjectSummary(cert);
			if (summary != NULL) {
				CFStringGetCString(summary, out[count].name, sizeof(out[count].name), kCFStringEncodingUTF8);
				CFRelease(summary);
			}
			CFRelease(cert);
		}
		count++;
	}

done:
	free(index);
	free(table);
	return count;
}

static bool has_anchor(const anchor_info* a, int n, const char* name, const char* hash)
{
	for (int i = 0; i < n; i++)
		if (strcmp(a[i].name, name) == 0 && strcmp(a[i].hash, hash) == 0)
			return true;
	return false;
}

static time_t mtime_of(const char* path)
{
	struct stat st;

	return stat(path, &st) == 0 ? st.st_mtime : 0;
}

int main(int argc, char** argv)
{
	anchor_info a[16];
	int n;
	time_t indexTime, stateTime;
	SecKeychainRef keychain = NULL;
	static const char junk[] = "# comment line\nsome text\n-----BEGIN CERTIFICATE-----\n!!not base64!!\n-----END CERTIFICATE-----\n";

	if (argc > 1)
		snprintf(g_tool, sizeof(g_tool), "%s", argv[1]);

	snprintf(g_dir, sizeof(g_dir), "/tmp/hostcerts-test.XXXXXX");
	if (mkdtemp(g_dir) == NULL) {
		printf("FAIL: mkdtemp\n");
		return 1;
	}
	snprintf(g_bundle, sizeof(g_bundle), "%s/bundle.pem", g_dir);
	snprintf(g_missing, sizeof(g_missing), "%s/no-such-bundle.pem", g_dir);
	snprintf(g_shipped, sizeof(g_shipped), "%s/shipped", g_dir);
	snprintf(g_output, sizeof(g_output), "%s/output", g_dir);
	snprintf(g_keychain, sizeof(g_keychain), "%s/test.keychain", g_dir);
	snprintf(g_state, sizeof(g_state), "%s/state.plist", g_dir);
	snprintf(g_index, sizeof(g_index), "%s/certsIndex.data", g_output);
	snprintf(g_table, sizeof(g_table), "%s/certsTable.data", g_output);
	mkdir(g_shipped, 0755);
	mkdir(g_output, 0755);

	// the shipped list holds CA 3 (a file with another suffix is not read)
	{
		char path[400];

		snprintf(path, sizeof(path), "%s/ca3.crt", g_shipped);
		write_file(path, kCA3, "", "");
		snprintf(path, sizeof(path), "%s/ca1.pem", g_shipped);
		write_file(path, kCA1, "", "");
	}

	// 1. no host bundle and no earlier replacement: the shipped files are left
	// alone (nothing is written), and the System keychain's stand-in is created
	// with an empty password
	CHECK(run_tool(false, g_missing) == 0);
	CHECK(stat(g_index, &(struct stat){0}) != 0);
	CHECK(stat(g_state, &(struct stat){0}) != 0);
	CHECK(SecKeychainOpen(g_keychain, &keychain) == errSecSuccess && keychain != NULL);
	CHECK(stat(g_keychain, &(struct stat){0}) == 0);
	CHECK(SecKeychainUnlock(keychain, 0, "", true) == errSecSuccess);

	// 2. the host bundle (with text outside the markers and an invalid block)
	// becomes the anchor set: CA 1 and CA 2, not the shipped CA 3
	write_file(g_bundle, kCA1, junk, kCA2);
	CHECK(run_tool(false, g_bundle) == 0);
	n = read_anchors(a, 16);
	CHECK(n == 2);
	CHECK(has_anchor(a, n, "Darling Host CA Test 1", kHash1));
	CHECK(has_anchor(a, n, "Darling Host CA Test 2", kHash2));
	CHECK(!has_anchor(a, n, "Darling Host CA Test 3", kHash3));
	CHECK(mtime_of(g_state) != 0);

	// 3. an unchanged bundle writes nothing
	indexTime = mtime_of(g_index);
	stateTime = mtime_of(g_state);
	sleep(1);
	CHECK(run_tool(false, g_bundle) == 0);
	CHECK(mtime_of(g_index) == indexTime && mtime_of(g_state) == stateTime);

	// 4. --force writes again, with the same content
	CHECK(run_tool(true, g_bundle) == 0);
	CHECK(mtime_of(g_index) > indexTime);
	CHECK(read_anchors(a, 16) == 2);

	// 5. a changed bundle is picked up: CA 1 is gone, and a duplicate of CA 2
	// does not add a second record
	write_file(g_bundle, kCA2, kCA2, "");
	CHECK(run_tool(false, g_bundle) == 0);
	n = read_anchors(a, 16);
	CHECK(n == 1);
	CHECK(has_anchor(a, n, "Darling Host CA Test 2", kHash2));

	// 6. the host bundle disappears: the shipped anchors (CA 3 only) are restored
	CHECK(run_tool(false, g_missing) == 0);
	n = read_anchors(a, 16);
	CHECK(n == 1);
	CHECK(has_anchor(a, n, "Darling Host CA Test 3", kHash3));

	// 7. a bundle without certificates counts as no host bundle; the shipped
	// list stays, and the unchanged files are not rewritten
	write_file(g_bundle, "nothing to see\n", "", "");
	indexTime = mtime_of(g_index);
	sleep(1);
	CHECK(run_tool(false, g_bundle) == 0);
	CHECK(read_anchors(a, 16) == 1);
	CHECK(mtime_of(g_index) == indexTime);

	// 8. the host bundle returns
	write_file(g_bundle, kCA1, "", "");
	CHECK(run_tool(false, g_bundle) == 0);
	n = read_anchors(a, 16);
	CHECK(n == 1);
	CHECK(has_anchor(a, n, "Darling Host CA Test 1", kHash1));

	// 9. an option the tool does not know is a usage error, not a crash
	{
		char* bad[] = { g_tool, "--no-such-option", NULL };
		pid_t pid;
		int st = 0;

		CHECK(posix_spawn(&pid, g_tool, NULL, NULL, bad, environ) == 0);
		waitpid(pid, &st, 0);
		CHECK(WIFEXITED(st) && WEXITSTATUS(st) != 0);
	}

	SecKeychainDelete(keychain);
	CFRelease(keychain);
	{
		char path[400];

		snprintf(path, sizeof(path), "%s/ca3.crt", g_shipped);
		unlink(path);
		snprintf(path, sizeof(path), "%s/ca1.pem", g_shipped);
		unlink(path);
	}
	unlink(g_index);
	unlink(g_table);
	unlink(g_bundle);
	unlink(g_keychain);
	unlink(g_state);
	rmdir(g_shipped);
	rmdir(g_output);
	rmdir(g_dir);

	if (g_failures == 0)
		printf("PASSED\n");
	return g_failures != 0;
}
