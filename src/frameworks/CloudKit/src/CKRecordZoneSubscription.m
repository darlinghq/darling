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

@implementation CKRecordZoneSubscription

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (instancetype)initWithZoneID:(CKRecordZoneID *)zoneID
{
    return [self initWithZoneID:zoneID subscriptionID:[[NSUUID UUID] UUIDString]];
}

- (instancetype)initWithZoneID:(CKRecordZoneID *)zoneID subscriptionID:(CKSubscriptionID)subscriptionID
{
    if (zoneID == nil)
        CKRaiseInvalid(@"CKException", @"zoneID can not be nil");
    if ((self = [self _initWithSubscriptionID:subscriptionID]))
        _zoneID = [zoneID copy];
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    if ((self = [super initWithCoder:coder])) {
        _zoneID = [[coder decodeObjectOfClass:[CKRecordZoneID class] forKey:@"ZoneID"] copy];
        _recordType = [[coder decodeObjectOfClass:[NSString class] forKey:@"RecordType"] copy];
        if (_zoneID == nil) {
            return nil;
        }
    }
    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [super encodeWithCoder:coder];
    [coder encodeObject:_zoneID forKey:@"ZoneID"];
    [coder encodeObject:_recordType forKey:@"RecordType"];
}

- (id)copyWithZone:(NSZone *)zone
{
    CKRecordZoneSubscription *copy = [super copyWithZone:zone];
    copy->_zoneID = _zoneID;
    copy->_recordType = _recordType;
    return copy;
}

- (CKSubscriptionType)subscriptionType
{
    return CKSubscriptionTypeRecordZone;
}

- (CKRecordZoneID *)zoneID
{
    return _zoneID;
}

- (void)setZoneID:(CKRecordZoneID *)zoneID
{
    _zoneID = [zoneID copy];
}

- (CKRecordType)recordType
{
    return _recordType;
}

- (void)setRecordType:(CKRecordType)recordType
{
    _recordType = [recordType copy];
}

@end
