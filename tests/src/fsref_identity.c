// Copyright (C) 2026 Lubos Dolezel
//
// FSRefs identify a file: every reference to one file is equivalent, in any process.
#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void touch(const char* path)
{
	FILE* f = fopen(path, "w");
	CHECK(f != NULL);
	if (f)
		fclose(f);
}

static bool resolvesTo(const FSRef* ref, const char* expected)
{
	UInt8 path[4096];

	if (FSRefMakePath(ref, path, sizeof(path)) != noErr)
		return false;

	char real1[PATH_MAX], real2[PATH_MAX];
	return realpath((const char*) path, real1) && realpath(expected, real2) && strcmp(real1, real2) == 0;
}

// In a separate process image, so that nothing in this process can help
static void checkInNewProcess(const char* self, const FSRef* ref, const char* expected)
{
	char hex[sizeof(FSRef) * 2 + 1];

	for (size_t i = 0; i < sizeof(FSRef); i++)
		sprintf(hex + i * 2, "%02x", ((const UInt8*) ref)[i]);

	pid_t pid = fork();
	if (pid == 0)
	{
		execl(self, self, "--check", hex, expected, NULL);
		_exit(127);
	}

	int status = 0;
	CHECK(waitpid(pid, &status, 0) == pid);
	CHECK(WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

static int checkMain(const char* hex, const char* expected)
{
	FSRef ref, own;

	for (size_t i = 0; i < sizeof(FSRef); i++)
	{
		unsigned value;
		sscanf(hex + i * 2, "%02x", &value);
		((UInt8*) &ref)[i] = (UInt8) value;
	}

	if (!resolvesTo(&ref, expected))
		return 1;
	if (FSPathMakeRef((const UInt8*) expected, &own, NULL) != noErr)
		return 2;
	if (memcmp(&ref, &own, sizeof(ref)) != 0)
		return 3;
	return FSCompareFSRefs(&ref, &own) == noErr ? 0 : 4;
}

static void testDirectory(const char* self, const char* dir)
{
	char a[1024], b[1024], soft[1024], hard[1024], moved[1024];
	FSRef refA, refA2, refB, refLink, refLinkTarget, refHard, refDir;

	snprintf(a, sizeof(a), "%s/a", dir);
	snprintf(b, sizeof(b), "%s/b", dir);
	snprintf(soft, sizeof(soft), "%s/link", dir);
	snprintf(hard, sizeof(hard), "%s/hard", dir);
	snprintf(moved, sizeof(moved), "%s/moved", dir);
	touch(a);
	touch(b);
	CHECK(symlink(a, soft) == 0);
	CHECK(link(a, hard) == 0);

	CHECK(FSPathMakeRef((const UInt8*) a, &refA, NULL) == noErr);
	CHECK(FSPathMakeRef((const UInt8*) a, &refA2, NULL) == noErr);
	CHECK(FSPathMakeRef((const UInt8*) b, &refB, NULL) == noErr);

	// The same file
	CHECK(memcmp(&refA, &refA2, sizeof(FSRef)) == 0);
	CHECK(FSCompareFSRefs(&refA, &refA2) == noErr);
	CHECK(resolvesTo(&refA, a));

	// Another file on the same volume
	CHECK(FSCompareFSRefs(&refA, &refB) == errFSRefsDifferent);
	CHECK(resolvesTo(&refB, b));

	// Another name for the same file
	CHECK(FSPathMakeRef((const UInt8*) hard, &refHard, NULL) == noErr);
	CHECK(FSCompareFSRefs(&refA, &refHard) == noErr);

	// The link itself is not the file it points to, unless asked to follow it
	Boolean isDirectory = true;
	CHECK(FSPathMakeRef((const UInt8*) soft, &refLink, &isDirectory) == noErr);
	CHECK(!isDirectory);
	CHECK(FSCompareFSRefs(&refA, &refLink) == errFSRefsDifferent);
	CHECK(FSPathMakeRefWithOptions((const UInt8*) soft, 0, &refLinkTarget, NULL) == noErr);
	CHECK(FSCompareFSRefs(&refA, &refLinkTarget) == noErr);

	CHECK(FSPathMakeRef((const UInt8*) dir, &refDir, &isDirectory) == noErr);
	CHECK(isDirectory);
	CHECK(resolvesTo(&refDir, dir));

	// A relative path must still work from anywhere else
	char cwd[PATH_MAX];
	FSRef refRelative;
	CHECK(getcwd(cwd, sizeof(cwd)) != NULL);
	CHECK(chdir(dir) == 0);
	CHECK(FSPathMakeRef((const UInt8*) "b", &refRelative, NULL) == noErr);
	CHECK(chdir("/") == 0);
	CHECK(FSCompareFSRefs(&refRelative, &refB) == noErr);
	CHECK(resolvesTo(&refRelative, b));
	CHECK(chdir(cwd) == 0);

	checkInNewProcess(self, &refB, b);

	// Many references later, the first still works
	for (int i = 0; i < 300; i++)
	{
		char other[1024];
		FSRef refOther;
		snprintf(other, sizeof(other), "%s/other%d", dir, i);
		touch(other);
		CHECK(FSPathMakeRef((const UInt8*) other, &refOther, NULL) == noErr);
		unlink(other);
	}
	CHECK(resolvesTo(&refB, b));

	// Taking a new reference after a rename finds the file again
	CHECK(rename(b, moved) == 0);
	CHECK(FSPathMakeRef((const UInt8*) moved, &refRelative, NULL) == noErr);
	CHECK(memcmp(&refRelative, &refB, sizeof(FSRef)) == 0);
	CHECK(resolvesTo(&refB, moved));

	// A reference must never lead to a different file that took over the path
	unlink(moved);
	FILE* keep = fopen(moved, "w");
	CHECK(keep != NULL);
	UInt8 path[4096];
	struct stat st;
	if (stat(moved, &st) == 0 && st.st_ino != 0)
	{
		FSRef refNew;
		CHECK(FSPathMakeRef((const UInt8*) moved, &refNew, NULL) == noErr);
		if (memcmp(&refNew, &refB, sizeof(FSRef)) != 0)
			CHECK(FSRefMakePath(&refB, path, sizeof(path)) != noErr);
	}
	if (keep)
		fclose(keep);

	unlink(a);
	unlink(hard);
	unlink(soft);
	unlink(moved);
}

int main(int argc, const char** argv)
{
	if (argc == 4 && strcmp(argv[1], "--check") == 0)
		return checkMain(argv[2], argv[3]);

	FSRef ref;
	CHECK(FSPathMakeRef((const UInt8*) "/does/not/exist", &ref, NULL) == fnfErr);
	CHECK(FSPathMakeRef(NULL, &ref, NULL) == paramErr);
	CHECK(FSCompareFSRefs(NULL, &ref) == paramErr);
	CHECK(FSCompareFSRefs(&ref, NULL) == paramErr);

	// Not something that came from FSPathMakeRef
	FSRef zero, root;
	memset(&zero, 0, sizeof(zero));
	CHECK(FSPathMakeRef((const UInt8*) "/", &root, NULL) == noErr);
	CHECK(FSCompareFSRefs(&zero, &root) == fnfErr);

	// Anything under the prefix is on overlayfs; further directories can be given to test others
	char dir[] = "/tmp/fsref-test-XXXXXX";
	CHECK(mkdtemp(dir) != NULL);
	testDirectory(argv[0], dir);
	rmdir(dir);

	for (int i = 1; i < argc; i++)
	{
		char other[1024];
		snprintf(other, sizeof(other), "%s/fsref-test-XXXXXX", argv[i]);
		CHECK(mkdtemp(other) != NULL);
		testDirectory(argv[0], other);
		rmdir(other);
	}

	if (g_failures)
	{
		printf("fsref_identity: %d failure(s)\n", g_failures);
		return 1;
	}

	printf("fsref_identity: OK\n");
	return 0;
}
