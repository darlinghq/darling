// CFLAGS: -framework Accelerate
#include <Accelerate/Accelerate.h>
#include <stdint.h>
#include <stdio.h>

#define CHECK(c) do { if (!(c)) { fprintf(stderr, "FAIL: %s (line %d)\n", #c, __LINE__); return 1; } } while (0)

int main(void) {
    /* ARGB1555 <-> ARGB8888 (including non-trivial 5->8 expansion and alpha 127/128) */
    uint16_t src1555[2] = { 0x8000u | (31u << 10) | (7u << 5) | 3u, 0x0000u };
    uint8_t dst8888[8] = { 0 };
    vImage_Buffer b1555 = { src1555, 1, 2, 4 };
    vImage_Buffer b8888 = { dst8888, 1, 2, 8 };
    CHECK(vImageConvert_ARGB1555toARGB8888(&b1555, &b8888, 0) == kvImageNoError);
    CHECK(dst8888[0] == 255 && dst8888[1] == 255 && dst8888[2] == 58 && dst8888[3] == 25);
    CHECK(dst8888[4] == 0 && dst8888[5] == 0 && dst8888[6] == 0 && dst8888[7] == 0);

    dst8888[0] = 128; dst8888[4] = 127;
    uint16_t rt1555[2] = { 0 };
    vImage_Buffer brt1555 = { rt1555, 1, 2, 4 };
    CHECK(vImageConvert_ARGB8888toARGB1555(&b8888, &brt1555, 0) == kvImageNoError);
    CHECK(rt1555[0] == src1555[0] && rt1555[1] == src1555[1]);

    /* RGB565 -> ARGB8888 / BGRA8888 (alpha first!) and back */
    uint16_t src565[1] = { (31u << 11) | (11u << 5) | 3u };
    uint8_t argb[4] = { 0 }, bgra[4] = { 0 };
    vImage_Buffer b565 = { src565, 1, 1, 2 };
    vImage_Buffer bargb = { argb, 1, 1, 4 };
    vImage_Buffer bbgra = { bgra, 1, 1, 4 };
    CHECK(vImageConvert_RGB565toARGB8888(0x7Fu, &b565, &bargb, 0) == kvImageNoError);
    CHECK(argb[0] == 0x7Fu && argb[1] == 255 && argb[2] == 45 && argb[3] == 25);
    CHECK(vImageConvert_RGB565toBGRA8888(0x7Fu, &b565, &bbgra, 0) == kvImageNoError);
    CHECK(bgra[0] == 25 && bgra[1] == 45 && bgra[2] == 255 && bgra[3] == 0x7Fu);

    uint16_t rt565_a = 0, rt565_b = 0;
    vImage_Buffer brt_a = { &rt565_a, 1, 1, 2 };
    vImage_Buffer brt_b = { &rt565_b, 1, 1, 2 };
    uint8_t rgba[4] = { 255, 45, 25, 0x7Fu };
    vImage_Buffer brgba = { rgba, 1, 1, 4 };
    CHECK(vImageConvert_BGRA8888toRGB565(&bbgra, &brt_a, 0) == kvImageNoError);
    CHECK(vImageConvert_RGBA8888toRGB565(&brgba, &brt_b, 0) == kvImageNoError);
    CHECK(rt565_a == src565[0] && rt565_b == src565[0]);

    /* Planar4 -> Planar8 (2 rows with stride) */
    uint8_t p4[6] = { 0xA3u, 0xF0u, 0, 0x12u, 0x00u, 0 };
    uint8_t p8[8] = { 0 };
    vImage_Buffer bp4 = { p4, 2, 3, 3 };
    vImage_Buffer bp8 = { p8, 2, 3, 4 };
    CHECK(vImageConvert_Planar4toPlanar8(&bp4, &bp8, 0) == kvImageNoError);
    CHECK(p8[0] == 0xAAu && p8[1] == 0x33u && p8[2] == 0xFFu);
    CHECK(p8[4] == 0x11u && p8[5] == 0x22u && p8[6] == 0x00u);

    /* Convolve_ARGB8888: kvImageGetTempBufferSize, kvImageTruncateKernel with nonzero divisor, negative divisor */
    uint8_t conv_src[4 * 4] = {
        10, 90, 90, 90,  20, 90, 90, 90,
        30, 90, 90, 90,  40, 90, 90, 90
    };
    uint8_t conv_dst[4 * 4] = { 0 };
    int16_t box3[9] = { 1, 1, 1, 1, 1, 1, 1, 1, 1 };
    vImage_Buffer bcsrc = { conv_src, 2, 2, 8 };
    vImage_Buffer bcdst = { conv_dst, 2, 2, 8 };
    CHECK(vImageConvolve_ARGB8888(&bcsrc, &bcdst, NULL, 0, 0, box3, 3, 3, 9, NULL, kvImageGetTempBufferSize) == 0);
    CHECK(vImageConvolve_ARGB8888(&bcsrc, &bcdst, NULL, 0, 0, box3, 3, 3, 9, NULL,
                                  kvImageTruncateKernel | kvImageLeaveAlphaUnchanged) == kvImageNoError);
    CHECK(conv_dst[0] == 10 && conv_dst[1] == 90 && conv_dst[4] == 20 && conv_dst[5] == 90);

    /* Negative divisor (-3) with negative kernel (-1) -> positive rounding (+2/3 -> 1) */
    uint8_t neg_src[4] = { 2, 2, 2, 2 };
    uint8_t neg_dst[4] = { 0 };
    int16_t neg_k[9] = { 0, 0, 0, 0, -1, 0, 0, 0, 0 };
    vImage_Buffer bns = { neg_src, 1, 1, 4 };
    vImage_Buffer bnd = { neg_dst, 1, 1, 4 };
    CHECK(vImageConvolve_ARGB8888(&bns, &bnd, NULL, 0, 0, neg_k, 3, 3, -3, NULL, kvImageEdgeExtend) == kvImageNoError);
    CHECK(neg_dst[0] == 1 && neg_dst[1] == 1 && neg_dst[2] == 1 && neg_dst[3] == 1);

    /* Error checks */
    CHECK(vImageConvolve_ARGB8888(NULL, &bnd, NULL, 0, 0, neg_k, 3, 3, 1, NULL, kvImageEdgeExtend) == kvImageNullPointerArgument);
    CHECK(vImageConvolve_ARGB8888(&bns, &bnd, NULL, 0, 0, neg_k, 2, 3, 1, NULL, kvImageEdgeExtend) == kvImageInvalidKernelSize);
    CHECK(vImageConvolve_ARGB8888(&bns, &bnd, NULL, 0, 0, neg_k, 3, 3, 1, NULL, 0) == kvImageInvalidEdgeStyle);

    puts("PASS");
    return 0;
}
