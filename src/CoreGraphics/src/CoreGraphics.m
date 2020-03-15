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


#include <CoreGraphics/CoreGraphics.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* CFTypeCreateWithCGPDFObject(void)
{
    if (verbose) puts("STUB: CFTypeCreateWithCGPDFObject called");
    return NULL;
}

void* CGAccessSessionCreate(void)
{
    if (verbose) puts("STUB: CGAccessSessionCreate called");
    return NULL;
}

void* CGAccessSessionGetBytePointer(void)
{
    if (verbose) puts("STUB: CGAccessSessionGetBytePointer called");
    return NULL;
}

void* CGAccessSessionGetBytes(void)
{
    if (verbose) puts("STUB: CGAccessSessionGetBytes called");
    return NULL;
}

void* CGAccessSessionGetChunks(void)
{
    if (verbose) puts("STUB: CGAccessSessionGetChunks called");
    return NULL;
}

void* CGAccessSessionRelease(void)
{
    if (verbose) puts("STUB: CGAccessSessionRelease called");
    return NULL;
}

void* CGAccessSessionRewind(void)
{
    if (verbose) puts("STUB: CGAccessSessionRewind called");
    return NULL;
}

void* CGAccessSessionSkipBytes(void)
{
    if (verbose) puts("STUB: CGAccessSessionSkipBytes called");
    return NULL;
}

void* CGAccessSessionSkipForward(void)
{
    if (verbose) puts("STUB: CGAccessSessionSkipForward called");
    return NULL;
}

void* CGAffineTransformConcat(void)
{
    if (verbose) puts("STUB: CGAffineTransformConcat called");
    return NULL;
}

void* CGAffineTransformEqualToTransform(void)
{
    if (verbose) puts("STUB: CGAffineTransformEqualToTransform called");
    return NULL;
}

void* CGAffineTransformInvert(void)
{
    if (verbose) puts("STUB: CGAffineTransformInvert called");
    return NULL;
}

void* CGAffineTransformIsIdentity(void)
{
    if (verbose) puts("STUB: CGAffineTransformIsIdentity called");
    return NULL;
}

void* CGAffineTransformIsRectilinear(void)
{
    if (verbose) puts("STUB: CGAffineTransformIsRectilinear called");
    return NULL;
}

void* CGAffineTransformIsSingular(void)
{
    if (verbose) puts("STUB: CGAffineTransformIsSingular called");
    return NULL;
}

void* CGAffineTransformMake(void)
{
    if (verbose) puts("STUB: CGAffineTransformMake called");
    return NULL;
}

void* CGAffineTransformMakeRotation(void)
{
    if (verbose) puts("STUB: CGAffineTransformMakeRotation called");
    return NULL;
}

void* CGAffineTransformMakeScale(void)
{
    if (verbose) puts("STUB: CGAffineTransformMakeScale called");
    return NULL;
}

void* CGAffineTransformMakeTranslation(void)
{
    if (verbose) puts("STUB: CGAffineTransformMakeTranslation called");
    return NULL;
}

void* CGAffineTransformMakeWithRect(void)
{
    if (verbose) puts("STUB: CGAffineTransformMakeWithRect called");
    return NULL;
}

void* CGAffineTransformRotate(void)
{
    if (verbose) puts("STUB: CGAffineTransformRotate called");
    return NULL;
}

void* CGAffineTransformScale(void)
{
    if (verbose) puts("STUB: CGAffineTransformScale called");
    return NULL;
}

void* CGAffineTransformTranslate(void)
{
    if (verbose) puts("STUB: CGAffineTransformTranslate called");
    return NULL;
}

void* CGApplicationPropertyGetBoolean(void)
{
    if (verbose) puts("STUB: CGApplicationPropertyGetBoolean called");
    return NULL;
}

void* CGBitmapAllocateData(void)
{
    if (verbose) puts("STUB: CGBitmapAllocateData called");
    return NULL;
}

void* CGBitmapContextCopyData(void)
{
    if (verbose) puts("STUB: CGBitmapContextCopyData called");
    return NULL;
}

void* CGBitmapContextCreate(void)
{
    if (verbose) puts("STUB: CGBitmapContextCreate called");
    return NULL;
}

void* CGBitmapContextCreateImage(void)
{
    if (verbose) puts("STUB: CGBitmapContextCreateImage called");
    return NULL;
}

void* CGBitmapContextCreateWithAlpha(void)
{
    if (verbose) puts("STUB: CGBitmapContextCreateWithAlpha called");
    return NULL;
}

void* CGBitmapContextCreateWithCallbacks(void)
{
    if (verbose) puts("STUB: CGBitmapContextCreateWithCallbacks called");
    return NULL;
}

void* CGBitmapContextCreateWithData(void)
{
    if (verbose) puts("STUB: CGBitmapContextCreateWithData called");
    return NULL;
}

void* CGBitmapContextCreateWithDictionary(void)
{
    if (verbose) puts("STUB: CGBitmapContextCreateWithDictionary called");
    return NULL;
}

void* CGBitmapContextCreateWithPlanarAlpha(void)
{
    if (verbose) puts("STUB: CGBitmapContextCreateWithPlanarAlpha called");
    return NULL;
}

void* CGBitmapContextDelegateCreateWithDictionary(void)
{
    if (verbose) puts("STUB: CGBitmapContextDelegateCreateWithDictionary called");
    return NULL;
}

void* CGBitmapContextGetAlphaBytesPerRow(void)
{
    if (verbose) puts("STUB: CGBitmapContextGetAlphaBytesPerRow called");
    return NULL;
}

void* CGBitmapContextGetAlphaData(void)
{
    if (verbose) puts("STUB: CGBitmapContextGetAlphaData called");
    return NULL;
}

void* CGBitmapContextGetAlphaInfo(void)
{
    if (verbose) puts("STUB: CGBitmapContextGetAlphaInfo called");
    return NULL;
}

void* CGBitmapContextGetBitmapInfo(void)
{
    if (verbose) puts("STUB: CGBitmapContextGetBitmapInfo called");
    return NULL;
}

void* CGBitmapContextGetBitsPerComponent(void)
{
    if (verbose) puts("STUB: CGBitmapContextGetBitsPerComponent called");
    return NULL;
}

void* CGBitmapContextGetBitsPerPixel(void)
{
    if (verbose) puts("STUB: CGBitmapContextGetBitsPerPixel called");
    return NULL;
}

void* CGBitmapContextGetBytesPerRow(void)
{
    if (verbose) puts("STUB: CGBitmapContextGetBytesPerRow called");
    return NULL;
}

void* CGBitmapContextGetColorSpace(void)
{
    if (verbose) puts("STUB: CGBitmapContextGetColorSpace called");
    return NULL;
}

void* CGBitmapContextGetData(void)
{
    if (verbose) puts("STUB: CGBitmapContextGetData called");
    return NULL;
}

void* CGBitmapContextGetHeight(void)
{
    if (verbose) puts("STUB: CGBitmapContextGetHeight called");
    return NULL;
}

void* CGBitmapContextGetWidth(void)
{
    if (verbose) puts("STUB: CGBitmapContextGetWidth called");
    return NULL;
}

void* CGBitmapContextSetData(void)
{
    if (verbose) puts("STUB: CGBitmapContextSetData called");
    return NULL;
}

void* CGBitmapContextWriteImageToFile(void)
{
    if (verbose) puts("STUB: CGBitmapContextWriteImageToFile called");
    return NULL;
}

void* CGBitmapFreeData(void)
{
    if (verbose) puts("STUB: CGBitmapFreeData called");
    return NULL;
}

void* CGBitmapGetAlignedBytesPerRow(void)
{
    if (verbose) puts("STUB: CGBitmapGetAlignedBytesPerRow called");
    return NULL;
}

void* CGBitmapGetFastestAlignment(void)
{
    if (verbose) puts("STUB: CGBitmapGetFastestAlignment called");
    return NULL;
}

void* CGBlt_copyBytes(void)
{
    if (verbose) puts("STUB: CGBlt_copyBytes called");
    return NULL;
}

void* CGBlt_depth(void)
{
    if (verbose) puts("STUB: CGBlt_depth called");
    return NULL;
}

void* CGBlt_fillBytes(void)
{
    if (verbose) puts("STUB: CGBlt_fillBytes called");
    return NULL;
}

void* CGBlt_initialize(void)
{
    if (verbose) puts("STUB: CGBlt_initialize called");
    return NULL;
}

void* CGBlt_release_pattern(void)
{
    if (verbose) puts("STUB: CGBlt_release_pattern called");
    return NULL;
}

void* CGBufferLockBytePtr(void)
{
    if (verbose) puts("STUB: CGBufferLockBytePtr called");
    return NULL;
}

void* CGBufferProviderCreate(void)
{
    if (verbose) puts("STUB: CGBufferProviderCreate called");
    return NULL;
}

void* CGBufferProviderCreateWithCFData(void)
{
    if (verbose) puts("STUB: CGBufferProviderCreateWithCFData called");
    return NULL;
}

void* CGBufferProviderGetSize(void)
{
    if (verbose) puts("STUB: CGBufferProviderGetSize called");
    return NULL;
}

void* CGBufferProviderGetTypeID(void)
{
    if (verbose) puts("STUB: CGBufferProviderGetTypeID called");
    return NULL;
}

void* CGBufferUnlockBytePtr(void)
{
    if (verbose) puts("STUB: CGBufferUnlockBytePtr called");
    return NULL;
}

void* CGCFArrayAppendCGFloat(void)
{
    if (verbose) puts("STUB: CGCFArrayAppendCGFloat called");
    return NULL;
}

void* CGCFArrayAppendInteger(void)
{
    if (verbose) puts("STUB: CGCFArrayAppendInteger called");
    return NULL;
}

void* CGCFArrayAppendRect(void)
{
    if (verbose) puts("STUB: CGCFArrayAppendRect called");
    return NULL;
}

void* CGCFArrayApplyBlock(void)
{
    if (verbose) puts("STUB: CGCFArrayApplyBlock called");
    return NULL;
}

void* CGCFArrayCreate(void)
{
    if (verbose) puts("STUB: CGCFArrayCreate called");
    return NULL;
}

void* CGCFArrayGetLastValue(void)
{
    if (verbose) puts("STUB: CGCFArrayGetLastValue called");
    return NULL;
}

void* CGCFArrayGetRectAtIndex(void)
{
    if (verbose) puts("STUB: CGCFArrayGetRectAtIndex called");
    return NULL;
}

void* CGCFDictionaryApplyBlock(void)
{
    if (verbose) puts("STUB: CGCFDictionaryApplyBlock called");
    return NULL;
}

void* CGCFDictionaryCopyKeys(void)
{
    if (verbose) puts("STUB: CGCFDictionaryCopyKeys called");
    return NULL;
}

void* CGCFDictionaryCopyValues(void)
{
    if (verbose) puts("STUB: CGCFDictionaryCopyValues called");
    return NULL;
}

void* CGCFDictionaryCreate(void)
{
    if (verbose) puts("STUB: CGCFDictionaryCreate called");
    return NULL;
}

void* CGCFDictionaryCreateCopy(void)
{
    if (verbose) puts("STUB: CGCFDictionaryCreateCopy called");
    return NULL;
}

void* CGCFDictionaryGetBoolean(void)
{
    if (verbose) puts("STUB: CGCFDictionaryGetBoolean called");
    return NULL;
}

void* CGCFDictionaryGetBooleanWithDefault(void)
{
    if (verbose) puts("STUB: CGCFDictionaryGetBooleanWithDefault called");
    return NULL;
}

void* CGCFDictionaryGetCFTypeRef(void)
{
    if (verbose) puts("STUB: CGCFDictionaryGetCFTypeRef called");
    return NULL;
}

void* CGCFDictionaryGetFloat(void)
{
    if (verbose) puts("STUB: CGCFDictionaryGetFloat called");
    return NULL;
}

void* CGCFDictionaryGetFloatWithDefault(void)
{
    if (verbose) puts("STUB: CGCFDictionaryGetFloatWithDefault called");
    return NULL;
}

void* CGCFDictionaryGetInteger(void)
{
    if (verbose) puts("STUB: CGCFDictionaryGetInteger called");
    return NULL;
}

void* CGCFDictionaryGetIntegerWithDefault(void)
{
    if (verbose) puts("STUB: CGCFDictionaryGetIntegerWithDefault called");
    return NULL;
}

void* CGCFDictionaryGetNumber(void)
{
    if (verbose) puts("STUB: CGCFDictionaryGetNumber called");
    return NULL;
}

void* CGCFDictionaryGetPoint(void)
{
    if (verbose) puts("STUB: CGCFDictionaryGetPoint called");
    return NULL;
}

void* CGCFDictionaryGetRect(void)
{
    if (verbose) puts("STUB: CGCFDictionaryGetRect called");
    return NULL;
}

void* CGCFDictionaryGetSize(void)
{
    if (verbose) puts("STUB: CGCFDictionaryGetSize called");
    return NULL;
}

void* CGCFDictionarySetBoolean(void)
{
    if (verbose) puts("STUB: CGCFDictionarySetBoolean called");
    return NULL;
}

void* CGCFDictionarySetCFTypeRef(void)
{
    if (verbose) puts("STUB: CGCFDictionarySetCFTypeRef called");
    return NULL;
}

void* CGCFDictionarySetCString(void)
{
    if (verbose) puts("STUB: CGCFDictionarySetCString called");
    return NULL;
}

void* CGCFDictionarySetFloat(void)
{
    if (verbose) puts("STUB: CGCFDictionarySetFloat called");
    return NULL;
}

void* CGCFDictionarySetInteger(void)
{
    if (verbose) puts("STUB: CGCFDictionarySetInteger called");
    return NULL;
}

void* CGCFDictionarySetNumber(void)
{
    if (verbose) puts("STUB: CGCFDictionarySetNumber called");
    return NULL;
}

void* CGCFDictionarySetPoint(void)
{
    if (verbose) puts("STUB: CGCFDictionarySetPoint called");
    return NULL;
}

void* CGCFDictionarySetRect(void)
{
    if (verbose) puts("STUB: CGCFDictionarySetRect called");
    return NULL;
}

void* CGCFDictionarySetSize(void)
{
    if (verbose) puts("STUB: CGCFDictionarySetSize called");
    return NULL;
}

void* CGCFRelease(void)
{
    if (verbose) puts("STUB: CGCFRelease called");
    return NULL;
}

void* CGCFRetain(void)
{
    if (verbose) puts("STUB: CGCFRetain called");
    return NULL;
}

void* CGCFURLCopyFileData(void)
{
    if (verbose) puts("STUB: CGCFURLCopyFileData called");
    return NULL;
}

void* CGCMSUtilsCreatePlatformRefFromData(void)
{
    if (verbose) puts("STUB: CGCMSUtilsCreatePlatformRefFromData called");
    return NULL;
}

void* CGCMSUtilsGetICCProfileData(void)
{
    if (verbose) puts("STUB: CGCMSUtilsGetICCProfileData called");
    return NULL;
}

void* CGCMSUtilsGetICCProfileDescription(void)
{
    if (verbose) puts("STUB: CGCMSUtilsGetICCProfileDescription called");
    return NULL;
}

void* CGCMSUtilsGetProcessColorModel(void)
{
    if (verbose) puts("STUB: CGCMSUtilsGetProcessColorModel called");
    return NULL;
}

void* CGCMSUtilsReleasePlatformRef(void)
{
    if (verbose) puts("STUB: CGCMSUtilsReleasePlatformRef called");
    return NULL;
}

void* CGClipCreateClipPath(void)
{
    if (verbose) puts("STUB: CGClipCreateClipPath called");
    return NULL;
}

void* CGClipGetFlatness(void)
{
    if (verbose) puts("STUB: CGClipGetFlatness called");
    return NULL;
}

void* CGClipGetIdentifier(void)
{
    if (verbose) puts("STUB: CGClipGetIdentifier called");
    return NULL;
}

void* CGClipGetMask(void)
{
    if (verbose) puts("STUB: CGClipGetMask called");
    return NULL;
}

void* CGClipGetMode(void)
{
    if (verbose) puts("STUB: CGClipGetMode called");
    return NULL;
}

void* CGClipGetPath(void)
{
    if (verbose) puts("STUB: CGClipGetPath called");
    return NULL;
}

void* CGClipGetRect(void)
{
    if (verbose) puts("STUB: CGClipGetRect called");
    return NULL;
}

void* CGClipGetShouldAntialias(void)
{
    if (verbose) puts("STUB: CGClipGetShouldAntialias called");
    return NULL;
}

void* CGClipGetStroke(void)
{
    if (verbose) puts("STUB: CGClipGetStroke called");
    return NULL;
}

void* CGClipGetTextClipping(void)
{
    if (verbose) puts("STUB: CGClipGetTextClipping called");
    return NULL;
}

void* CGClipGetType(void)
{
    if (verbose) puts("STUB: CGClipGetType called");
    return NULL;
}

void* CGClipGlyphsCreate(void)
{
    if (verbose) puts("STUB: CGClipGlyphsCreate called");
    return NULL;
}

void* CGClipGlyphsCreateCopy(void)
{
    if (verbose) puts("STUB: CGClipGlyphsCreateCopy called");
    return NULL;
}

void* CGClipGlyphsCreateCopyByApplyingTransform(void)
{
    if (verbose) puts("STUB: CGClipGlyphsCreateCopyByApplyingTransform called");
    return NULL;
}

void* CGClipGlyphsEqualToClipGlyphs(void)
{
    if (verbose) puts("STUB: CGClipGlyphsEqualToClipGlyphs called");
    return NULL;
}

void* CGClipGlyphsGetAdvanceAtIndex(void)
{
    if (verbose) puts("STUB: CGClipGlyphsGetAdvanceAtIndex called");
    return NULL;
}

void* CGClipGlyphsGetAdvances(void)
{
    if (verbose) puts("STUB: CGClipGlyphsGetAdvances called");
    return NULL;
}

void* CGClipGlyphsGetBounds(void)
{
    if (verbose) puts("STUB: CGClipGlyphsGetBounds called");
    return NULL;
}

void* CGClipGlyphsGetCount(void)
{
    if (verbose) puts("STUB: CGClipGlyphsGetCount called");
    return NULL;
}

void* CGClipGlyphsGetFont(void)
{
    if (verbose) puts("STUB: CGClipGlyphsGetFont called");
    return NULL;
}

void* CGClipGlyphsGetFontAtIndex(void)
{
    if (verbose) puts("STUB: CGClipGlyphsGetFontAtIndex called");
    return NULL;
}

void* CGClipGlyphsGetFontRenderingStyle(void)
{
    if (verbose) puts("STUB: CGClipGlyphsGetFontRenderingStyle called");
    return NULL;
}

void* CGClipGlyphsGetFontRenderingStyleAtIndex(void)
{
    if (verbose) puts("STUB: CGClipGlyphsGetFontRenderingStyleAtIndex called");
    return NULL;
}

void* CGClipGlyphsGetGlyphAtIndex(void)
{
    if (verbose) puts("STUB: CGClipGlyphsGetGlyphAtIndex called");
    return NULL;
}

void* CGClipGlyphsGetGlyphs(void)
{
    if (verbose) puts("STUB: CGClipGlyphsGetGlyphs called");
    return NULL;
}

void* CGClipGlyphsGetLength(void)
{
    if (verbose) puts("STUB: CGClipGlyphsGetLength called");
    return NULL;
}

void* CGClipGlyphsGetMatrix(void)
{
    if (verbose) puts("STUB: CGClipGlyphsGetMatrix called");
    return NULL;
}

void* CGClipGlyphsGetMatrixAtIndex(void)
{
    if (verbose) puts("STUB: CGClipGlyphsGetMatrixAtIndex called");
    return NULL;
}

void* CGClipGlyphsRelease(void)
{
    if (verbose) puts("STUB: CGClipGlyphsRelease called");
    return NULL;
}

void* CGClipGlyphsRetain(void)
{
    if (verbose) puts("STUB: CGClipGlyphsRetain called");
    return NULL;
}

void* CGClipMaskCreate(void)
{
    if (verbose) puts("STUB: CGClipMaskCreate called");
    return NULL;
}

void* CGClipMaskCreateCopy(void)
{
    if (verbose) puts("STUB: CGClipMaskCreateCopy called");
    return NULL;
}

void* CGClipMaskCreateCopyByApplyingTransform(void)
{
    if (verbose) puts("STUB: CGClipMaskCreateCopyByApplyingTransform called");
    return NULL;
}

void* CGClipMaskEqualToClipMask(void)
{
    if (verbose) puts("STUB: CGClipMaskEqualToClipMask called");
    return NULL;
}

void* CGClipMaskGetBounds(void)
{
    if (verbose) puts("STUB: CGClipMaskGetBounds called");
    return NULL;
}

void* CGClipMaskGetImage(void)
{
    if (verbose) puts("STUB: CGClipMaskGetImage called");
    return NULL;
}

void* CGClipMaskGetMatrix(void)
{
    if (verbose) puts("STUB: CGClipMaskGetMatrix called");
    return NULL;
}

void* CGClipMaskGetRect(void)
{
    if (verbose) puts("STUB: CGClipMaskGetRect called");
    return NULL;
}

void* CGClipMaskRelease(void)
{
    if (verbose) puts("STUB: CGClipMaskRelease called");
    return NULL;
}

void* CGClipMaskRetain(void)
{
    if (verbose) puts("STUB: CGClipMaskRetain called");
    return NULL;
}

void* CGClipRelease(void)
{
    if (verbose) puts("STUB: CGClipRelease called");
    return NULL;
}

void* CGClipRetain(void)
{
    if (verbose) puts("STUB: CGClipRetain called");
    return NULL;
}

void* CGClipStackAddClip(void)
{
    if (verbose) puts("STUB: CGClipStackAddClip called");
    return NULL;
}

void* CGClipStackAddRect(void)
{
    if (verbose) puts("STUB: CGClipStackAddRect called");
    return NULL;
}

void* CGClipStackContainsMask(void)
{
    if (verbose) puts("STUB: CGClipStackContainsMask called");
    return NULL;
}

void* CGClipStackCreateCopy(void)
{
    if (verbose) puts("STUB: CGClipStackCreateCopy called");
    return NULL;
}

void* CGClipStackCreateMutable(void)
{
    if (verbose) puts("STUB: CGClipStackCreateMutable called");
    return NULL;
}

void* CGClipStackCreateMutableCopy(void)
{
    if (verbose) puts("STUB: CGClipStackCreateMutableCopy called");
    return NULL;
}

void* CGClipStackEqual(void)
{
    if (verbose) puts("STUB: CGClipStackEqual called");
    return NULL;
}

void* CGClipStackGetBounds(void)
{
    if (verbose) puts("STUB: CGClipStackGetBounds called");
    return NULL;
}

void* CGClipStackGetClipAtIndex(void)
{
    if (verbose) puts("STUB: CGClipStackGetClipAtIndex called");
    return NULL;
}

void* CGClipStackGetCount(void)
{
    if (verbose) puts("STUB: CGClipStackGetCount called");
    return NULL;
}

void* CGClipStackGetIdentifier(void)
{
    if (verbose) puts("STUB: CGClipStackGetIdentifier called");
    return NULL;
}

void* CGClipStackGetRect(void)
{
    if (verbose) puts("STUB: CGClipStackGetRect called");
    return NULL;
}

void* CGClipStackIsInfinite(void)
{
    if (verbose) puts("STUB: CGClipStackIsInfinite called");
    return NULL;
}

void* CGClipStackPrint(void)
{
    if (verbose) puts("STUB: CGClipStackPrint called");
    return NULL;
}

void* CGClipStackRelease(void)
{
    if (verbose) puts("STUB: CGClipStackRelease called");
    return NULL;
}

void* CGClipStackRetain(void)
{
    if (verbose) puts("STUB: CGClipStackRetain called");
    return NULL;
}

void* CGClipStrokeCreate(void)
{
    if (verbose) puts("STUB: CGClipStrokeCreate called");
    return NULL;
}

void* CGClipStrokeCreateCopy(void)
{
    if (verbose) puts("STUB: CGClipStrokeCreateCopy called");
    return NULL;
}

void* CGClipStrokeCreateCopyWithTransform(void)
{
    if (verbose) puts("STUB: CGClipStrokeCreateCopyWithTransform called");
    return NULL;
}

void* CGClipStrokeCreateStrokedPath(void)
{
    if (verbose) puts("STUB: CGClipStrokeCreateStrokedPath called");
    return NULL;
}

void* CGClipStrokeCreateWithGState(void)
{
    if (verbose) puts("STUB: CGClipStrokeCreateWithGState called");
    return NULL;
}

void* CGClipStrokeGetDash(void)
{
    if (verbose) puts("STUB: CGClipStrokeGetDash called");
    return NULL;
}

void* CGClipStrokeGetFlatness(void)
{
    if (verbose) puts("STUB: CGClipStrokeGetFlatness called");
    return NULL;
}

void* CGClipStrokeGetLineCap(void)
{
    if (verbose) puts("STUB: CGClipStrokeGetLineCap called");
    return NULL;
}

void* CGClipStrokeGetLineJoin(void)
{
    if (verbose) puts("STUB: CGClipStrokeGetLineJoin called");
    return NULL;
}

void* CGClipStrokeGetLineWidth(void)
{
    if (verbose) puts("STUB: CGClipStrokeGetLineWidth called");
    return NULL;
}

void* CGClipStrokeGetMatrix(void)
{
    if (verbose) puts("STUB: CGClipStrokeGetMatrix called");
    return NULL;
}

void* CGClipStrokeGetMiterLimit(void)
{
    if (verbose) puts("STUB: CGClipStrokeGetMiterLimit called");
    return NULL;
}

void* CGClipStrokeGetShouldAntialias(void)
{
    if (verbose) puts("STUB: CGClipStrokeGetShouldAntialias called");
    return NULL;
}

void* CGClipStrokeGetStrokeAdjust(void)
{
    if (verbose) puts("STUB: CGClipStrokeGetStrokeAdjust called");
    return NULL;
}

void* CGClipStrokeRelease(void)
{
    if (verbose) puts("STUB: CGClipStrokeRelease called");
    return NULL;
}

void* CGClipStrokeRetain(void)
{
    if (verbose) puts("STUB: CGClipStrokeRetain called");
    return NULL;
}

void* CGColorCodeFragmentGetParametricConversion(void)
{
    if (verbose) puts("STUB: CGColorCodeFragmentGetParametricConversion called");
    return NULL;
}

void* CGColorCodeFragmentGetParametricCount(void)
{
    if (verbose) puts("STUB: CGColorCodeFragmentGetParametricCount called");
    return NULL;
}

void* CGColorCodeFragmentGetTextureData(void)
{
    if (verbose) puts("STUB: CGColorCodeFragmentGetTextureData called");
    return NULL;
}

void* CGColorCodeFragmentGetTextureFormat(void)
{
    if (verbose) puts("STUB: CGColorCodeFragmentGetTextureFormat called");
    return NULL;
}

void* CGColorCodeFragmentGetTextureGridPoints(void)
{
    if (verbose) puts("STUB: CGColorCodeFragmentGetTextureGridPoints called");
    return NULL;
}

void* CGColorCodeFragmentRelease(void)
{
    if (verbose) puts("STUB: CGColorCodeFragmentRelease called");
    return NULL;
}

void* CGColorConversionInfoCreate(void)
{
    if (verbose) puts("STUB: CGColorConversionInfoCreate called");
    return NULL;
}

void* CGColorConversionInfoCreateFromList(void)
{
    if (verbose) puts("STUB: CGColorConversionInfoCreateFromList called");
    return NULL;
}

void* CGColorConversionInfoCreateFromListWithArguments(void)
{
    if (verbose) puts("STUB: CGColorConversionInfoCreateFromListWithArguments called");
    return NULL;
}

void* CGColorConversionInfoCreateWithOptions(void)
{
    if (verbose) puts("STUB: CGColorConversionInfoCreateWithOptions called");
    return NULL;
}

void* CGColorConversionInfoGetTypeID(void)
{
    if (verbose) puts("STUB: CGColorConversionInfoGetTypeID called");
    return NULL;
}

void* CGColorConversionInfoIterateColorSpaceFunctionsWithCallbacks(void)
{
    if (verbose) puts("STUB: CGColorConversionInfoIterateColorSpaceFunctionsWithCallbacks called");
    return NULL;
}

void* CGColorConversionInfoIterateColorSpaceFunctionsWithCallbacksAndOptions(void)
{
    if (verbose) puts("STUB: CGColorConversionInfoIterateColorSpaceFunctionsWithCallbacksAndOptions called");
    return NULL;
}

void* CGColorConversionInfoIterateFunctionsWithCallbacks(void)
{
    if (verbose) puts("STUB: CGColorConversionInfoIterateFunctionsWithCallbacks called");
    return NULL;
}

void* CGColorConverterCreate(void)
{
    if (verbose) puts("STUB: CGColorConverterCreate called");
    return NULL;
}

void* CGColorConverterCreateSimple(void)
{
    if (verbose) puts("STUB: CGColorConverterCreateSimple called");
    return NULL;
}

void* CGColorConverterGetTypeID(void)
{
    if (verbose) puts("STUB: CGColorConverterGetTypeID called");
    return NULL;
}

void* CGColorConverterIterateColorSpaceFunctionsWithCallbacks(void)
{
    if (verbose) puts("STUB: CGColorConverterIterateColorSpaceFunctionsWithCallbacks called");
    return NULL;
}

void* CGColorConverterIterateFunctionsWithCallbacks(void)
{
    if (verbose) puts("STUB: CGColorConverterIterateFunctionsWithCallbacks called");
    return NULL;
}

void* CGColorConverterRelease(void)
{
    if (verbose) puts("STUB: CGColorConverterRelease called");
    return NULL;
}

void* CGColorConverterRetain(void)
{
    if (verbose) puts("STUB: CGColorConverterRetain called");
    return NULL;
}

void* CGColorCreate(void)
{
    if (verbose) puts("STUB: CGColorCreate called");
    return NULL;
}

void* CGColorCreateCopy(void)
{
    if (verbose) puts("STUB: CGColorCreateCopy called");
    return NULL;
}

void* CGColorCreateCopyByMatchingToColorSpace(void)
{
    if (verbose) puts("STUB: CGColorCreateCopyByMatchingToColorSpace called");
    return NULL;
}

void* CGColorCreateCopyWithAlpha(void)
{
    if (verbose) puts("STUB: CGColorCreateCopyWithAlpha called");
    return NULL;
}

void* CGColorCreateGenericCMYK(void)
{
    if (verbose) puts("STUB: CGColorCreateGenericCMYK called");
    return NULL;
}

void* CGColorCreateGenericGray(void)
{
    if (verbose) puts("STUB: CGColorCreateGenericGray called");
    return NULL;
}

void* CGColorCreateGenericRGB(void)
{
    if (verbose) puts("STUB: CGColorCreateGenericRGB called");
    return NULL;
}

void* CGColorCreateWithPattern(void)
{
    if (verbose) puts("STUB: CGColorCreateWithPattern called");
    return NULL;
}

void* CGColorCurveEqualToGammaCurve(void)
{
    if (verbose) puts("STUB: CGColorCurveEqualToGammaCurve called");
    return NULL;
}

void* CGColorCurveGetGamma(void)
{
    if (verbose) puts("STUB: CGColorCurveGetGamma called");
    return NULL;
}

void* CGColorCurveGetParameters(void)
{
    if (verbose) puts("STUB: CGColorCurveGetParameters called");
    return NULL;
}

void* CGColorCurveGetTRC(void)
{
    if (verbose) puts("STUB: CGColorCurveGetTRC called");
    return NULL;
}

void* CGColorCurveMapValue(void)
{
    if (verbose) puts("STUB: CGColorCurveMapValue called");
    return NULL;
}

void* CGColorCurvePrint(void)
{
    if (verbose) puts("STUB: CGColorCurvePrint called");
    return NULL;
}

void* CGColorCurveRelease(void)
{
    if (verbose) puts("STUB: CGColorCurveRelease called");
    return NULL;
}

void* CGColorCurveRetain(void)
{
    if (verbose) puts("STUB: CGColorCurveRetain called");
    return NULL;
}

void* CGColorEqualToColor(void)
{
    if (verbose) puts("STUB: CGColorEqualToColor called");
    return NULL;
}

void* CGColorFunctionGetOptions(void)
{
    if (verbose) puts("STUB: CGColorFunctionGetOptions called");
    return NULL;
}

void* CGColorFunctionGetProperties(void)
{
    if (verbose) puts("STUB: CGColorFunctionGetProperties called");
    return NULL;
}

void* CGColorGetAlpha(void)
{
    if (verbose) puts("STUB: CGColorGetAlpha called");
    return NULL;
}

void* CGColorGetColorSpace(void)
{
    if (verbose) puts("STUB: CGColorGetColorSpace called");
    return NULL;
}

void* CGColorGetComponents(void)
{
    if (verbose) puts("STUB: CGColorGetComponents called");
    return NULL;
}

void* CGColorGetConstantColor(void)
{
    if (verbose) puts("STUB: CGColorGetConstantColor called");
    return NULL;
}

void* CGColorGetIdentifier(void)
{
    if (verbose) puts("STUB: CGColorGetIdentifier called");
    return NULL;
}

void* CGColorGetNumberOfComponents(void)
{
    if (verbose) puts("STUB: CGColorGetNumberOfComponents called");
    return NULL;
}

void* CGColorGetPattern(void)
{
    if (verbose) puts("STUB: CGColorGetPattern called");
    return NULL;
}

void* CGColorGetPatternBaseColor(void)
{
    if (verbose) puts("STUB: CGColorGetPatternBaseColor called");
    return NULL;
}

void* CGColorGetTypeID(void)
{
    if (verbose) puts("STUB: CGColorGetTypeID called");
    return NULL;
}

void* CGColorMatrixConcat(void)
{
    if (verbose) puts("STUB: CGColorMatrixConcat called");
    return NULL;
}

void* CGColorMatrixGetMatrix(void)
{
    if (verbose) puts("STUB: CGColorMatrixGetMatrix called");
    return NULL;
}

void* CGColorMatrixGetTypeID(void)
{
    if (verbose) puts("STUB: CGColorMatrixGetTypeID called");
    return NULL;
}

void* CGColorMatrixIsIdentity(void)
{
    if (verbose) puts("STUB: CGColorMatrixIsIdentity called");
    return NULL;
}

void* CGColorMatrixMake(void)
{
    if (verbose) puts("STUB: CGColorMatrixMake called");
    return NULL;
}

void* CGColorMatrixMakeIdentity(void)
{
    if (verbose) puts("STUB: CGColorMatrixMakeIdentity called");
    return NULL;
}

void* CGColorNxMLUTGetLUT(void)
{
    if (verbose) puts("STUB: CGColorNxMLUTGetLUT called");
    return NULL;
}

void* CGColorNxMLUTGetTypeID(void)
{
    if (verbose) puts("STUB: CGColorNxMLUTGetTypeID called");
    return NULL;
}

void* CGColorNxMTransformGetTransform(void)
{
    if (verbose) puts("STUB: CGColorNxMTransformGetTransform called");
    return NULL;
}

void* CGColorNxMTransformGetTypeID(void)
{
    if (verbose) puts("STUB: CGColorNxMTransformGetTypeID called");
    return NULL;
}

void* CGColorRelease(void)
{
    if (verbose) puts("STUB: CGColorRelease called");
    return NULL;
}

void* CGColorRetain(void)
{
    if (verbose) puts("STUB: CGColorRetain called");
    return NULL;
}

