// Copyright (C) 2026 fr4nck
//
// FSOpenResFile opens the resource fork, like FSpOpenResFile: the file becomes the current resource file and the result
// is left in ResError(); a file without a resource fork is not opened as one, and closing it gives the current resource
// file back to the previous one. (Resource Manager: FSOpenResFile, FSpOpenResFile, CloseResFile.)
#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/xattr.h>
#include <unistd.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void put32(unsigned char *p, unsigned v) { p[0] = v >> 24; p[1] = v >> 16; p[2] = v >> 8; p[3] = v; }
static void put16(unsigned char *p, unsigned v) { p[0] = v >> 8; p[1] = v; }

// A resource file with one 'TEST' 128 resource holding `text`: header, 240 reserved bytes, data, map.
static size_t resource_file(unsigned char *out, const char *text)
{
	size_t n = strlen(text), dataLen = 4 + n, mapOff = 256 + dataLen, mapLen = 16 + 4 + 2 + 2 + 2 + 2 + 2 + 8 + 12;
	memset(out, 0, mapOff + mapLen);
	put32(out, 256); put32(out + 4, mapOff); put32(out + 8, dataLen); put32(out + 12, mapLen);
	put32(out + 256, n); memcpy(out + 260, text, n);
	unsigned char *m = out + mapOff;
	memcpy(m, out, 16);                      // copy of the header
	put16(m + 24, 28); put16(m + 26, mapLen); // type list offset, name list offset (empty)
	put16(m + 28, 0);                         // one type
	memcpy(m + 30, "TEST", 4); put16(m + 34, 0); put16(m + 36, 2 + 8);
	put16(m + 38, 128); put16(m + 40, 0xFFFF); // ID, no name; attributes 0, data offset 0
	return mapOff + mapLen;
}

static int text_is(Handle h, const char *want)
{
	size_t n = strlen(want);
	return h && *h && (size_t) GetHandleSize(h) == n && memcmp(*h, want, n) == 0;
}

int main(void)
{
	char dir[] = "/tmp/fsopenresfile.XXXXXX";
	if (!mkdtemp(dir)) return 1;
	char withFork[256], dataFork[256];
	snprintf(withFork, sizeof(withFork), "%s/with-resource-fork", dir);
	snprintf(dataFork, sizeof(dataFork), "%s/data-fork-resources.rsrc", dir);
	unsigned char file[1024];

	FILE *f = fopen(withFork, "wb");
	fputs("a data fork that is not a resource map", f); fclose(f);
	size_t len = resource_file(file, "from the resource fork");
	CHECK(setxattr(withFork, "com.apple.ResourceFork", file, len, 0, 0) == 0);
	f = fopen(dataFork, "wb");
	len = resource_file(file, "from the data fork");
	fwrite(file, 1, len, f); fclose(f);

	FSRef a, b;
	CHECK(FSPathMakeRef((const UInt8 *) withFork, &a, NULL) == noErr);
	CHECK(FSPathMakeRef((const UInt8 *) dataFork, &b, NULL) == noErr);

	// The resource fork is opened and becomes the current resource file.
	ResFileRefNum before = CurResFile();
	ResFileRefNum ra = FSOpenResFile(&a, fsRdPerm);
	CHECK(ra > 0);
	CHECK(ResError() == noErr);
	CHECK(CurResFile() == ra);
	CHECK(text_is(Get1Resource('TEST', 128), "from the resource fork"));
	if (ra > 0) CloseResFile(ra);
	CHECK(ResError() == noErr);
	CHECK(CurResFile() == before);

	// A data-fork resource file has no resource fork: not opened by FSOpenResFile, an error is reported; it is opened
	// with FSOpenResourceFile and the data fork name.
	ResFileRefNum rb = FSOpenResFile(&b, fsRdPerm);
	CHECK(rb == kResFileNotOpened);
	CHECK(ResError() != noErr);
	HFSUniStr255 dataForkName;
	FSGetDataForkName(&dataForkName);
	ResFileRefNum rd = kResFileNotOpened;
	CHECK(FSOpenResourceFile(&b, dataForkName.length, dataForkName.unicode, fsRdPerm, &rd) == noErr);
	UseResFile(rd);
	CHECK(text_is(Get1Resource('TEST', 128), "from the data fork"));
	if (rd > 0) CloseResFile(rd);

	unlink(withFork); unlink(dataFork); rmdir(dir);
	printf(g_failures ? "FAILED: %d\n" : "PASSED\n", g_failures);
	return g_failures != 0;
}
