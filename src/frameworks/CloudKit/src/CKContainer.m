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

NSNotificationName const CKAccountChangedNotification = @"CKAccountChangedNotification";

// Containers are shared by identifier, so the databases and operation queues are too.
static NSMutableDictionary<NSString *, CKContainer *> *containers;
static dispatch_once_t containersOnce;

@implementation CKContainer

+ (CKContainer *)containerWithIdentifier:(NSString *)containerIdentifier
{
    if (containerIdentifier == nil || containerIdentifier.length == 0)
        CKRaiseInvalid(@"CKException", @"containerIdentifier can not be nil or empty");
    dispatch_once(&containersOnce, ^{
        containers = [NSMutableDictionary dictionary];
    });
    @synchronized (containers) {
        CKContainer *container = containers[containerIdentifier];
        if (container == nil) {
            container = [[CKContainer alloc] _initWithContainerIdentifier:containerIdentifier];
            containers[containerIdentifier] = container;
        }
        return container;
    }
}

+ (CKContainer *)defaultContainer
{
    NSString *bundleIdentifier = [[NSBundle mainBundle] bundleIdentifier];
    if (bundleIdentifier.length == 0)
        bundleIdentifier = [[NSProcessInfo processInfo] processName];
    return [self containerWithIdentifier:[@"iCloud." stringByAppendingString:bundleIdentifier]];
}

- (instancetype)_initWithContainerIdentifier:(NSString *)containerIdentifier
{
    if ((self = [super init])) {
        _containerIdentifier = [containerIdentifier copy];
        _publicCloudDatabase = [[CKDatabase alloc] _initWithContainer:self databaseScope:CKDatabaseScopePublic];
        _privateCloudDatabase = [[CKDatabase alloc] _initWithContainer:self databaseScope:CKDatabaseScopePrivate];
        _sharedCloudDatabase = [[CKDatabase alloc] _initWithContainer:self databaseScope:CKDatabaseScopeShared];
        _operationQueue = [[NSOperationQueue alloc] init];
        _operationQueue.name = @"CloudKit container operations";
    }
    return self;
}

- (NSString *)containerIdentifier
{
    return _containerIdentifier;
}

- (CKDatabase *)publicCloudDatabase
{
    return _publicCloudDatabase;
}

- (CKDatabase *)privateCloudDatabase
{
    return _privateCloudDatabase;
}

- (CKDatabase *)sharedCloudDatabase
{
    return _sharedCloudDatabase;
}

- (CKDatabase *)databaseWithDatabaseScope:(CKDatabaseScope)databaseScope
{
    switch (databaseScope) {
        case CKDatabaseScopePublic:
            return _publicCloudDatabase;
        case CKDatabaseScopePrivate:
            return _privateCloudDatabase;
        case CKDatabaseScopeShared:
            return _sharedCloudDatabase;
    }
    CKRaiseInvalid(NSInvalidArgumentException, [NSString stringWithFormat:@"Unknown database scope %ld", (long)databaseScope]);
    return nil;
}

- (void)addOperation:(CKOperation *)operation
{
    if ([operation isKindOfClass:[CKDatabaseOperation class]] && ((CKDatabaseOperation *)operation).database == nil)
        ((CKDatabaseOperation *)operation).database = _privateCloudDatabase;
    if (operation.configuration.container == nil)
        operation.configuration.container = self;
    [_operationQueue addOperation:operation];
}

// Calls the handler on a background queue, as the operations do.
static void CKDeliver(dispatch_block_t block)
{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), block);
}

- (void)accountStatusWithCompletionHandler:(void (^)(CKAccountStatus, NSError *))completionHandler
{
    CKDeliver(^{
        completionHandler(CKAccountStatusNoAccount, nil);
    });
}

- (void)statusForApplicationPermission:(CKApplicationPermissions)applicationPermission completionHandler:(CKApplicationPermissionBlock)completionHandler
{
    CKDeliver(^{
        completionHandler(CKApplicationPermissionStatusCouldNotComplete, CKNotAuthenticatedError());
    });
}

- (void)requestApplicationPermission:(CKApplicationPermissions)applicationPermission completionHandler:(CKApplicationPermissionBlock)completionHandler
{
    CKDeliver(^{
        completionHandler(CKApplicationPermissionStatusCouldNotComplete, CKNotAuthenticatedError());
    });
}

- (void)fetchUserRecordIDWithCompletionHandler:(void (^)(CKRecordID *, NSError *))completionHandler
{
    CKFetchRecordsOperation *operation = [CKFetchRecordsOperation fetchCurrentUserRecordOperation];
    operation.fetchRecordsCompletionBlock = ^(NSDictionary<CKRecordID *, CKRecord *> *records, NSError *error) {
        completionHandler(nil, error);
    };
    [self addOperation:operation];
}

- (void)discoverAllIdentitiesWithCompletionHandler:(void (^)(NSArray<CKUserIdentity *> *, NSError *))completionHandler
{
    CKDiscoverAllUserIdentitiesOperation *operation = [[CKDiscoverAllUserIdentitiesOperation alloc] init];
    NSMutableArray<CKUserIdentity *> *identities = [NSMutableArray array];
    operation.userIdentityDiscoveredBlock = ^(CKUserIdentity *identity) {
        [identities addObject:identity];
    };
    operation.discoverAllUserIdentitiesCompletionBlock = ^(NSError *error) {
        completionHandler(error ? nil : identities, error);
    };
    [self addOperation:operation];
}

