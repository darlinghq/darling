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

// Graphics importers of the Image Compression Manager (32-bit only, as on macOS): a component of the type 'grip'
// that decodes the image of a file and draws it into a QuickDraw graphics world, transformed by a matrix.
// PICT files (with or without the 512 byte header) are played by QuickDraw; JPEG, PNG, GIF, TIFF and BMP files
// are decoded by ImageIO. One component is registered per file type; GetGraphicsImporterForFile finds the type by
// looking at the file's first bytes and opens that component. The state of an importer is the storage of its
// component instance.
// The matrix may scale and translate (no rotation: codecUnimpErr from the drawing). The quality is stored and not
// used, and the graphics device is ignored.
// Reference: Image Compression Manager Reference, "Graphics Importer Components".
#ifndef __LP64__

#include <QuickTime/ImageCompression.h>
#include <ImageIO/ImageIO.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// The sizes of the file formats' headers that the type detection reads
#define kSniffBytes 1024

struct Importer {
	UInt32 magic;
	char path[1024];
	Boolean haveFile;
	OSType type;                 // the file type ('PICT', 'JPEG', ...)
	Rect natural;
	CGrafPtr port;
	Boolean havePort;
	MatrixRecord matrix;
	Boolean haveMatrix;
	Rect bounds;
	Boolean haveBounds;
	CodecQ quality;
};

enum { kImporterMagic = 0x47524950 /* 'GRIP' */ };

static const OSType kImporterTypes[] = { 'PICT', 'JPEG', 'PNGf', 'GIFf', 'TIFF', 'BMPf' };

static struct Importer *importerOf(ComponentInstance ci)
{
	Handle storage = ci ? GetComponentInstanceStorage(ci) : NULL;
	struct Importer *importer = storage ? (struct Importer *) *storage : NULL;
	return importer && importer->magic == kImporterMagic ? importer : NULL;
}

static ComponentResult importerEntry(ComponentParameters *cp, Handle storage)
{
	switch (cp->what) {
		case kComponentOpenSelect: {
			Handle h = NewHandleClear(sizeof(struct Importer));
			if (!h)
				return memFullErr;
			struct Importer *importer = (struct Importer *) *h;
			importer->magic = kImporterMagic;
			importer->quality = codecNormalQuality;
			SetIdentityMatrix(&importer->matrix);
			SetComponentInstanceStorage((ComponentInstance) cp->params[0], h);
			return noErr;
		}
		case kComponentCloseSelect:
			if (storage)
				DisposeHandle(storage);
			return noErr;
		case kComponentVersionSelect:
			return 0x00010000;
		default:
			return badComponentSelector;
	}
}

static pthread_once_t g_registered = PTHREAD_ONCE_INIT;

static void registerImporters(void)
{
	for (size_t i = 0; i < sizeof(kImporterTypes) / sizeof(kImporterTypes[0]); i++) {
		ComponentDescription description = { GraphicsImporterComponentType, kImporterTypes[i], 'appl', 0, 0 };
		RegisterComponent(&description, importerEntry, registerComponentGlobal, NULL, NULL, NULL);
	}
}

// ---- matrices -------------------------------------------------------------------------------------------------

// The third column of a matrix is a Fract (2.30), the others Fixed (16.16)
static void toDouble(const MatrixRecord *m, double d[3][3])
{
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			d[i][j] = m->matrix[i][j] / (j == 2 ? 1073741824.0 : 65536.0);
}

static void fromDouble(const double d[3][3], MatrixRecord *m)
{
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			m->matrix[i][j] = (Fixed) (d[i][j] * (j == 2 ? 1073741824.0 : 65536.0) + (d[i][j] < 0 ? -0.5 : 0.5));
}

// next = first * second, the transformation of the first applied before that of the second
static void multiply(const double a[3][3], const double b[3][3], double out[3][3])
{
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			out[i][j] = a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j];
}

