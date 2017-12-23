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


#include <CoreVideo/CoreVideo.h>
#include <stdlib.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* CVBufferDumpToQTES(void) {
    if (verbose) puts("STUB: CVBufferDumpToQTES called");
    return NULL;
}

void* CVBufferGetAttachment(void) {
    if (verbose) puts("STUB: CVBufferGetAttachment called");
    return NULL;
}

void* CVBufferGetAttachments(void) {
    if (verbose) puts("STUB: CVBufferGetAttachments called");
    return NULL;
}

void* CVBufferPropagateAttachments(void) {
    if (verbose) puts("STUB: CVBufferPropagateAttachments called");
    return NULL;
}

void* CVBufferRelease(void) {
    if (verbose) puts("STUB: CVBufferRelease called");
    return NULL;
}

void* CVBufferRemoveAllAttachments(void) {
    if (verbose) puts("STUB: CVBufferRemoveAllAttachments called");
    return NULL;
}

void* CVBufferRemoveAttachment(void) {
    if (verbose) puts("STUB: CVBufferRemoveAttachment called");
    return NULL;
}

void* CVBufferRetain(void) {
    if (verbose) puts("STUB: CVBufferRetain called");
    return NULL;
}

void* CVBufferSetAttachment(void) {
    if (verbose) puts("STUB: CVBufferSetAttachment called");
    return NULL;
}

void* CVBufferSetAttachments(void) {
    if (verbose) puts("STUB: CVBufferSetAttachments called");
    return NULL;
}

void* CVColorPrimariesGetIntegerCodePointForString(void) {
    if (verbose) puts("STUB: CVColorPrimariesGetIntegerCodePointForString called");
    return NULL;
}

void* CVColorPrimariesGetStringForIntegerCodePoint(void) {
    if (verbose) puts("STUB: CVColorPrimariesGetStringForIntegerCodePoint called");
    return NULL;
}

void* CVDisplayLinkCreateWithActiveCGDisplays(void) {
    if (verbose) puts("STUB: CVDisplayLinkCreateWithActiveCGDisplays called");
    return NULL;
}

void* CVDisplayLinkCreateWithCGDisplay(void) {
    if (verbose) puts("STUB: CVDisplayLinkCreateWithCGDisplay called");
    return NULL;
}

void* CVDisplayLinkCreateWithCGDisplays(void) {
    if (verbose) puts("STUB: CVDisplayLinkCreateWithCGDisplays called");
    return NULL;
}

void* CVDisplayLinkCreateWithDisplay(void) {
    if (verbose) puts("STUB: CVDisplayLinkCreateWithDisplay called");
    return NULL;
}

void* CVDisplayLinkCreateWithDisplays(void) {
    if (verbose) puts("STUB: CVDisplayLinkCreateWithDisplays called");
    return NULL;
}

void* CVDisplayLinkCreateWithOpenGLDisplayMask(void) {
    if (verbose) puts("STUB: CVDisplayLinkCreateWithOpenGLDisplayMask called");
    return NULL;
}

void* CVDisplayLinkGetActualOutputVideoRefreshPeriod(void) {
    if (verbose) puts("STUB: CVDisplayLinkGetActualOutputVideoRefreshPeriod called");
    return NULL;
}

void* CVDisplayLinkGetCurrentCGDisplay(void) {
    if (verbose) puts("STUB: CVDisplayLinkGetCurrentCGDisplay called");
    return NULL;
}

void* CVDisplayLinkGetCurrentDisplay(void) {
    if (verbose) puts("STUB: CVDisplayLinkGetCurrentDisplay called");
    return NULL;
}

void* CVDisplayLinkGetCurrentTime(void) {
    if (verbose) puts("STUB: CVDisplayLinkGetCurrentTime called");
    return NULL;
}

void* CVDisplayLinkGetNominalOutputVideoRefreshPeriod(void) {
    if (verbose) puts("STUB: CVDisplayLinkGetNominalOutputVideoRefreshPeriod called");
    return NULL;
}

