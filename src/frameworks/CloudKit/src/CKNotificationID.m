/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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

#import "CKInternal.h"
#import <CloudKit/CKNotificationID.h>

@implementation CKNotificationID

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    if ((self = [super init]))
        _UUID = [[coder decodeObjectOfClass:[NSString class] forKey:@"UUID"] copy];
    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_UUID forKey:@"UUID"];
}

- (id)copyWithZone:(NSZone *)zone
{
    // Immutable.
    return self;
}

- (BOOL)isEqual:(id)object
{
    if (self == object)
        return YES;
    if (![object isKindOfClass:[CKNotificationID class]])
        return NO;
    NSString *other = ((CKNotificationID *)object)->_UUID;
    return _UUID == other || [_UUID isEqual:other];
}

- (NSUInteger)hash
{
    return _UUID.hash;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@: %p; UUID=%@>", [self class], self, _UUID];
}

@end