void* CGColorSpaceConcatenate(void)
{
    if (verbose) puts("STUB: CGColorSpaceConcatenate called");
    return NULL;
}

void* CGColorSpaceCopyColorTable(void)
{
    if (verbose) puts("STUB: CGColorSpaceCopyColorTable called");
    return NULL;
}

void* CGColorSpaceCopyDefaultColor(void)
{
    if (verbose) puts("STUB: CGColorSpaceCopyDefaultColor called");
    return NULL;
}

void* CGColorSpaceCopyICCData(void)
{
    if (verbose) puts("STUB: CGColorSpaceCopyICCData called");
    return NULL;
}

void* CGColorSpaceCopyICCProfile(void)
{
    if (verbose) puts("STUB: CGColorSpaceCopyICCProfile called");
    return NULL;
}

void* CGColorSpaceCopyICCProfileASCIIDescriptionString(void)
{
    if (verbose) puts("STUB: CGColorSpaceCopyICCProfileASCIIDescriptionString called");
    return NULL;
}

void* CGColorSpaceCopyICCProfileDescription(void)
{
    if (verbose) puts("STUB: CGColorSpaceCopyICCProfileDescription called");
    return NULL;
}

void* CGColorSpaceCopyName(void)
{
    if (verbose) puts("STUB: CGColorSpaceCopyName called");
    return NULL;
}

void* CGColorSpaceCopyPropertyList(void)
{
    if (verbose) puts("STUB: CGColorSpaceCopyPropertyList called");
    return NULL;
}

void* CGColorSpaceCreateCalibratedGray(void)
{
    if (verbose) puts("STUB: CGColorSpaceCreateCalibratedGray called");
    return NULL;
}

void* CGColorSpaceCreateCalibratedRGB(void)
{
    if (verbose) puts("STUB: CGColorSpaceCreateCalibratedRGB called");
    return NULL;
}

void* CGColorSpaceCreateConversionCurve(void)
{
    if (verbose) puts("STUB: CGColorSpaceCreateConversionCurve called");
    return NULL;
}

void* CGColorSpaceCreateDeviceCMYK(void)
{
    if (verbose) puts("STUB: CGColorSpaceCreateDeviceCMYK called");
    return NULL;
}

void* CGColorSpaceCreateDeviceGray(void)
{
    if (verbose) puts("STUB: CGColorSpaceCreateDeviceGray called");
    return NULL;
}

void* CGColorSpaceCreateDeviceN(void)
{
    if (verbose) puts("STUB: CGColorSpaceCreateDeviceN called");
    return NULL;
}

void* CGColorSpaceCreateDeviceRGB(void)
{
    if (verbose) puts("STUB: CGColorSpaceCreateDeviceRGB called");
    return NULL;
}

void* CGColorSpaceCreateDisplayColorSpaceWithID(void)
{
    if (verbose) puts("STUB: CGColorSpaceCreateDisplayColorSpaceWithID called");
    return NULL;
}

void* CGColorSpaceCreateDisplayGray(void)
{
    if (verbose) puts("STUB: CGColorSpaceCreateDisplayGray called");
    return NULL;
}

void* CGColorSpaceCreateDisplayRGB(void)
{
    if (verbose) puts("STUB: CGColorSpaceCreateDisplayRGB called");
    return NULL;
}

void* CGColorSpaceCreateDisplayRGBWithID(void)
{
    if (verbose) puts("STUB: CGColorSpaceCreateDisplayRGBWithID called");
    return NULL;
}

void* CGColorSpaceCreateICCBased(void)
{
    if (verbose) puts("STUB: CGColorSpaceCreateICCBased called");
    return NULL;
}

void* CGColorSpaceCreateIndexed(void)
{
    if (verbose) puts("STUB: CGColorSpaceCreateIndexed called");
    return NULL;
}

void* CGColorSpaceCreateLab(void)
{
    if (verbose) puts("STUB: CGColorSpaceCreateLab called");
    return NULL;
}

void* CGColorSpaceCreateLinearized(void)
{
    if (verbose) puts("STUB: CGColorSpaceCreateLinearized called");
    return NULL;
}

void* CGColorSpaceCreatePattern(void)
{
    if (verbose) puts("STUB: CGColorSpaceCreatePattern called");
    return NULL;
}

void* CGColorSpaceCreatePlatformProfile(void)
{
    if (verbose) puts("STUB: CGColorSpaceCreatePlatformProfile called");
    return NULL;
}

void* CGColorSpaceCreateSystemDefaultCMYK(void)
{
    if (verbose) puts("STUB: CGColorSpaceCreateSystemDefaultCMYK called");
    return NULL;
}

void* CGColorSpaceCreateSystemDefaultGray(void)
{
    if (verbose) puts("STUB: CGColorSpaceCreateSystemDefaultGray called");
    return NULL;
}

void* CGColorSpaceCreateSystemDefaultRGB(void)
{
    if (verbose) puts("STUB: CGColorSpaceCreateSystemDefaultRGB called");
    return NULL;
}

void* CGColorSpaceCreateWithICCData(void)
{
    if (verbose) puts("STUB: CGColorSpaceCreateWithICCData called");
    return NULL;
}

void* CGColorSpaceCreateWithICCProfile(void)
{
    if (verbose) puts("STUB: CGColorSpaceCreateWithICCProfile called");
    return NULL;
}

void* CGColorSpaceCreateWithName(void)
{
    if (verbose) puts("STUB: CGColorSpaceCreateWithName called");
    return NULL;
}

void* CGColorSpaceCreateWithPlatformColorSpace(void)
{
    if (verbose) puts("STUB: CGColorSpaceCreateWithPlatformColorSpace called");
    return NULL;
}

void* CGColorSpaceCreateWithPropertyList(void)
{
    if (verbose) puts("STUB: CGColorSpaceCreateWithPropertyList called");
    return NULL;
}

void* CGColorSpaceEqualToColorSpace(void)
{
    if (verbose) puts("STUB: CGColorSpaceEqualToColorSpace called");
    return NULL;
}

void* CGColorSpaceEqualToColorSpaceIgnoringRange(void)
{
    if (verbose) puts("STUB: CGColorSpaceEqualToColorSpaceIgnoringRange called");
    return NULL;
}

void* CGColorSpaceGetAlternateColorSpace(void)
{
    if (verbose) puts("STUB: CGColorSpaceGetAlternateColorSpace called");
    return NULL;
}

void* CGColorSpaceGetBaseColorSpace(void)
{
    if (verbose) puts("STUB: CGColorSpaceGetBaseColorSpace called");
    return NULL;
}

void* CGColorSpaceGetCalibratedGrayData(void)
{
    if (verbose) puts("STUB: CGColorSpaceGetCalibratedGrayData called");
    return NULL;
}

void* CGColorSpaceGetCalibratedRGBData(void)
{
    if (verbose) puts("STUB: CGColorSpaceGetCalibratedRGBData called");
    return NULL;
}

void* CGColorSpaceGetColorSpaceModel(void)
{
    if (verbose) puts("STUB: CGColorSpaceGetColorSpaceModel called");
    return NULL;
}

void* CGColorSpaceGetColorTable(void)
{
    if (verbose) puts("STUB: CGColorSpaceGetColorTable called");
    return NULL;
}

void* CGColorSpaceGetColorTableCount(void)
{
    if (verbose) puts("STUB: CGColorSpaceGetColorTableCount called");
    return NULL;
}

void* CGColorSpaceGetColorants(void)
{
    if (verbose) puts("STUB: CGColorSpaceGetColorants called");
    return NULL;
}

void* CGColorSpaceGetConversionMatrix(void)
{
    if (verbose) puts("STUB: CGColorSpaceGetConversionMatrix called");
    return NULL;
}

void* CGColorSpaceGetDescriptor(void)
{
    if (verbose) puts("STUB: CGColorSpaceGetDescriptor called");
    return NULL;
}

void* CGColorSpaceGetIdentifier(void)
{
    if (verbose) puts("STUB: CGColorSpaceGetIdentifier called");
    return NULL;
}

void* CGColorSpaceGetLabData(void)
{
    if (verbose) puts("STUB: CGColorSpaceGetLabData called");
    return NULL;
}

void* CGColorSpaceGetMD5Digest(void)
{
    if (verbose) puts("STUB: CGColorSpaceGetMD5Digest called");
    return NULL;
}

void* CGColorSpaceGetModel(void)
{
    if (verbose) puts("STUB: CGColorSpaceGetModel called");
    return NULL;
}

void* CGColorSpaceGetName(void)
{
    if (verbose) puts("STUB: CGColorSpaceGetName called");
    return NULL;
}

void* CGColorSpaceGetNames(void)
{
    if (verbose) puts("STUB: CGColorSpaceGetNames called");
    return NULL;
}

void* CGColorSpaceGetNumberOfComponents(void)
{
    if (verbose) puts("STUB: CGColorSpaceGetNumberOfComponents called");
    return NULL;
}

void* CGColorSpaceGetProcessColorModel(void)
{
    if (verbose) puts("STUB: CGColorSpaceGetProcessColorModel called");
    return NULL;
}

void* CGColorSpaceGetRenderingIntent(void)
{
    if (verbose) puts("STUB: CGColorSpaceGetRenderingIntent called");
    return NULL;
}

void* CGColorSpaceGetTintTransform(void)
{
    if (verbose) puts("STUB: CGColorSpaceGetTintTransform called");
    return NULL;
}

void* CGColorSpaceGetType(void)
{
    if (verbose) puts("STUB: CGColorSpaceGetType called");
    return NULL;
}

void* CGColorSpaceGetTypeID(void)
{
    if (verbose) puts("STUB: CGColorSpaceGetTypeID called");
    return NULL;
}

void* CGColorSpaceICCGetPlatformProfile(void)
{
    if (verbose) puts("STUB: CGColorSpaceICCGetPlatformProfile called");
    return NULL;
}

void* CGColorSpaceICCGetRange(void)
{
    if (verbose) puts("STUB: CGColorSpaceICCGetRange called");
    return NULL;
}

void* CGColorSpaceIgnoresIntent(void)
{
    if (verbose) puts("STUB: CGColorSpaceIgnoresIntent called");
    return NULL;
}

void* CGColorSpaceIsICCCompatible(void)
{
    if (verbose) puts("STUB: CGColorSpaceIsICCCompatible called");
    return NULL;
}

void* CGColorSpaceIsPSLevel2Compatible(void)
{
    if (verbose) puts("STUB: CGColorSpaceIsPSLevel2Compatible called");
    return NULL;
}

void* CGColorSpaceIsUncalibrated(void)
{
    if (verbose) puts("STUB: CGColorSpaceIsUncalibrated called");
    return NULL;
}

void* CGColorSpaceIsWideGamutRGB(void)
{
    if (verbose) puts("STUB: CGColorSpaceIsWideGamutRGB called");
    return NULL;
}

void* CGColorSpaceRelease(void)
{
    if (verbose) puts("STUB: CGColorSpaceRelease called");
    return NULL;
}

void* CGColorSpaceRetain(void)
{
    if (verbose) puts("STUB: CGColorSpaceRetain called");
    return NULL;
}

void* CGColorSpaceStreamPSDefinition(void)
{
    if (verbose) puts("STUB: CGColorSpaceStreamPSDefinition called");
    return NULL;
}

void* CGColorSpaceSupportsOutput(void)
{
    if (verbose) puts("STUB: CGColorSpaceSupportsOutput called");
    return NULL;
}

void* CGColorSpaceUsesExtendedRange(void)
{
    if (verbose) puts("STUB: CGColorSpaceUsesExtendedRange called");
    return NULL;
}

void* CGColorTRCGetFunction(void)
{
    if (verbose) puts("STUB: CGColorTRCGetFunction called");
    return NULL;
}

void* CGColorTRCGetGammaID(void)
{
    if (verbose) puts("STUB: CGColorTRCGetGammaID called");
    return NULL;
}

void* CGColorTRCGetTypeID(void)
{
    if (verbose) puts("STUB: CGColorTRCGetTypeID called");
    return NULL;
}

void* CGColorTRCPureGammaOriginal(void)
{
    if (verbose) puts("STUB: CGColorTRCPureGammaOriginal called");
    return NULL;
}

void* CGColorTransformConvertColor(void)
{
    if (verbose) puts("STUB: CGColorTransformConvertColor called");
    return NULL;
}

void* CGColorTransformConvertColorComponents(void)
{
    if (verbose) puts("STUB: CGColorTransformConvertColorComponents called");
    return NULL;
}

void* CGColorTransformConvertColorFloatComponents(void)
{
    if (verbose) puts("STUB: CGColorTransformConvertColorFloatComponents called");
    return NULL;
}

void* CGColorTransformConvertComponentData(void)
{
    if (verbose) puts("STUB: CGColorTransformConvertComponentData called");
    return NULL;
}

void* CGColorTransformConvertData(void)
{
    if (verbose) puts("STUB: CGColorTransformConvertData called");
    return NULL;
}

void* CGColorTransformConvertImage(void)
{
    if (verbose) puts("STUB: CGColorTransformConvertImage called");
    return NULL;
}

void* CGColorTransformConvertNeedsCMS(void)
{
    if (verbose) puts("STUB: CGColorTransformConvertNeedsCMS called");
    return NULL;
}

void* CGColorTransformConvertRequired(void)
{
    if (verbose) puts("STUB: CGColorTransformConvertRequired called");
    return NULL;
}

void* CGColorTransformCreate(void)
{
    if (verbose) puts("STUB: CGColorTransformCreate called");
    return NULL;
}

void* CGColorTransformCreateCodeFragment(void)
{
    if (verbose) puts("STUB: CGColorTransformCreateCodeFragment called");
    return NULL;
}

void* CGColorTransformCreateLookupTextureCodeFragment(void)
{
    if (verbose) puts("STUB: CGColorTransformCreateLookupTextureCodeFragment called");
    return NULL;
}

void* CGColorTransformCreateParametricCodeFragment(void)
{
    if (verbose) puts("STUB: CGColorTransformCreateParametricCodeFragment called");
    return NULL;
}

void* CGColorTransformGetColorSpace(void)
{
    if (verbose) puts("STUB: CGColorTransformGetColorSpace called");
    return NULL;
}

void* CGColorTransformGetIdentifier(void)
{
    if (verbose) puts("STUB: CGColorTransformGetIdentifier called");
    return NULL;
}

void* CGColorTransformGetMD5(void)
{
    if (verbose) puts("STUB: CGColorTransformGetMD5 called");
    return NULL;
}

void* CGColorTransformGetTypeID(void)
{
    if (verbose) puts("STUB: CGColorTransformGetTypeID called");
    return NULL;
}

void* CGColorTransformRelease(void)
{
    if (verbose) puts("STUB: CGColorTransformRelease called");
    return NULL;
}

void* CGColorTransformRetain(void)
{
    if (verbose) puts("STUB: CGColorTransformRetain called");
    return NULL;
}

void* CGContainerGetRect(void)
{
    if (verbose) puts("STUB: CGContainerGetRect called");
    return NULL;
}

void* CGContainerGetTypeID(void)
{
    if (verbose) puts("STUB: CGContainerGetTypeID called");
    return NULL;
}

void* CGContextAddArc(void)
{
    if (verbose) puts("STUB: CGContextAddArc called");
    return NULL;
}

void* CGContextAddArcToPoint(void)
{
    if (verbose) puts("STUB: CGContextAddArcToPoint called");
    return NULL;
}

void* CGContextAddCurveToPoint(void)
{
    if (verbose) puts("STUB: CGContextAddCurveToPoint called");
    return NULL;
}

void* CGContextAddEllipseInRect(void)
{
    if (verbose) puts("STUB: CGContextAddEllipseInRect called");
    return NULL;
}

void* CGContextAddFilter(void)
{
    if (verbose) puts("STUB: CGContextAddFilter called");
    return NULL;
}

void* CGContextAddLineToPoint(void)
{
    if (verbose) puts("STUB: CGContextAddLineToPoint called");
    return NULL;
}

void* CGContextAddLines(void)
{
    if (verbose) puts("STUB: CGContextAddLines called");
    return NULL;
}

void* CGContextAddPath(void)
{
    if (verbose) puts("STUB: CGContextAddPath called");
    return NULL;
}

void* CGContextAddQuadCurveToPoint(void)
{
    if (verbose) puts("STUB: CGContextAddQuadCurveToPoint called");
    return NULL;
}

void* CGContextAddRect(void)
{
    if (verbose) puts("STUB: CGContextAddRect called");
    return NULL;
}

void* CGContextAddRects(void)
{
    if (verbose) puts("STUB: CGContextAddRects called");
    return NULL;
}

void* CGContextBeginPage(void)
{
    if (verbose) puts("STUB: CGContextBeginPage called");
    return NULL;
}

void* CGContextBeginPage2(void)
{
    if (verbose) puts("STUB: CGContextBeginPage2 called");
    return NULL;
}

void* CGContextBeginPath(void)
{
    if (verbose) puts("STUB: CGContextBeginPath called");
    return NULL;
}

void* CGContextBeginTransparencyLayer(void)
{
    if (verbose) puts("STUB: CGContextBeginTransparencyLayer called");
    return NULL;
}

void* CGContextBeginTransparencyLayerWithRect(void)
{
    if (verbose) puts("STUB: CGContextBeginTransparencyLayerWithRect called");
    return NULL;
}

void* CGContextClear(void)
{
    if (verbose) puts("STUB: CGContextClear called");
    return NULL;
}

void* CGContextClearRect(void)
{
    if (verbose) puts("STUB: CGContextClearRect called");
    return NULL;
}

void* CGContextClip(void)
{
    if (verbose) puts("STUB: CGContextClip called");
    return NULL;
}

void* CGContextClipToMask(void)
{
    if (verbose) puts("STUB: CGContextClipToMask called");
    return NULL;
}

void* CGContextClipToRect(void)
{
    if (verbose) puts("STUB: CGContextClipToRect called");
    return NULL;
}

void* CGContextClipToRects(void)
{
    if (verbose) puts("STUB: CGContextClipToRects called");
    return NULL;
}

void* CGContextClipToTextClipping(void)
{
    if (verbose) puts("STUB: CGContextClipToTextClipping called");
    return NULL;
}

void* CGContextClosePath(void)
{
    if (verbose) puts("STUB: CGContextClosePath called");
    return NULL;
}

void* CGContextColorIsExtendedRGB(void)
{
    if (verbose) puts("STUB: CGContextColorIsExtendedRGB called");
    return NULL;
}

void* CGContextConcatCTM(void)
{
    if (verbose) puts("STUB: CGContextConcatCTM called");
    return NULL;
}

void* CGContextConvertPointToDeviceSpace(void)
{
    if (verbose) puts("STUB: CGContextConvertPointToDeviceSpace called");
    return NULL;
}

void* CGContextConvertPointToUserSpace(void)
{
    if (verbose) puts("STUB: CGContextConvertPointToUserSpace called");
    return NULL;
}

void* CGContextConvertRectToDeviceSpace(void)
{
    if (verbose) puts("STUB: CGContextConvertRectToDeviceSpace called");
    return NULL;
}

void* CGContextConvertRectToUserSpace(void)
{
    if (verbose) puts("STUB: CGContextConvertRectToUserSpace called");
    return NULL;
}

void* CGContextConvertSizeToDeviceSpace(void)
{
    if (verbose) puts("STUB: CGContextConvertSizeToDeviceSpace called");
    return NULL;
}

void* CGContextConvertSizeToUserSpace(void)
{
    if (verbose) puts("STUB: CGContextConvertSizeToUserSpace called");
    return NULL;
}

void* CGContextCopyDeviceColorSpace(void)
{
    if (verbose) puts("STUB: CGContextCopyDeviceColorSpace called");
    return NULL;
}

void* CGContextCopyPath(void)
{
    if (verbose) puts("STUB: CGContextCopyPath called");
    return NULL;
}

void* CGContextCopyTopGState(void)
{
    if (verbose) puts("STUB: CGContextCopyTopGState called");
    return NULL;
}

void* CGContextCreate(void)
{
    if (verbose) puts("STUB: CGContextCreate called");
    return NULL;
}

void* CGContextCreateWithDelegate(void)
{
    if (verbose) puts("STUB: CGContextCreateWithDelegate called");
    return NULL;
}

void* CGContextCreateWithDelegateAndInfo(void)
{
    if (verbose) puts("STUB: CGContextCreateWithDelegateAndInfo called");
    return NULL;
}

void* CGContextDelegateBeginLayer(void)
{
    if (verbose) puts("STUB: CGContextDelegateBeginLayer called");
    return NULL;
}

void* CGContextDelegateBeginPage(void)
{
    if (verbose) puts("STUB: CGContextDelegateBeginPage called");
    return NULL;
}

void* CGContextDelegateCreate(void)
{
    if (verbose) puts("STUB: CGContextDelegateCreate called");
    return NULL;
}

void* CGContextDelegateCreateFilter(void)
{
    if (verbose) puts("STUB: CGContextDelegateCreateFilter called");
    return NULL;
}

void* CGContextDelegateCreateWithCallbacks(void)
{
    if (verbose) puts("STUB: CGContextDelegateCreateWithCallbacks called");
    return NULL;
}

void* CGContextDelegateDrawConicGradient(void)
{
    if (verbose) puts("STUB: CGContextDelegateDrawConicGradient called");
    return NULL;
}

void* CGContextDelegateDrawDisplayList(void)
{
    if (verbose) puts("STUB: CGContextDelegateDrawDisplayList called");
    return NULL;
}

void* CGContextDelegateDrawGlyphs(void)
{
    if (verbose) puts("STUB: CGContextDelegateDrawGlyphs called");
    return NULL;
}

void* CGContextDelegateDrawImage(void)
{
    if (verbose) puts("STUB: CGContextDelegateDrawImage called");
    return NULL;
}

void* CGContextDelegateDrawImageFromRect(void)
{
    if (verbose) puts("STUB: CGContextDelegateDrawImageFromRect called");
    return NULL;
}

void* CGContextDelegateDrawImages(void)
{
    if (verbose) puts("STUB: CGContextDelegateDrawImages called");
    return NULL;
}

void* CGContextDelegateDrawLayer(void)
{
    if (verbose) puts("STUB: CGContextDelegateDrawLayer called");
    return NULL;
}

void* CGContextDelegateDrawLinearGradient(void)
{
    if (verbose) puts("STUB: CGContextDelegateDrawLinearGradient called");
    return NULL;
}

void* CGContextDelegateDrawLines(void)
{
    if (verbose) puts("STUB: CGContextDelegateDrawLines called");
    return NULL;
}

void* CGContextDelegateDrawPath(void)
{
    if (verbose) puts("STUB: CGContextDelegateDrawPath called");
    return NULL;
}

void* CGContextDelegateDrawPathDirect(void)
{
    if (verbose) puts("STUB: CGContextDelegateDrawPathDirect called");
    return NULL;
}

void* CGContextDelegateDrawRadialGradient(void)
{
    if (verbose) puts("STUB: CGContextDelegateDrawRadialGradient called");
    return NULL;
}

void* CGContextDelegateDrawRects(void)
{
    if (verbose) puts("STUB: CGContextDelegateDrawRects called");
    return NULL;
}

void* CGContextDelegateDrawShading(void)
{
    if (verbose) puts("STUB: CGContextDelegateDrawShading called");
    return NULL;
}

void* CGContextDelegateDrawWindowContents(void)
{
    if (verbose) puts("STUB: CGContextDelegateDrawWindowContents called");
    return NULL;
}

void* CGContextDelegateEndLayer(void)
{
    if (verbose) puts("STUB: CGContextDelegateEndLayer called");
    return NULL;
}

void* CGContextDelegateEndPage(void)
{
    if (verbose) puts("STUB: CGContextDelegateEndPage called");
    return NULL;
}

void* CGContextDelegateFinalize(void)
{
    if (verbose) puts("STUB: CGContextDelegateFinalize called");
    return NULL;
}

void* CGContextDelegateGetBitmapContextInfo(void)
{
    if (verbose) puts("STUB: CGContextDelegateGetBitmapContextInfo called");
    return NULL;
}

void* CGContextDelegateGetBounds(void)
{
    if (verbose) puts("STUB: CGContextDelegateGetBounds called");
    return NULL;
}

void* CGContextDelegateGetCallback(void)
{
    if (verbose) puts("STUB: CGContextDelegateGetCallback called");
    return NULL;
}

void* CGContextDelegateGetColorTransform(void)
{
    if (verbose) puts("STUB: CGContextDelegateGetColorTransform called");
    return NULL;
}

void* CGContextDelegateGetDelegate(void)
{
    if (verbose) puts("STUB: CGContextDelegateGetDelegate called");
    return NULL;
}

void* CGContextDelegateGetInfo(void)
{
    if (verbose) puts("STUB: CGContextDelegateGetInfo called");
    return NULL;
}

void* CGContextDelegateGetLayer(void)
{
    if (verbose) puts("STUB: CGContextDelegateGetLayer called");
    return NULL;
}

void* CGContextDelegateGetNext(void)
{
    if (verbose) puts("STUB: CGContextDelegateGetNext called");
    return NULL;
}

void* CGContextDelegateGetTransform(void)
{
    if (verbose) puts("STUB: CGContextDelegateGetTransform called");
    return NULL;
}

void* CGContextDelegateGetTypeID(void)
{
    if (verbose) puts("STUB: CGContextDelegateGetTypeID called");
    return NULL;
}

void* CGContextDelegateImplementsCallback(void)
{
    if (verbose) puts("STUB: CGContextDelegateImplementsCallback called");
    return NULL;
}

void* CGContextDelegateOperation(void)
{
    if (verbose) puts("STUB: CGContextDelegateOperation called");
    return NULL;
}

void* CGContextDelegateRelease(void)
{
    if (verbose) puts("STUB: CGContextDelegateRelease called");
    return NULL;
}

void* CGContextDelegateRetain(void)
{
    if (verbose) puts("STUB: CGContextDelegateRetain called");
    return NULL;
}

void* CGContextDelegateSetCallback(void)
{
    if (verbose) puts("STUB: CGContextDelegateSetCallback called");
    return NULL;
}

void* CGContextDelegateSetCallbacks(void)
{
    if (verbose) puts("STUB: CGContextDelegateSetCallbacks called");
    return NULL;
}

void* CGContextDelegateSetNext(void)
{
    if (verbose) puts("STUB: CGContextDelegateSetNext called");
    return NULL;
}

void* CGContextDrawConicGradient(void)
{
    if (verbose) puts("STUB: CGContextDrawConicGradient called");
    return NULL;
}

void* CGContextDrawDisplayList(void)
{
    if (verbose) puts("STUB: CGContextDrawDisplayList called");
    return NULL;
}

void* CGContextDrawImage(void)
{
    if (verbose) puts("STUB: CGContextDrawImage called");
    return NULL;
}

void* CGContextDrawImageFromRect(void)
{
    if (verbose) puts("STUB: CGContextDrawImageFromRect called");
    return NULL;
}

void* CGContextDrawImageWithOptions(void)
{
    if (verbose) puts("STUB: CGContextDrawImageWithOptions called");
    return NULL;
}

void* CGContextDrawImages(void)
{
    if (verbose) puts("STUB: CGContextDrawImages called");
    return NULL;
}

void* CGContextDrawLayerAtPoint(void)
{
    if (verbose) puts("STUB: CGContextDrawLayerAtPoint called");
    return NULL;
}

void* CGContextDrawLayerInRect(void)
{
    if (verbose) puts("STUB: CGContextDrawLayerInRect called");
    return NULL;
}

void* CGContextDrawLinearGradient(void)
{
    if (verbose) puts("STUB: CGContextDrawLinearGradient called");
    return NULL;
}

void* CGContextDrawPDFAnnotation(void)
{
    if (verbose) puts("STUB: CGContextDrawPDFAnnotation called");
    return NULL;
}

void* CGContextDrawPDFDocument(void)
{
    if (verbose) puts("STUB: CGContextDrawPDFDocument called");
    return NULL;
}

void* CGContextDrawPDFPage(void)
{
    if (verbose) puts("STUB: CGContextDrawPDFPage called");
    return NULL;
}

void* CGContextDrawPDFPageInRect(void)
{
    if (verbose) puts("STUB: CGContextDrawPDFPageInRect called");
    return NULL;
}

void* CGContextDrawPDFPageInRect2(void)
{
    if (verbose) puts("STUB: CGContextDrawPDFPageInRect2 called");
    return NULL;
}

void* CGContextDrawPDFPageWithAnnotations(void)
{
    if (verbose) puts("STUB: CGContextDrawPDFPageWithAnnotations called");
    return NULL;
}

void* CGContextDrawPDFPageWithDrawingCallbacks(void)
{
    if (verbose) puts("STUB: CGContextDrawPDFPageWithDrawingCallbacks called");
    return NULL;
}

void* CGContextDrawPDFPageWithProgressCallback(void)
{
    if (verbose) puts("STUB: CGContextDrawPDFPageWithProgressCallback called");
    return NULL;
}

void* CGContextDrawPath(void)
{
    if (verbose) puts("STUB: CGContextDrawPath called");
    return NULL;
}

void* CGContextDrawPathDirect(void)
{
    if (verbose) puts("STUB: CGContextDrawPathDirect called");
    return NULL;
}

void* CGContextDrawPatternCell(void)
{
    if (verbose) puts("STUB: CGContextDrawPatternCell called");
    return NULL;
}

void* CGContextDrawRadialGradient(void)
{
    if (verbose) puts("STUB: CGContextDrawRadialGradient called");
    return NULL;
}

void* CGContextDrawShading(void)
{
    if (verbose) puts("STUB: CGContextDrawShading called");
    return NULL;
}

void* CGContextDrawTiledImage(void)
{
    if (verbose) puts("STUB: CGContextDrawTiledImage called");
    return NULL;
}

void* CGContextDrawsWithCorrectShadowOffsets(void)
{
    if (verbose) puts("STUB: CGContextDrawsWithCorrectShadowOffsets called");
    return NULL;
}

void* CGContextEOClip(void)
{
    if (verbose) puts("STUB: CGContextEOClip called");
    return NULL;
}

void* CGContextEOFillPath(void)
{
    if (verbose) puts("STUB: CGContextEOFillPath called");
    return NULL;
}

void* CGContextEndPage(void)
{
    if (verbose) puts("STUB: CGContextEndPage called");
    return NULL;
}

void* CGContextEndPage2(void)
{
    if (verbose) puts("STUB: CGContextEndPage2 called");
    return NULL;
}

void* CGContextEndTransparencyLayer(void)
{
    if (verbose) puts("STUB: CGContextEndTransparencyLayer called");
    return NULL;
}

void* CGContextErase(void)
{
    if (verbose) puts("STUB: CGContextErase called");
    return NULL;
}

void* CGContextFillEllipseInRect(void)
{
    if (verbose) puts("STUB: CGContextFillEllipseInRect called");
    return NULL;
}

void* CGContextFillPath(void)
{
    if (verbose) puts("STUB: CGContextFillPath called");
    return NULL;
}

void* CGContextFillRect(void)
{
    if (verbose) puts("STUB: CGContextFillRect called");
    return NULL;
}

void* CGContextFillRects(void)
{
    if (verbose) puts("STUB: CGContextFillRects called");
    return NULL;
}

void* CGContextFilterCreateDictionaryWithURL(void)
{
    if (verbose) puts("STUB: CGContextFilterCreateDictionaryWithURL called");
    return NULL;
}

void* CGContextFlush(void)
{
    if (verbose) puts("STUB: CGContextFlush called");
    return NULL;
}

void* CGContextGetAdjustedLineWidth(void)
{
    if (verbose) puts("STUB: CGContextGetAdjustedLineWidth called");
    return NULL;
}

void* CGContextGetAllowedFontRenderingStyle(void)
{
    if (verbose) puts("STUB: CGContextGetAllowedFontRenderingStyle called");
    return NULL;
}

void* CGContextGetAllowsAntialiasing(void)
{
    if (verbose) puts("STUB: CGContextGetAllowsAntialiasing called");
    return NULL;
}

void* CGContextGetAllowsDithering(void)
{
    if (verbose) puts("STUB: CGContextGetAllowsDithering called");
    return NULL;
}

void* CGContextGetAllowsFontAntialiasing(void)
{
    if (verbose) puts("STUB: CGContextGetAllowsFontAntialiasing called");
    return NULL;
}

void* CGContextGetAllowsFontSmoothing(void)
{
    if (verbose) puts("STUB: CGContextGetAllowsFontSmoothing called");
    return NULL;
}

void* CGContextGetAllowsFontSubpixelPositioning(void)
{
    if (verbose) puts("STUB: CGContextGetAllowsFontSubpixelPositioning called");
    return NULL;
}

void* CGContextGetAllowsFontSubpixelQuantization(void)
{
    if (verbose) puts("STUB: CGContextGetAllowsFontSubpixelQuantization called");
    return NULL;
}

void* CGContextGetAlpha(void)
{
    if (verbose) puts("STUB: CGContextGetAlpha called");
    return NULL;
}

void* CGContextGetAlphaIsShape(void)
{
    if (verbose) puts("STUB: CGContextGetAlphaIsShape called");
    return NULL;
}

void* CGContextGetBaseCTM(void)
{
    if (verbose) puts("STUB: CGContextGetBaseCTM called");
    return NULL;
}

void* CGContextGetBlackGeneration(void)
{
    if (verbose) puts("STUB: CGContextGetBlackGeneration called");
    return NULL;
}

void* CGContextGetBlendMode(void)
{
    if (verbose) puts("STUB: CGContextGetBlendMode called");
    return NULL;
}

void* CGContextGetCTM(void)
{
    if (verbose) puts("STUB: CGContextGetCTM called");
    return NULL;
}

void* CGContextGetCharacterSpacing(void)
{
    if (verbose) puts("STUB: CGContextGetCharacterSpacing called");
    return NULL;
}

void* CGContextGetClipBoundingBox(void)
{
    if (verbose) puts("STUB: CGContextGetClipBoundingBox called");
    return NULL;
}

void* CGContextGetColorTransform(void)
{
    if (verbose) puts("STUB: CGContextGetColorTransform called");
    return NULL;
}

void* CGContextGetCompositeOperation(void)
{
    if (verbose) puts("STUB: CGContextGetCompositeOperation called");
    return NULL;
}

void* CGContextGetContentMD5(void)
{
    if (verbose) puts("STUB: CGContextGetContentMD5 called");
    return NULL;
}

void* CGContextGetDefaultUserSpaceToDeviceSpaceTransform(void)
{
    if (verbose) puts("STUB: CGContextGetDefaultUserSpaceToDeviceSpaceTransform called");
    return NULL;
}

void* CGContextGetDelegate(void)
{
    if (verbose) puts("STUB: CGContextGetDelegate called");
    return NULL;
}

void* CGContextGetDelegateInfo(void)
{
    if (verbose) puts("STUB: CGContextGetDelegateInfo called");
    return NULL;
}

void* CGContextGetDeviceClipBoundingBox(void)
{
    if (verbose) puts("STUB: CGContextGetDeviceClipBoundingBox called");
    return NULL;
}

void* CGContextGetFillColor(void)
{
    if (verbose) puts("STUB: CGContextGetFillColor called");
    return NULL;
}

void* CGContextGetFillColorAsColor(void)
{
    if (verbose) puts("STUB: CGContextGetFillColorAsColor called");
    return NULL;
}

void* CGContextGetFillColorSpace(void)
{
    if (verbose) puts("STUB: CGContextGetFillColorSpace called");
    return NULL;
}

