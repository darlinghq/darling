// Copyright (C) 2026 Darling Developers
//
// Opening an application bundle through LaunchServices runs the bundle's
// CFBundleExecutable, also when the bundle path needs percent-encoding
// as a URL (here: a space).
#include <CoreServices/CoreServices.h>
#include <mach-o/dyld.h>
#include <crt_externs.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <spawn.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define EXEC_NAME "launched"

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static bool endsWith(const char* str, const char* suffix)
{
	size_t len = strlen(str), slen = strlen(suffix);
	return len >= slen && strcmp(str + len - slen, suffix) == 0;
}

static bool copyFile(const char* from, const char* to)
{
	char buf[65536];
	ssize_t rd;
	bool ok = true;

	int in = open(from, O_RDONLY);
	if (in < 0)
		return false;
	int out = open(to, O_WRONLY | O_CREAT | O_TRUNC, 0755);
	if (out < 0)
	{
		close(in);
		return false;
	}

	while ((rd = read(in, buf, sizeof(buf))) > 0)
	{
		if (write(out, buf, rd) != rd)
		{
			ok = false;
			break;
		}
	}

	close(in);
	close(out);
	return ok && rd == 0;
}

static bool writeInfoPlist(const char* path, const char* name)
{
	FILE* f = fopen(path, "w");
	if (!f)
		return false;

	fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
		"<plist version=\"1.0\">\n<dict>\n"
		"\t<key>CFBundleExecutable</key>\n\t<string>" EXEC_NAME "</string>\n"
		"\t<key>CFBundleIdentifier</key>\n\t<string>org.darlinghq.tests.launch-app</string>\n"
		"\t<key>CFBundleName</key>\n\t<string>%s</string>\n"
		"\t<key>CFBundlePackageType</key>\n\t<string>APPL</string>\n"
		"</dict>\n</plist>\n", name);

	return fclose(f) == 0;
}

static CFURLRef dirURL(const char* path)
{
	return CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8*) path, strlen(path), true);
}

// open(1) takes the same route
static bool openTool(CFURLRef url)
{
	char str[PATH_MAX * 3];
	if (!CFStringGetCString(CFURLGetString(url), str, sizeof(str), kCFStringEncodingUTF8))
		return false;

	char* args[] = { "open", str, NULL };
	pid_t pid;
	if (posix_spawn(&pid, "/usr/bin/open", NULL, NULL, args, *_NSGetEnviron()) != 0)
		return false;

	int status = 0;
	return waitpid(pid, &status, 0) == pid && WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

// The launched process leaves a marker next to its executable
static bool waitForMarker(const char* marker)
{
	for (int i = 0; i < 200; i++)
	{
		if (access(marker, F_OK) == 0)
			return true;
		usleep(50000);
	}
	return false;
}

int main(int argc, const char** argv)
{
	char self[PATH_MAX];
	uint32_t size = sizeof(self);

	if (_NSGetExecutablePath(self, &size) != 0)
		return 1;

	// Launched from the bundle
	if (endsWith(argv[0], "/Contents/MacOS/" EXEC_NAME))
	{
		char marker[PATH_MAX];
		snprintf(marker, sizeof(marker), "%s.ran", argv[0]);
		int fd = open(marker, O_WRONLY | O_CREAT, 0644);
		if (fd >= 0)
			close(fd);
		return fd >= 0 ? 0 : 1;
	}

	char tmp[] = "/tmp/lslaunch.XXXXXX";
	if (!mkdtemp(tmp))
		return 1;

	char bundle[PATH_MAX], contents[PATH_MAX], macos[PATH_MAX], exec[PATH_MAX], plist[PATH_MAX], marker[PATH_MAX], name[64];
	snprintf(bundle, sizeof(bundle), "%s/Launch Test.app", tmp);
	snprintf(contents, sizeof(contents), "%s/Contents", bundle);
	snprintf(macos, sizeof(macos), "%s/MacOS", contents);
	snprintf(exec, sizeof(exec), "%s/" EXEC_NAME, macos);
	snprintf(plist, sizeof(plist), "%s/Info.plist", contents);
	snprintf(marker, sizeof(marker), "%s.ran", exec);
	snprintf(name, sizeof(name), "Launch Test %d", (int) getpid());

	CHECK(mkdir(bundle, 0755) == 0);
	CHECK(mkdir(contents, 0755) == 0);
	CHECK(mkdir(macos, 0755) == 0);
	CHECK(copyFile(self, exec));

	CFURLRef url = dirURL(bundle);

	// Without an Info.plist there is no executable to run
	CHECK(LSOpenCFURLRef(url, NULL) != noErr);

	CHECK(writeInfoPlist(plist, name));

	// LSOpenCFURLRef
	CHECK(LSOpenCFURLRef(url, NULL) == noErr);
	CHECK(waitForMarker(marker));
	unlink(marker);

	// LSOpenURLsWithRole with default parameters
	CFArrayRef urls = CFArrayCreate(NULL, (const void**) &url, 1, &kCFTypeArrayCallBacks);
	CHECK(LSOpenURLsWithRole(urls, kLSRolesAll, NULL, NULL, NULL, 0) == noErr);
	CHECK(waitForMarker(marker));
	unlink(marker);
	CFRelease(urls);

	// open(1) with the bundle's file URL (percent-encoded)
	CHECK(openTool(url));
	CHECK(waitForMarker(marker));
	unlink(marker);

	// Nothing to open
	CFArrayRef empty = CFArrayCreate(NULL, NULL, 0, &kCFTypeArrayCallBacks);
	CHECK(LSOpenURLsWithRole(empty, kLSRolesAll, NULL, NULL, NULL, 0) != noErr);
	CFRelease(empty);

	CFRelease(url);

	unlink(plist);
	unlink(exec);
	rmdir(macos);
	rmdir(contents);
	rmdir(bundle);
	rmdir(tmp);

	if (g_failures == 0)
		puts("PASS");
	return g_failures != 0;
}
