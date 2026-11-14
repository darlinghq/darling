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

@implementation CKRecordID

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (instancetype)initWithRecordName:(NSString *)recordName
{
    CKRecordZoneID *zoneID = [[CKRecordZoneID alloc] initWithZoneName:CKRecordZoneDefaultName ownerName:CKCurrentUserDefaultName];
    return [self initWithRecordName:recordName zoneID:zoneID];
}

- (instancetype)initWithRecordName:(NSString *)recordName zoneID:(CKRecordZoneID *)zoneID
{
    CKValidateName(recordName, @"recordName");
    if (zoneID == nil)
        CKRaiseInvalid(@"CKException", @"zoneID can not be nil");
    if ((self = [super init])) {
        _recordName = [recordName copy];
        _zoneID = [zoneID copy];
    }
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    NSString *name = [coder decodeObjectOfClass:[NSString class] forKey:@"RecordName"];
    CKRecordZoneID *zoneID = [coder decodeObjectOfClass:[CKRecordZoneID class] forKey:@"ZoneID"];
    if (name.length == 0 || zoneID == nil) {
        return nil;
    }
    return [self initWithRecordName:name zoneID:zoneID];
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_recordName forKey:@"RecordName"];
    [coder encodeObject:_zoneID forKey:@"ZoneID"];
}

- (id)copyWithZone:(NSZone *)zone
{
    // Immutable.
    return self;
}

- (NSString *)recordName
{
    return _recordName;
}

- (CKRecordZoneID *)zoneID
{
    return _zoneID;
}

- (BOOL)isEqual:(id)object
{
    if (self == object)
        return YES;
    if (![object isKindOfClass:[CKRecordID class]])
        return NO;
    CKRecordID *other = object;
    return [_recordName isEqualToString:other->_recordName] && [_zoneID isEqual:other->_zoneID];
}

- (NSUInteger)hash
{
    return _recordName.hash ^ (_zoneID.hash * 31);
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@: %p; recordName=%@, zoneID=%@:%@>", [self class], self, _recordName,
            _zoneID.zoneName, _zoneID.ownerName];
}

@end
