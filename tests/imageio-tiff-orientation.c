// CGImageSourceCreateImageAtIndex returns a TIFF's pixels as stored, row 0 at the
// top, and does not apply the Orientation tag (274). The orientation is reported as
// kCGImagePropertyOrientation (TIFF 6.0 values 1-8, the same as EXIF) for whoever
// wants to apply it, e.g. CGImageSourceCreateThumbnailAtIndex with
// kCGImageSourceCreateThumbnailWithTransform.
// The TIFFs are built here (TIFF 6.0, baseline RGB): little-endian, uncompressed,
// 8 bits per sample, one strip.
//   clang -framework CoreFoundation -framework ImageIO -framework CoreGraphics tests/imageio-tiff-orientation.c -o /tmp/imageio-tiff-orientation
// Exits non-zero on failure.
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t put16(uint8_t *p, uint16_t v) { p[0] = v; p[1] = v >> 8; return 2; }
static size_t put32(uint8_t *p, uint32_t v) { p[0] = v; p[1] = v >> 8; p[2] = v >> 16; p[3] = v >> 24; return 4; }

// Writes a width x height RGB TIFF with the given samples into out (at least
// 256 bytes) and returns its length. orientation 0 leaves the tag out.
static size_t buildTIFF(uint8_t *out, int width, int height, const uint8_t *rgb, int orientation) {
	struct { uint16_t tag, type; uint32_t count, value; } entries[11];
	int n = 0;
	int entryCount = orientation ? 11 : 10;
	uint32_t bitsOffset = 8 + 2 + entryCount * 12 + 4;
	uint32_t pixelsOffset = bitsOffset + 6;
	uint32_t pixelBytes = width * height * 3;

#define ENTRY(t, ty, c, v) do { entries[n].tag = t; entries[n].type = ty; entries[n].count = c; entries[n].value = v; n++; } while (0)
	ENTRY(256, 4, 1, width);		// ImageWidth, LONG
	ENTRY(257, 4, 1, height);		// ImageLength
	ENTRY(258, 3, 3, bitsOffset);		// BitsPerSample, 3 SHORTs at bitsOffset
	ENTRY(259, 3, 1, 1);			// Compression: none
	ENTRY(262, 3, 1, 2);			// PhotometricInterpretation: RGB
	ENTRY(273, 4, 1, pixelsOffset);		// StripOffsets
	if (orientation)
		ENTRY(274, 3, 1, orientation);	// Orientation
	ENTRY(277, 3, 1, 3);			// SamplesPerPixel
	ENTRY(278, 4, 1, height);		// RowsPerStrip
	ENTRY(279, 4, 1, pixelBytes);		// StripByteCounts
	ENTRY(284, 3, 1, 1);			// PlanarConfiguration: chunky
#undef ENTRY

	size_t p = 0;
	out[p++] = 'I';
	out[p++] = 'I';
	p += put16(out + p, 42);
	p += put32(out + p, 8);
	p += put16(out + p, n);
	for (int i = 0; i < n; i++) {
		p += put16(out + p, entries[i].tag);
		p += put16(out + p, entries[i].type);
		p += put32(out + p, entries[i].count);
		// A single SHORT sits in the first two bytes of the value field.
		if (entries[i].type == 3 && entries[i].count == 1) {
			p += put16(out + p, entries[i].value);
			p += put16(out + p, 0);
		} else {
			p += put32(out + p, entries[i].value);
		}
	}
	p += put32(out + p, 0);	// no next IFD
	for (int i = 0; i < 3; i++)
		p += put16(out + p, 8);
	memcpy(out + p, rgb, pixelBytes);
	return p + pixelBytes;
}

static CGImageSourceRef sourceFor(const uint8_t *bytes, size_t length) {
	CFDataRef data = CFDataCreate(NULL, bytes, length);
	CGImageSourceRef source = CGImageSourceCreateWithData(data, NULL);
	CFRelease(data);
	return source;
}

// R, G, B of every pixel, row 0 at the top, into rgb (width * height * 3 bytes).
static void readPixels(CGImageRef image, uint8_t *rgb) {
	size_t width = CGImageGetWidth(image), height = CGImageGetHeight(image);
	uint8_t *rgba = calloc(width * height, 4);
	CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
	CGContextRef context = CGBitmapContextCreate(rgba, width, height, 8, width * 4, space,
						     (CGBitmapInfo)kCGImageAlphaPremultipliedLast);
	CGColorSpaceRelease(space);
	CGContextDrawImage(context, CGRectMake(0, 0, width, height), image);
	CGContextRelease(context);
	for (size_t i = 0; i < width * height; i++)
		memcpy(rgb + i * 3, rgba + i * 4, 3);
	free(rgba);
}

static int failures;

static int sameColour(const uint8_t *a, const uint8_t *b) {
	return abs(a[0] - b[0]) <= 2 && abs(a[1] - b[1]) <= 2 && abs(a[2] - b[2]) <= 2;
}

static void check(int width, int height, int orientation) {
	static const uint8_t samples[6] = { 255, 0, 0, 0, 0, 255 };	// red, then blue
	uint8_t tiff[256];
	size_t length = buildTIFF(tiff, width, height, samples, orientation);
	CGImageSourceRef source = sourceFor(tiff, length);
	CGImageRef image = source ? CGImageSourceCreateImageAtIndex(source, 0, NULL) : NULL;
	if (image == NULL) {
		printf("FAIL: %dx%d TIFF with orientation %d did not decode\n", width, height, orientation);
		failures++;
		if (source)
			CFRelease(source);
		return;
	}

	if ((int)CGImageGetWidth(image) != width || (int)CGImageGetHeight(image) != height) {
		printf("FAIL: orientation %d: image is %zux%zu, expected the stored %dx%d\n", orientation,
		       CGImageGetWidth(image), CGImageGetHeight(image), width, height);
		failures++;
	} else {
		uint8_t rgb[6];
		readPixels(image, rgb);
		if (!sameColour(rgb, samples) || !sameColour(rgb + 3, samples + 3)) {
			printf("FAIL: orientation %d: pixels are %u %u %u / %u %u %u, expected the stored red / blue\n",
			       orientation, rgb[0], rgb[1], rgb[2], rgb[3], rgb[4], rgb[5]);
			failures++;
		}
	}
	CGImageRelease(image);

	if (orientation) {
		CFDictionaryRef properties = CGImageSourceCopyPropertiesAtIndex(source, 0, NULL);
		CFNumberRef value = properties ? CFDictionaryGetValue(properties, kCGImagePropertyOrientation) : NULL;
		int reported = 0;
		if (value == NULL || CFGetTypeID(value) != CFNumberGetTypeID() ||
		    !CFNumberGetValue(value, kCFNumberIntType, &reported) || reported != orientation) {
			printf("FAIL: kCGImagePropertyOrientation is %d, expected %d\n", reported, orientation);
			failures++;
		}
		if (properties)
			CFRelease(properties);
	}
	CFRelease(source);
}

int main(void) {
	check(1, 2, 0);	// no tag: row 0 (red) at the top
	check(1, 2, 1);
	check(2, 1, 3);	// rotated 180 degrees
	check(1, 2, 4);	// mirrored vertically
	check(2, 1, 6);	// rotated 90 degrees: stored as is, not transposed
	check(2, 1, 8);
	if (failures)
		return failures;
	puts("TIFF orientation passed");
	return 0;
}
