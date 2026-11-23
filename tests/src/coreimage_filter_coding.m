// CFLAGS: -framework Foundation -framework CoreImage
// Copyright (C) 2026 Darling developers
//
// CIFilter conforms to NSSecureCoding and manages its input parameters when archiving and deallocating
// (https://developer.apple.com/documentation/coreimage/cifilter-swift.class, "Subclassing notes"): the
// parameters are the instance variables whose names start with "input". This test uses a subclass written
// the way that page describes. The archive layout is not documented, so the tests only check what a round
// trip through our own archive gives back, plus that decoding an archive without parameters does not raise.
// Prints OK, or the failures with a non-zero exit status.
#import <CoreImage/CoreImage.h>
#import <Foundation/Foundation.h>
#include <stdio.h>

static int failures;
static int probeDeallocations;

#define CHECK(cond, msg) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, msg); failures++; } } while (0)

@interface CodingProbe : NSObject
@end

@implementation CodingProbe
- (void) dealloc {
    probeDeallocations++;
    [super dealloc];
}
@end

@interface CodingTestFilter : CIFilter {
    NSNumber *inputAmount;
    NSString *inputLabel;
    id inputProbe;
    NSAffineTransform *inputTransform;
    float inputScalar;   // not an object: must be skipped, setting it to nil through key-value coding raises
    NSNumber *notAnInput;
}
@end

@implementation CodingTestFilter
@end

static NSData *archive(id object, BOOL secure) {
    NSError *error = nil;
    NSData *data = [NSKeyedArchiver archivedDataWithRootObject: object requiringSecureCoding: secure error: &error];
    CHECK(data != nil, "the filter archives");
    return data;
}

// Returns the decoded object; a raised exception is a failure, not a crash. The plain decoder is the
// one a nib uses, the secure one is what -requiresSecureCoding callers use.
static id unarchive(NSData *data, BOOL secure) {
    id result = nil;
    @try {
        if (secure) {
            NSError *error = nil;
            result = [NSKeyedUnarchiver unarchivedObjectOfClass: [CIFilter class] fromData: data error: &error];
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
        // the documented class conformance
        CHECK([CIFilter conformsToProtocol: @protocol(NSSecureCoding)], "CIFilter conforms to NSSecureCoding");
        CHECK([CIFilter conformsToProtocol: @protocol(NSCoding)], "CIFilter conforms to NSCoding");
        CHECK([CIFilter supportsSecureCoding], "CIFilter supports secure coding");

        // input parameters are the "input"-prefixed object instance variables, nothing else
        CodingTestFilter *filter = [[[CodingTestFilter alloc] init] autorelease];
        NSArray *keys = @[@"inputAmount", @"inputLabel", @"inputProbe", @"inputTransform"];
        CHECK([[filter inputKeys] isEqualToArray: keys], "inputKeys lists the object input instance variables, not scalar ones");

        for (int secure = 0; secure <= 1; secure++) {
            // values set through key-value coding survive a round trip, in a secure and a plain coder
            [filter setValue: @2.5 forKey: @"inputAmount"];
            [filter setValue: @"caption" forKey: @"inputLabel"];
            id decoded = unarchive(archive(filter, secure), secure);
            CHECK([decoded isMemberOfClass: [CodingTestFilter class]], "the decoded object has the archived class");
            CHECK([[decoded valueForKey: @"inputAmount"] isEqual: @2.5], "the number parameter is decoded");
            CHECK([[decoded valueForKey: @"inputLabel"] isEqual: @"caption"], "the string parameter is decoded");

            // the filter reference lists NSAffineTransform as a parameter class (inputTransform)
            NSAffineTransform *transform = [NSAffineTransform transform];
            [transform translateXBy: 3 yBy: 4];
            [filter setValue: transform forKey: @"inputTransform"];
            decoded = unarchive(archive(filter, secure), secure);
            CHECK([[decoded valueForKey: @"inputTransform"] isEqual: transform], "an NSAffineTransform parameter is decoded");
            [filter setValue: nil forKey: @"inputTransform"];

            // parameters without a value are not stored, and an archive without any still decodes
            CodingTestFilter *empty = [[[CodingTestFilter alloc] init] autorelease];
            decoded = unarchive(archive(empty, secure), secure);
            CHECK([decoded isMemberOfClass: [CodingTestFilter class]], "a filter without parameters decodes");
            CHECK([decoded valueForKey: @"inputAmount"] == nil && [decoded valueForKey: @"inputLabel"] == nil,
                  "absent parameters stay without a value");

            // a partial filter keeps what it had
            CodingTestFilter *partial = [[[CodingTestFilter alloc] init] autorelease];
            [partial setValue: @7 forKey: @"inputAmount"];
            decoded = unarchive(archive(partial, secure), secure);
            CHECK([[decoded valueForKey: @"inputAmount"] isEqual: @7] && [decoded valueForKey: @"inputLabel"] == nil,
                  "a partly set filter decodes with the same parameters set");
        }
    }

    // the CIFilter implementation of dealloc clears the parameters, so a subclass does not release them
    @autoreleasepool {
        CodingTestFilter *holder = [[CodingTestFilter alloc] init];
        CodingProbe *probe = [[CodingProbe alloc] init];
        [holder setValue: probe forKey: @"inputProbe"];
        [probe release];
        CHECK(probeDeallocations == 0, "the filter keeps its parameter alive");
        [holder release];
    }
    CHECK(probeDeallocations == 1, "deallocating the filter releases its parameters");

    printf(failures ? "FAILED\n" : "OK\n");
    return failures ? 1 : 0;
}
