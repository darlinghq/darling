/*
 * This file is part of Darling.
 *
 * Copyright (C) 2023 Darling Developers
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

#import <objc/NSObject.h>

@interface NSString : NSObject
@end

#include <math.h>

#ifdef __LP64__
	typedef double CGFloat;
#else
	typedef float CGFloat;
#endif

struct CGPoint {
	CGFloat x;
	CGFloat y;
};
typedef struct CGPoint CGPoint;

struct CGSize {
	CGFloat width;
	CGFloat height;
};
typedef struct CGSize CGSize;

struct CGVector {
	CGFloat dx;
	CGFloat dy;
};
typedef struct CGVector CGVector;

struct CGRect {
	CGPoint origin;
	CGSize size;
};
typedef struct CGRect CGRect;

#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
	verbose = getenv("STUB_VERBOSE") != NULL;
}

void __simple_kprintf(const char* format, ...) __attribute__((format(printf, 1, 2)));

#define LOG_FUNC __simple_kprintf

NSString *const CGAffineTransformIdentity = @"CGAffineTransformIdentity";
const CGPoint CGPointZero = {0, 0};
const CGRect CGRectInfinite = {{0, 0}, {INFINITY, INFINITY}};
const CGRect CGRectNull = {{INFINITY, INFINITY}, {0, 0}};
const CGRect CGRectZero = {{0, 0}, {0, 0}};
const CGSize CGSizeZero = {0, 0};
NSString *const kCGColorBlack = @"kCGColorBlack";
NSString *const kCGColorClear = @"kCGColorClear";
NSString *const kCGColorSpaceDisplayP3 = @"kCGColorSpaceDisplayP3";
NSString *const kCGColorSpaceExtendedGray = @"kCGColorSpaceExtendedGray";
NSString *const kCGColorSpaceExtendedLinearSRGB = @"kCGColorSpaceExtendedLinearSRGB";
NSString *const kCGColorSpaceExtendedSRGB = @"kCGColorSpaceExtendedSRGB";
NSString *const kCGColorSpaceGenericCMYK = @"kCGColorSpaceGenericCMYK";
NSString *const kCGColorSpaceGenericGray = @"kCGColorSpaceGenericGray";
NSString *const kCGColorSpaceGenericGrayGamma2_2 = @"kCGColorSpaceGenericGrayGamma2_2";
NSString *const kCGColorSpaceGenericRGB = @"kCGColorSpaceGenericRGB";
NSString *const kCGColorSpaceLinearSRGB = @"kCGColorSpaceLinearSRGB";
NSString *const kCGColorSpaceSRGB = @"kCGColorSpaceSRGB";
NSString *const kCGColorWhite = @"kCGColorWhite";
unsigned int kCGDisplayPixelHeight = 1;
unsigned int kCGDisplayPixelWidth = 1;
NSString *const kCGDisplayProductNameKey = @"kCGDisplayProductNameKey";
NSString *const kCGImageBlockSingletonRequest = @"kCGImageBlockSingletonRequest";
NSString *const kCGPDFContextCreator = @"kCGPDFContextCreator";
NSString *const kCGPDFContextMediaBox = @"kCGPDFContextMediaBox";
NSString *const kCGPDFContextTitle = @"kCGPDFContextTitle";
NSString *const kCGSSessionIDKey = @"kCGSSessionIDKey";
NSString *const kCGSSessionUserIDKey = @"kCGSSessionUserIDKey";
NSString *const kCGSSessionUserNameKey = @"kCGSSessionUserNameKey";
NSString *const kCGWindowAlpha = @"kCGWindowAlpha";
NSString *const kCGWindowBounds = @"kCGWindowBounds";
NSString *const kCGWindowLayer = @"kCGWindowLayer";
NSString *const kCGWindowOwnerPID = @"kCGWindowOwnerPID";

void* CGAffineTransformConcat(void) {
	if (verbose) LOG_FUNC("STUB: CGAffineTransformConcat called\n");
	return NULL;
};

void* CGAffineTransformEqualToTransform(void) {
	if (verbose) LOG_FUNC("STUB: CGAffineTransformEqualToTransform called\n");
	return NULL;
};

void* CGAffineTransformInvert(void) {
	if (verbose) LOG_FUNC("STUB: CGAffineTransformInvert called\n");
	return NULL;
};

void* CGAffineTransformMakeRotation(void) {
	if (verbose) LOG_FUNC("STUB: CGAffineTransformMakeRotation called\n");
	return NULL;
};

void* CGAffineTransformMakeScale(void) {
	if (verbose) LOG_FUNC("STUB: CGAffineTransformMakeScale called\n");
	return NULL;
};

void* CGAffineTransformMakeTranslation(void) {
	if (verbose) LOG_FUNC("STUB: CGAffineTransformMakeTranslation called\n");
	return NULL;
};

void* CGAffineTransformRotate(void) {
	if (verbose) LOG_FUNC("STUB: CGAffineTransformRotate called\n");
	return NULL;
};

void* CGAffineTransformScale(void) {
	if (verbose) LOG_FUNC("STUB: CGAffineTransformScale called\n");
	return NULL;
};

void* CGAffineTransformTranslate(void) {
	if (verbose) LOG_FUNC("STUB: CGAffineTransformTranslate called\n");
	return NULL;
};

void* CGAssociateMouseAndMouseCursorPosition(void) {
	if (verbose) LOG_FUNC("STUB: CGAssociateMouseAndMouseCursorPosition called\n");
	return NULL;
};

void* CGBitmapContextCreate(void) {
	if (verbose) LOG_FUNC("STUB: CGBitmapContextCreate called\n");
	return NULL;
};

void* CGBitmapContextCreateImage(void) {
	if (verbose) LOG_FUNC("STUB: CGBitmapContextCreateImage called\n");
	return NULL;
};

void* CGBitmapContextGetBitsPerPixel(void) {
	if (verbose) LOG_FUNC("STUB: CGBitmapContextGetBitsPerPixel called\n");
	return NULL;
};

void* CGBitmapContextGetBytesPerRow(void) {
	if (verbose) LOG_FUNC("STUB: CGBitmapContextGetBytesPerRow called\n");
	return NULL;
};

void* CGBitmapContextGetData(void) {
	if (verbose) LOG_FUNC("STUB: CGBitmapContextGetData called\n");
	return NULL;
};

void* CGBitmapContextGetHeight(void) {
	if (verbose) LOG_FUNC("STUB: CGBitmapContextGetHeight called\n");
	return NULL;
};

void* CGBitmapContextGetWidth(void) {
	if (verbose) LOG_FUNC("STUB: CGBitmapContextGetWidth called\n");
	return NULL;
};

void* CGColorCreate(void) {
	if (verbose) LOG_FUNC("STUB: CGColorCreate called\n");
	return NULL;
};

void* CGColorCreateCopyByMatchingToColorSpace(void) {
	if (verbose) LOG_FUNC("STUB: CGColorCreateCopyByMatchingToColorSpace called\n");
	return NULL;
};

void* CGColorCreateCopyWithAlpha(void) {
	if (verbose) LOG_FUNC("STUB: CGColorCreateCopyWithAlpha called\n");
	return NULL;
};

void* CGColorCreateGenericGray(void) {
	if (verbose) LOG_FUNC("STUB: CGColorCreateGenericGray called\n");
	return NULL;
};

void* CGColorCreateGenericRGB(void) {
	if (verbose) LOG_FUNC("STUB: CGColorCreateGenericRGB called\n");
	return NULL;
};

void* CGColorGetAlpha(void) {
	if (verbose) LOG_FUNC("STUB: CGColorGetAlpha called\n");
	return NULL;
};

void* CGColorGetColorSpace(void) {
	if (verbose) LOG_FUNC("STUB: CGColorGetColorSpace called\n");
	return NULL;
};

void* CGColorGetComponents(void) {
	if (verbose) LOG_FUNC("STUB: CGColorGetComponents called\n");
	return NULL;
};

void* CGColorGetConstantColor(void) {
	if (verbose) LOG_FUNC("STUB: CGColorGetConstantColor called\n");
	return NULL;
};

void* CGColorGetNumberOfComponents(void) {
	if (verbose) LOG_FUNC("STUB: CGColorGetNumberOfComponents called\n");
	return NULL;
};

void* CGColorGetTypeID(void) {
	if (verbose) LOG_FUNC("STUB: CGColorGetTypeID called\n");
	return NULL;
};

void* CGColorRelease(void) {
	if (verbose) LOG_FUNC("STUB: CGColorRelease called\n");
	return NULL;
};

void* CGColorRetain(void) {
	if (verbose) LOG_FUNC("STUB: CGColorRetain called\n");
	return NULL;
};

void* CGColorSpaceCopyName(void) {
	if (verbose) LOG_FUNC("STUB: CGColorSpaceCopyName called\n");
	return NULL;
};

void* CGColorSpaceCreateDeviceGray(void) {
	if (verbose) LOG_FUNC("STUB: CGColorSpaceCreateDeviceGray called\n");
	return NULL;
};

void* CGColorSpaceCreateDeviceRGB(void) {
	if (verbose) LOG_FUNC("STUB: CGColorSpaceCreateDeviceRGB called\n");
	return NULL;
};

void* CGColorSpaceCreatePattern(void) {
	if (verbose) LOG_FUNC("STUB: CGColorSpaceCreatePattern called\n");
	return NULL;
};

void* CGColorSpaceCreateWithICCData(void) {
	if (verbose) LOG_FUNC("STUB: CGColorSpaceCreateWithICCData called\n");
	return NULL;
};

void* CGColorSpaceCreateWithName(void) {
	if (verbose) LOG_FUNC("STUB: CGColorSpaceCreateWithName called\n");
	return NULL;
};

void* CGColorSpaceGetBaseColorSpace(void) {
	if (verbose) LOG_FUNC("STUB: CGColorSpaceGetBaseColorSpace called\n");
	return NULL;
};

void* CGColorSpaceGetColorTable(void) {
	if (verbose) LOG_FUNC("STUB: CGColorSpaceGetColorTable called\n");
	return NULL;
};

void* CGColorSpaceGetColorTableCount(void) {
	if (verbose) LOG_FUNC("STUB: CGColorSpaceGetColorTableCount called\n");
	return NULL;
};

void* CGColorSpaceGetModel(void) {
	if (verbose) LOG_FUNC("STUB: CGColorSpaceGetModel called\n");
	return NULL;
};

void* CGColorSpaceGetNumberOfComponents(void) {
	if (verbose) LOG_FUNC("STUB: CGColorSpaceGetNumberOfComponents called\n");
	return NULL;
};

void* CGColorSpaceRelease(void) {
	if (verbose) LOG_FUNC("STUB: CGColorSpaceRelease called\n");
	return NULL;
};

void* CGColorSpaceRetain(void) {
	if (verbose) LOG_FUNC("STUB: CGColorSpaceRetain called\n");
	return NULL;
};

void* CGContextAddArc(void) {
	if (verbose) LOG_FUNC("STUB: CGContextAddArc called\n");
	return NULL;
};

void* CGContextAddArcToPoint(void) {
	if (verbose) LOG_FUNC("STUB: CGContextAddArcToPoint called\n");
	return NULL;
};

void* CGContextAddCurveToPoint(void) {
	if (verbose) LOG_FUNC("STUB: CGContextAddCurveToPoint called\n");
	return NULL;
};

void* CGContextAddEllipseInRect(void) {
	if (verbose) LOG_FUNC("STUB: CGContextAddEllipseInRect called\n");
	return NULL;
};

void* CGContextAddLineToPoint(void) {
	if (verbose) LOG_FUNC("STUB: CGContextAddLineToPoint called\n");
	return NULL;
};

void* CGContextAddLines(void) {
	if (verbose) LOG_FUNC("STUB: CGContextAddLines called\n");
	return NULL;
};

void* CGContextAddPath(void) {
	if (verbose) LOG_FUNC("STUB: CGContextAddPath called\n");
	return NULL;
};

void* CGContextAddQuadCurveToPoint(void) {
	if (verbose) LOG_FUNC("STUB: CGContextAddQuadCurveToPoint called\n");
	return NULL;
};

void* CGContextAddRect(void) {
	if (verbose) LOG_FUNC("STUB: CGContextAddRect called\n");
	return NULL;
};

void* CGContextAddRects(void) {
	if (verbose) LOG_FUNC("STUB: CGContextAddRects called\n");
	return NULL;
};

void* CGContextBeginPath(void) {
	if (verbose) LOG_FUNC("STUB: CGContextBeginPath called\n");
	return NULL;
};

void* CGContextBeginTransparencyLayer(void) {
	if (verbose) LOG_FUNC("STUB: CGContextBeginTransparencyLayer called\n");
	return NULL;
};

void* CGContextBeginTransparencyLayerWithRect(void) {
	if (verbose) LOG_FUNC("STUB: CGContextBeginTransparencyLayerWithRect called\n");
	return NULL;
};

void* CGContextClearRect(void) {
	if (verbose) LOG_FUNC("STUB: CGContextClearRect called\n");
	return NULL;
};

void* CGContextClip(void) {
	if (verbose) LOG_FUNC("STUB: CGContextClip called\n");
	return NULL;
};

void* CGContextClipToMask(void) {
	if (verbose) LOG_FUNC("STUB: CGContextClipToMask called\n");
	return NULL;
};

void* CGContextClipToRect(void) {
	if (verbose) LOG_FUNC("STUB: CGContextClipToRect called\n");
	return NULL;
};

void* CGContextClipToRects(void) {
	if (verbose) LOG_FUNC("STUB: CGContextClipToRects called\n");
	return NULL;
};

void* CGContextClosePath(void) {
	if (verbose) LOG_FUNC("STUB: CGContextClosePath called\n");
	return NULL;
};

void* CGContextConcatCTM(void) {
	if (verbose) LOG_FUNC("STUB: CGContextConcatCTM called\n");
	return NULL;
};

void* CGContextConvertRectToDeviceSpace(void) {
	if (verbose) LOG_FUNC("STUB: CGContextConvertRectToDeviceSpace called\n");
	return NULL;
};

void* CGContextCopyPath(void) {
	if (verbose) LOG_FUNC("STUB: CGContextCopyPath called\n");
	return NULL;
};

void* CGContextDrawImage(void) {
	if (verbose) LOG_FUNC("STUB: CGContextDrawImage called\n");
	return NULL;
};

void* CGContextDrawLayerAtPoint(void) {
	if (verbose) LOG_FUNC("STUB: CGContextDrawLayerAtPoint called\n");
	return NULL;
};

void* CGContextDrawLayerInRect(void) {
	if (verbose) LOG_FUNC("STUB: CGContextDrawLayerInRect called\n");
	return NULL;
};

void* CGContextDrawPDFPage(void) {
	if (verbose) LOG_FUNC("STUB: CGContextDrawPDFPage called\n");
	return NULL;
};

void* CGContextDrawPath(void) {
	if (verbose) LOG_FUNC("STUB: CGContextDrawPath called\n");
	return NULL;
};

void* CGContextDrawShading(void) {
	if (verbose) LOG_FUNC("STUB: CGContextDrawShading called\n");
	return NULL;
};

void* CGContextDrawTiledImage(void) {
	if (verbose) LOG_FUNC("STUB: CGContextDrawTiledImage called\n");
	return NULL;
};

void* CGContextEOClip(void) {
	if (verbose) LOG_FUNC("STUB: CGContextEOClip called\n");
	return NULL;
};

void* CGContextEOFillPath(void) {
	if (verbose) LOG_FUNC("STUB: CGContextEOFillPath called\n");
	return NULL;
};

void* CGContextEndTransparencyLayer(void) {
	if (verbose) LOG_FUNC("STUB: CGContextEndTransparencyLayer called\n");
	return NULL;
};

void* CGContextFillEllipseInRect(void) {
	if (verbose) LOG_FUNC("STUB: CGContextFillEllipseInRect called\n");
	return NULL;
};

void* CGContextFillPath(void) {
	if (verbose) LOG_FUNC("STUB: CGContextFillPath called\n");
	return NULL;
};

void* CGContextFillRect(void) {
	if (verbose) LOG_FUNC("STUB: CGContextFillRect called\n");
	return NULL;
};

void* CGContextFillRects(void) {
	if (verbose) LOG_FUNC("STUB: CGContextFillRects called\n");
	return NULL;
};

void* CGContextGetCTM(void) {
	if (verbose) LOG_FUNC("STUB: CGContextGetCTM called\n");
	return NULL;
};

void* CGContextGetClipBoundingBox(void) {
	if (verbose) LOG_FUNC("STUB: CGContextGetClipBoundingBox called\n");
	return NULL;
};

void* CGContextGetFontRenderingStyle(void) {
	if (verbose) LOG_FUNC("STUB: CGContextGetFontRenderingStyle called\n");
	return NULL;
};

void* CGContextGetFontSmoothingBackgroundColor(void) {
	if (verbose) LOG_FUNC("STUB: CGContextGetFontSmoothingBackgroundColor called\n");
	return NULL;
};

void* CGContextGetFontSmoothingStyle(void) {
	if (verbose) LOG_FUNC("STUB: CGContextGetFontSmoothingStyle called\n");
	return NULL;
};

void* CGContextGetTextMatrix(void) {
	if (verbose) LOG_FUNC("STUB: CGContextGetTextMatrix called\n");
	return NULL;
};

void* CGContextGetTextPosition(void) {
	if (verbose) LOG_FUNC("STUB: CGContextGetTextPosition called\n");
	return NULL;
};

void* CGContextMoveToPoint(void) {
	if (verbose) LOG_FUNC("STUB: CGContextMoveToPoint called\n");
	return NULL;
};

void* CGContextPathContainsPoint(void) {
	if (verbose) LOG_FUNC("STUB: CGContextPathContainsPoint called\n");
	return NULL;
};

void* CGContextRelease(void) {
	if (verbose) LOG_FUNC("STUB: CGContextRelease called\n");
	return NULL;
};

void* CGContextReplacePathWithStrokedPath(void) {
	if (verbose) LOG_FUNC("STUB: CGContextReplacePathWithStrokedPath called\n");
	return NULL;
};

void* CGContextRestoreGState(void) {
	if (verbose) LOG_FUNC("STUB: CGContextRestoreGState called\n");
	return NULL;
};

void* CGContextRotateCTM(void) {
	if (verbose) LOG_FUNC("STUB: CGContextRotateCTM called\n");
	return NULL;
};

void* CGContextSaveGState(void) {
	if (verbose) LOG_FUNC("STUB: CGContextSaveGState called\n");
	return NULL;
};

void* CGContextScaleCTM(void) {
	if (verbose) LOG_FUNC("STUB: CGContextScaleCTM called\n");
	return NULL;
};

void* CGContextSetAllowsAntialiasing(void) {
	if (verbose) LOG_FUNC("STUB: CGContextSetAllowsAntialiasing called\n");
	return NULL;
};

void* CGContextSetAllowsFontSmoothing(void) {
	if (verbose) LOG_FUNC("STUB: CGContextSetAllowsFontSmoothing called\n");
	return NULL;
};

void* CGContextSetAllowsFontSubpixelPositioning(void) {
	if (verbose) LOG_FUNC("STUB: CGContextSetAllowsFontSubpixelPositioning called\n");
	return NULL;
};

void* CGContextSetAllowsFontSubpixelQuantization(void) {
	if (verbose) LOG_FUNC("STUB: CGContextSetAllowsFontSubpixelQuantization called\n");
	return NULL;
};

void* CGContextSetAlpha(void) {
	if (verbose) LOG_FUNC("STUB: CGContextSetAlpha called\n");
	return NULL;
};

void* CGContextSetBaseCTM(void) {
	if (verbose) LOG_FUNC("STUB: CGContextSetBaseCTM called\n");
	return NULL;
};

void* CGContextSetBlendMode(void) {
	if (verbose) LOG_FUNC("STUB: CGContextSetBlendMode called\n");
	return NULL;
};

void* CGContextSetFillColorSpace(void) {
	if (verbose) LOG_FUNC("STUB: CGContextSetFillColorSpace called\n");
	return NULL;
};

void* CGContextSetFillColorWithColor(void) {
	if (verbose) LOG_FUNC("STUB: CGContextSetFillColorWithColor called\n");
	return NULL;
};

void* CGContextSetFillPattern(void) {
	if (verbose) LOG_FUNC("STUB: CGContextSetFillPattern called\n");
	return NULL;
};

void* CGContextSetFlatness(void) {
	if (verbose) LOG_FUNC("STUB: CGContextSetFlatness called\n");
	return NULL;
};

void* CGContextSetFontRenderingStyle(void) {
	if (verbose) LOG_FUNC("STUB: CGContextSetFontRenderingStyle called\n");
	return NULL;
};

void* CGContextSetFontSmoothingBackgroundColor(void) {
	if (verbose) LOG_FUNC("STUB: CGContextSetFontSmoothingBackgroundColor called\n");
	return NULL;
};

void* CGContextSetFontSmoothingStyle(void) {
	if (verbose) LOG_FUNC("STUB: CGContextSetFontSmoothingStyle called\n");
	return NULL;
};

void* CGContextSetGrayFillColor(void) {
	if (verbose) LOG_FUNC("STUB: CGContextSetGrayFillColor called\n");
	return NULL;
};

void* CGContextSetInterpolationQuality(void) {
	if (verbose) LOG_FUNC("STUB: CGContextSetInterpolationQuality called\n");
	return NULL;
};

void* CGContextSetLineCap(void) {
	if (verbose) LOG_FUNC("STUB: CGContextSetLineCap called\n");
	return NULL;
};

void* CGContextSetLineDash(void) {
	if (verbose) LOG_FUNC("STUB: CGContextSetLineDash called\n");
	return NULL;
};

void* CGContextSetLineJoin(void) {
	if (verbose) LOG_FUNC("STUB: CGContextSetLineJoin called\n");
	return NULL;
};

void* CGContextSetLineWidth(void) {
	if (verbose) LOG_FUNC("STUB: CGContextSetLineWidth called\n");
	return NULL;
};

void* CGContextSetMiterLimit(void) {
	if (verbose) LOG_FUNC("STUB: CGContextSetMiterLimit called\n");
	return NULL;
};

void* CGContextSetRGBFillColor(void) {
	if (verbose) LOG_FUNC("STUB: CGContextSetRGBFillColor called\n");
	return NULL;
};

void* CGContextSetRGBStrokeColor(void) {
	if (verbose) LOG_FUNC("STUB: CGContextSetRGBStrokeColor called\n");
	return NULL;
};

void* CGContextSetShouldAntialias(void) {
	if (verbose) LOG_FUNC("STUB: CGContextSetShouldAntialias called\n");
	return NULL;
};

void* CGContextSetShouldSmoothFonts(void) {
	if (verbose) LOG_FUNC("STUB: CGContextSetShouldSmoothFonts called\n");
	return NULL;
};

void* CGContextSetShouldSubpixelPositionFonts(void) {
	if (verbose) LOG_FUNC("STUB: CGContextSetShouldSubpixelPositionFonts called\n");
	return NULL;
};

void* CGContextSetShouldSubpixelQuantizeFonts(void) {
	if (verbose) LOG_FUNC("STUB: CGContextSetShouldSubpixelQuantizeFonts called\n");
	return NULL;
};

void* CGContextSetStrokeColorWithColor(void) {
	if (verbose) LOG_FUNC("STUB: CGContextSetStrokeColorWithColor called\n");
	return NULL;
};

void* CGContextSetTextDrawingMode(void) {
	if (verbose) LOG_FUNC("STUB: CGContextSetTextDrawingMode called\n");
	return NULL;
};

void* CGContextSetTextMatrix(void) {
	if (verbose) LOG_FUNC("STUB: CGContextSetTextMatrix called\n");
	return NULL;
};

void* CGContextSetTextPosition(void) {
	if (verbose) LOG_FUNC("STUB: CGContextSetTextPosition called\n");
	return NULL;
};

void* CGContextShowGlyphsAtPositions(void) {
	if (verbose) LOG_FUNC("STUB: CGContextShowGlyphsAtPositions called\n");
	return NULL;
};

void* CGContextShowGlyphsWithAdvances(void) {
	if (verbose) LOG_FUNC("STUB: CGContextShowGlyphsWithAdvances called\n");
	return NULL;
};

void* CGContextStrokeEllipseInRect(void) {
	if (verbose) LOG_FUNC("STUB: CGContextStrokeEllipseInRect called\n");
	return NULL;
};

void* CGContextStrokeLineSegments(void) {
	if (verbose) LOG_FUNC("STUB: CGContextStrokeLineSegments called\n");
	return NULL;
};

void* CGContextStrokePath(void) {
	if (verbose) LOG_FUNC("STUB: CGContextStrokePath called\n");
	return NULL;
};

void* CGContextStrokeRect(void) {
	if (verbose) LOG_FUNC("STUB: CGContextStrokeRect called\n");
	return NULL;
};

void* CGContextStrokeRectWithWidth(void) {
	if (verbose) LOG_FUNC("STUB: CGContextStrokeRectWithWidth called\n");
	return NULL;
};

void* CGContextTranslateCTM(void) {
	if (verbose) LOG_FUNC("STUB: CGContextTranslateCTM called\n");
	return NULL;
};

void* CGCursorIsVisible(void) {
	if (verbose) LOG_FUNC("STUB: CGCursorIsVisible called\n");
	return NULL;
};

void* CGDataConsumerCreateWithCFData(void) {
	if (verbose) LOG_FUNC("STUB: CGDataConsumerCreateWithCFData called\n");
	return NULL;
};

void* CGDataConsumerRelease(void) {
	if (verbose) LOG_FUNC("STUB: CGDataConsumerRelease called\n");
	return NULL;
};

void* CGDataProviderCopyData(void) {
	if (verbose) LOG_FUNC("STUB: CGDataProviderCopyData called\n");
	return NULL;
};

void* CGDataProviderCreateWithCFData(void) {
	if (verbose) LOG_FUNC("STUB: CGDataProviderCreateWithCFData called\n");
	return NULL;
};

void* CGDataProviderCreateWithData(void) {
	if (verbose) LOG_FUNC("STUB: CGDataProviderCreateWithData called\n");
	return NULL;
};

void* CGDataProviderCreateWithURL(void) {
	if (verbose) LOG_FUNC("STUB: CGDataProviderCreateWithURL called\n");
	return NULL;
};

void* CGDataProviderRelease(void) {
	if (verbose) LOG_FUNC("STUB: CGDataProviderRelease called\n");
	return NULL;
};

void* CGDisplayCopyDisplayMode(void) {
	if (verbose) LOG_FUNC("STUB: CGDisplayCopyDisplayMode called\n");
	return NULL;
};

void* CGDisplayCreateImage(void) {
	if (verbose) LOG_FUNC("STUB: CGDisplayCreateImage called\n");
	return NULL;
};

void* CGDisplayCreateImageForRect(void) {
	if (verbose) LOG_FUNC("STUB: CGDisplayCreateImageForRect called\n");
	return NULL;
};

void* CGDisplayModeGetRefreshRate(void) {
	if (verbose) LOG_FUNC("STUB: CGDisplayModeGetRefreshRate called\n");
	return NULL;
};

void* CGDisplayModeRelease(void) {
	if (verbose) LOG_FUNC("STUB: CGDisplayModeRelease called\n");
	return NULL;
};

void* CGDisplayStreamStart(void) {
	if (verbose) LOG_FUNC("STUB: CGDisplayStreamStart called\n");
	return NULL;
};

void* CGDisplayStreamStop(void) {
	if (verbose) LOG_FUNC("STUB: CGDisplayStreamStop called\n");
	return NULL;
};

void* CGEventCopyIOHIDEvent(void) {
	if (verbose) LOG_FUNC("STUB: CGEventCopyIOHIDEvent called\n");
	return NULL;
};

void* CGEventCreate(void) {
	if (verbose) LOG_FUNC("STUB: CGEventCreate called\n");
	return NULL;
};

void* CGEventCreateKeyboardEvent(void) {
	if (verbose) LOG_FUNC("STUB: CGEventCreateKeyboardEvent called\n");
	return NULL;
};

void* CGEventCreateMouseEvent(void) {
	if (verbose) LOG_FUNC("STUB: CGEventCreateMouseEvent called\n");
	return NULL;
};

void* CGEventCreateScrollWheelEvent(void) {
	if (verbose) LOG_FUNC("STUB: CGEventCreateScrollWheelEvent called\n");
	return NULL;
};

void* CGEventGetFlags(void) {
	if (verbose) LOG_FUNC("STUB: CGEventGetFlags called\n");
	return NULL;
};

void* CGEventGetIntegerValueField(void) {
	if (verbose) LOG_FUNC("STUB: CGEventGetIntegerValueField called\n");
	return NULL;
};

void* CGEventGetLocation(void) {
	if (verbose) LOG_FUNC("STUB: CGEventGetLocation called\n");
	return NULL;
};

void* CGEventGetTimestamp(void) {
	if (verbose) LOG_FUNC("STUB: CGEventGetTimestamp called\n");
	return NULL;
};

void* CGEventKeyboardGetUnicodeString(void) {
	if (verbose) LOG_FUNC("STUB: CGEventKeyboardGetUnicodeString called\n");
	return NULL;
};

void* CGEventPost(void) {
	if (verbose) LOG_FUNC("STUB: CGEventPost called\n");
	return NULL;
};

void* CGEventSetFlags(void) {
	if (verbose) LOG_FUNC("STUB: CGEventSetFlags called\n");
	return NULL;
};

void* CGEventSetIntegerValueField(void) {
	if (verbose) LOG_FUNC("STUB: CGEventSetIntegerValueField called\n");
	return NULL;
};

void* CGEventSetSource(void) {
	if (verbose) LOG_FUNC("STUB: CGEventSetSource called\n");
	return NULL;
};

void* CGEventSetType(void) {
	if (verbose) LOG_FUNC("STUB: CGEventSetType called\n");
	return NULL;
};

void* CGEventSourceButtonState(void) {
	if (verbose) LOG_FUNC("STUB: CGEventSourceButtonState called\n");
	return NULL;
};

void* CGEventSourceCreate(void) {
	if (verbose) LOG_FUNC("STUB: CGEventSourceCreate called\n");
	return NULL;
};

void* CGEventSourceFlagsState(void) {
	if (verbose) LOG_FUNC("STUB: CGEventSourceFlagsState called\n");
	return NULL;
};

void* CGEventSourceGetKeyboardType(void) {
	if (verbose) LOG_FUNC("STUB: CGEventSourceGetKeyboardType called\n");
	return NULL;
};

void* CGEventSourceGetSourceStateID(void) {
	if (verbose) LOG_FUNC("STUB: CGEventSourceGetSourceStateID called\n");
	return NULL;
};

void* CGEventTapCreate(void) {
	if (verbose) LOG_FUNC("STUB: CGEventTapCreate called\n");
	return NULL;
};

void* CGEventTapEnable(void) {
	if (verbose) LOG_FUNC("STUB: CGEventTapEnable called\n");
	return NULL;
};

void* CGFloatIsValid(void) {
	if (verbose) LOG_FUNC("STUB: CGFloatIsValid called\n");
	return NULL;
};

void* CGFunctionCreate(void) {
	if (verbose) LOG_FUNC("STUB: CGFunctionCreate called\n");
	return NULL;
};

void* CGFunctionRelease(void) {
	if (verbose) LOG_FUNC("STUB: CGFunctionRelease called\n");
	return NULL;
};

void* CGGetActiveDisplayList(void) {
	if (verbose) LOG_FUNC("STUB: CGGetActiveDisplayList called\n");
	return NULL;
};

void* CGGetLastMouseDelta(void) {
	if (verbose) LOG_FUNC("STUB: CGGetLastMouseDelta called\n");
	return NULL;
};

void* CGGradientCreateWithColors(void) {
	if (verbose) LOG_FUNC("STUB: CGGradientCreateWithColors called\n");
	return NULL;
};

void* CGImageBlockGetBytesPerRow(void) {
	if (verbose) LOG_FUNC("STUB: CGImageBlockGetBytesPerRow called\n");
	return NULL;
};

void* CGImageBlockGetData(void) {
	if (verbose) LOG_FUNC("STUB: CGImageBlockGetData called\n");
	return NULL;
};

void* CGImageBlockGetRect(void) {
	if (verbose) LOG_FUNC("STUB: CGImageBlockGetRect called\n");
	return NULL;
};

void* CGImageBlockSetGetColorSpace(void) {
	if (verbose) LOG_FUNC("STUB: CGImageBlockSetGetColorSpace called\n");
	return NULL;
};

void* CGImageBlockSetGetComponentType(void) {
	if (verbose) LOG_FUNC("STUB: CGImageBlockSetGetComponentType called\n");
	return NULL;
};

void* CGImageBlockSetGetImageBlock(void) {
	if (verbose) LOG_FUNC("STUB: CGImageBlockSetGetImageBlock called\n");
	return NULL;
};

void* CGImageBlockSetGetPixelSize(void) {
	if (verbose) LOG_FUNC("STUB: CGImageBlockSetGetPixelSize called\n");
	return NULL;
};

void* CGImageBlockSetGetSize(void) {
	if (verbose) LOG_FUNC("STUB: CGImageBlockSetGetSize called\n");
	return NULL;
};

void* CGImageBlockSetRelease(void) {
	if (verbose) LOG_FUNC("STUB: CGImageBlockSetRelease called\n");
	return NULL;
};

void* CGImageCreate(void) {
	if (verbose) LOG_FUNC("STUB: CGImageCreate called\n");
	return NULL;
};

void* CGImageCreateWithImageInRect(void) {
	if (verbose) LOG_FUNC("STUB: CGImageCreateWithImageInRect called\n");
	return NULL;
};

void* CGImageCreateWithMaskingColors(void) {
	if (verbose) LOG_FUNC("STUB: CGImageCreateWithMaskingColors called\n");
	return NULL;
};

void* CGImageCreateWithPNGDataProvider(void) {
	if (verbose) LOG_FUNC("STUB: CGImageCreateWithPNGDataProvider called\n");
	return NULL;
};

void* CGImageGetAlphaInfo(void) {
	if (verbose) LOG_FUNC("STUB: CGImageGetAlphaInfo called\n");
	return NULL;
};

void* CGImageGetBitmapInfo(void) {
	if (verbose) LOG_FUNC("STUB: CGImageGetBitmapInfo called\n");
	return NULL;
};

void* CGImageGetBitsPerComponent(void) {
	if (verbose) LOG_FUNC("STUB: CGImageGetBitsPerComponent called\n");
	return NULL;
};

void* CGImageGetBitsPerPixel(void) {
	if (verbose) LOG_FUNC("STUB: CGImageGetBitsPerPixel called\n");
	return NULL;
};

void* CGImageGetBytesPerRow(void) {
	if (verbose) LOG_FUNC("STUB: CGImageGetBytesPerRow called\n");
	return NULL;
};

void* CGImageGetColorSpace(void) {
	if (verbose) LOG_FUNC("STUB: CGImageGetColorSpace called\n");
	return NULL;
};

void* CGImageGetDataProvider(void) {
	if (verbose) LOG_FUNC("STUB: CGImageGetDataProvider called\n");
	return NULL;
};

void* CGImageGetHeight(void) {
	if (verbose) LOG_FUNC("STUB: CGImageGetHeight called\n");
	return NULL;
};

void* CGImageGetImageProvider(void) {
	if (verbose) LOG_FUNC("STUB: CGImageGetImageProvider called\n");
	return NULL;
};

void* CGImageGetTypeID(void) {
	if (verbose) LOG_FUNC("STUB: CGImageGetTypeID called\n");
	return NULL;
};

void* CGImageGetWidth(void) {
	if (verbose) LOG_FUNC("STUB: CGImageGetWidth called\n");
	return NULL;
};

void* CGImageMaskCreate(void) {
	if (verbose) LOG_FUNC("STUB: CGImageMaskCreate called\n");
	return NULL;
};

void* CGImageProviderCopyImageBlockSetWithOptions(void) {
	if (verbose) LOG_FUNC("STUB: CGImageProviderCopyImageBlockSetWithOptions called\n");
	return NULL;
};

void* CGImageProviderGetAlphaInfo(void) {
	if (verbose) LOG_FUNC("STUB: CGImageProviderGetAlphaInfo called\n");
	return NULL;
};

void* CGImageRelease(void) {
	if (verbose) LOG_FUNC("STUB: CGImageRelease called\n");
	return NULL;
};

void* CGImageRetain(void) {
	if (verbose) LOG_FUNC("STUB: CGImageRetain called\n");
	return NULL;
};

void* CGLayerCreateWithContext(void) {
	if (verbose) LOG_FUNC("STUB: CGLayerCreateWithContext called\n");
	return NULL;
};

void* CGLayerGetContext(void) {
	if (verbose) LOG_FUNC("STUB: CGLayerGetContext called\n");
	return NULL;
};

void* CGLayerGetSize(void) {
	if (verbose) LOG_FUNC("STUB: CGLayerGetSize called\n");
	return NULL;
};

void* CGMainDisplayID(void) {
	if (verbose) LOG_FUNC("STUB: CGMainDisplayID called\n");
	return NULL;
};

void* CGPDFContextAddDestinationAtPoint(void) {
	if (verbose) LOG_FUNC("STUB: CGPDFContextAddDestinationAtPoint called\n");
	return NULL;
};

void* CGPDFContextBeginPage(void) {
	if (verbose) LOG_FUNC("STUB: CGPDFContextBeginPage called\n");
	return NULL;
};

void* CGPDFContextClose(void) {
	if (verbose) LOG_FUNC("STUB: CGPDFContextClose called\n");
	return NULL;
};

void* CGPDFContextCreate(void) {
	if (verbose) LOG_FUNC("STUB: CGPDFContextCreate called\n");
	return NULL;
};

void* CGPDFContextCreateWithURL(void) {
	if (verbose) LOG_FUNC("STUB: CGPDFContextCreateWithURL called\n");
	return NULL;
};

void* CGPDFContextEndPage(void) {
	if (verbose) LOG_FUNC("STUB: CGPDFContextEndPage called\n");
	return NULL;
};

void* CGPDFContextSetDestinationForRect(void) {
	if (verbose) LOG_FUNC("STUB: CGPDFContextSetDestinationForRect called\n");
	return NULL;
};

void* CGPDFContextSetURLForRect(void) {
	if (verbose) LOG_FUNC("STUB: CGPDFContextSetURLForRect called\n");
	return NULL;
};

void* CGPDFDocumentCreateWithProvider(void) {
	if (verbose) LOG_FUNC("STUB: CGPDFDocumentCreateWithProvider called\n");
	return NULL;
};

void* CGPDFDocumentGetNumberOfPages(void) {
	if (verbose) LOG_FUNC("STUB: CGPDFDocumentGetNumberOfPages called\n");
	return NULL;
};

void* CGPDFDocumentGetPage(void) {
	if (verbose) LOG_FUNC("STUB: CGPDFDocumentGetPage called\n");
	return NULL;
};

void* CGPDFDocumentRelease(void) {
	if (verbose) LOG_FUNC("STUB: CGPDFDocumentRelease called\n");
	return NULL;
};

void* CGPDFPageContainsWideGamutContent(void) {
	if (verbose) LOG_FUNC("STUB: CGPDFPageContainsWideGamutContent called\n");
	return NULL;
};

void* CGPDFPageGetBoxRect(void) {
	if (verbose) LOG_FUNC("STUB: CGPDFPageGetBoxRect called\n");
	return NULL;
};

void* CGPDFPageGetColorSpace(void) {
	if (verbose) LOG_FUNC("STUB: CGPDFPageGetColorSpace called\n");
	return NULL;
};

void* CGPDFPageGetDrawingTransform(void) {
	if (verbose) LOG_FUNC("STUB: CGPDFPageGetDrawingTransform called\n");
	return NULL;
};

void* CGPathAddArc(void) {
	if (verbose) LOG_FUNC("STUB: CGPathAddArc called\n");
	return NULL;
};

void* CGPathAddArcToPoint(void) {
	if (verbose) LOG_FUNC("STUB: CGPathAddArcToPoint called\n");
	return NULL;
};

void* CGPathAddCurveToPoint(void) {
	if (verbose) LOG_FUNC("STUB: CGPathAddCurveToPoint called\n");
	return NULL;
};

void* CGPathAddEllipseInRect(void) {
	if (verbose) LOG_FUNC("STUB: CGPathAddEllipseInRect called\n");
	return NULL;
};

void* CGPathAddLineToPoint(void) {
	if (verbose) LOG_FUNC("STUB: CGPathAddLineToPoint called\n");
	return NULL;
};

void* CGPathAddLines(void) {
	if (verbose) LOG_FUNC("STUB: CGPathAddLines called\n");
	return NULL;
};

void* CGPathAddPath(void) {
	if (verbose) LOG_FUNC("STUB: CGPathAddPath called\n");
	return NULL;
};

void* CGPathAddQuadCurveToPoint(void) {
	if (verbose) LOG_FUNC("STUB: CGPathAddQuadCurveToPoint called\n");
	return NULL;
};

void* CGPathAddRect(void) {
	if (verbose) LOG_FUNC("STUB: CGPathAddRect called\n");
	return NULL;
};

void* CGPathAddRects(void) {
	if (verbose) LOG_FUNC("STUB: CGPathAddRects called\n");
	return NULL;
};

void* CGPathAddRelativeArc(void) {
	if (verbose) LOG_FUNC("STUB: CGPathAddRelativeArc called\n");
	return NULL;
};

void* CGPathAddRoundedRect(void) {
	if (verbose) LOG_FUNC("STUB: CGPathAddRoundedRect called\n");
	return NULL;
};

void* CGPathApply(void) {
	if (verbose) LOG_FUNC("STUB: CGPathApply called\n");
	return NULL;
};

void* CGPathCloseSubpath(void) {
	if (verbose) LOG_FUNC("STUB: CGPathCloseSubpath called\n");
	return NULL;
};

void* CGPathContainsPoint(void) {
	if (verbose) LOG_FUNC("STUB: CGPathContainsPoint called\n");
	return NULL;
};

void* CGPathCreateCopy(void) {
	if (verbose) LOG_FUNC("STUB: CGPathCreateCopy called\n");
	return NULL;
};

void* CGPathCreateCopyByDashingPath(void) {
	if (verbose) LOG_FUNC("STUB: CGPathCreateCopyByDashingPath called\n");
	return NULL;
};

void* CGPathCreateCopyByStrokingPath(void) {
	if (verbose) LOG_FUNC("STUB: CGPathCreateCopyByStrokingPath called\n");
	return NULL;
};

void* CGPathCreateCopyByTransformingPath(void) {
	if (verbose) LOG_FUNC("STUB: CGPathCreateCopyByTransformingPath called\n");
	return NULL;
};

void* CGPathCreateMutable(void) {
	if (verbose) LOG_FUNC("STUB: CGPathCreateMutable called\n");
	return NULL;
};

void* CGPathCreateMutableCopy(void) {
	if (verbose) LOG_FUNC("STUB: CGPathCreateMutableCopy called\n");
	return NULL;
};

void* CGPathCreateWithEllipseInRect(void) {
	if (verbose) LOG_FUNC("STUB: CGPathCreateWithEllipseInRect called\n");
	return NULL;
};

void* CGPathCreateWithRect(void) {
	if (verbose) LOG_FUNC("STUB: CGPathCreateWithRect called\n");
	return NULL;
};

void* CGPathCreateWithRoundedRect(void) {
	if (verbose) LOG_FUNC("STUB: CGPathCreateWithRoundedRect called\n");
	return NULL;
};

void* CGPathGetBoundingBox(void) {
	if (verbose) LOG_FUNC("STUB: CGPathGetBoundingBox called\n");
	return NULL;
};

void* CGPathGetPathBoundingBox(void) {
	if (verbose) LOG_FUNC("STUB: CGPathGetPathBoundingBox called\n");
	return NULL;
};

void* CGPathIsEmpty(void) {
	if (verbose) LOG_FUNC("STUB: CGPathIsEmpty called\n");
	return NULL;
};

void* CGPathMoveToPoint(void) {
	if (verbose) LOG_FUNC("STUB: CGPathMoveToPoint called\n");
	return NULL;
};

void* CGPathRelease(void) {
	if (verbose) LOG_FUNC("STUB: CGPathRelease called\n");
	return NULL;
};

void* CGPatternCreate(void) {
	if (verbose) LOG_FUNC("STUB: CGPatternCreate called\n");
	return NULL;
};

void* CGPatternRelease(void) {
	if (verbose) LOG_FUNC("STUB: CGPatternRelease called\n");
	return NULL;
};

void* CGPointApplyAffineTransform(void) {
	if (verbose) LOG_FUNC("STUB: CGPointApplyAffineTransform called\n");
	return NULL;
};

void* CGPointCreateDictionaryRepresentation(void) {
	if (verbose) LOG_FUNC("STUB: CGPointCreateDictionaryRepresentation called\n");
	return NULL;
};

void* CGPointMakeWithDictionaryRepresentation(void) {
	if (verbose) LOG_FUNC("STUB: CGPointMakeWithDictionaryRepresentation called\n");
	return NULL;
};

void* CGRectApplyAffineTransform(void) {
	if (verbose) LOG_FUNC("STUB: CGRectApplyAffineTransform called\n");
	return NULL;
};

void* CGRectContainsPoint(void) {
	if (verbose) LOG_FUNC("STUB: CGRectContainsPoint called\n");
	return NULL;
};

void* CGRectContainsRect(void) {
	if (verbose) LOG_FUNC("STUB: CGRectContainsRect called\n");
	return NULL;
};

void* CGRectCreateDictionaryRepresentation(void) {
	if (verbose) LOG_FUNC("STUB: CGRectCreateDictionaryRepresentation called\n");
	return NULL;
};

void* CGRectDivide(void) {
	if (verbose) LOG_FUNC("STUB: CGRectDivide called\n");
	return NULL;
};

void* CGRectEqualToRect(void) {
	if (verbose) LOG_FUNC("STUB: CGRectEqualToRect called\n");
	return NULL;
};

void* CGRectGetHeight(void) {
	if (verbose) LOG_FUNC("STUB: CGRectGetHeight called\n");
	return NULL;
};

void* CGRectGetMaxX(void) {
	if (verbose) LOG_FUNC("STUB: CGRectGetMaxX called\n");
	return NULL;
};

void* CGRectGetMaxY(void) {
	if (verbose) LOG_FUNC("STUB: CGRectGetMaxY called\n");
	return NULL;
};

void* CGRectGetMidX(void) {
	if (verbose) LOG_FUNC("STUB: CGRectGetMidX called\n");
	return NULL;
};

void* CGRectGetMidY(void) {
	if (verbose) LOG_FUNC("STUB: CGRectGetMidY called\n");
	return NULL;
};

void* CGRectGetMinX(void) {
	if (verbose) LOG_FUNC("STUB: CGRectGetMinX called\n");
	return NULL;
};

void* CGRectGetMinY(void) {
	if (verbose) LOG_FUNC("STUB: CGRectGetMinY called\n");
	return NULL;
};

void* CGRectGetWidth(void) {
	if (verbose) LOG_FUNC("STUB: CGRectGetWidth called\n");
	return NULL;
};

void* CGRectInset(void) {
	if (verbose) LOG_FUNC("STUB: CGRectInset called\n");
	return NULL;
};

void* CGRectIntegral(void) {
	if (verbose) LOG_FUNC("STUB: CGRectIntegral called\n");
	return NULL;
};

void* CGRectIntersection(void) {
	if (verbose) LOG_FUNC("STUB: CGRectIntersection called\n");
	return NULL;
};

void* CGRectIntersectsRect(void) {
	if (verbose) LOG_FUNC("STUB: CGRectIntersectsRect called\n");
	return NULL;
};

void* CGRectIsEmpty(void) {
	if (verbose) LOG_FUNC("STUB: CGRectIsEmpty called\n");
	return NULL;
};

void* CGRectIsInfinite(void) {
	if (verbose) LOG_FUNC("STUB: CGRectIsInfinite called\n");
	return NULL;
};

void* CGRectIsNull(void) {
	if (verbose) LOG_FUNC("STUB: CGRectIsNull called\n");
	return NULL;
};

void* CGRectMakeWithDictionaryRepresentation(void) {
	if (verbose) LOG_FUNC("STUB: CGRectMakeWithDictionaryRepresentation called\n");
	return NULL;
};

void* CGRectOffset(void) {
	if (verbose) LOG_FUNC("STUB: CGRectOffset called\n");
	return NULL;
};

void* CGRectStandardize(void) {
	if (verbose) LOG_FUNC("STUB: CGRectStandardize called\n");
	return NULL;
};

void* CGRectUnion(void) {
	if (verbose) LOG_FUNC("STUB: CGRectUnion called\n");
	return NULL;
};

void* CGSAcceleratorForDisplayNumber(void) {
	if (verbose) LOG_FUNC("STUB: CGSAcceleratorForDisplayNumber called\n");
	return NULL;
};

void* CGSCopyDisplayInfoDictionary(void) {
	if (verbose) LOG_FUNC("STUB: CGSCopyDisplayInfoDictionary called\n");
	return NULL;
};

void* CGSCurrentInputPointerPosition(void) {
	if (verbose) LOG_FUNC("STUB: CGSCurrentInputPointerPosition called\n");
	return NULL;
};

void* CGSDisplayStatusQuery(void) {
	if (verbose) LOG_FUNC("STUB: CGSDisplayStatusQuery called\n");
	return NULL;
};

void* CGSFindWindowAndOwner(void) {
	if (verbose) LOG_FUNC("STUB: CGSFindWindowAndOwner called\n");
	return NULL;
};

void* CGSGetCurrentCursorLocation(void) {
	if (verbose) LOG_FUNC("STUB: CGSGetCurrentCursorLocation called\n");
	return NULL;
};

void* CGSGetCurrentDisplayMode(void) {
	if (verbose) LOG_FUNC("STUB: CGSGetCurrentDisplayMode called\n");
	return NULL;
};

void* CGSGetDisplayList(void) {
	if (verbose) LOG_FUNC("STUB: CGSGetDisplayList called\n");
	return NULL;
};

void* CGSGetDisplayModeDescriptionOfLength(void) {
	if (verbose) LOG_FUNC("STUB: CGSGetDisplayModeDescriptionOfLength called\n");
	return NULL;
};

void* CGSGetGlobalHotKeyOperatingMode(void) {
	if (verbose) LOG_FUNC("STUB: CGSGetGlobalHotKeyOperatingMode called\n");
	return NULL;
};

void* CGSGetSymbolicHotKeyValuesAndStates(void) {
	if (verbose) LOG_FUNC("STUB: CGSGetSymbolicHotKeyValuesAndStates called\n");
	return NULL;
};

void* CGSGetWindowTransformAtPlacement(void) {
	if (verbose) LOG_FUNC("STUB: CGSGetWindowTransformAtPlacement called\n");
	return NULL;
};

void* CGSInputButtonState(void) {
	if (verbose) LOG_FUNC("STUB: CGSInputButtonState called\n");
	return NULL;
};

void* CGSMainConnectionID(void) {
	if (verbose) LOG_FUNC("STUB: CGSMainConnectionID called\n");
	return NULL;
};

void* CGSRegisterNotifyProc(void) {
	if (verbose) LOG_FUNC("STUB: CGSRegisterNotifyProc called\n");
	return NULL;
};

void* CGSRemoveNotifyProc(void) {
	if (verbose) LOG_FUNC("STUB: CGSRemoveNotifyProc called\n");
	return NULL;
};

void* CGSServerOperationState(void) {
	if (verbose) LOG_FUNC("STUB: CGSServerOperationState called\n");
	return NULL;
};

void* CGSSessionCopyAllSessionProperties(void) {
	if (verbose) LOG_FUNC("STUB: CGSSessionCopyAllSessionProperties called\n");
	return NULL;
};

void* CGSSessionReleaseSessionID(void) {
	if (verbose) LOG_FUNC("STUB: CGSSessionReleaseSessionID called\n");
	return NULL;
};

void* CGSSetGlobalHotKeyOperatingMode(void) {
	if (verbose) LOG_FUNC("STUB: CGSSetGlobalHotKeyOperatingMode called\n");
	return NULL;
};

void* CGSSetWindowBackgroundBlurRadius(void) {
	if (verbose) LOG_FUNC("STUB: CGSSetWindowBackgroundBlurRadius called\n");
	return NULL;
};

void* CGSSetWindowTransformAtPlacement(void) {
	if (verbose) LOG_FUNC("STUB: CGSSetWindowTransformAtPlacement called\n");
	return NULL;
};

void* CGShadingCreateAxial(void) {
	if (verbose) LOG_FUNC("STUB: CGShadingCreateAxial called\n");
	return NULL;
};

void* CGShadingRelease(void) {
	if (verbose) LOG_FUNC("STUB: CGShadingRelease called\n");
	return NULL;
};

void* CGSizeEqualToSize(void) {
	if (verbose) LOG_FUNC("STUB: CGSizeEqualToSize called\n");
	return NULL;
};

void* CGSizeMakeWithDictionaryRepresentation(void) {
	if (verbose) LOG_FUNC("STUB: CGSizeMakeWithDictionaryRepresentation called\n");
	return NULL;
};

void* CGWindowLevelForKey(void) {
	if (verbose) LOG_FUNC("STUB: CGWindowLevelForKey called\n");
	return NULL;
};

void* CGWindowListCopyWindowInfo(void) {
	if (verbose) LOG_FUNC("STUB: CGWindowListCopyWindowInfo called\n");
	return NULL;
};


void* CGSSetDenyWindowServerConnections(void) {
	if (verbose) LOG_FUNC("STUB: CGSSetDenyWindowServerConnections called\n");
	return NULL;
};

