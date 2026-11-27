// Copyright (C) 2026 Lubos Dolezel
//
// QuickTime graphics importers (32-bit only, as in macOS): GetGraphicsImporterForFile opens an importer component
// for an image file, which reports the natural bounds and the image description and draws the image into a
// QuickDraw graphics world through a matrix; the matrix calls; QTNewGWorldFromPtr worlds over the caller's memory
// in the 32-bit ARGB, 16-bit 555 and 8-bit gray pixel formats. The test makes its own files: a PNG with alpha, a
// gray and a color JPEG (written with ImageIO) and a picture file with the 512 byte header (recorded with
// OpenCPicture). Reference: Image Compression Manager Reference, Graphics Importer Components.
#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>
#include <ImageIO/ImageIO.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __LP64__
int main(void)
{
	printf("PASS quicktime_graphics_importers (no 64-bit API)\n");
	return 0;
}
#else

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static char g_dir[] = "/tmp/darling-qtimport-XXXXXX";

// The file specification of a file: the volume and directory ID of its folder, and its name
static void spec_of(const char *path, FSSpec *spec)
{
	char dir[512];
	FSRef folder;
	FSCatalogInfo info;
	strlcpy(dir, path, sizeof(dir));
	*strrchr(dir, '/') = 0;
	const char *leaf = strrchr(path, '/') + 1;
	Str255 name;
	name[0] = (unsigned char) strlen(leaf);
	memcpy(name + 1, leaf, name[0]);
	if (FSPathMakeRef((const UInt8 *) dir, &folder, NULL) != noErr
	    || FSGetCatalogInfo(&folder, kFSCatInfoVolume | kFSCatInfoNodeID, &info, NULL, NULL, NULL) != noErr
	    || FSMakeFSSpec(info.volume, (SInt32) info.nodeID, name, spec) != noErr) {
		printf("FAIL: no FSSpec for %s\n", path);
		exit(2);
	}
}

// A raster image of the width and height, as a file of the type, through ImageIO
static void write_image(const char *path, CFStringRef type, int width, int height, int gray, int alpha, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	CGColorSpaceRef space = gray ? CGColorSpaceCreateDeviceGray() : CGColorSpaceCreateDeviceRGB();
	size_t rowBytes = (size_t) width * 4;
	unsigned char *bits = calloc(rowBytes, (size_t) height);
	CGContextRef context;
	if (gray) {
		context = CGBitmapContextCreate(bits, width, height, 8, rowBytes, space, kCGImageAlphaNone);
	} else {
		context = CGBitmapContextCreate(bits, width, height, 8, rowBytes, space,
			alpha ? kCGImageAlphaPremultipliedLast : kCGImageAlphaNoneSkipLast);
	}
	CGContextSetRGBFillColor(context, r / 255.0, g / 255.0, b / 255.0, a / 255.0);
	if (gray)
		CGContextSetGrayFillColor(context, r / 255.0, 1);
	CGContextFillRect(context, CGRectMake(0, 0, width, height));
	CGImageRef image = CGBitmapContextCreateImage(context);
	CFURLRef url = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8 *) path, (CFIndex) strlen(path), false);
	CGImageDestinationRef destination = CGImageDestinationCreateWithURL(url, type, 1, NULL);
	CGImageDestinationAddImage(destination, image, NULL);
	if (!CGImageDestinationFinalize(destination)) {
		printf("FAIL: cannot write %s\n", path);
		exit(2);
	}
	CFRelease(destination);
	CFRelease(url);
	CGImageRelease(image);
	CGContextRelease(context);
	CGColorSpaceRelease(space);
	free(bits);
}

