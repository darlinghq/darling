/*
This file is part of Darling.

Copyright (C) 2017 Lubos Dolezel

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

#ifndef _TEXTCOMMON_H
#define _TEXTCOMMON_H
#include <MacTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef UInt8* TextPtr;
typedef const UInt8* ConstTextPtr;

enum {
	kTextEncodingUnicodeDefault = 0x100
};

enum {
	kTextEncodingDefaultVariant = 0
};

enum {
	kTextEncodingDefaultFormat = 0,
	kUnicodeUTF16Format = 0,
	kUnicodeUTF7Format = 1,
	kUnicodeUTF8Format = 2,
	kUnicodeUTF32Format = 3,
	kUnicodeUTF16BEFormat = 4,
	kUnicodeUTF16LEFormat = 5,
	kUnicodeUTF32BEFormat = 6,
	kUnicodeUTF32LEFormat = 7,
	kUnicodeSCSUFormat = 8,
	kUnicode16BitFormat = 0,
	kUnicode32BitFormat = 3,
};

enum {
	kUnicodeNoSubset = 0,
	kUnicodeNormalizationFormD = 5,
	kUnicodeNormalizationFormC = 3,
	kUnicodeCanonicalCompVariant = kUnicodeNormalizationFormC,
	kUnicodeHFSPlusDecompVariant = 8,
	kUnicodeHFSPlusCompVariant = 9,
};

typedef UInt32 TextEncoding;
typedef UInt32 TextEncodingBase;
typedef UInt32 TextEncodingFormat;
typedef UInt32 TextEncodingVariant;

TextEncoding CreateTextEncoding(TextEncodingBase encodingBase, TextEncodingVariant encodingVariant, TextEncodingFormat encodingFormat);

#ifdef __cplusplus
}
#endif

#endif

