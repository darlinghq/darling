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
#include <CoreGraphics/CGContext.h>
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
#ifndef __LP64__
/* Point arithmetic of QuickDraw (32-bit only): AddPt adds the first point to the second */
void AddPt(Point src, Point *dst);
void SubPt(Point src, Point *dst);
#endif
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
/* Pictures (32-bit only, as on macOS) */
#pragma pack(push, 2)
struct Picture {
	short picSize;
	Rect picFrame;
};
struct OpenCPicParams {
	Rect srcRect;
	Fixed hRes;
	Fixed vRes;
	short version;
	short reserved1;
	SInt32 reserved2;
};
#pragma pack(pop)
typedef struct Picture Picture;
typedef Picture *PicPtr;
typedef PicPtr *PicHandle;
typedef struct OpenCPicParams OpenCPicParams;
PicHandle OpenCPicture(const OpenCPicParams *newHeader);
void ClosePicture(void);
#endif

#ifndef __LP64__
/* Ports and offscreen pixel maps (32-bit only, as on macOS). */
enum {
	pixPurge = 1L << 0,
	noNewDevice = 1L << 1,
	kNativeEndianPixMap = 1L << 8,   /* 16 and 32 bit pixels in the byte order of the system (little-endian here) */
	useTempMem = 1L << 2,
	keepLocal = 1L << 3
};

QDErr NewGWorld(GWorldPtr *offscreenGWorld, short PixelDepth, const Rect *boundsRect,
	CTabHandle cTable, GDHandle aGDevice, GWorldFlags flags);
/* The pixel formats of NewGWorldFromPtr (the formats of the 32-bit QuickDraw offscreen API) */
enum {
	k16BE555PixelFormat = 0x00000010,
	k32ARGBPixelFormat = 0x00000020,
	k32BGRAPixelFormat = 0x42475241, /* 'BGRA' */
	k16LE555PixelFormat = 0x4C353535, /* 'L555' */
	k8IndexedGrayPixelFormat = 0x00000028 /* 8-bit gray, 0 is white */
};
QDErr NewGWorldFromPtr(GWorldPtr *offscreenGWorld, UInt32 PixelFormat, const Rect *boundsRect,
	CTabHandle cTable, GDHandle aGDevice, GWorldFlags flags, Ptr newBuffer, SInt32 rowBytes);
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

/* Transfer modes of the drawing and copying calls */
enum {
	srcCopy = 0,
	srcOr = 1,
	srcXor = 2,
	srcBic = 3,
	notSrcCopy = 4,
	notSrcOr = 5,
	notSrcXor = 6,
	notSrcBic = 7,
	patCopy = 8,
	patOr = 9,
	patXor = 10,
	patBic = 11,
	notPatCopy = 12,
	notPatOr = 13,
	notPatXor = 14,
	notPatBic = 15,
	blend = 32,
	addPin = 33,
	addOver = 34,
	subPin = 35,
	transparent = 36,
	addMax = 37,
	subOver = 38,
	addMin = 39,
	grayishTextOr = 49,
	hilite = 50,
	ditherCopy = 64
};

/* The colors that ForeColor and BackColor take */
enum {
	blackColor = 33,
	whiteColor = 30,
	redColor = 205,
	greenColor = 341,
	blueColor = 409,
	cyanColor = 273,
	magentaColor = 137,
	yellowColor = 69
};

/* Patterns, the pen and the colors of the current port */
struct Pattern {
	UInt8 pat[8];
};
typedef struct Pattern Pattern;
typedef const Pattern *ConstPatternParam;

void PenNormal(void);
void PenSize(short width, short height);
void PenMode(short mode);
void PenPat(const Pattern *pat);
void BackPat(const Pattern *pat);
void GetPen(Point *pt);
void HidePen(void);
void ShowPen(void);
void MoveTo(short h, short v);
void Move(short dh, short dv);
void LineTo(short h, short v);
void Line(short dh, short dv);
void RGBForeColor(const RGBColor *color);
void RGBBackColor(const RGBColor *color);
void GetForeColor(RGBColor *color);
void GetBackColor(RGBColor *color);
void ForeColor(SInt32 color);
void BackColor(SInt32 color);

/* Text state of the current port */
void TextFont(short font);
void TextSize(short size);
void TextMode(short mode);
void TextFace(StyleParameter face);
short GetPortTextFont(CGrafPtr port);
short GetPortTextSize(CGrafPtr port);
short GetPortTextMode(CGrafPtr port);
Style GetPortTextFace(CGrafPtr port);

/* Drawing of rectangles, clipping and coordinates */
void FrameRect(const Rect *r);
void PaintRect(const Rect *r);
void EraseRect(const Rect *r);
void InvertRect(const Rect *r);
void FillRect(const Rect *r, const Pattern *pat);
void ClipRect(const Rect *r);
void SetClip(RgnHandle rgn);
void GetClip(RgnHandle rgn);
void GlobalToLocal(Point *pt);
void LocalToGlobal(Point *pt);

/* The current graphics world and the copying of pixels */
void GetGWorld(CGrafPtr *port, GDHandle *gdh);
void SetGWorld(CGrafPtr port, GDHandle gdh);
const BitMap *GetPortBitMapForCopyBits(CGrafPtr port);
void CopyBits(const BitMap *srcBits, const BitMap *dstBits, const Rect *srcRect, const Rect *dstRect,
	short mode, RgnHandle maskRgn);
GWorldFlags UpdateGWorld(GWorldPtr *offscreenGWorld, short pixelDepth, const Rect *boundsRect, CTabHandle cTable,
	GDHandle aGDevice, GWorldFlags flags);

/* Pictures */
PicHandle GetPicture(short pictureID);
void DrawPicture(PicHandle myPicture, const Rect *dstRect);
/* Text (32-bit only, as on macOS) */
#pragma pack(push, 2)
struct FontInfo {
	short ascent;
	short descent;
	short widMax;
	short leading;
};
#pragma pack(pop)
typedef struct FontInfo FontInfo;
void DrawText(const void *textBuf, short firstByte, short byteCount);
void DrawString(ConstStr255Param s);
void DrawChar(CharParameter ch);
short TextWidth(const void *textBuf, short firstByte, short byteCount);
short StringWidth(ConstStr255Param s);
short CharWidth(CharParameter ch);
void GetFontInfo(FontInfo *info);
void GetFNum(ConstStr255Param name, short *familyID);
void GetFontName(short familyID, Str255 name);

/* Quartz drawing into a port (32-bit only, as on macOS) */
OSStatus QDBeginCGContext(CGrafPtr inPort, CGContextRef *outContext);
OSStatus QDEndCGContext(CGrafPtr inPort, CGContextRef *inoutContext);

/* Conversion between the port's local coordinates and the screen's global ones */
void QDGlobalToLocalPoint(CGrafPtr port, Point *point);
void QDLocalToGlobalPoint(CGrafPtr port, Point *point);
void QDGlobalToLocalRect(CGrafPtr port, Rect *bounds);
void QDLocalToGlobalRect(CGrafPtr port, Rect *bounds);
#endif

#ifdef __cplusplus
}
#endif

#endif
