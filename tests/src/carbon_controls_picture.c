// Copyright (C) 2026 Lubos Dolezel
//
// The content of a bevel button that is a QuickDraw picture (kControlContentPictHandle, 32-bit only): the picture is drawn
// at its own size in the middle of the button and does not draw outside the button. Read back from the pixels of the
// window (CGWindowListCreateImage()). Needs a window system (X11 or Wayland).
#include <Carbon/Carbon.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __LP64__
int main(void)
{
	printf("PASS carbon_controls_picture (QuickDraw pictures are 32-bit only)\n");
	return 0;
}
#else
extern int64_t _HIWindowGetCGSWindowID(WindowRef window);

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

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

static int isRed(const unsigned char* pixels, size_t width, int x, int y)
{
	const unsigned char* p = pixels + ((size_t) y * width + x) * 4;

	return p[0] > 200 && p[1] < 60 && p[2] < 60;
}

static void put16(unsigned char** p, unsigned v) { *(*p)++ = v >> 8; *(*p)++ = v & 255; }

// A version 2 picture of the frame 0, 0, 30, 30 that paints all of it red
static Handle makePicture(void)
{
	unsigned char bytes[64], *p = bytes;

	put16(&p, 0); put16(&p, 0); put16(&p, 0); put16(&p, 30); put16(&p, 30);
	put16(&p, 0x0011); put16(&p, 0x02FF);
	put16(&p, 0x0C00); for (int i = 0; i < 12; i++) put16(&p, 0);
	put16(&p, 0x001A); put16(&p, 0xFFFF); put16(&p, 0); put16(&p, 0);                  // RGBFgColor red
	put16(&p, 0x0031); put16(&p, 0); put16(&p, 0); put16(&p, 30); put16(&p, 30);       // PaintRect
	put16(&p, 0x00FF);

	Handle h = NewHandle((Size) (p - bytes));

	memcpy(*h, bytes, (size_t) (p - bytes));
	return h;
}

int main(void)
{
	Rect bounds = { 100, 100, 200, 220 };
	WindowRef window = NULL;
	ControlRef root = NULL, button = NULL;
	Rect buttonRect = { 10, 10, 60, 60 };
	ControlButtonContentInfo content;

	CHECK(kControlContentPictRes == 3);
	CHECK(CreateNewWindow(kPlainWindowClass, kWindowNoAttributes, &bounds, &window) == noErr);
	CHECK(CreateRootControl(window, &root) == noErr);
	memset(&content, 0, sizeof(content));
	content.contentType = kControlContentPictHandle;
	content.u.picture = makePicture();
	CHECK(CreateBevelButtonControl(window, &buttonRect, NULL, kControlBevelButtonNormalBevel, kControlBehaviorPushbutton, &content, 0, 0, 0, &button) == noErr);
	ShowWindow(window);
	spin(600);

	size_t width = 0, height = 0;
	unsigned char* pixels = readPixels(window, &width, &height);

	CHECK(pixels != NULL);
	if (pixels && width >= 120 && height >= 100)
	{
		// the picture is 30x30, in the middle of the button: x and y 20 to 50
		CHECK(isRed(pixels, width, 35, 35));
		CHECK(isRed(pixels, width, 22, 22));
		CHECK(isRed(pixels, width, 48, 48));
		// the button around it is not red, nor is the window outside the button
		CHECK(!isRed(pixels, width, 13, 13));
		CHECK(!isRed(pixels, width, 35, 56));
		CHECK(!isRed(pixels, width, 80, 80));
	}
	free(pixels);
	DisposeWindow(window);
	printf(g_failures ? "FAIL carbon_controls_picture\n" : "PASS carbon_controls_picture\n");
	return g_failures != 0;
}
#endif
