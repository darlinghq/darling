/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */


// Offscreen ports (32-bit only, as on macOS): NewGWorld and its pixel map,
// the pixel accessors, locking, the current port and flushing. Expected
// values were checked black-box on Mac OS X 10.14.4 (i386).

#include <QD/QD.h>
#include <stdio.h>
#include <string.h>

#ifdef __LP64__

int main(void)
{
	printf("SKIP: the port API is 32-bit only\n");
	return 0;
}

#else

static int failures = 0;

static void check(const char* what, int ok)
{
	if (!ok) {
		printf("FAIL: %s\n", what);
		failures++;
	}
}

int main(void)
{
	Rect rect = {0, 0, 30, 50};
	GWorldPtr gw = NULL;

	// A 32-bit world: pixmap fields and row size.
	check("NewGWorld 32", NewGWorld(&gw, 32, &rect, NULL, NULL, 0) == noErr && gw != NULL);
	PixMapHandle pm = GetPortPixMap(gw);
	check("pixmap of the port is the pixmap of the world", pm != NULL && pm == GetGWorldPixMap(gw));
	check("LockPixels", LockPixels(pm));
	check("base address", GetPixBaseAddr(pm) != NULL && GetPixBaseAddr(pm) == (**pm).baseAddr);
	check("row bytes are padded to 16", GetPixRowBytes(pm) == 208 && ((**pm).rowBytes & 0x3FFF) == 208);
	check("row bytes flag", ((unsigned short) (**pm).rowBytes & 0x8000) != 0);
	check("depth", GetPixDepth(pm) == 32);
	check("pixel type direct, 4 components of 8 bits",
		(**pm).pixelType == 16 && (**pm).pixelSize == 32 && (**pm).cmpCount == 4 && (**pm).cmpSize == 8);
	check("pixel format 32", (**pm).pixelFormat == 32);
	check("resolution 72 dpi", (**pm).hRes == (72 << 16) && (**pm).vRes == (72 << 16));
	check("pixmap bounds", (**pm).bounds.top == 0 && (**pm).bounds.left == 0 && (**pm).bounds.bottom == 30 && (**pm).bounds.right == 50);
	Rect b = {-1, -1, -1, -1};
	GetPixBounds(pm, &b);
	check("GetPixBounds", EqualRect(&b, &rect));
	Rect pb = {-1, -1, -1, -1};
	check("GetPortBounds", GetPortBounds(gw, &pb) == &pb && EqualRect(&pb, &rect));

	// The pixel memory is real: every row can be written and read back.
	unsigned char* base = (unsigned char*) GetPixBaseAddr(pm);
	long rowBytes = GetPixRowBytes(pm);
	for (int y = 0; y < 30; y++)
		memset(base + y * rowBytes, y + 1, rowBytes);
	int intact = 1;
	for (int y = 0; y < 30; y++)
		intact = intact && base[y * rowBytes] == y + 1 && base[y * rowBytes + rowBytes - 1] == y + 1;
	check("pixel memory holds its rows", intact);
	UnlockPixels(pm);

	// Locking the port's bits.
	check("LockPortBits", LockPortBits(gw) == noErr);
	check("base address while locked", GetPixBaseAddr(pm) != NULL);
	check("UnlockPortBits", UnlockPortBits(gw) == noErr);

	// The current port.
	GrafPtr before = NULL, current = NULL;
	GetPort(&before);
	SetPort(gw);
	GetPort(&current);
	check("GetPort returns the port set", current == (GrafPtr) gw);
	SetPort(before);
	GetPort(&current);
	check("SetPort restores", current == before);

	// Flushing a port without anything showing it does nothing and is fine, whole or in part.
	QDFlushPortBuffer(gw, NULL);
	RgnHandle part = NewRgn();
	SetRectRgn(part, 0, 0, 10, 10);
	QDFlushPortBuffer(gw, part);
	DisposeRgn(part);
	check("pixels survive a flush", base[0] == 1);
	DisposeGWorld(gw);

	// Other depths.
	GWorldPtr other = NULL;
	check("depth 0 is a 32-bit pixel in BGRA order",
		NewGWorld(&other, 0, &rect, NULL, NULL, 0) == noErr && (**GetPortPixMap(other)).pixelSize == 32 &&
		(**GetPortPixMap(other)).pixelFormat == 0x42475241 && (**GetPortPixMap(other)).cmpCount == 4);
	DisposeGWorld(other);
	check("depth 24 is a 32-bit pixel with 3 components",
		NewGWorld(&other, 24, &rect, NULL, NULL, 0) == noErr && (**GetPortPixMap(other)).pixelSize == 32 &&
		(**GetPortPixMap(other)).cmpCount == 3 && (**GetPortPixMap(other)).pixelFormat == 32);
	DisposeGWorld(other);
	check("depth 16",
		NewGWorld(&other, 16, &rect, NULL, NULL, 0) == noErr && GetPixDepth(GetPortPixMap(other)) == 16 &&
		(**GetPortPixMap(other)).cmpCount == 3 && (**GetPortPixMap(other)).cmpSize == 5 &&
		(**GetPortPixMap(other)).pixelFormat == 16 && GetPixRowBytes(GetPortPixMap(other)) == 112);
	DisposeGWorld(other);
	GWorldPtr failed = (GWorldPtr) 7;
	check("depth 7 is cDepthErr and gives no port", NewGWorld(&failed, 7, &rect, NULL, NULL, 0) == cDepthErr && failed == NULL);
	failed = (GWorldPtr) 7;
	check("depth 15 is cDepthErr", NewGWorld(&failed, 15, &rect, NULL, NULL, 0) == cDepthErr && failed == NULL);
	failed = (GWorldPtr) 7;
	check("depth -1 is cDepthErr", NewGWorld(&failed, -1, &rect, NULL, NULL, 0) == cDepthErr && failed == NULL);

	// Bounds.
	Rect empty = {5, 5, 5, 20}, inverted = {10, 10, 0, 0}, offset = {-10, -20, 20, 30}, wide = {0, 0, 10, 1000};
	failed = (GWorldPtr) 7;
	check("empty bounds are paramErr and give no port", NewGWorld(&failed, 32, &empty, NULL, NULL, 0) == paramErr && failed == NULL);
	failed = (GWorldPtr) 7;
	check("inverted bounds are paramErr", NewGWorld(&failed, 32, &inverted, NULL, NULL, 0) == paramErr && failed == NULL);
	check("bounds do not have to start at the origin",
		NewGWorld(&other, 32, &offset, NULL, NULL, 0) == noErr && (**GetPortPixMap(other)).bounds.top == -10 &&
		(**GetPortPixMap(other)).bounds.left == -20 && GetPixRowBytes(GetPortPixMap(other)) == 208);
	DisposeGWorld(other);
	check("a wide world", NewGWorld(&other, 32, &wide, NULL, NULL, 0) == noErr && GetPixRowBytes(GetPortPixMap(other)) == 4016);
	DisposeGWorld(other);
	// A row that is already a multiple of 16 bytes still grows by 16.
	struct { short width; long rowBytes; } rows[] = {{1, 16}, {3, 16}, {4, 32}, {16, 80}, {17, 80}, {32, 144}, {333, 1344}, {1023, 4096}, {1024, 4112}};
	for (unsigned i = 0; i < sizeof rows / sizeof *rows; i++) {
		Rect r = {0, 0, 4, rows[i].width};
		char what[64];
		snprintf(what, sizeof what, "row bytes of a world %d wide", rows[i].width);
		check(what, NewGWorld(&other, 32, &r, NULL, NULL, 0) == noErr && GetPixRowBytes(GetPortPixMap(other)) == rows[i].rowBytes);
		DisposeGWorld(other);
	}
	// Rows beyond what the pixmap's field holds are reported in full.
	Rect huge = {0, 0, 4, 4095};
	check("row bytes beyond 0x3FFF", NewGWorld(&other, 32, &huge, NULL, NULL, 0) == noErr && GetPixRowBytes(GetPortPixMap(other)) == 16384);
	DisposeGWorld(other);
	check("QDError", QDError() == noErr);

	if (failures == 0)
		printf("PASS\n");
	return failures != 0;
}

#endif