// A gray JPEG file of 32 by 20 pixels (one component, 8 bits, the gray value 127 everywhere), made with a JPEG
// encoder of the host: the encoder of ImageIO writes gray images as three components
static const unsigned char kGrayJpeg[] = {
	0xff, 0xd8, 0xff, 0xe0, 0x00, 0x10, 0x4a, 0x46, 0x49, 0x46, 0x00, 0x01,
	0x01, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0xff, 0xdb, 0x00, 0x43,
	0x00, 0x03, 0x02, 0x02, 0x03, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x04,
	0x03, 0x03, 0x04, 0x05, 0x08, 0x05, 0x05, 0x04, 0x04, 0x05, 0x0a, 0x07,
	0x07, 0x06, 0x08, 0x0c, 0x0a, 0x0c, 0x0c, 0x0b, 0x0a, 0x0b, 0x0b, 0x0d,
	0x0e, 0x12, 0x10, 0x0d, 0x0e, 0x11, 0x0e, 0x0b, 0x0b, 0x10, 0x16, 0x10,
	0x11, 0x13, 0x14, 0x15, 0x15, 0x15, 0x0c, 0x0f, 0x17, 0x18, 0x16, 0x14,
	0x18, 0x12, 0x14, 0x15, 0x14, 0xff, 0xc0, 0x00, 0x0b, 0x08, 0x00, 0x14,
	0x00, 0x20, 0x01, 0x01, 0x11, 0x00, 0xff, 0xc4, 0x00, 0x15, 0x00, 0x01,
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0xc4, 0x00, 0x14, 0x10, 0x01, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xff, 0xda, 0x00, 0x08, 0x01, 0x01, 0x00, 0x00, 0x3f,
	0x00, 0x80, 0x00, 0x00, 0x1f, 0xff, 0xd9,
};

static void write_gray_jpeg(const char *path)
{
	FILE *f = fopen(path, "wb");
	fwrite(kGrayJpeg, 1, sizeof(kGrayJpeg), f);
	fclose(f);
}

// A picture file: 512 bytes of zeros, then a picture that paints a green rectangle on a white one
static void write_pict(const char *path, int width, int height)
{
	Rect frame = { 0, 0, (short) height, (short) width };
	OpenCPicParams params = { frame, 72 << 16, 72 << 16, -2, 0, 0 };
	GWorldPtr world = NULL;
	CGrafPtr savedPort;
	GDHandle savedDevice;
	NewGWorld(&world, 32, &frame, NULL, NULL, 0);
	GetGWorld(&savedPort, &savedDevice);
	SetGWorld(world, NULL);
	PicHandle picture = OpenCPicture(&params);
	RGBColor white = { 0xFFFF, 0xFFFF, 0xFFFF }, green = { 0, 0xFFFF, 0 };
	RGBForeColor(&white);
	PaintRect(&frame);
	RGBForeColor(&green);
	Rect inner = { 0, 0, (short) (height / 2), (short) width };
	PaintRect(&inner);
	ClosePicture();
	SetGWorld(savedPort, savedDevice);
	DisposeGWorld(world);
	FILE *f = fopen(path, "wb");
	char zeros[512];
	memset(zeros, 0, sizeof(zeros));
	fwrite(zeros, 1, sizeof(zeros), f);
	fwrite(*picture, 1, (size_t) GetHandleSize((Handle) picture), f);
	fclose(f);
	DisposeHandle((Handle) picture);
}

// A picture file whose only drawing is an 8 by 8 pixel map of 16 bits per pixel (a version 2 picture with the
// DirectBitsRect opcode, rows not packed), written byte by byte as the picture format of QuickDraw lays it out
static unsigned char *put16(unsigned char *p, unsigned v) { *p++ = (unsigned char) (v >> 8); *p++ = (unsigned char) v; return p; }
static unsigned char *put32(unsigned char *p, unsigned v) { return put16(put16(p, v >> 16), v & 0xFFFF); }
static unsigned char *putRect(unsigned char *p, int t, int l, int b, int r) { return put16(put16(put16(put16(p, t), l), b), r); }

