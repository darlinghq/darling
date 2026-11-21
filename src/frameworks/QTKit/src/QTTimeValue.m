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

@implementation NSValue (NSValueQTTimeExtensions)

+ (NSValue *)valueWithQTTime:(QTTime)time
{
    return [NSValue valueWithBytes:&time objCType:@encode(QTTime)];
}

- (QTTime)QTTimeValue
{
    QTTime time;
    [self getValue:&time];
    return time;
}

+ (NSValue *)valueWithQTTimeRange:(QTTimeRange)range
{
    return [NSValue valueWithBytes:&range objCType:@encode(QTTimeRange)];
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
    return QTZeroTimeRange;
}

@end