- (void)_discoverIdentityWithLookupInfo:(CKUserIdentityLookupInfo *)lookupInfo completionHandler:(void (^)(CKUserIdentity *, NSError *))completionHandler
{
    CKDiscoverUserIdentitiesOperation *operation = [[CKDiscoverUserIdentitiesOperation alloc] initWithUserIdentityLookupInfos:@[lookupInfo]];
    __block CKUserIdentity *found = nil;
    operation.userIdentityDiscoveredBlock = ^(CKUserIdentity *identity, CKUserIdentityLookupInfo *info) {
        found = identity;
    };
    operation.discoverUserIdentitiesCompletionBlock = ^(NSError *error) {
        completionHandler(found, error);
    };
    [self addOperation:operation];
}

- (void)discoverUserIdentityWithEmailAddress:(NSString *)email completionHandler:(void (^)(CKUserIdentity *, NSError *))completionHandler
{
    [self _discoverIdentityWithLookupInfo:[[CKUserIdentityLookupInfo alloc] initWithEmailAddress:email] completionHandler:completionHandler];
}

- (void)discoverUserIdentityWithPhoneNumber:(NSString *)phoneNumber completionHandler:(void (^)(CKUserIdentity *, NSError *))completionHandler
{
    [self _discoverIdentityWithLookupInfo:[[CKUserIdentityLookupInfo alloc] initWithPhoneNumber:phoneNumber] completionHandler:completionHandler];
}

- (void)discoverUserIdentityWithUserRecordID:(CKRecordID *)userRecordID completionHandler:(void (^)(CKUserIdentity *, NSError *))completionHandler
{
    [self _discoverIdentityWithLookupInfo:[[CKUserIdentityLookupInfo alloc] initWithUserRecordID:userRecordID] completionHandler:completionHandler];
}

- (void)_fetchShareParticipantWithLookupInfo:(CKUserIdentityLookupInfo *)lookupInfo completionHandler:(void (^)(CKShareParticipant *, NSError *))completionHandler
{
    CKFetchShareParticipantsOperation *operation = [[CKFetchShareParticipantsOperation alloc] initWithUserIdentityLookupInfos:@[lookupInfo]];
    __block CKShareParticipant *found = nil;
    operation.shareParticipantFetchedBlock = ^(CKShareParticipant *participant) {
        found = participant;
    };
    operation.fetchShareParticipantsCompletionBlock = ^(NSError *error) {
        completionHandler(found, error);
    };
    [self addOperation:operation];
}

- (void)fetchShareParticipantWithEmailAddress:(NSString *)emailAddress completionHandler:(void (^)(CKShareParticipant *, NSError *))completionHandler
{
    [self _fetchShareParticipantWithLookupInfo:[[CKUserIdentityLookupInfo alloc] initWithEmailAddress:emailAddress] completionHandler:completionHandler];
}

- (void)fetchShareParticipantWithPhoneNumber:(NSString *)phoneNumber completionHandler:(void (^)(CKShareParticipant *, NSError *))completionHandler
{
    [self _fetchShareParticipantWithLookupInfo:[[CKUserIdentityLookupInfo alloc] initWithPhoneNumber:phoneNumber] completionHandler:completionHandler];
}

- (void)fetchShareParticipantWithUserRecordID:(CKRecordID *)userRecordID completionHandler:(void (^)(CKShareParticipant *, NSError *))completionHandler
{
    [self _fetchShareParticipantWithLookupInfo:[[CKUserIdentityLookupInfo alloc] initWithUserRecordID:userRecordID] completionHandler:completionHandler];
}

- (void)fetchShareMetadataWithURL:(NSURL *)url completionHandler:(void (^)(CKShareMetadata *, NSError *))completionHandler
{
    CKFetchShareMetadataOperation *operation = [[CKFetchShareMetadataOperation alloc] initWithShareURLs:@[url]];
    __block CKShareMetadata *found = nil;
    operation.perShareMetadataBlock = ^(NSURL *shareURL, CKShareMetadata *metadata, NSError *error) {
        found = metadata;
    };
    operation.fetchShareMetadataCompletionBlock = ^(NSError *error) {
        completionHandler(found, error);
    };
    [self addOperation:operation];
}

- (void)acceptShareMetadata:(CKShareMetadata *)metadata completionHandler:(void (^)(CKShare *, NSError *))completionHandler
{
    CKAcceptSharesOperation *operation = [[CKAcceptSharesOperation alloc] initWithShareMetadatas:@[metadata]];
    __block CKShare *accepted = nil;
    operation.perShareCompletionBlock = ^(CKShareMetadata *shareMetadata, CKShare *share, NSError *error) {
        accepted = share;
    };
    operation.acceptSharesCompletionBlock = ^(NSError *error) {
        completionHandler(accepted, error);
    };
    [self addOperation:operation];
}

// Nothing is ever persisted as a long lived operation, so there is nothing to resume.
- (void)fetchAllLongLivedOperationIDsWithCompletionHandler:(void (^)(NSArray<CKOperationID> *, NSError *))completionHandler
{
    CKDeliver(^{
        completionHandler(@[], nil);
    });
}

- (void)fetchLongLivedOperationWithID:(CKOperationID)operationID completionHandler:(void (^)(CKOperation *, NSError *))completionHandler
{
    CKDeliver(^{
        completionHandler(nil, nil);
    });
}

@end