static void write_pict16(const char *path)
{
	unsigned char data[512 + 400];
	unsigned char *p = data + 512;
	memset(data, 0, sizeof(data));
	p = put16(p, 0);                       // size (not used)
	p = putRect(p, 0, 0, 8, 8);            // frame
	p = put16(p, 0x0011); p = put16(p, 0x02FF);
	p = put16(p, 0x0C00);                  // HeaderOp
	p = put16(p, 0xFFFE); p = put16(p, 0); // version -2
	p = put32(p, 0x00480000); p = put32(p, 0x00480000);
	p = putRect(p, 0, 0, 8, 8); p = put32(p, 0);
	p = put16(p, 0x009A);                  // DirectBitsRect
	p = put32(p, 0xFF);                    // base address
	p = put16(p, 0x8000 | 16);             // row bytes
	p = putRect(p, 0, 0, 8, 8);            // bounds
	p = put16(p, 0); p = put16(p, 1); p = put32(p, 0); // pmVersion, packType (none), packSize
	p = put32(p, 0x00480000); p = put32(p, 0x00480000);
	p = put16(p, 16); p = put16(p, 16); p = put16(p, 3); p = put16(p, 5); // pixel type, 16 bits, 3 components of 5
	p = put32(p, 0); p = put32(p, 0); p = put32(p, 0);
	p = putRect(p, 0, 0, 8, 8); p = putRect(p, 0, 0, 8, 8); p = put16(p, 0); // source, destination, copy
	p += 8 * 16;                           // the pixels: black
	p = put16(p, 0x00FF);                  // OpEndPic
	FILE *f = fopen(path, "wb");
	fwrite(data, 1, (size_t) (p - data), f);
	fclose(f);
}

static void pixel32(const unsigned char *bits, long rowBytes, int h, int v, int *r, int *g, int *b, int *a)
{
	const unsigned char *p = bits + v * rowBytes + h * 4;
	*a = p[0];
	*r = p[1];
	*g = p[2];
	*b = p[3];
}

static int near(int value, int expected, int tolerance)
{
	return value >= expected - tolerance && value <= expected + tolerance;
}

static void matrices(void)
{
	MatrixRecord m;
	SetIdentityMatrix(&m);
	CHECK(m.matrix[0][0] == 0x10000 && m.matrix[1][1] == 0x10000 && m.matrix[2][2] == 0x40000000);
	CHECK(m.matrix[0][1] == 0 && m.matrix[1][0] == 0 && m.matrix[2][0] == 0 && m.matrix[2][1] == 0 && m.matrix[0][2] == 0 && m.matrix[1][2] == 0);
	ScaleMatrix(&m, X2Fix(2.0), X2Fix(3.0), 0, 0);
	CHECK(m.matrix[0][0] == 0x20000 && m.matrix[1][1] == 0x30000 && m.matrix[2][0] == 0 && m.matrix[2][1] == 0);
	TranslateMatrix(&m, X2Fix(10.0), X2Fix(-4.0));
	CHECK(m.matrix[2][0] == 0xA0000 && m.matrix[2][1] == -0x40000);
	// scaling about a point leaves that point where it is: x' = 2x - 5 for the point 5
	SetIdentityMatrix(&m);
	ScaleMatrix(&m, X2Fix(2.0), X2Fix(2.0), X2Fix(5.0), X2Fix(7.0));
	CHECK(m.matrix[2][0] == -0x50000 && m.matrix[2][1] == -0x70000);
	// RectMatrix maps one rectangle onto another
	Rect from = { 0, 0, 10, 20 }, to = { 5, 10, 25, 50 };
	RectMatrix(&m, &from, &to);
	CHECK(m.matrix[0][0] == 0x20000 && m.matrix[1][1] == 0x20000 && m.matrix[2][0] == 10 * 0x10000 && m.matrix[2][1] == 5 * 0x10000);
	// ConcatMatrix(a, b): b = a then b
	MatrixRecord a, b;
	SetIdentityMatrix(&a);
	TranslateMatrix(&a, X2Fix(3.0), 0);
	SetIdentityMatrix(&b);
	ScaleMatrix(&b, X2Fix(2.0), X2Fix(2.0), 0, 0);
	ConcatMatrix(&a, &b);
	CHECK(b.matrix[0][0] == 0x20000 && b.matrix[2][0] == 0x60000);
}

