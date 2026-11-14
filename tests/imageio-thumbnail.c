// CGImageSourceCreateThumbnailAtIndex, following the documented options:
//  - with none of the kCGImageSourceCreateThumbnailFromImage* options it only returns a
//    thumbnail stored in the file, and a PNG has none, so it returns NULL;
//  - kCGImageSourceThumbnailMaxPixelSize bounds the longer side, keeping the aspect
//    ratio; without it the thumbnail may be as big as the image, and it is a maximum,
//    so a smaller image is not scaled up;
//  - kCGImageSourceCreateThumbnailWithTransform applies the image's orientation
//    (here the TIFF Orientation tag, 6: the stored image is shown rotated 90 degrees
//    clockwise), which CGImageSourceCreateImageAtIndex does not.
// The PNG (ISO/IEC 15948) and TIFF (TIFF 6.0) inputs are built here.
//   clang -framework CoreFoundation -framework ImageIO -framework CoreGraphics tests/imageio-thumbnail.c -o /tmp/imageio-thumbnail
// Exits non-zero on failure.
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- A 4x2 PNG: left half red, right half blue.

enum { PW = 4, PH = 2 };

static uint32_t crc32Update(uint32_t crc, const uint8_t *bytes, size_t length) {
	crc = ~crc;
	for (size_t i = 0; i < length; i++) {
		crc ^= bytes[i];
		for (int k = 0; k < 8; k++)
			crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1)));
	}
	return ~crc;
}

static size_t putBE32(uint8_t *p, uint32_t v) {
	p[0] = v >> 24; p[1] = v >> 16; p[2] = v >> 8; p[3] = v;
	return 4;
}

static size_t chunk(uint8_t *out, const char *type, const uint8_t *data, uint32_t length) {
	size_t n = putBE32(out, length);
	memcpy(out + n, type, 4);
	memcpy(out + n + 4, data, length);
	n += 4 + length;
	return n + putBE32(out + n, crc32Update(0, out + 4, 4 + length));
}

static size_t buildPNG(uint8_t *out) {
	static const uint8_t signature[8] = { 0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n' };
	uint8_t ihdr[13] = { 0 };
	putBE32(ihdr, PW);
	putBE32(ihdr + 4, PH);
	ihdr[8] = 8;
	ihdr[9] = 2;

	uint8_t raw[PH * (1 + PW * 3)];
	for (int y = 0; y < PH; y++) {
		uint8_t *row = raw + y * (1 + PW * 3);
		row[0] = 0;
		for (int x = 0; x < PW; x++) {
			row[1 + x * 3] = x < PW / 2 ? 255 : 0;
			row[2 + x * 3] = 0;
			row[3 + x * 3] = x < PW / 2 ? 0 : 255;
		}
	}
	uint8_t idat[2 + 5 + sizeof(raw) + 4];
	size_t n = 0;
	idat[n++] = 0x78;
	idat[n++] = 0x01;
	idat[n++] = 0x01;
	idat[n++] = sizeof(raw) & 0xFF;
	idat[n++] = sizeof(raw) >> 8;
	idat[n++] = ~sizeof(raw) & 0xFF;
	idat[n++] = (~sizeof(raw) >> 8) & 0xFF;
	memcpy(idat + n, raw, sizeof(raw));
	n += sizeof(raw);
	uint32_t a = 1, b = 0;
	for (size_t i = 0; i < sizeof(raw); i++) {
		a = (a + raw[i]) % 65521;
		b = (b + a) % 65521;
	}
	n += putBE32(idat + n, (b << 16) | a);

	size_t length = 8;
	memcpy(out, signature, 8);
	length += chunk(out + length, "IHDR", ihdr, sizeof(ihdr));
	length += chunk(out + length, "IDAT", idat, n);
	length += chunk(out + length, "IEND", NULL, 0);
	return length;
}

// --- A 2x1 TIFF, red then blue, Orientation 6.

static size_t putLE16(uint8_t *p, uint16_t v) { p[0] = v; p[1] = v >> 8; return 2; }
static size_t putLE32(uint8_t *p, uint32_t v) { p[0] = v; p[1] = v >> 8; p[2] = v >> 16; p[3] = v >> 24; return 4; }

static size_t buildTIFF(uint8_t *out) {
	static const uint16_t entries[11][3] = {
		// tag, type (3 SHORT, 4 LONG), value; every count is 1 but BitsPerSample's
		{ 256, 4, 2 }, { 257, 4, 1 }, { 258, 3, 0 }, { 259, 3, 1 }, { 262, 3, 2 }, { 273, 4, 0 },
		{ 274, 3, 6 }, { 277, 3, 3 }, { 278, 4, 1 }, { 279, 4, 6 }, { 284, 3, 1 },
	};
	const uint32_t bitsOffset = 8 + 2 + 11 * 12 + 4, pixelsOffset = bitsOffset + 6;
	size_t p = 0;
	out[p++] = 'I';
	out[p++] = 'I';
	p += putLE16(out + p, 42);
	p += putLE32(out + p, 8);
	p += putLE16(out + p, 11);
	for (int i = 0; i < 11; i++) {
		uint16_t tag = entries[i][0], type = entries[i][1];
		uint32_t value = tag == 258 ? bitsOffset : tag == 273 ? pixelsOffset : entries[i][2];
		p += putLE16(out + p, tag);
		p += putLE16(out + p, type);
		p += putLE32(out + p, tag == 258 ? 3 : 1);
		if (type == 3 && tag != 258) {
			p += putLE16(out + p, value);
			p += putLE16(out + p, 0);
		} else {
			p += putLE32(out + p, value);
		}
	}
	p += putLE32(out + p, 0);
	for (int i = 0; i < 3; i++)
		p += putLE16(out + p, 8);
	static const uint8_t pixels[6] = { 255, 0, 0, 0, 0, 255 };
	memcpy(out + p, pixels, sizeof(pixels));
	return p + sizeof(pixels);
}

// --- Helpers.

static int failures;

static CGImageRef thumbnail(const uint8_t *bytes, size_t length, CFDictionaryRef options) {
	CFDataRef data = CFDataCreate(NULL, bytes, length);
	CGImageSourceRef source = CGImageSourceCreateWithData(data, NULL);
	CFRelease(data);
	if (source == NULL)
		return NULL;
	CGImageRef image = CGImageSourceCreateThumbnailAtIndex(source, 0, options);
	CFRelease(source);
	return image;
}

static CFDictionaryRef options(int fromImage, int maxPixelSize, int withTransform) {
	CFMutableDictionaryRef dictionary = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks,
								      &kCFTypeDictionaryValueCallBacks);
	if (fromImage)
		CFDictionarySetValue(dictionary, kCGImageSourceCreateThumbnailFromImageAlways, kCFBooleanTrue);
	if (maxPixelSize) {
		CFNumberRef number = CFNumberCreate(NULL, kCFNumberIntType, &maxPixelSize);
		CFDictionarySetValue(dictionary, kCGImageSourceThumbnailMaxPixelSize, number);
		CFRelease(number);
	}
	if (withTransform)
		CFDictionarySetValue(dictionary, kCGImageSourceCreateThumbnailWithTransform, kCFBooleanTrue);
	return dictionary;
}