void* CGContextGetFillOverprint(void)
{
    if (verbose) puts("STUB: CGContextGetFillOverprint called");
    return NULL;
}

void* CGContextGetFillPattern(void)
{
    if (verbose) puts("STUB: CGContextGetFillPattern called");
    return NULL;
}

void* CGContextGetFlatness(void)
{
    if (verbose) puts("STUB: CGContextGetFlatness called");
    return NULL;
}

void* CGContextGetFont(void)
{
    if (verbose) puts("STUB: CGContextGetFont called");
    return NULL;
}

void* CGContextGetFontAntialiasingStyle(void)
{
    if (verbose) puts("STUB: CGContextGetFontAntialiasingStyle called");
    return NULL;
}

void* CGContextGetFontDilation(void)
{
    if (verbose) puts("STUB: CGContextGetFontDilation called");
    return NULL;
}

void* CGContextGetFontRenderingStyle(void)
{
    if (verbose) puts("STUB: CGContextGetFontRenderingStyle called");
    return NULL;
}

void* CGContextGetFontSize(void)
{
    if (verbose) puts("STUB: CGContextGetFontSize called");
    return NULL;
}

void* CGContextGetFontSmoothingBackgroundColor(void)
{
    if (verbose) puts("STUB: CGContextGetFontSmoothingBackgroundColor called");
    return NULL;
}

void* CGContextGetFontSmoothingContrast(void)
{
    if (verbose) puts("STUB: CGContextGetFontSmoothingContrast called");
    return NULL;
}

void* CGContextGetFontSmoothingStyle(void)
{
    if (verbose) puts("STUB: CGContextGetFontSmoothingStyle called");
    return NULL;
}

void* CGContextGetFontSubpixelQuantizationLevel(void)
{
    if (verbose) puts("STUB: CGContextGetFontSubpixelQuantizationLevel called");
    return NULL;
}

void* CGContextGetGState(void)
{
    if (verbose) puts("STUB: CGContextGetGState called");
    return NULL;
}

void* CGContextGetGStateMark(void)
{
    if (verbose) puts("STUB: CGContextGetGStateMark called");
    return NULL;
}

void* CGContextGetGlyphAdvances(void)
{
    if (verbose) puts("STUB: CGContextGetGlyphAdvances called");
    return NULL;
}

void* CGContextGetIdentifier(void)
{
    if (verbose) puts("STUB: CGContextGetIdentifier called");
    return NULL;
}

void* CGContextGetImageRenderingIntent(void)
{
    if (verbose) puts("STUB: CGContextGetImageRenderingIntent called");
    return NULL;
}

void* CGContextGetInterpolationQuality(void)
{
    if (verbose) puts("STUB: CGContextGetInterpolationQuality called");
    return NULL;
}

void* CGContextGetInterpolationQualityRange(void)
{
    if (verbose) puts("STUB: CGContextGetInterpolationQualityRange called");
    return NULL;
}

void* CGContextGetLineCap(void)
{
    if (verbose) puts("STUB: CGContextGetLineCap called");
    return NULL;
}

void* CGContextGetLineDashCount(void)
{
    if (verbose) puts("STUB: CGContextGetLineDashCount called");
    return NULL;
}

void* CGContextGetLineDashPattern(void)
{
    if (verbose) puts("STUB: CGContextGetLineDashPattern called");
    return NULL;
}

void* CGContextGetLineJoin(void)
{
    if (verbose) puts("STUB: CGContextGetLineJoin called");
    return NULL;
}

void* CGContextGetLineWidth(void)
{
    if (verbose) puts("STUB: CGContextGetLineWidth called");
    return NULL;
}

void* CGContextGetLineWidthThreshold(void)
{
    if (verbose) puts("STUB: CGContextGetLineWidthThreshold called");
    return NULL;
}

void* CGContextGetMiterLimit(void)
{
    if (verbose) puts("STUB: CGContextGetMiterLimit called");
    return NULL;
}

void* CGContextGetOverprintMode(void)
{
    if (verbose) puts("STUB: CGContextGetOverprintMode called");
    return NULL;
}

void* CGContextGetPathBoundingBox(void)
{
    if (verbose) puts("STUB: CGContextGetPathBoundingBox called");
    return NULL;
}

void* CGContextGetPathCurrentPoint(void)
{
    if (verbose) puts("STUB: CGContextGetPathCurrentPoint called");
    return NULL;
}

void* CGContextGetPatternPhase(void)
{
    if (verbose) puts("STUB: CGContextGetPatternPhase called");
    return NULL;
}

void* CGContextGetPixelAccess(void)
{
    if (verbose) puts("STUB: CGContextGetPixelAccess called");
    return NULL;
}

void* CGContextGetProperty(void)
{
    if (verbose) puts("STUB: CGContextGetProperty called");
    return NULL;
}

void* CGContextGetRenderingIntent(void)
{
    if (verbose) puts("STUB: CGContextGetRenderingIntent called");
    return NULL;
}

void* CGContextGetRenderingResolution(void)
{
    if (verbose) puts("STUB: CGContextGetRenderingResolution called");
    return NULL;
}

void* CGContextGetRenderingState(void)
{
    if (verbose) puts("STUB: CGContextGetRenderingState called");
    return NULL;
}

void* CGContextGetShouldAntialias(void)
{
    if (verbose) puts("STUB: CGContextGetShouldAntialias called");
    return NULL;
}

void* CGContextGetShouldAntialiasFonts(void)
{
    if (verbose) puts("STUB: CGContextGetShouldAntialiasFonts called");
    return NULL;
}

void* CGContextGetShouldDrawBitmapRuns(void)
{
    if (verbose) puts("STUB: CGContextGetShouldDrawBitmapRuns called");
    return NULL;
}

void* CGContextGetShouldSmoothFonts(void)
{
    if (verbose) puts("STUB: CGContextGetShouldSmoothFonts called");
    return NULL;
}

void* CGContextGetShouldSubpixelPositionFonts(void)
{
    if (verbose) puts("STUB: CGContextGetShouldSubpixelPositionFonts called");
    return NULL;
}

void* CGContextGetShouldSubpixelQuantizeFonts(void)
{
    if (verbose) puts("STUB: CGContextGetShouldSubpixelQuantizeFonts called");
    return NULL;
}

void* CGContextGetShouldUsePlatformNativeGlyphs(void)
{
    if (verbose) puts("STUB: CGContextGetShouldUsePlatformNativeGlyphs called");
    return NULL;
}

void* CGContextGetSmoothness(void)
{
    if (verbose) puts("STUB: CGContextGetSmoothness called");
    return NULL;
}

void* CGContextGetSoftMask(void)
{
    if (verbose) puts("STUB: CGContextGetSoftMask called");
    return NULL;
}

void* CGContextGetStrokeAdjust(void)
{
    if (verbose) puts("STUB: CGContextGetStrokeAdjust called");
    return NULL;
}

void* CGContextGetStrokeColor(void)
{
    if (verbose) puts("STUB: CGContextGetStrokeColor called");
    return NULL;
}

void* CGContextGetStrokeColorAsColor(void)
{
    if (verbose) puts("STUB: CGContextGetStrokeColorAsColor called");
    return NULL;
}

void* CGContextGetStrokeColorSpace(void)
{
    if (verbose) puts("STUB: CGContextGetStrokeColorSpace called");
    return NULL;
}

void* CGContextGetStrokeOverprint(void)
{
    if (verbose) puts("STUB: CGContextGetStrokeOverprint called");
    return NULL;
}

void* CGContextGetStrokePattern(void)
{
    if (verbose) puts("STUB: CGContextGetStrokePattern called");
    return NULL;
}

void* CGContextGetStyle(void)
{
    if (verbose) puts("STUB: CGContextGetStyle called");
    return NULL;
}

void* CGContextGetTextDrawingMode(void)
{
    if (verbose) puts("STUB: CGContextGetTextDrawingMode called");
    return NULL;
}

void* CGContextGetTextGreekingThreshold(void)
{
    if (verbose) puts("STUB: CGContextGetTextGreekingThreshold called");
    return NULL;
}

void* CGContextGetTextMatrix(void)
{
    if (verbose) puts("STUB: CGContextGetTextMatrix called");
    return NULL;
}

void* CGContextGetTextPosition(void)
{
    if (verbose) puts("STUB: CGContextGetTextPosition called");
    return NULL;
}

void* CGContextGetTransferFunctions(void)
{
    if (verbose) puts("STUB: CGContextGetTransferFunctions called");
    return NULL;
}

void* CGContextGetType(void)
{
    if (verbose) puts("STUB: CGContextGetType called");
    return NULL;
}

void* CGContextGetTypeID(void)
{
    if (verbose) puts("STUB: CGContextGetTypeID called");
    return NULL;
}

void* CGContextGetUndercolorRemoval(void)
{
    if (verbose) puts("STUB: CGContextGetUndercolorRemoval called");
    return NULL;
}

void* CGContextGetUserSpaceToDeviceSpaceTransform(void)
{
    if (verbose) puts("STUB: CGContextGetUserSpaceToDeviceSpaceTransform called");
    return NULL;
}

void* CGContextHandleInvalid(void)
{
    if (verbose) puts("STUB: CGContextHandleInvalid called");
    return NULL;
}

void* CGContextIsEmptyClipBoundingBox(void)
{
    if (verbose) puts("STUB: CGContextIsEmptyClipBoundingBox called");
    return NULL;
}

void* CGContextIsPathEmpty(void)
{
    if (verbose) puts("STUB: CGContextIsPathEmpty called");
    return NULL;
}

void* CGContextMoveToPoint(void)
{
    if (verbose) puts("STUB: CGContextMoveToPoint called");
    return NULL;
}

void* CGContextPathContainsPoint(void)
{
    if (verbose) puts("STUB: CGContextPathContainsPoint called");
    return NULL;
}

void* CGContextRelease(void)
{
    if (verbose) puts("STUB: CGContextRelease called");
    return NULL;
}

void* CGContextRemoveFilter(void)
{
    if (verbose) puts("STUB: CGContextRemoveFilter called");
    return NULL;
}

void* CGContextReplacePathWithClipPath(void)
{
    if (verbose) puts("STUB: CGContextReplacePathWithClipPath called");
    return NULL;
}

void* CGContextReplacePathWithShapePath(void)
{
    if (verbose) puts("STUB: CGContextReplacePathWithShapePath called");
    return NULL;
}

void* CGContextReplacePathWithStrokedPath(void)
{
    if (verbose) puts("STUB: CGContextReplacePathWithStrokedPath called");
    return NULL;
}

void* CGContextReplaceTopGState(void)
{
    if (verbose) puts("STUB: CGContextReplaceTopGState called");
    return NULL;
}

void* CGContextResetCTM(void)
{
    if (verbose) puts("STUB: CGContextResetCTM called");
    return NULL;
}

void* CGContextResetClip(void)
{
    if (verbose) puts("STUB: CGContextResetClip called");
    return NULL;
}

void* CGContextResetState(void)
{
    if (verbose) puts("STUB: CGContextResetState called");
    return NULL;
}

void* CGContextResetTopGState(void)
{
    if (verbose) puts("STUB: CGContextResetTopGState called");
    return NULL;
}

void* CGContextRestoreGState(void)
{
    if (verbose) puts("STUB: CGContextRestoreGState called");
    return NULL;
}

void* CGContextRestoreToGStateMark(void)
{
    if (verbose) puts("STUB: CGContextRestoreToGStateMark called");
    return NULL;
}

void* CGContextRetain(void)
{
    if (verbose) puts("STUB: CGContextRetain called");
    return NULL;
}

void* CGContextRotateCTM(void)
{
    if (verbose) puts("STUB: CGContextRotateCTM called");
    return NULL;
}

void* CGContextSaveGState(void)
{
    if (verbose) puts("STUB: CGContextSaveGState called");
    return NULL;
}

void* CGContextScaleCTM(void)
{
    if (verbose) puts("STUB: CGContextScaleCTM called");
    return NULL;
}

void* CGContextSelectFont(void)
{
    if (verbose) puts("STUB: CGContextSelectFont called");
    return NULL;
}

void* CGContextSetAllowsAntialiasing(void)
{
    if (verbose) puts("STUB: CGContextSetAllowsAntialiasing called");
    return NULL;
}

void* CGContextSetAllowsDithering(void)
{
    if (verbose) puts("STUB: CGContextSetAllowsDithering called");
    return NULL;
}

void* CGContextSetAllowsFontAntialiasing(void)
{
    if (verbose) puts("STUB: CGContextSetAllowsFontAntialiasing called");
    return NULL;
}

void* CGContextSetAllowsFontSmoothing(void)
{
    if (verbose) puts("STUB: CGContextSetAllowsFontSmoothing called");
    return NULL;
}

void* CGContextSetAllowsFontSubpixelPositioning(void)
{
    if (verbose) puts("STUB: CGContextSetAllowsFontSubpixelPositioning called");
    return NULL;
}

void* CGContextSetAllowsFontSubpixelQuantization(void)
{
    if (verbose) puts("STUB: CGContextSetAllowsFontSubpixelQuantization called");
    return NULL;
}

void* CGContextSetAlpha(void)
{
    if (verbose) puts("STUB: CGContextSetAlpha called");
    return NULL;
}

void* CGContextSetAlphaIsShape(void)
{
    if (verbose) puts("STUB: CGContextSetAlphaIsShape called");
    return NULL;
}

void* CGContextSetBaseCTM(void)
{
    if (verbose) puts("STUB: CGContextSetBaseCTM called");
    return NULL;
}

void* CGContextSetBlackGeneration(void)
{
    if (verbose) puts("STUB: CGContextSetBlackGeneration called");
    return NULL;
}

void* CGContextSetBlendMode(void)
{
    if (verbose) puts("STUB: CGContextSetBlendMode called");
    return NULL;
}

void* CGContextSetCMYKFillColor(void)
{
    if (verbose) puts("STUB: CGContextSetCMYKFillColor called");
    return NULL;
}

void* CGContextSetCMYKStrokeColor(void)
{
    if (verbose) puts("STUB: CGContextSetCMYKStrokeColor called");
    return NULL;
}

void* CGContextSetCTM(void)
{
    if (verbose) puts("STUB: CGContextSetCTM called");
    return NULL;
}

void* CGContextSetCharacterSpacing(void)
{
    if (verbose) puts("STUB: CGContextSetCharacterSpacing called");
    return NULL;
}

void* CGContextSetCompositeOperation(void)
{
    if (verbose) puts("STUB: CGContextSetCompositeOperation called");
    return NULL;
}

void* CGContextSetDelegate(void)
{
    if (verbose) puts("STUB: CGContextSetDelegate called");
    return NULL;
}

void* CGContextSetFillColor(void)
{
    if (verbose) puts("STUB: CGContextSetFillColor called");
    return NULL;
}

void* CGContextSetFillColorSpace(void)
{
    if (verbose) puts("STUB: CGContextSetFillColorSpace called");
    return NULL;
}

void* CGContextSetFillColorWithColor(void)
{
    if (verbose) puts("STUB: CGContextSetFillColorWithColor called");
    return NULL;
}

void* CGContextSetFillOverprint(void)
{
    if (verbose) puts("STUB: CGContextSetFillOverprint called");
    return NULL;
}

void* CGContextSetFillPattern(void)
{
    if (verbose) puts("STUB: CGContextSetFillPattern called");
    return NULL;
}

void* CGContextSetFlatness(void)
{
    if (verbose) puts("STUB: CGContextSetFlatness called");
    return NULL;
}

void* CGContextSetFlush(void)
{
    if (verbose) puts("STUB: CGContextSetFlush called");
    return NULL;
}

void* CGContextSetFocusRingWithColor(void)
{
    if (verbose) puts("STUB: CGContextSetFocusRingWithColor called");
    return NULL;
}

void* CGContextSetFont(void)
{
    if (verbose) puts("STUB: CGContextSetFont called");
    return NULL;
}

void* CGContextSetFontAntialiasingStyle(void)
{
    if (verbose) puts("STUB: CGContextSetFontAntialiasingStyle called");
    return NULL;
}

void* CGContextSetFontDilation(void)
{
    if (verbose) puts("STUB: CGContextSetFontDilation called");
    return NULL;
}

void* CGContextSetFontRenderingMode(void)
{
    if (verbose) puts("STUB: CGContextSetFontRenderingMode called");
    return NULL;
}

void* CGContextSetFontRenderingStyle(void)
{
    if (verbose) puts("STUB: CGContextSetFontRenderingStyle called");
    return NULL;
}

void* CGContextSetFontSize(void)
{
    if (verbose) puts("STUB: CGContextSetFontSize called");
    return NULL;
}

void* CGContextSetFontSmoothingBackgroundColor(void)
{
    if (verbose) puts("STUB: CGContextSetFontSmoothingBackgroundColor called");
    return NULL;
}

void* CGContextSetFontSmoothingContrast(void)
{
    if (verbose) puts("STUB: CGContextSetFontSmoothingContrast called");
    return NULL;
}

void* CGContextSetFontSmoothingStyle(void)
{
    if (verbose) puts("STUB: CGContextSetFontSmoothingStyle called");
    return NULL;
}

void* CGContextSetFontSubpixelQuantizationLevel(void)
{
    if (verbose) puts("STUB: CGContextSetFontSubpixelQuantizationLevel called");
    return NULL;
}

void* CGContextSetGrayFillColor(void)
{
    if (verbose) puts("STUB: CGContextSetGrayFillColor called");
    return NULL;
}

void* CGContextSetGrayStrokeColor(void)
{
    if (verbose) puts("STUB: CGContextSetGrayStrokeColor called");
    return NULL;
}

void* CGContextSetImageRenderingIntent(void)
{
    if (verbose) puts("STUB: CGContextSetImageRenderingIntent called");
    return NULL;
}

void* CGContextSetImageReplacer(void)
{
    if (verbose) puts("STUB: CGContextSetImageReplacer called");
    return NULL;
}

void* CGContextSetInterpolationQuality(void)
{
    if (verbose) puts("STUB: CGContextSetInterpolationQuality called");
    return NULL;
}

void* CGContextSetInterpolationQualityRange(void)
{
    if (verbose) puts("STUB: CGContextSetInterpolationQualityRange called");
    return NULL;
}

void* CGContextSetLineCap(void)
{
    if (verbose) puts("STUB: CGContextSetLineCap called");
    return NULL;
}

void* CGContextSetLineDash(void)
{
    if (verbose) puts("STUB: CGContextSetLineDash called");
    return NULL;
}

void* CGContextSetLineJoin(void)
{
    if (verbose) puts("STUB: CGContextSetLineJoin called");
    return NULL;
}

void* CGContextSetLineWidth(void)
{
    if (verbose) puts("STUB: CGContextSetLineWidth called");
    return NULL;
}

void* CGContextSetLineWidthThreshold(void)
{
    if (verbose) puts("STUB: CGContextSetLineWidthThreshold called");
    return NULL;
}

void* CGContextSetMiterLimit(void)
{
    if (verbose) puts("STUB: CGContextSetMiterLimit called");
    return NULL;
}

void* CGContextSetOverprintMode(void)
{
    if (verbose) puts("STUB: CGContextSetOverprintMode called");
    return NULL;
}

void* CGContextSetPatternPhase(void)
{
    if (verbose) puts("STUB: CGContextSetPatternPhase called");
    return NULL;
}

void* CGContextSetProperty(void)
{
    if (verbose) puts("STUB: CGContextSetProperty called");
    return NULL;
}

void* CGContextSetRGBFillColor(void)
{
    if (verbose) puts("STUB: CGContextSetRGBFillColor called");
    return NULL;
}

void* CGContextSetRGBStrokeColor(void)
{
    if (verbose) puts("STUB: CGContextSetRGBStrokeColor called");
    return NULL;
}

void* CGContextSetRenderingIntent(void)
{
    if (verbose) puts("STUB: CGContextSetRenderingIntent called");
    return NULL;
}

void* CGContextSetRenderingResolution(void)
{
    if (verbose) puts("STUB: CGContextSetRenderingResolution called");
    return NULL;
}

void* CGContextSetShadow(void)
{
    if (verbose) puts("STUB: CGContextSetShadow called");
    return NULL;
}

void* CGContextSetShadowWithColor(void)
{
    if (verbose) puts("STUB: CGContextSetShadowWithColor called");
    return NULL;
}

void* CGContextSetShouldAntialias(void)
{
    if (verbose) puts("STUB: CGContextSetShouldAntialias called");
    return NULL;
}

void* CGContextSetShouldAntialiasFonts(void)
{
    if (verbose) puts("STUB: CGContextSetShouldAntialiasFonts called");
    return NULL;
}

void* CGContextSetShouldDrawBitmapRuns(void)
{
    if (verbose) puts("STUB: CGContextSetShouldDrawBitmapRuns called");
    return NULL;
}

void* CGContextSetShouldSmoothFonts(void)
{
    if (verbose) puts("STUB: CGContextSetShouldSmoothFonts called");
    return NULL;
}

void* CGContextSetShouldSubpixelPositionFonts(void)
{
    if (verbose) puts("STUB: CGContextSetShouldSubpixelPositionFonts called");
    return NULL;
}

void* CGContextSetShouldSubpixelQuantizeFonts(void)
{
    if (verbose) puts("STUB: CGContextSetShouldSubpixelQuantizeFonts called");
    return NULL;
}

void* CGContextSetShouldUsePlatformNativeGlyphs(void)
{
    if (verbose) puts("STUB: CGContextSetShouldUsePlatformNativeGlyphs called");
    return NULL;
}

void* CGContextSetSmoothness(void)
{
    if (verbose) puts("STUB: CGContextSetSmoothness called");
    return NULL;
}

void* CGContextSetSoftMask(void)
{
    if (verbose) puts("STUB: CGContextSetSoftMask called");
    return NULL;
}

void* CGContextSetStrokeAdjust(void)
{
    if (verbose) puts("STUB: CGContextSetStrokeAdjust called");
    return NULL;
}

void* CGContextSetStrokeColor(void)
{
    if (verbose) puts("STUB: CGContextSetStrokeColor called");
    return NULL;
}

void* CGContextSetStrokeColorSpace(void)
{
    if (verbose) puts("STUB: CGContextSetStrokeColorSpace called");
    return NULL;
}

void* CGContextSetStrokeColorWithColor(void)
{
    if (verbose) puts("STUB: CGContextSetStrokeColorWithColor called");
    return NULL;
}

void* CGContextSetStrokeOverprint(void)
{
    if (verbose) puts("STUB: CGContextSetStrokeOverprint called");
    return NULL;
}

void* CGContextSetStrokePattern(void)
{
    if (verbose) puts("STUB: CGContextSetStrokePattern called");
    return NULL;
}

void* CGContextSetStyle(void)
{
    if (verbose) puts("STUB: CGContextSetStyle called");
    return NULL;
}

void* CGContextSetTextDrawingMode(void)
{
    if (verbose) puts("STUB: CGContextSetTextDrawingMode called");
    return NULL;
}

void* CGContextSetTextGreekingThreshold(void)
{
    if (verbose) puts("STUB: CGContextSetTextGreekingThreshold called");
    return NULL;
}

void* CGContextSetTextMatrix(void)
{
    if (verbose) puts("STUB: CGContextSetTextMatrix called");
    return NULL;
}

void* CGContextSetTextPosition(void)
{
    if (verbose) puts("STUB: CGContextSetTextPosition called");
    return NULL;
}

void* CGContextSetTransferFunctions(void)
{
    if (verbose) puts("STUB: CGContextSetTransferFunctions called");
    return NULL;
}

void* CGContextSetUndercolorRemoval(void)
{
    if (verbose) puts("STUB: CGContextSetUndercolorRemoval called");
    return NULL;
}

void* CGContextShowGlyphs(void)
{
    if (verbose) puts("STUB: CGContextShowGlyphs called");
    return NULL;
}

void* CGContextShowGlyphsAtPoint(void)
{
    if (verbose) puts("STUB: CGContextShowGlyphsAtPoint called");
    return NULL;
}

void* CGContextShowGlyphsAtPositions(void)
{
    if (verbose) puts("STUB: CGContextShowGlyphsAtPositions called");
    return NULL;
}

void* CGContextShowGlyphsAtPositionsWithString(void)
{
    if (verbose) puts("STUB: CGContextShowGlyphsAtPositionsWithString called");
    return NULL;
}

void* CGContextShowGlyphsWithAdvances(void)
{
    if (verbose) puts("STUB: CGContextShowGlyphsWithAdvances called");
    return NULL;
}

void* CGContextShowText(void)
{
    if (verbose) puts("STUB: CGContextShowText called");
    return NULL;
}

void* CGContextShowTextAtPoint(void)
{
    if (verbose) puts("STUB: CGContextShowTextAtPoint called");
    return NULL;
}

void* CGContextStrokeEllipseInRect(void)
{
    if (verbose) puts("STUB: CGContextStrokeEllipseInRect called");
    return NULL;
}

void* CGContextStrokeLineSegments(void)
{
    if (verbose) puts("STUB: CGContextStrokeLineSegments called");
    return NULL;
}

void* CGContextStrokePath(void)
{
    if (verbose) puts("STUB: CGContextStrokePath called");
    return NULL;
}

void* CGContextStrokeRect(void)
{
    if (verbose) puts("STUB: CGContextStrokeRect called");
    return NULL;
}

void* CGContextStrokeRectWithWidth(void)
{
    if (verbose) puts("STUB: CGContextStrokeRectWithWidth called");
    return NULL;
}

void* CGContextSynchronize(void)
{
    if (verbose) puts("STUB: CGContextSynchronize called");
    return NULL;
}

void* CGContextTranslateCTM(void)
{
    if (verbose) puts("STUB: CGContextTranslateCTM called");
    return NULL;
}

void* CGContextValidate(void)
{
    if (verbose) puts("STUB: CGContextValidate called");
    return NULL;
}

void* CGContextWait(void)
{
    if (verbose) puts("STUB: CGContextWait called");
    return NULL;
}

void* CGCryptorCreateAES(void)
{
    if (verbose) puts("STUB: CGCryptorCreateAES called");
    return NULL;
}

void* CGCryptorCreateRC4(void)
{
    if (verbose) puts("STUB: CGCryptorCreateRC4 called");
    return NULL;
}

void* CGCryptorDrain(void)
{
    if (verbose) puts("STUB: CGCryptorDrain called");
    return NULL;
}

void* CGCryptorGetCipher(void)
{
    if (verbose) puts("STUB: CGCryptorGetCipher called");
    return NULL;
}

void* CGCryptorGetInitializationVector(void)
{
    if (verbose) puts("STUB: CGCryptorGetInitializationVector called");
    return NULL;
}

void* CGCryptorProcessBytes(void)
{
    if (verbose) puts("STUB: CGCryptorProcessBytes called");
    return NULL;
}

void* CGCryptorRelease(void)
{
    if (verbose) puts("STUB: CGCryptorRelease called");
    return NULL;
}

void* CGCryptorReset(void)
{
    if (verbose) puts("STUB: CGCryptorReset called");
    return NULL;
}

void* CGDashCreate(void)
{
    if (verbose) puts("STUB: CGDashCreate called");
    return NULL;
}

void* CGDashEqualToDash(void)
{
    if (verbose) puts("STUB: CGDashEqualToDash called");
    return NULL;
}

void* CGDashGetPattern(void)
{
    if (verbose) puts("STUB: CGDashGetPattern called");
    return NULL;
}

void* CGDashRelease(void)
{
    if (verbose) puts("STUB: CGDashRelease called");
    return NULL;
}

void* CGDashRetain(void)
{
    if (verbose) puts("STUB: CGDashRetain called");
    return NULL;
}

void* CGDataConsumerClose(void)
{
    if (verbose) puts("STUB: CGDataConsumerClose called");
    return NULL;
}

void* CGDataConsumerCreate(void)
{
    if (verbose) puts("STUB: CGDataConsumerCreate called");
    return NULL;
}

void* CGDataConsumerCreateFlateEncoder(void)
{
    if (verbose) puts("STUB: CGDataConsumerCreateFlateEncoder called");
    return NULL;
}

void* CGDataConsumerCreateWithCFData(void)
{
    if (verbose) puts("STUB: CGDataConsumerCreateWithCFData called");
    return NULL;
}

void* CGDataConsumerCreateWithFilename(void)
{
    if (verbose) puts("STUB: CGDataConsumerCreateWithFilename called");
    return NULL;
}

void* CGDataConsumerCreateWithURL(void)
{
    if (verbose) puts("STUB: CGDataConsumerCreateWithURL called");
    return NULL;
}

void* CGDataConsumerFlush(void)
{
    if (verbose) puts("STUB: CGDataConsumerFlush called");
    return NULL;
}

void* CGDataConsumerGetByteCount(void)
{
    if (verbose) puts("STUB: CGDataConsumerGetByteCount called");
    return NULL;
}

void* CGDataConsumerGetTypeID(void)
{
    if (verbose) puts("STUB: CGDataConsumerGetTypeID called");
    return NULL;
}

void* CGDataConsumerGetURL(void)
{
    if (verbose) puts("STUB: CGDataConsumerGetURL called");
    return NULL;
}

void* CGDataConsumerPrintf(void)
{
    if (verbose) puts("STUB: CGDataConsumerPrintf called");
    return NULL;
}

void* CGDataConsumerPutBytes(void)
{
    if (verbose) puts("STUB: CGDataConsumerPutBytes called");
    return NULL;
}

void* CGDataConsumerRelease(void)
{
    if (verbose) puts("STUB: CGDataConsumerRelease called");
    return NULL;
}

void* CGDataConsumerRetain(void)
{
    if (verbose) puts("STUB: CGDataConsumerRetain called");
    return NULL;
}

void* CGDataConsumerVPrintf(void)
{
    if (verbose) puts("STUB: CGDataConsumerVPrintf called");
    return NULL;
}

void* CGDataProviderAccessSessionBegin(void)
{
    if (verbose) puts("STUB: CGDataProviderAccessSessionBegin called");
    return NULL;
}

void* CGDataProviderAccessSessionEnd(void)
{
    if (verbose) puts("STUB: CGDataProviderAccessSessionEnd called");
    return NULL;
}

void* CGDataProviderCopyData(void)
{
    if (verbose) puts("STUB: CGDataProviderCopyData called");
    return NULL;
}

void* CGDataProviderCreate(void)
{
    if (verbose) puts("STUB: CGDataProviderCreate called");
    return NULL;
}

void* CGDataProviderCreateDirect(void)
{
    if (verbose) puts("STUB: CGDataProviderCreateDirect called");
    return NULL;
}

void* CGDataProviderCreateDirectAccess(void)
{
    if (verbose) puts("STUB: CGDataProviderCreateDirectAccess called");
    return NULL;
}

void* CGDataProviderCreateSequential(void)
{
    if (verbose) puts("STUB: CGDataProviderCreateSequential called");
    return NULL;
}

void* CGDataProviderCreateWithCFData(void)
{
    if (verbose) puts("STUB: CGDataProviderCreateWithCFData called");
    return NULL;
}

void* CGDataProviderCreateWithCopyOfData(void)
{
    if (verbose) puts("STUB: CGDataProviderCreateWithCopyOfData called");
    return NULL;
}

void* CGDataProviderCreateWithData(void)
{
    if (verbose) puts("STUB: CGDataProviderCreateWithData called");
    return NULL;
}

void* CGDataProviderCreateWithFilename(void)
{
    if (verbose) puts("STUB: CGDataProviderCreateWithFilename called");
    return NULL;
}

void* CGDataProviderCreateWithSwappedProvider(void)
{
    if (verbose) puts("STUB: CGDataProviderCreateWithSwappedProvider called");
    return NULL;
}

void* CGDataProviderCreateWithURL(void)
{
    if (verbose) puts("STUB: CGDataProviderCreateWithURL called");
    return NULL;
}

void* CGDataProviderGetBytePointer(void)
{
    if (verbose) puts("STUB: CGDataProviderGetBytePointer called");
    return NULL;
}

void* CGDataProviderGetBytePtr(void)
{
    if (verbose) puts("STUB: CGDataProviderGetBytePtr called");
    return NULL;
}

void* CGDataProviderGetBytes(void)
{
    if (verbose) puts("STUB: CGDataProviderGetBytes called");
    return NULL;
}

void* CGDataProviderGetBytesAtPosition(void)
{
    if (verbose) puts("STUB: CGDataProviderGetBytesAtPosition called");
    return NULL;
}

void* CGDataProviderGetChunks(void)
{
    if (verbose) puts("STUB: CGDataProviderGetChunks called");
    return NULL;
}

void* CGDataProviderGetInfo(void)
{
    if (verbose) puts("STUB: CGDataProviderGetInfo called");
    return NULL;
}

void* CGDataProviderGetSize(void)
{
    if (verbose) puts("STUB: CGDataProviderGetSize called");
    return NULL;
}

void* CGDataProviderGetSize2(void)
{
    if (verbose) puts("STUB: CGDataProviderGetSize2 called");
    return NULL;
}

void* CGDataProviderGetSizeOfData(void)
{
    if (verbose) puts("STUB: CGDataProviderGetSizeOfData called");
    return NULL;
}

void* CGDataProviderGetTypeID(void)
{
    if (verbose) puts("STUB: CGDataProviderGetTypeID called");
    return NULL;
}

void* CGDataProviderHasDataPtr(void)
{
    if (verbose) puts("STUB: CGDataProviderHasDataPtr called");
    return NULL;
}

void* CGDataProviderLock(void)
{
    if (verbose) puts("STUB: CGDataProviderLock called");
    return NULL;
}

void* CGDataProviderRelease(void)
{
    if (verbose) puts("STUB: CGDataProviderRelease called");
    return NULL;
}

void* CGDataProviderReleaseBytePtr(void)
{
    if (verbose) puts("STUB: CGDataProviderReleaseBytePtr called");
    return NULL;
}

void* CGDataProviderRetain(void)
{
    if (verbose) puts("STUB: CGDataProviderRetain called");
    return NULL;
}

void* CGDataProviderRetainBytePtr(void)
{
    if (verbose) puts("STUB: CGDataProviderRetainBytePtr called");
    return NULL;
}

void* CGDataProviderRewind(void)
{
    if (verbose) puts("STUB: CGDataProviderRewind called");
    return NULL;
}

void* CGDataProviderSkipBytes(void)
{
    if (verbose) puts("STUB: CGDataProviderSkipBytes called");
    return NULL;
}

void* CGDataProviderSkipForward(void)
{
    if (verbose) puts("STUB: CGDataProviderSkipForward called");
    return NULL;
}

void* CGDataProviderUngetBytePtr(void)
{
    if (verbose) puts("STUB: CGDataProviderUngetBytePtr called");
    return NULL;
}

void* CGDataProviderUnlock(void)
{
    if (verbose) puts("STUB: CGDataProviderUnlock called");
    return NULL;
}

void* CGDefaultsCopyString(void)
{
    if (verbose) puts("STUB: CGDefaultsCopyString called");
    return NULL;
}

void* CGDefaultsGetBoolean(void)
{
    if (verbose) puts("STUB: CGDefaultsGetBoolean called");
    return NULL;
}

void* CGDefaultsGetInteger(void)
{
    if (verbose) puts("STUB: CGDefaultsGetInteger called");
    return NULL;
}

void* CGDigesterAddBytes(void)
{
    if (verbose) puts("STUB: CGDigesterAddBytes called");
    return NULL;
}

void* CGDigesterCreateMD5(void)
{
    if (verbose) puts("STUB: CGDigesterCreateMD5 called");
    return NULL;
}

