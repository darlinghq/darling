/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#include "ICCBuilder.h"
#include <stdlib.h>
#include <string.h>

#define SIG(a, b, c, d) (((uint32_t) (a) << 24) | ((uint32_t) (b) << 16) | ((uint32_t) (c) << 8) | (uint32_t) (d))

#define HEADER_SIZE 128
#define MAX_DESCRIPTION 100
#define BUFFER_SIZE 2048

static void put32(uint8_t* p, uint32_t v)
{
	p[0] = v >> 24;
	p[1] = v >> 16;
	p[2] = v >> 8;
	p[3] = v;
}

// An s15Fixed16Number (ICC.1:2022, 4.6), rounded to nearest.
static void putFixed(uint8_t* p, double v)
{
	put32(p, (uint32_t) (int32_t) (v * 65536.0 + (v < 0 ? -0.5 : 0.5)));
}

typedef struct Matrix {
	double m[3][3];
} Matrix;

static Matrix multiply(Matrix a, Matrix b)
{
	Matrix r;
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			r.m[i][j] = a.m[i][0] * b.m[0][j] + a.m[i][1] * b.m[1][j] + a.m[i][2] * b.m[2][j];
	return r;
}

static Matrix invert(Matrix a)
{
	const double (*m)[3] = a.m;
	const double det = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
		m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
	Matrix r = { { {
		(m[1][1] * m[2][2] - m[1][2] * m[2][1]) / det, (m[0][2] * m[2][1] - m[0][1] * m[2][2]) / det, (m[0][1] * m[1][2] - m[0][2] * m[1][1]) / det }, {
		(m[1][2] * m[2][0] - m[1][0] * m[2][2]) / det, (m[0][0] * m[2][2] - m[0][2] * m[2][0]) / det, (m[0][2] * m[1][0] - m[0][0] * m[1][2]) / det }, {
		(m[1][0] * m[2][1] - m[1][1] * m[2][0]) / det, (m[0][1] * m[2][0] - m[0][0] * m[2][1]) / det, (m[0][0] * m[1][1] - m[0][1] * m[1][0]) / det } } };
	return r;
}

static void xyzOf(ICCChromaticity c, double xyz[3])
{
	xyz[0] = c.x / c.y;
	xyz[1] = 1.0;
	xyz[2] = (1.0 - c.x - c.y) / c.y;
}

static void apply(Matrix a, const double v[3], double out[3])
{
	for (int i = 0; i < 3; i++)
		out[i] = a.m[i][0] * v[0] + a.m[i][1] * v[1] + a.m[i][2] * v[2];
}

// The XYZ of the red, green and blue primaries at full intensity, scaled so that red = green = blue = 1 gives the white
// of the RGB space, then adapted to the PCS white (ICC.1:2022, 7.2.16: 0.9642, 1.0, 0.8249) by the linear Bradford
// transform that 9.2.15 and Annex E recommend: the cone responses of the Bradford matrix are scaled by the ratio of the
// two whites' responses. `adaptation` is that 3 x 3 chromatic adaptation matrix, written to the 'chad' tag.
static void adaptedColorants(const ICCDisplayProfileSpec* spec, double colorants[3][3], Matrix* adaptation)
{
	static const double pcsWhite[3] = { 0.9642, 1.0, 0.8249 };
	static const Matrix bradford = { { { 0.8951, 0.2664, -0.1614 }, { -0.7502, 1.7135, 0.0367 }, { 0.0389, -0.0685, 1.0296 } } };

	double white[3], primaryXYZ[3][3];
	xyzOf(spec->white, white);
	Matrix primaries;
	for (int c = 0; c < 3; c++)
	{
		xyzOf(spec->primaries[c], primaryXYZ[c]);
		for (int i = 0; i < 3; i++)
			primaries.m[i][c] = primaryXYZ[c][i];
	}
	double scale[3];
	apply(invert(primaries), white, scale);

	double sourceCones[3], targetCones[3];
	apply(bradford, white, sourceCones);
	apply(bradford, pcsWhite, targetCones);
	Matrix cones = { { { 0 } } };
	for (int i = 0; i < 3; i++)
		cones.m[i][i] = targetCones[i] / sourceCones[i];
	*adaptation = multiply(invert(bradford), multiply(cones, bradford));

	for (int c = 0; c < 3; c++)
	{
		double column[3] = { primaryXYZ[c][0] * scale[c], primaryXYZ[c][1] * scale[c], primaryXYZ[c][2] * scale[c] };
		apply(*adaptation, column, colorants[c]);
	}
}

typedef struct Builder {
	uint8_t* out;
	uint32_t used; // end of the tag data so far, always a multiple of 4
	uint32_t count;
} Builder;

