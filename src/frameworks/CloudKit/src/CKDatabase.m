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

@implementation CKDatabase

- (instancetype)_initWithContainer:(CKContainer *)container databaseScope:(CKDatabaseScope)databaseScope
{
    if ((self = [super init])) {
        _container = container;
        _databaseScope = databaseScope;
        _operationQueue = [[NSOperationQueue alloc] init];
        _operationQueue.name = @"CloudKit database operations";
    }
    return self;
}

- (CKDatabaseScope)databaseScope
{
    return _databaseScope;
}

- (void)addOperation:(CKOperation *)operation
{
    if ([operation isKindOfClass:[CKDatabaseOperation class]])
        ((CKDatabaseOperation *)operation).database = self;
    if (operation.configuration.container == nil)
        operation.configuration.container = _container;
    [_operationQueue addOperation:operation];
}

- (void)fetchRecordWithID:(CKRecordID *)recordID completionHandler:(void (^)(CKRecord *, NSError *))completionHandler
{
    CKFetchRecordsOperation *operation = [[CKFetchRecordsOperation alloc] initWithRecordIDs:@[recordID]];
    operation.fetchRecordsCompletionBlock = ^(NSDictionary<CKRecordID *, CKRecord *> *records, NSError *error) {
        completionHandler(records[recordID], error);
    };
    [self addOperation:operation];
}

- (void)saveRecord:(CKRecord *)record completionHandler:(void (^)(CKRecord *, NSError *))completionHandler
{
    CKModifyRecordsOperation *operation = [[CKModifyRecordsOperation alloc] initWithRecordsToSave:@[record] recordIDsToDelete:nil];
    operation.savePolicy = CKRecordSaveIfServerRecordUnchanged;
    operation.modifyRecordsCompletionBlock = ^(NSArray<CKRecord *> *saved, NSArray<CKRecordID *> *deleted, NSError *error) {
        completionHandler(saved.firstObject, error);
    };
    [self addOperation:operation];
}

- (void)deleteRecordWithID:(CKRecordID *)recordID completionHandler:(void (^)(CKRecordID *, NSError *))completionHandler
{
    CKModifyRecordsOperation *operation = [[CKModifyRecordsOperation alloc] initWithRecordsToSave:nil recordIDsToDelete:@[recordID]];
    operation.modifyRecordsCompletionBlock = ^(NSArray<CKRecord *> *saved, NSArray<CKRecordID *> *deleted, NSError *error) {
        completionHandler(deleted.firstObject, error);
    };
    [self addOperation:operation];
}

- (void)performQuery:(CKQuery *)query inZoneWithID:(CKRecordZoneID *)zoneID completionHandler:(void (^)(NSArray<CKRecord *> *, NSError *))completionHandler
{
    CKQueryOperation *operation = [[CKQueryOperation alloc] initWithQuery:query];
    operation.zoneID = zoneID;
    NSMutableArray<CKRecord *> *results = [NSMutableArray array];
    operation.recordFetchedBlock = ^(CKRecord *record) {
        [results addObject:record];
    };
    operation.queryCompletionBlock = ^(CKQueryCursor *cursor, NSError *error) {
        completionHandler(error ? nil : results, error);
    };
    [self addOperation:operation];
}

- (void)fetchAllRecordZonesWithCompletionHandler:(void (^)(NSArray<CKRecordZone *> *, NSError *))completionHandler
{
    CKFetchRecordZonesOperation *operation = [CKFetchRecordZonesOperation fetchAllRecordZonesOperation];
    operation.fetchRecordZonesCompletionBlock = ^(NSDictionary<CKRecordZoneID *, CKRecordZone *> *zones, NSError *error) {
        completionHandler(zones.allValues, error);
    };
    [self addOperation:operation];
}

