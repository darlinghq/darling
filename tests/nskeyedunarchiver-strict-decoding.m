// Guest test: NSKeyedUnarchiver's strict secure decoding mode,
// -_enableStrictSecureDecodingMode and
// +_strictlyUnarchivedObjectOfClasses:fromData:error:, which WebKit uses to
// decode its IPC messages. Expected values were observed on macOS 26.7.3
// (x86_64): strictly, every archived object must be of one of the allowed
// classes exactly (no subclasses), property list classes are not implied, and
// failures are NSCoderReadCorruptError. The ordinary API is checked alongside.
#import <Foundation/Foundation.h>
#include <stdio.h>

@interface NSKeyedUnarchiver (StrictSPI)
- (void)_enableStrictSecureDecodingMode;
+ (id)_strictlyUnarchivedObjectOfClasses:(NSSet *)classes fromData:(NSData *)data error:(NSError **)error;
@end

@interface TestBase : NSObject <NSSecureCoding>
@end
@implementation TestBase
+ (BOOL)supportsSecureCoding { return YES; }
- (void)encodeWithCoder:(NSCoder *)coder { }
- (id)initWithCoder:(NSCoder *)coder { return [super init]; }
@end
@interface TestSub : TestBase
@end
@implementation TestSub
@end

static int failures;

static void check(const char *label, BOOL ok)
{
    printf("%s: %s\n", ok ? "PASS" : "FAIL", label);
    if (!ok)
        failures++;
}

// strictOK and normalOK say whether each API should give back an object.
static void expect(const char *label, id root, NSSet *classes, BOOL strictOK, BOOL normalOK)
{
    NSData *data = [NSKeyedArchiver archivedDataWithRootObject: root requiringSecureCoding: YES error: NULL];
    NSError *error = nil;
    id result = [NSKeyedUnarchiver _strictlyUnarchivedObjectOfClasses: classes fromData: data error: &error];
    BOOL ok = strictOK ? (result != nil && error == nil)
                       : (result == nil && [error.domain isEqualToString: NSCocoaErrorDomain] && error.code == NSCoderReadCorruptError);
    char text[160];
    snprintf(text, sizeof text, "strict: %s %s", label, strictOK ? "decodes" : "fails with 4864");
    check(text, ok);

    error = nil;
    result = [NSKeyedUnarchiver unarchivedObjectOfClasses: classes fromData: data error: &error];
    snprintf(text, sizeof text, "normal: %s %s", label, normalOK ? "decodes" : "fails");
    check(text, normalOK ? result != nil : result == nil);
}

