// Copyright (C) 2026 Lubos Dolezel
//
// vImageBoxConvolve_ARGB8888 on a fixed pseudo-random image, for several kernel sizes, edge styles
// and regions of interest, and its error codes. The expected result of each case is a hash of the
// return code and the output bytes, taken from macOS 10.14.4 and macOS 26.7.3, where the two agree.
// "vimage_box_convolve dump" prints every case in full instead (to regenerate or inspect the table).
#include <Accelerate/Accelerate.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { W = 9, H = 7 };

static size_t gPad;
static void fillSource(uint8_t *p)
{
    uint32_t state = 12345;
    for (size_t y = 0; y < H; y++)
        for (size_t x = 0; x < (size_t)W * 4 + gPad; x++) {
            state = state * 1103515245u + 12345u;
            p[y * (W * 4 + gPad) + x] = (uint8_t)(state >> 16);
        }
}

static int gDump;
static int gFailures;
struct Expected { const char *name; uint64_t hash; };
static const struct Expected kExpected[] = {
    { "extend 3x3", 0x5f0799b4f63aaae4ull },
    { "extend 5x3", 0x07350f2115bcad31ull },
    { "extend 3x5", 0x504a2095084a86c5ull },
    { "extend 7x7", 0xdd1d2da6d4db6e4full },
    { "extend 1x1", 0xcd2c95363785a5b5ull },
    { "truncate 3x3", 0xde8fce853b3a9370ull },
    { "truncate 5x5", 0xd6913663958c9a3cull },
    { "background 3x3", 0x12c87b3f23e8b45eull },
    { "copy 3x3", 0x35fd6dd94817207bull },
    { "extend roi 3x3", 0x2c9407da8de5c1e9ull },
    { "truncate roi 5x5", 0x6e35f2815608d596ull },
    { "extend 3x3 leave alpha", 0xa76273d941758d94ull },
    { "extend 11x11", 0x10f515a196d7559full },
    { "truncate 11x11", 0xc58fea756c70b179ull },
    { "background 11x11", 0x0a7d7a09673bb954ull },
    { "copy 11x11", 0xcd2c95363785a5b5ull },
    { "copy roi 3x5", 0x68e20f165f246d8eull },
    { "background roi 5x3", 0x50873df48254df59ull },
    { "background null roi 3x3", 0xff7ffc4ae1c72cb4ull },
    { "truncate leave alpha 3x3", 0x5b34197e8720a9d0ull },
    { "padded rows extend 5x3", 0xe7c33c1eb3b7dddaull },
    { "padded rows truncate 3x3", 0x1bb1d4c1dc86fe4eull },
    { "error even height", 0xbaefd12b8ba5fa4eull },
    { "error even width", 0xbaefd12b8ba5fa4eull },
    { "error no edge style", 0xbaefd22b8ba5fc01ull },
    { "error two edge styles", 0xbaefd22b8ba5fc01ull },
    { "error background null", 0x8cd066dac4c212aaull },
    { "error roi too large", 0xbaefc82b8ba5eb03ull },
    { "error dest wider than source", 0xbaefc82b8ba5eb03ull },
    { "error dest taller than source", 0xbaefc82b8ba5eb03ull },
    { "error roi y too large", 0xbaefc82b8ba5eb03ull },
    { "error roi offset beyond source", 0xbaefc82b8ba5eb03ull },
    { "error zero kernel", 0xbaefd12b8ba5fa4eull }
};

static uint64_t hashCase(vImage_Error err, const uint8_t *p, size_t w, size_t h, size_t rowBytes)
{
    uint64_t hash = 1469598103934665603ull;
    hash = (hash ^ (uint64_t)(int64_t)err) * 1099511628211ull;
    if (err == kvImageNoError)
        for (size_t y = 0; y < h; y++)
            for (size_t x = 0; x < w * 4; x++)
                hash = (hash ^ p[y * rowBytes + x]) * 1099511628211ull;
    return hash;
}

static void dump(const char *label, vImage_Error err, const uint8_t *p, size_t w, size_t h, size_t rowBytes)
{
    if (!gDump) {
        uint64_t hash = hashCase(err, p, w, h, rowBytes);
        for (size_t i = 0; i < sizeof kExpected / sizeof kExpected[0]; i++) {
            if (strcmp(kExpected[i].name, label) == 0) {
                int ok = kExpected[i].hash == hash;
                printf("%s %s\n", ok ? "PASS" : "FAIL", label);
                if (!ok) {
                    gFailures++;
                    printf("  got %016llx\n", (unsigned long long)hash);
                }
                return;
            }
        }
        printf("FAIL %s: no expected hash (%016llx)\n", label, (unsigned long long)hash);
        gFailures++;
        return;
    }
    printf("%s err=%ld", label, (long)err);
    if (err == kvImageNoError) {
        for (size_t y = 0; y < h; y++) {
            printf("\n ");
            for (size_t x = 0; x < w * 4; x++)
                printf(" %02x", p[y * rowBytes + x]);
        }
    }
    printf("\n");
}

