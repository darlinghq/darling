// CFLAGS: -framework Accelerate
// Copyright (C) 2026 Lubos Dolezel
//
// vImage ARGB1555/RGB565/Planar4 conversions and vImageConvolve_ARGB8888. Every
// expected value is computed here, independently of the library, from the formulas
// of the public vImage documentation (Conversion.h and Convolution.h): the 5- and 6-bit
// expansions (v * 255 + 15) / 31 and (v * 255 + 31) / 63, the quantisations
// (c * 31 + 127) / 255 and (c * 63 + 127) / 255, nibble * 17 for Planar4, and
// the convolution sum (sum + divisor / 2) / divisor with the four edge styles. The
// conversions are checked over all 65536 16-bit values or all 256 channel values,
// on buffers with odd widths and padded rows (the padding must stay untouched), with
// destinations smaller than the source, and with the documented error returns.
#include <Accelerate/Accelerate.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures = 0;
#define CHECK(c) do { if (!(c)) { printf("FAIL %d: %s\n", __LINE__, #c); g_failures++; } } while (0)

#define GUARD 0xA5

static uint32_t expand5(uint32_t v) { return (v * 255u + 15u) / 31u; }
static uint32_t expand6(uint32_t v) { return (v * 255u + 31u) / 63u; }
static uint32_t quant5(uint32_t c) { return (c * 31u + 127u) / 255u; }
static uint32_t quant6(uint32_t c) { return (c * 63u + 127u) / 255u; }

typedef struct { uint8_t *mem; size_t size; vImage_Buffer buf; } Buf;

// A buffer whose rows are padded and whose padding and surroundings hold GUARD bytes.
static Buf make(size_t width, size_t height, size_t bytesPerPixel, size_t pad)
{
	Buf b;
	size_t rowBytes = width * bytesPerPixel + pad;
	b.size = rowBytes * height + 16;
	b.mem = malloc(b.size);
	memset(b.mem, GUARD, b.size);
	b.buf.data = b.mem + 8;
	b.buf.width = width;
	b.buf.height = height;
	b.buf.rowBytes = rowBytes;
	return b;
}

static uint8_t *px(Buf *b, size_t x, size_t y, size_t bpp) { return (uint8_t *)b->buf.data + y * b->buf.rowBytes + x * bpp; }

// Everything outside the first `w` x `h` pixels of the buffer still holds the guard value.
static int untouched(Buf *b, size_t w, size_t h, size_t bpp)
{
	for (size_t i = 0; i < b->size; i++) {
		size_t off = i - 8;
		int inside = 0;
		if (i >= 8 && off < b->buf.rowBytes * b->buf.height) {
			size_t y = off / b->buf.rowBytes, xb = off % b->buf.rowBytes;
			inside = y < h && xb < w * bpp;
		}
		if (!inside && b->mem[i] != GUARD)
			return 0;
	}
	return 1;
}

