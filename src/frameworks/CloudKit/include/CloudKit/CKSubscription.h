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

#import <Foundation/Foundation.h>
#import <CloudKit/CKDefines.h>
#import <CloudKit/CKNotificationInfo.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSInteger, CKSubscriptionType) {
    CKSubscriptionTypeQuery = 1,
    CKSubscriptionTypeRecordZone = 2,
    CKSubscriptionTypeDatabase = 3,
} NS_SWIFT_NAME(CKSubscription.SubscriptionType);

// Base class of the subscriptions that ask for a push notification when data changes on the server.
@interface CKSubscription : NSObject <NSSecureCoding, NSCopying> {
@private
    CKSubscriptionID _subscriptionID;
    CKNotificationInfo *_notificationInfo;
}

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@property (nonatomic, readonly, copy) CKSubscriptionID subscriptionID NS_REFINED_FOR_SWIFT;
@property (nonatomic, readonly, assign) CKSubscriptionType subscriptionType;
@property (nonatomic, copy, nullable) CKNotificationInfo *notificationInfo;

@end

NS_ASSUME_NONNULL_END
