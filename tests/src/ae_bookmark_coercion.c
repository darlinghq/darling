// Copyright (C) 2026 Lubos Dolezel
// ae_bookmark_coercion: bookmark data ('bmrk') coerces to a file URL (typeFileURL): with AECoerceDesc, and when a
// list item is read as typeFileURL with AEGetNthPtr. A file gives a file URL, a directory a directory URL. Prints
// "FAIL ..." for each check that does not hold, else "PASS". Link with -framework CoreServices. Also passes on
// macOS 26.7.3.
#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int failures;

static void expect(int ok, const char *what)
{
	if (!ok)
	{
		printf("FAIL %s\n", what);
		failures++;
	}
}

static void coerce(const char *path, Boolean directory, const char *label)
{
	char expected[PATH_MAX + 16];
	char got[PATH_MAX + 16];
	CFURLRef url = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8 *) path, strlen(path), directory);
	CFDataRef bookmark = CFURLCreateBookmarkData(NULL, url, 0, NULL, NULL, NULL);
	AEDesc item, result, list;
	AEKeyword keyword;
	DescType type;
	Size size = 0;

	CFStringGetCString(CFURLGetString(url), expected, sizeof(expected), kCFStringEncodingUTF8);
	AECreateDesc(typeBookmarkData, CFDataGetBytePtr(bookmark), CFDataGetLength(bookmark), &item);

	memset(got, 0, sizeof(got));
	OSErr err = AECoerceDesc(&item, typeFileURL, &result);
	if (err == noErr)
		AEGetDescData(&result, got, sizeof(got) - 1);
	printf("%s: AECoerceDesc %d %s\n", label, err, got);
	expect(err == noErr && result.descriptorType == typeFileURL && strcmp(got, expected) == 0, label);
	AEDisposeDesc(&result);

	memset(got, 0, sizeof(got));
	AECreateList(NULL, 0, false, &list);
	AEPutDesc(&list, 0, &item);
	err = AEGetNthPtr(&list, 1, typeFileURL, &keyword, &type, got, sizeof(got) - 1, &size);
	expect(err == noErr && type == typeFileURL && strcmp(got, expected) == 0, label);
	AEDisposeDesc(&list);
	AEDisposeDesc(&item);
	CFRelease(bookmark);
	CFRelease(url);
}

int main(void)
{
	char template[] = "/tmp/ae-bookmark.XXXXXX";
	char dir[PATH_MAX], file[PATH_MAX + 8];

	if (mkdtemp(template) == NULL || realpath(template, dir) == NULL)
		return 2;
	snprintf(file, sizeof(file), "%s/a.txt", dir);
	fclose(fopen(file, "w"));

	coerce(file, false, "a file");
	coerce(dir, true, "a directory");

	unlink(file);
	rmdir(dir);
	if (failures)
		return 1;
	printf("PASS\n");
	return 0;
}