int main(void)
{
    @autoreleasepool {
        expect("TestSub with {TestBase}", [TestSub new], [NSSet setWithObject: [TestBase class]], NO, YES);
        expect("TestSub with {TestSub}", [TestSub new], [NSSet setWithObject: [TestSub class]], YES, YES);
        expect("TestBase with {TestSub}", [TestBase new], [NSSet setWithObject: [TestSub class]], NO, NO);
        expect("NSMutableArray with {NSArray,NSString}", [@[ @"a" ] mutableCopy], [NSSet setWithObjects: [NSArray class], [NSString class], nil], NO, YES);
        expect("NSMutableArray with {NSMutableArray,NSString}", [@[ @"a" ] mutableCopy], [NSSet setWithObjects: [NSMutableArray class], [NSString class], nil], YES, YES);
        expect("NSArray with {NSMutableArray,NSString}", @[ @"a" ], [NSSet setWithObjects: [NSMutableArray class], [NSString class], nil], NO, YES);
        expect("NSArray of a mutable string with {NSArray,NSString}", @[ [@"a" mutableCopy] ], [NSSet setWithObjects: [NSArray class], [NSString class], nil], NO, YES);
        expect("NSString with {NSString}", @"x", [NSSet setWithObject: [NSString class]], YES, YES);
        expect("NSString with {NSNumber}", @"x", [NSSet setWithObject: [NSNumber class]], NO, YES);
        expect("NSString with {NSObject}", @"x", [NSSet setWithObject: [NSObject class]], NO, YES);
        expect("TestSub with {NSObject}", [TestSub new], [NSSet setWithObject: [NSObject class]], NO, YES);
        expect("NSNumber with {NSNumber}", @1, [NSSet setWithObject: [NSNumber class]], YES, YES);
        expect("NSArray of NSNumber with {NSArray}", @[ @1 ], [NSSet setWithObject: [NSArray class]], NO, YES);
        expect("NSData with {NSData}", [NSData dataWithBytes: "a" length: 1], [NSSet setWithObject: [NSData class]], YES, YES);
        expect("NSDate with {NSDate}", [NSDate dateWithTimeIntervalSinceReferenceDate: 1000], [NSSet setWithObject: [NSDate class]], YES, YES);
        expect("NSURL with {NSURL}", [NSURL URLWithString: @"http://example.com/"], [NSSet setWithObject: [NSURL class]], YES, YES);
        expect("NSDictionary of strings with {NSDictionary}", @{ @"k" : @"v" }, [NSSet setWithObject: [NSDictionary class]], NO, YES);
        expect("nested containers with every class",
               @{ @"a" : @[ @1, @"s" ] },
               [NSSet setWithObjects: [NSDictionary class], [NSArray class], [NSString class], [NSNumber class], nil], YES, YES);
        expect("nested containers without NSNumber",
               @{ @"a" : @[ @1, @"s" ] },
               [NSSet setWithObjects: [NSDictionary class], [NSArray class], [NSString class], nil], NO, YES);

        NSError *error = nil;
        id result = [NSKeyedUnarchiver _strictlyUnarchivedObjectOfClasses: [NSSet setWithObject: [NSString class]] fromData: nil error: &error];
        check("strict: nil data fails with 4864", result == nil && error.code == NSCoderReadCorruptError);
        error = nil;
        result = [NSKeyedUnarchiver _strictlyUnarchivedObjectOfClasses: [NSSet setWithObject: [NSString class]] fromData: [NSData dataWithBytes: "junk" length: 4] error: &error];
        check("strict: data that is not an archive fails with 4864", result == nil && error.code == NSCoderReadCorruptError);
        error = nil;
        NSData *string = [NSKeyedArchiver archivedDataWithRootObject: @"x" requiringSecureCoding: YES error: NULL];
        result = [NSKeyedUnarchiver _strictlyUnarchivedObjectOfClasses: [NSSet set] fromData: string error: &error];
        check("strict: an empty set of classes fails with 4864", result == nil && error.code == NSCoderReadCorruptError);

        error = nil;
        result = [NSKeyedUnarchiver unarchivedObjectOfClasses: [NSSet set] fromData: string error: &error];
        check("normal: an empty set of classes still allows property list classes", [result isEqual: @"x"] && error == nil);

        NSMutableDictionary *mutable = [NSMutableDictionary dictionaryWithObject: [@"v" mutableCopy] forKey: @"k"];
        NSData *mutableData = [NSKeyedArchiver archivedDataWithRootObject: mutable requiringSecureCoding: YES error: NULL];
        NSKeyedUnarchiver *unarchiver = [[NSKeyedUnarchiver alloc] initForReadingFromData: mutableData error: NULL];
        [unarchiver setRequiresSecureCoding: NO];
        [unarchiver _enableStrictSecureDecodingMode];
        check("enabling strict mode turns on requiresSecureCoding", [unarchiver requiresSecureCoding]);
        result = [unarchiver decodeObjectOfClasses: [NSSet setWithObjects: [NSDictionary class], [NSString class], nil] forKey: NSKeyedArchiveRootObjectKey];
        check("instance in strict mode refuses a mutable dictionary when only NSDictionary is allowed",
              result == nil && unarchiver.error.code == NSCoderReadCorruptError);
        [unarchiver release];

        unarchiver = [[NSKeyedUnarchiver alloc] initForReadingFromData: mutableData error: NULL];
        result = [unarchiver decodeObjectOfClasses: [NSSet setWithObjects: [NSDictionary class], [NSString class], nil] forKey: NSKeyedArchiveRootObjectKey];
        check("instance not in strict mode accepts it", result != nil && unarchiver.error == nil);
        [unarchiver release];

        unarchiver = [[NSKeyedUnarchiver alloc] initForReadingFromData: string error: NULL];
        [unarchiver _enableStrictSecureDecodingMode];
        unarchiver.decodingFailurePolicy = NSDecodingFailurePolicyRaiseException;
        BOOL raised = NO;
        @try {
            [unarchiver decodeObjectOfClasses: [NSSet setWithObject: [NSNumber class]] forKey: NSKeyedArchiveRootObjectKey];
        } @catch (NSException *exception) {
            raised = [exception.name isEqualToString: NSInvalidUnarchiveOperationException];
        }
        check("strict mode with the raise policy raises NSInvalidUnarchiveOperationException", raised);
        [unarchiver release];
    }
    return failures != 0;
}
