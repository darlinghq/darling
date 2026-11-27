/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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

#ifndef _QuickTime_ImageCompression_H_
#define _QuickTime_ImageCompression_H_

#include <ApplicationServices/ApplicationServices.h>
#include <CoreServices/CoreServices.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __LP64__

#pragma pack(push, 2)

typedef OSType CodecType;
typedef UInt32 CodecQ;

enum {
	codecMinQuality = 0x000,
	codecLowQuality = 0x100,
	codecNormalQuality = 0x200,
	codecHighQuality = 0x300,
	codecMaxQuality = 0x3FF,
	codecLosslessQuality = 0x400
};

/* The transformation of the images: row vectors of Fixed, the third column Fract */
struct MatrixRecord {
	Fixed matrix[3][3];
};
typedef struct MatrixRecord MatrixRecord;
typedef MatrixRecord *MatrixRecordPtr;

struct ImageDescription {
	SInt32 idSize;
	CodecType cType;
	SInt32 resvd1;
	SInt16 resvd2;
	SInt16 dataRefIndex;
	SInt16 version;
	SInt16 revisionLevel;
	SInt32 vendor;
	CodecQ temporalQuality;
	CodecQ spatialQuality;
	SInt16 width;
	SInt16 height;
	Fixed hRes;
	Fixed vRes;
	SInt32 dataSize;
	SInt16 frameCount;
	Str31 name;
	SInt16 depth;
	SInt16 clutID;
};
typedef struct ImageDescription ImageDescription;
typedef ImageDescription *ImageDescriptionPtr;
typedef ImageDescriptionPtr *ImageDescriptionHandle;

#pragma pack(pop)

enum {
	GraphicsImporterComponentType = 'grip'
};

/* Matrices */
void SetIdentityMatrix(MatrixRecord *matrix);
void TranslateMatrix(MatrixRecord *m, Fixed deltaH, Fixed deltaV);
void ScaleMatrix(MatrixRecord *m, Fixed scaleX, Fixed scaleY, Fixed aboutX, Fixed aboutY);
void ConcatMatrix(const MatrixRecord *a, MatrixRecord *b);
void RectMatrix(MatrixRecord *matrix, const Rect *srcRect, const Rect *dstRect);

/* Graphics worlds over memory of the caller, in the pixel formats of QuickDraw's offscreen API */
OSErr QTNewGWorldFromPtr(GWorldPtr *gw, OSType pixelFormat, const Rect *boundsRect, CTabHandle cTable,
	GDHandle aGDevice, GWorldFlags flags, void *baseAddr, long rowBytes);

/* Graphics importers */
typedef ComponentInstance GraphicsImportComponent;
OSErr GetGraphicsImporterForFile(const FSSpec *theFile, ComponentInstance *gi);
OSErr GetGraphicsImporterForFileWithFlags(const FSSpec *theFile, ComponentInstance *gi, UInt32 flags);
ComponentResult GraphicsImportSetDataFile(ComponentInstance ci, const FSSpec *theFile);
ComponentResult GraphicsImportGetImageDescription(ComponentInstance ci, ImageDescriptionHandle *desc);
ComponentResult GraphicsImportGetNaturalBounds(ComponentInstance ci, Rect *naturalBounds);
ComponentResult GraphicsImportSetMatrix(ComponentInstance ci, const MatrixRecord *matrix);
ComponentResult GraphicsImportGetMatrix(ComponentInstance ci, MatrixRecord *matrix);
ComponentResult GraphicsImportSetBoundsRect(ComponentInstance ci, const Rect *bounds);
ComponentResult GraphicsImportGetBoundsRect(ComponentInstance ci, Rect *bounds);
ComponentResult GraphicsImportSetGWorld(ComponentInstance ci, CGrafPtr port, GDHandle gd);
ComponentResult GraphicsImportGetGWorld(ComponentInstance ci, CGrafPtr *port, GDHandle *gd);
ComponentResult GraphicsImportSetQuality(ComponentInstance ci, CodecQ quality);
ComponentResult GraphicsImportGetQuality(ComponentInstance ci, CodecQ *quality);
ComponentResult GraphicsImportDraw(ComponentInstance ci);

#endif

#ifdef __cplusplus
}
#endif

#endif
