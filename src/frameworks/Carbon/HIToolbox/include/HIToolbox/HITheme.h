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

#ifndef _HITOOLBOX_HITHEME_H_
#define _HITOOLBOX_HITHEME_H_

#include <CoreGraphics/CoreGraphics.h>
#include <CoreServices/CoreServices.h>
#include <HIToolbox/Appearance.h>
#include <HIToolbox/HIGeometry.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef UInt32 ThemeMetric;

typedef enum HIThemeOrientation : UInt32 {
    kHIThemeOrientationNormal = 0,
    kHIThemeOrientationInverted = 1,
} HIThemeOrientation;

typedef struct HIThemeBackgroundDrawInfo {
    UInt32 version;
    ThemeDrawState state;
    ThemeBackgroundKind kind;
} HIThemeBackgroundDrawInfo;

typedef struct HIThemeAnimationTimeInfo {
    CFAbsoluteTime start;
    CFAbsoluteTime current;
} HIThemeAnimationTimeInfo;

typedef struct HIThemeAnimationFrameInfo {
    UInt32 index;
} HIThemeAnimationFrameInfo;

typedef struct HIThemeButtonDrawInfo {
    UInt32 version;
    ThemeDrawState state;
    enum ThemeButtonKind kind;
    enum ThemeButtonValue value;
    enum ThemeButtonAdornment adornment;
    union {
        HIThemeAnimationTimeInfo time;
        HIThemeAnimationFrameInfo frame;
    } animation;
} HIThemeButtonDrawInfo;

typedef UInt32 HIThemeFrameKind;

typedef struct HIThemeFrameDrawInfo {
    UInt32 version;
    HIThemeFrameKind kind;
    ThemeDrawState state;
    Boolean isFocused;
} HIThemeFrameDrawInfo;

typedef enum HIThemeSplitterAdornment : UInt32 {
    kHIThemeSplitterAdornmentNone = 0,
    kHIThemeSplitterAdornmentMetal = 1
} HIThemeSplitterAdornment;

typedef struct HIThemeSplitterDrawInfo {
    UInt32 version;
    ThemeDrawState state;
    enum HIThemeSplitterAdornment adornment;
} HIThemeSplitterDrawInfo;

extern OSStatus HIThemeDrawBackground(const HIRect *inBounds, const HIThemeBackgroundDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation);

extern OSStatus HIThemeDrawButton(const HIRect *inBounds, const HIThemeButtonDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation, HIRect *outLabelRect);

extern OSStatus HIThemeDrawFocusRect(const HIRect *inRect, Boolean inHasFocus, CGContextRef inContext, HIThemeOrientation inOrientation);

extern OSStatus HIThemeDrawFrame(const HIRect *inRect, const HIThemeFrameDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation);

extern OSStatus HIThemeDrawPaneSplitter(const HIRect *inRect, const HIThemeSplitterDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation);

extern OSStatus HIThemeBrushCreateCGColor(enum ThemeBrush inBrush, CGColorRef *outColor);

extern OSStatus GetThemeMetric(ThemeMetric inMetric, SInt32 *outMetric);

#ifdef __cplusplus
}
#endif

#endif // _HITOOLBOX_HITHEME_H_