void* CGDigesterGetDigest(void)
{
    if (verbose) puts("STUB: CGDigesterGetDigest called");
    return NULL;
}

void* CGDigesterRelease(void)
{
    if (verbose) puts("STUB: CGDigesterRelease called");
    return NULL;
}

void* CGDisplayList2Create(void)
{
    if (verbose) puts("STUB: CGDisplayList2Create called");
    return NULL;
}

void* CGDisplayList2CreateWithRect(void)
{
    if (verbose) puts("STUB: CGDisplayList2CreateWithRect called");
    return NULL;
}

void* CGDisplayList2GetImpl(void)
{
    if (verbose) puts("STUB: CGDisplayList2GetImpl called");
    return NULL;
}

void* CGDisplayList2GetTypeID(void)
{
    if (verbose) puts("STUB: CGDisplayList2GetTypeID called");
    return NULL;
}

void* CGDisplayListContainsTransparentContent(void)
{
    if (verbose) puts("STUB: CGDisplayListContainsTransparentContent called");
    return NULL;
}

void* CGDisplayListContextCreate(void)
{
    if (verbose) puts("STUB: CGDisplayListContextCreate called");
    return NULL;
}

void* CGDisplayListContextDelegateCreate(void)
{
    if (verbose) puts("STUB: CGDisplayListContextDelegateCreate called");
    return NULL;
}

void* CGDisplayListContextDelegateCreatePageBased(void)
{
    if (verbose) puts("STUB: CGDisplayListContextDelegateCreatePageBased called");
    return NULL;
}

void* CGDisplayListCreate(void)
{
    if (verbose) puts("STUB: CGDisplayListCreate called");
    return NULL;
}

void* CGDisplayListCreateWithRect(void)
{
    if (verbose) puts("STUB: CGDisplayListCreateWithRect called");
    return NULL;
}

void* CGDisplayListCreateWithURL(void)
{
    if (verbose) puts("STUB: CGDisplayListCreateWithURL called");
    return NULL;
}

void* CGDisplayListDelegateDrawDisplayList(void)
{
    if (verbose) puts("STUB: CGDisplayListDelegateDrawDisplayList called");
    return NULL;
}

void* CGDisplayListDrawInContext(void)
{
    if (verbose) puts("STUB: CGDisplayListDrawInContext called");
    return NULL;
}

void* CGDisplayListDrawInContextDelegate(void)
{
    if (verbose) puts("STUB: CGDisplayListDrawInContextDelegate called");
    return NULL;
}

void* CGDisplayListDumpToFile(void)
{
    if (verbose) puts("STUB: CGDisplayListDumpToFile called");
    return NULL;
}

void* CGDisplayListExecute(void)
{
    if (verbose) puts("STUB: CGDisplayListExecute called");
    return NULL;
}

void* CGDisplayListGetAuxiliaryInfo(void)
{
    if (verbose) puts("STUB: CGDisplayListGetAuxiliaryInfo called");
    return NULL;
}

void* CGDisplayListGetBoundingBox(void)
{
    if (verbose) puts("STUB: CGDisplayListGetBoundingBox called");
    return NULL;
}

void* CGDisplayListGetInfo(void)
{
    if (verbose) puts("STUB: CGDisplayListGetInfo called");
    return NULL;
}

void* CGDisplayListGetTypeID(void)
{
    if (verbose) puts("STUB: CGDisplayListGetTypeID called");
    return NULL;
}

void* CGDisplayListRelease(void)
{
    if (verbose) puts("STUB: CGDisplayListRelease called");
    return NULL;
}

void* CGDisplayListReset(void)
{
    if (verbose) puts("STUB: CGDisplayListReset called");
    return NULL;
}

void* CGDisplayListRetain(void)
{
    if (verbose) puts("STUB: CGDisplayListRetain called");
    return NULL;
}

void* CGDisplayListSerializeToURL(void)
{
    if (verbose) puts("STUB: CGDisplayListSerializeToURL called");
    return NULL;
}

void* CGDisplayListSetBoundingBox(void)
{
    if (verbose) puts("STUB: CGDisplayListSetBoundingBox called");
    return NULL;
}

void* CGFloatIsValid(void)
{
    if (verbose) puts("STUB: CGFloatIsValid called");
    return NULL;
}

void* CGFontApplyFunction(void)
{
    if (verbose) puts("STUB: CGFontApplyFunction called");
    return NULL;
}

void* CGFontCacheCreate(void)
{
    if (verbose) puts("STUB: CGFontCacheCreate called");
    return NULL;
}

void* CGFontCacheGetLocalCache(void)
{
    if (verbose) puts("STUB: CGFontCacheGetLocalCache called");
    return NULL;
}

void* CGFontCacheGetMaxSize(void)
{
    if (verbose) puts("STUB: CGFontCacheGetMaxSize called");
    return NULL;
}

void* CGFontCachePrint(void)
{
    if (verbose) puts("STUB: CGFontCachePrint called");
    return NULL;
}

void* CGFontCacheRelease(void)
{
    if (verbose) puts("STUB: CGFontCacheRelease called");
    return NULL;
}

void* CGFontCacheRemoveFont(void)
{
    if (verbose) puts("STUB: CGFontCacheRemoveFont called");
    return NULL;
}

void* CGFontCacheReset(void)
{
    if (verbose) puts("STUB: CGFontCacheReset called");
    return NULL;
}

void* CGFontCacheResetAllCaches(void)
{
    if (verbose) puts("STUB: CGFontCacheResetAllCaches called");
    return NULL;
}

void* CGFontCacheRetain(void)
{
    if (verbose) puts("STUB: CGFontCacheRetain called");
    return NULL;
}

void* CGFontCacheSetMaxSize(void)
{
    if (verbose) puts("STUB: CGFontCacheSetMaxSize called");
    return NULL;
}

void* CGFontCacheSetShouldAutoExpire(void)
{
    if (verbose) puts("STUB: CGFontCacheSetShouldAutoExpire called");
    return NULL;
}

void* CGFontCanCreatePostScriptSubset(void)
{
    if (verbose) puts("STUB: CGFontCanCreatePostScriptSubset called");
    return NULL;
}

void* CGFontCopyCharacterSet(void)
{
    if (verbose) puts("STUB: CGFontCopyCharacterSet called");
    return NULL;
}

void* CGFontCopyFamilyName(void)
{
    if (verbose) puts("STUB: CGFontCopyFamilyName called");
    return NULL;
}

void* CGFontCopyFullName(void)
{
    if (verbose) puts("STUB: CGFontCopyFullName called");
    return NULL;
}

void* CGFontCopyGlyphNameForGlyph(void)
{
    if (verbose) puts("STUB: CGFontCopyGlyphNameForGlyph called");
    return NULL;
}

void* CGFontCopyLocalizedName(void)
{
    if (verbose) puts("STUB: CGFontCopyLocalizedName called");
    return NULL;
}

void* CGFontCopyName(void)
{
    if (verbose) puts("STUB: CGFontCopyName called");
    return NULL;
}

void* CGFontCopyPath(void)
{
    if (verbose) puts("STUB: CGFontCopyPath called");
    return NULL;
}

void* CGFontCopyPostScriptName(void)
{
    if (verbose) puts("STUB: CGFontCopyPostScriptName called");
    return NULL;
}

void* CGFontCopyTableForTag(void)
{
    if (verbose) puts("STUB: CGFontCopyTableForTag called");
    return NULL;
}

void* CGFontCopyTableTags(void)
{
    if (verbose) puts("STUB: CGFontCopyTableTags called");
    return NULL;
}

void* CGFontCopyURL(void)
{
    if (verbose) puts("STUB: CGFontCopyURL called");
    return NULL;
}

void* CGFontCopyVariationAxes(void)
{
    if (verbose) puts("STUB: CGFontCopyVariationAxes called");
    return NULL;
}

void* CGFontCopyVariations(void)
{
    if (verbose) puts("STUB: CGFontCopyVariations called");
    return NULL;
}

void* CGFontCreate(void)
{
    if (verbose) puts("STUB: CGFontCreate called");
    return NULL;
}

void* CGFontCreateCopy(void)
{
    if (verbose) puts("STUB: CGFontCreateCopy called");
    return NULL;
}

void* CGFontCreateCopyWithVariations(void)
{
    if (verbose) puts("STUB: CGFontCreateCopyWithVariations called");
    return NULL;
}

void* CGFontCreateFontSubset(void)
{
    if (verbose) puts("STUB: CGFontCreateFontSubset called");
    return NULL;
}

void* CGFontCreateFontsFromData(void)
{
    if (verbose) puts("STUB: CGFontCreateFontsFromData called");
    return NULL;
}

void* CGFontCreateFontsWithPath(void)
{
    if (verbose) puts("STUB: CGFontCreateFontsWithPath called");
    return NULL;
}

void* CGFontCreateFontsWithURL(void)
{
    if (verbose) puts("STUB: CGFontCreateFontsWithURL called");
    return NULL;
}

void* CGFontCreateGlyphPath(void)
{
    if (verbose) puts("STUB: CGFontCreateGlyphPath called");
    return NULL;
}

void* CGFontCreateMatchingFont(void)
{
    if (verbose) puts("STUB: CGFontCreateMatchingFont called");
    return NULL;
}

void* CGFontCreatePostScriptEncoding(void)
{
    if (verbose) puts("STUB: CGFontCreatePostScriptEncoding called");
    return NULL;
}

void* CGFontCreatePostScriptSubset(void)
{
    if (verbose) puts("STUB: CGFontCreatePostScriptSubset called");
    return NULL;
}

void* CGFontCreateWithDataProvider(void)
{
    if (verbose) puts("STUB: CGFontCreateWithDataProvider called");
    return NULL;
}

void* CGFontCreateWithFontName(void)
{
    if (verbose) puts("STUB: CGFontCreateWithFontName called");
    return NULL;
}

void* CGFontCreateWithName(void)
{
    if (verbose) puts("STUB: CGFontCreateWithName called");
    return NULL;
}

void* CGFontCreateWithPlatformFont(void)
{
    if (verbose) puts("STUB: CGFontCreateWithPlatformFont called");
    return NULL;
}

void* CGFontCreateWithPlatformFontAndVariation(void)
{
    if (verbose) puts("STUB: CGFontCreateWithPlatformFontAndVariation called");
    return NULL;
}

void* CGFontCreateWithVariations(void)
{
    if (verbose) puts("STUB: CGFontCreateWithVariations called");
    return NULL;
}

void* CGFontDBAddFont(void)
{
    if (verbose) puts("STUB: CGFontDBAddFont called");
    return NULL;
}

void* CGFontDBAddFontReferencesAtPath(void)
{
    if (verbose) puts("STUB: CGFontDBAddFontReferencesAtPath called");
    return NULL;
}

void* CGFontDBAddFontsAtPath(void)
{
    if (verbose) puts("STUB: CGFontDBAddFontsAtPath called");
    return NULL;
}

void* CGFontDBAddFontsFromPlist(void)
{
    if (verbose) puts("STUB: CGFontDBAddFontsFromPlist called");
    return NULL;
}

void* CGFontDBAddFontsInDirectory(void)
{
    if (verbose) puts("STUB: CGFontDBAddFontsInDirectory called");
    return NULL;
}

void* CGFontDBApplyFunction(void)
{
    if (verbose) puts("STUB: CGFontDBApplyFunction called");
    return NULL;
}

void* CGFontDBCreate(void)
{
    if (verbose) puts("STUB: CGFontDBCreate called");
    return NULL;
}

void* CGFontDBCreatePlist(void)
{
    if (verbose) puts("STUB: CGFontDBCreatePlist called");
    return NULL;
}

void* CGFontDBGetFontWithName(void)
{
    if (verbose) puts("STUB: CGFontDBGetFontWithName called");
    return NULL;
}

void* CGFontDBGetLocal(void)
{
    if (verbose) puts("STUB: CGFontDBGetLocal called");
    return NULL;
}

void* CGFontDBPurgeFont(void)
{
    if (verbose) puts("STUB: CGFontDBPurgeFont called");
    return NULL;
}

void* CGFontDBRelease(void)
{
    if (verbose) puts("STUB: CGFontDBRelease called");
    return NULL;
}

void* CGFontDBRemoveFont(void)
{
    if (verbose) puts("STUB: CGFontDBRemoveFont called");
    return NULL;
}

void* CGFontDBSetInitializer(void)
{
    if (verbose) puts("STUB: CGFontDBSetInitializer called");
    return NULL;
}

void* CGFontDefaultAllowsFontSmoothing(void)
{
    if (verbose) puts("STUB: CGFontDefaultAllowsFontSmoothing called");
    return NULL;
}

void* CGFontDefaultGetAntialiasingStyle(void)
{
    if (verbose) puts("STUB: CGFontDefaultGetAntialiasingStyle called");
    return NULL;
}

void* CGFontDefaultGetSmoothingContrast(void)
{
    if (verbose) puts("STUB: CGFontDefaultGetSmoothingContrast called");
    return NULL;
}

void* CGFontDefaultGetSmoothingStyle(void)
{
    if (verbose) puts("STUB: CGFontDefaultGetSmoothingStyle called");
    return NULL;
}

void* CGFontFinderApplyFunction(void)
{
    if (verbose) puts("STUB: CGFontFinderApplyFunction called");
    return NULL;
}

void* CGFontFinderCopyAllURLs(void)
{
    if (verbose) puts("STUB: CGFontFinderCopyAllURLs called");
    return NULL;
}

void* CGFontFinderCopyURLForName(void)
{
    if (verbose) puts("STUB: CGFontFinderCopyURLForName called");
    return NULL;
}

void* CGFontFinderCopyURLForPlatformFont(void)
{
    if (verbose) puts("STUB: CGFontFinderCopyURLForPlatformFont called");
    return NULL;
}

void* CGFontFinderCreate(void)
{
    if (verbose) puts("STUB: CGFontFinderCreate called");
    return NULL;
}

void* CGFontFinderCreateFontWithName(void)
{
    if (verbose) puts("STUB: CGFontFinderCreateFontWithName called");
    return NULL;
}

void* CGFontFinderCreateFontWithPlatformFont(void)
{
    if (verbose) puts("STUB: CGFontFinderCreateFontWithPlatformFont called");
    return NULL;
}

void* CGFontFinderGetDefault(void)
{
    if (verbose) puts("STUB: CGFontFinderGetDefault called");
    return NULL;
}

void* CGFontFinderRelease(void)
{
    if (verbose) puts("STUB: CGFontFinderRelease called");
    return NULL;
}

void* CGFontFinderRetain(void)
{
    if (verbose) puts("STUB: CGFontFinderRetain called");
    return NULL;
}

void* CGFontFinderSetDefault(void)
{
    if (verbose) puts("STUB: CGFontFinderSetDefault called");
    return NULL;
}

void* CGFontGetAscent(void)
{
    if (verbose) puts("STUB: CGFontGetAscent called");
    return NULL;
}

void* CGFontGetCIDsForGlyphs(void)
{
    if (verbose) puts("STUB: CGFontGetCIDsForGlyphs called");
    return NULL;
}

void* CGFontGetCapHeight(void)
{
    if (verbose) puts("STUB: CGFontGetCapHeight called");
    return NULL;
}

void* CGFontGetContainingBoundingBoxForGlyphs(void)
{
    if (verbose) puts("STUB: CGFontGetContainingBoundingBoxForGlyphs called");
    return NULL;
}

void* CGFontGetDescent(void)
{
    if (verbose) puts("STUB: CGFontGetDescent called");
    return NULL;
}

void* CGFontGetDescriptor(void)
{
    if (verbose) puts("STUB: CGFontGetDescriptor called");
    return NULL;
}

void* CGFontGetFontBBox(void)
{
    if (verbose) puts("STUB: CGFontGetFontBBox called");
    return NULL;
}

void* CGFontGetFontSubsetFormat(void)
{
    if (verbose) puts("STUB: CGFontGetFontSubsetFormat called");
    return NULL;
}

void* CGFontGetGlyphAdvances(void)
{
    if (verbose) puts("STUB: CGFontGetGlyphAdvances called");
    return NULL;
}

void* CGFontGetGlyphAdvancesForStyle(void)
{
    if (verbose) puts("STUB: CGFontGetGlyphAdvancesForStyle called");
    return NULL;
}

void* CGFontGetGlyphBBoxes(void)
{
    if (verbose) puts("STUB: CGFontGetGlyphBBoxes called");
    return NULL;
}

void* CGFontGetGlyphBBoxesForStyle(void)
{
    if (verbose) puts("STUB: CGFontGetGlyphBBoxesForStyle called");
    return NULL;
}

void* CGFontGetGlyphBoundingBoxes(void)
{
    if (verbose) puts("STUB: CGFontGetGlyphBoundingBoxes called");
    return NULL;
}

void* CGFontGetGlyphForGlyphName(void)
{
    if (verbose) puts("STUB: CGFontGetGlyphForGlyphName called");
    return NULL;
}

void* CGFontGetGlyphIdentifiers(void)
{
    if (verbose) puts("STUB: CGFontGetGlyphIdentifiers called");
    return NULL;
}

void* CGFontGetGlyphNameForGlyph(void)
{
    if (verbose) puts("STUB: CGFontGetGlyphNameForGlyph called");
    return NULL;
}

void* CGFontGetGlyphNameMaxLength(void)
{
    if (verbose) puts("STUB: CGFontGetGlyphNameMaxLength called");
    return NULL;
}

void* CGFontGetGlyphPath(void)
{
    if (verbose) puts("STUB: CGFontGetGlyphPath called");
    return NULL;
}

void* CGFontGetGlyphPaths(void)
{
    if (verbose) puts("STUB: CGFontGetGlyphPaths called");
    return NULL;
}

void* CGFontGetGlyphScaledAdvances(void)
{
    if (verbose) puts("STUB: CGFontGetGlyphScaledAdvances called");
    return NULL;
}

void* CGFontGetGlyphTransformedAdvances(void)
{
    if (verbose) puts("STUB: CGFontGetGlyphTransformedAdvances called");
    return NULL;
}

void* CGFontGetGlyphVerticalAdvances(void)
{
    if (verbose) puts("STUB: CGFontGetGlyphVerticalAdvances called");
    return NULL;
}

void* CGFontGetGlyphVerticalOffsets(void)
{
    if (verbose) puts("STUB: CGFontGetGlyphVerticalOffsets called");
    return NULL;
}

void* CGFontGetGlyphWithGlyphName(void)
{
    if (verbose) puts("STUB: CGFontGetGlyphWithGlyphName called");
    return NULL;
}

void* CGFontGetGlyphsForCIDs(void)
{
    if (verbose) puts("STUB: CGFontGetGlyphsForCIDs called");
    return NULL;
}

void* CGFontGetGlyphsForGlyphNames(void)
{
    if (verbose) puts("STUB: CGFontGetGlyphsForGlyphNames called");
    return NULL;
}

void* CGFontGetGlyphsForUnichars(void)
{
    if (verbose) puts("STUB: CGFontGetGlyphsForUnichars called");
    return NULL;
}

void* CGFontGetGlyphsForUnicodes(void)
{
    if (verbose) puts("STUB: CGFontGetGlyphsForUnicodes called");
    return NULL;
}

void* CGFontGetHMetrics(void)
{
    if (verbose) puts("STUB: CGFontGetHMetrics called");
    return NULL;
}

void* CGFontGetIdentifier(void)
{
    if (verbose) puts("STUB: CGFontGetIdentifier called");
    return NULL;
}

void* CGFontGetInfo(void)
{
    if (verbose) puts("STUB: CGFontGetInfo called");
    return NULL;
}

void* CGFontGetIntProperty(void)
{
    if (verbose) puts("STUB: CGFontGetIntProperty called");
    return NULL;
}

void* CGFontGetItalicAngle(void)
{
    if (verbose) puts("STUB: CGFontGetItalicAngle called");
    return NULL;
}

void* CGFontGetLeading(void)
{
    if (verbose) puts("STUB: CGFontGetLeading called");
    return NULL;
}

void* CGFontGetMaxUnichars(void)
{
    if (verbose) puts("STUB: CGFontGetMaxUnichars called");
    return NULL;
}

void* CGFontGetNumberOfGlyphs(void)
{
    if (verbose) puts("STUB: CGFontGetNumberOfGlyphs called");
    return NULL;
}

void* CGFontGetParserFont(void)
{
    if (verbose) puts("STUB: CGFontGetParserFont called");
    return NULL;
}

void* CGFontGetPostScriptName(void)
{
    if (verbose) puts("STUB: CGFontGetPostScriptName called");
    return NULL;
}

void* CGFontGetProperty(void)
{
    if (verbose) puts("STUB: CGFontGetProperty called");
    return NULL;
}

void* CGFontGetQuantizationLevelWithStyle(void)
{
    if (verbose) puts("STUB: CGFontGetQuantizationLevelWithStyle called");
    return NULL;
}

void* CGFontGetRegistryOrderingSupplement(void)
{
    if (verbose) puts("STUB: CGFontGetRegistryOrderingSupplement called");
    return NULL;
}

void* CGFontGetStemV(void)
{
    if (verbose) puts("STUB: CGFontGetStemV called");
    return NULL;
}

void* CGFontGetTypeID(void)
{
    if (verbose) puts("STUB: CGFontGetTypeID called");
    return NULL;
}

void* CGFontGetUnderlinePosition(void)
{
    if (verbose) puts("STUB: CGFontGetUnderlinePosition called");
    return NULL;
}

void* CGFontGetUnderlineThickness(void)
{
    if (verbose) puts("STUB: CGFontGetUnderlineThickness called");
    return NULL;
}

void* CGFontGetUnicharsForGlyph(void)
{
    if (verbose) puts("STUB: CGFontGetUnicharsForGlyph called");
    return NULL;
}

void* CGFontGetUnitsPerEm(void)
{
    if (verbose) puts("STUB: CGFontGetUnitsPerEm called");
    return NULL;
}

void* CGFontGetVMetrics(void)
{
    if (verbose) puts("STUB: CGFontGetVMetrics called");
    return NULL;
}

void* CGFontGetVTable(void)
{
    if (verbose) puts("STUB: CGFontGetVTable called");
    return NULL;
}

void* CGFontGetVariationAxes(void)
{
    if (verbose) puts("STUB: CGFontGetVariationAxes called");
    return NULL;
}

void* CGFontGetVariations(void)
{
    if (verbose) puts("STUB: CGFontGetVariations called");
    return NULL;
}

void* CGFontGetXHeight(void)
{
    if (verbose) puts("STUB: CGFontGetXHeight called");
    return NULL;
}

void* CGFontIndexMapAddIndex(void)
{
    if (verbose) puts("STUB: CGFontIndexMapAddIndex called");
    return NULL;
}

void* CGFontIndexMapAddIndexes(void)
{
    if (verbose) puts("STUB: CGFontIndexMapAddIndexes called");
    return NULL;
}

void* CGFontIndexMapAddRange(void)
{
    if (verbose) puts("STUB: CGFontIndexMapAddRange called");
    return NULL;
}

void* CGFontIndexMapCreate(void)
{
    if (verbose) puts("STUB: CGFontIndexMapCreate called");
    return NULL;
}

void* CGFontIndexMapCreateIdentity(void)
{
    if (verbose) puts("STUB: CGFontIndexMapCreateIdentity called");
    return NULL;
}

void* CGFontIndexMapGetValue(void)
{
    if (verbose) puts("STUB: CGFontIndexMapGetValue called");
    return NULL;
}

void* CGFontIndexMapGetValues(void)
{
    if (verbose) puts("STUB: CGFontIndexMapGetValues called");
    return NULL;
}

void* CGFontIndexMapPrint(void)
{
    if (verbose) puts("STUB: CGFontIndexMapPrint called");
    return NULL;
}

void* CGFontIndexMapRelease(void)
{
    if (verbose) puts("STUB: CGFontIndexMapRelease called");
    return NULL;
}

void* CGFontIndexMapRetain(void)
{
    if (verbose) puts("STUB: CGFontIndexMapRetain called");
    return NULL;
}

void* CGFontIndexSetAddIndex(void)
{
    if (verbose) puts("STUB: CGFontIndexSetAddIndex called");
    return NULL;
}

void* CGFontIndexSetAddIndexRange(void)
{
    if (verbose) puts("STUB: CGFontIndexSetAddIndexRange called");
    return NULL;
}

void* CGFontIndexSetAddIndexes(void)
{
    if (verbose) puts("STUB: CGFontIndexSetAddIndexes called");
    return NULL;
}

void* CGFontIndexSetContainsIndex(void)
{
    if (verbose) puts("STUB: CGFontIndexSetContainsIndex called");
    return NULL;
}

void* CGFontIndexSetCreate(void)
{
    if (verbose) puts("STUB: CGFontIndexSetCreate called");
    return NULL;
}

void* CGFontIndexSetCreateCopy(void)
{
    if (verbose) puts("STUB: CGFontIndexSetCreateCopy called");
    return NULL;
}

void* CGFontIndexSetGetCount(void)
{
    if (verbose) puts("STUB: CGFontIndexSetGetCount called");
    return NULL;
}

void* CGFontIndexSetGetIndexRange(void)
{
    if (verbose) puts("STUB: CGFontIndexSetGetIndexRange called");
    return NULL;
}

void* CGFontIndexSetGetIndexVector(void)
{
    if (verbose) puts("STUB: CGFontIndexSetGetIndexVector called");
    return NULL;
}

void* CGFontIndexSetGetIndexVectorSize(void)
{
    if (verbose) puts("STUB: CGFontIndexSetGetIndexVectorSize called");
    return NULL;
}

void* CGFontIndexSetGetIndexes(void)
{
    if (verbose) puts("STUB: CGFontIndexSetGetIndexes called");
    return NULL;
}

void* CGFontIndexSetGetName(void)
{
    if (verbose) puts("STUB: CGFontIndexSetGetName called");
    return NULL;
}

void* CGFontIndexSetRelease(void)
{
    if (verbose) puts("STUB: CGFontIndexSetRelease called");
    return NULL;
}

void* CGFontIndexSetRetain(void)
{
    if (verbose) puts("STUB: CGFontIndexSetRetain called");
    return NULL;
}

void* CGFontIsBitmapOnly(void)
{
    if (verbose) puts("STUB: CGFontIsBitmapOnly called");
    return NULL;
}

void* CGFontIsFixedPitch(void)
{
    if (verbose) puts("STUB: CGFontIsFixedPitch called");
    return NULL;
}

void* CGFontIsItalic(void)
{
    if (verbose) puts("STUB: CGFontIsItalic called");
    return NULL;
}

void* CGFontNameTableCopyLocales(void)
{
    if (verbose) puts("STUB: CGFontNameTableCopyLocales called");
    return NULL;
}

void* CGFontNameTableCopyNameForLocale(void)
{
    if (verbose) puts("STUB: CGFontNameTableCopyNameForLocale called");
    return NULL;
}

void* CGFontNameTableCopyRootName(void)
{
    if (verbose) puts("STUB: CGFontNameTableCopyRootName called");
    return NULL;
}

void* CGFontNameTableCreate(void)
{
    if (verbose) puts("STUB: CGFontNameTableCreate called");
    return NULL;
}

void* CGFontNameTableGetPostScriptName(void)
{
    if (verbose) puts("STUB: CGFontNameTableGetPostScriptName called");
    return NULL;
}

void* CGFontNameTablePrint(void)
{
    if (verbose) puts("STUB: CGFontNameTablePrint called");
    return NULL;
}

void* CGFontNameTableRelease(void)
{
    if (verbose) puts("STUB: CGFontNameTableRelease called");
    return NULL;
}

void* CGFontQuantumEqualToQuantum(void)
{
    if (verbose) puts("STUB: CGFontQuantumEqualToQuantum called");
    return NULL;
}

void* CGFontRelease(void)
{
    if (verbose) puts("STUB: CGFontRelease called");
    return NULL;
}

void* CGFontRenderingGetFontSmoothingDisabled(void)
{
    if (verbose) puts("STUB: CGFontRenderingGetFontSmoothingDisabled called");
    return NULL;
}

void* CGFontRetain(void)
{
    if (verbose) puts("STUB: CGFontRetain called");
    return NULL;
}

void* CGFontSetShouldUseMulticache(void)
{
    if (verbose) puts("STUB: CGFontSetShouldUseMulticache called");
    return NULL;
}

void* CGFontURLCreate(void)
{
    if (verbose) puts("STUB: CGFontURLCreate called");
    return NULL;
}

void* CGFunctionCopyPDFDescription(void)
{
    if (verbose) puts("STUB: CGFunctionCopyPDFDescription called");
    return NULL;
}

void* CGFunctionCreate(void)
{
    if (verbose) puts("STUB: CGFunctionCreate called");
    return NULL;
}

void* CGFunctionEvaluate(void)
{
    if (verbose) puts("STUB: CGFunctionEvaluate called");
    return NULL;
}

void* CGFunctionEvaluateFloat(void)
{
    if (verbose) puts("STUB: CGFunctionEvaluateFloat called");
    return NULL;
}

void* CGFunctionGetAssociate(void)
{
    if (verbose) puts("STUB: CGFunctionGetAssociate called");
    return NULL;
}

void* CGFunctionGetDomain(void)
{
    if (verbose) puts("STUB: CGFunctionGetDomain called");
    return NULL;
}

void* CGFunctionGetDomainDimension(void)
{
    if (verbose) puts("STUB: CGFunctionGetDomainDimension called");
    return NULL;
}

void* CGFunctionGetIdentifier(void)
{
    if (verbose) puts("STUB: CGFunctionGetIdentifier called");
    return NULL;
}

void* CGFunctionGetRange(void)
{
    if (verbose) puts("STUB: CGFunctionGetRange called");
    return NULL;
}

void* CGFunctionGetRangeDimension(void)
{
    if (verbose) puts("STUB: CGFunctionGetRangeDimension called");
    return NULL;
}

void* CGFunctionGetTypeID(void)
{
    if (verbose) puts("STUB: CGFunctionGetTypeID called");
    return NULL;
}

void* CGFunctionIsIdentity(void)
{
    if (verbose) puts("STUB: CGFunctionIsIdentity called");
    return NULL;
}

void* CGFunctionRelease(void)
{
    if (verbose) puts("STUB: CGFunctionRelease called");
    return NULL;
}

void* CGFunctionRetain(void)
{
    if (verbose) puts("STUB: CGFunctionRetain called");
    return NULL;
}

void* CGFunctionSetAssociate(void)
{
    if (verbose) puts("STUB: CGFunctionSetAssociate called");
    return NULL;
}

void* CGFunctionSetProperty(void)
{
    if (verbose) puts("STUB: CGFunctionSetProperty called");
    return NULL;
}

void* CGGLContextCreate(void)
{
    if (verbose) puts("STUB: CGGLContextCreate called");
    return NULL;
}

void* CGGLContextUpdateViewportSize(void)
{
    if (verbose) puts("STUB: CGGLContextUpdateViewportSize called");
    return NULL;
}

void* CGGStateClipToRect(void)
{
    if (verbose) puts("STUB: CGGStateClipToRect called");
    return NULL;
}

void* CGGStateCreate(void)
{
    if (verbose) puts("STUB: CGGStateCreate called");
    return NULL;
}

void* CGGStateCreateCopy(void)
{
    if (verbose) puts("STUB: CGGStateCreateCopy called");
    return NULL;
}

void* CGGStateGetAdjustedLineWidth(void)
{
    if (verbose) puts("STUB: CGGStateGetAdjustedLineWidth called");
    return NULL;
}

void* CGGStateGetAlpha(void)
{
    if (verbose) puts("STUB: CGGStateGetAlpha called");
    return NULL;
}

void* CGGStateGetAlphaIsShape(void)
{
    if (verbose) puts("STUB: CGGStateGetAlphaIsShape called");
    return NULL;
}

void* CGGStateGetBlackGeneration(void)
{
    if (verbose) puts("STUB: CGGStateGetBlackGeneration called");
    return NULL;
}

void* CGGStateGetCTM(void)
{
    if (verbose) puts("STUB: CGGStateGetCTM called");
    return NULL;
}

void* CGGStateGetCharacterSpacing(void)
{
    if (verbose) puts("STUB: CGGStateGetCharacterSpacing called");
    return NULL;
}

void* CGGStateGetClipBoundingBox(void)
{
    if (verbose) puts("STUB: CGGStateGetClipBoundingBox called");
    return NULL;
}

void* CGGStateGetClipRect(void)
{
    if (verbose) puts("STUB: CGGStateGetClipRect called");
    return NULL;
}

void* CGGStateGetClipStack(void)
{
    if (verbose) puts("STUB: CGGStateGetClipStack called");
    return NULL;
}

void* CGGStateGetCompositeOperation(void)
{
    if (verbose) puts("STUB: CGGStateGetCompositeOperation called");
    return NULL;
}

void* CGGStateGetFillColor(void)
{
    if (verbose) puts("STUB: CGGStateGetFillColor called");
    return NULL;
}

void* CGGStateGetFillOverprint(void)
{
    if (verbose) puts("STUB: CGGStateGetFillOverprint called");
    return NULL;
}

void* CGGStateGetFlatness(void)
{
    if (verbose) puts("STUB: CGGStateGetFlatness called");
    return NULL;
}

void* CGGStateGetFont(void)
{
    if (verbose) puts("STUB: CGGStateGetFont called");
    return NULL;
}

void* CGGStateGetFontAntialiasingStyle(void)
{
    if (verbose) puts("STUB: CGGStateGetFontAntialiasingStyle called");
    return NULL;
}

void* CGGStateGetFontDilation(void)
{
    if (verbose) puts("STUB: CGGStateGetFontDilation called");
    return NULL;
}

void* CGGStateGetFontRenderingStyle(void)
{
    if (verbose) puts("STUB: CGGStateGetFontRenderingStyle called");
    return NULL;
}

void* CGGStateGetFontSize(void)
{
    if (verbose) puts("STUB: CGGStateGetFontSize called");
    return NULL;
}

void* CGGStateGetFontSmoothingBackgroundColor(void)
{
    if (verbose) puts("STUB: CGGStateGetFontSmoothingBackgroundColor called");
    return NULL;
}

void* CGGStateGetFontSmoothingContrast(void)
{
    if (verbose) puts("STUB: CGGStateGetFontSmoothingContrast called");
    return NULL;
}

void* CGGStateGetFontSmoothingStyle(void)
{
    if (verbose) puts("STUB: CGGStateGetFontSmoothingStyle called");
    return NULL;
}

void* CGGStateGetFontSubpixelQuantizationLevel(void)
{
    if (verbose) puts("STUB: CGGStateGetFontSubpixelQuantizationLevel called");
    return NULL;
}

void* CGGStateGetFontSubpixelQuantizationStyle(void)
{
    if (verbose) puts("STUB: CGGStateGetFontSubpixelQuantizationStyle called");
    return NULL;
}

void* CGGStateGetImageRenderingIntent(void)
{
    if (verbose) puts("STUB: CGGStateGetImageRenderingIntent called");
    return NULL;
}

void* CGGStateGetInterpolationQuality(void)
{
    if (verbose) puts("STUB: CGGStateGetInterpolationQuality called");
    return NULL;
}

void* CGGStateGetLineCap(void)
{
    if (verbose) puts("STUB: CGGStateGetLineCap called");
    return NULL;
}

void* CGGStateGetLineDash(void)
{
    if (verbose) puts("STUB: CGGStateGetLineDash called");
    return NULL;
}

