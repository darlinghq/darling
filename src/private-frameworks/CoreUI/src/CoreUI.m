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


#include <CoreUI/CoreUI.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* CSIEqualMetrics(void)
{
    if (verbose) puts("STUB: CSIEqualMetrics called");
    return NULL;
}

void* CUIBlendModeDescriptionFromCGBlendMode(void)
{
    if (verbose) puts("STUB: CUIBlendModeDescriptionFromCGBlendMode called");
    return NULL;
}

void* CUIBlendModeLSRDescriptionFromCGBlendMode(void)
{
    if (verbose) puts("STUB: CUIBlendModeLSRDescriptionFromCGBlendMode called");
    return NULL;
}

void* CUICFArrayCreateWithCGRects(void)
{
    if (verbose) puts("STUB: CUICFArrayCreateWithCGRects called");
    return NULL;
}

void* CUICFDictionaryGetCGRect(void)
{
    if (verbose) puts("STUB: CUICFDictionaryGetCGRect called");
    return NULL;
}

void* CUICFDictionarySetCFArrayOfCGRects(void)
{
    if (verbose) puts("STUB: CUICFDictionarySetCFArrayOfCGRects called");
    return NULL;
}

void* CUICFDictionarySetCGRect(void)
{
    if (verbose) puts("STUB: CUICFDictionarySetCGRect called");
    return NULL;
}

void* CUICFDictionarySetSegmentSeparatorTypes(void)
{
    if (verbose) puts("STUB: CUICFDictionarySetSegmentSeparatorTypes called");
    return NULL;
}

void* CUICFDictionarySetSegmentType(void)
{
    if (verbose) puts("STUB: CUICFDictionarySetSegmentType called");
    return NULL;
}

void* CUICGBlendModeFromLSRDescription(void)
{
    if (verbose) puts("STUB: CUICGBlendModeFromLSRDescription called");
    return NULL;
}

void* CUICGImageGetAlphaInfo(void)
{
    if (verbose) puts("STUB: CUICGImageGetAlphaInfo called");
    return NULL;
}

void* CUICalcNonAlphaAreaOfImage(void)
{
    if (verbose) puts("STUB: CUICalcNonAlphaAreaOfImage called");
    return NULL;
}

void* CUICalculateExtrusionMaskOfImage(void)
{
    if (verbose) puts("STUB: CUICalculateExtrusionMaskOfImage called");
    return NULL;
}

void* CUIColorCreateFromColorWithSystemEffectOptions(void)
{
    if (verbose) puts("STUB: CUIColorCreateFromColorWithSystemEffectOptions called");
    return NULL;
}

void* CUIColorHistogramARGBIntersection(void)
{
    if (verbose) puts("STUB: CUIColorHistogramARGBIntersection called");
    return NULL;
}

void* CUIColorHistogramARGBIsMonochrome(void)
{
    if (verbose) puts("STUB: CUIColorHistogramARGBIsMonochrome called");
    return NULL;
}

void* CUICompareRenditionKey(void)
{
    if (verbose) puts("STUB: CUICompareRenditionKey called");
    return NULL;
}

void* CUIComputeImageColorHistogramARGB(void)
{
    if (verbose) puts("STUB: CUIComputeImageColorHistogramARGB called");
    return NULL;
}

void* CUIConvertCGImageFormat(void)
{
    if (verbose) puts("STUB: CUIConvertCGImageFormat called");
    return NULL;
}

void* CUIConvertCompressionTypeToString(void)
{
    if (verbose) puts("STUB: CUIConvertCompressionTypeToString called");
    return NULL;
}

void* CUIConvertDeepImageTo8(void)
{
    if (verbose) puts("STUB: CUIConvertDeepImageTo8 called");
    return NULL;
}

void* CUIConvertImageToPixelFormat(void)
{
    if (verbose) puts("STUB: CUIConvertImageToPixelFormat called");
    return NULL;
}

void* CUIConvertPixelFormatToString(void)
{
    if (verbose) puts("STUB: CUIConvertPixelFormatToString called");
    return NULL;
}

void* CUICopyCustomColor(void)
{
    if (verbose) puts("STUB: CUICopyCustomColor called");
    return NULL;
}

void* CUICopyCustomColorWithOptions(void)
{
    if (verbose) puts("STUB: CUICopyCustomColorWithOptions called");
    return NULL;
}

void* CUICopyCustomFontDescriptor(void)
{
    if (verbose) puts("STUB: CUICopyCustomFontDescriptor called");
    return NULL;
}

void* CUICopyMeasurements(void)
{
    if (verbose) puts("STUB: CUICopyMeasurements called");
    return NULL;
}

void* CUICreateLayerContents(void)
{
    if (verbose) puts("STUB: CUICreateLayerContents called");
    return NULL;
}

void* CUICreateMenuPath(void)
{
    if (verbose) puts("STUB: CUICreateMenuPath called");
    return NULL;
}

