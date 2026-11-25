/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// The port record shared by the QD sources (32-bit only).
#ifndef _QD_PORT_PRIVATE_H_
#define _QD_PORT_PRIVATE_H_
#ifndef __LP64__

#include <QD/QD.h>
#include <stddef.h>

#pragma pack(push, 2)
typedef struct QDPattern { UInt8 pat[8]; } QDPattern;
#pragma pack(pop)

struct QDPort {
	PixMap pixMap;
	PixMapPtr pixMapPtr; // the master pointer PixMapHandle points at
	Rect portRect;
	Ptr pixels;
	size_t length;
	size_t rowBytes;
	int lockCount;

	// Graphics state (Inside Macintosh: Imaging With QuickDraw, chapter 3).
	struct QDPort *next;    // registry of live ports, for validation
	Point penLoc;
	Point penSize;
	short penMode;
	short penVis;           // drawn when >= 0
	QDPattern penPat;
	QDPattern bkPat;
	RGBColor fgColor;
	RGBColor bkColor;
	short txFont, txSize, txMode;
	Style txFace;
	RgnHandle clipRgn;      // NULL: wide open
	Boolean clipIsRect;
	Rect clipRect;          // valid when clipIsRect
	Point globalOrigin;     // global position of the port's top-left (window ports)
};

#define PORT_FROM(p) ((struct QDPort *) (p))

extern GrafPtr gCurrentPort;
extern QDErr gQDError;

void QDPortInitState(struct QDPort *port);
void QDPortDisposeState(struct QDPort *port);
struct QDPort *QDPortFromPixMap(const void *bits);
Boolean QDPortIsLive(const struct QDPort *port);
GDHandle GetMainDevice(void);
void SetCPixel(short h, short v, const RGBColor *c);
void CopyBits(const BitMap *srcBits, const BitMap *dstBits, const Rect *srcRect, const Rect *dstRect,
		short mode, RgnHandle maskRgn);

#endif
#endif