static void test_16bit_to_8888(void)
{
	// All 65536 values as a 255-wide image (odd) with padded rows: pixel i = value i.
	const size_t w = 255, h = 257;   // 65535 pixels + 1 spare row slot: fill value i for i < 65536, rest 0
	Buf src = make(w, h, 2, 6), dst = make(w, h, 4, 12), back = make(w, h, 2, 4);
	for (size_t i = 0; i < w * h; i++) {
		uint16_t v = (uint16_t)(i < 65536 ? i : 0);
		memcpy(px(&src, i % w, i / w, 2), &v, 2);
	}

	// ARGB1555 -> ARGB8888.
	CHECK(vImageConvert_ARGB1555toARGB8888(&src.buf, &dst.buf, kvImageNoFlags) == kvImageNoError);
	int bad = 0;
	for (size_t i = 0; i < w * h; i++) {
		uint16_t v;
		memcpy(&v, px(&src, i % w, i / w, 2), 2);
		uint8_t exp[4] = { (v & 0x8000) ? 255 : 0, expand5((v >> 10) & 31), expand5((v >> 5) & 31), expand5(v & 31) };
		if (memcmp(px(&dst, i % w, i / w, 4), exp, 4) != 0)
			bad++;
	}
	CHECK(bad == 0);
	CHECK(untouched(&dst, w, h, 4));

	// ARGB8888 -> ARGB1555 of those results gives back the original value.
	CHECK(vImageConvert_ARGB8888toARGB1555(&dst.buf, &back.buf, kvImageNoFlags) == kvImageNoError);
	bad = 0;
	for (size_t i = 0; i < w * h; i++) {
		uint16_t v, r;
		memcpy(&v, px(&src, i % w, i / w, 2), 2);
		memcpy(&r, px(&back, i % w, i / w, 2), 2);
		if (r != v)
			bad++;
	}
	CHECK(bad == 0);
	CHECK(untouched(&back, w, h, 2));

	// RGB565 -> ARGB8888 and BGRA8888, for three alpha values.
	const uint8_t alphas[3] = { 0, 77, 255 };
	for (int a = 0; a < 3; a++) {
		CHECK(vImageConvert_RGB565toARGB8888(alphas[a], &src.buf, &dst.buf, kvImageNoFlags) == kvImageNoError);
		Buf bgra = make(w, h, 4, 8);
		CHECK(vImageConvert_RGB565toBGRA8888(alphas[a], &src.buf, &bgra.buf, kvImageNoFlags) == kvImageNoError);
		int badArgb = 0, badBgra = 0;
		for (size_t i = 0; i < w * h; i++) {
			uint16_t v;
			memcpy(&v, px(&src, i % w, i / w, 2), 2);
			uint8_t r = expand5(v >> 11), g = expand6((v >> 5) & 63), b = expand5(v & 31);
			uint8_t eargb[4] = { alphas[a], r, g, b }, ebgra[4] = { b, g, r, alphas[a] };
			if (memcmp(px(&dst, i % w, i / w, 4), eargb, 4) != 0) badArgb++;
			if (memcmp(px(&bgra, i % w, i / w, 4), ebgra, 4) != 0) badBgra++;
		}
		CHECK(badArgb == 0);
		CHECK(badBgra == 0);
		CHECK(untouched(&dst, w, h, 4));
		CHECK(untouched(&bgra, w, h, 4));
		free(bgra.mem);
	}
	free(src.mem); free(dst.mem); free(back.mem);
}