void SetIdentityMatrix(MatrixRecord *matrix)
{
	if (!matrix)
		return;
	memset(matrix, 0, sizeof(*matrix));
	matrix->matrix[0][0] = 0x10000;
	matrix->matrix[1][1] = 0x10000;
	matrix->matrix[2][2] = 0x40000000;
}

// The transformations are added to the matrix: what it did before happens first
static void concatDouble(MatrixRecord *m, const double t[3][3])
{
	double a[3][3], r[3][3];
	toDouble(m, a);
	multiply(a, t, r);
	fromDouble(r, m);
}

void TranslateMatrix(MatrixRecord *m, Fixed deltaH, Fixed deltaV)
{
	if (!m)
		return;
	double t[3][3] = { { 1, 0, 0 }, { 0, 1, 0 }, { deltaH / 65536.0, deltaV / 65536.0, 1 } };
	concatDouble(m, t);
}

void ScaleMatrix(MatrixRecord *m, Fixed scaleX, Fixed scaleY, Fixed aboutX, Fixed aboutY)
{
	if (!m)
		return;
	double sx = scaleX / 65536.0, sy = scaleY / 65536.0, ax = aboutX / 65536.0, ay = aboutY / 65536.0;
	double t[3][3] = { { sx, 0, 0 }, { 0, sy, 0 }, { ax - sx * ax, ay - sy * ay, 1 } };
	concatDouble(m, t);
}

void ConcatMatrix(const MatrixRecord *a, MatrixRecord *b)
{
	if (!a || !b)
		return;
	double x[3][3], y[3][3], r[3][3];
	toDouble(a, x);
	toDouble(b, y);
	multiply(x, y, r);
	fromDouble(r, b);
}

void RectMatrix(MatrixRecord *matrix, const Rect *srcRect, const Rect *dstRect)
{
	if (!matrix || !srcRect || !dstRect)
		return;
	double sw = srcRect->right - srcRect->left, sh = srcRect->bottom - srcRect->top;
	double sx = sw != 0 ? (dstRect->right - dstRect->left) / sw : 1, sy = sh != 0 ? (dstRect->bottom - dstRect->top) / sh : 1;
	double t[3][3] = { { sx, 0, 0 }, { 0, sy, 0 }, { dstRect->left - sx * srcRect->left, dstRect->top - sy * srcRect->top, 1 } };
	fromDouble(t, matrix);
}

// ---- pixel worlds ---------------------------------------------------------------------------------------------

OSErr QTNewGWorldFromPtr(GWorldPtr *gw, OSType pixelFormat, const Rect *boundsRect, CTabHandle cTable,
	GDHandle aGDevice, GWorldFlags flags, void *baseAddr, long rowBytes)
{
	return NewGWorldFromPtr(gw, pixelFormat, boundsRect, cTable, aGDevice, flags, (Ptr) baseAddr, (SInt32) rowBytes);
}

// ---- files ----------------------------------------------------------------------------------------------------

static Boolean readFile(const char *path, uint8_t *buffer, size_t size, size_t *got)
{
	FILE *f = fopen(path, "rb");
	if (!f)
		return false;
	*got = fread(buffer, 1, size, f);
	fclose(f);
	return true;
}

static Boolean looksLikePict(const uint8_t *b, size_t n, size_t *offset)
{
	// version 2 pictures: the size and the frame, then 0x0011 0x02FF
	for (size_t o = 0; o <= 512 && o + 14 <= n; o += 512)
		if (b[o + 10] == 0x00 && b[o + 11] == 0x11 && b[o + 12] == 0x02 && b[o + 13] == 0xFF) {
			*offset = o;
			return true;
		}
	return false;
}

