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

NSString * const CKRecordZoneDefaultName = @"_defaultZone";
NSString * const CKCurrentUserDefaultName = @"__defaultOwner__";
NSString * const CKOwnerDefaultName = @"__defaultOwner__";

void CKValidateName(NSString *name, NSString *what)
{
    if (name == nil)
        CKRaiseInvalid(@"CKException", [NSString stringWithFormat:@"%@ can not be nil", what]);
    if (name.length == 0)
        CKRaiseInvalid(@"CKException", [NSString stringWithFormat:@"%@ can not be empty", what]);
    if (name.length > CKMaxNameLength)
        CKRaiseInvalid(@"CKException", [NSString stringWithFormat:@"%@ (%@) is too long", what, name]);
}

@implementation CKRecordZoneID

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (instancetype)initWithZoneName:(NSString *)zoneName ownerName:(NSString *)ownerName
{
    CKValidateName(zoneName, @"zoneName");
    if (ownerName == nil)
        CKRaiseInvalid(@"CKException", @"ownerName can not be nil");
    if ((self = [super init])) {
        _zoneName = [zoneName copy];
        _ownerName = [ownerName copy];
    }
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    NSString *zoneName = [coder decodeObjectOfClass:[NSString class] forKey:@"ZoneName"];
    NSString *ownerName = [coder decodeObjectOfClass:[NSString class] forKey:@"OwnerName"];
    if (zoneName.length == 0 || ownerName == nil) {
        return nil;
    }
    return [self initWithZoneName:zoneName ownerName:ownerName];
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_zoneName forKey:@"ZoneName"];
    [coder encodeObject:_ownerName forKey:@"OwnerName"];
}

- (id)copyWithZone:(NSZone *)zone
{
    // Immutable.
    return self;
}

- (NSString *)zoneName
{
    return _zoneName;
}

- (NSString *)ownerName
{
    return _ownerName;
}

- (BOOL)isEqual:(id)object
{
    if (self == object)
        return YES;
    if (![object isKindOfClass:[CKRecordZoneID class]])
        return NO;
    CKRecordZoneID *other = object;
    return [_zoneName isEqualToString:other->_zoneName] && [_ownerName isEqualToString:other->_ownerName];
}

- (NSUInteger)hash
{
    return _zoneName.hash ^ (_ownerName.hash * 31);
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@: %p; zoneName=%@, ownerName=%@>", [self class], self, _zoneName, _ownerName];
}

@end