// 8-bit channels to 16 bit: every value of every channel, in rotations so all channels see all values.
static void test_8888_to_16bit(void)
{
	const size_t w = 255, h = 4;   // 255 columns x 4 rows, value v = column + row-dependent shifts
	Buf src = make(w, h, 4, 8), a1555 = make(w, h, 2, 2), b565 = make(w, h, 2, 6), r565 = make(w, h, 2, 6);
	for (size_t y = 0; y < h; y++)
		for (size_t x = 0; x < w; x++) {
			uint8_t *p = px(&src, x, y, 4);
			p[0] = (uint8_t)(x + 3 * y);
			p[1] = (uint8_t)(255 - x);
			p[2] = (uint8_t)(x * 7 + y * 31);
			p[3] = (uint8_t)(x * 11 + 5 * y);
		}
	// (alpha can reach every value: alpha cut-off of ARGB1555 is (a + 127) / 255, so 128 is the first 1.)
	for (int v = 0; v < 256; v++) {
		uint8_t *p = px(&src, v % w, 3, 4);
		(void)p;
	}

	CHECK(vImageConvert_ARGB8888toARGB1555(&src.buf, &a1555.buf, kvImageNoFlags) == kvImageNoError);
	CHECK(vImageConvert_BGRA8888toRGB565(&src.buf, &b565.buf, kvImageNoFlags) == kvImageNoError);
	CHECK(vImageConvert_RGBA8888toRGB565(&src.buf, &r565.buf, kvImageNoFlags) == kvImageNoError);
	int bad1555 = 0, badB = 0, badR = 0;
	for (size_t y = 0; y < h; y++)
		for (size_t x = 0; x < w; x++) {
			const uint8_t *p = px(&src, x, y, 4);
			uint16_t e1555 = (uint16_t)((((p[0] + 127u) / 255u) << 15) | (quant5(p[1]) << 10) | (quant5(p[2]) << 5) | quant5(p[3]));
			uint16_t eb = (uint16_t)((quant5(p[2]) << 11) | (quant6(p[1]) << 5) | quant5(p[0]));   // BGRA: B G R A in memory
			uint16_t er = (uint16_t)((quant5(p[0]) << 11) | (quant6(p[1]) << 5) | quant5(p[2]));   // RGBA: R G B A in memory
			uint16_t g1, gb, gr;
			memcpy(&g1, px(&a1555, x, y, 2), 2);
			memcpy(&gb, px(&b565, x, y, 2), 2);
			memcpy(&gr, px(&r565, x, y, 2), 2);
			if (g1 != e1555) bad1555++;
			if (gb != eb) badB++;
			if (gr != er) badR++;
		}
	CHECK(bad1555 == 0 && badB == 0 && badR == 0);
	CHECK(untouched(&a1555, w, h, 2) && untouched(&b565, w, h, 2) && untouched(&r565, w, h, 2));

	// All 256 values of each channel, one channel at a time, against the formulas.
	Buf ramp = make(256, 1, 4, 0), o1555 = make(256, 1, 2, 0), o565 = make(256, 1, 2, 0);
	int bad = 0;
	for (int ch = 0; ch < 4; ch++) {
		for (int v = 0; v < 256; v++) {
			uint8_t *p = px(&ramp, v, 0, 4);
			memset(p, 0, 4);
			p[ch] = (uint8_t)v;
		}
		vImageConvert_ARGB8888toARGB1555(&ramp.buf, &o1555.buf, kvImageNoFlags);
		vImageConvert_RGBA8888toRGB565(&ramp.buf, &o565.buf, kvImageNoFlags);
		for (int v = 0; v < 256; v++) {
			uint32_t a = ch == 0 ? v : 0, c1 = ch == 1 ? v : 0, c2 = ch == 2 ? v : 0, c3 = ch == 3 ? v : 0;
			uint16_t e = (uint16_t)((((a + 127u) / 255u) << 15) | (quant5(c1) << 10) | (quant5(c2) << 5) | quant5(c3));
			uint16_t e565 = (uint16_t)((quant5(a) << 11) | (quant6(c1) << 5) | quant5(c2));
			uint16_t g, g565;
			memcpy(&g, px(&o1555, v, 0, 2), 2);
			memcpy(&g565, px(&o565, v, 0, 2), 2);
			if (g != e || g565 != e565) bad++;
		}
	}
	CHECK(bad == 0);
	free(ramp.mem); free(o1555.mem); free(o565.mem);
	free(src.mem); free(a1555.mem); free(b565.mem); free(r565.mem);
}

static void test_planar4(void)
{
	// Widths from 1 to 9 (odd and even), rows padded; the unused low nibble of an odd row is ignored.
	for (size_t w = 1; w <= 9; w++) {
		const size_t h = 3;
		Buf src = make((w + 1) / 2, h, 1, 3), dst = make(w, h, 1, 5);
		src.buf.width = w;   // widths are in pixels
		for (size_t y = 0; y < h; y++)
			for (size_t i = 0; i < (w + 1) / 2; i++)
				*px(&src, i, y, 1) = (uint8_t)(((y * 5 + i * 3) & 15) << 4 | ((y * 7 + i * 5 + 1) & 15));
		if (w & 1)
			for (size_t y = 0; y < h; y++)
				*px(&src, w / 2, y, 1) |= 0x0F;   // garbage in the unused nibble
		CHECK(vImageConvert_Planar4toPlanar8(&src.buf, &dst.buf, kvImageNoFlags) == kvImageNoError);
		int bad = 0;
		for (size_t y = 0; y < h; y++)
			for (size_t x = 0; x < w; x++) {
				uint8_t byte = *px(&src, x / 2, y, 1);
				uint8_t nibble = (x & 1) ? (byte & 15) : (byte >> 4);
				if (*px(&dst, x, y, 1) != nibble * 17)
					bad++;
			}
		CHECK(bad == 0);
		CHECK(untouched(&dst, w, h, 1));
		free(src.mem); free(dst.mem);
	}
	// All sixteen values.
	uint8_t src16[8], dst16[16];
	for (int i = 0; i < 8; i++) src16[i] = (uint8_t)((2 * i) << 4 | (2 * i + 1));
	vImage_Buffer s = { src16, 1, 16, 8 }, d = { dst16, 1, 16, 16 };
	CHECK(vImageConvert_Planar4toPlanar8(&s, &d, kvImageDoNotTile) == kvImageNoError);
	for (int i = 0; i < 16; i++) CHECK(dst16[i] == i * 17);
}