static OSType typeOfFile(const char *path, size_t *pictOffset)
{
	uint8_t b[kSniffBytes];
	size_t n = 0;
	*pictOffset = 0;
	memset(b, 0xFF, sizeof(b));
	if (!readFile(path, b, sizeof(b), &n) || n < 4)
		return 0;
	if (b[0] == 0xFF && b[1] == 0xD8)
		return 'JPEG';
	if (b[0] == 0x89 && memcmp(b + 1, "PNG", 3) == 0)
		return 'PNGf';
	if (memcmp(b, "GIF8", 4) == 0)
		return 'GIFf';
	if (memcmp(b, "II*\0", 4) == 0 || memcmp(b, "MM\0*", 4) == 0)
		return 'TIFF';
	if (b[0] == 'B' && b[1] == 'M')
		return 'BMPf';
	if (looksLikePict(b, n, pictOffset))
		return 'PICT';
	return 0;
}

static Boolean pathOfSpec(const FSSpec *spec, char *path, size_t size)
{
	FSRef ref;
	if (!spec || FSpMakeFSRef(spec, &ref) != noErr)
		return false;
	return FSRefMakePath(&ref, (UInt8 *) path, (UInt32) size) == noErr;
}

// What the file holds, for the description and the bounds
struct Details {
	Rect natural;
	SInt16 depth;
	CodecType codec;
	const char *name;
	long fileSize;
};

// The picture's own pixel depth: that of the deepest pixel map it draws (QuickDraw plays the picture to find out);
// a picture of shapes only is 32 deep, as QuickTime describes it
extern short _QDPictureGetPixelDepth(PicHandle pic);

static Boolean pictDetails(const char *path, size_t offset, struct Details *d)
{
	FILE *f = fopen(path, "rb");
	long size;
	if (!f)
		return false;
	fseek(f, 0, SEEK_END);
	size = ftell(f) - (long) offset;
	if (size < 10) {
		fclose(f);
		return false;
	}
	Handle pic = NewHandle(size);
	if (!pic) {
		fclose(f);
		return false;
	}
	fseek(f, (long) offset, SEEK_SET);
	size_t got = fread(*pic, 1, (size_t) size, f);
	fclose(f);
	if ((long) got != size) {
		DisposeHandle(pic);
		return false;
	}
	const uint8_t *b = (const uint8_t *) *pic;
	d->natural.top = (short) (b[2] << 8 | b[3]);
	d->natural.left = (short) (b[4] << 8 | b[5]);
	d->natural.bottom = (short) (b[6] << 8 | b[7]);
	d->natural.right = (short) (b[8] << 8 | b[9]);
	short depth = _QDPictureGetPixelDepth((PicHandle) pic);
	d->depth = depth > 1 ? depth : 32;
	DisposeHandle(pic);
	d->codec = 'PICT';
	d->name = "Photo - JPEG";
	return true;
}

static CGImageRef copyImage(const char *path)
{
	CFURLRef url = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8 *) path, (CFIndex) strlen(path), false);
	CGImageSourceRef source = url ? CGImageSourceCreateWithURL(url, NULL) : NULL;
	CGImageRef image = source ? CGImageSourceCreateImageAtIndex(source, 0, NULL) : NULL;
	if (source)
		CFRelease(source);
	if (url)
		CFRelease(url);
	return image;
}

