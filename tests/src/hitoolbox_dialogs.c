// Copyright (C) 2026 fr4nck
//
// The Dialog Manager: dialogs and alerts from 'DLOG', 'ALRT' and 'DITL' resources (buttons, check box, radio button,
// static and edit text, picture, icon, user item and a 'CNTL' pop-up), their items, the drawing of the items the
// Dialog Manager draws (read back from the pixels of the window, CGWindowListCreateImage()), and ModalDialog(),
// DialogSelect() and Alert() with keys posted to the event queue and simulated clicks. Needs a window system (X11 or
// Wayland). The classic Dialog Manager is 32-bit only.
#include <Carbon/Carbon.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static Boolean g_filterCalled;

static Boolean filterProc(DialogRef dialog, EventRecord* event, DialogItemIndex* itemHit)
{
	g_filterCalled = true;
	if (event->what != nullEvent)
		return false;
	*itemHit = 7;
	return true;
}

#ifdef __LP64__
int main(void)
{
	DialogItemIndex hit = 0;
	EventRecord event = { nullEvent };
	ModalFilterUPP filter = NewModalFilterUPP(filterProc);

	CHECK(InvokeModalFilterUPP(NULL, &event, &hit, filter) && hit == 7 && g_filterCalled);
	DisposeModalFilterUPP(filter);
	printf(g_failures ? "FAILED: %d\n" : "PASSED (the classic Dialog Manager is 32-bit only)\n", g_failures);
	return g_failures != 0;
}
#else
extern int64_t _HIWindowGetCGSWindowID(WindowRef window);

static void put16(unsigned char* p, unsigned v) { p[0] = v >> 8; p[1] = v; }
static void put32(unsigned char* p, unsigned v) { p[0] = v >> 24; p[1] = v >> 16; p[2] = v >> 8; p[3] = v; }

struct Res { const char* type; short id; const unsigned char* data; size_t len; };

// A resource file (resources in the data fork); the resources of a type are next to each other in the array
static size_t resourceFile(unsigned char* out, const struct Res* res, int count)
{
	size_t data = 256, p = data;
	unsigned offsets[64];
	int types = 0, firstOfType[64], countOfType[64];

	for (int i = 0; i < count; i++)
	{
		offsets[i] = (unsigned) (p - data);
		put32(out + p, (unsigned) res[i].len);
		memcpy(out + p + 4, res[i].data, res[i].len);
		p += 4 + res[i].len;
		if (i == 0 || memcmp(res[i].type, res[i - 1].type, 4))
		{
			firstOfType[types] = i;
			countOfType[types++] = 0;
		}
		countOfType[types - 1]++;
	}

	size_t map = p, typeList = 28, refList = typeList + 2 + 8 * types, mapLen = refList + 12 * count;

	memset(out + map, 0, mapLen);
	put32(out, 256); put32(out + 4, (unsigned) map); put32(out + 8, (unsigned) (map - data)); put32(out + 12, (unsigned) mapLen);
	memset(out + 16, 0, 240);
	memcpy(out + map, out, 16);
	put16(out + map + 24, (unsigned) typeList); put16(out + map + 26, (unsigned) mapLen);
	put16(out + map + typeList, (unsigned) (types - 1));
	for (int t = 0; t < types; t++)
	{
		unsigned char* e = out + map + typeList + 2 + 8 * t;

		memcpy(e, res[firstOfType[t]].type, 4);
		put16(e + 4, (unsigned) (countOfType[t] - 1));
		put16(e + 6, (unsigned) (refList - typeList + 12 * firstOfType[t]));
	}
	for (int i = 0; i < count; i++)
	{
		unsigned char* r = out + map + refList + 12 * i;

		put16(r, (unsigned) res[i].id); put16(r + 2, 0xFFFF); put32(r + 4, offsets[i]);
	}
	return map + mapLen;
}

// A 'DITL' item: rectangle (top, left, bottom, right), type and data
static size_t ditlItem(unsigned char* p, int top, int left, int bottom, int right, int type, const void* data, int len)
{
	memset(p, 0, 4);
	put16(p + 4, top); put16(p + 6, left); put16(p + 8, bottom); put16(p + 10, right);
	p[12] = type;
	p[13] = len;
	memcpy(p + 14, data, len);
	return (14 + len + 1) & ~1;
}

