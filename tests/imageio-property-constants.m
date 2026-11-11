// Copyright (C) 2026 Lubos Dolezel
//
// ImageIO property constants and CGImageMetadata entry points.
//   clang -fobjc-arc -framework Foundation -framework ImageIO tests/imageio-property-constants.m -o /tmp/imageio-property-constants
// Exits non-zero on failure.
#import <Foundation/Foundation.h>
#import <ImageIO/ImageIO.h>
#include <stdio.h>

static int failures;

static void expect_string(const char *name, CFStringRef value, const char *expected) {
	char buf[256];
	if (!value || !CFStringGetCString(value, buf, sizeof(buf), kCFStringEncodingUTF8)) {
		printf("FAIL: %s is NULL or not convertible\n", name);
		failures++;
	} else if (strcmp(buf, expected) != 0) {
		printf("FAIL: %s is '%s', expected '%s'\n", name, buf, expected);
		failures++;
	}
}

int main(void) {
	expect_string("kCGImagePropertyGIFDictionary", kCGImagePropertyGIFDictionary, "{GIF}");
	expect_string("kCGImagePropertyGIFHasGlobalColorMap", kCGImagePropertyGIFHasGlobalColorMap, "HasGlobalColorMap");
	expect_string("kCGImagePropertyGIFImageColorMap", kCGImagePropertyGIFImageColorMap, "ImageColorMap");
	expect_string("kCGImagePropertyIPTCDictionary", kCGImagePropertyIPTCDictionary, "{IPTC}");
	expect_string("kCGImagePropertyIPTCKeywords", kCGImagePropertyIPTCKeywords, "Keywords");
	expect_string("kCGImagePropertyPNGAuthor", kCGImagePropertyPNGAuthor, "Author");
	expect_string("kCGImagePropertyPNGDescription", kCGImagePropertyPNGDescription, "Description");
	expect_string("kCGImagePropertyAPNGDelayTime", kCGImagePropertyAPNGDelayTime, "DelayTime");
	expect_string("kCGImagePropertyAPNGLoopCount", kCGImagePropertyAPNGLoopCount, "LoopCount");
	expect_string("kCGImageMetadataNamespaceExif", kCGImageMetadataNamespaceExif, "http://ns.adobe.com/exif/1.0/");
	expect_string("kCGImageMetadataPrefixExif", kCGImageMetadataPrefixExif, "exif");

	CGMutableImageMetadataRef metadata = CGImageMetadataCreateMutable();
	if (!metadata) {
		printf("FAIL: CGImageMetadataCreateMutable returned NULL\n");
		failures++;
	} else {
		CGImageMetadataTagRef tag = CGImageMetadataTagCreate(kCGImageMetadataNamespaceExif, kCGImageMetadataPrefixExif,
			CFSTR("UserComment"), 0, CFSTR("comment"));
		if (!tag || !CGImageMetadataSetTagWithPath(metadata, NULL, CFSTR("exif:UserComment"), tag)) {
			printf("FAIL: CGImageMetadataSetTagWithPath rejected a valid tag\n");
			failures++;
		}
		if (tag)
			CFRelease(tag);
		if (CGImageMetadataSetTagWithPath(NULL, NULL, CFSTR("exif:UserComment"), NULL)) {
			printf("FAIL: CGImageMetadataSetTagWithPath accepted a NULL metadata\n");
			failures++;
		}
		CFRelease(metadata);
	}

	// A source without data holds no images and has no properties.
	CGImageSourceRef source = CGImageSourceCreateWithData((__bridge CFDataRef)[NSData data], NULL);
	if (source) {
		CFDictionaryRef props = CGImageSourceCopyProperties(source, NULL);
		if (props && CGImageSourceGetCount(source) == 0) {
			printf("FAIL: CGImageSourceCopyProperties returned properties for an empty source\n");
			failures++;
		}
		if (props)
			CFRelease(props);
		CFRelease(source);
	}

	if (failures == 0)
		printf("PASS: ImageIO property constants and metadata entry points\n");
	return failures != 0;
}
