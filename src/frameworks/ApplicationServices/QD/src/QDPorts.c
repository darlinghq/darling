/*
 * Copyright (C) 2026 Lubos Dolezel
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// Graphics ports and offscreen pixel maps (32-bit only, as on macOS).
//
// A port is a pixel buffer of real memory with the PixMap that describes it.
// Pixels are 32 bits (depth 32 or 24: ARGB, a big-endian value per pixel, so
// the first byte in memory is the alpha or unused byte; depth 0, the display
// depth: BGRA, the first byte in memory is blue), or 16 bits (big-endian
// 1-5-5-5), and rows are padded to a multiple of 16 bytes, always by at least one byte. Indexed depths (1 to 8) need a
// color table and are not supported. QDFlushPortBuffer announces the flushed part of
// a port with a local CFNotificationCenter notification, so a higher layer
// that shows a port on screen (AppKit's NSQuickDrawView) can observe its own
// port without QuickDraw knowing about it.
#ifndef __LP64__

#include <QD/QD.h>
#include <CoreFoundation/CoreFoundation.h>
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

enum {
	kQDRowBytesFlag = 0x8000,
	kQDRGBDirect = 16,
	kQDBGRAPixelFormat = 0x42475241 // 'BGRA'
};

#include "QDPortPrivate.h"

GrafPtr gCurrentPort;
QDErr gQDError;

// The name of the notification QDFlushPortBuffer posts to the local
// notification center, with the port as the object and, in the user info, the
// flushed rectangle as the CFNumber values "top", "left", "bottom", "right"
// (absent when the whole port was flushed, that is, with no region).
#define kQDFlushNotificationName "com.darlinghq.QD.PortFlushed"

// On failure *offscreenGWorld is NULL. Depth 0 is the depth of the display,
// 32 bits here; depth 24 is a 32-bit pixel with three components; a depth
// other than 0, 16, 24 and 32 is cDepthErr.
QDErr NewGWorld(GWorldPtr *offscreenGWorld, short PixelDepth, const Rect *boundsRect,
		CTabHandle cTable, GDHandle aGDevice, GWorldFlags flags) {
	if (offscreenGWorld == NULL)
		return gQDError = paramErr;
	*offscreenGWorld = NULL;
	if (boundsRect == NULL || EmptyRect(boundsRect))
		return gQDError = paramErr;
	if (PixelDepth != 0 && PixelDepth != 16 && PixelDepth != 24 && PixelDepth != 32)
		return gQDError = cDepthErr;
	short pixelSize = PixelDepth == 16 ? 16 : 32;

	size_t width = (size_t) (boundsRect->right - boundsRect->left);
	size_t height = (size_t) (boundsRect->bottom - boundsRect->top);
	// A row whose size is already a multiple of 16 still grows by 16.
	size_t rowBytes = (width * (pixelSize / 8) + 16) & ~(size_t) 15;
	if (rowBytes > 0x7FFFFFFF / height)
		return gQDError = cNoMemErr;

	struct QDPort *port = calloc(1, sizeof(struct QDPort));
	if (port == NULL)
		return gQDError = cNoMemErr;
	port->length = rowBytes * height;
	port->pixels = calloc(1, port->length);
	if (port->pixels == NULL) {
		free(port);
		return gQDError = cNoMemErr;
	}
	port->portRect = *boundsRect;

	PixMap *pm = &port->pixMap;
	pm->baseAddr = port->pixels;
	// The field holds the low bits of the row size; GetPixRowBytes has all of it.
	pm->rowBytes = (short) (kQDRowBytesFlag | (rowBytes & 0x3FFF));
	port->rowBytes = rowBytes;
	pm->bounds = *boundsRect;
	pm->pmVersion = 1;
	pm->packType = 0;
	pm->packSize = 0;
	pm->hRes = 72 << 16;
	pm->vRes = 72 << 16;
	pm->pixelType = kQDRGBDirect;
	pm->pixelSize = pixelSize;
	pm->cmpCount = (PixelDepth == 24 || PixelDepth == 16) ? 3 : 4;
	pm->cmpSize = pixelSize == 16 ? 5 : 8;
	pm->pixelFormat = PixelDepth == 0 ? kQDBGRAPixelFormat : (OSType) pixelSize;
	port->pixMapPtr = pm;

	QDPortInitState(port);
	*offscreenGWorld = (GWorldPtr) port;
	return gQDError = noErr;
}

void DisposeGWorld(GWorldPtr offscreenGWorld) {
	struct QDPort *port = PORT_FROM(offscreenGWorld);
	if (port == NULL)
		return;
	if (gCurrentPort == (GrafPtr) port)
		gCurrentPort = NULL;
	QDPortDisposeState(port);
	free(port->pixels);
	free(port);
}

PixMapHandle GetPortPixMap(CGrafPtr port) {
	if (port == NULL)
		return NULL;
	return &PORT_FROM(port)->pixMapPtr;
}

PixMapHandle GetGWorldPixMap(GWorldPtr offscreenGWorld) {
	return GetPortPixMap(offscreenGWorld);
}

// The pixels never move or get purged, so locking always succeeds.
Boolean LockPixels(PixMapHandle pm) {
	return pm != NULL;
}

void UnlockPixels(PixMapHandle pm) {
}

Ptr GetPixBaseAddr(PixMapHandle pm) {
	if (pm == NULL || *pm == NULL)
		return NULL;
	return (*pm)->baseAddr;
}

// Pixel maps here are only ever those of ports, which carry the full row size.
long GetPixRowBytes(PixMapHandle pm) {
	if (pm == NULL || *pm == NULL)
		return 0;
	struct QDPort *port = (struct QDPort *) ((char *) pm - offsetof(struct QDPort, pixMapPtr));
	return (long) port->rowBytes;
}

short GetPixDepth(PixMapHandle pm) {
	if (pm == NULL || *pm == NULL)
		return 0;
	return (*pm)->pixelSize;
}

void GetPixBounds(PixMapHandle pixMap, Rect *bounds) {
	if (pixMap == NULL || *pixMap == NULL)
		SetRect(bounds, 0, 0, 0, 0);
	else
		*bounds = (*pixMap)->bounds;
}

Rect *GetPortBounds(CGrafPtr port, Rect *rect) {
	if (port == NULL)
		SetRect(rect, 0, 0, 0, 0);
	else
		*rect = PORT_FROM(port)->portRect;
	return rect;
}

void SetPort(GrafPtr port) {
	gCurrentPort = port;
}

void GetPort(GrafPtr *port) {
	if (port != NULL)
		*port = gCurrentPort;
}

QDErr LockPortBits(GrafPtr port) {
	if (port == NULL)
		return gQDError = paramErr;
	PORT_FROM(port)->lockCount++;
	return gQDError = noErr;
}

QDErr UnlockPortBits(GrafPtr port) {
	if (port == NULL)
		return gQDError = paramErr;
	if (PORT_FROM(port)->lockCount > 0)
		PORT_FROM(port)->lockCount--;
	return gQDError = noErr;
}

void QDFlushPortBuffer(CGrafPtr port, RgnHandle region) {
	if (port == NULL)
		return;
	CFDictionaryRef info = NULL;
	Rect bounds;
	// A region without area has nothing to flush.
	if (region != NULL && EmptyRgn(region))
		return;
	if (region != NULL) {
		GetRegionBounds(region, &bounds);
		const CFStringRef keys[4] = {CFSTR("top"), CFSTR("left"), CFSTR("bottom"), CFSTR("right")};
		const short values[4] = {bounds.top, bounds.left, bounds.bottom, bounds.right};
		CFNumberRef numbers[4];
		for (int i = 0; i < 4; i++)
			numbers[i] = CFNumberCreate(kCFAllocatorDefault, kCFNumberShortType, &values[i]);
		info = CFDictionaryCreate(kCFAllocatorDefault, (const void **) keys, (const void **) numbers, 4,
				&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		for (int i = 0; i < 4; i++)
			CFRelease(numbers[i]);
	}
	CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(),
			CFSTR(kQDFlushNotificationName), port, info, true);
	if (info != NULL)
		CFRelease(info);
}

QDErr QDError(void) {
	return gQDError;
}

#endif /* !__LP64__ */