// QuickTime describes a gray image by the depth 32 plus its bits per pixel (an 8-bit gray image is 40 deep), a color
// image by 24, or 32 when it has an alpha channel, a palette image by its bits per pixel. The importer reads that off
// the file's own header (the image decoder may give a gray file as RGB): the frame header of a JPEG file and the
// header chunk of a PNG file. For the other formats the decoded image tells.
static SInt16 depthOfHeader(const char *path, OSType type)
{
	uint8_t b[64];
	size_t n = 0;
	if (!readFile(path, b, sizeof(b), &n))
		return 0;
	if (type == 'PNGf' && n >= 26 && memcmp(b + 12, "IHDR", 4) == 0) {
		int bits = b[24], colorType = b[25];
		switch (colorType) {
			case 0: case 4: return (SInt16) (32 + (bits > 8 ? 16 : bits));
			case 2: return 24;
			case 3: return (SInt16) bits;
			case 6: return 32;
		}
		return 0;
	}
	if (type == 'JPEG') {
		FILE *f = fopen(path, "rb");
		int c;
		SInt16 depth = 0;
		if (!f)
			return 0;
		fseek(f, 2, SEEK_SET);
		// the markers, up to the frame header: precision, height, width, the number of components
		while ((c = fgetc(f)) == 0xFF) {
			int marker = fgetc(f), length;
			uint8_t segment[8];
			if (marker == EOF)
				break;
			length = (fgetc(f) << 8) | fgetc(f);
			if (length < 2)
				break;
			if (marker >= 0xC0 && marker <= 0xCF && marker != 0xC4 && marker != 0xC8 && marker != 0xCC) {
				if (length >= 8 && fread(segment, 1, 6, f) == 6) {
					int precision = segment[0], components = segment[5];
					depth = components == 1 ? (SInt16) (32 + (precision > 8 ? 16 : precision)) : components == 4 ? 32 : 24;
				}
				break;
			}
			fseek(f, length - 2, SEEK_CUR);
		}
		fclose(f);
		return depth;
	}
	if (type == 'GIFf')
		return 8;
	return 0;
}

static SInt16 depthOfImage(const char *path, OSType type, CGImageRef image)
{
	SInt16 depth = depthOfHeader(path, type);
	if (depth)
		return depth;
	CGColorSpaceRef space = CGImageGetColorSpace(image);
	CGColorSpaceModel model = space ? CGColorSpaceGetModel(space) : kCGColorSpaceModelRGB;
	if (model == kCGColorSpaceModelMonochrome)
		return (SInt16) (32 + (CGImageGetBitsPerComponent(image) > 8 ? 16 : 8));
	if (model == kCGColorSpaceModelIndexed)
		return 8;
	CGImageAlphaInfo alphaInfo = CGImageGetAlphaInfo(image);
	Boolean hasAlpha = alphaInfo != kCGImageAlphaNone && alphaInfo != kCGImageAlphaNoneSkipFirst && alphaInfo != kCGImageAlphaNoneSkipLast;
	return hasAlpha ? 32 : 24;
}

static Boolean imageDetails(const char *path, OSType type, struct Details *d)
{
	CGImageRef image = copyImage(path);
	if (!image)
		return false;
	SetRect(&d->natural, 0, 0, (short) CGImageGetWidth(image), (short) CGImageGetHeight(image));
	d->depth = depthOfImage(path, type, image);
	switch (type) {
		case 'JPEG': d->codec = 'jpeg'; d->name = "Photo - JPEG"; break;
		case 'PNGf': d->codec = 'png '; d->name = "PNG"; break;
		case 'GIFf': d->codec = 'gif '; d->name = "GIF"; break;
		case 'TIFF': d->codec = 'tiff'; d->name = "TIFF"; break;
		default: d->codec = 'WRLE'; d->name = "BMP"; break;
	}
	CGImageRelease(image);
	return true;
}

static Boolean fileDetails(struct Importer *importer, struct Details *d)
{
	size_t pictOffset;
	memset(d, 0, sizeof(*d));
	if (!importer->haveFile)
		return false;
	OSType type = typeOfFile(importer->path, &pictOffset);
	if (!type)
		return false;
	FILE *f = fopen(importer->path, "rb");
	if (f) {
		fseek(f, 0, SEEK_END);
		d->fileSize = ftell(f);
		fclose(f);
	}
	return type == 'PICT' ? pictDetails(importer->path, pictOffset, d) : imageDetails(importer->path, type, d);
}

