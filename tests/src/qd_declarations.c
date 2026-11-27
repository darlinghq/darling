// Copyright (C) 2026 Lubos Dolezel
//
// The public QuickDraw header declares the pen, color, text state, clipping, graphics world and picture calls of the
// 32-bit API (a program that uses them without a declaration does not build with a current compiler). The program
// uses each declaration of a small drawing through the header alone: the port is an off-screen graphics world, its
// pixels are read back, and the state calls report what was set. 32-bit only, as the calls are in macOS.
#include <Carbon/Carbon.h>
#include <stdio.h>
#include <string.h>

#ifdef __LP64__
int main(void)
{
	printf("PASS qd_declarations (no 64-bit API)\n");
	return 0;
}
#else

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

// The color of a pixel of a 32-bit world: the first byte in memory is the alpha, then red, green, blue
static void pixel(GWorldPtr world, int h, int v, unsigned *red, unsigned *green, unsigned *blue)
{
	PixMapHandle pixels = GetGWorldPixMap(world);
	LockPixels(pixels);
	const unsigned char *p = (const unsigned char *) GetPixBaseAddr(pixels) + v * GetPixRowBytes(pixels) + h * 4;
	*red = p[1];
	*green = p[2];
	*blue = p[3];
	UnlockPixels(pixels);
}

int main(void)
{
	Rect bounds = { 0, 0, 40, 60 };
	GWorldPtr world = NULL;
	CGrafPtr savedPort;
	GDHandle savedDevice;
	CHECK(NewGWorld(&world, 32, &bounds, NULL, NULL, 0) == noErr);
	GetGWorld(&savedPort, &savedDevice);
	SetGWorld(world, NULL);
	CGrafPtr current;
	GetGWorld(&current, NULL);
	CHECK(current == world);

	// colors and rectangles
	RGBColor white = { 0xFFFF, 0xFFFF, 0xFFFF }, red = { 0xFFFF, 0, 0 }, blue = { 0, 0, 0xFFFF }, got;
	RGBBackColor(&white);
	EraseRect(&bounds);
	RGBForeColor(&red);
	Rect box = { 10, 10, 20, 30 };
	PaintRect(&box);
	GetForeColor(&got);
	CHECK(got.red == 0xFFFF && got.green == 0 && got.blue == 0);
	GetBackColor(&got);
	CHECK(got.red == 0xFFFF && got.green == 0xFFFF && got.blue == 0xFFFF);
	unsigned r, g, b;
	pixel(world, 15, 15, &r, &g, &b);
	CHECK(r == 255 && g == 0 && b == 0);
	pixel(world, 5, 5, &r, &g, &b);
	CHECK(r == 255 && g == 255 && b == 255);

	// a pattern fill, the clip rectangle and the pen
	Pattern solid;
	memset(&solid, 0xFF, sizeof(solid));
	RGBForeColor(&blue);
	Rect clip = { 0, 0, 40, 5 };
	ClipRect(&clip);
	Rect everything = { 0, 0, 40, 60 };
	FillRect(&everything, &solid);
	pixel(world, 2, 2, &r, &g, &b);
	CHECK(r == 0 && g == 0 && b == 255);
	pixel(world, 30, 30, &r, &g, &b);
	CHECK(r == 255 && g == 255 && b == 255);
	ClipRect(&bounds);
	PenNormal();
	PenSize(1, 1);
	PenMode(patCopy);
	PenPat(&solid);
	BackPat(&solid);
	MoveTo(1, 1);
	Point pen;
	GetPen(&pen);
	CHECK(pen.h == 1 && pen.v == 1);
	Move(2, 3);
	GetPen(&pen);
	CHECK(pen.h == 3 && pen.v == 4);
	LineTo(30, 4);
	Line(0, 0);
	GetPen(&pen);
	CHECK(pen.h == 30 && pen.v == 4);
	HidePen();
	ShowPen();
	ForeColor(blackColor);
	BackColor(whiteColor);
	FrameRect(&box);
	InvertRect(&box);
	RgnHandle region = NewRgn();
	GetClip(region);
	SetClip(region);
	DisposeRgn(region);

	// text state, per port
	TextFont(21);
	TextSize(14);
	TextMode(srcOr);
	TextFace(bold);
	CHECK(GetPortTextFont(world) == 21 && GetPortTextSize(world) == 14 && GetPortTextMode(world) == srcOr && GetPortTextFace(world) == bold);

	// coordinates and copying between worlds
	Point point = { 3, 4 };
	LocalToGlobal(&point);
	GlobalToLocal(&point);
	CHECK(point.v == 3 && point.h == 4);
	GWorldPtr copy = NULL;
	CHECK(NewGWorld(&copy, 32, &bounds, NULL, NULL, 0) == noErr);
	CopyBits(GetPortBitMapForCopyBits(world), GetPortBitMapForCopyBits(copy), &bounds, &bounds, srcCopy, NULL);
	pixel(copy, 2, 2, &r, &g, &b);
	CHECK(r == 0 && g == 0 && b == 255);
	Rect larger = { 0, 0, 80, 120 };
	UpdateGWorld(&copy, 32, &larger, NULL, NULL, 0);
	Rect after;
	CHECK(GetPortBounds(copy, &after)->bottom == 80 && after.right == 120);
	DisposeGWorld(copy);

	// a picture of no content: drawing a missing picture is an error, not a crash
	DrawPicture(NULL, &bounds);
	CHECK(QDError() != noErr);
	CHECK(GetPicture(-12345) == NULL);

	SetGWorld(savedPort, savedDevice);
	DisposeGWorld(world);

	printf(g_failures ? "FAIL qd_declarations\n" : "PASS qd_declarations\n");
	return g_failures != 0;
}
#endif
