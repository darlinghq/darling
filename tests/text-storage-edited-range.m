// Copyright (C) 2026 Lubos Dolezel
//
// Edits made between -beginEditing and -endEditing are reported as one edited
// range in the string as it is after all of them: a later edit moves and
// resizes the range of the earlier ones. The range and the change in length
// are seen by NSTextStorageWillProcessEditingNotification observers and by
// -fixAttributesInRange: of a subclass. Results as on macOS 26.7.3 and 10.14.4.
// No display needed. MRC.
#import <AppKit/AppKit.h>
#include <stdio.h>

static int failures;

@interface WrappingStorage : NSTextStorage {
    NSMutableAttributedString *_contents;
@public
    NSRange _fixedRange;
    NSUInteger _fixedLength;
    int _fixes;
}
@end

@implementation WrappingStorage
- (instancetype) initWithString: (NSString *) string {
    if ((self = [super init]))
        _contents = [[NSMutableAttributedString alloc] initWithString: string];
    return self;
}
- (void) dealloc {
    [_contents release];
    [super dealloc];
}
- (NSString *) string {
    return [_contents string];
}
- (NSDictionary *) attributesAtIndex: (NSUInteger) index effectiveRange: (NSRangePointer) range {
    return [_contents attributesAtIndex: index effectiveRange: range];
}
- (void) replaceCharactersInRange: (NSRange) range withString: (NSString *) string {
    NSUInteger before = [_contents length];
    [_contents replaceCharactersInRange: range withString: string];
    [self edited: NSTextStorageEditedCharacters range: range changeInLength: (NSInteger) [_contents length] - (NSInteger) before];
}
- (void) setAttributes: (NSDictionary *) attributes range: (NSRange) range {
    [_contents setAttributes: attributes range: range];
    [self edited: NSTextStorageEditedAttributes range: range changeInLength: 0];
}
- (void) fixAttributesInRange: (NSRange) range {
    _fixedRange = range;
    _fixedLength = [self length];
    _fixes++;
    // Only the range is checked; not calling super keeps the attributes as
    // they are (10.14 raises when fixing an empty range of an empty string).
}
@end

static NSRange notifiedRange;
static NSInteger notifiedDelta;

@interface Observer : NSObject
@end

@implementation Observer
- (void) willProcess: (NSNotification *) note {
    NSTextStorage *storage = [note object];
    notifiedRange = [storage editedRange];
    notifiedDelta = [storage changeInLength];
}
@end

typedef void (^Edits)(NSTextStorage *storage);

static void run(const char *label, NSTextStorage *storage, Edits edits, NSRange expected, NSInteger delta) {
    notifiedRange = NSMakeRange(12345, 12345);
    notifiedDelta = 12345;
    [storage beginEditing];
    edits(storage);
    NSRange range = [storage editedRange];
    NSInteger change = [storage changeInLength];
    [storage endEditing];

    BOOL ok = NSEqualRanges(range, expected) && change == delta &&
              NSEqualRanges(notifiedRange, expected) && notifiedDelta == delta;
    printf("%s %s: editedRange %s change %ld, when processed %s change %ld; expected %s change %ld\n",
           ok ? "PASS" : "FAIL", label,
           [NSStringFromRange(range) UTF8String], (long) change,
           [NSStringFromRange(notifiedRange) UTF8String], (long) notifiedDelta,
           [NSStringFromRange(expected) UTF8String], (long) delta);
    failures += !ok;
}

static void runBoth(const char *label, Edits edits, NSRange expected, NSInteger delta) {
    NSTextStorage *plain = [[NSTextStorage alloc] initWithString: @"Hello world"];
    run(label, plain, edits, expected, delta);
    [plain release];

    WrappingStorage *subclass = [[WrappingStorage alloc] initWithString: @"Hello world"];
    char subLabel[200];
    snprintf(subLabel, sizeof subLabel, "%s (subclass)", label);
    run(subLabel, subclass, edits, expected, delta);
    BOOL fixedOK = subclass->_fixes == 1 && NSEqualRanges(subclass->_fixedRange, expected) &&
                   NSMaxRange(subclass->_fixedRange) <= subclass->_fixedLength;
    printf("%s %s: attributes fixed %d time(s) in %s of a string of %lu\n", fixedOK ? "PASS" : "FAIL", subLabel,
           subclass->_fixes, [NSStringFromRange(subclass->_fixedRange) UTF8String], (unsigned long) subclass->_fixedLength);
    failures += !fixedOK;
    [subclass release];
}

int main(void) {
    @autoreleasepool {
        Observer *observer = [[Observer alloc] init];
        [[NSNotificationCenter defaultCenter] addObserver: observer selector: @selector(willProcess:)
                                                     name: NSTextStorageWillProcessEditingNotification object: nil];

        runBoth("delete the end, then the start", ^(NSTextStorage *s) {
            [s replaceCharactersInRange: NSMakeRange(6, 5) withString: @""];
            [s replaceCharactersInRange: NSMakeRange(0, 6) withString: @""];
        }, NSMakeRange(0, 0), -11);
        runBoth("insert at the start, then later", ^(NSTextStorage *s) {
            [s replaceCharactersInRange: NSMakeRange(0, 0) withString: @"abc"];
            [s replaceCharactersInRange: NSMakeRange(10, 0) withString: @"x"];
        }, NSMakeRange(0, 11), 4);
        runBoth("replace in the middle, then delete the first character", ^(NSTextStorage *s) {
            [s replaceCharactersInRange: NSMakeRange(2, 3) withString: @"XYZW"];
            [s replaceCharactersInRange: NSMakeRange(0, 1) withString: @""];
        }, NSMakeRange(0, 5), 0);
        runBoth("delete late, then insert early", ^(NSTextStorage *s) {
            [s replaceCharactersInRange: NSMakeRange(8, 1) withString: @""];
            [s replaceCharactersInRange: NSMakeRange(0, 0) withString: @"12"];
        }, NSMakeRange(0, 10), 1);
        runBoth("insert early, then delete late", ^(NSTextStorage *s) {
            [s replaceCharactersInRange: NSMakeRange(0, 0) withString: @"12"];
            [s replaceCharactersInRange: NSMakeRange(10, 1) withString: @""];
        }, NSMakeRange(0, 10), 1);
        runBoth("attributes in two places", ^(NSTextStorage *s) {
            [s addAttribute: @"A" value: @1 range: NSMakeRange(1, 2)];
            [s addAttribute: @"A" value: @1 range: NSMakeRange(5, 1)];
        }, NSMakeRange(1, 5), 0);
        runBoth("delete inside an earlier insertion", ^(NSTextStorage *s) {
            [s replaceCharactersInRange: NSMakeRange(3, 0) withString: @"abcdef"];
            [s replaceCharactersInRange: NSMakeRange(4, 2) withString: @""];
        }, NSMakeRange(3, 4), 4);
        runBoth("replace around an earlier insertion", ^(NSTextStorage *s) {
            [s replaceCharactersInRange: NSMakeRange(3, 0) withString: @"ab"];
            [s replaceCharactersInRange: NSMakeRange(1, 8) withString: @"Q"];
        }, NSMakeRange(1, 1), -5);

        [[NSNotificationCenter defaultCenter] removeObserver: observer];
        [observer release];
    }
    printf("failures=%d\n", failures);
    return failures ? 1 : 0;
}