// Checks the size and, if colours is not NULL, the R, G, B of each pixel (row 0 at the top).
static void expect(const char *what, CGImageRef image, size_t width, size_t height, const uint8_t *colours) {
	if (image == NULL) {
		printf("FAIL: %s: no thumbnail\n", what);
		failures++;
		return;
	}
	if (CGImageGetWidth(image) != width || CGImageGetHeight(image) != height) {
		printf("FAIL: %s: %zux%zu, expected %zux%zu\n", what, CGImageGetWidth(image), CGImageGetHeight(image), width, height);
		failures++;
	} else if (colours != NULL) {
		uint8_t *rgba = calloc(width * height, 4);
		CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
		CGContextRef context = CGBitmapContextCreate(rgba, width, height, 8, width * 4, space,
							     (CGBitmapInfo)kCGImageAlphaPremultipliedLast);
		CGColorSpaceRelease(space);
		CGContextDrawImage(context, CGRectMake(0, 0, width, height), image);
		CGContextRelease(context);
		for (size_t i = 0; i < width * height; i++) {
			const uint8_t *got = rgba + i * 4, *want = colours + i * 3;
			if (abs(got[0] - want[0]) > 8 || abs(got[1] - want[1]) > 8 || abs(got[2] - want[2]) > 8) {
				printf("FAIL: %s: pixel %zu is %u %u %u, expected %u %u %u\n", what, i, got[0], got[1], got[2],
				       want[0], want[1], want[2]);
				failures++;
			}
		}
		free(rgba);
	}
	CGImageRelease(image);
}

int main(void) {
	uint8_t png[256], tiff[256];
	size_t pngLength = buildPNG(png), tiffLength = buildTIFF(tiff);
	static const uint8_t redBlue[6] = { 255, 0, 0, 0, 0, 255 };
	CFDictionaryRef o;

	CGImageRef none = thumbnail(png, pngLength, NULL);
	if (none != NULL) {
		printf("FAIL: a PNG without a stored thumbnail returned one without the FromImage options\n");
		failures++;
		CGImageRelease(none);
	}

	o = options(1, 2, 0);
	expect("PNG 4x2, max 2", thumbnail(png, pngLength, o), 2, 1, redBlue);
	CFRelease(o);
	o = options(1, 100, 0);
	expect("PNG 4x2, max 100 (not scaled up)", thumbnail(png, pngLength, o), 4, 2, NULL);
	CFRelease(o);
	o = options(1, 0, 0);
	expect("PNG 4x2, no maximum", thumbnail(png, pngLength, o), 4, 2, NULL);
	CFRelease(o);

	o = options(1, 0, 0);
	expect("TIFF orientation 6, no transform", thumbnail(tiff, tiffLength, o), 2, 1, redBlue);
	CFRelease(o);
	o = options(1, 0, 1);
	expect("TIFF orientation 6, with transform", thumbnail(tiff, tiffLength, o), 1, 2, redBlue);
	CFRelease(o);

	if (failures)
		return failures;
	puts("ImageIO thumbnails passed");
	return 0;
}
