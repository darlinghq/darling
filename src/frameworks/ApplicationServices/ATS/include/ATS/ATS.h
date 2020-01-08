/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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


#ifndef _ATS_H_
#define _ATS_H_

#include <MacTypes.h>
#include <stdint.h>

typedef UInt32 FMFont;

typedef uint32_t ATSFontRef;
typedef OptionBits ATSOptionFlags;
typedef void* ATSUTextLayout;
typedef void* ATSUStyle;
typedef uint32_t ATSUAttributeTag;
typedef Fixed ATSUTextMeasurement;
typedef void* ATSUAttributeValuePtr;

typedef uint32_t UniCharArrayOffset;
typedef char* ConstUniCharArrayPtr;

typedef void* ATSFontMetrics;

#endif