static OSErr openImporterForFile(const FSSpec *theFile, ComponentInstance *gi)
{
	char path[1024];
	size_t pictOffset;

	if (!gi)
		return paramErr;
	*gi = NULL;
	pthread_once(&g_registered, registerImporters);
	if (!pathOfSpec(theFile, path, sizeof(path)))
		return fnfErr;
	OSType type = typeOfFile(path, &pictOffset);
	if (!type)
		return badFileFormat;
	ComponentDescription description = { GraphicsImporterComponentType, type, 0, 0, 0 };
	Component component = FindNextComponent(NULL, &description);
	if (!component)
		return badFileFormat;
	OSErr err = OpenAComponent(component, gi);
	if (err != noErr)
		return err;
	err = (OSErr) GraphicsImportSetDataFile(*gi, theFile);
	if (err != noErr) {
		CloseComponent(*gi);
		*gi = NULL;
	}
	return err;
}

OSErr GetGraphicsImporterForFile(const FSSpec *theFile, ComponentInstance *gi)
{
	return openImporterForFile(theFile, gi);
}

OSErr GetGraphicsImporterForFileWithFlags(const FSSpec *theFile, ComponentInstance *gi, UInt32 flags)
{
	return openImporterForFile(theFile, gi);
}

ComponentResult GraphicsImportSetDataFile(ComponentInstance ci, const FSSpec *theFile)
{
	struct Importer *importer = importerOf(ci);
	char path[1024];
	size_t pictOffset;
	if (!importer)
		return badComponentInstance;
	if (!pathOfSpec(theFile, path, sizeof(path)))
		return fnfErr;
	if (!typeOfFile(path, &pictOffset))
		return badFileFormat;
	strlcpy(importer->path, path, sizeof(importer->path));
	importer->haveFile = true;
	struct Details details;
	if (!fileDetails(importer, &details))
		return badImageDescription;
	importer->natural = details.natural;
	return noErr;
}

ComponentResult GraphicsImportGetNaturalBounds(ComponentInstance ci, Rect *naturalBounds)
{
	struct Importer *importer = importerOf(ci);
	if (!importer)
		return badComponentInstance;
	if (!naturalBounds)
		return paramErr;
	if (!importer->haveFile)
		return dataNoDataRef;
	*naturalBounds = importer->natural;
	return noErr;
}

ComponentResult GraphicsImportGetImageDescription(ComponentInstance ci, ImageDescriptionHandle *desc)
{
	struct Importer *importer = importerOf(ci);
	struct Details details;
	if (!importer)
		return badComponentInstance;
	if (!desc)
		return paramErr;
	*desc = NULL;
	if (!fileDetails(importer, &details))
		return importer->haveFile ? badImageDescription : dataNoDataRef;
	Handle h = NewHandleClear(sizeof(ImageDescription));
	if (!h)
		return memFullErr;
	ImageDescription *d = (ImageDescription *) *h;
	d->idSize = sizeof(ImageDescription);
	d->cType = details.codec;
	d->version = 0;
	d->revisionLevel = 0;
	d->temporalQuality = 0;
	d->spatialQuality = codecNormalQuality;
	d->width = details.natural.right - details.natural.left;
	d->height = details.natural.bottom - details.natural.top;
	d->hRes = 72 << 16;
	d->vRes = 72 << 16;
	d->dataSize = details.fileSize;
	d->frameCount = 1;
	d->name[0] = (UInt8) strlen(details.name);
	memcpy(d->name + 1, details.name, d->name[0]);
	d->depth = details.depth;
	d->clutID = -1;
	*desc = (ImageDescriptionHandle) h;
	return noErr;
}

ComponentResult GraphicsImportSetMatrix(ComponentInstance ci, const MatrixRecord *matrix)
{
	struct Importer *importer = importerOf(ci);
	if (!importer)
		return badComponentInstance;
	if (!matrix)
		return paramErr;
	importer->matrix = *matrix;
	importer->haveMatrix = true;
	importer->haveBounds = false;
	return noErr;
}