void* CUICreateOrUpdateLayer(void)
{
    if (verbose) puts("STUB: CUICreateOrUpdateLayer called");
    return NULL;
}

void* CUICreateRenderer(void)
{
    if (verbose) puts("STUB: CUICreateRenderer called");
    return NULL;
}

void* CUICurrentPlatform(void)
{
    if (verbose) puts("STUB: CUICurrentPlatform called");
    return NULL;
}

void* CUIDeallocateRGBAImageBuffer(void)
{
    if (verbose) puts("STUB: CUIDeallocateRGBAImageBuffer called");
    return NULL;
}

void* CUIDefaultThemeRenditionKeyFormat(void)
{
    if (verbose) puts("STUB: CUIDefaultThemeRenditionKeyFormat called");
    return NULL;
}

void* CUIDraw(void)
{
    if (verbose) puts("STUB: CUIDraw called");
    return NULL;
}

void* CUIDrawExtrudedImageInContext(void)
{
    if (verbose) puts("STUB: CUIDrawExtrudedImageInContext called");
    return NULL;
}

void* CUIDrawStyledGlyphsAtPositions(void)
{
    if (verbose) puts("STUB: CUIDrawStyledGlyphsAtPositions called");
    return NULL;
}

void* CUIDrawStyledGlyphsWithAdvances(void)
{
    if (verbose) puts("STUB: CUIDrawStyledGlyphsWithAdvances called");
    return NULL;
}

void* CUIEdgeInsetsEqualToEdgeInsets(void)
{
    if (verbose) puts("STUB: CUIEdgeInsetsEqualToEdgeInsets called");
    return NULL;
}

void* CUIEdgeInsetsFromCGRect(void)
{
    if (verbose) puts("STUB: CUIEdgeInsetsFromCGRect called");
    return NULL;
}

void* CUIEdgeInsetsInsetRect(void)
{
    if (verbose) puts("STUB: CUIEdgeInsetsInsetRect called");
    return NULL;
}

void* CUIEdgeInsetsMake(void)
{
    if (verbose) puts("STUB: CUIEdgeInsetsMake called");
    return NULL;
}

void* CUIEdgeInsetsOutsetRect(void)
{
    if (verbose) puts("STUB: CUIEdgeInsetsOutsetRect called");
    return NULL;
}

void* CUIEdgeInsetsToCGRect(void)
{
    if (verbose) puts("STUB: CUIEdgeInsetsToCGRect called");
    return NULL;
}

void* CUIEdgeInsetsValid(void)
{
    if (verbose) puts("STUB: CUIEdgeInsetsValid called");
    return NULL;
}

void* CUIEffectBevelStyleDescription(void)
{
    if (verbose) puts("STUB: CUIEffectBevelStyleDescription called");
    return NULL;
}

void* CUIEffectBlendModeDescription(void)
{
    if (verbose) puts("STUB: CUIEffectBlendModeDescription called");
    return NULL;
}

void* CUIEqualRenditionKeys(void)
{
    if (verbose) puts("STUB: CUIEqualRenditionKeys called");
    return NULL;
}

void* CUIEqualRenditionKeysIgnoringAttribute(void)
{
    if (verbose) puts("STUB: CUIEqualRenditionKeysIgnoringAttribute called");
    return NULL;
}

void* CUIExists(void)
{
    if (verbose) puts("STUB: CUIExists called");
    return NULL;
}

void* CUIFillCARKeyArrayForRenditionKey(void)
{
    if (verbose) puts("STUB: CUIFillCARKeyArrayForRenditionKey called");
    return NULL;
}

void* CUIFillRenditionKeyForCARKeyArray(void)
{
    if (verbose) puts("STUB: CUIFillRenditionKeyForCARKeyArray called");
    return NULL;
}

void* CUIGetChannelDataForImage(void)
{
    if (verbose) puts("STUB: CUIGetChannelDataForImage called");
    return NULL;
}

void* CUIGetCustomFontSize(void)
{
    if (verbose) puts("STUB: CUIGetCustomFontSize called");
    return NULL;
}

void* CUIGetDebugValue(void)
{
    if (verbose) puts("STUB: CUIGetDebugValue called");
    return NULL;
}

void* CUIGetDeviceArtworkDisplayGamut(void)
{
    if (verbose) puts("STUB: CUIGetDeviceArtworkDisplayGamut called");
    return NULL;
}

void* CUIGetFormatForFeatureSetAndPixelFormat(void)
{
    if (verbose) puts("STUB: CUIGetFormatForFeatureSetAndPixelFormat called");
    return NULL;
}

void* CUIGetRGBAImageBuffer(void)
{
    if (verbose) puts("STUB: CUIGetRGBAImageBuffer called");
    return NULL;
}

void* CUIHitTest(void)
{
    if (verbose) puts("STUB: CUIHitTest called");
    return NULL;
}

