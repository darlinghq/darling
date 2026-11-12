// Copyright (C) 2026 Lubos Dolezel
//
// CGImageSourceCreateWithURL returns NULL for a file that
// does not exist or is a directory; it does not trap.
// Passes unchanged on macOS 27.0.1 (arm64 and x86_64).
#include <ApplicationServices/ApplicationServices.h>
#include <ImageIO/ImageIO.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static CFURLRef url_for(const char* path, bool dir)
{
	return CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8*) path, (CFIndex) strlen(path), dir);
}

int main(void)
{
	char dir[] = "/tmp/imageio-missing-XXXXXX";
	CHECK(mkdtemp(dir) != NULL);
	char path[256];

	snprintf(path, sizeof(path), "%s/missing.png", dir);
	CFURLRef missing = url_for(path, false);
	CGImageSourceRef source = CGImageSourceCreateWithURL(missing, NULL);
	CHECK(source == NULL);
	if (source)
		CFRelease(source);
	CFRelease(missing);

	// A directory is not an image.
	CFURLRef directory = url_for(dir, true);
	source = CGImageSourceCreateWithURL(directory, NULL);
	CHECK(source == NULL);
	if (source)
		CFRelease(source);
	CFRelease(directory);

	snprintf(path, sizeof(path), "rm -rf %s", dir);
	CHECK(system(path) == 0);

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