ComponentResult GraphicsImportGetMatrix(ComponentInstance ci, MatrixRecord *matrix)
{
	struct Importer *importer = importerOf(ci);
	if (!importer)
		return badComponentInstance;
	if (!matrix)
		return paramErr;
	*matrix = importer->matrix;
	return noErr;
}

// Setting the bounds sets the matrix that maps the natural bounds onto them
ComponentResult GraphicsImportSetBoundsRect(ComponentInstance ci, const Rect *bounds)
{
	struct Importer *importer = importerOf(ci);
	if (!importer)
		return badComponentInstance;
	if (!bounds)
		return paramErr;
	importer->bounds = *bounds;
	importer->haveBounds = true;
	RectMatrix(&importer->matrix, &importer->natural, bounds);
	importer->haveMatrix = true;
	return noErr;
}

ComponentResult GraphicsImportGetBoundsRect(ComponentInstance ci, Rect *bounds)
{
	struct Importer *importer = importerOf(ci);
	if (!importer)
		return badComponentInstance;
	if (!bounds)
		return paramErr;
	double m[3][3];
	toDouble(&importer->matrix, m);
	bounds->left = (short) (importer->natural.left * m[0][0] + m[2][0] + 0.5);
	bounds->top = (short) (importer->natural.top * m[1][1] + m[2][1] + 0.5);
	bounds->right = (short) (importer->natural.right * m[0][0] + m[2][0] + 0.5);
	bounds->bottom = (short) (importer->natural.bottom * m[1][1] + m[2][1] + 0.5);
	return noErr;
}

ComponentResult GraphicsImportSetGWorld(ComponentInstance ci, CGrafPtr port, GDHandle gd)
{
	struct Importer *importer = importerOf(ci);
	if (!importer)
		return badComponentInstance;
	importer->port = port;
	importer->havePort = true;
	return noErr;
}

ComponentResult GraphicsImportGetGWorld(ComponentInstance ci, CGrafPtr *port, GDHandle *gd)
{
	struct Importer *importer = importerOf(ci);
	if (!importer)
		return badComponentInstance;
	if (port)
		*port = importer->havePort ? importer->port : NULL;
	if (gd)
		*gd = NULL;
	return noErr;
}

ComponentResult GraphicsImportSetQuality(ComponentInstance ci, CodecQ quality)
{
	struct Importer *importer = importerOf(ci);
	if (!importer)
		return badComponentInstance;
	importer->quality = quality;
	return noErr;
}

ComponentResult GraphicsImportGetQuality(ComponentInstance ci, CodecQ *quality)
{
	struct Importer *importer = importerOf(ci);
	if (!importer)
		return badComponentInstance;
	if (!quality)
		return paramErr;
	*quality = importer->quality;
	return noErr;
}

// ---- drawing --------------------------------------------------------------------------------------------------

// The rectangle of the image in the port: the natural bounds through the matrix
static OSErr destinationRect(const struct Importer *importer, Rect *dest)
{
	double m[3][3];
	toDouble(&importer->matrix, m);
	if (m[0][1] != 0 || m[1][0] != 0 || m[0][2] != 0 || m[1][2] != 0)
		return codecUnimpErr;
	double x0 = importer->natural.left * m[0][0] + m[2][0], x1 = importer->natural.right * m[0][0] + m[2][0];
	double y0 = importer->natural.top * m[1][1] + m[2][1], y1 = importer->natural.bottom * m[1][1] + m[2][1];
	dest->left = (short) (x0 < x1 ? x0 + 0.5 : x1 + 0.5);
	dest->right = (short) (x0 < x1 ? x1 + 0.5 : x0 + 0.5);
	dest->top = (short) (y0 < y1 ? y0 + 0.5 : y1 + 0.5);
	dest->bottom = (short) (y0 < y1 ? y1 + 0.5 : y0 + 0.5);
	return noErr;
}