static size_t ditlText(unsigned char* p, int top, int left, int bottom, int right, int type, const char* text)
{
	return ditlItem(p, top, left, bottom, right, type, text, (int) strlen(text));
}

static size_t ditlID(unsigned char* p, int top, int left, int bottom, int right, int type, int id)
{
	unsigned char data[2];

	put16(data, id);
	return ditlItem(p, top, left, bottom, right, type, data, 2);
}

// A 'DLOG' resource
static size_t dlog(unsigned char* p, int procID, int visible, int itemsID, const char* title, int position)
{
	size_t n;

	put16(p, 100); put16(p + 2, 100); put16(p + 4, 320); put16(p + 6, 380);
	put16(p + 8, procID); p[10] = visible; p[11] = 0; p[12] = 0; p[13] = 0;
	put32(p + 14, 0x01020304); put16(p + 18, itemsID);
	p[20] = (unsigned char) strlen(title);
	memcpy(p + 21, title, p[20]);
	n = 21 + p[20];
	if (n & 1)
		p[n++] = 0;
	put16(p + n, position);
	return n + 2;
}

static size_t alrt(unsigned char* p, int itemsID, int stages)
{
	put16(p, 100); put16(p + 2, 100); put16(p + 4, 200); put16(p + 6, 400);
	put16(p + 8, itemsID); put16(p + 10, stages); put16(p + 12, 0x300A);
	return 14;
}

static void quitTimer(EventLoopTimerRef t, void* u) { QuitApplicationEventLoop(); }

static void spin(int ms)
{
	EventLoopTimerRef timer;

	InstallEventLoopTimer(GetMainEventLoop(), ms / 1000.0, 0, NewEventLoopTimerUPP(quitTimer), NULL, &timer);
	RunApplicationEventLoop();
	RemoveEventLoopTimer(timer);
}

static unsigned char* readPixels(WindowRef w, size_t* width, size_t* height)
{
	CGImageRef image = CGWindowListCreateImage(CGRectNull, kCGWindowListOptionIncludingWindow, (CGWindowID) _HIWindowGetCGSWindowID(w), kCGWindowImageBoundsIgnoreFraming);

	if (!image)
		return NULL;
	*width = CGImageGetWidth(image);
	*height = CGImageGetHeight(image);

	unsigned char* pixels = calloc(*width * *height, 4);
	CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
	CGContextRef ctx = CGBitmapContextCreate(pixels, *width, *height, 8, *width * 4, space, kCGImageAlphaPremultipliedLast);

	CGContextSetBlendMode(ctx, kCGBlendModeCopy);
	CGContextDrawImage(ctx, CGRectMake(0, 0, *width, *height), image);
	CGContextRelease(ctx);
	CGColorSpaceRelease(space);
	CGImageRelease(image);
	return pixels;
}

static const unsigned char* pixel(const unsigned char* pixels, size_t width, int x, int y)
{
	return pixels + ((size_t) y * width + x) * 4;
}

static int isRed(const unsigned char* p) { return p[0] > 200 && p[1] < 60 && p[2] < 60; }
static int isDark(const unsigned char* p) { return p[0] < 90 && p[1] < 90 && p[2] < 90; }

// The dark pixels in a rectangle of the window's content
static int darkPixels(const unsigned char* pixels, size_t width, size_t height, int top, int left, int bottom, int right)
{
	int n = 0;

	for (int y = top; y < bottom && y < (int) height; y++)
	{
		for (int x = left; x < right && x < (int) width; x++)
			n += isDark(pixel(pixels, width, x, y));
	}
	return n;
}

static void postKey(char ch, UInt32 modifiers)
{
	EventRef event;
	UInt32 code = 0;

	CreateEvent(NULL, kEventClassKeyboard, kEventRawKeyDown, GetCurrentEventTime(), kEventAttributeNone, &event);
	SetEventParameter(event, kEventParamKeyMacCharCodes, typeChar, sizeof(ch), &ch);
	SetEventParameter(event, kEventParamKeyCode, typeUInt32, sizeof(code), &code);
	SetEventParameter(event, kEventParamKeyModifiers, typeUInt32, sizeof(modifiers), &modifiers);
	PostEventToQueue(GetMainEventQueue(), event, kEventPriorityStandard);
	ReleaseEvent(event);
}

