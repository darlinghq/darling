/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef _QD_H_
#define _QD_H_

#include <MacTypes.h>
#include <CarbonCore/MacErrors.h>

// The QuickDraw structures use 68K (2-byte) alignment.
#pragma pack(push, 2)

struct RGBColor;
struct ColorSpec;
struct GDevice;
struct ColorTable;

typedef struct GDevice GDevice;
typedef GDevice * GDPtr;
typedef GDPtr * GDHandle;

typedef struct RGBColor RGBColor;

typedef struct ColorSpec ColorSpec;
typedef ColorSpec * ColorSpecPtr;

struct RGBColor {
	unsigned short red;
	unsigned short green;
	unsigned short blue;
};

struct ColorSpec {
	short value;
	RGBColor rgb;
};

typedef ColorSpec CSpecArray[1];

typedef struct ColorTable ColorTable;
typedef ColorTable * CTabPtr;
typedef CTabPtr * CTabHandle;

struct PixMap {
	Ptr baseAddr;
	short rowBytes;
	Rect bounds;
	short pmVersion;
	short packType;
	SInt32 packSize;
	Fixed hRes;
	Fixed vRes;
	short pixelType;
	short pixelSize;
	short cmpCount;
	short cmpSize;
	OSType pixelFormat;
	CTabHandle pmTable;
	void * pmExt;
};

typedef struct PixMap PixMap;
typedef PixMap * PixMapPtr;
typedef PixMapPtr * PixMapHandle;

struct GDevice {
	short gdRefNum;
	short gdID;
	short gdType;
	Handle gdITable;
	short gdResPref;
	Handle gdSearchProc;
	Handle gdCompProc;
	short gdFlags;
	PixMapHandle gdPMap;
	SInt32 gdRefCon;
	GDHandle gdNextGD;
	Rect gdRect;
	SInt32 gdMode;
	short gdCCBytes;
	short gdCCDepth;
	Handle gdCCXData;
	Handle gdCCXMask;
	Handle gdExt;
};

struct ColorTable {
	SInt32 ctSeed;
	short ctFlags;
	short ctSize;
	CSpecArray ctTable;
};

#pragma pack(pop)

typedef struct OpaqueWindowPtr* WindowPtr;
typedef WindowPtr WindowRef;

typedef struct OpaqueGrafPtr* GrafPtr;
typedef GrafPtr CGrafPtr;
typedef CGrafPtr GWorldPtr;

struct BitMap {
  Ptr baseAddr;
  short rowBytes;
  Rect bounds;
};
typedef struct BitMap BitMap;
typedef BitMap* BitMapPtr;

typedef short QDErr;
typedef unsigned long GWorldFlags;

#ifdef __cplusplus
extern "C" {
#endif

/* Cursor visibility. */
void InitCursor(void);
void HideCursor(void);
void ShowCursor(void);

/* Rectangles. */
void SetRect(Rect *r, short left, short top, short right, short bottom);
void OffsetRect(Rect *r, short dh, short dv);
void InsetRect(Rect *r, short dh, short dv);
Boolean EmptyRect(const Rect *r);
Boolean EqualRect(const Rect *rect1, const Rect *rect2);
Boolean PtInRect(Point pt, const Rect *r);
Boolean SectRect(const Rect *src1, const Rect *src2, Rect *dstRect);
void UnionRect(const Rect *src1, const Rect *src2, Rect *dstRect);

/* Regions. */
/* On 32-bit the region record is public, under the name MacRegion (a plain
   Region would clash with Xlib's); on 64-bit RgnHandle is opaque. The record
   is declared for source compatibility only: its contents are not meaningful
   on macOS (observed on 10.14 i386, where the bounds are not at the start of
   the block), so use GetRegionBounds. */
#ifdef __LP64__
typedef struct OpaqueRgnHandle *RgnHandle;
#else
#pragma pack(push, 2)
struct MacRegion {
	UInt16 rgnSize;
	Rect rgnBBox;
};
#pragma pack(pop)
typedef struct MacRegion MacRegion;
typedef MacRegion *RgnPtr;
typedef RgnPtr *RgnHandle;
#endif

RgnHandle NewRgn(void);
void DisposeRgn(RgnHandle rgn);
void SetEmptyRgn(RgnHandle rgn);
void SetRectRgn(RgnHandle rgn, short left, short top, short right, short bottom);
void RectRgn(RgnHandle rgn, const Rect *r);
void CopyRgn(RgnHandle srcRgn, RgnHandle dstRgn);
void OffsetRgn(RgnHandle rgn, short dh, short dv);
void InsetRgn(RgnHandle rgn, short dh, short dv);
void SectRgn(RgnHandle srcRgnA, RgnHandle srcRgnB, RgnHandle dstRgn);
void UnionRgn(RgnHandle srcRgnA, RgnHandle srcRgnB, RgnHandle dstRgn);
void DiffRgn(RgnHandle srcRgnA, RgnHandle srcRgnB, RgnHandle dstRgn);
void XorRgn(RgnHandle srcRgnA, RgnHandle srcRgnB, RgnHandle dstRgn);
Boolean EmptyRgn(RgnHandle rgn);
Boolean EqualRgn(RgnHandle rgnA, RgnHandle rgnB);
Boolean PtInRgn(Point pt, RgnHandle rgn);
Boolean RectInRgn(const Rect *r, RgnHandle rgn);
Rect *GetRegionBounds(RgnHandle region, Rect *bounds);

#ifndef __LP64__
/* Ports and offscreen pixel maps (32-bit only, as on macOS). */
enum {
	pixPurge = 1L << 0,
	noNewDevice = 1L << 1,
	useTempMem = 1L << 2,
	keepLocal = 1L << 3
};

QDErr NewGWorld(GWorldPtr *offscreenGWorld, short PixelDepth, const Rect *boundsRect,
	CTabHandle cTable, GDHandle aGDevice, GWorldFlags flags);
void DisposeGWorld(GWorldPtr offscreenGWorld);
PixMapHandle GetGWorldPixMap(GWorldPtr offscreenGWorld);
PixMapHandle GetPortPixMap(CGrafPtr port);
Boolean LockPixels(PixMapHandle pm);
void UnlockPixels(PixMapHandle pm);
Ptr GetPixBaseAddr(PixMapHandle pm);
long GetPixRowBytes(PixMapHandle pm);
short GetPixDepth(PixMapHandle pm);
void GetPixBounds(PixMapHandle pixMap, Rect *bounds);
Rect *GetPortBounds(CGrafPtr port, Rect *rect);
void SetPort(GrafPtr port);
void GetPort(GrafPtr *port);
QDErr LockPortBits(GrafPtr port);
QDErr UnlockPortBits(GrafPtr port);
void QDFlushPortBuffer(CGrafPtr port, RgnHandle region);
QDErr QDError(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