void* CVDisplayLinkGetOutputVideoLatency(void) {
    if (verbose) puts("STUB: CVDisplayLinkGetOutputVideoLatency called");
    return NULL;
}

void* CVDisplayLinkGetTypeID(void) {
    if (verbose) puts("STUB: CVDisplayLinkGetTypeID called");
    return NULL;
}

void* CVDisplayLinkIsRunning(void) {
    if (verbose) puts("STUB: CVDisplayLinkIsRunning called");
    return NULL;
}

void* CVDisplayLinkRelease(void) {
    if (verbose) puts("STUB: CVDisplayLinkRelease called");
    return NULL;
}

void* CVDisplayLinkRetain(void) {
    if (verbose) puts("STUB: CVDisplayLinkRetain called");
    return NULL;
}

void* CVDisplayLinkSetCurrentCGDisplay(void) {
    if (verbose) puts("STUB: CVDisplayLinkSetCurrentCGDisplay called");
    return NULL;
}

void* CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(void) {
    if (verbose) puts("STUB: CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext called");
    return NULL;
}

void* CVDisplayLinkSetCurrentDisplay(void) {
    if (verbose) puts("STUB: CVDisplayLinkSetCurrentDisplay called");
    return NULL;
}

void* CVDisplayLinkSetDisplayCallback(void) {
    if (verbose) puts("STUB: CVDisplayLinkSetDisplayCallback called");
    return NULL;
}

void* CVDisplayLinkSetOutputCallback(void) {
    if (verbose) puts("STUB: CVDisplayLinkSetOutputCallback called");
    return NULL;
}

void* CVDisplayLinkSetOutputHandler(void) {
    if (verbose) puts("STUB: CVDisplayLinkSetOutputHandler called");
    return NULL;
}

void* CVDisplayLinkSetPaused(void) {
    if (verbose) puts("STUB: CVDisplayLinkSetPaused called");
    return NULL;
}

void* CVDisplayLinkSetRenderCallback(void) {
    if (verbose) puts("STUB: CVDisplayLinkSetRenderCallback called");
    return NULL;
}

void* CVDisplayLinkStart(void) {
    if (verbose) puts("STUB: CVDisplayLinkStart called");
    return NULL;
}

void* CVDisplayLinkStop(void) {
    if (verbose) puts("STUB: CVDisplayLinkStop called");
    return NULL;
}

void* CVDisplayLinkTranslateTime(void) {
    if (verbose) puts("STUB: CVDisplayLinkTranslateTime called");
    return NULL;
}

void* CVGetCurrentHostTime(void) {
    if (verbose) puts("STUB: CVGetCurrentHostTime called");
    return NULL;
}

void* CVGetHostClockFrequency(void) {
    if (verbose) puts("STUB: CVGetHostClockFrequency called");
    return NULL;
}

void* CVGetHostClockMinimumTimeDelta(void) {
    if (verbose) puts("STUB: CVGetHostClockMinimumTimeDelta called");
    return NULL;
}

void* CVImageBufferCreateColorSpaceFromAttachments(void) {
    if (verbose) puts("STUB: CVImageBufferCreateColorSpaceFromAttachments called");
    return NULL;
}

void* CVImageBufferDeleteImageTexture(void) {
    if (verbose) puts("STUB: CVImageBufferDeleteImageTexture called");
    return NULL;
}

void* CVImageBufferGetBufferBackingSize(void) {
    if (verbose) puts("STUB: CVImageBufferGetBufferBackingSize called");
    return NULL;
}

void* CVImageBufferGetCleanRect(void) {
    if (verbose) puts("STUB: CVImageBufferGetCleanRect called");
    return NULL;
}

void* CVImageBufferGetColorSpace(void) {
    if (verbose) puts("STUB: CVImageBufferGetColorSpace called");
    return NULL;
}

void* CVImageBufferGetDisplaySize(void) {
    if (verbose) puts("STUB: CVImageBufferGetDisplaySize called");
    return NULL;
}

