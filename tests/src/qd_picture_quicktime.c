// Copyright (C) 2026 Lubos Dolezel
//
// QuickDraw version 2 pictures with a QuickTime image (opcode 0x8200, CompressedQuickTime; 32-bit only): the image
// of a format that ImageIO reads (here TIFF and PNG, written by the test with ImageIO) is drawn at the source rectangle
// of the record; an unknown codec or a record that is cut short makes the picture unsupported (pictureDataErr).
// Every check reads back real pixels of a GWorld.
#include <QD/QD.h>
#include <CarbonCore/MacMemory.h>
#include <ApplicationServices/ApplicationServices.h>
#include <stdio.h>
#include <string.h>

void RGBBackColor(const RGBColor *);
void EraseRect(const Rect *);
void GetCPixel(short, short, RGBColor *);
void DrawPicture(Handle, const Rect *);

static int g_failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL %d: %s\n", __LINE__, #c); g_failures++; } } while (0)

static const RGBColor red = {0xFFFF, 0, 0}, blue = {0, 0, 0xFFFF}, white = {0xFFFF, 0xFFFF, 0xFFFF};

static int same(short h, short v, RGBColor want)
{
	RGBColor c;
	GetCPixel(h, v, &c);
	return (c.red >> 8) == (want.red >> 8) && (c.green >> 8) == (want.green >> 8) && (c.blue >> 8) == (want.blue >> 8);
}

// 2x1 image: a red and a blue pixel, encoded with the given ImageIO type
static CFDataRef makeImage(CFStringRef type)
{
	static const unsigned char pixels[8] = { 255, 0, 0, 255, 0, 0, 255, 255 };
	CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
	CGContextRef context = CGBitmapContextCreate((void *) pixels, 2, 1, 8, 8, space, kCGImageAlphaPremultipliedLast);
	CGImageRef image = CGBitmapContextCreateImage(context);
	CFMutableDataRef data = CFDataCreateMutable(NULL, 0);
	CGImageDestinationRef destination = CGImageDestinationCreateWithData(data, type, 1, NULL);

	CGImageDestinationAddImage(destination, image, NULL);
	CGImageDestinationFinalize(destination);
	CFRelease(destination);
	CGImageRelease(image);
	CGContextRelease(context);
	CGColorSpaceRelease(space);
	return data;
}

static void put16(unsigned char **p, unsigned v) { *(*p)++ = v >> 8; *(*p)++ = v & 255; }
static void put32(unsigned char **p, unsigned v) { put16(p, v >> 16); put16(p, v & 0xFFFF); }

// A picture with one CompressedQuickTime record (frame 0,0,1,2) with an image description of the codec `codec`
// (ImageIO recognises the format from the data); `cut` bytes are dropped from the end
static Handle makePicture(const unsigned char *image, size_t imageSize, const char *codec, int cut)
{
	unsigned char *bytes = calloc(1, 512 + imageSize), *p = bytes;
	enum { kDescriptionSize = 86 };

	put16(&p, 0); put16(&p, 0); put16(&p, 0); put16(&p, 1); put16(&p, 2);            // size, frame
	put16(&p, 0x0011); put16(&p, 0x02FF);                                              // version 2
	put16(&p, 0x0C00); for (int i = 0; i < 12; i++) put16(&p, 0);                      // header
	put16(&p, 0x001F); put16(&p, 0x8000); put16(&p, 0x8000); put16(&p, 0x8000);      // OpColor, as pictures made by QuickTime-aware tools have before the image
	put16(&p, 0x0010); put16(&p, 30); put16(&p, 30); put16(&p, 217); put16(&p, 217);  // TxRatio
	put16(&p, 0x8200);
	put32(&p, (unsigned) (2 + 36 + 4 + 8 + 2 + 8 + 4 + 4 + kDescriptionSize + imageSize));
	put16(&p, 0);                                                                      // version
	put32(&p, 0x10000); put32(&p, 0); put32(&p, 0); put32(&p, 0); put32(&p, 0x10000); put32(&p, 0); put32(&p, 0); put32(&p, 0); put32(&p, 0x40000000); // identity matrix
	put32(&p, 0);                                                                      // matte size
	for (int i = 0; i < 4; i++) put16(&p, 0);                                          // matte rectangle
	put16(&p, 0x40);                                                                   // transfer mode
	put16(&p, 0); put16(&p, 0); put16(&p, 1); put16(&p, 2);                            // source rectangle
	put32(&p, 0x300);                                                                  // accuracy
	put32(&p, 0);                                                                      // mask size: no mask
	unsigned char *description = p;             // the image description: its size, codec, width, height and data size
	put32(&p, kDescriptionSize);
	memcpy(p, codec, 4); p += 4;
	p = description + 32;
	put16(&p, 2); put16(&p, 1);
	p = description + 44;
	put32(&p, (unsigned) imageSize);
	p = description + kDescriptionSize;
	memcpy(p, image, imageSize); p += imageSize;
	put16(&p, 0x00FF);
	size_t size = (size_t) (p - bytes) - (size_t) cut;
	Handle h = NewHandle((Size) size);
	memcpy(*h, bytes, size);
	free(bytes);
	return h;
}

int main(void)
{
	Rect r, b = {0, 0, 32, 32};
	GWorldPtr g = NULL;

	CHECK(NewGWorld(&g, 32, &b, NULL, NULL, 0) == noErr);
	SetPort(g);
	RGBBackColor(&white);
	EraseRect(&b);

	CFStringRef types[2] = { CFSTR("public.tiff"), CFSTR("public.png") };
	const char *codecs[2] = { "tiff", "png " };

	for (int i = 0; i < 2; i++)
	{
		CFDataRef image = makeImage(types[i]);
		Handle h = makePicture(CFDataGetBytePtr(image), (size_t) CFDataGetLength(image), codecs[i], 0);

		EraseRect(&b);
		SetRect(&r, 20, 20, 22, 21);
		DrawPicture(h, &r);
		CHECK(QDError() == noErr);
		CHECK(same(20, 20, red) && same(21, 20, blue));
		CHECK(same(19, 20, white) && same(22, 20, white) && same(20, 21, white));
		// the picture is scaled to the destination rectangle
		EraseRect(&b);
		SetRect(&r, 10, 10, 14, 12);
		DrawPicture(h, &r);
		CHECK(QDError() == noErr);
		CHECK(same(10, 10, red) && same(11, 11, red) && same(12, 10, blue) && same(13, 11, blue));
		DisposeHandle(h);

		// the record is cut short: not drawn, an error
		h = makePicture(CFDataGetBytePtr(image), (size_t) CFDataGetLength(image), codecs[i], 40);
		EraseRect(&b);
		SetRect(&r, 20, 20, 22, 21);
		DrawPicture(h, &r);
		CHECK(QDError() == -11005);
		CHECK(same(20, 20, white));
		DisposeHandle(h);
		CFRelease(image);
	}
	// data that is no image of a format ImageIO reads
	{
		static const unsigned char garbage[64] = { 1, 2, 3, 4, 5, 6, 7, 8 };
		Handle h = makePicture(garbage, sizeof(garbage), "rle ", 0);

		EraseRect(&b);
		SetRect(&r, 20, 20, 22, 21);
		DrawPicture(h, &r);
		CHECK(QDError() == -11005);
		CHECK(same(20, 20, white));
		DisposeHandle(h);
	}
	DisposeGWorld(g);
	printf("%s\n", g_failures ? "FAILED" : "PASSED");
	return g_failures != 0;
}
