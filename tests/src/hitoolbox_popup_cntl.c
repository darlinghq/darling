// Copyright (C) 2026 fr4nck
//
// Pop-up menu controls made by NewControl (procID 1008, popupMenuProc, and 400, kControlPopupButtonProc) and controls
// read from 'CNTL' resources by GetNewControl: the minimum names the menu, which is read with GetMenu and put in the
// hierarchical part of the menu list when it is not there yet; the control's value is then the chosen item (1) and
// its maximum the number of items; the bounds, visibility and reference constant come from the resource.
// (Control Manager: NewControl, GetNewControl, pop-up menu control.)
#include <Carbon/Carbon.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __LP64__
int main(void)
{
	printf("PASSED (pop-up menu controls: 32-bit only)\n");
	return 0;
}
#else

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void put32(unsigned char *p, unsigned v) { p[0] = v >> 24; p[1] = v >> 16; p[2] = v >> 8; p[3] = v; }
static void put16(unsigned char *p, unsigned v) { p[0] = v >> 8; p[1] = v; }

struct Res { const char *type; short id; const unsigned char *data; size_t len; };

// A resource file (resources in the data fork) holding the given resources, one per type.
static size_t resource_file(unsigned char *out, const struct Res *res, int count)
{
	size_t data = 256, p = data;
	unsigned offsets[8];
	for (int i = 0; i < count; i++) {
		offsets[i] = (unsigned) (p - data);
		put32(out + p, (unsigned) res[i].len); memcpy(out + p + 4, res[i].data, res[i].len);
		p += 4 + res[i].len;
	}
	size_t map = p, typeList = 28, refList = typeList + 2 + 8 * count, mapLen = refList + 12 * count;
	memset(out + map, 0, mapLen);
	put32(out, 256); put32(out + 4, (unsigned) map); put32(out + 8, (unsigned) (map - data)); put32(out + 12, (unsigned) mapLen);
	memset(out + 16, 0, 240);
	memcpy(out + map, out, 16);
	put16(out + map + 24, (unsigned) typeList); put16(out + map + 26, (unsigned) mapLen);
	put16(out + map + typeList, (unsigned) (count - 1));
	for (int i = 0; i < count; i++) {
		unsigned char *t = out + map + typeList + 2 + 8 * i;
		memcpy(t, res[i].type, 4); put16(t + 4, 0); put16(t + 6, (unsigned) (refList - typeList + 12 * i));
		unsigned char *r = out + map + refList + 12 * i;
		put16(r, (unsigned) res[i].id); put16(r + 2, 0xFFFF); put32(r + 4, offsets[i]);
	}
	return map + mapLen;
}

int main(void)
{
	// MENU 200 "Pop" with three items; CNTL 128: pop-up of menu 200, title width 60, in (10,10)-(30,200).
	static const unsigned char menu[] = {
		0x00, 0xC8, 0, 0, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF, 3, 'P', 'o', 'p',
		3, 'O', 'n', 'e', 0, 0, 0, 0, 3, 'T', 'w', 'o', 0, 0, 0, 0, 5, 'T', 'h', 'r', 'e', 'e', 0, 0, 0, 0, 0 };
	static const unsigned char cntl[] = {
		0, 10, 0, 10, 0, 30, 0, 200, 0, 0, 1, 0, 0, 60, 0, 0xC8, 0x03, 0xF0, 0x12, 0x34, 0x56, 0x78, 5, 'T', 'y', 'p', 'e', ':' };
	struct Res res[] = { {"MENU", 200, menu, sizeof(menu)}, {"CNTL", 128, cntl, sizeof(cntl)} };
	unsigned char file[1024];
	size_t len = resource_file(file, res, 2);
	char path[] = "/tmp/hitoolbox_popup_cntl.XXXXXX";
	int fd = mkstemp(path);
	if (fd < 0 || write(fd, file, len) != (ssize_t) len) return 1;
	close(fd);

	FSRef ref;
	HFSUniStr255 dataFork;
	FSGetDataForkName(&dataFork);
	ResFileRefNum rf = kResFileNotOpened;
	CHECK(FSPathMakeRef((const UInt8 *) path, &ref, NULL) == noErr);
	CHECK(FSOpenResourceFile(&ref, dataFork.length, dataFork.unicode, fsRdPerm, &rf) == noErr);
	UseResFile(rf);

	Rect bounds = { 100, 100, 300, 400 };
	WindowRef window = NULL;
	CHECK(CreateNewWindow(kDocumentWindowClass, 0, &bounds, &window) == noErr && window);
	ControlRef root = NULL;
	CHECK(CreateRootControl(window, &root) == noErr);   // as the Control Manager of this system requires

	// NewControl, procID 1008: menu 200 is read and put in the menu list.
	CHECK(GetMenuHandle(200) == NULL);
	Rect r = { 40, 10, 60, 200 };
	ControlRef popup = NewControl(window, &r, "\pFormat:", true, 0, 200, 60, 1008, 7);
	CHECK(popup != NULL);
	CHECK(GetMenuHandle(200) != NULL);
	CHECK(GetControlValue(popup) == 1);
	CHECK(GetControlMaximum(popup) == 3);
	CHECK(GetControlReference(popup) == 7);
	SetControlValue(popup, 3);
	CHECK(GetControlValue(popup) == 3);
	CHECK(CountMenuItems(GetMenuHandle(200)) == 3);

	// procID 400 with the variable width variant takes the menu already in the list.
	ControlRef appearance = NewControl(window, &r, "\p", true, 0, 200, 0, kControlPopupButtonProc | kControlPopupVariableWidthVariant, 0);
	CHECK(appearance != NULL && GetControlMaximum(appearance) == 3);

	// GetNewControl reads CNTL 128.
	ControlRef fromResource = GetNewControl(128, window);
	CHECK(fromResource != NULL);
	if (fromResource) {
		Rect b; GetControlBounds(fromResource, &b);
		CHECK(b.top == 10 && b.left == 10 && b.bottom == 30 && b.right == 200);
		CHECK(GetControlReference(fromResource) == 0x12345678);
		CHECK(GetControlValue(fromResource) == 1 && GetControlMaximum(fromResource) == 3);
		CHECK(IsControlVisible(fromResource));
	}
	CHECK(GetNewControl(999, window) == NULL);

	DisposeWindow(window);
	CloseResFile(rf);
	unlink(path);
	printf(g_failures ? "FAILED: %d\n" : "PASSED\n", g_failures);
	return g_failures != 0;
}

#endif