void* CVImageBufferGetEncodedSize(void) {
    if (verbose) puts("STUB: CVImageBufferGetEncodedSize called");
    return NULL;
}

void* CVImageBufferGetPixelFormatType(void) {
    if (verbose) puts("STUB: CVImageBufferGetPixelFormatType called");
    return NULL;
}

void* CVImageBufferHasCleanRect(void) {
    if (verbose) puts("STUB: CVImageBufferHasCleanRect called");
    return NULL;
}

void* CVImageBufferIsFlipped(void) {
    if (verbose) puts("STUB: CVImageBufferIsFlipped called");
    return NULL;
}

void* CVImageBufferProvideImageData(void) {
    if (verbose) puts("STUB: CVImageBufferProvideImageData called");
    return NULL;
}

void* CVImageBufferProvideImageTexture(void) {
    if (verbose) puts("STUB: CVImageBufferProvideImageTexture called");
    return NULL;
}

void* CVImageBufferSetCleanAperture(void) {
    if (verbose) puts("STUB: CVImageBufferSetCleanAperture called");
    return NULL;
}

void* CVImageBufferSetColorSpace(void) {
    if (verbose) puts("STUB: CVImageBufferSetColorSpace called");
    return NULL;
}

void* CVImageBufferSetDisplayDimensions(void) {
    if (verbose) puts("STUB: CVImageBufferSetDisplayDimensions called");
    return NULL;
}

void* CVImageBufferSetFieldCount(void) {
    if (verbose) puts("STUB: CVImageBufferSetFieldCount called");
    return NULL;
}

void* CVImageBufferSetPixelAspectRatio(void) {
    if (verbose) puts("STUB: CVImageBufferSetPixelAspectRatio called");
    return NULL;
}

void* CVMetalTextureCacheCreate(void) {
    if (verbose) puts("STUB: CVMetalTextureCacheCreate called");
    return NULL;
}

void* CVMetalTextureCacheCreateTextureFromImage(void) {
    if (verbose) puts("STUB: CVMetalTextureCacheCreateTextureFromImage called");
    return NULL;
}

void* CVMetalTextureCacheFlush(void) {
    if (verbose) puts("STUB: CVMetalTextureCacheFlush called");
    return NULL;
}

void* CVMetalTextureCacheGetTypeID(void) {
    if (verbose) puts("STUB: CVMetalTextureCacheGetTypeID called");
    return NULL;
}

void* CVMetalTextureGetCleanTexCoords(void) {
    if (verbose) puts("STUB: CVMetalTextureGetCleanTexCoords called");
    return NULL;
}

void* CVMetalTextureGetTexture(void) {
    if (verbose) puts("STUB: CVMetalTextureGetTexture called");
    return NULL;
}

void* CVMetalTextureGetTypeID(void) {
    if (verbose) puts("STUB: CVMetalTextureGetTypeID called");
    return NULL;
}

void* CVMetalTextureIsFlipped(void) {
    if (verbose) puts("STUB: CVMetalTextureIsFlipped called");
    return NULL;
}

void* CVOpenGLBufferAttach(void) {
    if (verbose) puts("STUB: CVOpenGLBufferAttach called");
    return NULL;
}

void* CVOpenGLBufferCreate(void) {
    if (verbose) puts("STUB: CVOpenGLBufferCreate called");
    return NULL;
}

void* CVOpenGLBufferGetAttributes(void) {
    if (verbose) puts("STUB: CVOpenGLBufferGetAttributes called");
    return NULL;
}

void* CVOpenGLBufferGetDisplayMask(void) {
    if (verbose) puts("STUB: CVOpenGLBufferGetDisplayMask called");
    return NULL;
}

void* CVOpenGLBufferGetPBuffer(void) {
    if (verbose) puts("STUB: CVOpenGLBufferGetPBuffer called");
    return NULL;
}

void* CVOpenGLBufferGetSurfaceID(void) {
    if (verbose) puts("STUB: CVOpenGLBufferGetSurfaceID called");
    return NULL;
}

