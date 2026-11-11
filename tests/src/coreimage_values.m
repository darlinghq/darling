// Copyright (C) 2026 Lubos Dolezel
//
// CoreImage constants have the values macOS gives them (apps compare these strings and
// the pixel format numbers), CIImage reports an extent, CIColor keeps the components it
// was made from. The expected values were captured on macOS 27.0.1.
#import <CoreImage/CoreImage.h>
#import <CoreGraphics/CoreGraphics.h>
#include <stdio.h>
#include <string.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void checkString(NSString* actual, const char* expected, int line)
{
	if (actual == nil || strcmp([actual UTF8String], expected) != 0)
	{
		printf("FAIL %d: \"%s\" expected, got \"%s\"\n", line, expected, actual ? [actual UTF8String] : "(nil)");
		g_failures++;
	}
}

#define CHECK_STRING(actual, expected) checkString(actual, expected, __LINE__)

static BOOL near(CGFloat a, CGFloat b)
{
	return a - b < 0.001 && b - a < 0.001;
}

int main(void)
{
	@autoreleasepool
	{
		CHECK_STRING(kCIAttributeClass, "CIAttributeClass");
		CHECK_STRING(kCIAttributeDefault, "CIAttributeDefault");
		CHECK_STRING(kCIAttributeDisplayName, "CIAttributeDisplayName");
		CHECK_STRING(kCIAttributeFilterDisplayName, "CIAttributeFilterDisplayName");
		CHECK_STRING(kCIAttributeFilterName, "CIAttributeFilterName");
		CHECK_STRING(kCIAttributeMax, "CIAttributeMax");
		CHECK_STRING(kCIAttributeMin, "CIAttributeMin");
		CHECK_STRING(kCIAttributeSliderMin, "CIAttributeSliderMin");
		CHECK_STRING(kCIAttributeSliderMax, "CIAttributeSliderMax");
		CHECK_STRING(kCIAttributeType, "CIAttributeType");

		CHECK_STRING(kCIAttributeTypeAngle, "CIAttributeTypeAngle");
		CHECK_STRING(kCIAttributeTypeBoolean, "CIAttributeTypeBoolean");
		CHECK_STRING(kCIAttributeTypeDistance, "CIAttributeTypeDistance");
		CHECK_STRING(kCIAttributeTypeOffset, "CIAttributeTypeOffset");
		CHECK_STRING(kCIAttributeTypePosition, "CIAttributeTypePosition");
		CHECK_STRING(kCIAttributeTypePosition3, "CIAttributeTypePosition3");
		CHECK_STRING(kCIAttributeTypeRectangle, "CIAttributeTypeRectangle");
		CHECK_STRING(kCIAttributeTypeScalar, "CIAttributeTypeScalar");
		CHECK_STRING(kCIAttributeTypeTime, "CIAttributeTypeTime");

		CHECK_STRING(kCICategoryCompositeOperation, "CICategoryCompositeOperation");
		CHECK_STRING(kCICategoryGenerator, "CICategoryGenerator");
		CHECK_STRING(kCICategoryGradient, "CICategoryGradient");
		CHECK_STRING(kCICategoryReduction, "CICategoryReduction");
		CHECK_STRING(kCICategoryTransition, "CICategoryTransition");

		CHECK(kCIFormatARGB8 == 265);
		CHECK(kCIFormatRGBA8 == 264);
		CHECK(kCIFormatBGRA8 == 266);
		CHECK(kCIFormatABGR8 == 267);
		CHECK(kCIFormatRGBAh == 2056);
		CHECK(kCIFormatRGBA16 == 1800);
		CHECK(kCIFormatRGBAf == 2312);

		// CIImage extent: the pixel size of a CGImage, at the origin
		CGColorSpaceRef rgb = CGColorSpaceCreateDeviceRGB();
		CGContextRef context = CGBitmapContextCreate(NULL, 7, 5, 8, 0, rgb, kCGImageAlphaPremultipliedLast);
		CGImageRef image = CGBitmapContextCreateImage(context);
		CIImage* ciImage = [[CIImage alloc] initWithCGImage:image];
		CGRect extent = [ciImage extent];
		CHECK(extent.origin.x == 0 && extent.origin.y == 0 && extent.size.width == 7 && extent.size.height == 5);
		[ciImage release];
		CGImageRelease(image);
		CGContextRelease(context);
		CGColorSpaceRelease(rgb);

		// The empty image has the null extent (macOS reports CGRectNull)
		CIImage* empty = [CIImage emptyImage];
		CHECK(empty != nil);
		CHECK(CGRectIsNull([empty extent]));

		// CIColor
		CIColor* color = [CIColor colorWithRed:0.25 green:0.5 blue:0.75 alpha:0.5];
		CHECK(near([color red], 0.25) && near([color green], 0.5) && near([color blue], 0.75) && near([color alpha], 0.5));
		CHECK([color numberOfComponents] == 4);
		CHECK([color components] != NULL);
		CIColor* opaque = [CIColor colorWithRed:1 green:0 blue:0];
		CHECK(near([opaque alpha], 1.0));

		// A gray CGColor keeps its gray model and reads back as equal RGB components
		CGColorSpaceRef gray = CGColorSpaceCreateDeviceGray();
		CGFloat grayComponents[2] = { 0.4, 1.0 };
		CGColorRef grayColor = CGColorCreate(gray, grayComponents);
		CIColor* fromGray = [CIColor colorWithCGColor:grayColor];
		CHECK(near([fromGray red], 0.4) && near([fromGray green], 0.4) && near([fromGray blue], 0.4));
		CHECK([fromGray numberOfComponents] == 2);
		CHECK(near([fromGray alpha], 1.0));
		CGColorRelease(grayColor);
		CGColorSpaceRelease(gray);

		// -[CIFilter setDefaults] exists
		CHECK([CIFilter instancesRespondToSelector:@selector(setDefaults)]);
	}

	if (g_failures)
	{
		printf("%d failures\n", g_failures);
		return 1;
	}
	printf("PASS\n");
	return 0;
}