static ControlRef g_clickControl;
static ControlPartCode g_clickPart;

static void clickTimer(EventLoopTimerRef t, void* u)
{
	HIViewSimulateClick(g_clickControl, g_clickPart, 0, NULL);
}

// ModalDialog() while a click on the control is simulated
static DialogItemIndex modalWithClick(ControlRef control, ControlPartCode part)
{
	EventLoopTimerRef timer;
	DialogItemIndex hit = 0;

	g_clickControl = control;
	g_clickPart = part;
	InstallEventLoopTimer(GetMainEventLoop(), 0.2, 0, NewEventLoopTimerUPP(clickTimer), NULL, &timer);
	ModalDialog(NULL, &hit);
	RemoveEventLoopTimer(timer);
	return hit;
}

static DialogItemIndex modalWithKey(char ch, UInt32 modifiers)
{
	DialogItemIndex hit = 0;

	postKey(ch, modifiers);
	ModalDialog(NULL, &hit);
	return hit;
}

static int g_userCalls;
static DialogItemIndex g_userItem;

static void userProc(DialogRef dialog, DialogItemIndex item)
{
	g_userCalls++;
	g_userItem = item;
}

static int textIs(Handle h, const char* expected)
{
	Str255 text;

	GetDialogItemText(h, text);
	return text[0] == strlen(expected) && memcmp(text + 1, expected, text[0]) == 0;
}

// A version 2 picture of the frame 0, 0, 30, 30 that paints all of it red
static size_t redPicture(unsigned char* bytes)
{
	unsigned char* p = bytes;

	put16(p, 0); p += 2; put16(p, 0); p += 2; put16(p, 0); p += 2; put16(p, 30); p += 2; put16(p, 30); p += 2;
	put16(p, 0x0011); p += 2; put16(p, 0x02FF); p += 2;
	put16(p, 0x0C00); p += 2; memset(p, 0, 24); p += 24;
	put16(p, 0x001A); p += 2; put16(p, 0xFFFF); p += 2; put16(p, 0); p += 2; put16(p, 0); p += 2;
	put16(p, 0x0031); p += 2; put16(p, 0); p += 2; put16(p, 0); p += 2; put16(p, 30); p += 2; put16(p, 30); p += 2;
	put16(p, 0x00FF); p += 2;
	return (size_t) (p - bytes);
}