void* CVOpenGLBufferGetTypeID(void) {
    if (verbose) puts("STUB: CVOpenGLBufferGetTypeID called");
    return NULL;
}

void* CVOpenGLBufferPoolCreate(void) {
    if (verbose) puts("STUB: CVOpenGLBufferPoolCreate called");
    return NULL;
}

void* CVOpenGLBufferPoolCreateOpenGLBuffer(void) {
    if (verbose) puts("STUB: CVOpenGLBufferPoolCreateOpenGLBuffer called");
    return NULL;
}

void* CVOpenGLBufferPoolGetAttributes(void) {
    if (verbose) puts("STUB: CVOpenGLBufferPoolGetAttributes called");
    return NULL;
}

void* CVOpenGLBufferPoolGetOpenGLBufferAttributes(void) {
    if (verbose) puts("STUB: CVOpenGLBufferPoolGetOpenGLBufferAttributes called");
    return NULL;
}

void* CVOpenGLBufferPoolGetTypeID(void) {
    if (verbose) puts("STUB: CVOpenGLBufferPoolGetTypeID called");
    return NULL;
}

void* CVOpenGLBufferPoolRelease(void) {
    if (verbose) puts("STUB: CVOpenGLBufferPoolRelease called");
    return NULL;
}

void* CVOpenGLBufferPoolRetain(void) {
    if (verbose) puts("STUB: CVOpenGLBufferPoolRetain called");
    return NULL;
}

void* CVOpenGLBufferRelease(void) {
    if (verbose) puts("STUB: CVOpenGLBufferRelease called");
    return NULL;
}

void* CVOpenGLBufferRetain(void) {
    if (verbose) puts("STUB: CVOpenGLBufferRetain called");
    return NULL;
}

void* CVOpenGLTextureBackingImage(void) {
    if (verbose) puts("STUB: CVOpenGLTextureBackingImage called");
    return NULL;
}

void* CVOpenGLTextureBufferBackingImage(void) {
    if (verbose) puts("STUB: CVOpenGLTextureBufferBackingImage called");
    return NULL;
}

void* CVOpenGLTextureCacheCreate(void) {
    if (verbose) puts("STUB: CVOpenGLTextureCacheCreate called");
    return NULL;
}

void* CVOpenGLTextureCacheCreateCGLS(void) {
    if (verbose) puts("STUB: CVOpenGLTextureCacheCreateCGLS called");
    return NULL;
}

void* CVOpenGLTextureCacheCreateTextureFromImage(void) {
    if (verbose) puts("STUB: CVOpenGLTextureCacheCreateTextureFromImage called");
    return NULL;
}

void* CVOpenGLTextureCacheFlush(void) {
    if (verbose) puts("STUB: CVOpenGLTextureCacheFlush called");
    return NULL;
}

void* CVOpenGLTextureCacheGetTypeID(void) {
    if (verbose) puts("STUB: CVOpenGLTextureCacheGetTypeID called");
    return NULL;
}

void* CVOpenGLTextureCacheRelease(void) {
    if (verbose) puts("STUB: CVOpenGLTextureCacheRelease called");
    return NULL;
}

void* CVOpenGLTextureCacheRetain(void) {
    if (verbose) puts("STUB: CVOpenGLTextureCacheRetain called");
    return NULL;
}

void* CVOpenGLTextureCopyBacking(void) {
    if (verbose) puts("STUB: CVOpenGLTextureCopyBacking called");
    return NULL;
}

void* CVOpenGLTextureGetBounds(void) {
    if (verbose) puts("STUB: CVOpenGLTextureGetBounds called");
    return NULL;
}

void* CVOpenGLTextureGetCGLContextObj(void) {
    if (verbose) puts("STUB: CVOpenGLTextureGetCGLContextObj called");
    return NULL;
}

void* CVOpenGLTextureGetCleanRect(void) {
    if (verbose) puts("STUB: CVOpenGLTextureGetCleanRect called");
    return NULL;
}

void* CVOpenGLTextureGetCleanTexCoords(void) {
    if (verbose) puts("STUB: CVOpenGLTextureGetCleanTexCoords called");
    return NULL;
}

