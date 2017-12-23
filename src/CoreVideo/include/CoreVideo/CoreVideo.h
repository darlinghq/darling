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


#ifndef _CoreVideo_H_
#define _CoreVideo_H_

#import <Foundation/Foundation.h>


void* CVBufferDumpToQTES(void);
void* CVBufferGetAttachment(void);
void* CVBufferGetAttachments(void);
void* CVBufferPropagateAttachments(void);
void* CVBufferRelease(void);
void* CVBufferRemoveAllAttachments(void);
void* CVBufferRemoveAttachment(void);
void* CVBufferRetain(void);
void* CVBufferSetAttachment(void);
void* CVBufferSetAttachments(void);
void* CVColorPrimariesGetIntegerCodePointForString(void);
void* CVColorPrimariesGetStringForIntegerCodePoint(void);
void* CVDisplayLinkCreateWithActiveCGDisplays(void);
void* CVDisplayLinkCreateWithCGDisplay(void);
void* CVDisplayLinkCreateWithCGDisplays(void);
void* CVDisplayLinkCreateWithDisplay(void);
void* CVDisplayLinkCreateWithDisplays(void);
void* CVDisplayLinkCreateWithOpenGLDisplayMask(void);
void* CVDisplayLinkGetActualOutputVideoRefreshPeriod(void);
void* CVDisplayLinkGetCurrentCGDisplay(void);
void* CVDisplayLinkGetCurrentDisplay(void);
void* CVDisplayLinkGetCurrentTime(void);
void* CVDisplayLinkGetNominalOutputVideoRefreshPeriod(void);
void* CVDisplayLinkGetOutputVideoLatency(void);
void* CVDisplayLinkGetTypeID(void);
void* CVDisplayLinkIsRunning(void);
void* CVDisplayLinkRelease(void);
void* CVDisplayLinkRetain(void);
void* CVDisplayLinkSetCurrentCGDisplay(void);
void* CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(void);
void* CVDisplayLinkSetCurrentDisplay(void);
void* CVDisplayLinkSetDisplayCallback(void);
void* CVDisplayLinkSetOutputCallback(void);
void* CVDisplayLinkSetOutputHandler(void);
void* CVDisplayLinkSetPaused(void);
void* CVDisplayLinkSetRenderCallback(void);
void* CVDisplayLinkStart(void);
void* CVDisplayLinkStop(void);
void* CVDisplayLinkTranslateTime(void);
void* CVGetCurrentHostTime(void);
void* CVGetHostClockFrequency(void);
void* CVGetHostClockMinimumTimeDelta(void);
void* CVImageBufferCreateColorSpaceFromAttachments(void);
void* CVImageBufferDeleteImageTexture(void);
void* CVImageBufferGetBufferBackingSize(void);
void* CVImageBufferGetCleanRect(void);
void* CVImageBufferGetColorSpace(void);
void* CVImageBufferGetDisplaySize(void);
void* CVImageBufferGetEncodedSize(void);
void* CVImageBufferGetPixelFormatType(void);
void* CVImageBufferHasCleanRect(void);
void* CVImageBufferIsFlipped(void);
void* CVImageBufferProvideImageData(void);
void* CVImageBufferProvideImageTexture(void);
void* CVImageBufferSetCleanAperture(void);
void* CVImageBufferSetColorSpace(void);
void* CVImageBufferSetDisplayDimensions(void);
void* CVImageBufferSetFieldCount(void);
void* CVImageBufferSetPixelAspectRatio(void);
void* CVMetalTextureCacheCreate(void);
void* CVMetalTextureCacheCreateTextureFromImage(void);
void* CVMetalTextureCacheFlush(void);
void* CVMetalTextureCacheGetTypeID(void);
void* CVMetalTextureGetCleanTexCoords(void);
void* CVMetalTextureGetTexture(void);
void* CVMetalTextureGetTypeID(void);
void* CVMetalTextureIsFlipped(void);
void* CVOpenGLBufferAttach(void);
void* CVOpenGLBufferCreate(void);
void* CVOpenGLBufferGetAttributes(void);
void* CVOpenGLBufferGetDisplayMask(void);
void* CVOpenGLBufferGetPBuffer(void);
void* CVOpenGLBufferGetSurfaceID(void);
void* CVOpenGLBufferGetTypeID(void);
void* CVOpenGLBufferPoolCreate(void);
void* CVOpenGLBufferPoolCreateOpenGLBuffer(void);
void* CVOpenGLBufferPoolGetAttributes(void);
void* CVOpenGLBufferPoolGetOpenGLBufferAttributes(void);
void* CVOpenGLBufferPoolGetTypeID(void);
void* CVOpenGLBufferPoolRelease(void);
void* CVOpenGLBufferPoolRetain(void);
void* CVOpenGLBufferRelease(void);
void* CVOpenGLBufferRetain(void);
void* CVOpenGLTextureBackingImage(void);
void* CVOpenGLTextureBufferBackingImage(void);
void* CVOpenGLTextureCacheCreate(void);
void* CVOpenGLTextureCacheCreateCGLS(void);
void* CVOpenGLTextureCacheCreateTextureFromImage(void);
void* CVOpenGLTextureCacheFlush(void);
void* CVOpenGLTextureCacheGetTypeID(void);
void* CVOpenGLTextureCacheRelease(void);
void* CVOpenGLTextureCacheRetain(void);
void* CVOpenGLTextureCopyBacking(void);
void* CVOpenGLTextureGetBounds(void);
void* CVOpenGLTextureGetCGLContextObj(void);
void* CVOpenGLTextureGetCleanRect(void);
void* CVOpenGLTextureGetCleanTexCoords(void);
void* CVOpenGLTextureGetName(void);
void* CVOpenGLTextureGetSize(void);
void* CVOpenGLTextureGetTarget(void);
void* CVOpenGLTextureGetTypeID(void);
void* CVOpenGLTextureIsFlipped(void);
void* CVOpenGLTextureRelease(void);
void* CVOpenGLTextureRetain(void);
void* CVPixelBufferCreate(void);
void* CVPixelBufferCreateFromCVImageBufferRef(void);
void* CVPixelBufferCreateResolvedAttributesDictionary(void);
void* CVPixelBufferCreateWithBytes(void);
void* CVPixelBufferCreateWithIOSurface(void);
void* CVPixelBufferCreateWithPlanarBytes(void);
void* CVPixelBufferDrawColorBars(void);
void* CVPixelBufferFillExtendedPixels(void);
void* CVPixelBufferGetAttributes(void);
void* CVPixelBufferGetBaseAddress(void);
void* CVPixelBufferGetBaseAddressOfPlane(void);
void* CVPixelBufferGetBufferBacking(void);
void* CVPixelBufferGetBytesPerRow(void);
void* CVPixelBufferGetBytesPerRowOfPlane(void);
void* CVPixelBufferGetDataSize(void);
void* CVPixelBufferGetExtendedPixels(void);
void* CVPixelBufferGetFixedPointFractionalBits(void);
void* CVPixelBufferGetFixedPointIntegerBits(void);
void* CVPixelBufferGetFixedPointPaddingBits(void);
void* CVPixelBufferGetFixedPointSignBits(void);
void* CVPixelBufferGetHeight(void);
void* CVPixelBufferGetHeightOfPlane(void);
void* CVPixelBufferGetIOSurface(void);
void* CVPixelBufferGetPixelFormatType(void);
void* CVPixelBufferGetPlaneCount(void);
void* CVPixelBufferGetTypeID(void);
void* CVPixelBufferGetWidth(void);
void* CVPixelBufferGetWidthOfPlane(void);
void* CVPixelBufferIsCompatibleWithAttributes(void);
void* CVPixelBufferIsPlanar(void);
void* CVPixelBufferLockBaseAddress(void);
void* CVPixelBufferPoolCreate(void);
void* CVPixelBufferPoolCreatePixelBuffer(void);
void* CVPixelBufferPoolCreatePixelBufferWithAuxAttributes(void);
void* CVPixelBufferPoolFlush(void);
void* CVPixelBufferPoolGetAttributes(void);
void* CVPixelBufferPoolGetMaxBufferAge(void);
void* CVPixelBufferPoolGetMinBufferCount(void);
void* CVPixelBufferPoolGetPixelBufferAttributes(void);
void* CVPixelBufferPoolGetTypeID(void);
void* CVPixelBufferPoolPreAllocate(void);
void* CVPixelBufferPoolPrefetchPages(void);
void* CVPixelBufferPoolRelease(void);
void* CVPixelBufferPoolRetain(void);
void* CVPixelBufferPoolScanIOSurfaces(void);
void* CVPixelBufferPoolSetMaxBufferAge(void);
void* CVPixelBufferPoolSetMinBufferCount(void);
void* CVPixelBufferRelease(void);
void* CVPixelBufferRetain(void);
void* CVPixelBufferUnlockBaseAddress(void);
void* CVPixelBufferWasAllocatedFromPool(void);
void* CVPixelFormatDescriptionArrayCreateWithAllPixelFormatTypes(void);
void* CVPixelFormatDescriptionCreateWithPixelFormatType(void);
void* CVPixelFormatDescriptionGetDescriptionWithPixelFormatType(void);
void* CVPixelFormatDescriptionGetPixelFormatTypes(void);
void* CVPixelFormatDescriptionRegisterDescriptionWithPixelFormatType(void);
void* CVPixelFormatTypeIsSubsampled(void);
void* CVTransferFunctionGetIntegerCodePointForString(void);
void* CVTransferFunctionGetStringForIntegerCodePoint(void);
void* CVYCbCrMatrixGetIntegerCodePointForString(void);
void* CVYCbCrMatrixGetStringForIntegerCodePoint(void);

#endif
