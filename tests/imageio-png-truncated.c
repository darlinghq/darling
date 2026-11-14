// A PNG that is cut short or damaged is a decoding failure the caller can see,
// not a reason to end the process: CGImageSourceCreateImageAtIndex returns NULL
// (or, if the decoder keeps what it read, an image of the declared size), and the
// next image decodes normally.
// The PNGs are built here (PNG specification, ISO/IEC 15948): 4x4 RGB, 8 bits,
// one stored (uncompressed) deflate block.
//   clang -framework CoreFoundation -framework ImageIO -framework CoreGraphics tests/imageio-png-truncated.c -o /tmp/imageio-png-truncated
// Exits non-zero on failure.
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum { W = 4, H = 4 };

static uint32_t crc32Update(uint32_t crc, const uint8_t *bytes, size_t length) {
	crc = ~crc;
	for (size_t i = 0; i < length; i++) {
		crc ^= bytes[i];
		for (int k = 0; k < 8; k++)
			crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1)));
	}
	return ~crc;
}

static size_t put32(uint8_t *p, uint32_t v) {
	p[0] = v >> 24; p[1] = v >> 16; p[2] = v >> 8; p[3] = v;
	return 4;
}

static size_t chunk(uint8_t *out, const char *type, const uint8_t *data, uint32_t length) {
	size_t n = put32(out, length);
	memcpy(out + n, type, 4);
	memcpy(out + n + 4, data, length);
	n += 4 + length;
	return n + put32(out + n, crc32Update(0, out + 4, 4 + length));
}

// Writes the PNG into out (at least 256 bytes) and returns its length.
static size_t buildPNG(uint8_t *out) {
	static const uint8_t signature[8] = { 0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n' };
	uint8_t ihdr[13] = { 0 };
	put32(ihdr, W);
	put32(ihdr + 4, H);
	ihdr[8] = 8;	// bit depth
	ihdr[9] = 2;	// truecolour

	uint8_t raw[H * (1 + W * 3)];
	for (int y = 0; y < H; y++) {
		uint8_t *row = raw + y * (1 + W * 3);
		row[0] = 0;	// filter: none
		for (int x = 0; x < W; x++) {
			row[1 + x * 3] = x * 60;
			row[2 + x * 3] = y * 60;
			row[3 + x * 3] = 200;
		}
	}
	uint8_t idat[2 + 5 + sizeof(raw) + 4];
	size_t n = 0;
	idat[n++] = 0x78;	// zlib: deflate, 32K window
	idat[n++] = 0x01;
	idat[n++] = 0x01;	// final stored block
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
	n += put32(idat + n, (b << 16) | a);

	size_t length = 0;
	memcpy(out, signature, 8);
	length += 8;
	length += chunk(out + length, "IHDR", ihdr, sizeof(ihdr));
	length += chunk(out + length, "IDAT", idat, n);
	length += chunk(out + length, "IEND", NULL, 0);
	return length;
}

// 1: decoded at the declared size, 0: NULL; -1: an image of another size.
static int decode(const uint8_t *bytes, size_t length) {
	CFDataRef data = CFDataCreate(NULL, bytes, length);
	CGImageSourceRef source = CGImageSourceCreateWithData(data, NULL);
	CFRelease(data);
	if (source == NULL)
		return 0;
	CGImageRef image = CGImageSourceCreateImageAtIndex(source, 0, NULL);
	CFRelease(source);
	if (image == NULL)
		return 0;
	int result = (CGImageGetWidth(image) == W && CGImageGetHeight(image) == H) ? 1 : -1;
	CGImageRelease(image);
	return result;
}

int main(void) {
	uint8_t png[256];
	size_t length = buildPNG(png);
	int failures = 0;

	if (decode(png, length) != 1) {
		printf("FAIL: the complete PNG did not decode as %dx%d\n", W, H);
		return 1;
	}

	// Cut inside the IDAT data, just before IEND, and inside IHDR.
	const size_t idatData = 8 + 25 + 8;
	size_t cuts[] = { idatData + 20, length - 12, 8 + 12 };
	for (size_t i = 0; i < sizeof(cuts) / sizeof(cuts[0]); i++) {
		if (decode(png, cuts[i]) < 0) {
			printf("FAIL: PNG cut at %zu of %zu bytes decoded at a wrong size\n", cuts[i], length);
			failures++;
		}
	}

	// A damaged IDAT (its CRC no longer matches) is an error for libpng-style decoders.
	uint8_t damaged[256];
	memcpy(damaged, png, length);
	damaged[idatData + 30] ^= 0xFF;
	if (decode(damaged, length) < 0) {
		printf("FAIL: damaged PNG decoded at a wrong size\n");
		failures++;
	}

	if (decode(png, length) != 1) {
		printf("FAIL: the complete PNG no longer decodes after the failures\n");
		failures++;
	}

	if (failures)
		return failures;
	puts("truncated and damaged PNGs passed");
	return 0;
}
