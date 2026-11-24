/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
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

// System classes that support secure coding on macOS (10.14.4 and 26.7.3 alike) still decode with
// a secure unarchiver that refuses classes which do not adopt NSSecureCoding, answer NO to
// +supportsSecureCoding, or have a -initWithCoder: below their +supportsSecureCoding. Each object
// is archived with secure coding and decoded with unarchivedObjectOfClass(es):fromData:error:.
// Build: -framework Foundation -framework AppKit (manual reference
// counting, so it builds for i386 too).

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#include <stdio.h>

static int failures;

static void roundTrip(const char *name, id object, NSSet *classes, Class expected)
{
    NSError *error = nil;
    NSData *data = [NSKeyedArchiver archivedDataWithRootObject:object requiringSecureCoding:YES error:&error];
    if (data == nil) {
        printf("FAIL %s: archiving: %s\n", name, [[error description] UTF8String]);
        failures++;
        return;
    }
    error = nil;
    id decoded = [NSKeyedUnarchiver unarchivedObjectOfClasses:classes fromData:data error:&error];
    if (decoded == nil || ![decoded isKindOfClass:expected]) {
        NSString *why = [[error userInfo] objectForKey:NSDebugDescriptionErrorKey] ?: [error description];
        printf("FAIL %s: decoded %s: %s\n", name, decoded ? [NSStringFromClass([decoded class]) UTF8String] : "nil",
               why ? [why UTF8String] : "no error");
        failures++;
    }
}

#define ONE(c) [NSSet setWithObject:[c class]]

// encodes a C array with -encodeArrayOfObjCType:count:at:
@interface IntTriple : NSObject <NSSecureCoding> { @public int values[3]; }
@end
@implementation IntTriple
+ (BOOL)supportsSecureCoding { return YES; }
- (void)encodeWithCoder:(NSCoder *)c { [c encodeArrayOfObjCType:@encode(int) count:3 at:values]; }
- (id)initWithCoder:(NSCoder *)c
{
    if ((self = [super init]) != nil)
        [c decodeArrayOfObjCType:@encode(int) count:3 at:values];
    return self;
}
@end

int main(void)
{
    @autoreleasepool {
        roundTrip("NSUnitLength", [NSUnitLength meters], ONE(NSUnitLength), [NSUnitLength class]);
        roundTrip("NSUnitTemperature", [NSUnitTemperature celsius], ONE(NSUnit), [NSUnitTemperature class]);
        NSHashTable *hash = [NSHashTable hashTableWithOptions:NSPointerFunctionsStrongMemory];
        [hash addObject:@"a"];
        roundTrip("NSHashTable", hash, ONE(NSHashTable), [NSHashTable class]);
        NSMapTable *map = [NSMapTable strongToStrongObjectsMapTable];
        [map setObject:@"v" forKey:@"k"];
        roundTrip("NSMapTable", map, ONE(NSMapTable), [NSMapTable class]);
        NSPointerArray *pointers = [NSPointerArray strongObjectsPointerArray];
        [pointers addPointer:@"p"];
        roundTrip("NSPointerArray", pointers, ONE(NSPointerArray), [NSPointerArray class]);
        roundTrip("NSCalendar", [NSCalendar calendarWithIdentifier:NSCalendarIdentifierGregorian], ONE(NSCalendar), [NSCalendar class]);
        roundTrip("NSOrderedSet", [NSOrderedSet orderedSetWithObjects:@"a", @"b", nil], ONE(NSOrderedSet), [NSOrderedSet class]);
        roundTrip("NSMeasurementFormatter", [[[NSMeasurementFormatter alloc] init] autorelease], ONE(NSMeasurementFormatter), [NSMeasurementFormatter class]);

        NSRange range = NSMakeRange(0, 3);
        roundTrip("link checking result", [NSTextCheckingResult linkCheckingResultWithRange:range URL:[NSURL URLWithString:@"http://example.com/"]], ONE(NSTextCheckingResult), [NSTextCheckingResult class]);
        roundTrip("spell checking result", [NSTextCheckingResult spellCheckingResultWithRange:range], ONE(NSTextCheckingResult), [NSTextCheckingResult class]);
        roundTrip("replacement checking result", [NSTextCheckingResult replacementCheckingResultWithRange:range replacementString:@"xyz"], ONE(NSTextCheckingResult), [NSTextCheckingResult class]);
        roundTrip("date checking result", [NSTextCheckingResult dateCheckingResultWithRange:range date:[NSDate dateWithTimeIntervalSince1970:0]], ONE(NSTextCheckingResult), [NSTextCheckingResult class]);
        roundTrip("phone number checking result", [NSTextCheckingResult phoneNumberCheckingResultWithRange:range phoneNumber:@"555"], ONE(NSTextCheckingResult), [NSTextCheckingResult class]);

        NSHTTPURLResponse *response = [[[NSHTTPURLResponse alloc] initWithURL:[NSURL URLWithString:@"http://example.com/"] statusCode:200 HTTPVersion:@"HTTP/1.1" headerFields:[NSDictionary dictionaryWithObject:@"text/plain" forKey:@"Content-Type"]] autorelease];
        roundTrip("NSHTTPURLResponse", response, ONE(NSHTTPURLResponse), [NSHTTPURLResponse class]);
        roundTrip("NSURLCredential", [NSURLCredential credentialWithUser:@"user" password:@"secret" persistence:NSURLCredentialPersistenceNone], ONE(NSURLCredential), [NSURLCredential class]);

        NSBitmapImageRep *rep = [[[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL pixelsWide:2 pixelsHigh:2 bitsPerSample:8 samplesPerPixel:4 hasAlpha:YES isPlanar:NO colorSpaceName:NSDeviceRGBColorSpace bytesPerRow:0 bitsPerPixel:0] autorelease];
        NSImage *image = [[[NSImage alloc] initWithSize:NSMakeSize(2, 2)] autorelease];
        [image addRepresentation:rep];
        roundTrip("NSImage", image, ONE(NSImage), [NSImage class]);


        IntTriple *triple = [[[IntTriple alloc] init] autorelease];
        triple->values[0] = 1; triple->values[1] = 2; triple->values[2] = 3;
        NSData *data = [NSKeyedArchiver archivedDataWithRootObject:triple requiringSecureCoding:YES error:NULL];
        IntTriple *decoded = data ? [NSKeyedUnarchiver unarchivedObjectOfClass:[IntTriple class] fromData:data error:NULL] : nil;
        if (decoded == nil || decoded->values[0] != 1 || decoded->values[1] != 2 || decoded->values[2] != 3) {
            printf("FAIL decodeArrayOfObjCType: %s\n", data ? "decoded wrong" : "not archived");
            failures++;
        }
    }
    if (failures) {
        printf("%d failure(s)\n", failures);
        return 1;
    }
    printf("PASS\n");
    return 0;
}
