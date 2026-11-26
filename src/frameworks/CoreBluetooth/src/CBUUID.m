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
- (instancetype)initWithCanonicalString:(NSString *)canonical;
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
    NSData *_UUIDData;
}

+ (CBUUID *)UUIDWithString:(NSString *)theString
{
    NSString *canonical = CanonicalUUIDString(theString);
    return canonical != nil ? [[[self alloc] initWithCanonicalString:canonical] autorelease] : nil;
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
    return canonical != nil ? [[[self alloc] initWithCanonicalString:canonical] autorelease] : nil;
}

+ (CBUUID *)UUIDWithNSUUID:(NSUUID *)theNSUUID
{
    return [self UUIDWithString:[theNSUUID UUIDString]];
}

- (instancetype)initWithCanonicalString:(NSString *)canonical
{
    self = [super init];
    if (self == nil)
        return nil;
    _UUIDString = [canonical copy];
    _UUIDData = [DataFromCanonicalString(canonical) copy];
    return self;
}

- (void)dealloc
{
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
    return [_UUIDString hash];
}

- (BOOL)isEqual:(id)object
{
    if (object == self)
        return YES;
    if (![object isKindOfClass:[CBUUID class]])
        return NO;
    return [_UUIDString isEqualToString:[(CBUUID *) object UUIDString]];
}

- (id)copyWithZone:(NSZone *)zone
{
    (void) zone;
    return [self retain];
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@: %@>", NSStringFromClass([self class]), _UUIDString];
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
