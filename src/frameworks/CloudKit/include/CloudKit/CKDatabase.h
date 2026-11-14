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
#import <CloudKit/CKRecord.h>
#import <CloudKit/CKRecordZone.h>
#import <CloudKit/CKSubscription.h>

NS_ASSUME_NONNULL_BEGIN

@class CKContainer;
@class CKOperation;
@class CKQuery;

// One of the databases of a container. The convenience methods run the corresponding operation and
// report its outcome once through their completion handler.
@interface CKDatabase : NSObject {
@private
    CKContainer *_container;
    CKDatabaseScope _databaseScope;
    NSOperationQueue *_operationQueue;
}

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@property (nonatomic, readonly, assign) CKDatabaseScope databaseScope;

// Runs the operation against this database.
- (void)addOperation:(CKOperation *)operation;

- (void)fetchRecordWithID:(CKRecordID *)recordID completionHandler:(void (^)(CKRecord * _Nullable record, NSError * _Nullable error))completionHandler NS_SWIFT_NAME(fetch(withRecordID:completionHandler:));
- (void)saveRecord:(CKRecord *)record completionHandler:(void (^)(CKRecord * _Nullable record, NSError * _Nullable error))completionHandler NS_SWIFT_NAME(save(_:completionHandler:));
- (void)deleteRecordWithID:(CKRecordID *)recordID completionHandler:(void (^)(CKRecordID * _Nullable recordID, NSError * _Nullable error))completionHandler NS_SWIFT_NAME(delete(withRecordID:completionHandler:));
// A nil zone ID searches all the zones of the database (private database only).
- (void)performQuery:(CKQuery *)query inZoneWithID:(nullable CKRecordZoneID *)zoneID completionHandler:(void (^)(NSArray<CKRecord *> * _Nullable results, NSError * _Nullable error))completionHandler NS_SWIFT_NAME(perform(_:inZoneWith:completionHandler:));

- (void)fetchAllRecordZonesWithCompletionHandler:(void (^)(NSArray<CKRecordZone *> * _Nullable zones, NSError * _Nullable error))completionHandler NS_SWIFT_NAME(fetchAllRecordZones(completionHandler:));
- (void)fetchRecordZoneWithID:(CKRecordZoneID *)zoneID completionHandler:(void (^)(CKRecordZone * _Nullable zone, NSError * _Nullable error))completionHandler NS_SWIFT_NAME(fetch(withRecordZoneID:completionHandler:));
- (void)saveRecordZone:(CKRecordZone *)zone completionHandler:(void (^)(CKRecordZone * _Nullable zone, NSError * _Nullable error))completionHandler NS_SWIFT_NAME(save(_:completionHandler:));
- (void)deleteRecordZoneWithID:(CKRecordZoneID *)zoneID completionHandler:(void (^)(CKRecordZoneID * _Nullable zoneID, NSError * _Nullable error))completionHandler NS_SWIFT_NAME(delete(withRecordZoneID:completionHandler:));

- (void)fetchSubscriptionWithID:(CKSubscriptionID)subscriptionID completionHandler:(void (^)(CKSubscription * _Nullable subscription, NSError * _Nullable error))completionHandler NS_SWIFT_NAME(__fetch(withSubscriptionID:completionHandler:));
- (void)fetchAllSubscriptionsWithCompletionHandler:(void (^)(NSArray<CKSubscription *> * _Nullable subscriptions, NSError * _Nullable error))completionHandler NS_SWIFT_NAME(fetchAllSubscriptions(completionHandler:));
- (void)saveSubscription:(CKSubscription *)subscription completionHandler:(void (^)(CKSubscription * _Nullable subscription, NSError * _Nullable error))completionHandler NS_SWIFT_NAME(save(_:completionHandler:));
- (void)deleteSubscriptionWithID:(CKSubscriptionID)subscriptionID completionHandler:(void (^)(NSString * _Nullable subscriptionID, NSError * _Nullable error))completionHandler NS_SWIFT_NAME(__delete(withSubscriptionID:completionHandler:));

@end

NS_ASSUME_NONNULL_END
