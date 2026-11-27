// Copyright (C) 2026 Lubos Dolezel
//
// The kNativeEndianPixMap flag of NewGWorld (32-bit only, as in macOS): by default a 16 or 32 bit graphics world has
// big-endian pixels (k16BE555PixelFormat, k32ARGBPixelFormat: the alpha is the first byte of a 32-bit pixel, whatever
// the system), and with the flag the pixels have the byte order of the system, which is little-endian
// (k16LE555PixelFormat, k32BGRAPixelFormat). QuickDraw draws red the same in both, and the bytes of the pixels
// differ. (Universal Binary Programming Guidelines, "GWorlds".)
#include <Carbon/Carbon.h>
#include <stdio.h>

#ifdef __LP64__
int main(void)
{
	printf("PASS qd_native_endian_pixmap (no 64-bit API)\n");
	return 0;
}
#else

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

// A world of the depth and flags, painted red; returns the bytes of its first pixel
static void red_pixel(short depth, GWorldFlags flags, OSType *format, unsigned char *bytes, short *size)
{
	Rect bounds = { 0, 0, 4, 4 };
	GWorldPtr world = NULL;
	CGrafPtr savedPort;
	GDHandle savedDevice;
	RGBColor red = { 0xFFFF, 0, 0 };
	CHECK(NewGWorld(&world, depth, &bounds, NULL, NULL, flags) == noErr);
	GetGWorld(&savedPort, &savedDevice);
	SetGWorld(world, NULL);
	RGBForeColor(&red);
	PaintRect(&bounds);
	SetGWorld(savedPort, savedDevice);
	PixMapHandle pixels = GetGWorldPixMap(world);
	*format = (**pixels).pixelFormat;
	*size = GetPixDepth(pixels);
	const unsigned char *p = (const unsigned char *) GetPixBaseAddr(pixels);
	for (int i = 0; i < 4; i++)
		bytes[i] = p[i];
	DisposeGWorld(world);
}

int main(void)
{
	OSType format;
	unsigned char b[4];
	short size;

	red_pixel(32, 0, &format, b, &size);
	CHECK(format == k32ARGBPixelFormat && size == 32);
	CHECK(b[0] == 0xFF && b[1] == 0xFF && b[2] == 0 && b[3] == 0);
	red_pixel(32, kNativeEndianPixMap, &format, b, &size);
	CHECK(format == k32BGRAPixelFormat && size == 32);
	CHECK(b[0] == 0 && b[1] == 0 && b[2] == 0xFF && b[3] == 0xFF);

	red_pixel(16, 0, &format, b, &size);
	CHECK(format == k16BE555PixelFormat && size == 16);
	CHECK(b[0] == 0x7C && b[1] == 0x00);
	red_pixel(16, kNativeEndianPixMap, &format, b, &size);
	CHECK(format == k16LE555PixelFormat && size == 16);
	CHECK(b[0] == 0x00 && b[1] == 0x7C);

	// a world over memory of the little-endian formats
	unsigned char memory[4 * 4 * 4];
	Rect bounds = { 0, 0, 4, 4 };
	GWorldPtr world = NULL;
	CHECK(NewGWorldFromPtr(&world, k16LE555PixelFormat, &bounds, NULL, NULL, 0, (Ptr) memory, 16) == noErr && world);
	DisposeGWorld(world);

	printf(g_failures ? "FAIL qd_native_endian_pixmap\n" : "PASS qd_native_endian_pixmap\n");
	return g_failures != 0;
}
#endif
