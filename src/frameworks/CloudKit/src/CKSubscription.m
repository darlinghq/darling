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

@implementation CKSubscription

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (instancetype)_initWithSubscriptionID:(CKSubscriptionID)subscriptionID
{
    if (subscriptionID == nil)
        CKRaiseInvalid(@"CKException", @"subscriptionID can not be nil");
    if ((self = [super init]))
        _subscriptionID = [subscriptionID copy];
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    CKSubscriptionID subscriptionID = [coder decodeObjectOfClass:[NSString class] forKey:@"SubscriptionID"];
    if (subscriptionID == nil) {
        return nil;
    }
    if ((self = [self _initWithSubscriptionID:subscriptionID]))
        _notificationInfo = [[coder decodeObjectOfClass:[CKNotificationInfo class] forKey:@"NotificationInfo"] copy];
    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_subscriptionID forKey:@"SubscriptionID"];
    [coder encodeObject:_notificationInfo forKey:@"NotificationInfo"];
}

- (id)copyWithZone:(NSZone *)zone
{
    CKSubscription *copy = [[[self class] allocWithZone:zone] _initWithSubscriptionID:_subscriptionID];
    copy->_notificationInfo = [_notificationInfo copy];
    return copy;
}

- (CKSubscriptionID)subscriptionID
{
    return _subscriptionID;
}

// Overridden by the concrete subscriptions.
- (CKSubscriptionType)subscriptionType
{
    return 0;
}

- (CKNotificationInfo *)notificationInfo
{
    return _notificationInfo;
}

- (void)setNotificationInfo:(CKNotificationInfo *)notificationInfo
{
    _notificationInfo = [notificationInfo copy];
}

- (BOOL)isEqual:(id)object
{
    if (self == object)
        return YES;
    if (![object isKindOfClass:[CKSubscription class]] || [object subscriptionType] != [self subscriptionType])
        return NO;
    return [_subscriptionID isEqual:((CKSubscription *)object)->_subscriptionID];
}

- (NSUInteger)hash
{
    return _subscriptionID.hash;
}

@end
