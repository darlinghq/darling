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

// Opaque to applications; the contents are whatever the server handed out.
@implementation CKServerChangeToken

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (instancetype)_initWithData:(NSData *)data
{
    if ((self = [super init]))
        _data = [data copy];
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    NSData *data = [coder decodeObjectOfClass:[NSData class] forKey:@"Data"];
    if (data == nil) {
        return nil;
    }
    return [self _initWithData:data];
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_data forKey:@"Data"];
}

- (id)copyWithZone:(NSZone *)zone
{
    // Immutable.
    return self;
}

- (NSData *)_data
{
    return _data;
}

- (BOOL)isEqual:(id)object
{
    if (self == object)
        return YES;
    if (![object isKindOfClass:[CKServerChangeToken class]])
        return NO;
    return [_data isEqualToData:(((CKServerChangeToken *)object))->_data];
}

- (NSUInteger)hash
{
    return _data.hash;
}

@end