static void test_conversion_errors(void)
{
	Buf small = make(3, 2, 2, 0), big = make(5, 4, 4, 0), bigPlanar = make(5, 4, 1, 0);
	Buf bigSrc = make(5, 4, 2, 0), smallDst = make(3, 2, 4, 0);

	// A destination larger than the source is refused and nothing is written.
	CHECK(vImageConvert_ARGB1555toARGB8888(&small.buf, &big.buf, kvImageNoFlags) == kvImageRoiLargerThanInputBuffer);
	CHECK(vImageConvert_RGB565toARGB8888(7, &small.buf, &big.buf, kvImageNoFlags) == kvImageRoiLargerThanInputBuffer);
	CHECK(vImageConvert_RGB565toBGRA8888(7, &small.buf, &big.buf, kvImageNoFlags) == kvImageRoiLargerThanInputBuffer);
	CHECK(vImageConvert_Planar4toPlanar8(&small.buf, &bigPlanar.buf, kvImageNoFlags) == kvImageRoiLargerThanInputBuffer);
	CHECK(untouched(&big, 0, 0, 4));

	// A destination smaller than the source converts the top-left part only.
	uint16_t *s16 = (uint16_t *)bigSrc.buf.data;
	for (int i = 0; i < 5 * 4; i++) s16[i] = (uint16_t)(0x8000 | i);
	CHECK(vImageConvert_ARGB1555toARGB8888(&bigSrc.buf, &smallDst.buf, kvImageNoFlags) == kvImageNoError);
	CHECK(untouched(&smallDst, 3, 2, 4));
	CHECK(*px(&smallDst, 2, 1, 4) == 255 && *(px(&smallDst, 2, 1, 4) + 3) == expand5((uint32_t)(5 + 2)));

	// An unknown flag bit does no work; kvImageGetTempBufferSize returns zero and does no work.
	CHECK(vImageConvert_ARGB1555toARGB8888(&bigSrc.buf, &smallDst.buf, 0x40000000u) == kvImageUnknownFlagsBit);
	Buf fresh = make(3, 2, 4, 0);
	CHECK(vImageConvert_ARGB1555toARGB8888(&bigSrc.buf, &fresh.buf, kvImageGetTempBufferSize) == 0);
	CHECK(vImageConvert_RGB565toARGB8888(1, &bigSrc.buf, &fresh.buf, kvImageGetTempBufferSize) == 0);
	CHECK(untouched(&fresh, 0, 0, 4));

	// Empty destinations succeed.
	vImage_Buffer empty = { NULL, 0, 0, 0 };
	CHECK(vImageConvert_ARGB1555toARGB8888(&bigSrc.buf, &empty, kvImageNoFlags) == kvImageNoError);

	free(small.mem); free(big.mem); free(bigPlanar.mem); free(bigSrc.mem); free(smallDst.mem); free(fresh.mem);
}

// ---- Convolution --------------------------------------------------------------------------------

static int g_edge_pixel_needed;

// The documented convolution sum for one channel of one destination pixel; src_x/src_y is the
// position of the pixel in the source. Edge styles as documented.
static int ref_convolve_channel(const Buf *src, long sx, long sy, int ch, const int16_t *k, long kh, long kw,
                                long divisor, const uint8_t *bg, vImage_Flags edge, int *missing)
{
	long w = (long)src->buf.width, h = (long)src->buf.height;
	long sum = 0;
	long covered = 0, full = 0;   // kernel sum over existing pixels / over the whole kernel (TruncateKernel)
	*missing = 0;
	for (long i = 0; i < kh; i++)
		for (long j = 0; j < kw; j++) {
			long y = sy + i - kh / 2, x = sx + j - kw / 2;
			int16_t kv = k[i * kw + j];
			full += kv;
			if (x < 0 || y < 0 || x >= w || y >= h) {
				*missing = 1;
				if (edge == kvImageBackgroundColorFill) {
					sum += kv * (long)bg[ch];
				} else if (edge == kvImageEdgeExtend) {
					long cx = x < 0 ? 0 : (x >= w ? w - 1 : x), cy = y < 0 ? 0 : (y >= h ? h - 1 : y);
					sum += kv * (long)((const uint8_t *)src->buf.data)[cy * src->buf.rowBytes + cx * 4 + ch];
				}
				continue;
			}
			covered += kv;
			sum += kv * (long)((const uint8_t *)src->buf.data)[y * src->buf.rowBytes + x * 4 + ch];
		}
	if (divisor == 0)
		divisor = 1;
	if (edge == kvImageTruncateKernel && *missing) {
		// Pixels outside the image are ignored and the divisor is adjusted in proportion to the weight that is left.
		if (covered == 0)
			return 0;
		long d = divisor * covered;   // sum * full / (divisor * covered), rounded
		long n = sum * full;
		if (full == 0)
			return 0;   // a kernel of zero total weight gives 0
		long v = (n + d / 2) / d;
		return v < 0 ? 0 : (v > 255 ? 255 : (int)v);
	}
	long v = (sum + divisor / 2) / divisor;
	return v < 0 ? 0 : (v > 255 ? 255 : (int)v);
}