// Adds a tag-table entry for `size` bytes of new data and returns where the data goes.
static uint8_t* addTag(Builder* b, uint32_t signature, uint32_t size, uint32_t* outOffset)
{
	uint8_t* entry = b->out + HEADER_SIZE + 4 + b->count * 12;
	const uint32_t offset = b->used;
	put32(entry, signature);
	put32(entry + 4, offset);
	put32(entry + 8, size);
	b->count++;
	b->used += (size + 3) & ~3u;
	*outOffset = offset;
	return b->out + offset;
}

// An entry that shares the data of an earlier tag, with the same offset and size (7.3.1 allows this).
static void addSharedTag(Builder* b, uint32_t signature, uint32_t offset, uint32_t size)
{
	uint8_t* entry = b->out + HEADER_SIZE + 4 + b->count * 12;
	put32(entry, signature);
	put32(entry + 4, offset);
	put32(entry + 8, size);
	b->count++;
}

// multiLocalizedUnicodeType (10.15) with one record, English (United States).
static void addText(Builder* b, uint32_t signature, const char* text)
{
	const size_t length = strlen(text);
	uint32_t offset;
	uint8_t* p = addTag(b, signature, (uint32_t) (28 + 2 * length), &offset);
	memcpy(p, "mluc", 4);
	put32(p + 8, 1);
	put32(p + 12, 12);
	memcpy(p + 16, "enUS", 4);
	put32(p + 20, (uint32_t) (2 * length));
	put32(p + 24, 28);
	for (size_t i = 0; i < length; i++)
		p[28 + 2 * i + 1] = (uint8_t) text[i];
}

// XYZType (10.31).
static uint8_t* addXYZ(Builder* b, uint32_t signature, const double xyz[3])
{
	uint32_t offset;
	uint8_t* p = addTag(b, signature, 20, &offset);
	memcpy(p, "XYZ ", 4);
	for (int i = 0; i < 3; i++)
		putFixed(p + 8 + i * 4, xyz[i]);
	return p;
}

static uint32_t parameterCount(uint16_t function)
{
	static const uint32_t counts[5] = { 1, 3, 4, 5, 7 };
	return counts[function];
}

uint8_t* ICCBuildDisplayProfile(const ICCDisplayProfileSpec* spec, size_t* outLength)
{
	if (!spec->description || strlen(spec->description) > MAX_DESCRIPTION || spec->curve.function > 4)
		return NULL;
	uint8_t* out = calloc(1, BUFFER_SIZE);
	if (!out)
		return NULL;

	const uint32_t tagCount = spec->isGray ? 4 : 10;
	Builder b = { out, HEADER_SIZE + 4 + tagCount * 12, 0 };
	static const double d50[3] = { 0.9642, 1.0, 0.8249 };

	addText(&b, SIG('d', 'e', 's', 'c'), spec->description);
	addText(&b, SIG('c', 'p', 'r', 't'), "Generated by Darling");
	addXYZ(&b, SIG('w', 't', 'p', 't'), d50);
	if (!spec->isGray)
	{
		double colorants[3][3];
		Matrix adaptation;
		adaptedColorants(spec, colorants, &adaptation);
		addXYZ(&b, SIG('r', 'X', 'Y', 'Z'), colorants[0]);
		addXYZ(&b, SIG('g', 'X', 'Y', 'Z'), colorants[1]);
		addXYZ(&b, SIG('b', 'X', 'Y', 'Z'), colorants[2]);

		// s15Fixed16ArrayType (10.22), the matrix with its rows in order (9.2.15).
		uint32_t chadOffset;
		uint8_t* chad = addTag(&b, SIG('c', 'h', 'a', 'd'), 44, &chadOffset);
		memcpy(chad, "sf32", 4);
		for (int i = 0; i < 9; i++)
			putFixed(chad + 8 + 4 * i, adaptation.m[i / 3][i % 3]);
	}

	const uint32_t curveSize = 12 + 4 * parameterCount(spec->curve.function);
	uint32_t curveOffset;
	uint8_t* curve = addTag(&b, spec->isGray ? SIG('k', 'T', 'R', 'C') : SIG('r', 'T', 'R', 'C'), curveSize, &curveOffset);
	memcpy(curve, "para", 4);
	curve[8] = spec->curve.function >> 8;
	curve[9] = (uint8_t) spec->curve.function;
	for (uint32_t i = 0; i < parameterCount(spec->curve.function); i++)
		putFixed(curve + 12 + 4 * i, spec->curve.parameters[i]);
	if (!spec->isGray)
	{
		addSharedTag(&b, SIG('g', 'T', 'R', 'C'), curveOffset, curveSize);
		addSharedTag(&b, SIG('b', 'T', 'R', 'C'), curveOffset, curveSize);
	}

	// The header (7.2): version 4.3, display class, no CMM, platform, flags, device or creator, and rendering intent 0
	// (perceptual). The illuminant of the PCS is D50.
	put32(out + 0, b.used);
	put32(out + 8, 0x04300000u);
	put32(out + 12, SIG('m', 'n', 't', 'r'));
	put32(out + 16, spec->isGray ? SIG('G', 'R', 'A', 'Y') : SIG('R', 'G', 'B', ' '));
	put32(out + 20, SIG('X', 'Y', 'Z', ' '));
	put32(out + 36, SIG('a', 'c', 's', 'p'));
	putFixed(out + 68, d50[0]);
	putFixed(out + 72, d50[1]);
	putFixed(out + 76, d50[2]);
	put32(out + HEADER_SIZE, b.count);

	*outLength = b.used;
	return out;
}

