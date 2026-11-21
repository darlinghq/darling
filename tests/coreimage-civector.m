// CIVector value class, from the Apple documentation (https://developer.apple.com/documentation/coreimage/civector).
// Build as an Objective-C executable linked against Foundation, CoreGraphics and CoreImage:
//   clang -fno-objc-arc -framework Foundation -framework CoreGraphics -framework CoreImage tests/coreimage-civector.m -o /tmp/civector-test
// Run it with `darling shell`. Exits non-zero on failure.
// The QUIRK section pins behaviour the documentation does not state (marked QUIRK in the implementation).
#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <CoreImage/CIVector.h>

static int failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); failures++; } } while (0)

int main(void) {
    @autoreleasepool {
        // vectors of length 1 to 4 hold the values given, in order
        CIVector *one = [CIVector vectorWithX: 1.5];
        CHECK([one count] == 1 && [one valueAtIndex: 0] == 1.5 && [one X] == 1.5);
        CIVector *two = [CIVector vectorWithX: 1 Y: 2];
        CHECK([two count] == 2 && [two X] == 1 && [two Y] == 2);
        CIVector *three = [CIVector vectorWithX: 1 Y: 2 Z: 3];
        CHECK([three count] == 3 && [three Z] == 3);
        CIVector *four = [CIVector vectorWithX: 1 Y: 2 Z: 3 W: 4];
        CHECK([four count] == 4 && [four X] == 1 && [four Y] == 2 && [four Z] == 3 && [four W] == 4);

        CIVector *initialised = [[[CIVector alloc] initWithX: 7 Y: 8] autorelease];
        CHECK([initialised count] == 2 && [initialised valueAtIndex: 1] == 8);

        // an arbitrary number of values; the vector keeps a copy
        CGFloat values[5] = {10, 20, 30, 40, 50};
        CIVector *five = [CIVector vectorWithValues: values count: 5];
        values[0] = -1;
        CHECK([five count] == 5);
        for (size_t i = 0; i < 5; i++) {
            CHECK([five valueAtIndex: i] == (CGFloat)(10 * (i + 1)));
        }

        // a position that is not in the vector gives 0
        CHECK([five valueAtIndex: 5] == 0 && [five valueAtIndex: 1000] == 0);
        CHECK([one Y] == 0 && [one Z] == 0 && [one W] == 0);

        // CGPoint: x and y are the two values
        CIVector *point = [CIVector vectorWithCGPoint: CGPointMake(3, 4)];
        CHECK([point count] == 2 && [point X] == 3 && [point Y] == 4);
        CGPoint back = [point CGPointValue];
        CHECK(back.x == 3 && back.y == 4);
        CHECK([[CIVector vectorWithX: 9 Y: 8 Z: 7] CGPointValue].x == 9);

        // CGAffineTransform: a, b, c, d, tx, ty are the six values, and the first six are read back
        CGAffineTransform transform = CGAffineTransformMake(1, 2, 3, 4, 5, 6);
        CIVector *matrix = [CIVector vectorWithCGAffineTransform: transform];
        CHECK([matrix count] == 6);
        for (size_t i = 0; i < 6; i++) {
            CHECK([matrix valueAtIndex: i] == (CGFloat)(i + 1));
        }
        CGAffineTransform restored = [matrix CGAffineTransformValue];
        CHECK(restored.a == 1 && restored.b == 2 && restored.c == 3 && restored.d == 4 && restored.tx == 5 && restored.ty == 6);
        CGFloat eight[8] = {1, 0, 0, 1, 10, 20, 99, 99};
        CGAffineTransform fromEight = [[CIVector vectorWithValues: eight count: 8] CGAffineTransformValue];
        CHECK(fromEight.tx == 10 && fromEight.ty == 20);

        // CGRect: x, y, width, height (observed on macOS 26.7.3)
        CIVector *rect = [CIVector vectorWithCGRect: CGRectMake(1, 2, 3, 4)];
        CHECK([rect count] == 4 && [rect X] == 1 && [rect Y] == 2 && [rect Z] == 3 && [rect W] == 4);
        CGRect rectBack = [rect CGRectValue];
        CHECK(rectBack.origin.x == 1 && rectBack.origin.y == 2 && rectBack.size.width == 3 && rectBack.size.height == 4);
        CGRect shortRect = [[CIVector vectorWithX: 1 Y: 2] CGRectValue];
        CHECK(shortRect.origin.x == 1 && shortRect.origin.y == 2 && shortRect.size.width == 0 && shortRect.size.height == 0);

        // NSCopying
        CIVector *copy = [[three copy] autorelease];
        CHECK([copy count] == 3 && [copy X] == 1 && [copy Y] == 2 && [copy Z] == 3);

        // QUIRK section: behaviour the documentation does not state, observed on macOS 26.7.3 (see
        // the QUIRK comments in CIVector.m).

        // string representation: %g spelling, so a whole number has no decimal point (the
        // documentation's examples show "[1.0 0.5 0.3]")
        CHECK([[[CIVector vectorWithX: 1.0 Y: 0.5 Z: 0.3] stringRepresentation] isEqualToString: @"[1 0.5 0.3]"]);
        CHECK([[[CIVector vectorWithX: 10.0 Y: 23.0] stringRepresentation] isEqualToString: @"[10 23]"]);
        CGFloat awkward[5] = {0.1, 1234567.0, -1e-7, 1.0 / 3.0, 123456.0};
        CHECK([[[CIVector vectorWithValues: awkward count: 5] stringRepresentation] isEqualToString: @"[0.1 1.23457e+06 -1e-07 0.333333 123456]"]);
        CGFloat special[3] = {NAN, INFINITY, -0.0};
        CHECK([[[CIVector vectorWithValues: special count: 3] stringRepresentation] isEqualToString: @"[nan inf -0]"]);
        CHECK([[[CIVector vectorWithValues: awkward count: 0] stringRepresentation] isEqualToString: @"[]"]);

        // string reader: the documented form comes back
        CIVector *parsed = [CIVector vectorWithString: @"[1.0 0.5 0.3]"];
        CHECK(parsed != nil && [parsed count] == 3 && [parsed X] == 1.0 && [parsed Y] == 0.5 && [parsed Z] == (CGFloat)0.3);
        CIVector *roundTrip = [CIVector vectorWithString: [five stringRepresentation]];
        CHECK([roundTrip count] == 5 && [roundTrip valueAtIndex: 4] == 50);
        // the brackets are optional, only as the first character
        CHECK([[CIVector vectorWithString: @"1 2 3 4 5 6 7"] count] == 7);
        CHECK([[CIVector vectorWithString: @"  [1 2]"] count] == 0);
        CHECK([[CIVector vectorWithString: @"[ 1 2"] count] == 2);
        CHECK([[CIVector vectorWithString: @"[1\n2]"] count] == 2);
        // reading ends at the first thing that is not a number, without an error
        CHECK([[CIVector vectorWithString: @"[1.0 abc]"] count] == 1);
        CHECK([[CIVector vectorWithString: @"[1 2][3 4]"] count] == 2);
        CHECK([[CIVector vectorWithString: @"[1 2] extra"] count] == 2);
        CHECK([[CIVector vectorWithString: @"[1.0, 2.0]"] count] == 1);
        CHECK([[CIVector vectorWithString: @"[1;2]"] count] == 1);
        CHECK([[CIVector vectorWithString: @"[1.5.5]"] count] == 2 && [[CIVector vectorWithString: @"[1.5.5]"] valueAtIndex: 1] == 0.5);
        CHECK([[CIVector vectorWithString: @"[abc]"] count] == 0);
        CHECK([[CIVector vectorWithString: @"[]"] count] == 0);
        CHECK([[CIVector vectorWithString: nil] count] == 0);
        CIVector *signs = [CIVector vectorWithString: @"[-1e2 .5 -.5 +3]"];
        CHECK([signs count] == 4 && [signs X] == -100 && [signs Y] == 0.5 && [signs Z] == -0.5 && [signs W] == 3);
        CIVector *infinite = [CIVector vectorWithString: @"[inf -inf nan]"];
        CHECK([infinite count] == 3 && [infinite X] == INFINITY && [infinite Y] == -INFINITY && [infinite Z] != [infinite Z]);

        // equality: same count and the same bits
        CHECK([[CIVector vectorWithX: 1 Y: 2] isEqual: [CIVector vectorWithX: 1 Y: 2]]);
        CHECK([[CIVector vectorWithX: 1 Y: 2] hash] == [[CIVector vectorWithX: 1 Y: 2] hash]);
        CHECK(![[CIVector vectorWithX: 1 Y: 2] isEqual: [CIVector vectorWithX: 1 Y: 3]]);
        CHECK(![[CIVector vectorWithX: 1 Y: 2] isEqual: [CIVector vectorWithX: 1 Y: 2 Z: 0]]);
        CHECK(![[CIVector vectorWithX: 1] isEqual: @"[1]"]);
        CHECK(![[CIVector vectorWithX: 1] isEqual: nil]);
        CHECK(![[CIVector vectorWithX: 0.0] isEqual: [CIVector vectorWithX: -0.0]]);
        CGFloat notANumber[1] = {NAN};
        CIVector *nanVector = [CIVector vectorWithValues: notANumber count: 1];
        CHECK([nanVector isEqual: [CIVector vectorWithValues: notANumber count: 1]]);
    }

    if (failures) {
        printf("%d failure(s)\n", failures);
        return 1;
    }
    printf("CIVector tests passed\n");
    return 0;
}