// Draws the image of an image file into a graphics world of 32-bit pixels, over what is there
static OSErr drawImageFile(const char *path, GWorldPtr world, const Rect *dest)
{
	CGImageRef image = copyImage(path);
	if (!image)
		return badImageDescription;
	CGContextRef context = NULL;
	OSStatus status = QDBeginCGContext(world, &context);
	if (status != noErr || !context) {
		CGImageRelease(image);
		return (OSErr) status;
	}
	Rect bounds;
	GetPortBounds(world, &bounds);
	// Quartz has its origin at the bottom left of the port
	CGRect target = CGRectMake(dest->left - bounds.left, bounds.bottom - dest->bottom, dest->right - dest->left, dest->bottom - dest->top);
	CGContextSetInterpolationQuality(context, kCGInterpolationHigh);
	CGContextDrawImage(context, target, image);
	QDEndCGContext(world, &context);
	CGImageRelease(image);
	return noErr;
}

// A picture file: the picture, played by QuickDraw into the destination
static OSErr drawPictFile(const char *path, size_t offset, GWorldPtr world, const Rect *dest)
{
	FILE *f = fopen(path, "rb");
	if (!f)
		return fnfErr;
	fseek(f, 0, SEEK_END);
	long size = ftell(f) - (long) offset;
	if (size < 12) {
		fclose(f);
		return badImageDescription;
	}
	Handle pic = NewHandle(size);
	if (!pic) {
		fclose(f);
		return memFullErr;
	}
	fseek(f, (long) offset, SEEK_SET);
	size_t got = fread(*pic, 1, (size_t) size, f);
	fclose(f);
	if ((long) got != size) {
		DisposeHandle(pic);
		return ioErr;
	}
	CGrafPtr savedPort;
	GDHandle savedDevice;
	GetGWorld(&savedPort, &savedDevice);
	SetGWorld(world, NULL);
	DrawPicture((PicHandle) pic, dest);
	QDErr err = QDError();
	SetGWorld(savedPort, savedDevice);
	DisposeHandle(pic);
	return err;
}

// The graphics worlds of 32-bit pixels are drawn into directly; the others (16-bit and 8-bit gray) go through one
// of 32 bits that holds a copy of what is in the destination, which is copied back afterwards.
ComponentResult GraphicsImportDraw(ComponentInstance ci)
{
	struct Importer *importer = importerOf(ci);
	Rect dest;
	size_t pictOffset;
	if (!importer)
		return badComponentInstance;
	if (!importer->haveFile)
		return dataNoDataRef;
	CGrafPtr port = importer->havePort ? importer->port : NULL;
	if (!port) {
		GrafPtr current;
		GetPort(&current);
		port = current;
	}
	if (!port)
		return paramErr;
	OSErr err = destinationRect(importer, &dest);
	if (err != noErr)
		return err;
	OSType type = typeOfFile(importer->path, &pictOffset);
	if (!type)
		return badImageDescription;

	PixMapHandle pixels = GetPortPixMap(port);
	short depth = pixels ? GetPixDepth(pixels) : 0;
	GWorldPtr target = port, temp = NULL;
	Rect bounds;
	GetPortBounds(port, &bounds);
	if (depth != 32) {
		// QuickDraw's copy converts between the pixel formats
		if (NewGWorld(&temp, 32, &bounds, NULL, NULL, 0) != noErr || !temp)
			return memFullErr;
		CopyBits(GetPortBitMapForCopyBits(port), GetPortBitMapForCopyBits(temp), &bounds, &bounds, 0, NULL);
		target = temp;
	}
	err = type == 'PICT' ? drawPictFile(importer->path, pictOffset, target, &dest) : drawImageFile(importer->path, target, &dest);
	if (temp) {
		if (err == noErr)
			CopyBits(GetPortBitMapForCopyBits(temp), GetPortBitMapForCopyBits(port), &bounds, &bounds, 0, NULL);
		DisposeGWorld(temp);
	}
	return err;
}

#endif