void* CVOpenGLTextureGetName(void) {
    if (verbose) puts("STUB: CVOpenGLTextureGetName called");
    return NULL;
}

void* CVOpenGLTextureGetSize(void) {
    if (verbose) puts("STUB: CVOpenGLTextureGetSize called");
    return NULL;
}

void* CVOpenGLTextureGetTarget(void) {
    if (verbose) puts("STUB: CVOpenGLTextureGetTarget called");
    return NULL;
}

void* CVOpenGLTextureGetTypeID(void) {
    if (verbose) puts("STUB: CVOpenGLTextureGetTypeID called");
    return NULL;
}

void* CVOpenGLTextureIsFlipped(void) {
    if (verbose) puts("STUB: CVOpenGLTextureIsFlipped called");
    return NULL;
}

void* CVOpenGLTextureRelease(void) {
    if (verbose) puts("STUB: CVOpenGLTextureRelease called");
    return NULL;
}

void* CVOpenGLTextureRetain(void) {
    if (verbose) puts("STUB: CVOpenGLTextureRetain called");
    return NULL;
}

void* CVPixelBufferCreate(void) {
    if (verbose) puts("STUB: CVPixelBufferCreate called");
    return NULL;
}

void* CVPixelBufferCreateFromCVImageBufferRef(void) {
    if (verbose) puts("STUB: CVPixelBufferCreateFromCVImageBufferRef called");
    return NULL;
}

void* CVPixelBufferCreateResolvedAttributesDictionary(void) {
    if (verbose) puts("STUB: CVPixelBufferCreateResolvedAttributesDictionary called");
    return NULL;
}

void* CVPixelBufferCreateWithBytes(void) {
    if (verbose) puts("STUB: CVPixelBufferCreateWithBytes called");
    return NULL;
}

void* CVPixelBufferCreateWithIOSurface(void) {
    if (verbose) puts("STUB: CVPixelBufferCreateWithIOSurface called");
    return NULL;
}

void* CVPixelBufferCreateWithPlanarBytes(void) {
    if (verbose) puts("STUB: CVPixelBufferCreateWithPlanarBytes called");
    return NULL;
}

void* CVPixelBufferDrawColorBars(void) {
    if (verbose) puts("STUB: CVPixelBufferDrawColorBars called");
    return NULL;
}

void* CVPixelBufferFillExtendedPixels(void) {
    if (verbose) puts("STUB: CVPixelBufferFillExtendedPixels called");
    return NULL;
}

void* CVPixelBufferGetAttributes(void) {
    if (verbose) puts("STUB: CVPixelBufferGetAttributes called");
    return NULL;
}

void* CVPixelBufferGetBaseAddress(void) {
    if (verbose) puts("STUB: CVPixelBufferGetBaseAddress called");
    return NULL;
}

void* CVPixelBufferGetBaseAddressOfPlane(void) {
    if (verbose) puts("STUB: CVPixelBufferGetBaseAddressOfPlane called");
    return NULL;
}

void* CVPixelBufferGetBufferBacking(void) {
    if (verbose) puts("STUB: CVPixelBufferGetBufferBacking called");
    return NULL;
}

void* CVPixelBufferGetBytesPerRow(void) {
    if (verbose) puts("STUB: CVPixelBufferGetBytesPerRow called");
    return NULL;
}

void* CVPixelBufferGetBytesPerRowOfPlane(void) {
    if (verbose) puts("STUB: CVPixelBufferGetBytesPerRowOfPlane called");
    return NULL;
}

void* CVPixelBufferGetDataSize(void) {
    if (verbose) puts("STUB: CVPixelBufferGetDataSize called");
    return NULL;
}

void* CVPixelBufferGetExtendedPixels(void) {
    if (verbose) puts("STUB: CVPixelBufferGetExtendedPixels called");
    return NULL;
}

void* CVPixelBufferGetFixedPointFractionalBits(void) {
    if (verbose) puts("STUB: CVPixelBufferGetFixedPointFractionalBits called");
    return NULL;
}

