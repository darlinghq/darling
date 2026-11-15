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

@implementation CKQuerySubscription

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (instancetype)initWithRecordType:(CKRecordType)recordType predicate:(NSPredicate *)predicate options:(CKQuerySubscriptionOptions)options
{
    return [self initWithRecordType:recordType predicate:predicate subscriptionID:[[NSUUID UUID] UUIDString] options:options];
}

- (instancetype)initWithRecordType:(CKRecordType)recordType predicate:(NSPredicate *)predicate subscriptionID:(CKSubscriptionID)subscriptionID options:(CKQuerySubscriptionOptions)options
{
    if (recordType == nil || recordType.length == 0)
        CKRaiseInvalid(@"CKException", @"recordType can not be nil or empty");
    if (predicate == nil)
        CKRaiseInvalid(@"CKException", @"predicate can not be nil");
    if (!(options & (CKQuerySubscriptionOptionsFiresOnRecordCreation | CKQuerySubscriptionOptionsFiresOnRecordUpdate | CKQuerySubscriptionOptionsFiresOnRecordDeletion)))
        CKRaiseInvalid(NSInvalidArgumentException, @"Query subscriptions options must include at least one firing behavior");
    if ((self = [self _initWithSubscriptionID:subscriptionID])) {
        _recordType = [recordType copy];
        _predicate = [predicate copy];
        _querySubscriptionOptions = options;
    }
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    if ((self = [super initWithCoder:coder])) {
        _recordType = [[coder decodeObjectOfClass:[NSString class] forKey:@"RecordType"] copy];
        _predicate = [[coder decodeObjectOfClass:[NSPredicate class] forKey:@"Predicate"] copy];
        _querySubscriptionOptions = (CKQuerySubscriptionOptions)[coder decodeIntegerForKey:@"Options"];
        if (_recordType == nil || _predicate == nil) {
            return nil;
        }
    }
    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [super encodeWithCoder:coder];
    [coder encodeObject:_recordType forKey:@"RecordType"];
    [coder encodeObject:_predicate forKey:@"Predicate"];
    [coder encodeInteger:(NSInteger)_querySubscriptionOptions forKey:@"Options"];
}

- (id)copyWithZone:(NSZone *)zone
{
    CKQuerySubscription *copy = [super copyWithZone:zone];
    copy->_recordType = _recordType;
    copy->_predicate = _predicate;
    copy->_querySubscriptionOptions = _querySubscriptionOptions;
    return copy;
}

- (CKSubscriptionType)subscriptionType
{
    return CKSubscriptionTypeQuery;
}

- (CKRecordType)recordType
{
    return _recordType;
}

- (NSPredicate *)predicate
{
    return _predicate;
}

- (CKQuerySubscriptionOptions)querySubscriptionOptions
{
    return _querySubscriptionOptions;
}

@end
