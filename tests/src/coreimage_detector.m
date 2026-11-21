// Copyright (C) 2026 Lubos Dolezel
//
// CIDetector on an image with nothing to detect. Expected results from macOS 10.14.4 and 26.7.3:
// the detector types and option keys are the documented strings, an unknown (or nil) type gives no
// detector and a known type gives a detector whose featuresInImage: returns an empty array for an
// image without that feature. Darling's detectors never find anything, so only that case is tested.
#import <CoreGraphics/CoreGraphics.h>
#import <CoreImage/CIDetector.h>
#import <CoreImage/CIFeature.h>
#import <CoreImage/CIImage.h>
#import <Foundation/Foundation.h>
#include <stdio.h>

static int failures;
static void expect(BOOL ok, NSString *what) {
    printf("%s %s\n", ok ? "PASS" : "FAIL", [what UTF8String]);
    if (!ok)
        failures++;
}

int main(void) {
    @autoreleasepool {
        expect([CIDetectorTypeFace isEqual:@"CIDetectorTypeFace"], @"CIDetectorTypeFace");
        expect([CIDetectorTypeRectangle isEqual:@"CIDetectorTypeRectangle"], @"CIDetectorTypeRectangle");
        expect([CIDetectorTypeQRCode isEqual:@"CIDetectorTypeQRCode"], @"CIDetectorTypeQRCode");
        expect([CIDetectorTypeText isEqual:@"CIDetectorTypeText"], @"CIDetectorTypeText");
        expect([CIDetectorAccuracy isEqual:@"CIDetectorAccuracy"], @"CIDetectorAccuracy");
        expect([CIDetectorAccuracyLow isEqual:@"CIDetectorAccuracyLow"], @"CIDetectorAccuracyLow");
        expect([CIDetectorAccuracyHigh isEqual:@"CIDetectorAccuracyHigh"], @"CIDetectorAccuracyHigh");
        expect([CIDetectorTracking isEqual:@"CIDetectorTracking"], @"CIDetectorTracking");
        expect([CIDetectorMinFeatureSize isEqual:@"CIDetectorMinFeatureSize"], @"CIDetectorMinFeatureSize");
        expect([CIDetectorImageOrientation isEqual:@"CIDetectorImageOrientation"], @"CIDetectorImageOrientation");
        expect([CIFeatureTypeFace isEqual:@"Face"], @"CIFeatureTypeFace");
        expect([CIFeatureTypeRectangle isEqual:@"Rectangle"], @"CIFeatureTypeRectangle");
        expect([CIFeatureTypeQRCode isEqual:@"QRCode"], @"CIFeatureTypeQRCode");
        expect([CIFeatureTypeText isEqual:@"Text"], @"CIFeatureTypeText");

        CGColorSpaceRef rgb = CGColorSpaceCreateDeviceRGB();
        CGContextRef bitmap = CGBitmapContextCreate(NULL, 64, 64, 8, 0, rgb, kCGImageAlphaPremultipliedLast);
        CGContextSetRGBFillColor(bitmap, 1, 1, 1, 1);
        CGContextFillRect(bitmap, CGRectMake(0, 0, 64, 64));
        CGImageRef blank = CGBitmapContextCreateImage(bitmap);
        CIImage *white = [[[CIImage alloc] initWithCGImage:blank] autorelease];
        CGImageRelease(blank);
        CGContextRelease(bitmap);
        CGColorSpaceRelease(rgb);
        expect(white != nil, @"a white 64x64 CIImage");

        NSArray *types = @[ CIDetectorTypeFace, CIDetectorTypeRectangle, CIDetectorTypeQRCode, CIDetectorTypeText ];
        for (NSString *type in types) {
            CIDetector *detector = [CIDetector detectorOfType:type context:nil
                                                      options:@{ CIDetectorAccuracy : CIDetectorAccuracyLow }];
            expect(detector != nil, [type stringByAppendingString:@": a detector is created"]);
            NSArray *features = [detector featuresInImage:white];
            expect(features != nil && [features count] == 0,
                  [type stringByAppendingString:@": no features in a blank image"]);
            features = [detector featuresInImage:white options:@{ CIDetectorImageOrientation : @1 }];
            expect(features != nil && [features count] == 0,
                  [type stringByAppendingString:@": no features in a blank image, with options"]);
        }
        expect([CIDetector detectorOfType:CIDetectorTypeQRCode context:nil options:nil] != nil,
              @"a detector without options");
        expect([CIDetector detectorOfType:@"CIDetectorTypeBogus" context:nil options:nil] == nil,
              @"an unknown detector type gives nil");
        expect([CIDetector detectorOfType:nil context:nil options:nil] == nil, @"a nil detector type gives nil");
        printf("RESULT failures=%d\n", failures);
    }
    return failures != 0;
}
