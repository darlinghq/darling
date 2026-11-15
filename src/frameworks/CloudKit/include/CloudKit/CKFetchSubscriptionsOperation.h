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
#import <CloudKit/CKDatabaseOperation.h>
#import <CloudKit/CKSubscription.h>

NS_ASSUME_NONNULL_BEGIN

// Fetches subscriptions by ID, or all of them.
@interface CKFetchSubscriptionsOperation : CKDatabaseOperation {
@private
    NSArray<CKSubscriptionID> *_subscriptionIDs;
    void (^_fetchSubscriptionCompletionBlock)(NSDictionary<CKSubscriptionID, CKSubscription *> * _Nullable subscriptionsBySubscriptionID, NSError * _Nullable operationError);
}

- (instancetype)initWithSubscriptionIDs:(NSArray<CKSubscriptionID> *)subscriptionIDs NS_REFINED_FOR_SWIFT;
+ (instancetype)fetchAllSubscriptionsOperation NS_SWIFT_NAME(fetchAllSubscriptionsOperation());

// nil for the operation that fetches all subscriptions.
@property (nonatomic, copy, nullable) NSArray<CKSubscriptionID> *subscriptionIDs NS_REFINED_FOR_SWIFT;
@property (nonatomic, copy, nullable) void (^fetchSubscriptionCompletionBlock)(NSDictionary<CKSubscriptionID, CKSubscription *> * _Nullable subscriptionsBySubscriptionID, NSError * _Nullable operationError) NS_REFINED_FOR_SWIFT;
@end

NS_ASSUME_NONNULL_END
