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


#endif
