// Copyright (C) 2026 Lubos Dolezel
//
// ModalDialog() with keys typed by a user: a harness (carbon_dialog_user.py) acts as the user and types Return into
// the first dialog and Escape into the second. The documented contract (Dialog Manager): Return hits the default
// item, Escape (and Command-period) the cancel item set with SetDialogCancelItem(), and ModalDialog() returns the item hit. The dialog has an OK
// button (the default item, 1) and a Cancel button (2).
//
//   hitoolbox_dialog_modal_keys
//
// Lines the harness reads:
//   DIALOG_READY n    the dialog n (1: Return is typed, 2: Escape is typed) is shown
//   PASS hitoolbox_dialog_modal_keys / FAIL ...
//
// Needs a window system (X11 or Wayland); the classic Dialog Manager is 32-bit only.
#include <Carbon/Carbon.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#ifdef __LP64__
int main(void)
{
	printf("PASS hitoolbox_dialog_modal_keys (no 64-bit API)\n");
	return 0;
}
#else

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void onAlarm(int sig)
{
	printf("FAIL: timed out\n");
	fflush(stdout);
	_exit(1);
}

static void put16(unsigned char *p, unsigned v) { p[0] = v >> 8; p[1] = v; }
static void put32(unsigned char *p, unsigned v) { p[0] = v >> 24; p[1] = v >> 16; p[2] = v >> 8; p[3] = v; }

// A resource file (resources in the data fork) with one DLOG and one DITL
static size_t resourceFile(unsigned char *out, const unsigned char *dlog, size_t dlogLen, const unsigned char *ditl, size_t ditlLen)
{
	size_t data = 256, p = data, ditlOff, mapLen, map;

	put32(out + p, (unsigned) dlogLen); memcpy(out + p + 4, dlog, dlogLen); p += 4 + dlogLen;
	ditlOff = p - data;
	put32(out + p, (unsigned) ditlLen); memcpy(out + p + 4, ditl, ditlLen); p += 4 + ditlLen;
	map = p;
	mapLen = 28 + 2 + 16 + 24;
	memset(out, 0, 256);
	memset(out + map, 0, mapLen);
	put32(out, 256); put32(out + 4, (unsigned) map); put32(out + 8, (unsigned) (map - data)); put32(out + 12, (unsigned) mapLen);
	memcpy(out + map, out, 16);
	put16(out + map + 24, 28); put16(out + map + 26, (unsigned) mapLen);
	put16(out + map + 28, 1);   // two types
	memcpy(out + map + 30, "DITL", 4); put16(out + map + 34, 0); put16(out + map + 36, 2 + 16);
	memcpy(out + map + 38, "DLOG", 4); put16(out + map + 42, 0); put16(out + map + 44, 2 + 16 + 12);
	put16(out + map + 46, 128); put16(out + map + 48, 0xFFFF); put32(out + map + 50, (unsigned) ditlOff);
	put16(out + map + 58, 128); put16(out + map + 60, 0xFFFF); put32(out + map + 62, 0);
	return map + mapLen;
}

// A DITL item: rectangle (top, left, bottom, right), type, text
static size_t item(unsigned char *p, int top, int left, int bottom, int right, int type, const char *text)
{
	size_t len = strlen(text);

	memset(p, 0, 4);
	put16(p + 4, top); put16(p + 6, left); put16(p + 8, bottom); put16(p + 10, right);
	p[12] = type; p[13] = (unsigned char) len;
	memcpy(p + 14, text, len);
	return (14 + len + 1) & ~1;
}

static void runDialog(int number, DialogItemIndex expected)
{
	DialogRef dialog = GetNewDialog(128, NULL, (WindowRef) -1);
	DialogItemIndex hit = 0;

	CHECK(dialog != NULL);
	if (!dialog)
		return;
	CHECK(GetDialogDefaultItem(dialog) == 1);
	CHECK(SetDialogCancelItem(dialog, 2) == noErr);
	ShowWindow(GetDialogWindow(dialog));
	SelectWindow(GetDialogWindow(dialog));
	printf("DIALOG_READY %d\n", number);
	fflush(stdout);
	ModalDialog(NULL, &hit);
	CHECK(hit == expected);
	DisposeDialog(dialog);
}

int main(void)
{
	static unsigned char ditl[128], dlog[64], file[1024];
	size_t n = 2, dlogLen = 0;

	signal(SIGALRM, onAlarm);
	alarm(140);

	n += item(ditl + n, 80, 200, 100, 260, kButtonDialogItem, "OK");
	n += item(ditl + n, 80, 120, 100, 180, kButtonDialogItem, "Cancel");
	n += item(ditl + n, 20, 20, 40, 260, statText, "Press a key");
	put16(ditl, 2);

	put16(dlog, 100); put16(dlog + 2, 100); put16(dlog + 4, 200); put16(dlog + 6, 400);
	put16(dlog + 8, 5); dlog[10] = 0; dlog[11] = 0; dlog[12] = 0; dlog[13] = 0;   // dBoxProc-like window, not shown yet
	put32(dlog + 14, 0); put16(dlog + 18, 128);
	dlog[20] = 4; memcpy(dlog + 21, "Keys", 4);
	put16(dlog + 26, 0);
	dlogLen = 28;

	size_t len = resourceFile(file, dlog, dlogLen, ditl, n);
	char path[] = "/tmp/hitoolbox_modal_keys.XXXXXX";
	int fd = mkstemp(path);

	if (fd < 0 || write(fd, file, len) != (ssize_t) len)
		return 1;
	close(fd);

	FSRef ref;
	HFSUniStr255 dataFork;
	ResFileRefNum rf = kResFileNotOpened;

	FSGetDataForkName(&dataFork);
	CHECK(FSPathMakeRef((const UInt8 *) path, &ref, NULL) == noErr);
	CHECK(FSOpenResourceFile(&ref, dataFork.length, dataFork.unicode, fsRdPerm, &rf) == noErr);
	UseResFile(rf);

	runDialog(1, 1);   // Return hits the default item
	runDialog(2, 2);   // Escape hits the cancel item

	CloseResFile(rf);
	unlink(path);
	if (g_failures)
		printf("FAIL hitoolbox_dialog_modal_keys: %d\n", g_failures);
	else
		printf("PASS hitoolbox_dialog_modal_keys\n");
	return g_failures != 0;
}
#endif
