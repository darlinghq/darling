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

@implementation CKRecordZone

+ (BOOL)supportsSecureCoding
{
    return YES;
}

+ (CKRecordZone *)defaultRecordZone
{
    CKRecordZoneID *zoneID = [[CKRecordZoneID alloc] initWithZoneName:CKRecordZoneDefaultName ownerName:CKCurrentUserDefaultName];
    return [[CKRecordZone alloc] initWithZoneID:zoneID];
}

- (instancetype)initWithZoneName:(NSString *)zoneName
{
    return [self initWithZoneID:[[CKRecordZoneID alloc] initWithZoneName:zoneName ownerName:CKCurrentUserDefaultName]];
}

- (instancetype)initWithZoneID:(CKRecordZoneID *)zoneID
{
    if (zoneID == nil)
        CKRaiseInvalid(@"CKException", @"zoneID can not be nil");
    if ((self = [super init]))
        _zoneID = [zoneID copy];
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    CKRecordZoneID *zoneID = [coder decodeObjectOfClass:[CKRecordZoneID class] forKey:@"ZoneID"];
    if (zoneID == nil) {
        return nil;
    }
    if ((self = [self initWithZoneID:zoneID])) {
        _capabilities = (CKRecordZoneCapabilities)[coder decodeIntegerForKey:@"Capabilities"];
        _share = [[coder decodeObjectOfClass:[CKReference class] forKey:@"Share"] copy];
    }
    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_zoneID forKey:@"ZoneID"];
    [coder encodeInteger:(NSInteger)_capabilities forKey:@"Capabilities"];
    [coder encodeObject:_share forKey:@"Share"];
}

- (id)copyWithZone:(NSZone *)zone
{
    CKRecordZone *copy = [[[self class] allocWithZone:zone] initWithZoneID:_zoneID];
    copy->_capabilities = _capabilities;
    copy->_share = _share;
    return copy;
}

- (CKRecordZoneID *)zoneID
{
    return _zoneID;
}

- (CKRecordZoneCapabilities)capabilities
{
    return _capabilities;
}

- (CKReference *)share
{
    return _share;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@: %p; zoneID=%@:%@, capabilities=%lu>", [self class], self,
            _zoneID.zoneName, _zoneID.ownerName, (unsigned long)_capabilities];
}

@end
