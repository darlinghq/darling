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

#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
	verbose = getenv("STUB_VERBOSE") != NULL;
}

void __simple_kprintf(const char* format, ...) __attribute__((format(printf, 1, 2)));

#define LOG_FUNC __simple_kprintf

NSString *const kCGImageDestinationLossyCompressionQuality = @"kCGImageDestinationLossyCompressionQuality";
NSString *const kCGImagePropertyAPNGDelayTime = @"kCGImagePropertyAPNGDelayTime";
NSString *const kCGImagePropertyAPNGLoopCount = @"kCGImagePropertyAPNGLoopCount";
NSString *const kCGImagePropertyDPIHeight = @"kCGImagePropertyDPIHeight";
NSString *const kCGImagePropertyDPIWidth = @"kCGImagePropertyDPIWidth";
NSString *const kCGImagePropertyHasAlpha = @"kCGImagePropertyHasAlpha";
NSString *const kCGImagePropertyJFIFDensityUnit = @"kCGImagePropertyJFIFDensityUnit";
NSString *const kCGImagePropertyJFIFDictionary = @"kCGImagePropertyJFIFDictionary";
NSString *const kCGImagePropertyJFIFXDensity = @"kCGImagePropertyJFIFXDensity";
NSString *const kCGImagePropertyJFIFYDensity = @"kCGImagePropertyJFIFYDensity";
NSString *const kCGImagePropertyOrientation = @"kCGImagePropertyOrientation";
NSString *const kCGImagePropertyPNGDictionary = @"kCGImagePropertyPNGDictionary";
NSString *const kCGImagePropertyPNGXPixelsPerMeter = @"kCGImagePropertyPNGXPixelsPerMeter";
NSString *const kCGImagePropertyPNGYPixelsPerMeter = @"kCGImagePropertyPNGYPixelsPerMeter";
NSString *const kCGImagePropertyPixelHeight = @"kCGImagePropertyPixelHeight";
NSString *const kCGImagePropertyPixelWidth = @"kCGImagePropertyPixelWidth";
NSString *const kCGImagePropertyProfileName = @"kCGImagePropertyProfileName";
NSString *const kCGImagePropertyTIFFDictionary = @"kCGImagePropertyTIFFDictionary";
NSString *const kCGImagePropertyTIFFResolutionUnit = @"kCGImagePropertyTIFFResolutionUnit";
NSString *const kCGImagePropertyTIFFXResolution = @"kCGImagePropertyTIFFXResolution";
NSString *const kCGImagePropertyTIFFYResolution = @"kCGImagePropertyTIFFYResolution";
NSString *const kCGImageSourceCreateThumbnailFromImageAlways = @"kCGImageSourceCreateThumbnailFromImageAlways";
NSString *const kCGImageSourceCreateThumbnailFromImageIfAbsent = @"kCGImageSourceCreateThumbnailFromImageIfAbsent";
NSString *const kCGImageSourceCreateThumbnailWithTransform = @"kCGImageSourceCreateThumbnailWithTransform";
NSString *const kCGImageSourceShouldAllowFloat = @"kCGImageSourceShouldAllowFloat";
NSString *const kCGImageSourceShouldCache = @"kCGImageSourceShouldCache";
NSString *const kCGImageSourceSubsampleFactor = @"kCGImageSourceSubsampleFactor";
NSString *const kCGImageSourceThumbnailMaxPixelSize = @"kCGImageSourceThumbnailMaxPixelSize";

void* CGImageCreateByScaling(void) {
	if (verbose) LOG_FUNC("STUB: CGImageCreateByScaling called\n");
	return NULL;
};

void* CGImageDestinationAddImage(void) {
	if (verbose) LOG_FUNC("STUB: CGImageDestinationAddImage called\n");
	return NULL;
};

void* CGImageDestinationAddImageFromSource(void) {
	if (verbose) LOG_FUNC("STUB: CGImageDestinationAddImageFromSource called\n");
	return NULL;
};

void* CGImageDestinationCreateWithData(void) {
	if (verbose) LOG_FUNC("STUB: CGImageDestinationCreateWithData called\n");
	return NULL;
};

void* CGImageDestinationCreateWithDataConsumer(void) {
	if (verbose) LOG_FUNC("STUB: CGImageDestinationCreateWithDataConsumer called\n");
	return NULL;
};

void* CGImageDestinationCreateWithURL(void) {
	if (verbose) LOG_FUNC("STUB: CGImageDestinationCreateWithURL called\n");
	return NULL;
};

void* CGImageDestinationFinalize(void) {
	if (verbose) LOG_FUNC("STUB: CGImageDestinationFinalize called\n");
	return NULL;
};

void* CGImageDestinationSetProperties(void) {
	if (verbose) LOG_FUNC("STUB: CGImageDestinationSetProperties called\n");
	return NULL;
};

void* CGImageSourceCopyProperties(void) {
	if (verbose) LOG_FUNC("STUB: CGImageSourceCopyProperties called\n");
	return NULL;
};

void* CGImageSourceCopyPropertiesAtIndex(void) {
	if (verbose) LOG_FUNC("STUB: CGImageSourceCopyPropertiesAtIndex called\n");
	return NULL;
};

void* CGImageSourceCopyTypeIdentifiers(void) {
	if (verbose) LOG_FUNC("STUB: CGImageSourceCopyTypeIdentifiers called\n");
	return NULL;
};

void* CGImageSourceCreateImageAtIndex(void) {
	if (verbose) LOG_FUNC("STUB: CGImageSourceCreateImageAtIndex called\n");
	return NULL;
};

void* CGImageSourceCreateThumbnailAtIndex(void) {
	if (verbose) LOG_FUNC("STUB: CGImageSourceCreateThumbnailAtIndex called\n");
	return NULL;
};

void* CGImageSourceCreateWithData(void) {
	if (verbose) LOG_FUNC("STUB: CGImageSourceCreateWithData called\n");
	return NULL;
};

void* CGImageSourceCreateWithURL(void) {
	if (verbose) LOG_FUNC("STUB: CGImageSourceCreateWithURL called\n");
	return NULL;
};

void* CGImageSourceGetCount(void) {
	if (verbose) LOG_FUNC("STUB: CGImageSourceGetCount called\n");
	return NULL;
};

void* CGImageSourceGetType(void) {
	if (verbose) LOG_FUNC("STUB: CGImageSourceGetType called\n");
	return NULL;
};


