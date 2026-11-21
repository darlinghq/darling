/*
 This file is part of Darling.

 Copyright (C) 2019-2026 Darling Developers

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

#import <QTKit/QTTimeValue.h>
#import <darling-stub-forwarding.h>

@implementation QTTimeValue

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

@end

// NSValue's type string for these structs is the one of an unnamed struct ("{?=qll}"), as on macOS.
// QUIRK: the real QTKit makes its values from unnamed structs; the type is observable through -objCType
// and -isEqual:. Black-box observation of macOS 10.14.4 (tier 2). Test: tests/qtkit-time.m "NSValue".
typedef struct { long long timeValue; long timeScale; long flags; } QTUnnamedTime;
typedef struct { QTUnnamedTime time; QTUnnamedTime duration; } QTUnnamedTimeRange;

@implementation NSValue (NSValueQTTimeExtensions)

+ (NSValue *)valueWithQTTime:(QTTime)time
{
    return [NSValue valueWithBytes:&time objCType:@encode(QTUnnamedTime)];
}

- (QTTime)QTTimeValue
{
    QTTime time;
    [self getValue:&time];
    return time;
}

+ (NSValue *)valueWithQTTimeRange:(QTTimeRange)range
{
    return [NSValue valueWithBytes:&range objCType:@encode(QTUnnamedTimeRange)];
}

- (QTTimeRange)QTTimeRangeValue
{
    QTTimeRange range;
    [self getValue:&range];
    return range;
}

@end

@implementation NSCoder (NSCoderQTTimeExtensions)

- (void)encodeQTTime:(QTTime)time forKey:(NSString *)key
{
    [self encodeObject:[NSValue valueWithQTTime:time] forKey:key];
}

- (QTTime)decodeQTTimeForKey:(NSString *)key
{
    NSValue *val = [self decodeObjectForKey:key];
    if ([val respondsToSelector:@selector(QTTimeValue)]) {
        return [val QTTimeValue];
    }
    return QTZeroTime;
}

- (void)encodeQTTimeRange:(QTTimeRange)range forKey:(NSString *)key
{
    [self encodeObject:[NSValue valueWithQTTimeRange:range] forKey:key];
}

- (QTTimeRange)decodeQTTimeRangeForKey:(NSString *)key
{
    NSValue *val = [self decodeObjectForKey:key];
    if ([val respondsToSelector:@selector(QTTimeRangeValue)]) {
        return [val QTTimeRangeValue];
    }
    return QTMakeTimeRange(QTZeroTime, QTZeroTime);
}

@end
