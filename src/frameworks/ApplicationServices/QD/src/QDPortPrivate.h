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
typedef Pattern QDPattern;
#pragma pack(pop)

struct QDPort {
	PixMap pixMap;
	PixMapPtr pixMapPtr; // the master pointer PixMapHandle points at
	Rect portRect;
	Ptr pixels;
	Boolean ownsPixels;      // the port frees its pixels (not for those of NewGWorldFromPtr)
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

#define QD_HIDDEN __attribute__((visibility("hidden")))

#define PORT_FROM(p) ((struct QDPort *) (p))

extern GrafPtr gCurrentPort;
extern QDErr gQDError;

QD_HIDDEN void QDPortInitState(struct QDPort *port);
QD_HIDDEN void QDPortDisposeState(struct QDPort *port);
QD_HIDDEN struct QDPort *QDPortFromPixMap(const void *bits);
// Picture recording (QDRecord.inc): OpenCPicture / ClosePicture. These are internal to the framework, not exported.
QD_HIDDEN void QDRecordColor(struct QDPort *p, Boolean fore, const RGBColor *c);
QD_HIDDEN void QDRecordRect(struct QDPort *p, unsigned op, const Rect *r, const QDPattern *pat);
QD_HIDDEN void QDRecordLine(struct QDPort *p, Point from, Point to);
QD_HIDDEN void QDRecordCopyBits(struct QDPort *s, struct QDPort *d, const Rect *src, const Rect *dst, short mode);
QD_HIDDEN void QDRecordPenPat(struct QDPort *p, const QDPattern *pat);
QD_HIDDEN Handle QDRecordOpen(const Rect *srcRect, Fixed hRes, Fixed vRes, short version);
QD_HIDDEN void QDRecordClose(void);
QD_HIDDEN Boolean QDPortIsLive(const struct QDPort *port);
GDHandle GetMainDevice(void);
void SetCPixel(short h, short v, const RGBColor *c);
void CopyBits(const BitMap *srcBits, const BitMap *dstBits, const Rect *srcRect, const Rect *dstRect,
		short mode, RgnHandle maskRgn);

#endif
#endif
