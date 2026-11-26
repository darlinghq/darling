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

#ifndef _ICC_BUILDER_H_
#define _ICC_BUILDER_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Writes a version 4 ICC display profile (ICC.1:2022, https://www.color.org/specification/ICC.1-2022-05.pdf) from the
// published definition of an RGB colour space or of a gray response, so it holds no vendor data. These functions
// know nothing about CoreFoundation.

typedef struct ICCChromaticity {
	double x, y; // CIE 1931 xy chromaticity coordinates
} ICCChromaticity;

// A parametricCurveType (10.18) that maps device values to linear light: function is the type of Table 68
// (0 to 4), parameters are g, a, b, c, d, e, f in the order of that table (as many as the type uses).
typedef struct ICCCurve {
	uint16_t function;
	double parameters[7];
} ICCCurve;

typedef struct ICCDisplayProfileSpec {
	const char* description; // ASCII, at most 100 characters
	bool isGray; // a monochrome profile (colour space 'GRAY', one tag 'kTRC'); primaries and white are ignored
	ICCChromaticity primaries[3]; // red, green, blue
	ICCChromaticity white; // of the RGB colour space; its colorants are adapted to the PCS white (D50) with the linear Bradford transform (ICC.1:2022, 9.2.15)
	ICCCurve curve; // the same for each channel
} ICCDisplayProfileSpec;

// A profile with the required tags of 8.2 and 8.4.3 (or 8.4.4 for gray): profile description, copyright, media white
// point, chromatic adaptation (RGB only), and the three colorants and curves (or the gray curve). The date, the preferred CMM, the platform and the
// Profile ID are left zero; the zero date is a known limitation, because 7.2.8 says the field shall hold the date of creation, kept so that
// the output is the same on every call. Returns a buffer to free(), or NULL if the description is too long, the description is NULL, or memory runs out.
uint8_t* ICCBuildDisplayProfile(const ICCDisplayProfileSpec* spec, size_t* outLength);

// The colour spaces that have a definition in a public standard. Their sources:
// ICCNamedSRGB: IEC 61966-2-1, the sRGB primaries, D65 white and the piecewise curve (parametric type 3, Table 68);
// ICCNamedAdobeRGB1998: Adobe's "Adobe RGB (1998) Color Image Encoding" (May 2005), whose primaries and D65 white are
//   as listed there and whose curve is a pure power of 563/256 (2.19921875);
// ICCNamedITUR2020: Recommendation ITU-R BT.2020, its primaries and D65 white, and the inverse of its transfer function
//   with the 12-bit alpha (1.09929682680944) and the knee at 0.081 (see ICCBuilder.c);
// ICCNamedGrayGamma22: a gray response with a power of 2.2 and the PCS white point;
// ICCNamedGenericRGB, ICCNamedGenericGray: the generic spaces of the platform, which no standard defines: the numbers come
//   from black-box observation of macOS 26.7.3 and 10.14.4 through the public ColorSync and CoreGraphics API (see
//   ICCBuilder.c).
typedef enum ICCNamedProfile {
	ICCNamedSRGB,
	ICCNamedAdobeRGB1998,
	ICCNamedITUR2020,
	ICCNamedGrayGamma22,
	ICCNamedGenericRGB,
	ICCNamedGenericGray,
} ICCNamedProfile;

ICCDisplayProfileSpec ICCNamedProfileSpec(ICCNamedProfile which);

#endif
