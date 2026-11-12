// Copyright (C) 2026 Lubos Dolezel
//
// Details of the files -[NSBitmapImageRep representationUsingType:properties:] writes, as
// observed on macOS 27.0.1 (arm64 and x86_64): the PNG color type follows the alpha channel
// of the source, PNG, BMP and GIF store straight (not premultiplied) alpha, BMP is top-down
// with 24 bits per pixel (no alpha) or a 124-byte header with 32 bits per pixel (alpha), GIF
// uses a global palette sized to the next power of two of the colors of an image whose pixels are all different and treats
// alpha below 64 as transparent, and JPEG at quality 1.0 has no chroma subsampling while the
// JFIF density units are 0. Passes unchanged on macOS 27.0.1.
#import <AppKit/AppKit.h>
#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(cond, ...) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "FAIL line %d: ", __LINE__); \
            fprintf(stderr, __VA_ARGS__); \
            fputc('\n', stderr); \
            failures++; \
        } \
    } while (0)

static unsigned u16le(const unsigned char *b) { return b[0] | (b[1] << 8); }
static unsigned u32le(const unsigned char *b) { return b[0] | (b[1] << 8) | (b[2] << 16) | ((unsigned) b[3] << 24); }

// A rep holding the color (200, 100, 50) at alpha x for x in 0..255 (one row), premultiplied.
static NSBitmapImageRep *alphaRamp(void) {
    NSBitmapImageRep *rep = [[[NSBitmapImageRep alloc]
            initWithBitmapDataPlanes: NULL
                          pixelsWide: 256
                          pixelsHigh: 1
                       bitsPerSample: 8
                     samplesPerPixel: 4
                            hasAlpha: YES
                            isPlanar: NO
                      colorSpaceName: NSDeviceRGBColorSpace
                         bytesPerRow: 0
                        bitsPerPixel: 0] autorelease];
    unsigned char *d = [rep bitmapData];
    for (int x = 0; x < 256; x++) {
        d[x * 4 + 0] = 200 * x / 255;
        d[x * 4 + 1] = 100 * x / 255;
        d[x * 4 + 2] = 50 * x / 255;
        d[x * 4 + 3] = x;
    }
    return rep;
}

static NSBitmapImageRep *opaque(int w, int h, BOOL alpha, int colors) {
    NSBitmapImageRep *rep = [[[NSBitmapImageRep alloc]
            initWithBitmapDataPlanes: NULL
                          pixelsWide: w
                          pixelsHigh: h
                       bitsPerSample: 8
                     samplesPerPixel: alpha ? 4 : 3
                            hasAlpha: alpha
                            isPlanar: NO
                      colorSpaceName: NSDeviceRGBColorSpace
                         bytesPerRow: 0
                        bitsPerPixel: 0] autorelease];
    unsigned char *d = [rep bitmapData];
    NSInteger bpp = [rep bitsPerPixel] / 8, bpr = [rep bytesPerRow];
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            unsigned char *p = d + y * bpr + x * bpp;
            int c = (y * w + x) % colors;
            // Well separated colors: macOS merges colors that are close together.
            p[0] = (c & 7) * 36;
            p[1] = ((c >> 3) & 7) * 36;
            p[2] = ((c >> 6) & 3) * 80;
            if (alpha)
                p[3] = 255;
        }
    return rep;
}