void* CGGStateGetLineJoin(void)
{
    if (verbose) puts("STUB: CGGStateGetLineJoin called");
    return NULL;
}

void* CGGStateGetLineWidth(void)
{
    if (verbose) puts("STUB: CGGStateGetLineWidth called");
    return NULL;
}

void* CGGStateGetLineWidthThreshold(void)
{
    if (verbose) puts("STUB: CGGStateGetLineWidthThreshold called");
    return NULL;
}

void* CGGStateGetMiterLimit(void)
{
    if (verbose) puts("STUB: CGGStateGetMiterLimit called");
    return NULL;
}

void* CGGStateGetOverprintMode(void)
{
    if (verbose) puts("STUB: CGGStateGetOverprintMode called");
    return NULL;
}

void* CGGStateGetPatternPhase(void)
{
    if (verbose) puts("STUB: CGGStateGetPatternPhase called");
    return NULL;
}

void* CGGStateGetRenderingIntent(void)
{
    if (verbose) puts("STUB: CGGStateGetRenderingIntent called");
    return NULL;
}

void* CGGStateGetShouldAntialias(void)
{
    if (verbose) puts("STUB: CGGStateGetShouldAntialias called");
    return NULL;
}

void* CGGStateGetShouldAntialiasFonts(void)
{
    if (verbose) puts("STUB: CGGStateGetShouldAntialiasFonts called");
    return NULL;
}

void* CGGStateGetShouldDrawBitmapRuns(void)
{
    if (verbose) puts("STUB: CGGStateGetShouldDrawBitmapRuns called");
    return NULL;
}

void* CGGStateGetShouldSmoothFonts(void)
{
    if (verbose) puts("STUB: CGGStateGetShouldSmoothFonts called");
    return NULL;
}

void* CGGStateGetShouldSubpixelPositionFonts(void)
{
    if (verbose) puts("STUB: CGGStateGetShouldSubpixelPositionFonts called");
    return NULL;
}

void* CGGStateGetShouldSubpixelQuantizeFonts(void)
{
    if (verbose) puts("STUB: CGGStateGetShouldSubpixelQuantizeFonts called");
    return NULL;
}

void* CGGStateGetSmoothness(void)
{
    if (verbose) puts("STUB: CGGStateGetSmoothness called");
    return NULL;
}

void* CGGStateGetSoftMask(void)
{
    if (verbose) puts("STUB: CGGStateGetSoftMask called");
    return NULL;
}

void* CGGStateGetStrokeAdjust(void)
{
    if (verbose) puts("STUB: CGGStateGetStrokeAdjust called");
    return NULL;
}

void* CGGStateGetStrokeColor(void)
{
    if (verbose) puts("STUB: CGGStateGetStrokeColor called");
    return NULL;
}

void* CGGStateGetStrokeOverprint(void)
{
    if (verbose) puts("STUB: CGGStateGetStrokeOverprint called");
    return NULL;
}

void* CGGStateGetStyle(void)
{
    if (verbose) puts("STUB: CGGStateGetStyle called");
    return NULL;
}

void* CGGStateGetTextDrawingMode(void)
{
    if (verbose) puts("STUB: CGGStateGetTextDrawingMode called");
    return NULL;
}

void* CGGStateGetTextGreekingThreshold(void)
{
    if (verbose) puts("STUB: CGGStateGetTextGreekingThreshold called");
    return NULL;
}

void* CGGStateGetTextKnockout(void)
{
    if (verbose) puts("STUB: CGGStateGetTextKnockout called");
    return NULL;
}

void* CGGStateGetTransferFunctions(void)
{
    if (verbose) puts("STUB: CGGStateGetTransferFunctions called");
    return NULL;
}

void* CGGStateGetUndercolorRemoval(void)
{
    if (verbose) puts("STUB: CGGStateGetUndercolorRemoval called");
    return NULL;
}

void* CGGStateRelease(void)
{
    if (verbose) puts("STUB: CGGStateRelease called");
    return NULL;
}

void* CGGStateResetClip(void)
{
    if (verbose) puts("STUB: CGGStateResetClip called");
    return NULL;
}

void* CGGStateRetain(void)
{
    if (verbose) puts("STUB: CGGStateRetain called");
    return NULL;
}

void* CGGStateSetAlpha(void)
{
    if (verbose) puts("STUB: CGGStateSetAlpha called");
    return NULL;
}

void* CGGStateSetCTM(void)
{
    if (verbose) puts("STUB: CGGStateSetCTM called");
    return NULL;
}

void* CGGStateSetCompositeOperation(void)
{
    if (verbose) puts("STUB: CGGStateSetCompositeOperation called");
    return NULL;
}

void* CGGStateSetFillColor(void)
{
    if (verbose) puts("STUB: CGGStateSetFillColor called");
    return NULL;
}

void* CGGStateSetFontDilation(void)
{
    if (verbose) puts("STUB: CGGStateSetFontDilation called");
    return NULL;
}

void* CGGStateSetPatternPhase(void)
{
    if (verbose) puts("STUB: CGGStateSetPatternPhase called");
    return NULL;
}

void* CGGStateSetRenderingIntent(void)
{
    if (verbose) puts("STUB: CGGStateSetRenderingIntent called");
    return NULL;
}

void* CGGStateSetSoftMask(void)
{
    if (verbose) puts("STUB: CGGStateSetSoftMask called");
    return NULL;
}

void* CGGStateSetStrokeColor(void)
{
    if (verbose) puts("STUB: CGGStateSetStrokeColor called");
    return NULL;
}

void* CGGStateSetStyle(void)
{
    if (verbose) puts("STUB: CGGStateSetStyle called");
    return NULL;
}

void* CGGetMD5DigestOfBytes(void)
{
    if (verbose) puts("STUB: CGGetMD5DigestOfBytes called");
    return NULL;
}

void* CGGlyphBitmapCreate(void)
{
    if (verbose) puts("STUB: CGGlyphBitmapCreate called");
    return NULL;
}

void* CGGlyphBitmapCreateWithPath(void)
{
    if (verbose) puts("STUB: CGGlyphBitmapCreateWithPath called");
    return NULL;
}

void* CGGlyphBitmapCreateWithPathAndDilation(void)
{
    if (verbose) puts("STUB: CGGlyphBitmapCreateWithPathAndDilation called");
    return NULL;
}

void* CGGlyphBitmapRelease(void)
{
    if (verbose) puts("STUB: CGGlyphBitmapRelease called");
    return NULL;
}

void* CGGlyphBitmapRetain(void)
{
    if (verbose) puts("STUB: CGGlyphBitmapRetain called");
    return NULL;
}

void* CGGlyphBuilderCreate(void)
{
    if (verbose) puts("STUB: CGGlyphBuilderCreate called");
    return NULL;
}

void* CGGlyphBuilderGetQuantizationLevel(void)
{
    if (verbose) puts("STUB: CGGlyphBuilderGetQuantizationLevel called");
    return NULL;
}

void* CGGlyphBuilderLockBitmaps(void)
{
    if (verbose) puts("STUB: CGGlyphBuilderLockBitmaps called");
    return NULL;
}

void* CGGlyphBuilderRelease(void)
{
    if (verbose) puts("STUB: CGGlyphBuilderRelease called");
    return NULL;
}

void* CGGlyphBuilderUnlockBitmaps(void)
{
    if (verbose) puts("STUB: CGGlyphBuilderUnlockBitmaps called");
    return NULL;
}

void* CGGlyphLockAccess(void)
{
    if (verbose) puts("STUB: CGGlyphLockAccess called");
    return NULL;
}

void* CGGlyphLockAccessCustomized(void)
{
    if (verbose) puts("STUB: CGGlyphLockAccessCustomized called");
    return NULL;
}

void* CGGlyphLockCreate(void)
{
    if (verbose) puts("STUB: CGGlyphLockCreate called");
    return NULL;
}

void* CGGlyphLockGetQuantizationLevel(void)
{
    if (verbose) puts("STUB: CGGlyphLockGetQuantizationLevel called");
    return NULL;
}

void* CGGlyphLockLockGlyphBitmaps(void)
{
    if (verbose) puts("STUB: CGGlyphLockLockGlyphBitmaps called");
    return NULL;
}

void* CGGlyphLockRelease(void)
{
    if (verbose) puts("STUB: CGGlyphLockRelease called");
    return NULL;
}

void* CGGlyphLockUnlock(void)
{
    if (verbose) puts("STUB: CGGlyphLockUnlock called");
    return NULL;
}

void* CGGradientApply(void)
{
    if (verbose) puts("STUB: CGGradientApply called");
    return NULL;
}

void* CGGradientApplyWithBlock(void)
{
    if (verbose) puts("STUB: CGGradientApplyWithBlock called");
    return NULL;
}

void* CGGradientCreateWithColorComponents(void)
{
    if (verbose) puts("STUB: CGGradientCreateWithColorComponents called");
    return NULL;
}

void* CGGradientCreateWithColors(void)
{
    if (verbose) puts("STUB: CGGradientCreateWithColors called");
    return NULL;
}

void* CGGradientGetColorSpace(void)
{
    if (verbose) puts("STUB: CGGradientGetColorSpace called");
    return NULL;
}

void* CGGradientGetFunction(void)
{
    if (verbose) puts("STUB: CGGradientGetFunction called");
    return NULL;
}

void* CGGradientGetIdentifier(void)
{
    if (verbose) puts("STUB: CGGradientGetIdentifier called");
    return NULL;
}

void* CGGradientGetLocationCount(void)
{
    if (verbose) puts("STUB: CGGradientGetLocationCount called");
    return NULL;
}

void* CGGradientGetTypeID(void)
{
    if (verbose) puts("STUB: CGGradientGetTypeID called");
    return NULL;
}

void* CGGradientPrint(void)
{
    if (verbose) puts("STUB: CGGradientPrint called");
    return NULL;
}

void* CGGradientRelease(void)
{
    if (verbose) puts("STUB: CGGradientRelease called");
    return NULL;
}

void* CGGradientRetain(void)
{
    if (verbose) puts("STUB: CGGradientRetain called");
    return NULL;
}

void* CGIOSurfaceContextCreate(void)
{
    if (verbose) puts("STUB: CGIOSurfaceContextCreate called");
    return NULL;
}

void* CGIOSurfaceContextCreateImage(void)
{
    if (verbose) puts("STUB: CGIOSurfaceContextCreateImage called");
    return NULL;
}

void* CGIOSurfaceContextCreateImageReference(void)
{
    if (verbose) puts("STUB: CGIOSurfaceContextCreateImageReference called");
    return NULL;
}

void* CGIOSurfaceContextGetBitmapInfo(void)
{
    if (verbose) puts("STUB: CGIOSurfaceContextGetBitmapInfo called");
    return NULL;
}

void* CGIOSurfaceContextGetBitsPerComponent(void)
{
    if (verbose) puts("STUB: CGIOSurfaceContextGetBitsPerComponent called");
    return NULL;
}

void* CGIOSurfaceContextGetBitsPerPixel(void)
{
    if (verbose) puts("STUB: CGIOSurfaceContextGetBitsPerPixel called");
    return NULL;
}

void* CGIOSurfaceContextGetColorSpace(void)
{
    if (verbose) puts("STUB: CGIOSurfaceContextGetColorSpace called");
    return NULL;
}

void* CGIOSurfaceContextGetDisplayMask(void)
{
    if (verbose) puts("STUB: CGIOSurfaceContextGetDisplayMask called");
    return NULL;
}

void* CGIOSurfaceContextGetHeight(void)
{
    if (verbose) puts("STUB: CGIOSurfaceContextGetHeight called");
    return NULL;
}

void* CGIOSurfaceContextGetSizeLimits(void)
{
    if (verbose) puts("STUB: CGIOSurfaceContextGetSizeLimits called");
    return NULL;
}

void* CGIOSurfaceContextGetSurface(void)
{
    if (verbose) puts("STUB: CGIOSurfaceContextGetSurface called");
    return NULL;
}

void* CGIOSurfaceContextGetWidth(void)
{
    if (verbose) puts("STUB: CGIOSurfaceContextGetWidth called");
    return NULL;
}

void* CGIOSurfaceContextSetDisplayMask(void)
{
    if (verbose) puts("STUB: CGIOSurfaceContextSetDisplayMask called");
    return NULL;
}

void* CGImageBlockCreate(void)
{
    if (verbose) puts("STUB: CGImageBlockCreate called");
    return NULL;
}

void* CGImageBlockGetBytesPerRow(void)
{
    if (verbose) puts("STUB: CGImageBlockGetBytesPerRow called");
    return NULL;
}

void* CGImageBlockGetData(void)
{
    if (verbose) puts("STUB: CGImageBlockGetData called");
    return NULL;
}

void* CGImageBlockGetRect(void)
{
    if (verbose) puts("STUB: CGImageBlockGetRect called");
    return NULL;
}

void* CGImageBlockRelease(void)
{
    if (verbose) puts("STUB: CGImageBlockRelease called");
    return NULL;
}

void* CGImageBlockSetCreate(void)
{
    if (verbose) puts("STUB: CGImageBlockSetCreate called");
    return NULL;
}

void* CGImageBlockSetCreateWithType(void)
{
    if (verbose) puts("STUB: CGImageBlockSetCreateWithType called");
    return NULL;
}

void* CGImageBlockSetGetColorSpace(void)
{
    if (verbose) puts("STUB: CGImageBlockSetGetColorSpace called");
    return NULL;
}

void* CGImageBlockSetGetComponentType(void)
{
    if (verbose) puts("STUB: CGImageBlockSetGetComponentType called");
    return NULL;
}

void* CGImageBlockSetGetCount(void)
{
    if (verbose) puts("STUB: CGImageBlockSetGetCount called");
    return NULL;
}

void* CGImageBlockSetGetImageBlock(void)
{
    if (verbose) puts("STUB: CGImageBlockSetGetImageBlock called");
    return NULL;
}

void* CGImageBlockSetGetPixelSize(void)
{
    if (verbose) puts("STUB: CGImageBlockSetGetPixelSize called");
    return NULL;
}

void* CGImageBlockSetGetProperty(void)
{
    if (verbose) puts("STUB: CGImageBlockSetGetProperty called");
    return NULL;
}

void* CGImageBlockSetGetRect(void)
{
    if (verbose) puts("STUB: CGImageBlockSetGetRect called");
    return NULL;
}

void* CGImageBlockSetGetSize(void)
{
    if (verbose) puts("STUB: CGImageBlockSetGetSize called");
    return NULL;
}

void* CGImageBlockSetRelease(void)
{
    if (verbose) puts("STUB: CGImageBlockSetRelease called");
    return NULL;
}

void* CGImageBlockSetRetain(void)
{
    if (verbose) puts("STUB: CGImageBlockSetRetain called");
    return NULL;
}

void* CGImageBlockSetSetProperty(void)
{
    if (verbose) puts("STUB: CGImageBlockSetSetProperty called");
    return NULL;
}

void* CGImageCopyProperty(void)
{
    if (verbose) puts("STUB: CGImageCopyProperty called");
    return NULL;
}

void* CGImageCreate(void)
{
    if (verbose) puts("STUB: CGImageCreate called");
    return NULL;
}

void* CGImageCreateCopy(void)
{
    if (verbose) puts("STUB: CGImageCreateCopy called");
    return NULL;
}

void* CGImageCreateCopyWithClipPath(void)
{
    if (verbose) puts("STUB: CGImageCreateCopyWithClipPath called");
    return NULL;
}

void* CGImageCreateCopyWithColorSpace(void)
{
    if (verbose) puts("STUB: CGImageCreateCopyWithColorSpace called");
    return NULL;
}

void* CGImageCreateCopyWithOptions(void)
{
    if (verbose) puts("STUB: CGImageCreateCopyWithOptions called");
    return NULL;
}

void* CGImageCreateCopyWithRenderingIntent(void)
{
    if (verbose) puts("STUB: CGImageCreateCopyWithRenderingIntent called");
    return NULL;
}

void* CGImageCreateWithCGPDFStream(void)
{
    if (verbose) puts("STUB: CGImageCreateWithCGPDFStream called");
    return NULL;
}

void* CGImageCreateWithCoreImage(void)
{
    if (verbose) puts("STUB: CGImageCreateWithCoreImage called");
    return NULL;
}

void* CGImageCreateWithEPSDataProvider(void)
{
    if (verbose) puts("STUB: CGImageCreateWithEPSDataProvider called");
    return NULL;
}

void* CGImageCreateWithImageInRect(void)
{
    if (verbose) puts("STUB: CGImageCreateWithImageInRect called");
    return NULL;
}

void* CGImageCreateWithImageProvider(void)
{
    if (verbose) puts("STUB: CGImageCreateWithImageProvider called");
    return NULL;
}

void* CGImageCreateWithJPEGDataProvider(void)
{
    if (verbose) puts("STUB: CGImageCreateWithJPEGDataProvider called");
    return NULL;
}

void* CGImageCreateWithJPEGDataProvider2(void)
{
    if (verbose) puts("STUB: CGImageCreateWithJPEGDataProvider2 called");
    return NULL;
}

void* CGImageCreateWithMask(void)
{
    if (verbose) puts("STUB: CGImageCreateWithMask called");
    return NULL;
}

void* CGImageCreateWithMaskAndMatte(void)
{
    if (verbose) puts("STUB: CGImageCreateWithMaskAndMatte called");
    return NULL;
}

void* CGImageCreateWithMaskingColor(void)
{
    if (verbose) puts("STUB: CGImageCreateWithMaskingColor called");
    return NULL;
}

void* CGImageCreateWithMaskingColors(void)
{
    if (verbose) puts("STUB: CGImageCreateWithMaskingColors called");
    return NULL;
}

void* CGImageCreateWithPNGDataProvider(void)
{
    if (verbose) puts("STUB: CGImageCreateWithPNGDataProvider called");
    return NULL;
}

void* CGImageDumpToFile(void)
{
    if (verbose) puts("STUB: CGImageDumpToFile called");
    return NULL;
}

void* CGImageEPSRepCreate(void)
{
    if (verbose) puts("STUB: CGImageEPSRepCreate called");
    return NULL;
}

void* CGImageEPSRepGetAlternateImage(void)
{
    if (verbose) puts("STUB: CGImageEPSRepGetAlternateImage called");
    return NULL;
}

void* CGImageEPSRepGetBBox(void)
{
    if (verbose) puts("STUB: CGImageEPSRepGetBBox called");
    return NULL;
}

void* CGImageEPSRepGetSource(void)
{
    if (verbose) puts("STUB: CGImageEPSRepGetSource called");
    return NULL;
}

void* CGImageEPSRepRelease(void)
{
    if (verbose) puts("STUB: CGImageEPSRepRelease called");
    return NULL;
}

void* CGImageEPSRepRetain(void)
{
    if (verbose) puts("STUB: CGImageEPSRepRetain called");
    return NULL;
}

void* CGImageGetAlphaInfo(void)
{
    if (verbose) puts("STUB: CGImageGetAlphaInfo called");
    return NULL;
}

void* CGImageGetBitmapInfo(void)
{
    if (verbose) puts("STUB: CGImageGetBitmapInfo called");
    return NULL;
}

void* CGImageGetBitsPerComponent(void)
{
    if (verbose) puts("STUB: CGImageGetBitsPerComponent called");
    return NULL;
}

void* CGImageGetBitsPerPixel(void)
{
    if (verbose) puts("STUB: CGImageGetBitsPerPixel called");
    return NULL;
}

void* CGImageGetByteOrderInfo(void)
{
    if (verbose) puts("STUB: CGImageGetByteOrderInfo called");
    return NULL;
}

void* CGImageGetBytesPerRow(void)
{
    if (verbose) puts("STUB: CGImageGetBytesPerRow called");
    return NULL;
}

void* CGImageGetCachingFlags(void)
{
    if (verbose) puts("STUB: CGImageGetCachingFlags called");
    return NULL;
}

void* CGImageGetClipPath(void)
{
    if (verbose) puts("STUB: CGImageGetClipPath called");
    return NULL;
}

void* CGImageGetColorSpace(void)
{
    if (verbose) puts("STUB: CGImageGetColorSpace called");
    return NULL;
}

void* CGImageGetData(void)
{
    if (verbose) puts("STUB: CGImageGetData called");
    return NULL;
}

void* CGImageGetDataProvider(void)
{
    if (verbose) puts("STUB: CGImageGetDataProvider called");
    return NULL;
}

void* CGImageGetDecode(void)
{
    if (verbose) puts("STUB: CGImageGetDecode called");
    return NULL;
}

void* CGImageGetEPSRep(void)
{
    if (verbose) puts("STUB: CGImageGetEPSRep called");
    return NULL;
}

void* CGImageGetHeight(void)
{
    if (verbose) puts("STUB: CGImageGetHeight called");
    return NULL;
}

void* CGImageGetIdentifier(void)
{
    if (verbose) puts("STUB: CGImageGetIdentifier called");
    return NULL;
}

void* CGImageGetImageProvider(void)
{
    if (verbose) puts("STUB: CGImageGetImageProvider called");
    return NULL;
}

void* CGImageGetMask(void)
{
    if (verbose) puts("STUB: CGImageGetMask called");
    return NULL;
}

void* CGImageGetMaskingColors(void)
{
    if (verbose) puts("STUB: CGImageGetMaskingColors called");
    return NULL;
}

void* CGImageGetMatte(void)
{
    if (verbose) puts("STUB: CGImageGetMatte called");
    return NULL;
}

void* CGImageGetPixelFormatInfo(void)
{
    if (verbose) puts("STUB: CGImageGetPixelFormatInfo called");
    return NULL;
}

void* CGImageGetProperty(void)
{
    if (verbose) puts("STUB: CGImageGetProperty called");
    return NULL;
}

void* CGImageGetRenderingIntent(void)
{
    if (verbose) puts("STUB: CGImageGetRenderingIntent called");
    return NULL;
}

void* CGImageGetShouldInterpolate(void)
{
    if (verbose) puts("STUB: CGImageGetShouldInterpolate called");
    return NULL;
}

void* CGImageGetTypeID(void)
{
    if (verbose) puts("STUB: CGImageGetTypeID called");
    return NULL;
}

void* CGImageGetUTType(void)
{
    if (verbose) puts("STUB: CGImageGetUTType called");
    return NULL;
}

void* CGImageGetWidth(void)
{
    if (verbose) puts("STUB: CGImageGetWidth called");
    return NULL;
}

void* CGImageHasAlpha(void)
{
    if (verbose) puts("STUB: CGImageHasAlpha called");
    return NULL;
}

void* CGImageIOSurfaceSetCreate(void)
{
    if (verbose) puts("STUB: CGImageIOSurfaceSetCreate called");
    return NULL;
}

void* CGImageIOSurfaceSetGetColorSpace(void)
{
    if (verbose) puts("STUB: CGImageIOSurfaceSetGetColorSpace called");
    return NULL;
}

void* CGImageIOSurfaceSetGetComponentType(void)
{
    if (verbose) puts("STUB: CGImageIOSurfaceSetGetComponentType called");
    return NULL;
}

void* CGImageIOSurfaceSetGetCount(void)
{
    if (verbose) puts("STUB: CGImageIOSurfaceSetGetCount called");
    return NULL;
}

void* CGImageIOSurfaceSetGetPixelSize(void)
{
    if (verbose) puts("STUB: CGImageIOSurfaceSetGetPixelSize called");
    return NULL;
}

void* CGImageIOSurfaceSetGetProperty(void)
{
    if (verbose) puts("STUB: CGImageIOSurfaceSetGetProperty called");
    return NULL;
}

void* CGImageIOSurfaceSetGetRect(void)
{
    if (verbose) puts("STUB: CGImageIOSurfaceSetGetRect called");
    return NULL;
}

void* CGImageIOSurfaceSetGetSize(void)
{
    if (verbose) puts("STUB: CGImageIOSurfaceSetGetSize called");
    return NULL;
}

void* CGImageIOSurfaceSetGetTypeID(void)
{
    if (verbose) puts("STUB: CGImageIOSurfaceSetGetTypeID called");
    return NULL;
}

void* CGImageIOSurfaceSetRelease(void)
{
    if (verbose) puts("STUB: CGImageIOSurfaceSetRelease called");
    return NULL;
}

void* CGImageIOSurfaceSetRetain(void)
{
    if (verbose) puts("STUB: CGImageIOSurfaceSetRetain called");
    return NULL;
}

void* CGImageIOSurfaceSetSetProperty(void)
{
    if (verbose) puts("STUB: CGImageIOSurfaceSetSetProperty called");
    return NULL;
}

void* CGImageIsMask(void)
{
    if (verbose) puts("STUB: CGImageIsMask called");
    return NULL;
}

void* CGImageIsOptimizedForSharing(void)
{
    if (verbose) puts("STUB: CGImageIsOptimizedForSharing called");
    return NULL;
}

void* CGImageIsSubimage(void)
{
    if (verbose) puts("STUB: CGImageIsSubimage called");
    return NULL;
}

void* CGImageMaskCreate(void)
{
    if (verbose) puts("STUB: CGImageMaskCreate called");
    return NULL;
}

void* CGImageNotificationCenter(void)
{
    if (verbose) puts("STUB: CGImageNotificationCenter called");
    return NULL;
}

void* CGImageProviderCopyIOSurface(void)
{
    if (verbose) puts("STUB: CGImageProviderCopyIOSurface called");
    return NULL;
}

void* CGImageProviderCopyIOSurfaceSet(void)
{
    if (verbose) puts("STUB: CGImageProviderCopyIOSurfaceSet called");
    return NULL;
}

void* CGImageProviderCopyImageBlockSet(void)
{
    if (verbose) puts("STUB: CGImageProviderCopyImageBlockSet called");
    return NULL;
}

void* CGImageProviderCopyImageBlockSetWithOptions(void)
{
    if (verbose) puts("STUB: CGImageProviderCopyImageBlockSetWithOptions called");
    return NULL;
}

void* CGImageProviderCopyImageTextureData(void)
{
    if (verbose) puts("STUB: CGImageProviderCopyImageTextureData called");
    return NULL;
}

void* CGImageProviderCreate(void)
{
    if (verbose) puts("STUB: CGImageProviderCreate called");
    return NULL;
}

void* CGImageProviderGetAlphaInfo(void)
{
    if (verbose) puts("STUB: CGImageProviderGetAlphaInfo called");
    return NULL;
}

void* CGImageProviderGetCallbackVersion(void)
{
    if (verbose) puts("STUB: CGImageProviderGetCallbackVersion called");
    return NULL;
}

void* CGImageProviderGetColorSpace(void)
{
    if (verbose) puts("STUB: CGImageProviderGetColorSpace called");
    return NULL;
}

void* CGImageProviderGetComponentType(void)
{
    if (verbose) puts("STUB: CGImageProviderGetComponentType called");
    return NULL;
}

void* CGImageProviderGetInfo(void)
{
    if (verbose) puts("STUB: CGImageProviderGetInfo called");
    return NULL;
}

void* CGImageProviderGetPixelSize(void)
{
    if (verbose) puts("STUB: CGImageProviderGetPixelSize called");
    return NULL;
}

void* CGImageProviderGetProperty(void)
{
    if (verbose) puts("STUB: CGImageProviderGetProperty called");
    return NULL;
}

void* CGImageProviderGetSize(void)
{
    if (verbose) puts("STUB: CGImageProviderGetSize called");
    return NULL;
}

void* CGImageProviderGetTypeID(void)
{
    if (verbose) puts("STUB: CGImageProviderGetTypeID called");
    return NULL;
}

void* CGImageProviderRelease(void)
{
    if (verbose) puts("STUB: CGImageProviderRelease called");
    return NULL;
}

void* CGImageProviderRetain(void)
{
    if (verbose) puts("STUB: CGImageProviderRetain called");
    return NULL;
}

void* CGImageProviderSetProperty(void)
{
    if (verbose) puts("STUB: CGImageProviderSetProperty called");
    return NULL;
}

void* CGImageRelease(void)
{
    if (verbose) puts("STUB: CGImageRelease called");
    return NULL;
}

void* CGImageRetain(void)
{
    if (verbose) puts("STUB: CGImageRetain called");
    return NULL;
}

void* CGImageSetCachingFlags(void)
{
    if (verbose) puts("STUB: CGImageSetCachingFlags called");
    return NULL;
}

void* CGImageSetProperty(void)
{
    if (verbose) puts("STUB: CGImageSetProperty called");
    return NULL;
}

void* CGImageSetSharedIdentifier(void)
{
    if (verbose) puts("STUB: CGImageSetSharedIdentifier called");
    return NULL;
}

void* CGImageSurfaceSetGetIOSurface(void)
{
    if (verbose) puts("STUB: CGImageSurfaceSetGetIOSurface called");
    return NULL;
}

void* CGImageTextureDataCopyProperties(void)
{
    if (verbose) puts("STUB: CGImageTextureDataCopyProperties called");
    return NULL;
}

void* CGImageTextureDataCreate(void)
{
    if (verbose) puts("STUB: CGImageTextureDataCreate called");
    return NULL;
}

void* CGImageTextureDataGetColorSpace(void)
{
    if (verbose) puts("STUB: CGImageTextureDataGetColorSpace called");
    return NULL;
}

void* CGImageTextureDataGetDataWithBlock(void)
{
    if (verbose) puts("STUB: CGImageTextureDataGetDataWithBlock called");
    return NULL;
}

void* CGImageTextureDataGetNumberOfFaces(void)
{
    if (verbose) puts("STUB: CGImageTextureDataGetNumberOfFaces called");
    return NULL;
}

void* CGImageTextureDataGetNumberOfMipmaps(void)
{
    if (verbose) puts("STUB: CGImageTextureDataGetNumberOfMipmaps called");
    return NULL;
}

void* CGImageTextureDataGetOffsetWithBlock(void)
{
    if (verbose) puts("STUB: CGImageTextureDataGetOffsetWithBlock called");
    return NULL;
}

void* CGImageTextureDataGetPixelFormat(void)
{
    if (verbose) puts("STUB: CGImageTextureDataGetPixelFormat called");
    return NULL;
}

void* CGImageTextureDataGetProperty(void)
{
    if (verbose) puts("STUB: CGImageTextureDataGetProperty called");
    return NULL;
}

void* CGImageTextureDataGetSize(void)
{
    if (verbose) puts("STUB: CGImageTextureDataGetSize called");
    return NULL;
}

void* CGImageTextureDataGetTypeID(void)
{
    if (verbose) puts("STUB: CGImageTextureDataGetTypeID called");
    return NULL;
}

void* CGImageTextureDataIsCompressed(void)
{
    if (verbose) puts("STUB: CGImageTextureDataIsCompressed called");
    return NULL;
}

void* CGImageTextureDataRelease(void)
{
    if (verbose) puts("STUB: CGImageTextureDataRelease called");
    return NULL;
}

void* CGImageTextureDataRetain(void)
{
    if (verbose) puts("STUB: CGImageTextureDataRetain called");
    return NULL;
}

void* CGImageTextureDataSetProperty(void)
{
    if (verbose) puts("STUB: CGImageTextureDataSetProperty called");
    return NULL;
}

void* CGImageTextureDataSupportsTiledLayout(void)
{
    if (verbose) puts("STUB: CGImageTextureDataSupportsTiledLayout called");
    return NULL;
}

void* CGImageWriteToFile(void)
{
    if (verbose) puts("STUB: CGImageWriteToFile called");
    return NULL;
}

void* CGKSeparationContextCreateWithDictionary(void)
{
    if (verbose) puts("STUB: CGKSeparationContextCreateWithDictionary called");
    return NULL;
}

void* CGKSeparationContextRelease(void)
{
    if (verbose) puts("STUB: CGKSeparationContextRelease called");
    return NULL;
}

void* CGLayerCreateWithContext(void)
{
    if (verbose) puts("STUB: CGLayerCreateWithContext called");
    return NULL;
}

void* CGLayerGetContext(void)
{
    if (verbose) puts("STUB: CGLayerGetContext called");
    return NULL;
}

void* CGLayerGetSize(void)
{
    if (verbose) puts("STUB: CGLayerGetSize called");
    return NULL;
}

void* CGLayerGetTypeID(void)
{
    if (verbose) puts("STUB: CGLayerGetTypeID called");
    return NULL;
}

void* CGLayerRelease(void)
{
    if (verbose) puts("STUB: CGLayerRelease called");
    return NULL;
}

void* CGLayerRetain(void)
{
    if (verbose) puts("STUB: CGLayerRetain called");
    return NULL;
}

void* CGLibraryLoadFunction(void)
{
    if (verbose) puts("STUB: CGLibraryLoadFunction called");
    return NULL;
}

void* CGNotificationCenterAddCallback(void)
{
    if (verbose) puts("STUB: CGNotificationCenterAddCallback called");
    return NULL;
}

void* CGNotificationCenterRemoveCallback(void)
{
    if (verbose) puts("STUB: CGNotificationCenterRemoveCallback called");
    return NULL;
}

void* CGPDFAdvancesCreate(void)
{
    if (verbose) puts("STUB: CGPDFAdvancesCreate called");
    return NULL;
}

void* CGPDFAdvancesGetHorizontalAdvance(void)
{
    if (verbose) puts("STUB: CGPDFAdvancesGetHorizontalAdvance called");
    return NULL;
}

void* CGPDFAdvancesGetVerticalAdvance(void)
{
    if (verbose) puts("STUB: CGPDFAdvancesGetVerticalAdvance called");
    return NULL;
}

void* CGPDFAdvancesPrint(void)
{
    if (verbose) puts("STUB: CGPDFAdvancesPrint called");
    return NULL;
}

void* CGPDFAdvancesRelease(void)
{
    if (verbose) puts("STUB: CGPDFAdvancesRelease called");
    return NULL;
}

void* CGPDFAdvancesValidate(void)
{
    if (verbose) puts("STUB: CGPDFAdvancesValidate called");
    return NULL;
}

void* CGPDFAnnotationCreate(void)
{
    if (verbose) puts("STUB: CGPDFAnnotationCreate called");
    return NULL;
}

void* CGPDFAnnotationCreateWithCGPDFDictionary(void)
{
    if (verbose) puts("STUB: CGPDFAnnotationCreateWithCGPDFDictionary called");
    return NULL;
}

void* CGPDFAnnotationGetBurnIn(void)
{
    if (verbose) puts("STUB: CGPDFAnnotationGetBurnIn called");
    return NULL;
}

void* CGPDFAnnotationGetCGPDFDictionary(void)
{
    if (verbose) puts("STUB: CGPDFAnnotationGetCGPDFDictionary called");
    return NULL;
}

void* CGPDFAnnotationGetDictionary(void)
{
    if (verbose) puts("STUB: CGPDFAnnotationGetDictionary called");
    return NULL;
}

void* CGPDFAnnotationGetHash(void)
{
    if (verbose) puts("STUB: CGPDFAnnotationGetHash called");
    return NULL;
}

void* CGPDFAnnotationGetRect(void)
{
    if (verbose) puts("STUB: CGPDFAnnotationGetRect called");
    return NULL;
}

void* CGPDFAnnotationGetTag(void)
{
    if (verbose) puts("STUB: CGPDFAnnotationGetTag called");
    return NULL;
}

void* CGPDFAnnotationGetType(void)
{
    if (verbose) puts("STUB: CGPDFAnnotationGetType called");
    return NULL;
}

void* CGPDFAnnotationGetTypeID(void)
{
    if (verbose) puts("STUB: CGPDFAnnotationGetTypeID called");
    return NULL;
}

