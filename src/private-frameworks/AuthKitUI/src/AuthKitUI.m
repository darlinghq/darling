/*
 This file is part of Darling.

 Copyright (C) 2021 Lubos Dolezel

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


#include <AuthKitUI/AuthKitUI.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* AKColorSpaceStandardRGB(void)
{
    if (verbose) puts("STUB: AKColorSpaceStandardRGB called");
    return NULL;
}

void* AKContentAlignmentMake(void)
{
    if (verbose) puts("STUB: AKContentAlignmentMake called");
    return NULL;
}

void* AKCreateAppleIDButtonImage(void)
{
    if (verbose) puts("STUB: AKCreateAppleIDButtonImage called");
    return NULL;
}

void* AKCreateAppleIDButtonImageWithCornerRadius(void)
{
    if (verbose) puts("STUB: AKCreateAppleIDButtonImageWithCornerRadius called");
    return NULL;
}

void* AKDrawAppleIDButton(void)
{
    if (verbose) puts("STUB: AKDrawAppleIDButton called");
    return NULL;
}

void* AKDrawAppleIDButtonWithCornerRadius(void)
{
    if (verbose) puts("STUB: AKDrawAppleIDButtonWithCornerRadius called");
    return NULL;
}

void* AKFloatRoundToPixelWithScale(void)
{
    if (verbose) puts("STUB: AKFloatRoundToPixelWithScale called");
    return NULL;
}

void* AKPointRoundToPixelWithScale(void)
{
    if (verbose) puts("STUB: AKPointRoundToPixelWithScale called");
    return NULL;
}

void* AKRectRoundToPixelWithScale(void)
{
    if (verbose) puts("STUB: AKRectRoundToPixelWithScale called");
    return NULL;
}

void* AKSizeAlignedInRectWithScale(void)
{
    if (verbose) puts("STUB: AKSizeAlignedInRectWithScale called");
    return NULL;
}

void* AKSizeRoundToPixelWithScale(void)
{
    if (verbose) puts("STUB: AKSizeRoundToPixelWithScale called");
    return NULL;
}

void* AKSizeScaleAspectFit(void)
{
    if (verbose) puts("STUB: AKSizeScaleAspectFit called");
    return NULL;
}

void* AddGlobalObject(void)
{
    if (verbose) puts("STUB: AddGlobalObject called");
    return NULL;
}

void* CallJSFunctionWithParameters(void)
{
    if (verbose) puts("STUB: CallJSFunctionWithParameters called");
    return NULL;
}

void* EvaluateJavaScript(void)
{
    if (verbose) puts("STUB: EvaluateJavaScript called");
    return NULL;
}

void* JSCopyStringValue(void)
{
    if (verbose) puts("STUB: JSCopyStringValue called");
    return NULL;
}

void* JSValueFromNSString(void)
{
    if (verbose) puts("STUB: JSValueFromNSString called");
    return NULL;
}

void* JSValueRefsAreEqual(void)
{
    if (verbose) puts("STUB: JSValueRefsAreEqual called");
    return NULL;
}

void* NSObjectFromJSValue(void)
{
    if (verbose) puts("STUB: NSObjectFromJSValue called");
    return NULL;
}

void* NSStringFromJSString(void)
{
    if (verbose) puts("STUB: NSStringFromJSString called");
    return NULL;
}

void* NSStringFromJSValue(void)
{
    if (verbose) puts("STUB: NSStringFromJSValue called");
    return NULL;
}

void* _AKUIMainBundle(void)
{
    if (verbose) puts("STUB: _AKUIMainBundle called");
    return NULL;
}

void* _CreateAttributedStringWithFontSize(void)
{
    if (verbose) puts("STUB: _CreateAttributedStringWithFontSize called");
    return NULL;
}

void* _DrawBorder(void)
{
    if (verbose) puts("STUB: _DrawBorder called");
    return NULL;
}

void* _DrawLogoPDFIntoContext(void)
{
    if (verbose) puts("STUB: _DrawLogoPDFIntoContext called");
    return NULL;
}

void* _IsJSValueAnArray(void)
{
    if (verbose) puts("STUB: _IsJSValueAnArray called");
    return NULL;
}

void* _MaximumFontSizeForStringInRect(void)
{
    if (verbose) puts("STUB: _MaximumFontSizeForStringInRect called");
    return NULL;
}

void* _NSDictionaryFromJSObject(void)
{
    if (verbose) puts("STUB: _NSDictionaryFromJSObject called");
    return NULL;
}

void* vsCallJSFunction(void)
{
    if (verbose) puts("STUB: vsCallJSFunction called");
    return NULL;
}