int main(void)
{
	// The dialog's items
	static unsigned char ditl128[1024], ditl132[64], ditl200[256], dlog128[64], dlog129[64], dlog130[64], dlog131[64];
	static unsigned char alrt200[16], alrt201[16], alrt202[16], icon[128], pict[128];
	size_t n = 2;

	n += ditlText(ditl128 + n, 20, 200, 40, 260, kButtonDialogItem, "OK");					// 1
	n += ditlText(ditl128 + n, 50, 200, 70, 260, kButtonDialogItem, "Cancel");				// 2
	n += ditlText(ditl128 + n, 20, 20, 36, 150, editText, "abc");							// 3
	n += ditlText(ditl128 + n, 50, 20, 66, 150, statText, "Name ^0");						// 4
	n += ditlText(ditl128 + n, 80, 20, 96, 150, kCheckBoxDialogItem, "Loud");				// 5
	n += ditlText(ditl128 + n, 100, 20, 116, 150, kRadioButtonDialogItem, "Mono");			// 6
	n += ditlID(ditl128 + n, 80, 200, 110, 230, picItem, 300);								// 7
	n += ditlID(ditl128 + n, 120, 200, 152, 232, iconItem, 400);							// 8
	n += ditlItem(ditl128 + n, 130, 20, 150, 60, userItem, "", 0);							// 9
	n += ditlID(ditl128 + n, 160, 20, 180, 200, kResourceControlDialogItem, 128);			// 10
	n += ditlText(ditl128 + n, 190, 20, 206, 100, statText | itemDisable, "Off");			// 11
	put16(ditl128, 10);
	size_t ditl128Len = n;

	n = 2;
	n += ditlID(ditl132 + n, 20, 20, 40, 200, kResourceControlDialogItem, 999);			// a missing 'CNTL'
	put16(ditl132, 0);
	size_t ditl132Len = n;

	n = 2;
	n += ditlText(ditl200 + n, 60, 220, 80, 290, kButtonDialogItem, "OK");
	n += ditlText(ditl200 + n, 60, 130, 80, 200, kButtonDialogItem, "Cancel");
	n += ditlText(ditl200 + n, 10, 60, 50, 290, statText, "^0 is ^1.");
	put16(ditl200, 2);
	size_t ditl200Len = n;

	size_t dlog128Len = dlog(dlog128, 5, 1, 128, "Test", 0);
	size_t dlog129Len = dlog(dlog129, 16, 1, 128, "Rounded", 0);		// a window definition that is not available
	size_t dlog130Len = dlog(dlog130, 5, 1, 131, "No items", 0);		// no 'DITL' 131
	size_t dlog131Len = dlog(dlog131, 5, 0, 132, "Missing CNTL", 0x280A);
	size_t alrt200Len = alrt(alrt200, 200, 0x4444);						// stage 1 drawn, default item 1
	size_t alrt201Len = alrt(alrt201, 200, 0x0000);						// stage 1 not drawn
	size_t alrt202Len = alrt(alrt202, 200, 0x000C);						// stage 1 drawn, default item 2

	// ICON 400: the top left quarter set
	memset(icon, 0, sizeof(icon));
	for (int y = 0; y < 16; y++)
		put16(icon + y * 4, 0xFFFF);

	size_t pictLen = redPicture(pict);

	// MENU 200 "Pop" with three items; CNTL 128: pop-up of menu 200 in (160,20)-(180,200)
	static const unsigned char menu[] = {
		0x00, 0xC8, 0, 0, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF, 3, 'P', 'o', 'p',
		3, 'O', 'n', 'e', 0, 0, 0, 0, 3, 'T', 'w', 'o', 0, 0, 0, 0, 5, 'T', 'h', 'r', 'e', 'e', 0, 0, 0, 0, 0 };
	static const unsigned char cntl[] = {
		0, 160, 0, 20, 0, 180, 0, 200, 0, 0, 1, 0, 0, 60, 0, 0xC8, 0x03, 0xF0, 0, 0, 0, 0, 5, 'T', 'y', 'p', 'e', ':' };
	struct Res res[] = {
		{ "ALRT", 200, alrt200, alrt200Len }, { "ALRT", 201, alrt201, alrt201Len }, { "ALRT", 202, alrt202, alrt202Len },
		{ "CNTL", 128, cntl, sizeof(cntl) },
		{ "DITL", 128, ditl128, ditl128Len }, { "DITL", 132, ditl132, ditl132Len }, { "DITL", 200, ditl200, ditl200Len },
		{ "DLOG", 128, dlog128, dlog128Len }, { "DLOG", 129, dlog129, dlog129Len }, { "DLOG", 130, dlog130, dlog130Len },
		{ "DLOG", 131, dlog131, dlog131Len },
		{ "ICON", 400, icon, sizeof(icon) }, { "MENU", 200, menu, sizeof(menu) }, { "PICT", 300, pict, pictLen },
	};
	static unsigned char file[8192];
	size_t len = resourceFile(file, res, sizeof(res) / sizeof(res[0]));
	char path[] = "/tmp/hitoolbox_dialogs.XXXXXX";
	int fd = mkstemp(path);

	if (fd < 0 || write(fd, file, len) != (ssize_t) len)
		return 1;
	close(fd);

	FSRef ref;
	HFSUniStr255 dataFork;
	ResFileRefNum rf = kResFileNotOpened;

	FSGetDataForkName(&dataFork);
	CHECK(FSPathMakeRef((const UInt8*) path, &ref, NULL) == noErr);
	CHECK(FSOpenResourceFile(&ref, dataFork.length, dataFork.unicode, fsRdPerm, &rf) == noErr);
	UseResFile(rf);

	// A resource of one type, then one of another type with the same ID
	Handle dlogHandle = GetResource('DLOG', 128), ditlHandle = GetResource('DITL', 128);
	CHECK(dlogHandle && ditlHandle && dlogHandle != ditlHandle && GetHandleSize(ditlHandle) == (Size) ditl128Len);

	// Dialogs that cannot be made are not made
	CHECK(GetNewDialog(999, NULL, (WindowRef) -1) == NULL);
	CHECK(GetNewDialog(129, NULL, (WindowRef) -1) == NULL);
	CHECK(GetNewDialog(130, NULL, (WindowRef) -1) == NULL);
	CHECK(GetNewDialog(131, NULL, (WindowRef) -1) == NULL);

	ParamText("\pBob", NULL, NULL, NULL);

	DialogRef d = GetNewDialog(128, NULL, (WindowRef) -1);
	CHECK(d != NULL);
	if (!d)
	{
		printf("FAILED: %d\n", g_failures);
		return 1;
	}

	WindowRef w = GetDialogWindow(d);
	CFStringRef title = NULL;

	CHECK(w != NULL && GetDialogFromWindow(w) == d);
	CHECK(GetDialogPort(d) == GetWindowPort(w));
	CHECK(GetWRefCon(w) == 0x01020304);
	CHECK(IsWindowVisible(w));
	CHECK(CopyWindowTitleAsCFString(w, &title) == noErr && title && CFStringCompare(title, CFSTR("Test"), 0) == kCFCompareEqualTo);
	CHECK(CountDITL(d) == 11);

	// The items
	DialogItemType type;
	Handle h[12];
	Rect box[12];

	for (int i = 1; i <= 11; i++)
		GetDialogItem(d, i, &type, &h[i], &box[i]);
	GetDialogItem(d, 1, &type, &h[1], &box[1]);
	CHECK(type == kButtonDialogItem && h[1] && box[1].top == 20 && box[1].left == 200 && box[1].bottom == 40 && box[1].right == 260);
	GetDialogItem(d, 3, &type, &h[3], &box[3]);
	CHECK(type == editText && textIs(h[3], "abc"));
	GetDialogItem(d, 4, &type, &h[4], &box[4]);
	CHECK(type == statText && textIs(h[4], "Name ^0"));
	GetDialogItem(d, 11, &type, &h[11], &box[11]);
	CHECK(type == (statText | itemDisable));
	CHECK(textIs(h[1], "OK"));								// a button's text is its control's title

	ControlRef okButton = (ControlRef) h[1], checkBox = (ControlRef) h[5], popup = (ControlRef) h[10];
	CHECK(IsControlVisible(okButton) && IsControlVisible(checkBox));
	CHECK(popup && GetControlMaximum(popup) == 3 && GetControlValue(popup) == 1);
	SetControlValue(checkBox, 1);
	CHECK(GetControlValue(checkBox) == 1);
	SetControlValue(checkBox, 0);

	SetDialogItemText(h[3], "\pxyz");
	CHECK(textIs(h[3], "xyz"));
	SetDialogItemText(h[1], "\pYes");
	CHECK(textIs(h[1], "Yes"));
	SetDialogItemText(h[1], "\pOK");

	CHECK(FindDialogItem(d, (Point) { 25, 30 }) == 2);
	CHECK(FindDialogItem(d, (Point) { 300, 300 }) == -1);

	HideDialogItem(d, 5);
	GetDialogItem(d, 5, &type, &h[5], &box[5]);
	CHECK(box[5].left >= 16384 && !IsControlVisible(checkBox));
	CHECK(FindDialogItem(d, (Point) { 85, 30 }) == -1);
	ShowDialogItem(d, 5);
	GetDialogItem(d, 5, &type, &h[5], &box[5]);
	CHECK(box[5].left == 20 && IsControlVisible(checkBox));
	CHECK(FindDialogItem(d, (Point) { 85, 30 }) == 4);

	CHECK(GetControlHilite(okButton) == kControlNoPart);
	HiliteControl(okButton, 255);
	CHECK(GetControlHilite(okButton) == 255);
	HiliteControl(okButton, kControlNoPart);

	Rect userBox = { 130, 20, 150, 60 };
	SetDialogItem(d, 9, userItem, (Handle) userProc, &userBox);
	GetDialogItem(d, 9, &type, &h[9], &box[9]);
	CHECK(h[9] == (Handle) userProc);

	CHECK(GetDialogDefaultItem(d) == 1);
	CHECK(SetDialogCancelItem(d, 2) == noErr && GetDialogCancelItem(d) == 2);
	CHECK(SetDialogCancelItem(d, 99) == paramErr);
	CHECK(GetDialogKeyboardFocusItem(d) == 3);

	// What the Dialog Manager draws
	spin(500);
	DrawDialog(d);
	CHECK(g_userCalls > 0 && g_userItem == 9);
	spin(300);

	size_t width = 0, height = 0;
	unsigned char* pixels = readPixels(w, &width, &height);

	CHECK(pixels != NULL);
	if (pixels)
	{
		CHECK(isRed(pixel(pixels, width, 215, 95)));								// the picture
		CHECK(!isRed(pixel(pixels, width, 240, 95)));								// only in its rectangle
		CHECK(isDark(pixel(pixels, width, 205, 125)));								// the icon's set quarter
		CHECK(!isDark(pixel(pixels, width, 225, 145)));								// and not the rest
		CHECK(darkPixels(pixels, width, height, 50, 20, 66, 150) > 10);				// the static text
		CHECK(darkPixels(pixels, width, height, 26, 15, 30, 19) > 0);				// the frame of the edit text
		CHECK(darkPixels(pixels, width, height, 20, 20, 36, 150) > 10);				// its text
		CHECK(darkPixels(pixels, width, height, 28, 195, 32, 200) > 0);				// the outline of the default button
		free(pixels);
	}

	// ModalDialog: keys
	CHECK(modalWithKey('q', 0) == 3 && textIs(h[3], "xyzq"));
	CHECK(modalWithKey(8, 0) == 3 && textIs(h[3], "xyz"));
	CHECK(modalWithKey('\r', 0) == 1);
	CHECK(modalWithKey(3, 0) == 1);
	CHECK(modalWithKey(27, 0) == 2);
	CHECK(modalWithKey('.', cmdKey) == 2);

	// A disabled item takes keys and reports no hit
	SetDialogItem(d, 3, editText | itemDisable, h[3], &box[3]);
	postKey('z', 0);
	CHECK(modalWithKey('\r', 0) == 1 && textIs(h[3], "xyzz"));
	SetDialogItem(d, 3, editText, h[3], &box[3]);

	// Clicks on controls
	CHECK(modalWithClick(checkBox, kControlCheckBoxPart) == 5);
	CHECK(modalWithClick(okButton, kControlButtonPart) == 1);

	// The filter procedure sees the null events
	DialogItemIndex hit = 0;
	ModalFilterUPP filter = NewModalFilterUPP(filterProc);
	ModalDialog(filter, &hit);
	CHECK(g_filterCalled && hit == 7);

	// DialogSelect
	Rect content;
	GetWindowBounds(w, kWindowContentRgn, &content);
	EventRecord event = { mouseDown, 0, TickCount(), { (short) (content.top + 28), (short) (content.left + 30) }, 0 };
	DialogRef which = NULL;

	hit = 0;
	CHECK(IsDialogEvent(&event));
	CHECK(DialogSelect(&event, &which, &hit) && which == d && hit == 3);
	event.what = keyDown;
	event.message = 'k';
	hit = 0;
	CHECK(DialogSelect(&event, &which, &hit) && hit == 3 && textIs(h[3], "xyzzk"));
	event.message = '\r';
	CHECK(!DialogSelect(&event, &which, &hit));					// Return is not the default item outside ModalDialog
	event.what = updateEvt;
	event.message = (unsigned long) w;
	CHECK(IsDialogEvent(&event) && !DialogSelect(&event, &which, &hit));

	DisposeDialog(d);
	CHECK(GetDialogWindow(d) == NULL);

	// Alerts
	ParamText("\pThe file", "\plocked", NULL, NULL);
	postKey('\r', 0);
	CHECK(Alert(200, NULL) == 1);
	postKey(27, 0);
	CHECK(Alert(200, NULL) == 2);								// the other button is the cancel item
	CHECK(Alert(201, NULL) == -1);
	postKey('\r', 0);
	CHECK(StopAlert(202, NULL) == 2);
	CHECK(Alert(999, NULL) == -1);

	CloseResFile(rf);
	unlink(path);
	printf(g_failures ? "FAILED: %d\n" : "PASSED\n", g_failures);
	return g_failures != 0;
}
#endif