void* CVPixelBufferGetFixedPointIntegerBits(void) {
    if (verbose) puts("STUB: CVPixelBufferGetFixedPointIntegerBits called");
    return NULL;
}

void* CVPixelBufferGetFixedPointPaddingBits(void) {
    if (verbose) puts("STUB: CVPixelBufferGetFixedPointPaddingBits called");
    return NULL;
}

void* CVPixelBufferGetFixedPointSignBits(void) {
    if (verbose) puts("STUB: CVPixelBufferGetFixedPointSignBits called");
    return NULL;
}

void* CVPixelBufferGetHeight(void) {
    if (verbose) puts("STUB: CVPixelBufferGetHeight called");
    return NULL;
}

void* CVPixelBufferGetHeightOfPlane(void) {
    if (verbose) puts("STUB: CVPixelBufferGetHeightOfPlane called");
    return NULL;
}

void* CVPixelBufferGetIOSurface(void) {
    if (verbose) puts("STUB: CVPixelBufferGetIOSurface called");
    return NULL;
}

void* CVPixelBufferGetPixelFormatType(void) {
    if (verbose) puts("STUB: CVPixelBufferGetPixelFormatType called");
    return NULL;
}

void* CVPixelBufferGetPlaneCount(void) {
    if (verbose) puts("STUB: CVPixelBufferGetPlaneCount called");
    return NULL;
}

void* CVPixelBufferGetTypeID(void) {
    if (verbose) puts("STUB: CVPixelBufferGetTypeID called");
    return NULL;
}

void* CVPixelBufferGetWidth(void) {
    if (verbose) puts("STUB: CVPixelBufferGetWidth called");
    return NULL;
}

void* CVPixelBufferGetWidthOfPlane(void) {
    if (verbose) puts("STUB: CVPixelBufferGetWidthOfPlane called");
    return NULL;
}

void* CVPixelBufferIsCompatibleWithAttributes(void) {
    if (verbose) puts("STUB: CVPixelBufferIsCompatibleWithAttributes called");
    return NULL;
}

void* CVPixelBufferIsPlanar(void) {
    if (verbose) puts("STUB: CVPixelBufferIsPlanar called");
    return NULL;
}

void* CVPixelBufferLockBaseAddress(void) {
    if (verbose) puts("STUB: CVPixelBufferLockBaseAddress called");
    return NULL;
}

void* CVPixelBufferPoolCreate(void) {
    if (verbose) puts("STUB: CVPixelBufferPoolCreate called");
    return NULL;
}

void* CVPixelBufferPoolCreatePixelBuffer(void) {
    if (verbose) puts("STUB: CVPixelBufferPoolCreatePixelBuffer called");
    return NULL;
}

void* CVPixelBufferPoolCreatePixelBufferWithAuxAttributes(void) {
    if (verbose) puts("STUB: CVPixelBufferPoolCreatePixelBufferWithAuxAttributes called");
    return NULL;
}

void* CVPixelBufferPoolFlush(void) {
    if (verbose) puts("STUB: CVPixelBufferPoolFlush called");
    return NULL;
}

void* CVPixelBufferPoolGetAttributes(void) {
    if (verbose) puts("STUB: CVPixelBufferPoolGetAttributes called");
    return NULL;
}

void* CVPixelBufferPoolGetMaxBufferAge(void) {
    if (verbose) puts("STUB: CVPixelBufferPoolGetMaxBufferAge called");
    return NULL;
}

void* CVPixelBufferPoolGetMinBufferCount(void) {
    if (verbose) puts("STUB: CVPixelBufferPoolGetMinBufferCount called");
    return NULL;
}

void* CVPixelBufferPoolGetPixelBufferAttributes(void) {
    if (verbose) puts("STUB: CVPixelBufferPoolGetPixelBufferAttributes called");
    return NULL;
}

void* CVPixelBufferPoolGetTypeID(void) {
    if (verbose) puts("STUB: CVPixelBufferPoolGetTypeID called");
    return NULL;
}