static void run(const char *name, uint32_t kh, uint32_t kw, vImage_Flags flags, size_t offX, size_t offY,
                size_t dw, size_t dh, int useBackground)
{
    uint8_t src[(W * 4 + 16) * H], out[(W * 4 + 16) * H];
    uint8_t background[4] = { 10, 100, 200, 250 };
    fillSource(src);
    memset(out, 0xee, sizeof out);
    vImage_Buffer s = { src, H, W, W * 4 + gPad }, d = { out, dh, dw, dw * 4 + gPad };
    vImage_Error err = vImageBoxConvolve_ARGB8888(&s, &d, NULL, offX, offY, kh, kw,
                                                  useBackground ? background : NULL, flags);
    dump(name, err, out, dw, dh, dw * 4 + gPad);
}

int main(int argc, char **argv)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    gDump = argc > 1 && strcmp(argv[1], "dump") == 0;
    run("extend 3x3", 3, 3, kvImageEdgeExtend, 0, 0, W, H, 0);
    run("extend 5x3", 5, 3, kvImageEdgeExtend, 0, 0, W, H, 0);
    run("extend 3x5", 3, 5, kvImageEdgeExtend, 0, 0, W, H, 0);
    run("extend 7x7", 7, 7, kvImageEdgeExtend, 0, 0, W, H, 0);
    run("extend 1x1", 1, 1, kvImageEdgeExtend, 0, 0, W, H, 0);
    run("truncate 3x3", 3, 3, kvImageTruncateKernel, 0, 0, W, H, 0);
    run("truncate 5x5", 5, 5, kvImageTruncateKernel, 0, 0, W, H, 0);
    run("background 3x3", 3, 3, kvImageBackgroundColorFill, 0, 0, W, H, 1);
    run("copy 3x3", 3, 3, kvImageCopyInPlace, 0, 0, W, H, 0);
    run("extend roi 3x3", 3, 3, kvImageEdgeExtend, 2, 1, 5, 4, 0);
    run("truncate roi 5x5", 5, 5, kvImageTruncateKernel, 3, 2, 4, 3, 0);
    run("extend 3x3 leave alpha", 3, 3, kvImageEdgeExtend | kvImageLeaveAlphaUnchanged, 0, 0, W, H, 0);
    run("extend 11x11", 11, 11, kvImageEdgeExtend, 0, 0, W, H, 0);
    run("truncate 11x11", 11, 11, kvImageTruncateKernel, 0, 0, W, H, 0);
    run("background 11x11", 11, 11, kvImageBackgroundColorFill, 0, 0, W, H, 1);
    run("copy 11x11", 11, 11, kvImageCopyInPlace, 0, 0, W, H, 0);
    run("copy roi 3x5", 3, 5, kvImageCopyInPlace, 2, 1, 5, 5, 0);
    run("background roi 5x3", 5, 3, kvImageBackgroundColorFill, 1, 2, 7, 3, 1);
    run("background null roi 3x3", 3, 3, kvImageBackgroundColorFill, 0, 0, 4, 4, 0);
    run("truncate leave alpha 3x3", 3, 3, kvImageTruncateKernel | kvImageLeaveAlphaUnchanged, 0, 0, W, H, 0);
    gPad = 12;
    run("padded rows extend 5x3", 5, 3, kvImageEdgeExtend, 1, 1, 6, 5, 0);
    run("padded rows truncate 3x3", 3, 3, kvImageTruncateKernel, 0, 0, W, H, 0);
    gPad = 0;
    run("error even height", 4, 3, kvImageEdgeExtend, 0, 0, W, H, 0);
    run("error even width", 3, 2, kvImageEdgeExtend, 0, 0, W, H, 0);
    run("error no edge style", 3, 3, 0, 0, 0, W, H, 0);
    run("error two edge styles", 3, 3, kvImageEdgeExtend | kvImageTruncateKernel, 0, 0, W, H, 0);
    run("error background null", 3, 3, kvImageBackgroundColorFill, 0, 0, W, H, 0);
    run("error roi too large", 3, 3, kvImageEdgeExtend, 5, 0, 5, H, 0);
    run("error dest wider than source", 3, 3, kvImageEdgeExtend, 0, 0, W + 1, H, 0);
    run("error dest taller than source", 3, 3, kvImageEdgeExtend, 0, 0, W, H + 1, 0);
    run("error roi y too large", 3, 3, kvImageEdgeExtend, 0, 4, W, 4, 0);
    run("error roi offset beyond source", 3, 3, kvImageEdgeExtend, 9, 0, 1, H, 0);
    run("error zero kernel", 0, 3, kvImageEdgeExtend, 0, 0, W, H, 0);
    {
        uint8_t src[W * H * 4];
        vImage_Buffer s = { src, H, W, W * 4 };
        vImage_Error err;
        fillSource(src);
        // The temp-size query: a non-negative size, or an error, never a result in the buffers.
        err = vImageBoxConvolve_ARGB8888(&s, &s, NULL, 0, 0, 3, 3, NULL, kvImageEdgeExtend | kvImageGetTempBufferSize);
        printf("%s temp size query is not an error\n", err >= 0 ? "PASS" : "FAIL");
        if (err < 0) gFailures++;
    }
    if (!gDump)
        printf("RESULT failures=%d\n", gFailures);
    return gFailures != 0;
}
