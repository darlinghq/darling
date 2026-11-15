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

@implementation CKDatabaseSubscription

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (instancetype)init
{
    return [self initWithSubscriptionID:[[NSUUID UUID] UUIDString]];
}

- (instancetype)initWithSubscriptionID:(CKSubscriptionID)subscriptionID
{
    return [self _initWithSubscriptionID:subscriptionID];
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    if ((self = [super initWithCoder:coder]))
        _recordType = [[coder decodeObjectOfClass:[NSString class] forKey:@"RecordType"] copy];
    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [super encodeWithCoder:coder];
    [coder encodeObject:_recordType forKey:@"RecordType"];
}

- (id)copyWithZone:(NSZone *)zone
{
    CKDatabaseSubscription *copy = [super copyWithZone:zone];
    copy->_recordType = _recordType;
    return copy;
}

- (CKSubscriptionType)subscriptionType
{
    return CKSubscriptionTypeDatabase;
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