void* CUIImageCompressedWithColorQuantization(void)
{
    if (verbose) puts("STUB: CUIImageCompressedWithColorQuantization called");
    return NULL;
}

void* CUIImageIsMonochrome(void)
{
    if (verbose) puts("STUB: CUIImageIsMonochrome called");
    return NULL;
}

void* CUIImageIsWideGamut(void)
{
    if (verbose) puts("STUB: CUIImageIsWideGamut called");
    return NULL;
}

void* CUIKeyFormatToArrayString(void)
{
    if (verbose) puts("STUB: CUIKeyFormatToArrayString called");
    return NULL;
}

void* CUILogLogToStderr(void)
{
    if (verbose) puts("STUB: CUILogLogToStderr called");
    return NULL;
}

void* CUIMaxDimensionForVectorGlyph(void)
{
    if (verbose) puts("STUB: CUIMaxDimensionForVectorGlyph called");
    return NULL;
}

void* CUIMaxScaleForTargetPlatform(void)
{
    if (verbose) puts("STUB: CUIMaxScaleForTargetPlatform called");
    return NULL;
}

void* CUIMinDimensionForVectorGlyph(void)
{
    if (verbose) puts("STUB: CUIMinDimensionForVectorGlyph called");
    return NULL;
}

void* CUIMinScaleForTargetPlatform(void)
{
    if (verbose) puts("STUB: CUIMinScaleForTargetPlatform called");
    return NULL;
}

void* CUIParseDeploymentTarget(void)
{
    if (verbose) puts("STUB: CUIParseDeploymentTarget called");
    return NULL;
}

void* CUIPlaceHolderRendition(void)
{
    if (verbose) puts("STUB: CUIPlaceHolderRendition called");
    return NULL;
}

void* CUIPlatformNameForPlatform(void)
{
    if (verbose) puts("STUB: CUIPlatformNameForPlatform called");
    return NULL;
}

void* CUIPointSizeForDimensionForVectorGlyph(void)
{
    if (verbose) puts("STUB: CUIPointSizeForDimensionForVectorGlyph called");
    return NULL;
}

void* CUIRendererCopyProperties(void)
{
    if (verbose) puts("STUB: CUIRendererCopyProperties called");
    return NULL;
}

void* CUIRendererSetProperties(void)
{
    if (verbose) puts("STUB: CUIRendererSetProperties called");
    return NULL;
}

void* CUIRenditionKeyCopy(void)
{
    if (verbose) puts("STUB: CUIRenditionKeyCopy called");
    return NULL;
}

void* CUIRenditionKeyFormatRemoveAttribute(void)
{
    if (verbose) puts("STUB: CUIRenditionKeyFormatRemoveAttribute called");
    return NULL;
}

void* CUIRenditionKeyHasIdentifier(void)
{
    if (verbose) puts("STUB: CUIRenditionKeyHasIdentifier called");
    return NULL;
}

void* CUIRenditionKeySetValueForAttribute(void)
{
    if (verbose) puts("STUB: CUIRenditionKeySetValueForAttribute called");
    return NULL;
}

void* CUIRenditionKeyStandardize(void)
{
    if (verbose) puts("STUB: CUIRenditionKeyStandardize called");
    return NULL;
}

void* CUIRenditionKeyTokenCount(void)
{
    if (verbose) puts("STUB: CUIRenditionKeyTokenCount called");
    return NULL;
}

void* CUIRenditionKeyValueForAttribute(void)
{
    if (verbose) puts("STUB: CUIRenditionKeyValueForAttribute called");
    return NULL;
}

void* CUISetCustomColorInContext(void)
{
    if (verbose) puts("STUB: CUISetCustomColorInContext called");
    return NULL;
}

void* CUISetDebugValue(void)
{
    if (verbose) puts("STUB: CUISetDebugValue called");
    return NULL;
}

void* CUISetUpContext(void)
{
    if (verbose) puts("STUB: CUISetUpContext called");
    return NULL;
}

void* CUISystemThemeRenditionKeyFormat(void)
{
    if (verbose) puts("STUB: CUISystemThemeRenditionKeyFormat called");
    return NULL;
}

void* CUIThemeInfoVersion(void)
{
    if (verbose) puts("STUB: CUIThemeInfoVersion called");
    return NULL;
}

void* CUIUncompressQuantizedImageData(void)
{
    if (verbose) puts("STUB: CUIUncompressQuantizedImageData called");
    return NULL;
}

void* _CUILog(void)
{
    if (verbose) puts("STUB: _CUILog called");
    return NULL;
}

void* _LookupStructuredThemeProvider(void)
{
    if (verbose) puts("STUB: _LookupStructuredThemeProvider called");
    return NULL;
}

void* _RegisterThemeProvider(void)
{
    if (verbose) puts("STUB: _RegisterThemeProvider called");
    return NULL;
}