static unsigned g_seed = 12345;
static uint8_t rnd(void) { g_seed = g_seed * 1103515245u + 12345u; return (uint8_t)(g_seed >> 16); }

static int run_convolve_case(const char *label, size_t sw, size_t sh, size_t ox, size_t oy, size_t dw, size_t dh,
                             const int16_t *k, long kh, long kw, int32_t divisor, vImage_Flags edge, int strict)
{
	Buf src = make(sw, sh, 4, 12), dst = make(dw, dh, 4, 8);
	for (size_t y = 0; y < sh; y++)
		for (size_t x = 0; x < sw; x++)
			for (int c = 0; c < 4; c++)
				px(&src, x, y, 4)[c] = rnd();
	uint8_t bg[4] = { 200, 20, 90, 130 };
	vImage_Error err = vImageConvolve_ARGB8888(&src.buf, &dst.buf, NULL, ox, oy, k, (uint32_t)kh, (uint32_t)kw, divisor, bg, edge);
	int bad = 0, firstBad = -1;
	if (err != kvImageNoError) {
		printf("  %s: error %ld\n", label, (long)err);
		bad = 1;
	} else {
		for (size_t y = 0; y < dh; y++)
			for (size_t x = 0; x < dw; x++)
				for (int c = 0; c < 4; c++) {
					int missing;
					int e = ref_convolve_channel(&src, (long)(x + ox), (long)(y + oy), c, k, kh, kw, divisor, bg, edge, &missing);
					if (edge == kvImageCopyInPlace && missing)
						e = px(&src, x + ox, y + oy, 4)[c];
					int g = px(&dst, x, y, 4)[c];
					// The rounding of the truncated-kernel divisor is not documented: allow macOS's 1.
					int tol = (edge == kvImageTruncateKernel && missing) ? 1 : 0;
					if (g < e - tol || g > e + tol) {
						bad++;
						if (firstBad < 0) { firstBad = 1; printf("  %s: (%zu,%zu) ch %d got %d expected %d\n", label, x, y, c, g, e); }
					}
				}
		if (!untouched(&dst, dw, dh, 4)) { bad++; printf("  %s: wrote outside the destination\n", label); }
	}
	if (bad && strict) { printf("FAIL: convolve %s\n", label); g_failures++; }
	else if (bad) printf("NOTE: convolve %s differs (not strict)\n", label);
	free(src.mem); free(dst.mem);
	return bad;
}