static void worlds(void)
{
	unsigned char buffer[16 * 40];
	Rect r = { 0, 0, 8, 8 };
	GWorldPtr world = NULL;
	memset(buffer, 0x5A, sizeof(buffer));
	CHECK(QTNewGWorldFromPtr(&world, k32ARGBPixelFormat, &r, NULL, NULL, 0, buffer, 40) == noErr && world);
	CHECK(GetPixBaseAddr(GetGWorldPixMap(world)) == (Ptr) buffer);
	CHECK(GetPixRowBytes(GetGWorldPixMap(world)) == 40);
	CHECK(GetPixDepth(GetGWorldPixMap(world)) == 32);
	// the caller's memory is not cleared, and is not freed by the world
	CHECK(buffer[0] == 0x5A);
	DisposeGWorld(world);
	CHECK(buffer[0] == 0x5A);
	CHECK(QTNewGWorldFromPtr(&world, k16BE555PixelFormat, &r, NULL, NULL, 0, buffer, 16) == noErr && world);
	CHECK(GetPixDepth(GetGWorldPixMap(world)) == 16);
	DisposeGWorld(world);
	CHECK(QTNewGWorldFromPtr(&world, k8IndexedGrayPixelFormat, &r, NULL, NULL, 0, buffer, 8) == noErr && world);
	CHECK(GetPixDepth(GetGWorldPixMap(world)) == 8);
	DisposeGWorld(world);
	// rows that do not hold the pixels, a missing buffer and an unknown format
	CHECK(QTNewGWorldFromPtr(&world, k32ARGBPixelFormat, &r, NULL, NULL, 0, buffer, 31) != noErr && world == NULL);
	CHECK(QTNewGWorldFromPtr(&world, k32ARGBPixelFormat, &r, NULL, NULL, 0, NULL, 32) != noErr);
	CHECK(QTNewGWorldFromPtr(&world, 'zzzz', &r, NULL, NULL, 0, buffer, 32) != noErr);
}

