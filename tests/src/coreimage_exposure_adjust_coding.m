// CFLAGS: -framework Foundation -framework CoreImage
// Copyright (C) 2026 Darling developers
//
// A keyed archive that holds a CIExposureAdjust filter decodes. Sources:
// https://developer.apple.com/documentation/coreimage/cifilter-swift.class (CIFilter conforms to
// NSSecureCoding, manages its input parameters when archiving, and -setDefaults leaves parameters
// that have no default unchanged), https://developer.apple.com/documentation/coreimage/ciexposureadjust
// (the parameters are inputImage and inputEV) and the filter reference
// https://developer.apple.com/library/archive/documentation/GraphicsImaging/Reference/CoreImageFilterReference/index.html
// (inputEV is an NSNumber; its default is 0, as observed on macOS 26.7.3 and 10.14.4, although the page says 0.50). The archive layout is not documented, so the tests
// only use archives written by this implementation. The class is looked up by name because on macOS
// CIExposureAdjust is a protocol in the public headers.
// Prints OK, or the failures with a non-zero exit status.
#import <CoreImage/CoreImage.h>
#import <Foundation/Foundation.h>
#include <stdio.h>

static int failures;

#define CHECK(cond, msg) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, msg); failures++; } } while (0)

static NSData *archive(id object, BOOL secure) {
    NSError *error = nil;
    NSData *data = [NSKeyedArchiver archivedDataWithRootObject: object requiringSecureCoding: secure error: &error];
    CHECK(data != nil, "the object archives");
    return data;
}

// A raised exception is a failure, not a crash. The plain decoder is the one a nib uses.
static id unarchive(NSData *data, BOOL secure) {
    id result = nil;
    @try {
        if (secure) {
            NSError *error = nil;
            NSSet *classes = [NSSet setWithObjects: [NSArray class], [CIFilter class], nil];
            result = [NSKeyedUnarchiver unarchivedObjectOfClasses: classes fromData: data error: &error];
        } else {
            NSKeyedUnarchiver *unarchiver = [[NSKeyedUnarchiver alloc] initForReadingWithData: data];
            result = [unarchiver decodeObjectForKey: NSKeyedArchiveRootObjectKey];
            [unarchiver finishDecoding];
            [unarchiver release];
        }
    } @catch (NSException *exception) {
        printf("FAIL line %d: decoding raised %s\n", __LINE__, [[exception name] UTF8String]);
        failures++;
    }
    return result;
}

int main(void) {
    @autoreleasepool {
        Class filterClass = NSClassFromString(@"CIExposureAdjust");
        CHECK(filterClass != Nil && [filterClass isSubclassOfClass: [CIFilter class]], "CIExposureAdjust is a CIFilter");
        if (filterClass == Nil) {
            printf("FAILED\n");
            return 1;
        }

        id filter = [[[filterClass alloc] init] autorelease];
        CHECK([[filter inputKeys] containsObject: kCIInputEVKey], "inputEV is an input parameter");
        CHECK([[filter inputKeys] containsObject: kCIInputImageKey], "inputImage is an input parameter");

        for (int secure = 0; secure <= 1; secure++) {
            // a filter with its exposure set, inside an array like the one a nib stores objects in
            [filter setValue: @1.25 forKey: kCIInputEVKey];
            NSArray *decodedArray = unarchive(archive(@[filter], secure), secure);
            CHECK([decodedArray count] == 1, "the array decodes with its element");
            id decoded = [decodedArray firstObject];
            CHECK([decoded isMemberOfClass: filterClass], "the decoded filter has the archived class");
            CHECK([[decoded valueForKey: kCIInputEVKey] isEqual: @1.25], "inputEV is decoded");
            CHECK([decoded valueForKey: kCIInputImageKey] == nil, "a filter without an image decodes with a nil inputImage");

            // a filter with no parameter set still decodes; the defaults apply after -setDefaults
            id blank = [[[filterClass alloc] init] autorelease];
            decoded = unarchive(archive(blank, secure), secure);
            CHECK([decoded isMemberOfClass: filterClass], "a filter without parameters decodes");
            CHECK([decoded valueForKey: kCIInputEVKey] == nil, "decoding applies no defaults");
            [decoded setDefaults];
            CHECK([[decoded valueForKey: kCIInputEVKey] isEqual: @0], "-setDefaults gives inputEV its default of 0");
            CHECK([decoded valueForKey: kCIInputImageKey] == nil, "-setDefaults has no default for inputImage");
        }

        // -setDefaults changes inputEV and leaves a parameter without a default alone
        id image = [CIImage emptyImage];
        id other = [[[filterClass alloc] init] autorelease];
        [other setValue: image forKey: kCIInputImageKey];
        [other setValue: @-3 forKey: kCIInputEVKey];
        [other setDefaults];
        CHECK([[other valueForKey: kCIInputEVKey] isEqual: @0], "-setDefaults replaces a set inputEV");
        CHECK([other valueForKey: kCIInputImageKey] == image, "-setDefaults leaves inputImage unchanged");
    }
    printf(failures ? "FAILED\n" : "OK\n");
    return failures ? 1 : 0;
}