- (void)fetchRecordZoneWithID:(CKRecordZoneID *)zoneID completionHandler:(void (^)(CKRecordZone *, NSError *))completionHandler
{
    CKFetchRecordZonesOperation *operation = [[CKFetchRecordZonesOperation alloc] initWithRecordZoneIDs:@[zoneID]];
    operation.fetchRecordZonesCompletionBlock = ^(NSDictionary<CKRecordZoneID *, CKRecordZone *> *zones, NSError *error) {
        completionHandler(zones[zoneID], error);
    };
    [self addOperation:operation];
}

- (void)saveRecordZone:(CKRecordZone *)zone completionHandler:(void (^)(CKRecordZone *, NSError *))completionHandler
{
    CKModifyRecordZonesOperation *operation = [[CKModifyRecordZonesOperation alloc] initWithRecordZonesToSave:@[zone] recordZoneIDsToDelete:nil];
    operation.modifyRecordZonesCompletionBlock = ^(NSArray<CKRecordZone *> *saved, NSArray<CKRecordZoneID *> *deleted, NSError *error) {
        completionHandler(saved.firstObject, error);
    };
    [self addOperation:operation];
}

- (void)deleteRecordZoneWithID:(CKRecordZoneID *)zoneID completionHandler:(void (^)(CKRecordZoneID *, NSError *))completionHandler
{
    CKModifyRecordZonesOperation *operation = [[CKModifyRecordZonesOperation alloc] initWithRecordZonesToSave:nil recordZoneIDsToDelete:@[zoneID]];
    operation.modifyRecordZonesCompletionBlock = ^(NSArray<CKRecordZone *> *saved, NSArray<CKRecordZoneID *> *deleted, NSError *error) {
        completionHandler(deleted.firstObject, error);
    };
    [self addOperation:operation];
}

- (void)fetchSubscriptionWithID:(CKSubscriptionID)subscriptionID completionHandler:(void (^)(CKSubscription *, NSError *))completionHandler
{
    CKFetchSubscriptionsOperation *operation = [[CKFetchSubscriptionsOperation alloc] initWithSubscriptionIDs:@[subscriptionID]];
    operation.fetchSubscriptionCompletionBlock = ^(NSDictionary<CKSubscriptionID, CKSubscription *> *subscriptions, NSError *error) {
        completionHandler(subscriptions[subscriptionID], error);
    };
    [self addOperation:operation];
}

- (void)fetchAllSubscriptionsWithCompletionHandler:(void (^)(NSArray<CKSubscription *> *, NSError *))completionHandler
{
    CKFetchSubscriptionsOperation *operation = [CKFetchSubscriptionsOperation fetchAllSubscriptionsOperation];
    operation.fetchSubscriptionCompletionBlock = ^(NSDictionary<CKSubscriptionID, CKSubscription *> *subscriptions, NSError *error) {
        completionHandler(subscriptions.allValues, error);
    };
    [self addOperation:operation];
}

- (void)saveSubscription:(CKSubscription *)subscription completionHandler:(void (^)(CKSubscription *, NSError *))completionHandler
{
    CKModifySubscriptionsOperation *operation = [[CKModifySubscriptionsOperation alloc] initWithSubscriptionsToSave:@[subscription] subscriptionIDsToDelete:nil];
    operation.modifySubscriptionsCompletionBlock = ^(NSArray<CKSubscription *> *saved, NSArray<CKSubscriptionID> *deleted, NSError *error) {
        completionHandler(saved.firstObject, error);
    };
    [self addOperation:operation];
}

- (void)deleteSubscriptionWithID:(CKSubscriptionID)subscriptionID completionHandler:(void (^)(NSString *, NSError *))completionHandler
{
    CKModifySubscriptionsOperation *operation = [[CKModifySubscriptionsOperation alloc] initWithSubscriptionsToSave:nil subscriptionIDsToDelete:@[subscriptionID]];
    operation.modifySubscriptionsCompletionBlock = ^(NSArray<CKSubscription *> *saved, NSArray<CKSubscriptionID> *deleted, NSError *error) {
        completionHandler(deleted.firstObject, error);
    };
    [self addOperation:operation];
}

@end
