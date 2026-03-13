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

#include <HIToolbox/HITheme.h>

OSStatus HIThemeDrawBackground(const HIRect *inBounds, const HIThemeBackgroundDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);

    return unimpErr;
}

OSStatus HIThemeDrawButton(const HIRect *inBounds, const HIThemeButtonDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation, HIRect *outLabelRect)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);

    return unimpErr;
}

OSStatus HIThemeDrawFocusRect(const HIRect *inRect, Boolean inHasFocus, CGContextRef inContext, HIThemeOrientation inOrientation)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);

    return unimpErr;
}

OSStatus HIThemeDrawFrame(const HIRect *inRect, const HIThemeFrameDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);

    return unimpErr;
}

OSStatus HIThemeDrawPaneSplitter(const HIRect *inRect, const HIThemeSplitterDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);

    return unimpErr;
}

OSStatus HIThemeBrushCreateCGColor(enum ThemeBrush inBrush, CGColorRef *outColor)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);

    return unimpErr;
}

OSStatus GetThemeMetric(ThemeMetric inMetric, SInt32 *outMetric)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);

    return unimpErr;
}
