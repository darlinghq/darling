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
#import <CloudKit/CKSubscription.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_OPTIONS(NSUInteger, CKQuerySubscriptionOptions) {
    CKQuerySubscriptionOptionsFiresOnRecordCreation = 1 << 0,
    CKQuerySubscriptionOptionsFiresOnRecordUpdate = 1 << 1,
    CKQuerySubscriptionOptionsFiresOnRecordDeletion = 1 << 2,
    CKQuerySubscriptionOptionsFiresOnce = 1 << 3,
} NS_SWIFT_NAME(CKQuerySubscription.Options);

// Fires when a record that matches a predicate is created, changed or deleted. At least one firing option is required.
@interface CKQuerySubscription : CKSubscription <NSSecureCoding, NSCopying> {
@private
    CKRecordType _recordType;
    NSPredicate *_predicate;
    CKQuerySubscriptionOptions _querySubscriptionOptions;
}

- (instancetype)initWithRecordType:(CKRecordType)recordType predicate:(NSPredicate *)predicate options:(CKQuerySubscriptionOptions)querySubscriptionOptions NS_REFINED_FOR_SWIFT;
- (instancetype)initWithRecordType:(CKRecordType)recordType predicate:(NSPredicate *)predicate subscriptionID:(CKSubscriptionID)subscriptionID options:(CKQuerySubscriptionOptions)querySubscriptionOptions NS_REFINED_FOR_SWIFT;

@property (nonatomic, readonly, copy, nullable) CKRecordType recordType NS_REFINED_FOR_SWIFT;
@property (nonatomic, readonly, copy) NSPredicate *predicate;
@property (nonatomic, readonly, assign) CKQuerySubscriptionOptions querySubscriptionOptions;

@end

NS_ASSUME_NONNULL_END
