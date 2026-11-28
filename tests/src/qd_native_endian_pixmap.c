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

// A red world of the depth and flags is resized with UpdateGWorld and the update flags: the replacement has the
// pixel format and the bytes the byte order calls for
static void update_keeps_format(short depth, GWorldFlags newFlags, GWorldFlags updateFlags, OSType expected,
		unsigned char e0, unsigned char e1, unsigned char e2, unsigned char e3)
{
	Rect bounds = { 0, 0, 4, 4 }, larger = { 0, 0, 8, 8 };
	GWorldPtr world = NULL;
	CGrafPtr savedPort;
	GDHandle savedDevice;
	RGBColor red = { 0xFFFF, 0, 0 };
	CHECK(NewGWorld(&world, depth, &bounds, NULL, NULL, newFlags) == noErr);
	GetGWorld(&savedPort, &savedDevice);
	SetGWorld(world, NULL);
	RGBForeColor(&red);
	PaintRect(&bounds);
	SetGWorld(savedPort, savedDevice);
	GWorldFlags result = UpdateGWorld(&world, depth, &larger, NULL, NULL, updateFlags | (1L << 28));
	CHECK(!(result & (1UL << 31)));
	PixMapHandle pixels = GetGWorldPixMap(world);
	CHECK((**pixels).pixelFormat == expected);
	CHECK(GetPixDepth(pixels) == depth);
	CHECK((**pixels).bounds.right == 8 && (**pixels).bounds.bottom == 8);
	const unsigned char *p = (const unsigned char *) GetPixBaseAddr(pixels);
	CHECK(p[0] == e0 && p[1] == e1);
	if (depth == 32)
		CHECK(p[2] == e2 && p[3] == e3);
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

	// UpdateGWorld keeps the byte order of the world's pixels: a native-endian world is replaced by a native-endian
	// one (of the new size, with the old pixels kept with clipPix), a big-endian one by a big-endian one, and the
	// flag turns a big-endian world into a native-endian one.
	update_keeps_format(32, 0, 0, k32ARGBPixelFormat, 0xFF, 0xFF, 0, 0);
	update_keeps_format(32, kNativeEndianPixMap, 0, k32BGRAPixelFormat, 0, 0, 0xFF, 0xFF);
	update_keeps_format(16, 0, 0, k16BE555PixelFormat, 0x7C, 0x00, 0, 0);
	update_keeps_format(16, kNativeEndianPixMap, 0, k16LE555PixelFormat, 0x00, 0x7C, 0, 0);
	update_keeps_format(32, 0, kNativeEndianPixMap, k32BGRAPixelFormat, 0, 0, 0xFF, 0xFF);

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
