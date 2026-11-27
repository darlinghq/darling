/*
  This file is part of Darling.

  Copyright (C) 2019 Lubos Dolezel
  Copyright (C) 2026 Darling contributors

  Darling is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Darling is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#import <CoreBluetooth/CBUUID.h>

@interface CBUUID ()
- (instancetype)initWithCanonicalString:(NSString *)canonical display:(NSString *)display;
@end

static int HexValue(unichar character)
{
    if (character >= '0' && character <= '9')
        return character - '0';
    if (character >= 'a' && character <= 'f')
        return character - 'a' + 10;
    if (character >= 'A' && character <= 'F')
        return character - 'A' + 10;
    return -1;
}

static NSString *CanonicalUUIDString(NSString *input)
{
    if (input == nil)
        return nil;

    NSMutableString *hex = [NSMutableString stringWithCapacity:32];
    for (NSUInteger index = 0; index < [input length]; ++index)
    {
        unichar character = [input characterAtIndex:index];
        if (character == '-')
            continue;
        if (HexValue(character) < 0)
            return nil;
        [hex appendFormat:@"%X", HexValue(character)];
    }

    NSUInteger length = [hex length];
    if (length == 4 || length == 8)
    {
        while ([hex length] < 8)
            [hex insertString:@"0" atIndex:0];
        return [NSString stringWithFormat:@"%@-0000-1000-8000-00805F9B34FB", hex];
    }
    if (length != 32)
        return nil;

    return [NSString stringWithFormat:@"%@-%@-%@-%@-%@",
                     [hex substringWithRange:NSMakeRange(0, 8)],
                     [hex substringWithRange:NSMakeRange(8, 4)],
                     [hex substringWithRange:NSMakeRange(12, 4)],
                     [hex substringWithRange:NSMakeRange(16, 4)],
                     [hex substringWithRange:NSMakeRange(20, 12)]];
}

// QUIRK: a UUID keeps the width it was created with. UUIDString and data of a 16-bit or 32-bit
// UUID are the 4 or 8 hexadecimal digits (2 or 4 bytes), a 128-bit UUID keeps all of its digits
// even when it is derived from the Bluetooth base UUID, and UUIDs of different widths that expand
// to the same 128-bit value are equal and have the same hash. The public header does not say
// this. Evidence (tier 2): black-box observation on macOS 26.7.3 of UUIDWithString: with
// "180F", "180f", "0000180f", "12345678" and the 128-bit forms of the base UUID, UUIDWithData:
// with two bytes, and isEqual:/hash between them.
// Test: tests/src/corebluetooth_central.m, mode "uuid" (the checks marked QUIRK).
static NSString *DisplayUUIDString(NSString *input)
{
    NSString *canonical = CanonicalUUIDString(input);
    if (canonical == nil)
        return nil;
    NSMutableString *hex = [NSMutableString stringWithCapacity:32];
    for (NSUInteger index = 0; index < [input length]; ++index)
    {
        if ([input characterAtIndex:index] != '-')
            [hex appendFormat:@"%X", HexValue([input characterAtIndex:index])];
    }
    // 4 digits stay 4 and 8 stay 8; anything else was expanded to the 128-bit form
    return ([hex length] == 4 || [hex length] == 8) ? hex : canonical;
}

static NSData *DataFromCanonicalString(NSString *canonical)
{
    NSMutableData *data = [NSMutableData dataWithCapacity:16];
    unsigned char byte = 0;
    BOOL haveHighNibble = NO;
    for (NSUInteger index = 0; index < [canonical length]; ++index)
    {
        unichar character = [canonical characterAtIndex:index];
        if (character == '-')
            continue;
        const int value = HexValue(character);
        if (value < 0)
            return nil;
        if (haveHighNibble)
        {
            byte = (unsigned char)((byte << 4) | value);
            [data appendBytes:&byte length:1];
            haveHighNibble = NO;
        }
        else
        {
            byte = (unsigned char) value;
            haveHighNibble = YES;
        }
    }
    return haveHighNibble ? nil : data;
}

#if __LP64__

@implementation CBUUID
{
    NSString *_UUIDString;
    NSString *_canonicalString;
    NSData *_UUIDData;
}

+ (CBUUID *)UUIDWithString:(NSString *)theString
{
    NSString *canonical = CanonicalUUIDString(theString);
    return canonical != nil
        ? [[[self alloc] initWithCanonicalString:canonical display:DisplayUUIDString(theString)] autorelease]
        : nil;
}

+ (CBUUID *)UUIDWithData:(NSData *)theData
{
    if (theData == nil)
        return nil;
    const NSUInteger length = [theData length];
    if (length != 2 && length != 4 && length != 16)
        return nil;

    const unsigned char *bytes = [theData bytes];
    NSMutableString *hex = [NSMutableString stringWithCapacity:length * 2];
    for (NSUInteger index = 0; index < length; ++index)
        [hex appendFormat:@"%02X", bytes[index]];

    NSString *canonical = CanonicalUUIDString(hex);
    return canonical != nil
        ? [[[self alloc] initWithCanonicalString:canonical display:DisplayUUIDString(hex)] autorelease]
        : nil;
}

+ (CBUUID *)UUIDWithNSUUID:(NSUUID *)theNSUUID
{
    return [self UUIDWithString:[theNSUUID UUIDString]];
}

- (instancetype)initWithCanonicalString:(NSString *)canonical display:(NSString *)display
{
    self = [super init];
    if (self == nil)
        return nil;
    _canonicalString = [canonical copy];
    _UUIDString = [display copy];
    _UUIDData = [DataFromCanonicalString(display) copy];
    return self;
}

- (void)dealloc
{
    [_canonicalString release];
    [_UUIDString release];
    [_UUIDData release];
    [super dealloc];
}

- (NSString *)UUIDString
{
    return _UUIDString;
}

- (NSData *)data
{
    return _UUIDData;
}

- (NSUInteger)hash
{
    return [_canonicalString hash];
}

- (BOOL)isEqual:(id)object
{
    if (object == self)
        return YES;
    if (![object isKindOfClass:[CBUUID class]])
        return NO;
    return [_canonicalString isEqualToString:((CBUUID *) object)->_canonicalString];
}

- (id)copyWithZone:(NSZone *)zone
{
    (void) zone;
    return [self retain];
}

- (NSString *)description
{
    return _UUIDString;
}

@end

#else // __LP64__

#import <darling-stub-forwarding.h>

#pragma clang diagnostic ignored "-Wobjc-property-implementation"
#pragma clang diagnostic ignored "-Wincomplete-implementation"
#pragma clang diagnostic ignored "-Wprotocol"

@implementation CBUUID

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

- (id)copyWithZone:(NSZone *)zone
{
    (void) zone;
    return [self retain];
}

@end

#endif // __LP64__