int main(void)
{
	if (!mkdtemp(g_dir))
		return 2;
	char png[512], jpegGray[512], jpegColor[512], pict[512], text[512];
	snprintf(png, sizeof(png), "%s/alpha.png", g_dir);
	snprintf(jpegGray, sizeof(jpegGray), "%s/gray.jpg", g_dir);
	snprintf(jpegColor, sizeof(jpegColor), "%s/color.jpg", g_dir);
	snprintf(pict, sizeof(pict), "%s/shape.pct", g_dir);
	snprintf(text, sizeof(text), "%s/text.txt", g_dir);
	write_image(png, CFSTR("public.png"), 24, 16, 0, 1, 200, 40, 20, 255);
	write_gray_jpeg(jpegGray);
	write_image(jpegColor, CFSTR("public.jpeg"), 32, 20, 0, 0, 20, 160, 240, 255);
	write_pict(pict, 30, 20);
	FILE *f = fopen(text, "w");
	fputs("not an image\n", f);
	fclose(f);

	matrices();
	worlds();

	FSSpec spec;
	ComponentInstance importer = NULL;

	// PNG with alpha: 32 deep, drawn exactly
	spec_of(png, &spec);
	CHECK(GetGraphicsImporterForFile(&spec, &importer) == noErr && importer);
	Rect natural;
	CHECK(GraphicsImportGetNaturalBounds(importer, &natural) == noErr);
	CHECK(natural.left == 0 && natural.top == 0 && natural.right == 24 && natural.bottom == 16);
	ImageDescriptionHandle description = NULL;
	CHECK(GraphicsImportGetImageDescription(importer, &description) == noErr && description);
	if (description) {
		CHECK((**description).idSize == sizeof(ImageDescription) && (**description).cType == 'png ');
		CHECK((**description).width == 24 && (**description).height == 16 && (**description).depth == 32 && (**description).frameCount == 1);
		DisposeHandle((Handle) description);
	}
	Rect bounds = { 0, 0, 16, 24 };
	unsigned char *bits = calloc(1, 24 * 4 * 16);
	GWorldPtr world = NULL;
	CHECK(QTNewGWorldFromPtr(&world, k32ARGBPixelFormat, &bounds, NULL, NULL, 0, bits, 24 * 4) == noErr);
	CHECK(GraphicsImportSetGWorld(importer, world, NULL) == noErr);
	MatrixRecord matrix;
	SetIdentityMatrix(&matrix);
	CHECK(GraphicsImportSetMatrix(importer, &matrix) == noErr);
	CHECK(GraphicsImportSetQuality(importer, codecLosslessQuality) == noErr);
	CodecQ quality = 0;
	CHECK(GraphicsImportGetQuality(importer, &quality) == noErr && quality == codecLosslessQuality);
	CHECK(GraphicsImportDraw(importer) == noErr);
	int r, g, b, a;
	pixel32(bits, 24 * 4, 12, 8, &r, &g, &b, &a);
	CHECK(a == 255 && near(r, 200, 1) && near(g, 40, 1) && near(b, 20, 1));
	DisposeGWorld(world);

	// scaled to twice the size by the matrix: the image covers the larger port
	Rect big = { 0, 0, 32, 48 };
	unsigned char *bigBits = calloc(1, 48 * 4 * 32);
	CHECK(QTNewGWorldFromPtr(&world, k32ARGBPixelFormat, &big, NULL, NULL, 0, bigBits, 48 * 4) == noErr);
	SetIdentityMatrix(&matrix);
	ScaleMatrix(&matrix, X2Fix(2.0), X2Fix(2.0), 0, 0);
	CHECK(GraphicsImportSetGWorld(importer, world, NULL) == noErr);
	CHECK(GraphicsImportSetMatrix(importer, &matrix) == noErr);
	CHECK(GraphicsImportDraw(importer) == noErr);
	pixel32(bigBits, 48 * 4, 1, 1, &r, &g, &b, &a);
	CHECK(near(r, 200, 1) && near(g, 40, 1));
	pixel32(bigBits, 48 * 4, 46, 30, &r, &g, &b, &a);
	CHECK(near(r, 200, 1) && near(g, 40, 1));
	Rect placed;
	CHECK(GraphicsImportGetBoundsRect(importer, &placed) == noErr && placed.right == 48 && placed.bottom == 32);
	DisposeGWorld(world);
	free(bigBits);

	// a matrix that rotates is not supported
	MatrixRecord rotated = matrix;
	rotated.matrix[0][1] = 0x8000;
	CHECK(GraphicsImportSetMatrix(importer, &rotated) == noErr);
	CHECK(GraphicsImportDraw(importer) == codecUnimpErr);

	// the calls refuse what they cannot work with
	CHECK(GraphicsImportGetNaturalBounds(importer, NULL) == paramErr);
	CHECK(GraphicsImportGetNaturalBounds(NULL, &natural) != noErr);
	CHECK(CloseComponent(importer) == noErr);
	CHECK(GraphicsImportGetNaturalBounds(importer, &natural) != noErr);
	free(bits);

	// the gray JPEG is described as 40 deep (32 and the 8 bits of gray) and drawn into an 8-bit gray world, in
	// which 0 is white: the gray 127 is stored as about 128
	spec_of(jpegGray, &spec);
	CHECK(GetGraphicsImporterForFile(&spec, &importer) == noErr && importer);
	CHECK(GraphicsImportGetImageDescription(importer, &description) == noErr && description);
	if (description) {
		CHECK((**description).cType == 'jpeg' && (**description).depth == 40 && (**description).width == 32 && (**description).height == 20);
		DisposeHandle((Handle) description);
	}
	Rect grayBounds = { 0, 0, 20, 32 };
	unsigned char *grayBits = calloc(1, 40 * 20);
	CHECK(QTNewGWorldFromPtr(&world, k8IndexedGrayPixelFormat, &grayBounds, NULL, NULL, 0, grayBits, 40) == noErr);
	SetIdentityMatrix(&matrix);
	CHECK(GraphicsImportSetMatrix(importer, &matrix) == noErr);
	CHECK(GraphicsImportSetGWorld(importer, world, NULL) == noErr);
	CHECK(GraphicsImportDraw(importer) == noErr);
	CHECK(near(grayBits[10 * 40 + 16], 128, 3));
	CHECK(near(grayBits[0], 128, 3));
	CHECK(grayBits[10 * 40 + 36] == 0); // beyond the row's pixels: not touched
	DisposeGWorld(world);
	free(grayBits);
	CHECK(CloseComponent(importer) == noErr);

	// the color JPEG is 24 deep; into a 16-bit world: 5 bits per component
	spec_of(jpegColor, &spec);
	CHECK(GetGraphicsImporterForFile(&spec, &importer) == noErr && importer);
	CHECK(GraphicsImportGetImageDescription(importer, &description) == noErr && description);
	if (description) {
		CHECK((**description).depth == 24);
		DisposeHandle((Handle) description);
	}
	unsigned char *bits16 = calloc(1, 64 * 20);
	CHECK(QTNewGWorldFromPtr(&world, k16BE555PixelFormat, &grayBounds, NULL, NULL, 0, bits16, 64) == noErr);
	CHECK(GraphicsImportSetMatrix(importer, &matrix) == noErr);
	CHECK(GraphicsImportSetGWorld(importer, world, NULL) == noErr);
	CHECK(GraphicsImportDraw(importer) == noErr);
	unsigned v = (unsigned) bits16[10 * 64 + 32] << 8 | bits16[10 * 64 + 33];
	CHECK(near((int) ((v >> 10) & 31), 20 * 31 / 255, 1) && near((int) ((v >> 5) & 31), 160 * 31 / 255, 1) && near((int) (v & 31), 240 * 31 / 255, 1));
	DisposeGWorld(world);
	free(bits16);
	CHECK(CloseComponent(importer) == noErr);

	// a picture file with the header: the frame is the natural bounds, the drawing plays into the world
	spec_of(pict, &spec);
	CHECK(GetGraphicsImporterForFile(&spec, &importer) == noErr && importer);
	CHECK(GraphicsImportGetNaturalBounds(importer, &natural) == noErr && natural.right == 30 && natural.bottom == 20 && natural.left == 0 && natural.top == 0);
	CHECK(GraphicsImportGetImageDescription(importer, &description) == noErr && description);
	if (description) {
		CHECK((**description).cType == 'PICT' && (**description).width == 30 && (**description).height == 20);
		DisposeHandle((Handle) description);
	}
	Rect pictBounds = { 0, 0, 20, 30 };
	unsigned char *pictBits = calloc(1, 30 * 4 * 20);
	CHECK(QTNewGWorldFromPtr(&world, k32ARGBPixelFormat, &pictBounds, NULL, NULL, 0, pictBits, 30 * 4) == noErr);
	CHECK(GraphicsImportSetGWorld(importer, world, NULL) == noErr);
	SetIdentityMatrix(&matrix);
	CHECK(GraphicsImportSetMatrix(importer, &matrix) == noErr);
	CHECK(GraphicsImportDraw(importer) == noErr);
	pixel32(pictBits, 30 * 4, 15, 5, &r, &g, &b, &a);
	CHECK(r == 0 && g == 255 && b == 0);
	pixel32(pictBits, 30 * 4, 15, 15, &r, &g, &b, &a);
	CHECK(r == 255 && g == 255 && b == 255);
	DisposeGWorld(world);
	free(pictBits);
	CHECK(CloseComponent(importer) == noErr);

	// the depth of a picture is that of its deepest pixel map: 16 for a 16-bit one, 32 for one of shapes
	char pict16[512];
	snprintf(pict16, sizeof(pict16), "%s/deep16.pct", g_dir);
	write_pict16(pict16);
	spec_of(pict16, &spec);
	CHECK(GetGraphicsImporterForFile(&spec, &importer) == noErr && importer);
	CHECK(GraphicsImportGetImageDescription(importer, &description) == noErr && description);
	if (description) {
		CHECK((**description).cType == 'PICT' && (**description).depth == 16);
		DisposeHandle((Handle) description);
	}
	CHECK(CloseComponent(importer) == noErr);
	spec_of(pict, &spec);
	CHECK(GetGraphicsImporterForFile(&spec, &importer) == noErr && importer);
	CHECK(GraphicsImportGetImageDescription(importer, &description) == noErr && description);
	if (description) {
		CHECK((**description).depth == 32);
		DisposeHandle((Handle) description);
	}
	CHECK(CloseComponent(importer) == noErr);

	// what is not an image, and what is not there
	spec_of(text, &spec);
	importer = NULL;
	CHECK(GetGraphicsImporterForFile(&spec, &importer) != noErr && importer == NULL);
	CHECK(GetGraphicsImporterForFile(&spec, NULL) == paramErr);
	FSSpec missing = spec;
	missing.name[0] = 1;
	missing.name[1] = '~';
	CHECK(GetGraphicsImporterForFile(&missing, &importer) != noErr && importer == NULL);

	char command[600];
	snprintf(command, sizeof(command), "rm -rf %s", g_dir);
	if (system(command) != 0)
		printf("note: left %s\n", g_dir);
	printf(g_failures ? "FAIL quicktime_graphics_importers\n" : "PASS quicktime_graphics_importers\n");
	return g_failures != 0;
}
#endif
