/*
 * Copyright (C) 2026 Lubos Dolezel
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// The bridge between QuickDraw ports and Quartz (32-bit only, as on macOS): a Quartz context over a port's
// pixels, and the conversion between a port's local coordinates and the screen's global ones.
#ifndef __LP64__

#include <CoreGraphics/CoreGraphics.h>
#include <stdlib.h>
#include "QDPortPrivate.h"

enum {
	kBGRAPixelFormat = 0x42475241, // 'BGRA': the display depth, the first byte in memory is blue
	kRowBytesFlag = 0x8000
};

// The bitmap layout of the port's pixels for Quartz: the 32-bit ports are premultiplied ARGB, big-endian
// (the alpha first in memory) or, at the display depth, BGRA, which is the little-endian ARGB value; the
// 16-bit ones 1-5-5-5, big-endian.
static CGContextRef createPortContext(struct QDPort *port) {
	CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
	if (space == NULL)
		return NULL;
	size_t width = (size_t) (port->portRect.right - port->portRect.left);
	size_t height = (size_t) (port->portRect.bottom - port->portRect.top);
	CGContextRef context;
	if (port->pixMap.pixelSize == 16) {
		context = CGBitmapContextCreate(port->pixels, width, height, 5, port->rowBytes, space,
				(port->pixMap.pixelFormat == 0x4C353535 ? kCGBitmapByteOrder16Little : kCGBitmapByteOrder16Big) | kCGImageAlphaNoneSkipFirst);
	} else if (port->pixMap.pixelFormat == kBGRAPixelFormat) {
		context = CGBitmapContextCreate(port->pixels, width, height, 8, port->rowBytes, space,
				kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst);
	} else {
		context = CGBitmapContextCreate(port->pixels, width, height, 8, port->rowBytes, space,
				kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedFirst);
	}
	CGColorSpaceRelease(space);
	return context;
}

// A Quartz context that draws into the port. Its origin is the bottom left corner of the port, with y
// up (the QuickDraw y runs down from the top). The port is flushed when the context is given back with
// QDEndCGContext().
OSStatus QDBeginCGContext(CGrafPtr inPort, CGContextRef *outContext) {
	if (outContext == NULL)
		return paramErr;
	*outContext = NULL;
	if (inPort == NULL || !QDPortIsLive(PORT_FROM(inPort)))
		return paramErr;
	struct QDPort *port = PORT_FROM(inPort);
	// The 8-bit gray ports have no Quartz context: Quartz draws 0 as black, these ports have it white
	if (port->pixMap.pixelSize == 8)
		return cDepthErr;
	CGContextRef context = createPortContext(port);
	if (context == NULL)
		return memFullErr;
	port->lockCount++;
	*outContext = context;
	return noErr;
}

// Flushes what the context drew (QDFlushPortBuffer) and releases the context.
OSStatus QDEndCGContext(CGrafPtr inPort, CGContextRef *inoutContext) {
	if (inPort == NULL || !QDPortIsLive(PORT_FROM(inPort)) || inoutContext == NULL || *inoutContext == NULL)
		return paramErr;
	struct QDPort *port = PORT_FROM(inPort);
	CGContextFlush(*inoutContext);
	CGContextRelease(*inoutContext);
	*inoutContext = NULL;
	if (port->lockCount > 0)
		port->lockCount--;
	QDFlushPortBuffer(inPort, NULL);
	return noErr;
}

// Local is relative to the top left of the port's bounds, global to the screen
static Boolean portOffset(CGrafPtr port, short *dh, short *dv) {
	if (port == NULL || !QDPortIsLive(PORT_FROM(port)))
		return false;
	struct QDPort *p = PORT_FROM(port);
	*dh = (short) (p->globalOrigin.h - p->portRect.left);
	*dv = (short) (p->globalOrigin.v - p->portRect.top);
	return true;
}

void QDGlobalToLocalPoint(CGrafPtr port, Point *point) {
	short dh, dv;
	if (point == NULL || !portOffset(port, &dh, &dv))
		return;
	point->h = (short) (point->h - dh);
	point->v = (short) (point->v - dv);
}

void QDLocalToGlobalPoint(CGrafPtr port, Point *point) {
	short dh, dv;
	if (point == NULL || !portOffset(port, &dh, &dv))
		return;
	point->h = (short) (point->h + dh);
	point->v = (short) (point->v + dv);
}

void QDGlobalToLocalRect(CGrafPtr port, Rect *bounds) {
	short dh, dv;
	if (bounds == NULL || !portOffset(port, &dh, &dv))
		return;
	OffsetRect(bounds, (short) -dh, (short) -dv);
}

void QDLocalToGlobalRect(CGrafPtr port, Rect *bounds) {
	short dh, dv;
	if (bounds == NULL || !portOffset(port, &dh, &dv))
		return;
	OffsetRect(bounds, dh, dv);
}

// Gives the port the size and the pixel buffer of a window that was resized (not part of the public API):
// the port keeps its identity, its pixels start out transparent.
OSStatus _QDResizePort(CGrafPtr inPort, short width, short height) {
	if (inPort == NULL || !QDPortIsLive(PORT_FROM(inPort)) || width < 1 || height < 1)
		return paramErr;
	struct QDPort *port = PORT_FROM(inPort);
	short pixelSize = port->pixMap.pixelSize;
	size_t rowBytes = ((size_t) width * (pixelSize / 8) + 16) & ~(size_t) 15;
	Ptr pixels = calloc(1, rowBytes * (size_t) height);
	if (pixels == NULL)
		return memFullErr;

	if (port->ownsPixels)
		free(port->pixels);
	port->ownsPixels = true;
	port->pixels = pixels;
	port->length = rowBytes * (size_t) height;
	port->rowBytes = rowBytes;
	port->portRect = (Rect) { port->portRect.top, port->portRect.left,
		(short) (port->portRect.top + height), (short) (port->portRect.left + width) };
	port->pixMap.baseAddr = pixels;
	port->pixMap.rowBytes = (short) (kRowBytesFlag | (rowBytes & 0x3FFF));
	port->pixMap.bounds = port->portRect;
	return noErr;
}

#endif /* !__LP64__ */