void* CGPDFAnnotationIsGhost(void)
{
    if (verbose) puts("STUB: CGPDFAnnotationIsGhost called");
    return NULL;
}

void* CGPDFAnnotationIsPristine(void)
{
    if (verbose) puts("STUB: CGPDFAnnotationIsPristine called");
    return NULL;
}

void* CGPDFAnnotationIsSupportedType(void)
{
    if (verbose) puts("STUB: CGPDFAnnotationIsSupportedType called");
    return NULL;
}

void* CGPDFAnnotationIsVersioned(void)
{
    if (verbose) puts("STUB: CGPDFAnnotationIsVersioned called");
    return NULL;
}

void* CGPDFAnnotationSetAnnotationForKey(void)
{
    if (verbose) puts("STUB: CGPDFAnnotationSetAnnotationForKey called");
    return NULL;
}

void* CGPDFAnnotationSetBurnIn(void)
{
    if (verbose) puts("STUB: CGPDFAnnotationSetBurnIn called");
    return NULL;
}

void* CGPDFAnnotationSetIsGhost(void)
{
    if (verbose) puts("STUB: CGPDFAnnotationSetIsGhost called");
    return NULL;
}

void* CGPDFAnnotationSetTag(void)
{
    if (verbose) puts("STUB: CGPDFAnnotationSetTag called");
    return NULL;
}

void* CGPDFArrayApplyBlock(void)
{
    if (verbose) puts("STUB: CGPDFArrayApplyBlock called");
    return NULL;
}

void* CGPDFArrayGetArray(void)
{
    if (verbose) puts("STUB: CGPDFArrayGetArray called");
    return NULL;
}

void* CGPDFArrayGetBoolean(void)
{
    if (verbose) puts("STUB: CGPDFArrayGetBoolean called");
    return NULL;
}

void* CGPDFArrayGetCount(void)
{
    if (verbose) puts("STUB: CGPDFArrayGetCount called");
    return NULL;
}

void* CGPDFArrayGetDictionary(void)
{
    if (verbose) puts("STUB: CGPDFArrayGetDictionary called");
    return NULL;
}

void* CGPDFArrayGetInteger(void)
{
    if (verbose) puts("STUB: CGPDFArrayGetInteger called");
    return NULL;
}

void* CGPDFArrayGetIntegers(void)
{
    if (verbose) puts("STUB: CGPDFArrayGetIntegers called");
    return NULL;
}

void* CGPDFArrayGetName(void)
{
    if (verbose) puts("STUB: CGPDFArrayGetName called");
    return NULL;
}

void* CGPDFArrayGetNull(void)
{
    if (verbose) puts("STUB: CGPDFArrayGetNull called");
    return NULL;
}

void* CGPDFArrayGetNumber(void)
{
    if (verbose) puts("STUB: CGPDFArrayGetNumber called");
    return NULL;
}

void* CGPDFArrayGetNumbers(void)
{
    if (verbose) puts("STUB: CGPDFArrayGetNumbers called");
    return NULL;
}

void* CGPDFArrayGetObject(void)
{
    if (verbose) puts("STUB: CGPDFArrayGetObject called");
    return NULL;
}

void* CGPDFArrayGetOffset(void)
{
    if (verbose) puts("STUB: CGPDFArrayGetOffset called");
    return NULL;
}

void* CGPDFArrayGetStream(void)
{
    if (verbose) puts("STUB: CGPDFArrayGetStream called");
    return NULL;
}

void* CGPDFArrayGetString(void)
{
    if (verbose) puts("STUB: CGPDFArrayGetString called");
    return NULL;
}

void* CGPDFArrayGetUnresolvedObject(void)
{
    if (verbose) puts("STUB: CGPDFArrayGetUnresolvedObject called");
    return NULL;
}

void* CGPDFArrayPrint(void)
{
    if (verbose) puts("STUB: CGPDFArrayPrint called");
    return NULL;
}

void* CGPDFArrayPrintWithIndent(void)
{
    if (verbose) puts("STUB: CGPDFArrayPrintWithIndent called");
    return NULL;
}

void* CGPDFArrayRelease(void)
{
    if (verbose) puts("STUB: CGPDFArrayRelease called");
    return NULL;
}

void* CGPDFArrayRetain(void)
{
    if (verbose) puts("STUB: CGPDFArrayRetain called");
    return NULL;
}

void* CGPDFBase14GetBase14Name(void)
{
    if (verbose) puts("STUB: CGPDFBase14GetBase14Name called");
    return NULL;
}

void* CGPDFBase14GetInfo(void)
{
    if (verbose) puts("STUB: CGPDFBase14GetInfo called");
    return NULL;
}

void* CGPDFBase14InfoGetAdvances(void)
{
    if (verbose) puts("STUB: CGPDFBase14InfoGetAdvances called");
    return NULL;
}

void* CGPDFCMapCreate(void)
{
    if (verbose) puts("STUB: CGPDFCMapCreate called");
    return NULL;
}

void* CGPDFCMapCreateWithName(void)
{
    if (verbose) puts("STUB: CGPDFCMapCreateWithName called");
    return NULL;
}

void* CGPDFCMapGetCIDsForText(void)
{
    if (verbose) puts("STUB: CGPDFCMapGetCIDsForText called");
    return NULL;
}

void* CGPDFCMapGetMaxUnicodes(void)
{
    if (verbose) puts("STUB: CGPDFCMapGetMaxUnicodes called");
    return NULL;
}

void* CGPDFCMapGetName(void)
{
    if (verbose) puts("STUB: CGPDFCMapGetName called");
    return NULL;
}

void* CGPDFCMapGetROS(void)
{
    if (verbose) puts("STUB: CGPDFCMapGetROS called");
    return NULL;
}

void* CGPDFCMapGetUnicodeCMap(void)
{
    if (verbose) puts("STUB: CGPDFCMapGetUnicodeCMap called");
    return NULL;
}

void* CGPDFCMapGetUnicodesForIndex(void)
{
    if (verbose) puts("STUB: CGPDFCMapGetUnicodesForIndex called");
    return NULL;
}

void* CGPDFCMapGetZapfTable(void)
{
    if (verbose) puts("STUB: CGPDFCMapGetZapfTable called");
    return NULL;
}

void* CGPDFCMapIsHorizontal(void)
{
    if (verbose) puts("STUB: CGPDFCMapIsHorizontal called");
    return NULL;
}

void* CGPDFCMapRelease(void)
{
    if (verbose) puts("STUB: CGPDFCMapRelease called");
    return NULL;
}

void* CGPDFContentStreamCreateResolvedColor(void)
{
    if (verbose) puts("STUB: CGPDFContentStreamCreateResolvedColor called");
    return NULL;
}

void* CGPDFContentStreamCreateResolvedColorSpace(void)
{
    if (verbose) puts("STUB: CGPDFContentStreamCreateResolvedColorSpace called");
    return NULL;
}

void* CGPDFContentStreamCreateWithPage(void)
{
    if (verbose) puts("STUB: CGPDFContentStreamCreateWithPage called");
    return NULL;
}

void* CGPDFContentStreamCreateWithStream(void)
{
    if (verbose) puts("STUB: CGPDFContentStreamCreateWithStream called");
    return NULL;
}

void* CGPDFContentStreamGetColorSpace(void)
{
    if (verbose) puts("STUB: CGPDFContentStreamGetColorSpace called");
    return NULL;
}

void* CGPDFContentStreamGetExtGState(void)
{
    if (verbose) puts("STUB: CGPDFContentStreamGetExtGState called");
    return NULL;
}

void* CGPDFContentStreamGetFont(void)
{
    if (verbose) puts("STUB: CGPDFContentStreamGetFont called");
    return NULL;
}

void* CGPDFContentStreamGetPattern(void)
{
    if (verbose) puts("STUB: CGPDFContentStreamGetPattern called");
    return NULL;
}

void* CGPDFContentStreamGetPropertyList(void)
{
    if (verbose) puts("STUB: CGPDFContentStreamGetPropertyList called");
    return NULL;
}

void* CGPDFContentStreamGetResource(void)
{
    if (verbose) puts("STUB: CGPDFContentStreamGetResource called");
    return NULL;
}

void* CGPDFContentStreamGetShading(void)
{
    if (verbose) puts("STUB: CGPDFContentStreamGetShading called");
    return NULL;
}

void* CGPDFContentStreamGetStreams(void)
{
    if (verbose) puts("STUB: CGPDFContentStreamGetStreams called");
    return NULL;
}

void* CGPDFContentStreamGetXObject(void)
{
    if (verbose) puts("STUB: CGPDFContentStreamGetXObject called");
    return NULL;
}

void* CGPDFContentStreamRelease(void)
{
    if (verbose) puts("STUB: CGPDFContentStreamRelease called");
    return NULL;
}

void* CGPDFContentStreamRetain(void)
{
    if (verbose) puts("STUB: CGPDFContentStreamRetain called");
    return NULL;
}

void* CGPDFContextAddAnnotation(void)
{
    if (verbose) puts("STUB: CGPDFContextAddAnnotation called");
    return NULL;
}

void* CGPDFContextAddCatalogEntry(void)
{
    if (verbose) puts("STUB: CGPDFContextAddCatalogEntry called");
    return NULL;
}

void* CGPDFContextAddDestinationAtPoint(void)
{
    if (verbose) puts("STUB: CGPDFContextAddDestinationAtPoint called");
    return NULL;
}

void* CGPDFContextAddDocumentMetadata(void)
{
    if (verbose) puts("STUB: CGPDFContextAddDocumentMetadata called");
    return NULL;
}

void* CGPDFContextAddPDFXInfo(void)
{
    if (verbose) puts("STUB: CGPDFContextAddPDFXInfo called");
    return NULL;
}

void* CGPDFContextBeginPage(void)
{
    if (verbose) puts("STUB: CGPDFContextBeginPage called");
    return NULL;
}

void* CGPDFContextBeginTag(void)
{
    if (verbose) puts("STUB: CGPDFContextBeginTag called");
    return NULL;
}

void* CGPDFContextClose(void)
{
    if (verbose) puts("STUB: CGPDFContextClose called");
    return NULL;
}

void* CGPDFContextCreate(void)
{
    if (verbose) puts("STUB: CGPDFContextCreate called");
    return NULL;
}

void* CGPDFContextCreateWithFilename(void)
{
    if (verbose) puts("STUB: CGPDFContextCreateWithFilename called");
    return NULL;
}

void* CGPDFContextCreateWithURL(void)
{
    if (verbose) puts("STUB: CGPDFContextCreateWithURL called");
    return NULL;
}

void* CGPDFContextEndPage(void)
{
    if (verbose) puts("STUB: CGPDFContextEndPage called");
    return NULL;
}

void* CGPDFContextEndTag(void)
{
    if (verbose) puts("STUB: CGPDFContextEndTag called");
    return NULL;
}

void* CGPDFContextSetDestinationForRect(void)
{
    if (verbose) puts("STUB: CGPDFContextSetDestinationForRect called");
    return NULL;
}

void* CGPDFContextSetImageTag(void)
{
    if (verbose) puts("STUB: CGPDFContextSetImageTag called");
    return NULL;
}

void* CGPDFContextSetOutline(void)
{
    if (verbose) puts("STUB: CGPDFContextSetOutline called");
    return NULL;
}

void* CGPDFContextSetOutputIntent(void)
{
    if (verbose) puts("STUB: CGPDFContextSetOutputIntent called");
    return NULL;
}

void* CGPDFContextSetURLForRect(void)
{
    if (verbose) puts("STUB: CGPDFContextSetURLForRect called");
    return NULL;
}

void* CGPDFDictionaryApplyBlock(void)
{
    if (verbose) puts("STUB: CGPDFDictionaryApplyBlock called");
    return NULL;
}

void* CGPDFDictionaryApplyFunction(void)
{
    if (verbose) puts("STUB: CGPDFDictionaryApplyFunction called");
    return NULL;
}

void* CGPDFDictionaryApplyFunctionRaw(void)
{
    if (verbose) puts("STUB: CGPDFDictionaryApplyFunctionRaw called");
    return NULL;
}

void* CGPDFDictionaryGetArray(void)
{
    if (verbose) puts("STUB: CGPDFDictionaryGetArray called");
    return NULL;
}

void* CGPDFDictionaryGetBoolean(void)
{
    if (verbose) puts("STUB: CGPDFDictionaryGetBoolean called");
    return NULL;
}

void* CGPDFDictionaryGetBooleans(void)
{
    if (verbose) puts("STUB: CGPDFDictionaryGetBooleans called");
    return NULL;
}

void* CGPDFDictionaryGetCount(void)
{
    if (verbose) puts("STUB: CGPDFDictionaryGetCount called");
    return NULL;
}

void* CGPDFDictionaryGetDictionary(void)
{
    if (verbose) puts("STUB: CGPDFDictionaryGetDictionary called");
    return NULL;
}

void* CGPDFDictionaryGetInteger(void)
{
    if (verbose) puts("STUB: CGPDFDictionaryGetInteger called");
    return NULL;
}

void* CGPDFDictionaryGetIntegers(void)
{
    if (verbose) puts("STUB: CGPDFDictionaryGetIntegers called");
    return NULL;
}

void* CGPDFDictionaryGetMatrix(void)
{
    if (verbose) puts("STUB: CGPDFDictionaryGetMatrix called");
    return NULL;
}

void* CGPDFDictionaryGetName(void)
{
    if (verbose) puts("STUB: CGPDFDictionaryGetName called");
    return NULL;
}

void* CGPDFDictionaryGetNumber(void)
{
    if (verbose) puts("STUB: CGPDFDictionaryGetNumber called");
    return NULL;
}

void* CGPDFDictionaryGetNumbers(void)
{
    if (verbose) puts("STUB: CGPDFDictionaryGetNumbers called");
    return NULL;
}

void* CGPDFDictionaryGetObject(void)
{
    if (verbose) puts("STUB: CGPDFDictionaryGetObject called");
    return NULL;
}

void* CGPDFDictionaryGetOffset(void)
{
    if (verbose) puts("STUB: CGPDFDictionaryGetOffset called");
    return NULL;
}

void* CGPDFDictionaryGetOwner(void)
{
    if (verbose) puts("STUB: CGPDFDictionaryGetOwner called");
    return NULL;
}

void* CGPDFDictionaryGetRect(void)
{
    if (verbose) puts("STUB: CGPDFDictionaryGetRect called");
    return NULL;
}

void* CGPDFDictionaryGetStream(void)
{
    if (verbose) puts("STUB: CGPDFDictionaryGetStream called");
    return NULL;
}

void* CGPDFDictionaryGetString(void)
{
    if (verbose) puts("STUB: CGPDFDictionaryGetString called");
    return NULL;
}

void* CGPDFDictionaryGetUnresolvedObject(void)
{
    if (verbose) puts("STUB: CGPDFDictionaryGetUnresolvedObject called");
    return NULL;
}

void* CGPDFDictionaryPrint(void)
{
    if (verbose) puts("STUB: CGPDFDictionaryPrint called");
    return NULL;
}

void* CGPDFDictionaryPrintWithIndent(void)
{
    if (verbose) puts("STUB: CGPDFDictionaryPrintWithIndent called");
    return NULL;
}

void* CGPDFDictionaryRelease(void)
{
    if (verbose) puts("STUB: CGPDFDictionaryRelease called");
    return NULL;
}

void* CGPDFDictionaryRetain(void)
{
    if (verbose) puts("STUB: CGPDFDictionaryRetain called");
    return NULL;
}

void* CGPDFDocumentAllowsCopying(void)
{
    if (verbose) puts("STUB: CGPDFDocumentAllowsCopying called");
    return NULL;
}

void* CGPDFDocumentAllowsPrinting(void)
{
    if (verbose) puts("STUB: CGPDFDocumentAllowsPrinting called");
    return NULL;
}

void* CGPDFDocumentAppendChanges(void)
{
    if (verbose) puts("STUB: CGPDFDocumentAppendChanges called");
    return NULL;
}

void* CGPDFDocumentAppendMetadata(void)
{
    if (verbose) puts("STUB: CGPDFDocumentAppendMetadata called");
    return NULL;
}

void* CGPDFDocumentAppendPage(void)
{
    if (verbose) puts("STUB: CGPDFDocumentAppendPage called");
    return NULL;
}

void* CGPDFDocumentAuthenticateOwnerPassword(void)
{
    if (verbose) puts("STUB: CGPDFDocumentAuthenticateOwnerPassword called");
    return NULL;
}

void* CGPDFDocumentAuthenticateUserPassword(void)
{
    if (verbose) puts("STUB: CGPDFDocumentAuthenticateUserPassword called");
    return NULL;
}

void* CGPDFDocumentCopyInfoDictionary(void)
{
    if (verbose) puts("STUB: CGPDFDocumentCopyInfoDictionary called");
    return NULL;
}

void* CGPDFDocumentCopyOutputIntents(void)
{
    if (verbose) puts("STUB: CGPDFDocumentCopyOutputIntents called");
    return NULL;
}

void* CGPDFDocumentCopyPage(void)
{
    if (verbose) puts("STUB: CGPDFDocumentCopyPage called");
    return NULL;
}

void* CGPDFDocumentCopyToConsumer(void)
{
    if (verbose) puts("STUB: CGPDFDocumentCopyToConsumer called");
    return NULL;
}

void* CGPDFDocumentCopyToConsumerWithChanges(void)
{
    if (verbose) puts("STUB: CGPDFDocumentCopyToConsumerWithChanges called");
    return NULL;
}

void* CGPDFDocumentCreateWithProvider(void)
{
    if (verbose) puts("STUB: CGPDFDocumentCreateWithProvider called");
    return NULL;
}

void* CGPDFDocumentCreateWithURL(void)
{
    if (verbose) puts("STUB: CGPDFDocumentCreateWithURL called");
    return NULL;
}

void* CGPDFDocumentEnableMRUMode(void)
{
    if (verbose) puts("STUB: CGPDFDocumentEnableMRUMode called");
    return NULL;
}

void* CGPDFDocumentEnableTaggedStructureSerialization(void)
{
    if (verbose) puts("STUB: CGPDFDocumentEnableTaggedStructureSerialization called");
    return NULL;
}

void* CGPDFDocumentExchangePagesAtIndices(void)
{
    if (verbose) puts("STUB: CGPDFDocumentExchangePagesAtIndices called");
    return NULL;
}

void* CGPDFDocumentGetAccessPermissions(void)
{
    if (verbose) puts("STUB: CGPDFDocumentGetAccessPermissions called");
    return NULL;
}

void* CGPDFDocumentGetArtBox(void)
{
    if (verbose) puts("STUB: CGPDFDocumentGetArtBox called");
    return NULL;
}

void* CGPDFDocumentGetBleedBox(void)
{
    if (verbose) puts("STUB: CGPDFDocumentGetBleedBox called");
    return NULL;
}

void* CGPDFDocumentGetCatalog(void)
{
    if (verbose) puts("STUB: CGPDFDocumentGetCatalog called");
    return NULL;
}

void* CGPDFDocumentGetCropBox(void)
{
    if (verbose) puts("STUB: CGPDFDocumentGetCropBox called");
    return NULL;
}

void* CGPDFDocumentGetDataProvider(void)
{
    if (verbose) puts("STUB: CGPDFDocumentGetDataProvider called");
    return NULL;
}

void* CGPDFDocumentGetDestination(void)
{
    if (verbose) puts("STUB: CGPDFDocumentGetDestination called");
    return NULL;
}

void* CGPDFDocumentGetID(void)
{
    if (verbose) puts("STUB: CGPDFDocumentGetID called");
    return NULL;
}

void* CGPDFDocumentGetInfo(void)
{
    if (verbose) puts("STUB: CGPDFDocumentGetInfo called");
    return NULL;
}

void* CGPDFDocumentGetMediaBox(void)
{
    if (verbose) puts("STUB: CGPDFDocumentGetMediaBox called");
    return NULL;
}

void* CGPDFDocumentGetNumberOfObjects(void)
{
    if (verbose) puts("STUB: CGPDFDocumentGetNumberOfObjects called");
    return NULL;
}

void* CGPDFDocumentGetNumberOfPages(void)
{
    if (verbose) puts("STUB: CGPDFDocumentGetNumberOfPages called");
    return NULL;
}

void* CGPDFDocumentGetObject(void)
{
    if (verbose) puts("STUB: CGPDFDocumentGetObject called");
    return NULL;
}

void* CGPDFDocumentGetOutline(void)
{
    if (verbose) puts("STUB: CGPDFDocumentGetOutline called");
    return NULL;
}

void* CGPDFDocumentGetPage(void)
{
    if (verbose) puts("STUB: CGPDFDocumentGetPage called");
    return NULL;
}

void* CGPDFDocumentGetPageDictionary(void)
{
    if (verbose) puts("STUB: CGPDFDocumentGetPageDictionary called");
    return NULL;
}

void* CGPDFDocumentGetPageForPageDictionary(void)
{
    if (verbose) puts("STUB: CGPDFDocumentGetPageForPageDictionary called");
    return NULL;
}

void* CGPDFDocumentGetPageNumberForPageDictionary(void)
{
    if (verbose) puts("STUB: CGPDFDocumentGetPageNumberForPageDictionary called");
    return NULL;
}

void* CGPDFDocumentGetPermissions(void)
{
    if (verbose) puts("STUB: CGPDFDocumentGetPermissions called");
    return NULL;
}

void* CGPDFDocumentGetProperty(void)
{
    if (verbose) puts("STUB: CGPDFDocumentGetProperty called");
    return NULL;
}

void* CGPDFDocumentGetRotationAngle(void)
{
    if (verbose) puts("STUB: CGPDFDocumentGetRotationAngle called");
    return NULL;
}

void* CGPDFDocumentGetTrimBox(void)
{
    if (verbose) puts("STUB: CGPDFDocumentGetTrimBox called");
    return NULL;
}

void* CGPDFDocumentGetTypeID(void)
{
    if (verbose) puts("STUB: CGPDFDocumentGetTypeID called");
    return NULL;
}

void* CGPDFDocumentGetURL(void)
{
    if (verbose) puts("STUB: CGPDFDocumentGetURL called");
    return NULL;
}

void* CGPDFDocumentGetVersion(void)
{
    if (verbose) puts("STUB: CGPDFDocumentGetVersion called");
    return NULL;
}

void* CGPDFDocumentInsertPageAtIndex(void)
{
    if (verbose) puts("STUB: CGPDFDocumentInsertPageAtIndex called");
    return NULL;
}

void* CGPDFDocumentIsEncrypted(void)
{
    if (verbose) puts("STUB: CGPDFDocumentIsEncrypted called");
    return NULL;
}

void* CGPDFDocumentIsLinearized(void)
{
    if (verbose) puts("STUB: CGPDFDocumentIsLinearized called");
    return NULL;
}

void* CGPDFDocumentIsTaggedPDF(void)
{
    if (verbose) puts("STUB: CGPDFDocumentIsTaggedPDF called");
    return NULL;
}

void* CGPDFDocumentIsUnlocked(void)
{
    if (verbose) puts("STUB: CGPDFDocumentIsUnlocked called");
    return NULL;
}

void* CGPDFDocumentIsUnlockedAsOwner(void)
{
    if (verbose) puts("STUB: CGPDFDocumentIsUnlockedAsOwner called");
    return NULL;
}

void* CGPDFDocumentPrintObject(void)
{
    if (verbose) puts("STUB: CGPDFDocumentPrintObject called");
    return NULL;
}

void* CGPDFDocumentRelease(void)
{
    if (verbose) puts("STUB: CGPDFDocumentRelease called");
    return NULL;
}

void* CGPDFDocumentRemovePageAtIndex(void)
{
    if (verbose) puts("STUB: CGPDFDocumentRemovePageAtIndex called");
    return NULL;
}

void* CGPDFDocumentRemoveProperty(void)
{
    if (verbose) puts("STUB: CGPDFDocumentRemoveProperty called");
    return NULL;
}

void* CGPDFDocumentRetain(void)
{
    if (verbose) puts("STUB: CGPDFDocumentRetain called");
    return NULL;
}

void* CGPDFDocumentSetDeviceCMYKColorSpace(void)
{
    if (verbose) puts("STUB: CGPDFDocumentSetDeviceCMYKColorSpace called");
    return NULL;
}

void* CGPDFDocumentSetDeviceGrayColorSpace(void)
{
    if (verbose) puts("STUB: CGPDFDocumentSetDeviceGrayColorSpace called");
    return NULL;
}

void* CGPDFDocumentSetDeviceRGBColorSpace(void)
{
    if (verbose) puts("STUB: CGPDFDocumentSetDeviceRGBColorSpace called");
    return NULL;
}

void* CGPDFDocumentSetProperty(void)
{
    if (verbose) puts("STUB: CGPDFDocumentSetProperty called");
    return NULL;
}

void* CGPDFDocumentUnlockAsOwnerWithPassword(void)
{
    if (verbose) puts("STUB: CGPDFDocumentUnlockAsOwnerWithPassword called");
    return NULL;
}

void* CGPDFDocumentUnlockWithPassword(void)
{
    if (verbose) puts("STUB: CGPDFDocumentUnlockWithPassword called");
    return NULL;
}

void* CGPDFDrawingContextCreateWithStream(void)
{
    if (verbose) puts("STUB: CGPDFDrawingContextCreateWithStream called");
    return NULL;
}

void* CGPDFDrawingContextDraw(void)
{
    if (verbose) puts("STUB: CGPDFDrawingContextDraw called");
    return NULL;
}

void* CGPDFDrawingContextRelease(void)
{
    if (verbose) puts("STUB: CGPDFDrawingContextRelease called");
    return NULL;
}

void* CGPDFEncodingCreate(void)
{
    if (verbose) puts("STUB: CGPDFEncodingCreate called");
    return NULL;
}

void* CGPDFEncodingGetGlyphVector(void)
{
    if (verbose) puts("STUB: CGPDFEncodingGetGlyphVector called");
    return NULL;
}

void* CGPDFEncodingGetGlyphVectorWithFont(void)
{
    if (verbose) puts("STUB: CGPDFEncodingGetGlyphVectorWithFont called");
    return NULL;
}

void* CGPDFEncodingGetNameForIndex(void)
{
    if (verbose) puts("STUB: CGPDFEncodingGetNameForIndex called");
    return NULL;
}

void* CGPDFEncodingGetUnicodeMaxLength(void)
{
    if (verbose) puts("STUB: CGPDFEncodingGetUnicodeMaxLength called");
    return NULL;
}

void* CGPDFEncodingGetUnicodesForIndex(void)
{
    if (verbose) puts("STUB: CGPDFEncodingGetUnicodesForIndex called");
    return NULL;
}

void* CGPDFEncodingRelease(void)
{
    if (verbose) puts("STUB: CGPDFEncodingRelease called");
    return NULL;
}

void* CGPDFFontCreate(void)
{
    if (verbose) puts("STUB: CGPDFFontCreate called");
    return NULL;
}

void* CGPDFFontCreateToUnicodeCMap(void)
{
    if (verbose) puts("STUB: CGPDFFontCreateToUnicodeCMap called");
    return NULL;
}

void* CGPDFFontCreateWithObject(void)
{
    if (verbose) puts("STUB: CGPDFFontCreateWithObject called");
    return NULL;
}

void* CGPDFFontDescriptorGetDictionary(void)
{
    if (verbose) puts("STUB: CGPDFFontDescriptorGetDictionary called");
    return NULL;
}

void* CGPDFFontDescriptorGetFlags(void)
{
    if (verbose) puts("STUB: CGPDFFontDescriptorGetFlags called");
    return NULL;
}

void* CGPDFFontDescriptorRelease(void)
{
    if (verbose) puts("STUB: CGPDFFontDescriptorRelease called");
    return NULL;
}

void* CGPDFFontDescriptorRetain(void)
{
    if (verbose) puts("STUB: CGPDFFontDescriptorRetain called");
    return NULL;
}

void* CGPDFFontGetAdvances(void)
{
    if (verbose) puts("STUB: CGPDFFontGetAdvances called");
    return NULL;
}

void* CGPDFFontGetCIDSystemInfo(void)
{
    if (verbose) puts("STUB: CGPDFFontGetCIDSystemInfo called");
    return NULL;
}

void* CGPDFFontGetCIDToGlyphMap(void)
{
    if (verbose) puts("STUB: CGPDFFontGetCIDToGlyphMap called");
    return NULL;
}

void* CGPDFFontGetCMap(void)
{
    if (verbose) puts("STUB: CGPDFFontGetCMap called");
    return NULL;
}

void* CGPDFFontGetDescendant(void)
{
    if (verbose) puts("STUB: CGPDFFontGetDescendant called");
    return NULL;
}

void* CGPDFFontGetDictionary(void)
{
    if (verbose) puts("STUB: CGPDFFontGetDictionary called");
    return NULL;
}

void* CGPDFFontGetEncoding(void)
{
    if (verbose) puts("STUB: CGPDFFontGetEncoding called");
    return NULL;
}

void* CGPDFFontGetFont(void)
{
    if (verbose) puts("STUB: CGPDFFontGetFont called");
    return NULL;
}

void* CGPDFFontGetFontBBox(void)
{
    if (verbose) puts("STUB: CGPDFFontGetFontBBox called");
    return NULL;
}

void* CGPDFFontGetFontDescriptor(void)
{
    if (verbose) puts("STUB: CGPDFFontGetFontDescriptor called");
    return NULL;
}

void* CGPDFFontGetFontMatrix(void)
{
    if (verbose) puts("STUB: CGPDFFontGetFontMatrix called");
    return NULL;
}

void* CGPDFFontGetMutator(void)
{
    if (verbose) puts("STUB: CGPDFFontGetMutator called");
    return NULL;
}

void* CGPDFFontGetName(void)
{
    if (verbose) puts("STUB: CGPDFFontGetName called");
    return NULL;
}

void* CGPDFFontGetProcedures(void)
{
    if (verbose) puts("STUB: CGPDFFontGetProcedures called");
    return NULL;
}

void* CGPDFFontGetROSUnicodeCMap(void)
{
    if (verbose) puts("STUB: CGPDFFontGetROSUnicodeCMap called");
    return NULL;
}

void* CGPDFFontGetToUnicodeCMap(void)
{
    if (verbose) puts("STUB: CGPDFFontGetToUnicodeCMap called");
    return NULL;
}

void* CGPDFFontGetType(void)
{
    if (verbose) puts("STUB: CGPDFFontGetType called");
    return NULL;
}

void* CGPDFFontIsBase14(void)
{
    if (verbose) puts("STUB: CGPDFFontIsBase14 called");
    return NULL;
}

void* CGPDFFontIsEmbedded(void)
{
    if (verbose) puts("STUB: CGPDFFontIsEmbedded called");
    return NULL;
}

void* CGPDFFontRelease(void)
{
    if (verbose) puts("STUB: CGPDFFontRelease called");
    return NULL;
}

void* CGPDFFontRetain(void)
{
    if (verbose) puts("STUB: CGPDFFontRetain called");
    return NULL;
}

void* CGPDFFormCreate(void)
{
    if (verbose) puts("STUB: CGPDFFormCreate called");
    return NULL;
}

void* CGPDFFormGetBBox(void)
{
    if (verbose) puts("STUB: CGPDFFormGetBBox called");
    return NULL;
}

void* CGPDFFormGetMatrix(void)
{
    if (verbose) puts("STUB: CGPDFFormGetMatrix called");
    return NULL;
}

void* CGPDFFormGetResources(void)
{
    if (verbose) puts("STUB: CGPDFFormGetResources called");
    return NULL;
}

void* CGPDFFormGetStream(void)
{
    if (verbose) puts("STUB: CGPDFFormGetStream called");
    return NULL;
}

void* CGPDFFormRelease(void)
{
    if (verbose) puts("STUB: CGPDFFormRelease called");
    return NULL;
}

void* CGPDFFormRetain(void)
{
    if (verbose) puts("STUB: CGPDFFormRetain called");
    return NULL;
}

void* CGPDFFunctionCreateFunction(void)
{
    if (verbose) puts("STUB: CGPDFFunctionCreateFunction called");
    return NULL;
}

void* CGPDFGetBaseEncodingForName(void)
{
    if (verbose) puts("STUB: CGPDFGetBaseEncodingForName called");
    return NULL;
}

void* CGPDFGetNameForBaseEncoding(void)
{
    if (verbose) puts("STUB: CGPDFGetNameForBaseEncoding called");
    return NULL;
}

void* CGPDFGetNameVectorForBaseEncoding(void)
{
    if (verbose) puts("STUB: CGPDFGetNameVectorForBaseEncoding called");
    return NULL;
}

void* CGPDFGetUnicharGuessForCID(void)
{
    if (verbose) puts("STUB: CGPDFGetUnicharGuessForCID called");
    return NULL;
}

void* CGPDFGetUnicharGuessForGlyphName(void)
{
    if (verbose) puts("STUB: CGPDFGetUnicharGuessForGlyphName called");
    return NULL;
}

void* CGPDFGetUnicharVectorForBaseEncoding(void)
{
    if (verbose) puts("STUB: CGPDFGetUnicharVectorForBaseEncoding called");
    return NULL;
}

void* CGPDFGetUnicharsForGlyphName(void)
{
    if (verbose) puts("STUB: CGPDFGetUnicharsForGlyphName called");
    return NULL;
}

void* CGPDFImageCreateImage(void)
{
    if (verbose) puts("STUB: CGPDFImageCreateImage called");
    return NULL;
}

void* CGPDFLayoutCreate(void)
{
    if (verbose) puts("STUB: CGPDFLayoutCreate called");
    return NULL;
}

void* CGPDFObjectGetReference(void)
{
    if (verbose) puts("STUB: CGPDFObjectGetReference called");
    return NULL;
}

void* CGPDFObjectGetType(void)
{
    if (verbose) puts("STUB: CGPDFObjectGetType called");
    return NULL;
}

void* CGPDFObjectGetValue(void)
{
    if (verbose) puts("STUB: CGPDFObjectGetValue called");
    return NULL;
}

void* CGPDFObjectPrint(void)
{
    if (verbose) puts("STUB: CGPDFObjectPrint called");
    return NULL;
}

void* CGPDFOperatorTableCreate(void)
{
    if (verbose) puts("STUB: CGPDFOperatorTableCreate called");
    return NULL;
}

void* CGPDFOperatorTableRelease(void)
{
    if (verbose) puts("STUB: CGPDFOperatorTableRelease called");
    return NULL;
}

void* CGPDFOperatorTableRetain(void)
{
    if (verbose) puts("STUB: CGPDFOperatorTableRetain called");
    return NULL;
}

void* CGPDFOperatorTableSetCallback(void)
{
    if (verbose) puts("STUB: CGPDFOperatorTableSetCallback called");
    return NULL;
}

void* CGPDFPageAddAnnotation(void)
{
    if (verbose) puts("STUB: CGPDFPageAddAnnotation called");
    return NULL;
}

void* CGPDFPageContainsWideGamutContent(void)
{
    if (verbose) puts("STUB: CGPDFPageContainsWideGamutContent called");
    return NULL;
}

void* CGPDFPageCopyAnnotations(void)
{
    if (verbose) puts("STUB: CGPDFPageCopyAnnotations called");
    return NULL;
}

void* CGPDFPageCopyPageLabel(void)
{
    if (verbose) puts("STUB: CGPDFPageCopyPageLabel called");
    return NULL;
}

void* CGPDFPageCopyTextString(void)
{
    if (verbose) puts("STUB: CGPDFPageCopyTextString called");
    return NULL;
}