void* CVPixelBufferPoolPreAllocate(void) {
    if (verbose) puts("STUB: CVPixelBufferPoolPreAllocate called");
    return NULL;
}

void* CVPixelBufferPoolPrefetchPages(void) {
    if (verbose) puts("STUB: CVPixelBufferPoolPrefetchPages called");
    return NULL;
}

void* CVPixelBufferPoolRelease(void) {
    if (verbose) puts("STUB: CVPixelBufferPoolRelease called");
    return NULL;
}

void* CVPixelBufferPoolRetain(void) {
    if (verbose) puts("STUB: CVPixelBufferPoolRetain called");
    return NULL;
}

void* CVPixelBufferPoolScanIOSurfaces(void) {
    if (verbose) puts("STUB: CVPixelBufferPoolScanIOSurfaces called");
    return NULL;
}

void* CVPixelBufferPoolSetMaxBufferAge(void) {
    if (verbose) puts("STUB: CVPixelBufferPoolSetMaxBufferAge called");
    return NULL;
}

void* CVPixelBufferPoolSetMinBufferCount(void) {
    if (verbose) puts("STUB: CVPixelBufferPoolSetMinBufferCount called");
    return NULL;
}

void* CVPixelBufferRelease(void) {
    if (verbose) puts("STUB: CVPixelBufferRelease called");
    return NULL;
}

void* CVPixelBufferRetain(void) {
    if (verbose) puts("STUB: CVPixelBufferRetain called");
    return NULL;
}

void* CVPixelBufferUnlockBaseAddress(void) {
    if (verbose) puts("STUB: CVPixelBufferUnlockBaseAddress called");
    return NULL;
}

void* CVPixelBufferWasAllocatedFromPool(void) {
    if (verbose) puts("STUB: CVPixelBufferWasAllocatedFromPool called");
    return NULL;
}

void* CVPixelFormatDescriptionArrayCreateWithAllPixelFormatTypes(void) {
    if (verbose) puts("STUB: CVPixelFormatDescriptionArrayCreateWithAllPixelFormatTypes called");
    return NULL;
}

void* CVPixelFormatDescriptionCreateWithPixelFormatType(void) {
    if (verbose) puts("STUB: CVPixelFormatDescriptionCreateWithPixelFormatType called");
    return NULL;
}

void* CVPixelFormatDescriptionGetDescriptionWithPixelFormatType(void) {
    if (verbose) puts("STUB: CVPixelFormatDescriptionGetDescriptionWithPixelFormatType called");
    return NULL;
}

void* CVPixelFormatDescriptionGetPixelFormatTypes(void) {
    if (verbose) puts("STUB: CVPixelFormatDescriptionGetPixelFormatTypes called");
    return NULL;
}

void* CVPixelFormatDescriptionRegisterDescriptionWithPixelFormatType(void) {
    if (verbose) puts("STUB: CVPixelFormatDescriptionRegisterDescriptionWithPixelFormatType called");
    return NULL;
}

void* CVPixelFormatTypeIsSubsampled(void) {
    if (verbose) puts("STUB: CVPixelFormatTypeIsSubsampled called");
    return NULL;
}

void* CVTransferFunctionGetIntegerCodePointForString(void) {
    if (verbose) puts("STUB: CVTransferFunctionGetIntegerCodePointForString called");
    return NULL;
}

void* CVTransferFunctionGetStringForIntegerCodePoint(void) {
    if (verbose) puts("STUB: CVTransferFunctionGetStringForIntegerCodePoint called");
    return NULL;
}

void* CVYCbCrMatrixGetIntegerCodePointForString(void) {
    if (verbose) puts("STUB: CVYCbCrMatrixGetIntegerCodePointForString called");
    return NULL;
}

void* CVYCbCrMatrixGetStringForIntegerCodePoint(void) {
    if (verbose) puts("STUB: CVYCbCrMatrixGetStringForIntegerCodePoint called");
    return NULL;
}