int main(void) {
    @autoreleasepool {
        // PNG: color type 2 without alpha, 6 with alpha, even when all alpha values are 255.
        NSData *png = [opaque(7, 5, NO, 35) representationUsingType: NSPNGFileType properties: nil];
        CHECK(png != nil && [png length] > 26 && ((const unsigned char *) [png bytes])[25] == 2,
              "PNG color type without alpha");
        png = [opaque(7, 5, YES, 35) representationUsingType: NSPNGFileType properties: nil];
        CHECK(png != nil && [png length] > 26 && ((const unsigned char *) [png bytes])[25] == 6,
              "PNG color type with an all-opaque alpha channel");

        // Straight alpha in PNG: color (200, 100, 50) at alpha 127 reads back near 200, 100, 50
        // after converting the decoded rep to straight alpha.
        png = [alphaRamp() representationUsingType: NSPNGFileType properties: nil];
        NSBitmapImageRep *back = [NSBitmapImageRep imageRepWithData: png];
        CHECK(back != nil && [back pixelsWide] == 256 && [back hasAlpha], "PNG alpha decode");
        if (back) {
            const unsigned char *p = [back bitmapData] + 127 * ([back bitsPerPixel] / 8);
            int r = p[0], g = p[1], b = p[2], a = p[3];
            if (!([back bitmapFormat] & NSAlphaNonpremultipliedBitmapFormat) && a > 0) {
                r = r * 255 / a;
                g = g * 255 / a;
                b = b * 255 / a;
            }
            CHECK(a == 127, "PNG alpha at x=127 is %d", a);
            CHECK(abs(r - 200) <= 3 && abs(g - 100) <= 3 && abs(b - 50) <= 3,
                  "PNG straight color at alpha 127 is %d %d %d", r, g, b);
        }

        // BMP: BITMAPINFOHEADER, 24 bits, top-down; with alpha BITMAPV5HEADER, 32 bits, bit fields.
        NSData *bmp = [opaque(7, 5, NO, 35) representationUsingType: NSBMPFileType properties: nil];
        const unsigned char *b = [bmp bytes];
        CHECK([bmp length] == 54 + 24 * 5 && u32le(b + 10) == 54 && u32le(b + 14) == 40 &&
              u16le(b + 28) == 24 && u32le(b + 30) == 0 && (int) u32le(b + 22) == -5,
              "BMP without alpha: header %u bpp %u compression %u height %d", u32le(b + 14),
              u16le(b + 28), u32le(b + 30), (int) u32le(b + 22));
        bmp = [opaque(7, 5, YES, 35) representationUsingType: NSBMPFileType properties: nil];
        b = [bmp bytes];
        CHECK([bmp length] == 138 + 28 * 5 && u32le(b + 10) == 138 && u32le(b + 14) == 124 &&
              u16le(b + 28) == 32 && u32le(b + 30) == 3 && u32le(b + 54) == 0x00FF0000 &&
              u32le(b + 58) == 0x0000FF00 && u32le(b + 62) == 0x000000FF &&
              u32le(b + 66) == 0xFF000000,
              "BMP with alpha: header %u bpp %u compression %u", u32le(b + 14), u16le(b + 28),
              u32le(b + 30));
        bmp = [alphaRamp() representationUsingType: NSBMPFileType properties: nil];
        b = [bmp bytes];
        {
            // Pixel 127 is stored as straight B, G, R, A.
            const unsigned char *p = b + u32le(b + 10) + 127 * 4;
            CHECK(p[3] == 127 && abs(p[2] - 200) <= 3 && abs(p[1] - 100) <= 3 && abs(p[0] - 50) <= 3,
                  "BMP pixel at alpha 127 is B%d G%d R%d A%d", p[0], p[1], p[2], p[3]);
        }

        // GIF: 87a, global palette of 2^n entries for an image of n distinct pixel colors, flags
        // 0x80 | (n-1) << 4 | (n-1).
        static const struct { int colors, packed; } palettes[] = {
            {3, 0x91}, {5, 0xA2}, {9, 0xB3}, {35, 0xD5}, {100, 0xE6}, {200, 0xF7}};
        for (unsigned i = 0; i < sizeof(palettes) / sizeof(palettes[0]); i++) {
            NSData *gif = [opaque(palettes[i].colors, 1, NO, palettes[i].colors) representationUsingType: NSGIFFileType
                                                                               properties: nil];
            const unsigned char *g = [gif bytes];
            CHECK(gif != nil && memcmp(g, "GIF87a", 6) == 0 && g[10] == palettes[i].packed,
                  "GIF with %d colors: flags %02x, expected %02x", palettes[i].colors,
                  gif ? g[10] : 0, palettes[i].packed);
        }

        // GIF transparency: alpha below 64 becomes transparent, 64 and above opaque.
        NSData *gif = [alphaRamp() representationUsingType: NSGIFFileType properties: nil];
        back = [NSBitmapImageRep imageRepWithData: gif];
        CHECK(back != nil && [back hasAlpha], "GIF with transparency decodes with alpha");
        if (back && [back hasAlpha]) {
            const unsigned char *d = [back bitmapData];
            NSInteger bpp = [back bitsPerPixel] / 8;
            CHECK(d[63 * bpp + 3] == 0, "GIF pixel with alpha 63 is not transparent");
            CHECK(d[64 * bpp + 3] == 255, "GIF pixel with alpha 64 is not opaque");
            CHECK(d[200 * bpp + 3] == 255, "GIF pixel with alpha 200 is not opaque");
        }

        // JPEG at quality 1.0: three components all sampled 1x1; the default has 2x2 luma.
        NSDictionary *q = [NSDictionary dictionaryWithObject: [NSNumber numberWithFloat: 1.0f]
                                                       forKey: NSImageCompressionFactor];
        NSData *jpeg = [opaque(16, 16, NO, 35) representationUsingType: NSJPEGFileType properties: q];
        const unsigned char *j = [jpeg bytes];
        NSUInteger n = [jpeg length], i = 2;
        int lumaH = -1, lumaV = -1, units = -1;
        while (i + 4 <= n && j[i] == 0xFF) {
            unsigned m = j[i + 1], len = (j[i + 2] << 8) | j[i + 3];
            if (m == 0xE0 && len >= 16)
                units = j[i + 11];
            if (m == 0xC0) {
                lumaH = j[i + 11] >> 4;
                lumaV = j[i + 11] & 15;
            }
            if (m == 0xDA)
                break;
            i += 2 + len;
        }
        CHECK(lumaH == 1 && lumaV == 1, "JPEG q1.0 luma sampling %dx%d", lumaH, lumaV);
        CHECK(units == 0, "JPEG density units %d", units);
    }
    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("PASS");
    return 0;
}