void* CGPDFPageCopyTextStringWithGlyphs(void)
{
    if (verbose) puts("STUB: CGPDFPageCopyTextStringWithGlyphs called");
    return NULL;
}

void* CGPDFPageCopyThumbnail(void)
{
    if (verbose) puts("STUB: CGPDFPageCopyThumbnail called");
    return NULL;
}

void* CGPDFPageCreate(void)
{
    if (verbose) puts("STUB: CGPDFPageCreate called");
    return NULL;
}

void* CGPDFPageCreateThumbnail(void)
{
    if (verbose) puts("STUB: CGPDFPageCreateThumbnail called");
    return NULL;
}

void* CGPDFPageCreateWithImage(void)
{
    if (verbose) puts("STUB: CGPDFPageCreateWithImage called");
    return NULL;
}

void* CGPDFPageEnumerateAnnotations(void)
{
    if (verbose) puts("STUB: CGPDFPageEnumerateAnnotations called");
    return NULL;
}

void* CGPDFPageGetAnnotationAtIndex(void)
{
    if (verbose) puts("STUB: CGPDFPageGetAnnotationAtIndex called");
    return NULL;
}

void* CGPDFPageGetAnnotationCount(void)
{
    if (verbose) puts("STUB: CGPDFPageGetAnnotationCount called");
    return NULL;
}

void* CGPDFPageGetBoxRect(void)
{
    if (verbose) puts("STUB: CGPDFPageGetBoxRect called");
    return NULL;
}

void* CGPDFPageGetColorSpace(void)
{
    if (verbose) puts("STUB: CGPDFPageGetColorSpace called");
    return NULL;
}

void* CGPDFPageGetContents(void)
{
    if (verbose) puts("STUB: CGPDFPageGetContents called");
    return NULL;
}

void* CGPDFPageGetDictionary(void)
{
    if (verbose) puts("STUB: CGPDFPageGetDictionary called");
    return NULL;
}

void* CGPDFPageGetDocument(void)
{
    if (verbose) puts("STUB: CGPDFPageGetDocument called");
    return NULL;
}

void* CGPDFPageGetDrawingTransform(void)
{
    if (verbose) puts("STUB: CGPDFPageGetDrawingTransform called");
    return NULL;
}

void* CGPDFPageGetDrawingTransformForRect(void)
{
    if (verbose) puts("STUB: CGPDFPageGetDrawingTransformForRect called");
    return NULL;
}

void* CGPDFPageGetImages(void)
{
    if (verbose) puts("STUB: CGPDFPageGetImages called");
    return NULL;
}

void* CGPDFPageGetPage(void)
{
    if (verbose) puts("STUB: CGPDFPageGetPage called");
    return NULL;
}

void* CGPDFPageGetPageNumber(void)
{
    if (verbose) puts("STUB: CGPDFPageGetPageNumber called");
    return NULL;
}

void* CGPDFPageGetProperty(void)
{
    if (verbose) puts("STUB: CGPDFPageGetProperty called");
    return NULL;
}

void* CGPDFPageGetRotationAngle(void)
{
    if (verbose) puts("STUB: CGPDFPageGetRotationAngle called");
    return NULL;
}

void* CGPDFPageGetThumbnail(void)
{
    if (verbose) puts("STUB: CGPDFPageGetThumbnail called");
    return NULL;
}

void* CGPDFPageGetTypeID(void)
{
    if (verbose) puts("STUB: CGPDFPageGetTypeID called");
    return NULL;
}

void* CGPDFPagePrint(void)
{
    if (verbose) puts("STUB: CGPDFPagePrint called");
    return NULL;
}

void* CGPDFPageRelease(void)
{
    if (verbose) puts("STUB: CGPDFPageRelease called");
    return NULL;
}

void* CGPDFPageRemoveAnnotation(void)
{
    if (verbose) puts("STUB: CGPDFPageRemoveAnnotation called");
    return NULL;
}

void* CGPDFPageRemoveProperty(void)
{
    if (verbose) puts("STUB: CGPDFPageRemoveProperty called");
    return NULL;
}

void* CGPDFPageRetain(void)
{
    if (verbose) puts("STUB: CGPDFPageRetain called");
    return NULL;
}

void* CGPDFPageSetBoxRect(void)
{
    if (verbose) puts("STUB: CGPDFPageSetBoxRect called");
    return NULL;
}

void* CGPDFPageSetProperty(void)
{
    if (verbose) puts("STUB: CGPDFPageSetProperty called");
    return NULL;
}

void* CGPDFPageSetRotationAngle(void)
{
    if (verbose) puts("STUB: CGPDFPageSetRotationAngle called");
    return NULL;
}

void* CGPDFPageSortAnnotations(void)
{
    if (verbose) puts("STUB: CGPDFPageSortAnnotations called");
    return NULL;
}

void* CGPDFPageSubstituteAnnotation(void)
{
    if (verbose) puts("STUB: CGPDFPageSubstituteAnnotation called");
    return NULL;
}

void* CGPDFScannerCreate(void)
{
    if (verbose) puts("STUB: CGPDFScannerCreate called");
    return NULL;
}

void* CGPDFScannerGetContentStream(void)
{
    if (verbose) puts("STUB: CGPDFScannerGetContentStream called");
    return NULL;
}

void* CGPDFScannerPopArray(void)
{
    if (verbose) puts("STUB: CGPDFScannerPopArray called");
    return NULL;
}

void* CGPDFScannerPopBoolean(void)
{
    if (verbose) puts("STUB: CGPDFScannerPopBoolean called");
    return NULL;
}

void* CGPDFScannerPopDictionary(void)
{
    if (verbose) puts("STUB: CGPDFScannerPopDictionary called");
    return NULL;
}

void* CGPDFScannerPopInteger(void)
{
    if (verbose) puts("STUB: CGPDFScannerPopInteger called");
    return NULL;
}

void* CGPDFScannerPopName(void)
{
    if (verbose) puts("STUB: CGPDFScannerPopName called");
    return NULL;
}

void* CGPDFScannerPopNumber(void)
{
    if (verbose) puts("STUB: CGPDFScannerPopNumber called");
    return NULL;
}

void* CGPDFScannerPopObject(void)
{
    if (verbose) puts("STUB: CGPDFScannerPopObject called");
    return NULL;
}

void* CGPDFScannerPopStream(void)
{
    if (verbose) puts("STUB: CGPDFScannerPopStream called");
    return NULL;
}

void* CGPDFScannerPopString(void)
{
    if (verbose) puts("STUB: CGPDFScannerPopString called");
    return NULL;
}

void* CGPDFScannerRelease(void)
{
    if (verbose) puts("STUB: CGPDFScannerRelease called");
    return NULL;
}

void* CGPDFScannerRetain(void)
{
    if (verbose) puts("STUB: CGPDFScannerRetain called");
    return NULL;
}

void* CGPDFScannerScan(void)
{
    if (verbose) puts("STUB: CGPDFScannerScan called");
    return NULL;
}

void* CGPDFSourceGetLength(void)
{
    if (verbose) puts("STUB: CGPDFSourceGetLength called");
    return NULL;
}

void* CGPDFSourceGetPosition(void)
{
    if (verbose) puts("STUB: CGPDFSourceGetPosition called");
    return NULL;
}

void* CGPDFSourceGetc(void)
{
    if (verbose) puts("STUB: CGPDFSourceGetc called");
    return NULL;
}

void* CGPDFSourceIsAtEOF(void)
{
    if (verbose) puts("STUB: CGPDFSourceIsAtEOF called");
    return NULL;
}

void* CGPDFSourceRead(void)
{
    if (verbose) puts("STUB: CGPDFSourceRead called");
    return NULL;
}

void* CGPDFSourceRelease(void)
{
    if (verbose) puts("STUB: CGPDFSourceRelease called");
    return NULL;
}

void* CGPDFSourceRetain(void)
{
    if (verbose) puts("STUB: CGPDFSourceRetain called");
    return NULL;
}

void* CGPDFSourceRewind(void)
{
    if (verbose) puts("STUB: CGPDFSourceRewind called");
    return NULL;
}

void* CGPDFSourceSetPosition(void)
{
    if (verbose) puts("STUB: CGPDFSourceSetPosition called");
    return NULL;
}

void* CGPDFSourceUngetc(void)
{
    if (verbose) puts("STUB: CGPDFSourceUngetc called");
    return NULL;
}

void* CGPDFStreamCopyData(void)
{
    if (verbose) puts("STUB: CGPDFStreamCopyData called");
    return NULL;
}

void* CGPDFStreamCreate(void)
{
    if (verbose) puts("STUB: CGPDFStreamCreate called");
    return NULL;
}

void* CGPDFStreamCreateDataProvider(void)
{
    if (verbose) puts("STUB: CGPDFStreamCreateDataProvider called");
    return NULL;
}

void* CGPDFStreamCreateFilterChain(void)
{
    if (verbose) puts("STUB: CGPDFStreamCreateFilterChain called");
    return NULL;
}

void* CGPDFStreamCreateWithData(void)
{
    if (verbose) puts("STUB: CGPDFStreamCreateWithData called");
    return NULL;
}

void* CGPDFStreamGetAssociation(void)
{
    if (verbose) puts("STUB: CGPDFStreamGetAssociation called");
    return NULL;
}

void* CGPDFStreamGetData(void)
{
    if (verbose) puts("STUB: CGPDFStreamGetData called");
    return NULL;
}

void* CGPDFStreamGetDictionary(void)
{
    if (verbose) puts("STUB: CGPDFStreamGetDictionary called");
    return NULL;
}

void* CGPDFStreamPrint(void)
{
    if (verbose) puts("STUB: CGPDFStreamPrint called");
    return NULL;
}

void* CGPDFStreamPrintContents(void)
{
    if (verbose) puts("STUB: CGPDFStreamPrintContents called");
    return NULL;
}

void* CGPDFStreamPrintContentsWithIndent(void)
{
    if (verbose) puts("STUB: CGPDFStreamPrintContentsWithIndent called");
    return NULL;
}

void* CGPDFStreamPrintWithIndent(void)
{
    if (verbose) puts("STUB: CGPDFStreamPrintWithIndent called");
    return NULL;
}

void* CGPDFStreamRelease(void)
{
    if (verbose) puts("STUB: CGPDFStreamRelease called");
    return NULL;
}

void* CGPDFStreamRetain(void)
{
    if (verbose) puts("STUB: CGPDFStreamRetain called");
    return NULL;
}

void* CGPDFStreamSetAssociation(void)
{
    if (verbose) puts("STUB: CGPDFStreamSetAssociation called");
    return NULL;
}

void* CGPDFStreamSetObjectReference(void)
{
    if (verbose) puts("STUB: CGPDFStreamSetObjectReference called");
    return NULL;
}

void* CGPDFStringCopyDate(void)
{
    if (verbose) puts("STUB: CGPDFStringCopyDate called");
    return NULL;
}

void* CGPDFStringCopyTextString(void)
{
    if (verbose) puts("STUB: CGPDFStringCopyTextString called");
    return NULL;
}

void* CGPDFStringGetBytePtr(void)
{
    if (verbose) puts("STUB: CGPDFStringGetBytePtr called");
    return NULL;
}

void* CGPDFStringGetLength(void)
{
    if (verbose) puts("STUB: CGPDFStringGetLength called");
    return NULL;
}

void* CGPDFStringPrint(void)
{
    if (verbose) puts("STUB: CGPDFStringPrint called");
    return NULL;
}

void* CGPDFStringRelease(void)
{
    if (verbose) puts("STUB: CGPDFStringRelease called");
    return NULL;
}

void* CGPDFStringRetain(void)
{
    if (verbose) puts("STUB: CGPDFStringRetain called");
    return NULL;
}

void* CGPDFTagTypeGetName(void)
{
    if (verbose) puts("STUB: CGPDFTagTypeGetName called");
    return NULL;
}

void* CGPDFTextStringGetBBoxForIndex(void)
{
    if (verbose) puts("STUB: CGPDFTextStringGetBBoxForIndex called");
    return NULL;
}

void* CGPDFTextStringGetCharacterAtIndex(void)
{
    if (verbose) puts("STUB: CGPDFTextStringGetCharacterAtIndex called");
    return NULL;
}

void* CGPDFTextStringGetColorForIndex(void)
{
    if (verbose) puts("STUB: CGPDFTextStringGetColorForIndex called");
    return NULL;
}

void* CGPDFTextStringGetFontForIndex(void)
{
    if (verbose) puts("STUB: CGPDFTextStringGetFontForIndex called");
    return NULL;
}

void* CGPDFTextStringGetGlyphAtIndex(void)
{
    if (verbose) puts("STUB: CGPDFTextStringGetGlyphAtIndex called");
    return NULL;
}

void* CGPDFTextStringGetLength(void)
{
    if (verbose) puts("STUB: CGPDFTextStringGetLength called");
    return NULL;
}

void* CGPDFTextStringGetPositionForIndex(void)
{
    if (verbose) puts("STUB: CGPDFTextStringGetPositionForIndex called");
    return NULL;
}

void* CGPDFTextStringGetTransformForIndex(void)
{
    if (verbose) puts("STUB: CGPDFTextStringGetTransformForIndex called");
    return NULL;
}

void* CGPDFTextStringRelease(void)
{
    if (verbose) puts("STUB: CGPDFTextStringRelease called");
    return NULL;
}

void* CGPDFTextStringRetain(void)
{
    if (verbose) puts("STUB: CGPDFTextStringRetain called");
    return NULL;
}

void* CGPSContextClose(void)
{
    if (verbose) puts("STUB: CGPSContextClose called");
    return NULL;
}

void* CGPSContextCreate(void)
{
    if (verbose) puts("STUB: CGPSContextCreate called");
    return NULL;
}

void* CGPSContextCreateWithFilename(void)
{
    if (verbose) puts("STUB: CGPSContextCreateWithFilename called");
    return NULL;
}

void* CGPSContextCreateWithURL(void)
{
    if (verbose) puts("STUB: CGPSContextCreateWithURL called");
    return NULL;
}

void* CGPSConverterAbort(void)
{
    if (verbose) puts("STUB: CGPSConverterAbort called");
    return NULL;
}

void* CGPSConverterConvert(void)
{
    if (verbose) puts("STUB: CGPSConverterConvert called");
    return NULL;
}

void* CGPSConverterConvertFromURLToURL(void)
{
    if (verbose) puts("STUB: CGPSConverterConvertFromURLToURL called");
    return NULL;
}

void* CGPSConverterCreate(void)
{
    if (verbose) puts("STUB: CGPSConverterCreate called");
    return NULL;
}

void* CGPSConverterGetTypeID(void)
{
    if (verbose) puts("STUB: CGPSConverterGetTypeID called");
    return NULL;
}

void* CGPSConverterIsConverting(void)
{
    if (verbose) puts("STUB: CGPSConverterIsConverting called");
    return NULL;
}

void* CGPathAddArc(void)
{
    if (verbose) puts("STUB: CGPathAddArc called");
    return NULL;
}

void* CGPathAddArcToPoint(void)
{
    if (verbose) puts("STUB: CGPathAddArcToPoint called");
    return NULL;
}

void* CGPathAddCurveToPoint(void)
{
    if (verbose) puts("STUB: CGPathAddCurveToPoint called");
    return NULL;
}

void* CGPathAddEllipseInRect(void)
{
    if (verbose) puts("STUB: CGPathAddEllipseInRect called");
    return NULL;
}

void* CGPathAddLineToPoint(void)
{
    if (verbose) puts("STUB: CGPathAddLineToPoint called");
    return NULL;
}

void* CGPathAddLines(void)
{
    if (verbose) puts("STUB: CGPathAddLines called");
    return NULL;
}

void* CGPathAddPath(void)
{
    if (verbose) puts("STUB: CGPathAddPath called");
    return NULL;
}

void* CGPathAddQuadCurveToPoint(void)
{
    if (verbose) puts("STUB: CGPathAddQuadCurveToPoint called");
    return NULL;
}

void* CGPathAddRect(void)
{
    if (verbose) puts("STUB: CGPathAddRect called");
    return NULL;
}

void* CGPathAddRects(void)
{
    if (verbose) puts("STUB: CGPathAddRects called");
    return NULL;
}

void* CGPathAddRegion(void)
{
    if (verbose) puts("STUB: CGPathAddRegion called");
    return NULL;
}

void* CGPathAddRelativeArc(void)
{
    if (verbose) puts("STUB: CGPathAddRelativeArc called");
    return NULL;
}

void* CGPathAddRoundedRect(void)
{
    if (verbose) puts("STUB: CGPathAddRoundedRect called");
    return NULL;
}

void* CGPathAddUnevenCornersRoundedRect(void)
{
    if (verbose) puts("STUB: CGPathAddUnevenCornersRoundedRect called");
    return NULL;
}

void* CGPathApply(void)
{
    if (verbose) puts("STUB: CGPathApply called");
    return NULL;
}

void* CGPathApplyDashedPath(void)
{
    if (verbose) puts("STUB: CGPathApplyDashedPath called");
    return NULL;
}

void* CGPathApplyIterator(void)
{
    if (verbose) puts("STUB: CGPathApplyIterator called");
    return NULL;
}

void* CGPathApplyStrokedDashedPath(void)
{
    if (verbose) puts("STUB: CGPathApplyStrokedDashedPath called");
    return NULL;
}

void* CGPathApplyWithBlock(void)
{
    if (verbose) puts("STUB: CGPathApplyWithBlock called");
    return NULL;
}

void* CGPathCloseSubpath(void)
{
    if (verbose) puts("STUB: CGPathCloseSubpath called");
    return NULL;
}

void* CGPathContainsPoint(void)
{
    if (verbose) puts("STUB: CGPathContainsPoint called");
    return NULL;
}

void* CGPathCreateByNormalizingGlyphPath(void)
{
    if (verbose) puts("STUB: CGPathCreateByNormalizingGlyphPath called");
    return NULL;
}

void* CGPathCreateCopy(void)
{
    if (verbose) puts("STUB: CGPathCreateCopy called");
    return NULL;
}

void* CGPathCreateCopyByDashingPath(void)
{
    if (verbose) puts("STUB: CGPathCreateCopyByDashingPath called");
    return NULL;
}

void* CGPathCreateCopyByStrokingPath(void)
{
    if (verbose) puts("STUB: CGPathCreateCopyByStrokingPath called");
    return NULL;
}

void* CGPathCreateCopyByTransformingPath(void)
{
    if (verbose) puts("STUB: CGPathCreateCopyByTransformingPath called");
    return NULL;
}

void* CGPathCreateDashedPath(void)
{
    if (verbose) puts("STUB: CGPathCreateDashedPath called");
    return NULL;
}

void* CGPathCreateMutable(void)
{
    if (verbose) puts("STUB: CGPathCreateMutable called");
    return NULL;
}

void* CGPathCreateMutableCopy(void)
{
    if (verbose) puts("STUB: CGPathCreateMutableCopy called");
    return NULL;
}

void* CGPathCreateMutableCopyByTransformingPath(void)
{
    if (verbose) puts("STUB: CGPathCreateMutableCopyByTransformingPath called");
    return NULL;
}

void* CGPathCreateStrokedPath(void)
{
    if (verbose) puts("STUB: CGPathCreateStrokedPath called");
    return NULL;
}

void* CGPathCreateWithEllipseInRect(void)
{
    if (verbose) puts("STUB: CGPathCreateWithEllipseInRect called");
    return NULL;
}

void* CGPathCreateWithLineSegment(void)
{
    if (verbose) puts("STUB: CGPathCreateWithLineSegment called");
    return NULL;
}

void* CGPathCreateWithRect(void)
{
    if (verbose) puts("STUB: CGPathCreateWithRect called");
    return NULL;
}

void* CGPathCreateWithRoundedRect(void)
{
    if (verbose) puts("STUB: CGPathCreateWithRoundedRect called");
    return NULL;
}

void* CGPathCreateWithThresholdedImage(void)
{
    if (verbose) puts("STUB: CGPathCreateWithThresholdedImage called");
    return NULL;
}

void* CGPathCreateWithThresholdedImageDebug(void)
{
    if (verbose) puts("STUB: CGPathCreateWithThresholdedImageDebug called");
    return NULL;
}

void* CGPathCreateWithUnevenCornersRoundedRect(void)
{
    if (verbose) puts("STUB: CGPathCreateWithUnevenCornersRoundedRect called");
    return NULL;
}

void* CGPathEqualToPath(void)
{
    if (verbose) puts("STUB: CGPathEqualToPath called");
    return NULL;
}

void* CGPathGetBoundingBox(void)
{
    if (verbose) puts("STUB: CGPathGetBoundingBox called");
    return NULL;
}

void* CGPathGetCurrentPoint(void)
{
    if (verbose) puts("STUB: CGPathGetCurrentPoint called");
    return NULL;
}

void* CGPathGetGeometricBoundingBox(void)
{
    if (verbose) puts("STUB: CGPathGetGeometricBoundingBox called");
    return NULL;
}

void* CGPathGetPathBoundingBox(void)
{
    if (verbose) puts("STUB: CGPathGetPathBoundingBox called");
    return NULL;
}

void* CGPathGetTypeID(void)
{
    if (verbose) puts("STUB: CGPathGetTypeID called");
    return NULL;
}

void* CGPathIsEllipse(void)
{
    if (verbose) puts("STUB: CGPathIsEllipse called");
    return NULL;
}

void* CGPathIsEllipse2(void)
{
    if (verbose) puts("STUB: CGPathIsEllipse2 called");
    return NULL;
}

void* CGPathIsEllipseWithTransform(void)
{
    if (verbose) puts("STUB: CGPathIsEllipseWithTransform called");
    return NULL;
}

void* CGPathIsEmpty(void)
{
    if (verbose) puts("STUB: CGPathIsEmpty called");
    return NULL;
}

void* CGPathIsLine(void)
{
    if (verbose) puts("STUB: CGPathIsLine called");
    return NULL;
}

void* CGPathIsRect(void)
{
    if (verbose) puts("STUB: CGPathIsRect called");
    return NULL;
}

void* CGPathIsRectWithTransform(void)
{
    if (verbose) puts("STUB: CGPathIsRectWithTransform called");
    return NULL;
}

void* CGPathIsRegion(void)
{
    if (verbose) puts("STUB: CGPathIsRegion called");
    return NULL;
}

void* CGPathIsRoundedRect(void)
{
    if (verbose) puts("STUB: CGPathIsRoundedRect called");
    return NULL;
}

void* CGPathIsRoundedRectWithTransform(void)
{
    if (verbose) puts("STUB: CGPathIsRoundedRectWithTransform called");
    return NULL;
}

void* CGPathIsUnevenCornersRoundedRectWithTransform(void)
{
    if (verbose) puts("STUB: CGPathIsUnevenCornersRoundedRectWithTransform called");
    return NULL;
}

void* CGPathIsValidGlyphPath(void)
{
    if (verbose) puts("STUB: CGPathIsValidGlyphPath called");
    return NULL;
}

void* CGPathMoveToPoint(void)
{
    if (verbose) puts("STUB: CGPathMoveToPoint called");
    return NULL;
}

void* CGPathPrint(void)
{
    if (verbose) puts("STUB: CGPathPrint called");
    return NULL;
}

void* CGPathRelease(void)
{
    if (verbose) puts("STUB: CGPathRelease called");
    return NULL;
}

void* CGPathRetain(void)
{
    if (verbose) puts("STUB: CGPathRetain called");
    return NULL;
}

void* CGPatternCreate(void)
{
    if (verbose) puts("STUB: CGPatternCreate called");
    return NULL;
}

void* CGPatternCreateColoredEmpty(void)
{
    if (verbose) puts("STUB: CGPatternCreateColoredEmpty called");
    return NULL;
}

void* CGPatternCreateUncoloredEmpty(void)
{
    if (verbose) puts("STUB: CGPatternCreateUncoloredEmpty called");
    return NULL;
}

void* CGPatternCreateWithImage(void)
{
    if (verbose) puts("STUB: CGPatternCreateWithImage called");
    return NULL;
}

void* CGPatternCreateWithImage2(void)
{
    if (verbose) puts("STUB: CGPatternCreateWithImage2 called");
    return NULL;
}

void* CGPatternDrawInContextDelegate(void)
{
    if (verbose) puts("STUB: CGPatternDrawInContextDelegate called");
    return NULL;
}

void* CGPatternGetAssociate(void)
{
    if (verbose) puts("STUB: CGPatternGetAssociate called");
    return NULL;
}

void* CGPatternGetBounds(void)
{
    if (verbose) puts("STUB: CGPatternGetBounds called");
    return NULL;
}

void* CGPatternGetIdentifier(void)
{
    if (verbose) puts("STUB: CGPatternGetIdentifier called");
    return NULL;
}

void* CGPatternGetImage(void)
{
    if (verbose) puts("STUB: CGPatternGetImage called");
    return NULL;
}

void* CGPatternGetMatrix(void)
{
    if (verbose) puts("STUB: CGPatternGetMatrix called");
    return NULL;
}

void* CGPatternGetShading(void)
{
    if (verbose) puts("STUB: CGPatternGetShading called");
    return NULL;
}

void* CGPatternGetStep(void)
{
    if (verbose) puts("STUB: CGPatternGetStep called");
    return NULL;
}

void* CGPatternGetTiling(void)
{
    if (verbose) puts("STUB: CGPatternGetTiling called");
    return NULL;
}

void* CGPatternGetType(void)
{
    if (verbose) puts("STUB: CGPatternGetType called");
    return NULL;
}

void* CGPatternGetTypeID(void)
{
    if (verbose) puts("STUB: CGPatternGetTypeID called");
    return NULL;
}

void* CGPatternIsColored(void)
{
    if (verbose) puts("STUB: CGPatternIsColored called");
    return NULL;
}

void* CGPatternIsOpaque(void)
{
    if (verbose) puts("STUB: CGPatternIsOpaque called");
    return NULL;
}

void* CGPatternNotificationCenter(void)
{
    if (verbose) puts("STUB: CGPatternNotificationCenter called");
    return NULL;
}

void* CGPatternRelease(void)
{
    if (verbose) puts("STUB: CGPatternRelease called");
    return NULL;
}

void* CGPatternRetain(void)
{
    if (verbose) puts("STUB: CGPatternRetain called");
    return NULL;
}

void* CGPatternSetAssociate(void)
{
    if (verbose) puts("STUB: CGPatternSetAssociate called");
    return NULL;
}

void* CGPointApplyAffineTransform(void)
{
    if (verbose) puts("STUB: CGPointApplyAffineTransform called");
    return NULL;
}

void* CGPointApplyInverseAffineTransform(void)
{
    if (verbose) puts("STUB: CGPointApplyInverseAffineTransform called");
    return NULL;
}

void* CGPointCreateDictionaryRepresentation(void)
{
    if (verbose) puts("STUB: CGPointCreateDictionaryRepresentation called");
    return NULL;
}

void* CGPointEqualToPoint(void)
{
    if (verbose) puts("STUB: CGPointEqualToPoint called");
    return NULL;
}

void* CGPointMakeWithDictionaryRepresentation(void)
{
    if (verbose) puts("STUB: CGPointMakeWithDictionaryRepresentation called");
    return NULL;
}

void* CGPostError(void)
{
    if (verbose) puts("STUB: CGPostError called");
    return NULL;
}

void* CGPostErrorWithArguments(void)
{
    if (verbose) puts("STUB: CGPostErrorWithArguments called");
    return NULL;
}

void* CGPreferencesCopyString(void)
{
    if (verbose) puts("STUB: CGPreferencesCopyString called");
    return NULL;
}

void* CGPreferencesGetBoolean(void)
{
    if (verbose) puts("STUB: CGPreferencesGetBoolean called");
    return NULL;
}

void* CGPreferencesGetCGFloat(void)
{
    if (verbose) puts("STUB: CGPreferencesGetCGFloat called");
    return NULL;
}

void* CGPreferencesGetInteger(void)
{
    if (verbose) puts("STUB: CGPreferencesGetInteger called");
    return NULL;
}

void* CGRIPImageCachePrint(void)
{
    if (verbose) puts("STUB: CGRIPImageCachePrint called");
    return NULL;
}

void* CGRIPImageCachePurge(void)
{
    if (verbose) puts("STUB: CGRIPImageCachePurge called");
    return NULL;
}

void* CGRectApplyAffineTransform(void)
{
    if (verbose) puts("STUB: CGRectApplyAffineTransform called");
    return NULL;
}

void* CGRectApplyInverseAffineTransform(void)
{
    if (verbose) puts("STUB: CGRectApplyInverseAffineTransform called");
    return NULL;
}

void* CGRectContainsPoint(void)
{
    if (verbose) puts("STUB: CGRectContainsPoint called");
    return NULL;
}

void* CGRectContainsRect(void)
{
    if (verbose) puts("STUB: CGRectContainsRect called");
    return NULL;
}

void* CGRectCreateDictionaryRepresentation(void)
{
    if (verbose) puts("STUB: CGRectCreateDictionaryRepresentation called");
    return NULL;
}

void* CGRectDivide(void)
{
    if (verbose) puts("STUB: CGRectDivide called");
    return NULL;
}

void* CGRectEqualToRect(void)
{
    if (verbose) puts("STUB: CGRectEqualToRect called");
    return NULL;
}

void* CGRectGetHeight(void)
{
    if (verbose) puts("STUB: CGRectGetHeight called");
    return NULL;
}

void* CGRectGetMaxX(void)
{
    if (verbose) puts("STUB: CGRectGetMaxX called");
    return NULL;
}

void* CGRectGetMaxY(void)
{
    if (verbose) puts("STUB: CGRectGetMaxY called");
    return NULL;
}

void* CGRectGetMidX(void)
{
    if (verbose) puts("STUB: CGRectGetMidX called");
    return NULL;
}

void* CGRectGetMidY(void)
{
    if (verbose) puts("STUB: CGRectGetMidY called");
    return NULL;
}

void* CGRectGetMinX(void)
{
    if (verbose) puts("STUB: CGRectGetMinX called");
    return NULL;
}

void* CGRectGetMinY(void)
{
    if (verbose) puts("STUB: CGRectGetMinY called");
    return NULL;
}

void* CGRectGetWidth(void)
{
    if (verbose) puts("STUB: CGRectGetWidth called");
    return NULL;
}

void* CGRectInset(void)
{
    if (verbose) puts("STUB: CGRectInset called");
    return NULL;
}

void* CGRectIntegral(void)
{
    if (verbose) puts("STUB: CGRectIntegral called");
    return NULL;
}

void* CGRectIntersection(void)
{
    if (verbose) puts("STUB: CGRectIntersection called");
    return NULL;
}

void* CGRectIntersectsRect(void)
{
    if (verbose) puts("STUB: CGRectIntersectsRect called");
    return NULL;
}

void* CGRectIsEmpty(void)
{
    if (verbose) puts("STUB: CGRectIsEmpty called");
    return NULL;
}

void* CGRectIsInfinite(void)
{
    if (verbose) puts("STUB: CGRectIsInfinite called");
    return NULL;
}

void* CGRectIsIntegral(void)
{
    if (verbose) puts("STUB: CGRectIsIntegral called");
    return NULL;
}

void* CGRectIsNull(void)
{
    if (verbose) puts("STUB: CGRectIsNull called");
    return NULL;
}

void* CGRectMakeWithDictionaryRepresentation(void)
{
    if (verbose) puts("STUB: CGRectMakeWithDictionaryRepresentation called");
    return NULL;
}

void* CGRectOffset(void)
{
    if (verbose) puts("STUB: CGRectOffset called");
    return NULL;
}

void* CGRectStandardize(void)
{
    if (verbose) puts("STUB: CGRectStandardize called");
    return NULL;
}

void* CGRectUnion(void)
{
    if (verbose) puts("STUB: CGRectUnion called");
    return NULL;
}

void* CGRegionContainsPoint(void)
{
    if (verbose) puts("STUB: CGRegionContainsPoint called");
    return NULL;
}

void* CGRegionContainsRect(void)
{
    if (verbose) puts("STUB: CGRegionContainsRect called");
    return NULL;
}

void* CGRegionContainsRegion(void)
{
    if (verbose) puts("STUB: CGRegionContainsRegion called");
    return NULL;
}

void* CGRegionCopyData(void)
{
    if (verbose) puts("STUB: CGRegionCopyData called");
    return NULL;
}

void* CGRegionCreateByScalingRegion(void)
{
    if (verbose) puts("STUB: CGRegionCreateByScalingRegion called");
    return NULL;
}

void* CGRegionCreateBySimplifyingRegion(void)
{
    if (verbose) puts("STUB: CGRegionCreateBySimplifyingRegion called");
    return NULL;
}

void* CGRegionCreateByTransformingRegion(void)
{
    if (verbose) puts("STUB: CGRegionCreateByTransformingRegion called");
    return NULL;
}

void* CGRegionCreateCopy(void)
{
    if (verbose) puts("STUB: CGRegionCreateCopy called");
    return NULL;
}

void* CGRegionCreateDifferenceWithRegion(void)
{
    if (verbose) puts("STUB: CGRegionCreateDifferenceWithRegion called");
    return NULL;
}

void* CGRegionCreateEmptyRegion(void)
{
    if (verbose) puts("STUB: CGRegionCreateEmptyRegion called");
    return NULL;
}

void* CGRegionCreateIntersectionWithRect(void)
{
    if (verbose) puts("STUB: CGRegionCreateIntersectionWithRect called");
    return NULL;
}

void* CGRegionCreateIntersectionWithRegion(void)
{
    if (verbose) puts("STUB: CGRegionCreateIntersectionWithRegion called");
    return NULL;
}

void* CGRegionCreateUnionWithRect(void)
{
    if (verbose) puts("STUB: CGRegionCreateUnionWithRect called");
    return NULL;
}

void* CGRegionCreateUnionWithRegion(void)
{
    if (verbose) puts("STUB: CGRegionCreateUnionWithRegion called");
    return NULL;
}

void* CGRegionCreateWithBytes(void)
{
    if (verbose) puts("STUB: CGRegionCreateWithBytes called");
    return NULL;
}

void* CGRegionCreateWithData(void)
{
    if (verbose) puts("STUB: CGRegionCreateWithData called");
    return NULL;
}

void* CGRegionCreateWithInset(void)
{
    if (verbose) puts("STUB: CGRegionCreateWithInset called");
    return NULL;
}

void* CGRegionCreateWithOffset(void)
{
    if (verbose) puts("STUB: CGRegionCreateWithOffset called");
    return NULL;
}

void* CGRegionCreateWithRect(void)
{
    if (verbose) puts("STUB: CGRegionCreateWithRect called");
    return NULL;
}

void* CGRegionCreateWithRects(void)
{
    if (verbose) puts("STUB: CGRegionCreateWithRects called");
    return NULL;
}

void* CGRegionCreateXORWithRegion(void)
{
    if (verbose) puts("STUB: CGRegionCreateXORWithRegion called");
    return NULL;
}

void* CGRegionEnumeratorCreate(void)
{
    if (verbose) puts("STUB: CGRegionEnumeratorCreate called");
    return NULL;
}

void* CGRegionEnumeratorCreateWithDirection(void)
{
    if (verbose) puts("STUB: CGRegionEnumeratorCreateWithDirection called");
    return NULL;
}

void* CGRegionEnumeratorGetNextRect(void)
{
    if (verbose) puts("STUB: CGRegionEnumeratorGetNextRect called");
    return NULL;
}

void* CGRegionEnumeratorRelease(void)
{
    if (verbose) puts("STUB: CGRegionEnumeratorRelease called");
    return NULL;
}