ICCDisplayProfileSpec ICCNamedProfileSpec(ICCNamedProfile which)
{
	static const ICCChromaticity d65 = { 0.3127, 0.3290 };
	static const double alpha = 1.09929682680944;
	switch (which)
	{
		case ICCNamedAdobeRGB1998:
			return (ICCDisplayProfileSpec) {
				.description = "Adobe RGB (1998)",
				.primaries = { { 0.64, 0.33 }, { 0.21, 0.71 }, { 0.15, 0.06 } },
				.white = d65,
				.curve = { 0, { 563.0 / 256.0 } },
			};
		// The curve is the inverse of the BT.2020 transfer function (OETF), as the profile maps device values to scene-linear
		// light as IEC 61966-2-1 does for sRGB (not the display EOTF of Recommendation ITU-R BT.1886, a power of 2.4). macOS 26.7.3
		// and 10.14.4 give the curve of their ITU-R BT.2020 profile the parameters g, a, b and c below and d = 0.081 (the 10-bit
		// constant 4.5 * 0.018, read back with ColorSyncProfileCopyTag), while a and b use the 12-bit alpha; the segments meet
		// to within 1e-5. Test: tests/src/colorsync_profile_name.c.
		case ICCNamedITUR2020:
			return (ICCDisplayProfileSpec) {
				.description = "Rec. ITU-R BT.2020-1",
				.primaries = { { 0.708, 0.292 }, { 0.170, 0.797 }, { 0.131, 0.046 } },
				.white = d65,
				.curve = { 3, { 1.0 / 0.45, 1.0 / alpha, (alpha - 1.0) / alpha, 1.0 / 4.5, 0.081 } },
			};
		case ICCNamedGrayGamma22:
			return (ICCDisplayProfileSpec) {
				.description = "Generic Gray Gamma 2.2 Profile",
				.isGray = true,
				.curve = { 0, { 2.2 } },
			};
		// Generic RGB and Generic Gray: no standard defines them. Black-box observation (tier 2) of macOS 26.7.3 and 10.14.4
		// (ColorSyncProfileCopyTag numbers, and CGColor conversions into sRGB and XYZ): the primaries are the chromaticities
		// below, the white is D65, the curve is a power of 1.8 (the profiles of macOS store it as the u8Fixed8 number 1.80078)
		// and the colorants are the primaries adapted from D65 to D50 by the Bradford transform; the gray one is the same
		// power on the neutral axis. The colorants of macOS follow from these chromaticities to within 1e-5 once its
		// chromatic adaptation matrix is applied backwards. Test: tests/src/colorsync_generic_rgb.c.
		case ICCNamedGenericRGB:
			return (ICCDisplayProfileSpec) {
				.description = "Generic RGB Profile",
				.primaries = { { 0.630, 0.340 }, { 0.295, 0.605 }, { 0.155, 0.077 } },
				.white = d65,
				.curve = { 0, { 1.8 } },
			};
		case ICCNamedGenericGray:
			return (ICCDisplayProfileSpec) {
				.description = "Generic Gray Profile",
				.isGray = true,
				.curve = { 0, { 1.8 } },
			};
		case ICCNamedSRGB:
			return (ICCDisplayProfileSpec) {
				.description = "sRGB IEC61966-2.1",
				.primaries = { { 0.64, 0.33 }, { 0.30, 0.60 }, { 0.15, 0.06 } },
				.white = d65,
				.curve = { 3, { 2.4, 1.0 / 1.055, 0.055 / 1.055, 1.0 / 12.92, 0.04045 } },
			};
	}
	// Not a name of the enumeration: a spec without a description, which ICCBuildDisplayProfile refuses.
	return (ICCDisplayProfileSpec) { 0 };
}
