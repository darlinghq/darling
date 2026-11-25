// Copyright (C) 2026 Lubos Dolezel
//
// The resource file of an application bundle is open when the application starts: GetResource() finds the resources of
// the 'name.rsrc' file in the bundle's Resources directory, named after the executable or the bundle's name, without
// the program opening it. A program with no such file starts with no resource file open.
//
// The test makes bundles in a temporary directory, with a copy of its own executable in them, and runs the copies.
#include <CoreServices/CoreServices.h>
#include <fcntl.h>
#include <mach-o/dyld.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

// A resource file (resource data in the data fork, as the Rez tool's -useDF writes it) with two 'TEST' resources:
// 128 "hello" and 129 "world"
static const unsigned char kResourceFile[] = {
	0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x12, 0x00, 0x00, 0x00, 0x12,
	0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x68, 0x65, 0x6c, 0x6c,
	0x6f, 0x00, 0x00, 0x00, 0x05, 0x77, 0x6f, 0x72, 0x6c, 0x64, 0x00, 0x00,
	0x01, 0x00, 0x00, 0x00, 0x01, 0x12, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00,
	0x00, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c,
	0x00, 0x3e, 0x00, 0x00, 0x54, 0x45, 0x53, 0x54, 0x00, 0x01, 0x00, 0x0a,
	0x00, 0x80, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x81, 0xff, 0xff, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00
};

static void writeFile(const char* path, const void* data, size_t size, mode_t mode)
{
	int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, mode);

	if (fd < 0 || write(fd, data, size) != (ssize_t) size)
		printf("FAIL: cannot write %s\n", path), g_failures++;
	if (fd >= 0)
		close(fd);
}

static int runChild(const char* executable, const char* mode)
{
	pid_t pid = fork();
	int status = 0;

	if (pid == 0)
	{
		execl(executable, executable, mode, (char*) NULL);
		_exit(127);
	}
	if (pid < 0 || waitpid(pid, &status, 0) != pid)
		return -1;
	return WIFEXITED(status) ? WEXITSTATUS(status) : -2;
}

// a bundle with the given executable name and bundle name, and the resource file with the given name (none if NULL)
static void makeBundle(const char* root, const char* dir, const char* execName, const char* bundleName, const char* resourceFileName, char* outExecutable, size_t outSize)
{
	char path[2048], self[2048];
	uint32_t selfSize = sizeof(self);
	struct stat st;

	snprintf(path, sizeof(path), "%s/%s", root, dir);
	mkdir(path, 0755);
	snprintf(path, sizeof(path), "%s/%s/%s.app", root, dir, execName);
	mkdir(path, 0755);
	snprintf(path, sizeof(path), "%s/%s/%s.app/Contents", root, dir, execName);
	mkdir(path, 0755);
	snprintf(path, sizeof(path), "%s/%s/%s.app/Contents/MacOS", root, dir, execName);
	mkdir(path, 0755);
	snprintf(path, sizeof(path), "%s/%s/%s.app/Contents/Resources", root, dir, execName);
	mkdir(path, 0755);

	char plist[1024];

	snprintf(plist, sizeof(plist), "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
		"<plist version=\"1.0\"><dict><key>CFBundleExecutable</key><string>%s</string><key>CFBundleName</key><string>%s</string>"
		"<key>CFBundleIdentifier</key><string>test.resourcefile.%s</string><key>CFBundlePackageType</key><string>APPL</string></dict></plist>\n", execName, bundleName, dir);
	snprintf(path, sizeof(path), "%s/%s/%s.app/Contents/Info.plist", root, dir, execName);
	writeFile(path, plist, strlen(plist), 0644);
	if (resourceFileName)
	{
		snprintf(path, sizeof(path), "%s/%s/%s.app/Contents/Resources/%s", root, dir, execName, resourceFileName);
		writeFile(path, kResourceFile, sizeof(kResourceFile), 0644);
	}

	// a copy of this executable
	_NSGetExecutablePath(self, &selfSize);
	snprintf(outExecutable, outSize, "%s/%s/%s.app/Contents/MacOS/%s", root, dir, execName, execName);
	if (stat(self, &st) == 0)
	{
		FILE* in = fopen(self, "rb");
		unsigned char* bytes = malloc(st.st_size);

		if (in && bytes && fread(bytes, 1, st.st_size, in) == (size_t) st.st_size)
			writeFile(outExecutable, bytes, st.st_size, 0755);
		else
			printf("FAIL: cannot copy the executable\n"), g_failures++;
		if (in)
			fclose(in);
		free(bytes);
	}
}

// Child: with "file" the resource file is open and holds the two resources; with "none" no file is open
static int child(const char* mode)
{
	Handle hello = GetResource('TEST', 128);

	if (strcmp(mode, "file") == 0)
	{
		CHECK(CurResFile() != kResFileNotOpened);
		CHECK(hello != NULL);
		if (hello)
		{
			CHECK(GetHandleSize(hello) >= 5);
			CHECK(memcmp(*hello, "hello", 5) == 0);
		}
		Handle world = Get1Resource('TEST', 129);

		CHECK(world != NULL);
		CHECK(Count1Resources('TEST') == 2);
		CHECK(CountResources('TEST') == 2);
		CHECK(GetResource('TEST', 4711) == NULL);
		CHECK(ResError() == resNotFound);
	}
	else
	{
		CHECK(hello == NULL);
		CHECK(CurResFile() == kResFileNotOpened);
	}
	return g_failures != 0;
}

int main(int argc, char** argv)
{
	if (argc > 1)
		return child(argv[1]);

	char root[] = "/tmp/carboncore-app-resource-file.XXXXXX";
	char executable[2048];

	if (!mkdtemp(root))
		return 1;
	// named after the executable
	makeBundle(root, "a", "resprobea", "Other Name A", "resprobea.rsrc", executable, sizeof(executable));
	CHECK(runChild(executable, "file") == 0);
	// named after the bundle's name
	makeBundle(root, "b", "resprobeb", "Bundle Name B", "Bundle Name B.rsrc", executable, sizeof(executable));
	CHECK(runChild(executable, "file") == 0);
	// a file with another name is not the application's
	makeBundle(root, "c", "resprobec", "Bundle Name C", "unrelated.rsrc", executable, sizeof(executable));
	CHECK(runChild(executable, "none") == 0);
	// no file
	makeBundle(root, "d", "resprobed", "Bundle Name D", NULL, executable, sizeof(executable));
	CHECK(runChild(executable, "none") == 0);

	printf(g_failures ? "FAIL carboncore_app_resource_file\n" : "PASS carboncore_app_resource_file\n");
	return g_failures != 0;
}