void* CGRegionEqualToRegion(void)
{
    if (verbose) puts("STUB: CGRegionEqualToRegion called");
    return NULL;
}

void* CGRegionForwardEnumeratorGetNextBounds(void)
{
    if (verbose) puts("STUB: CGRegionForwardEnumeratorGetNextBounds called");
    return NULL;
}

void* CGRegionForwardEnumeratorGetNextRect(void)
{
    if (verbose) puts("STUB: CGRegionForwardEnumeratorGetNextRect called");
    return NULL;
}

void* CGRegionForwardEnumeratorMake(void)
{
    if (verbose) puts("STUB: CGRegionForwardEnumeratorMake called");
    return NULL;
}

void* CGRegionGetBoundingBox(void)
{
    if (verbose) puts("STUB: CGRegionGetBoundingBox called");
    return NULL;
}

void* CGRegionGetDataBytePtr(void)
{
    if (verbose) puts("STUB: CGRegionGetDataBytePtr called");
    return NULL;
}

void* CGRegionGetDataLength(void)
{
    if (verbose) puts("STUB: CGRegionGetDataLength called");
    return NULL;
}

void* CGRegionGetTypeID(void)
{
    if (verbose) puts("STUB: CGRegionGetTypeID called");
    return NULL;
}

void* CGRegionIntersectsRect(void)
{
    if (verbose) puts("STUB: CGRegionIntersectsRect called");
    return NULL;
}

void* CGRegionIntersectsRegion(void)
{
    if (verbose) puts("STUB: CGRegionIntersectsRegion called");
    return NULL;
}

void* CGRegionIsEmpty(void)
{
    if (verbose) puts("STUB: CGRegionIsEmpty called");
    return NULL;
}

void* CGRegionIsRect(void)
{
    if (verbose) puts("STUB: CGRegionIsRect called");
    return NULL;
}

void* CGRegionIsValid(void)
{
    if (verbose) puts("STUB: CGRegionIsValid called");
    return NULL;
}

void* CGRegionRelease(void)
{
    if (verbose) puts("STUB: CGRegionRelease called");
    return NULL;
}

void* CGRegionRetain(void)
{
    if (verbose) puts("STUB: CGRegionRetain called");
    return NULL;
}

void* CGRenderingStateCreate(void)
{
    if (verbose) puts("STUB: CGRenderingStateCreate called");
    return NULL;
}

void* CGRenderingStateCreateCopy(void)
{
    if (verbose) puts("STUB: CGRenderingStateCreateCopy called");
    return NULL;
}

void* CGRenderingStateGetAllowedFontRenderingStyle(void)
{
    if (verbose) puts("STUB: CGRenderingStateGetAllowedFontRenderingStyle called");
    return NULL;
}

void* CGRenderingStateGetAllowsAcceleration(void)
{
    if (verbose) puts("STUB: CGRenderingStateGetAllowsAcceleration called");
    return NULL;
}

void* CGRenderingStateGetAllowsAntialiasing(void)
{
    if (verbose) puts("STUB: CGRenderingStateGetAllowsAntialiasing called");
    return NULL;
}

void* CGRenderingStateGetAllowsDithering(void)
{
    if (verbose) puts("STUB: CGRenderingStateGetAllowsDithering called");
    return NULL;
}

void* CGRenderingStateGetAllowsFontAntialiasing(void)
{
    if (verbose) puts("STUB: CGRenderingStateGetAllowsFontAntialiasing called");
    return NULL;
}

void* CGRenderingStateGetAllowsFontSmoothing(void)
{
    if (verbose) puts("STUB: CGRenderingStateGetAllowsFontSmoothing called");
    return NULL;
}

void* CGRenderingStateGetAllowsFontSubpixelPositioning(void)
{
    if (verbose) puts("STUB: CGRenderingStateGetAllowsFontSubpixelPositioning called");
    return NULL;
}

void* CGRenderingStateGetAllowsFontSubpixelQuantization(void)
{
    if (verbose) puts("STUB: CGRenderingStateGetAllowsFontSubpixelQuantization called");
    return NULL;
}

void* CGRenderingStateGetBaseCTM(void)
{
    if (verbose) puts("STUB: CGRenderingStateGetBaseCTM called");
    return NULL;
}

void* CGRenderingStateGetFontRenderingStyle(void)
{
    if (verbose) puts("STUB: CGRenderingStateGetFontRenderingStyle called");
    return NULL;
}

void* CGRenderingStateGetMaxInterpolationQuality(void)
{
    if (verbose) puts("STUB: CGRenderingStateGetMaxInterpolationQuality called");
    return NULL;
}

void* CGRenderingStateGetMinInterpolationQuality(void)
{
    if (verbose) puts("STUB: CGRenderingStateGetMinInterpolationQuality called");
    return NULL;
}

void* CGRenderingStateGetRenderingResolution(void)
{
    if (verbose) puts("STUB: CGRenderingStateGetRenderingResolution called");
    return NULL;
}

void* CGRenderingStateRelease(void)
{
    if (verbose) puts("STUB: CGRenderingStateRelease called");
    return NULL;
}

void* CGRenderingStateReset(void)
{
    if (verbose) puts("STUB: CGRenderingStateReset called");
    return NULL;
}

void* CGRenderingStateRetain(void)
{
    if (verbose) puts("STUB: CGRenderingStateRetain called");
    return NULL;
}

void* CGRenderingStateSetAllowsAcceleration(void)
{
    if (verbose) puts("STUB: CGRenderingStateSetAllowsAcceleration called");
    return NULL;
}

void* CGRenderingStateSetAllowsAntialiasing(void)
{
    if (verbose) puts("STUB: CGRenderingStateSetAllowsAntialiasing called");
    return NULL;
}

void* CGRenderingStateSetAllowsDithering(void)
{
    if (verbose) puts("STUB: CGRenderingStateSetAllowsDithering called");
    return NULL;
}

void* CGRenderingStateSetAllowsFontAntialiasing(void)
{
    if (verbose) puts("STUB: CGRenderingStateSetAllowsFontAntialiasing called");
    return NULL;
}

void* CGRenderingStateSetAllowsFontSmoothing(void)
{
    if (verbose) puts("STUB: CGRenderingStateSetAllowsFontSmoothing called");
    return NULL;
}

void* CGRenderingStateSetAllowsFontSubpixelPositioning(void)
{
    if (verbose) puts("STUB: CGRenderingStateSetAllowsFontSubpixelPositioning called");
    return NULL;
}

void* CGRenderingStateSetAllowsFontSubpixelQuantization(void)
{
    if (verbose) puts("STUB: CGRenderingStateSetAllowsFontSubpixelQuantization called");
    return NULL;
}

void* CGRenderingStateSetBaseCTM(void)
{
    if (verbose) puts("STUB: CGRenderingStateSetBaseCTM called");
    return NULL;
}

void* CGRenderingStateSetMaxInterpolationQuality(void)
{
    if (verbose) puts("STUB: CGRenderingStateSetMaxInterpolationQuality called");
    return NULL;
}

void* CGRenderingStateSetMinInterpolationQuality(void)
{
    if (verbose) puts("STUB: CGRenderingStateSetMinInterpolationQuality called");
    return NULL;
}

void* CGRenderingStateSetRenderingResolution(void)
{
    if (verbose) puts("STUB: CGRenderingStateSetRenderingResolution called");
    return NULL;
}

void* CGRenderingStateSynchronize(void)
{
    if (verbose) puts("STUB: CGRenderingStateSynchronize called");
    return NULL;
}

void* CGSBoundingShapeAdd(void)
{
    if (verbose) puts("STUB: CGSBoundingShapeAdd called");
    return NULL;
}

void* CGSBoundingShapeAddRect(void)
{
    if (verbose) puts("STUB: CGSBoundingShapeAddRect called");
    return NULL;
}

void* CGSBoundingShapeAddRegion(void)
{
    if (verbose) puts("STUB: CGSBoundingShapeAddRegion called");
    return NULL;
}

void* CGSBoundingShapeAddRegionWithScale(void)
{
    if (verbose) puts("STUB: CGSBoundingShapeAddRegionWithScale called");
    return NULL;
}

void* CGSBoundingShapeCreate(void)
{
    if (verbose) puts("STUB: CGSBoundingShapeCreate called");
    return NULL;
}

void* CGSBoundingShapeGetBounds(void)
{
    if (verbose) puts("STUB: CGSBoundingShapeGetBounds called");
    return NULL;
}

void* CGSBoundingShapeGetRegion(void)
{
    if (verbose) puts("STUB: CGSBoundingShapeGetRegion called");
    return NULL;
}

void* CGSBoundingShapeRelease(void)
{
    if (verbose) puts("STUB: CGSBoundingShapeRelease called");
    return NULL;
}

void* CGSBoundingShapeReset(void)
{
    if (verbose) puts("STUB: CGSBoundingShapeReset called");
    return NULL;
}

void* CGSBoundsEqualToBounds(void)
{
    if (verbose) puts("STUB: CGSBoundsEqualToBounds called");
    return NULL;
}

void* CGSBoundsIntersection(void)
{
    if (verbose) puts("STUB: CGSBoundsIntersection called");
    return NULL;
}

void* CGSBoundsUnion(void)
{
    if (verbose) puts("STUB: CGSBoundsUnion called");
    return NULL;
}

void* CGSColorMaskCopyARGB8888(void)
{
    if (verbose) puts("STUB: CGSColorMaskCopyARGB8888 called");
    return NULL;
}

void* CGSConvertBGRA8888toRGBA8888(void)
{
    if (verbose) puts("STUB: CGSConvertBGRA8888toRGBA8888 called");
    return NULL;
}

void* CGSConvertBitsToMask(void)
{
    if (verbose) puts("STUB: CGSConvertBitsToMask called");
    return NULL;
}

void* CGSConvertRGB555toXRGB8888(void)
{
    if (verbose) puts("STUB: CGSConvertRGB555toXRGB8888 called");
    return NULL;
}

void* CGSConvertW8toRGBA8888(void)
{
    if (verbose) puts("STUB: CGSConvertW8toRGBA8888 called");
    return NULL;
}

void* CGSConvertXRGB8888toARGB8888(void)
{
    if (verbose) puts("STUB: CGSConvertXRGB8888toARGB8888 called");
    return NULL;
}

void* CGSCopyRegion(void)
{
    if (verbose) puts("STUB: CGSCopyRegion called");
    return NULL;
}

void* CGSCreateRegionFromData(void)
{
    if (verbose) puts("STUB: CGSCreateRegionFromData called");
    return NULL;
}

void* CGSDiffRegion(void)
{
    if (verbose) puts("STUB: CGSDiffRegion called");
    return NULL;
}

void* CGSGetQDRgnLength(void)
{
    if (verbose) puts("STUB: CGSGetQDRgnLength called");
    return NULL;
}

void* CGSGetRegionBounds(void)
{
    if (verbose) puts("STUB: CGSGetRegionBounds called");
    return NULL;
}

void* CGSGetRegionData(void)
{
    if (verbose) puts("STUB: CGSGetRegionData called");
    return NULL;
}

void* CGSGetRegionRectCount(void)
{
    if (verbose) puts("STUB: CGSGetRegionRectCount called");
    return NULL;
}

void* CGSInsetRegion(void)
{
    if (verbose) puts("STUB: CGSInsetRegion called");
    return NULL;
}

void* CGSIntersectRegion(void)
{
    if (verbose) puts("STUB: CGSIntersectRegion called");
    return NULL;
}

void* CGSIntersectRegionWithRect(void)
{
    if (verbose) puts("STUB: CGSIntersectRegionWithRect called");
    return NULL;
}

void* CGSNewEmptyRegion(void)
{
    if (verbose) puts("STUB: CGSNewEmptyRegion called");
    return NULL;
}

void* CGSNewRegionWithData(void)
{
    if (verbose) puts("STUB: CGSNewRegionWithData called");
    return NULL;
}

void* CGSNewRegionWithQDRgn(void)
{
    if (verbose) puts("STUB: CGSNewRegionWithQDRgn called");
    return NULL;
}

void* CGSNewRegionWithRect(void)
{
    if (verbose) puts("STUB: CGSNewRegionWithRect called");
    return NULL;
}

void* CGSNewRegionWithRectList(void)
{
    if (verbose) puts("STUB: CGSNewRegionWithRectList called");
    return NULL;
}

void* CGSNextPoint(void)
{
    if (verbose) puts("STUB: CGSNextPoint called");
    return NULL;
}

void* CGSNextRect(void)
{
    if (verbose) puts("STUB: CGSNextRect called");
    return NULL;
}

void* CGSOffsetRegion(void)
{
    if (verbose) puts("STUB: CGSOffsetRegion called");
    return NULL;
}

void* CGSPointInRegion(void)
{
    if (verbose) puts("STUB: CGSPointInRegion called");
    return NULL;
}

void* CGSQuadrangleToRegion(void)
{
    if (verbose) puts("STUB: CGSQuadrangleToRegion called");
    return NULL;
}

void* CGSRectInRegion(void)
{
    if (verbose) puts("STUB: CGSRectInRegion called");
    return NULL;
}

void* CGSRegionCreateWithBoundingShape(void)
{
    if (verbose) puts("STUB: CGSRegionCreateWithBoundingShape called");
    return NULL;
}

void* CGSRegionEnumerator(void)
{
    if (verbose) puts("STUB: CGSRegionEnumerator called");
    return NULL;
}

void* CGSRegionEnumeratorWithDirection(void)
{
    if (verbose) puts("STUB: CGSRegionEnumeratorWithDirection called");
    return NULL;
}

void* CGSRegionFromBitmap(void)
{
    if (verbose) puts("STUB: CGSRegionFromBitmap called");
    return NULL;
}

void* CGSRegionGetShape(void)
{
    if (verbose) puts("STUB: CGSRegionGetShape called");
    return NULL;
}

void* CGSRegionInRegion(void)
{
    if (verbose) puts("STUB: CGSRegionInRegion called");
    return NULL;
}

void* CGSRegionIntersectsRect(void)
{
    if (verbose) puts("STUB: CGSRegionIntersectsRect called");
    return NULL;
}

void* CGSRegionIntersectsRegion(void)
{
    if (verbose) puts("STUB: CGSRegionIntersectsRegion called");
    return NULL;
}

void* CGSRegionIsEmpty(void)
{
    if (verbose) puts("STUB: CGSRegionIsEmpty called");
    return NULL;
}

void* CGSRegionIsRectangular(void)
{
    if (verbose) puts("STUB: CGSRegionIsRectangular called");
    return NULL;
}

void* CGSRegionMergeFlushAndMoveRegions(void)
{
    if (verbose) puts("STUB: CGSRegionMergeFlushAndMoveRegions called");
    return NULL;
}

void* CGSRegionPathEnumerator(void)
{
    if (verbose) puts("STUB: CGSRegionPathEnumerator called");
    return NULL;
}

void* CGSRegionRelease(void)
{
    if (verbose) puts("STUB: CGSRegionRelease called");
    return NULL;
}

void* CGSRegionRetain(void)
{
    if (verbose) puts("STUB: CGSRegionRetain called");
    return NULL;
}

void* CGSRegionToQDRgn(void)
{
    if (verbose) puts("STUB: CGSRegionToQDRgn called");
    return NULL;
}

void* CGSRegionsEqual(void)
{
    if (verbose) puts("STUB: CGSRegionsEqual called");
    return NULL;
}

void* CGSReleaseRegion(void)
{
    if (verbose) puts("STUB: CGSReleaseRegion called");
    return NULL;
}

void* CGSReleaseRegionEnumerator(void)
{
    if (verbose) puts("STUB: CGSReleaseRegionEnumerator called");
    return NULL;
}

void* CGSSetShouldLogErrors(void)
{
    if (verbose) puts("STUB: CGSSetShouldLogErrors called");
    return NULL;
}

void* CGSSetShouldLogMessages(void)
{
    if (verbose) puts("STUB: CGSSetShouldLogMessages called");
    return NULL;
}

void* CGSTransformRegion(void)
{
    if (verbose) puts("STUB: CGSTransformRegion called");
    return NULL;
}

void* CGSUnionRegion(void)
{
    if (verbose) puts("STUB: CGSUnionRegion called");
    return NULL;
}

void* CGSUnionRegionWithRect(void)
{
    if (verbose) puts("STUB: CGSUnionRegionWithRect called");
    return NULL;
}

void* CGSXorRegion(void)
{
    if (verbose) puts("STUB: CGSXorRegion called");
    return NULL;
}

void* CGShadingCreateAxial(void)
{
    if (verbose) puts("STUB: CGShadingCreateAxial called");
    return NULL;
}

void* CGShadingCreateConic(void)
{
    if (verbose) puts("STUB: CGShadingCreateConic called");
    return NULL;
}

void* CGShadingCreateRadial(void)
{
    if (verbose) puts("STUB: CGShadingCreateRadial called");
    return NULL;
}

void* CGShadingCreateWithCoreImage(void)
{
    if (verbose) puts("STUB: CGShadingCreateWithCoreImage called");
    return NULL;
}

void* CGShadingDelegateDrawShading(void)
{
    if (verbose) puts("STUB: CGShadingDelegateDrawShading called");
    return NULL;
}

void* CGShadingDrawInContextDelegate(void)
{
    if (verbose) puts("STUB: CGShadingDrawInContextDelegate called");
    return NULL;
}

void* CGShadingGetAssociate(void)
{
    if (verbose) puts("STUB: CGShadingGetAssociate called");
    return NULL;
}

void* CGShadingGetBackground(void)
{
    if (verbose) puts("STUB: CGShadingGetBackground called");
    return NULL;
}

void* CGShadingGetBounds(void)
{
    if (verbose) puts("STUB: CGShadingGetBounds called");
    return NULL;
}

void* CGShadingGetColorSpace(void)
{
    if (verbose) puts("STUB: CGShadingGetColorSpace called");
    return NULL;
}

void* CGShadingGetDescriptor(void)
{
    if (verbose) puts("STUB: CGShadingGetDescriptor called");
    return NULL;
}

void* CGShadingGetIdentifier(void)
{
    if (verbose) puts("STUB: CGShadingGetIdentifier called");
    return NULL;
}

void* CGShadingGetType(void)
{
    if (verbose) puts("STUB: CGShadingGetType called");
    return NULL;
}

void* CGShadingGetTypeID(void)
{
    if (verbose) puts("STUB: CGShadingGetTypeID called");
    return NULL;
}

void* CGShadingRelease(void)
{
    if (verbose) puts("STUB: CGShadingRelease called");
    return NULL;
}

void* CGShadingRetain(void)
{
    if (verbose) puts("STUB: CGShadingRetain called");
    return NULL;
}

void* CGShadingSetAssociate(void)
{
    if (verbose) puts("STUB: CGShadingSetAssociate called");
    return NULL;
}

void* CGSizeApplyAffineTransform(void)
{
    if (verbose) puts("STUB: CGSizeApplyAffineTransform called");
    return NULL;
}

void* CGSizeCreateDictionaryRepresentation(void)
{
    if (verbose) puts("STUB: CGSizeCreateDictionaryRepresentation called");
    return NULL;
}

void* CGSizeEqualToSize(void)
{
    if (verbose) puts("STUB: CGSizeEqualToSize called");
    return NULL;
}

void* CGSizeMakeWithDictionaryRepresentation(void)
{
    if (verbose) puts("STUB: CGSizeMakeWithDictionaryRepresentation called");
    return NULL;
}

void* CGSoftMaskCreate(void)
{
    if (verbose) puts("STUB: CGSoftMaskCreate called");
    return NULL;
}

void* CGSoftMaskDelegateDrawSoftMask(void)
{
    if (verbose) puts("STUB: CGSoftMaskDelegateDrawSoftMask called");
    return NULL;
}

void* CGSoftMaskEqual(void)
{
    if (verbose) puts("STUB: CGSoftMaskEqual called");
    return NULL;
}

void* CGSoftMaskGetBackground(void)
{
    if (verbose) puts("STUB: CGSoftMaskGetBackground called");
    return NULL;
}

void* CGSoftMaskGetBounds(void)
{
    if (verbose) puts("STUB: CGSoftMaskGetBounds called");
    return NULL;
}

void* CGSoftMaskGetIdentifier(void)
{
    if (verbose) puts("STUB: CGSoftMaskGetIdentifier called");
    return NULL;
}

void* CGSoftMaskGetMatrix(void)
{
    if (verbose) puts("STUB: CGSoftMaskGetMatrix called");
    return NULL;
}

void* CGSoftMaskGetTransfer(void)
{
    if (verbose) puts("STUB: CGSoftMaskGetTransfer called");
    return NULL;
}

void* CGSoftMaskGetTypeID(void)
{
    if (verbose) puts("STUB: CGSoftMaskGetTypeID called");
    return NULL;
}

void* CGSoftMaskRelease(void)
{
    if (verbose) puts("STUB: CGSoftMaskRelease called");
    return NULL;
}

void* CGSoftMaskRetain(void)
{
    if (verbose) puts("STUB: CGSoftMaskRetain called");
    return NULL;
}

void* CGStyleCreate(void)
{
    if (verbose) puts("STUB: CGStyleCreate called");
    return NULL;
}

void* CGStyleCreateFocusRing(void)
{
    if (verbose) puts("STUB: CGStyleCreateFocusRing called");
    return NULL;
}

void* CGStyleCreateFocusRingWithColor(void)
{
    if (verbose) puts("STUB: CGStyleCreateFocusRingWithColor called");
    return NULL;
}

void* CGStyleCreateShadow(void)
{
    if (verbose) puts("STUB: CGStyleCreateShadow called");
    return NULL;
}

void* CGStyleCreateShadow2(void)
{
    if (verbose) puts("STUB: CGStyleCreateShadow2 called");
    return NULL;
}

void* CGStyleCreateShadowWithColor(void)
{
    if (verbose) puts("STUB: CGStyleCreateShadowWithColor called");
    return NULL;
}

void* CGStyleEqualToStyle(void)
{
    if (verbose) puts("STUB: CGStyleEqualToStyle called");
    return NULL;
}

void* CGStyleFilterContextDelegateCreate(void)
{
    if (verbose) puts("STUB: CGStyleFilterContextDelegateCreate called");
    return NULL;
}

void* CGStyleGetColor(void)
{
    if (verbose) puts("STUB: CGStyleGetColor called");
    return NULL;
}

void* CGStyleGetData(void)
{
    if (verbose) puts("STUB: CGStyleGetData called");
    return NULL;
}

void* CGStyleGetDrawBoundingBox(void)
{
    if (verbose) puts("STUB: CGStyleGetDrawBoundingBox called");
    return NULL;
}

void* CGStyleGetDrawOrdering(void)
{
    if (verbose) puts("STUB: CGStyleGetDrawOrdering called");
    return NULL;
}

void* CGStyleGetIdentifier(void)
{
    if (verbose) puts("STUB: CGStyleGetIdentifier called");
    return NULL;
}

void* CGStyleGetType(void)
{
    if (verbose) puts("STUB: CGStyleGetType called");
    return NULL;
}

void* CGStyleGetTypeID(void)
{
    if (verbose) puts("STUB: CGStyleGetTypeID called");
    return NULL;
}

void* CGStyleRelease(void)
{
    if (verbose) puts("STUB: CGStyleRelease called");
    return NULL;
}

void* CGStyleRetain(void)
{
    if (verbose) puts("STUB: CGStyleRetain called");
    return NULL;
}

void* CGStyleShouldUseCorrectShadowOffsets(void)
{
    if (verbose) puts("STUB: CGStyleShouldUseCorrectShadowOffsets called");
    return NULL;
}

void* CGTextClippingCreatePath(void)
{
    if (verbose) puts("STUB: CGTextClippingCreatePath called");
    return NULL;
}

void* CGTriangleMeshAddTriangle(void)
{
    if (verbose) puts("STUB: CGTriangleMeshAddTriangle called");
    return NULL;
}

void* CGTriangleMeshAddTriangleWithRelativePoint(void)
{
    if (verbose) puts("STUB: CGTriangleMeshAddTriangleWithRelativePoint called");
    return NULL;
}

void* CGTriangleMeshAddTriangleWithTrianglePoints(void)
{
    if (verbose) puts("STUB: CGTriangleMeshAddTriangleWithTrianglePoints called");
    return NULL;
}

void* CGTriangleMeshCreate(void)
{
    if (verbose) puts("STUB: CGTriangleMeshCreate called");
    return NULL;
}

void* CGTriangleMeshGetColorSpace(void)
{
    if (verbose) puts("STUB: CGTriangleMeshGetColorSpace called");
    return NULL;
}

void* CGTriangleMeshGetCount(void)
{
    if (verbose) puts("STUB: CGTriangleMeshGetCount called");
    return NULL;
}

void* CGTriangleMeshGetNumberOfComponents(void)
{
    if (verbose) puts("STUB: CGTriangleMeshGetNumberOfComponents called");
    return NULL;
}

void* CGTriangleMeshGetTriangleAtIndex(void)
{
    if (verbose) puts("STUB: CGTriangleMeshGetTriangleAtIndex called");
    return NULL;
}

void* CGTriangleMeshPrint(void)
{
    if (verbose) puts("STUB: CGTriangleMeshPrint called");
    return NULL;
}

void* CGTriangleMeshRelease(void)
{
    if (verbose) puts("STUB: CGTriangleMeshRelease called");
    return NULL;
}

void* CGTriangleMeshRetain(void)
{
    if (verbose) puts("STUB: CGTriangleMeshRetain called");
    return NULL;
}

void* CGTypeCreateInstance(void)
{
    if (verbose) puts("STUB: CGTypeCreateInstance called");
    return NULL;
}

void* CGUpdateConvolveKernelBlue(void)
{
    if (verbose) puts("STUB: CGUpdateConvolveKernelBlue called");
    return NULL;
}

void* CGUpdateConvolveKernelGreen(void)
{
    if (verbose) puts("STUB: CGUpdateConvolveKernelGreen called");
    return NULL;
}

void* CGUpdateConvolveKernelMonochrome(void)
{
    if (verbose) puts("STUB: CGUpdateConvolveKernelMonochrome called");
    return NULL;
}

void* CGUpdateConvolveKernelRed(void)
{
    if (verbose) puts("STUB: CGUpdateConvolveKernelRed called");
    return NULL;
}

void* __CGTypeRegister(void)
{
    if (verbose) puts("STUB: __CGTypeRegister called");
    return NULL;
}

void* __CGWindowContextDelegateCreate(void)
{
    if (verbose) puts("STUB: __CGWindowContextDelegateCreate called");
    return NULL;
}

void* aa_clipping(void)
{
    if (verbose) puts("STUB: aa_clipping called");
    return NULL;
}

void* aa_closepath(void)
{
    if (verbose) puts("STUB: aa_closepath called");
    return NULL;
}

void* aa_create(void)
{
    if (verbose) puts("STUB: aa_create called");
    return NULL;
}

void* aa_cubeto(void)
{
    if (verbose) puts("STUB: aa_cubeto called");
    return NULL;
}

void* aa_destroy(void)
{
    if (verbose) puts("STUB: aa_destroy called");
    return NULL;
}

void* aa_flatness(void)
{
    if (verbose) puts("STUB: aa_flatness called");
    return NULL;
}

void* aa_geometry(void)
{
    if (verbose) puts("STUB: aa_geometry called");
    return NULL;
}

void* aa_line_render(void)
{
    if (verbose) puts("STUB: aa_line_render called");
    return NULL;
}

void* aa_line_render_shape(void)
{
    if (verbose) puts("STUB: aa_line_render_shape called");
    return NULL;
}

void* aa_lineto(void)
{
    if (verbose) puts("STUB: aa_lineto called");
    return NULL;
}

void* aa_moveto(void)
{
    if (verbose) puts("STUB: aa_moveto called");
    return NULL;
}

void* aa_newpath(void)
{
    if (verbose) puts("STUB: aa_newpath called");
    return NULL;
}

void* aa_quadto(void)
{
    if (verbose) puts("STUB: aa_quadto called");
    return NULL;
}

void* aa_rectat(void)
{
    if (verbose) puts("STUB: aa_rectat called");
    return NULL;
}

void* aa_render(void)
{
    if (verbose) puts("STUB: aa_render called");
    return NULL;
}

void* aa_render_shape(void)
{
    if (verbose) puts("STUB: aa_render_shape called");
    return NULL;
}

void* aal_clipping(void)
{
    if (verbose) puts("STUB: aal_clipping called");
    return NULL;
}

void* aal_create(void)
{
    if (verbose) puts("STUB: aal_create called");
    return NULL;
}

void* aal_destroy(void)
{
    if (verbose) puts("STUB: aal_destroy called");
    return NULL;
}

void* aal_flatness(void)
{
    if (verbose) puts("STUB: aal_flatness called");
    return NULL;
}

void* aal_geometry(void)
{
    if (verbose) puts("STUB: aal_geometry called");
    return NULL;
}

void* aal_newpath(void)
{
    if (verbose) puts("STUB: aal_newpath called");
    return NULL;
}

void* decode_create(void)
{
    if (verbose) puts("STUB: decode_create called");
    return NULL;
}

void* decode_data(void)
{
    if (verbose) puts("STUB: decode_data called");
    return NULL;
}

void* decode_destroy(void)
{
    if (verbose) puts("STUB: decode_destroy called");
    return NULL;
}

void* path_iterator_aaline_create(void)
{
    if (verbose) puts("STUB: path_iterator_aaline_create called");
    return NULL;
}

void* path_iterator_add_curve_to_point(void)
{
    if (verbose) puts("STUB: path_iterator_add_curve_to_point called");
    return NULL;
}

void* path_iterator_add_line_to_point(void)
{
    if (verbose) puts("STUB: path_iterator_add_line_to_point called");
    return NULL;
}

void* path_iterator_add_quad_curve_to_point(void)
{
    if (verbose) puts("STUB: path_iterator_add_quad_curve_to_point called");
    return NULL;
}

void* path_iterator_begin(void)
{
    if (verbose) puts("STUB: path_iterator_begin called");
    return NULL;
}

void* path_iterator_call(void)
{
    if (verbose) puts("STUB: path_iterator_call called");
    return NULL;
}

void* path_iterator_close_subpath(void)
{
    if (verbose) puts("STUB: path_iterator_close_subpath called");
    return NULL;
}

void* path_iterator_create(void)
{
    if (verbose) puts("STUB: path_iterator_create called");
    return NULL;
}

void* path_iterator_dasher_create(void)
{
    if (verbose) puts("STUB: path_iterator_dasher_create called");
    return NULL;
}

void* path_iterator_end(void)
{
    if (verbose) puts("STUB: path_iterator_end called");
    return NULL;
}

void* path_iterator_move_to_point(void)
{
    if (verbose) puts("STUB: path_iterator_move_to_point called");
    return NULL;
}

void* path_iterator_release(void)
{
    if (verbose) puts("STUB: path_iterator_release called");
    return NULL;
}

void* path_iterator_stroker_create(void)
{
    if (verbose) puts("STUB: path_iterator_stroker_create called");
    return NULL;
}

void* rect_to_bounds(void)
{
    if (verbose) puts("STUB: rect_to_bounds called");
    return NULL;
}

void* region_create_with_shape(void)
{
    if (verbose) puts("STUB: region_create_with_shape called");
    return NULL;
}

void* shape_alloc_bounds(void)
{
    if (verbose) puts("STUB: shape_alloc_bounds called");
    return NULL;
}

void* shape_alloc_data(void)
{
    if (verbose) puts("STUB: shape_alloc_data called");
    return NULL;
}

void* shape_alloc_qdrgn(void)
{
    if (verbose) puts("STUB: shape_alloc_qdrgn called");
    return NULL;
}

void* shape_bounds(void)
{
    if (verbose) puts("STUB: shape_bounds called");
    return NULL;
}

void* shape_combine(void)
{
    if (verbose) puts("STUB: shape_combine called");
    return NULL;
}

void* shape_contains_bounds(void)
{
    if (verbose) puts("STUB: shape_contains_bounds called");
    return NULL;
}

void* shape_contains_point(void)
{
    if (verbose) puts("STUB: shape_contains_point called");
    return NULL;
}

void* shape_copy(void)
{
    if (verbose) puts("STUB: shape_copy called");
    return NULL;
}

void* shape_count(void)
{
    if (verbose) puts("STUB: shape_count called");
    return NULL;
}

void* shape_difference(void)
{
    if (verbose) puts("STUB: shape_difference called");
    return NULL;
}

void* shape_empty(void)
{
    if (verbose) puts("STUB: shape_empty called");
    return NULL;
}

void* shape_enclose_with_bounds(void)
{
    if (verbose) puts("STUB: shape_enclose_with_bounds called");
    return NULL;
}

void* shape_enum_alloc(void)
{
    if (verbose) puts("STUB: shape_enum_alloc called");
    return NULL;
}

void* shape_enum_binary_clean_up(void)
{
    if (verbose) puts("STUB: shape_enum_binary_clean_up called");
    return NULL;
}

void* shape_enum_binary_init(void)
{
    if (verbose) puts("STUB: shape_enum_binary_init called");
    return NULL;
}

void* shape_enum_binary_next(void)
{
    if (verbose) puts("STUB: shape_enum_binary_next called");
    return NULL;
}

void* shape_enum_clip_alloc(void)
{
    if (verbose) puts("STUB: shape_enum_clip_alloc called");
    return NULL;
}

void* shape_enum_clip_next(void)
{
    if (verbose) puts("STUB: shape_enum_clip_next called");
    return NULL;
}

void* shape_enum_clip_scan(void)
{
    if (verbose) puts("STUB: shape_enum_clip_scan called");
    return NULL;
}

void* shape_enum_free(void)
{
    if (verbose) puts("STUB: shape_enum_free called");
    return NULL;
}

void* shape_enum_next(void)
{
    if (verbose) puts("STUB: shape_enum_next called");
    return NULL;
}

void* shape_forward_enum(void)
{
    if (verbose) puts("STUB: shape_forward_enum called");
    return NULL;
}

void* shape_forward_enum_next(void)
{
    if (verbose) puts("STUB: shape_forward_enum_next called");
    return NULL;
}

void* shape_free(void)
{
    if (verbose) puts("STUB: shape_free called");
    return NULL;
}

void* shape_inset(void)
{
    if (verbose) puts("STUB: shape_inset called");
    return NULL;
}

void* shape_intersect(void)
{
    if (verbose) puts("STUB: shape_intersect called");
    return NULL;
}

void* shape_intersect_with_bounds(void)
{
    if (verbose) puts("STUB: shape_intersect_with_bounds called");
    return NULL;
}

void* shape_is_empty(void)
{
    if (verbose) puts("STUB: shape_is_empty called");
    return NULL;
}

void* shape_is_rectangular(void)
{
    if (verbose) puts("STUB: shape_is_rectangular called");
    return NULL;
}

void* shape_length(void)
{
    if (verbose) puts("STUB: shape_length called");
    return NULL;
}

void* shape_offset(void)
{
    if (verbose) puts("STUB: shape_offset called");
    return NULL;
}

void* shape_qdrgn_alloc(void)
{
    if (verbose) puts("STUB: shape_qdrgn_alloc called");
    return NULL;
}

void* shape_qdrgn_free(void)
{
    if (verbose) puts("STUB: shape_qdrgn_free called");
    return NULL;
}

void* shape_union(void)
{
    if (verbose) puts("STUB: shape_union called");
    return NULL;
}

void* shape_xor(void)
{
    if (verbose) puts("STUB: shape_xor called");
    return NULL;
}