static void test_convolve(void)
{
	const int16_t box3[9] = { 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	const int16_t gauss[9] = { 1, 2, 1, 2, 4, 2, 1, 2, 1 };
	const int16_t sharp[9] = { 0, -1, 0, -1, 5, -1, 0, -1, 0 };
	const int16_t row5[5] = { 1, 2, 3, 2, 1 };       // 1 high, 5 wide
	const int16_t col3[3] = { -1, 0, 1 };            // 3 high, 1 wide
	const int16_t k53[15] = { 1, 0, 2, 0, 1, 0, 3, 0, 3, 0, 1, 0, 2, 0, 1 };   // 3 high, 5 wide
	const int16_t ident[1] = { 1 };

	const vImage_Flags edges[3] = { kvImageEdgeExtend, kvImageBackgroundColorFill, kvImageCopyInPlace };
	const char *names[3] = { "extend", "background", "copy" };
	char label[96];
	for (int e = 0; e < 3; e++) {
		struct { const int16_t *k; long kh, kw; int32_t div; const char *n; } kernels[] = {
			{ box3, 3, 3, 9, "box3" }, { gauss, 3, 3, 16, "gauss" }, { sharp, 3, 3, 1, "sharp" }, { row5, 1, 5, 9, "row5" },
			{ col3, 3, 1, 1, "col3" }, { k53, 3, 5, 16, "k53" }, { ident, 1, 1, 1, "ident" }, { box3, 3, 3, 0, "div0" },
			{ gauss, 3, 3, -16, "negdiv" }, { box3, 3, 3, 4, "div4" },
		};
		for (size_t i = 0; i < sizeof(kernels) / sizeof(kernels[0]); i++) {
			// odd sizes with padded rows; whole image and a region of interest with offsets.
			snprintf(label, sizeof(label), "%s %s 7x5", names[e], kernels[i].n);
			run_convolve_case(label, 7, 5, 0, 0, 7, 5, kernels[i].k, kernels[i].kh, kernels[i].kw, kernels[i].div, edges[e], 1);
			snprintf(label, sizeof(label), "%s %s roi", names[e], kernels[i].n);
			run_convolve_case(label, 9, 7, 2, 1, 5, 4, kernels[i].k, kernels[i].kh, kernels[i].kw, kernels[i].div, edges[e], 1);
			snprintf(label, sizeof(label), "%s %s 1x1", names[e], kernels[i].n);
			run_convolve_case(label, 1, 1, 0, 0, 1, 1, kernels[i].k, kernels[i].kh, kernels[i].kw, kernels[i].div, edges[e], 1);
		}
	}
	// Truncated kernels: the divisor follows the weight that is left (macOS 26.7.3 rounds this
	// within 1 of the exact value).
	for (int use = 0; use < 4; use++) {
		const int16_t *k = use == 0 ? box3 : (use == 1 ? gauss : (use == 2 ? row5 : k53));
		long kh = use == 2 ? 1 : 3, kw = use == 2 ? 5 : (use == 3 ? 5 : 3);
		int32_t div = use == 0 ? 9 : (use == 1 ? 16 : (use == 2 ? 9 : 16));
		snprintf(label, sizeof(label), "truncate %d", use);
		run_convolve_case(label, 7, 5, 0, 0, 7, 5, k, kh, kw, div, kvImageTruncateKernel, 1);
		snprintf(label, sizeof(label), "truncate %d div0", use);
		run_convolve_case(label, 6, 4, 0, 0, 6, 4, k, kh, kw, 0, kvImageTruncateKernel, 1);
	}

	// Known values: a blur of a constant image is that constant, for every edge style but background fill.
	{
		Buf src = make(5, 5, 4, 4), dst = make(5, 5, 4, 4);
		for (size_t y = 0; y < 5; y++) for (size_t x = 0; x < 5; x++) { uint8_t *p = px(&src, x, y, 4); p[0] = 10; p[1] = 20; p[2] = 30; p[3] = 40; }
		uint8_t bg[4] = { 0, 0, 0, 0 };
		const vImage_Flags flagsList[3] = { kvImageEdgeExtend, kvImageTruncateKernel, kvImageCopyInPlace };
		for (int f = 0; f < 3; f++) {
			CHECK(vImageConvolve_ARGB8888(&src.buf, &dst.buf, NULL, 0, 0, box3, 3, 3, 9, bg, flagsList[f]) == kvImageNoError);
			int ok = 1;
			for (size_t y = 0; y < 5; y++) for (size_t x = 0; x < 5; x++) {
				const uint8_t *p = px(&dst, x, y, 4);
				if (p[0] != 10 || p[1] != 20 || p[2] != 30 || p[3] != 40) ok = 0;
			}
			CHECK(ok);
		}
		// A single bright pixel spreads over its 3x3 neighbourhood; a box with divisor 9 shows 90 / 9 = 10.
		memset(src.buf.data, 0, src.buf.rowBytes * 5);
		px(&src, 2, 2, 4)[1] = 90;
		CHECK(vImageConvolve_ARGB8888(&src.buf, &dst.buf, NULL, 0, 0, box3, 3, 3, 9, bg, kvImageEdgeExtend) == kvImageNoError);
		CHECK(px(&dst, 2, 2, 4)[1] == 10 && px(&dst, 1, 1, 4)[1] == 10 && px(&dst, 3, 3, 4)[1] == 10);
		CHECK(px(&dst, 0, 0, 4)[1] == 0 && px(&dst, 4, 4, 4)[1] == 0 && px(&dst, 2, 2, 4)[0] == 0);
		// Rounding: 90 * 1 / 4 = 22.5 rounds up to 23 ((sum + divisor / 2) / divisor).
		const int16_t one[1] = { 1 };
		CHECK(vImageConvolve_ARGB8888(&src.buf, &dst.buf, NULL, 0, 0, one, 1, 1, 4, bg, kvImageEdgeExtend) == kvImageNoError);
		CHECK(px(&dst, 2, 2, 4)[1] == 23);
		// Background colour fill at a corner: a 3x3 box with divisor 9 of an all-zero image and bg 90 gives 5 * 90 / 9 = 50.
		memset(src.buf.data, 0, src.buf.rowBytes * 5);
		uint8_t bg90[4] = { 90, 90, 90, 90 };
		CHECK(vImageConvolve_ARGB8888(&src.buf, &dst.buf, NULL, 0, 0, box3, 3, 3, 9, bg90, kvImageBackgroundColorFill) == kvImageNoError);
		CHECK(px(&dst, 0, 0, 4)[2] == 50 && px(&dst, 2, 2, 4)[2] == 0 && px(&dst, 2, 0, 4)[2] == 30);
		free(src.mem); free(dst.mem);
	}

	// Documented errors.
	{
		Buf src = make(4, 4, 4, 0), dst = make(4, 4, 4, 0), bigDst = make(5, 4, 4, 0);
		uint8_t bg[4] = { 0 };
		CHECK(vImageConvolve_ARGB8888(&src.buf, &dst.buf, NULL, 0, 0, NULL, 3, 3, 9, bg, kvImageEdgeExtend) == kvImageNullPointerArgument);
		CHECK(vImageConvolve_ARGB8888(&src.buf, &dst.buf, NULL, 0, 0, box3, 2, 3, 9, bg, kvImageEdgeExtend) == kvImageInvalidKernelSize);
		CHECK(vImageConvolve_ARGB8888(&src.buf, &dst.buf, NULL, 0, 0, box3, 3, 4, 9, bg, kvImageEdgeExtend) == kvImageInvalidKernelSize);
		CHECK(vImageConvolve_ARGB8888(&src.buf, &dst.buf, NULL, 0, 0, box3, 3, 3, 9, bg, kvImageNoFlags) == kvImageInvalidEdgeStyle);
		CHECK(vImageConvolve_ARGB8888(&src.buf, &dst.buf, NULL, 0, 0, box3, 3, 3, 9, bg, kvImageEdgeExtend | kvImageCopyInPlace) == kvImageInvalidEdgeStyle);
		CHECK(vImageConvolve_ARGB8888(&src.buf, &bigDst.buf, NULL, 0, 0, box3, 3, 3, 9, bg, kvImageEdgeExtend) == kvImageRoiLargerThanInputBuffer);
		CHECK(vImageConvolve_ARGB8888(&src.buf, &dst.buf, NULL, 1, 0, box3, 3, 3, 9, bg, kvImageEdgeExtend) == kvImageInvalidOffset_X);
		CHECK(vImageConvolve_ARGB8888(&src.buf, &dst.buf, NULL, 0, 1, box3, 3, 3, 9, bg, kvImageEdgeExtend) == kvImageInvalidOffset_Y);
		CHECK(untouched(&dst, 0, 0, 4));
		// kvImageGetTempBufferSize returns a non-negative size and does no work.
		CHECK(vImageConvolve_ARGB8888(&src.buf, &dst.buf, NULL, 0, 0, box3, 3, 3, 9, bg, kvImageEdgeExtend | kvImageGetTempBufferSize) >= 0);
		CHECK(untouched(&dst, 0, 0, 4));
		free(src.mem); free(dst.mem); free(bigDst.mem);
	}
}

int main(void)
{
	test_16bit_to_8888();
	test_8888_to_16bit();
	test_planar4();
	test_conversion_errors();
	test_convolve();
	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
