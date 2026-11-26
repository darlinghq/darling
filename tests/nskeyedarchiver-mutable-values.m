// Guest test: how NSKeyedArchiver writes strings and data. Observed on
// macOS 26.7.3 (x86_64): an immutable string or data object is stored as a
// plain property list value, while a mutable one is stored as an object of
// class NSMutableString / NSMutableData holding NS.string / NS.data, so it
// decodes mutable and is checked as that class by secure decoding.
#import <Foundation/Foundation.h>
#include <stdio.h>

static int failures;

static void check(const char *label, BOOL ok)
{
    printf("%s: %s\n", ok ? "PASS" : "FAIL", label);
    if (!ok)
        failures++;
}

// The archived $objects entry for the root object and, if it is an object, its class name.
static id archivedRoot(id root, NSString **className)
{
    NSData *data = [NSKeyedArchiver archivedDataWithRootObject: root requiringSecureCoding: YES error: NULL];
    NSDictionary *plist = [NSPropertyListSerialization propertyListWithData: data options: 0 format: NULL error: NULL];
    NSArray *objects = plist[@"$objects"];
    *className = nil;
    id entry = objects.count > 1 ? objects[1] : nil;
    if ([entry isKindOfClass: [NSDictionary class]]) {
        id classRef = entry[@"$class"];
        // The class reference is a CFKeyedArchiverUID; its index is in its description.
        NSString *text = [classRef description];
        NSRange equals = [text rangeOfString: @"value = "];
        if (equals.location != NSNotFound) {
            NSInteger index = [[text substringFromIndex: NSMaxRange(equals)] integerValue];
            *className = objects[index][@"$classname"];
        }
    }
    return entry;
}

int main(void)
{
    @autoreleasepool {
        NSString *className;
        id entry = archivedRoot(@"immutable", &className);
        check("an immutable string is stored as a plain string", [entry isEqual: @"immutable"]);

        entry = archivedRoot([[@"mutable" mutableCopy] autorelease], &className);
        check("a mutable string is stored as an NSMutableString object",
              [entry isKindOfClass: [NSDictionary class]] && [className isEqualToString: @"NSMutableString"]
                  && [entry[@"NS.string"] isEqual: @"mutable"]);

        entry = archivedRoot([NSData dataWithBytes: "ab" length: 2], &className);
        check("an immutable data object is stored as plain data", [entry isEqual: [NSData dataWithBytes: "ab" length: 2]]);

        entry = archivedRoot([NSMutableData dataWithBytes: "ab" length: 2], &className);
        check("a mutable data object is stored as an NSMutableData object",
              [entry isKindOfClass: [NSDictionary class]] && [className isEqualToString: @"NSMutableData"]
                  && [entry[@"NS.data"] isEqual: [NSData dataWithBytes: "ab" length: 2]]);

        NSMutableString *shared = [[@"z" mutableCopy] autorelease];
        NSData *data = [NSKeyedArchiver archivedDataWithRootObject: @[ shared, shared ] requiringSecureCoding: YES error: NULL];
        NSArray *decoded = [NSKeyedUnarchiver unarchivedObjectOfClasses: [NSSet setWithObjects: [NSArray class], [NSMutableString class], nil]
                                                               fromData: data
                                                                  error: NULL];
        check("a mutable string referenced twice decodes as one object", decoded.count == 2 && decoded[0] == decoded[1]);
        BOOL mutable = NO;
        @try {
            [decoded[0] appendString: @"x"];
            mutable = [decoded[0] isEqual: @"zx"];
        } @catch (NSException *exception) {
        }
        check("a mutable string decodes mutable", mutable);

        data = [NSKeyedArchiver archivedDataWithRootObject: [NSMutableData dataWithBytes: "ab" length: 2] requiringSecureCoding: YES error: NULL];
        NSMutableData *decodedData = [NSKeyedUnarchiver unarchivedObjectOfClass: [NSMutableData class] fromData: data error: NULL];
        BOOL dataMutable = NO;
        @try {
            [decodedData appendBytes: "c" length: 1];
            dataMutable = [decodedData isEqual: [NSData dataWithBytes: "abc" length: 3]];
        } @catch (NSException *exception) {
        }
        check("a mutable data object decodes mutable", dataMutable);
    }
    return failures != 0;
}
