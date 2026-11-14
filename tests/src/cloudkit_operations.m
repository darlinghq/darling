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

// CloudKit operations, containers and databases: enum values and defaults of the operation classes,
// operation configuration and groups, and the outcome of every operation when there is no iCloud account.
// Prints PASS when everything holds; the exit code is the number of failed checks.
//
// The no-account section needs CKContainer objects. On macOS a process without the iCloud entitlements
// traps as soon as it asks for a container (observed on macOS 26.7.3), so there the test is built with
// -DCK_SKIP_NO_ACCOUNT_PATH, which leaves the value and default checks; those run unchanged on macOS.

#import <Foundation/Foundation.h>
#import <CloudKit/CloudKit.h>

#pragma clang diagnostic ignored "-Wnonnull"
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

static int failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)

static void testEnums(void)
{
    CHECK(CKAccountStatusCouldNotDetermine == 0 && CKAccountStatusAvailable == 1 && CKAccountStatusRestricted == 2
          && CKAccountStatusNoAccount == 3 && CKAccountStatusTemporarilyUnavailable == 4);
    CHECK(CKApplicationPermissionUserDiscoverability == 1);
    CHECK(CKApplicationPermissionStatusInitialState == 0 && CKApplicationPermissionStatusCouldNotComplete == 1
          && CKApplicationPermissionStatusDenied == 2 && CKApplicationPermissionStatusGranted == 3);
    CHECK(CKRecordSaveIfServerRecordUnchanged == 0 && CKRecordSaveChangedKeys == 1 && CKRecordSaveAllKeys == 2);
    CHECK(CKOperationGroupTransferSizeUnknown == 0 && CKOperationGroupTransferSizeKilobytes == 1
          && CKOperationGroupTransferSizeMegabytes == 2 && CKOperationGroupTransferSizeTensOfMegabytes == 3
          && CKOperationGroupTransferSizeHundredsOfMegabytes == 4 && CKOperationGroupTransferSizeGigabytes == 5
          && CKOperationGroupTransferSizeTensOfGigabytes == 6 && CKOperationGroupTransferSizeHundredsOfGigabytes == 7);
    CHECK(CKQueryOperationMaximumResults == 0);
    CHECK([CKAccountChangedNotification isEqualToString:@"CKAccountChangedNotification"]);
}

static void testConfigurationAndGroup(void)
{
    CKOperationConfiguration *configuration = [[CKOperationConfiguration alloc] init];
    CHECK(configuration.qualityOfService == NSQualityOfServiceDefault);
    CHECK(configuration.allowsCellularAccess && !configuration.longLived);
    CHECK(configuration.timeoutIntervalForRequest == 60 && configuration.timeoutIntervalForResource == -1);
    configuration.allowsCellularAccess = NO;
    configuration.timeoutIntervalForRequest = 5;
    configuration.qualityOfService = NSQualityOfServiceUtility;
    configuration.longLived = YES;
    CKOperationConfiguration *copy = [configuration copy];
    CHECK(!copy.allowsCellularAccess && copy.timeoutIntervalForRequest == 5 && copy.longLived);
    CHECK(copy.qualityOfService == NSQualityOfServiceUtility);
    copy.allowsCellularAccess = YES;
    CHECK(!configuration.allowsCellularAccess);

    CKOperationGroup *group = [[CKOperationGroup alloc] init];
    CHECK(group.operationGroupID.length == 16);
    CHECK(group.name == nil && group.quantity == 0 && group.expectedSendSize == 0 && group.expectedReceiveSize == 0);
    CHECK(group.defaultConfiguration != nil);
    CHECK(![[[CKOperationGroup alloc] init].operationGroupID isEqualToString:group.operationGroupID]);
    group.name = @"sync";
    group.quantity = 3;
    group.expectedSendSize = CKOperationGroupTransferSizeMegabytes;
    NSError *error = nil;
    NSData *data = [NSKeyedArchiver archivedDataWithRootObject:group requiringSecureCoding:YES error:&error];
    CKOperationGroup *restored = data ? [NSKeyedUnarchiver unarchivedObjectOfClass:[CKOperationGroup class] fromData:data error:&error] : nil;
    CHECK(restored != nil);
    CHECK([restored.name isEqualToString:@"sync"] && restored.quantity == 3 && restored.expectedSendSize == CKOperationGroupTransferSizeMegabytes);
    CHECK([restored.operationGroupID isEqualToString:group.operationGroupID]);
}

static void testOperationDefaults(void)
{
    CKFetchRecordsOperation *fetch = [[CKFetchRecordsOperation alloc] init];
    CHECK(fetch.recordIDs == nil && fetch.desiredKeys == nil && fetch.group == nil);
    CHECK(fetch.configuration != nil && fetch.qualityOfService == NSQualityOfServiceDefault);
    CHECK(fetch.allowsCellularAccess && !fetch.isLongLived);
    CHECK(fetch.timeoutIntervalForRequest == 60 && fetch.timeoutIntervalForResource == -1);
    CHECK(fetch.operationID.length == 16);
    CHECK(![[[CKFetchRecordsOperation alloc] init].operationID isEqualToString:fetch.operationID]);
    CHECK(fetch.isConcurrent && fetch.isReady && !fetch.isExecuting && !fetch.isFinished);
    CHECK(fetch.perRecordProgressBlock == nil && fetch.fetchRecordsCompletionBlock == nil);

    // The configuration and the operation's own settings are one thing.
    CKOperationConfiguration *configuration = [[CKOperationConfiguration alloc] init];
    configuration.qualityOfService = NSQualityOfServiceUtility;
    configuration.timeoutIntervalForRequest = 7;
    fetch.configuration = configuration;
    CHECK(fetch.qualityOfService == NSQualityOfServiceUtility && fetch.timeoutIntervalForRequest == 7);
    fetch.qualityOfService = NSQualityOfServiceBackground;
    CHECK(fetch.configuration.qualityOfService == NSQualityOfServiceBackground);
    fetch.allowsCellularAccess = NO;
    CHECK(!fetch.configuration.allowsCellularAccess);

    CKRecordID *recordID = [[CKRecordID alloc] initWithRecordName:@"a"];
    CKFetchRecordsOperation *byIDs = [[CKFetchRecordsOperation alloc] initWithRecordIDs:@[recordID]];
    CHECK(byIDs.recordIDs.count == 1 && [byIDs.recordIDs[0] isEqual:recordID]);
    CHECK([CKFetchRecordsOperation fetchCurrentUserRecordOperation] != nil);

    CKModifyRecordsOperation *modify = [[CKModifyRecordsOperation alloc] init];
    CHECK(modify.recordsToSave == nil && modify.recordIDsToDelete == nil && modify.clientChangeTokenData == nil);
    CHECK(modify.savePolicy == CKRecordSaveIfServerRecordUnchanged && modify.atomic);
    CKRecord *record = [[CKRecord alloc] initWithRecordType:@"T"];
    CKModifyRecordsOperation *modifyBoth = [[CKModifyRecordsOperation alloc] initWithRecordsToSave:@[record] recordIDsToDelete:@[recordID]];
    CHECK(modifyBoth.recordsToSave.count == 1 && modifyBoth.recordIDsToDelete.count == 1);

    CKQuery *query = [[CKQuery alloc] initWithRecordType:@"T" predicate:[NSPredicate predicateWithValue:YES]];
    CKQueryOperation *queryOperation = [[CKQueryOperation alloc] initWithQuery:query];
    CHECK(queryOperation.query != nil && queryOperation.cursor == nil && queryOperation.zoneID == nil);
    CHECK(queryOperation.resultsLimit == CKQueryOperationMaximumResults && queryOperation.desiredKeys == nil);
    CHECK([[[CKQueryOperation alloc] init] query] == nil);

    CKFetchRecordZonesOperation *allZones = [CKFetchRecordZonesOperation fetchAllRecordZonesOperation];
    CHECK(allZones.recordZoneIDs == nil);
    CKRecordZoneID *zoneID = [CKRecordZone defaultRecordZone].zoneID;
    CHECK([[[CKFetchRecordZonesOperation alloc] initWithRecordZoneIDs:@[zoneID]] recordZoneIDs].count == 1);
    CKModifyRecordZonesOperation *modifyZones = [[CKModifyRecordZonesOperation alloc] initWithRecordZonesToSave:@[[CKRecordZone defaultRecordZone]] recordZoneIDsToDelete:nil];
    CHECK(modifyZones.recordZonesToSave.count == 1 && modifyZones.recordZoneIDsToDelete == nil);

    CKFetchRecordZoneChangesConfiguration *zoneConfiguration = [[CKFetchRecordZoneChangesConfiguration alloc] init];
    CHECK(zoneConfiguration.previousServerChangeToken == nil && zoneConfiguration.resultsLimit == 0 && zoneConfiguration.desiredKeys == nil);
    zoneConfiguration.resultsLimit = 5;
    zoneConfiguration.desiredKeys = @[@"a"];
    CKFetchRecordZoneChangesConfiguration *zoneCopy = [zoneConfiguration copy];
    CHECK(zoneCopy.resultsLimit == 5 && [zoneCopy.desiredKeys isEqual:@[@"a"]]);
    CKFetchRecordZoneChangesOperation *changes = [[CKFetchRecordZoneChangesOperation alloc] initWithRecordZoneIDs:@[zoneID]
                                                                                    configurationsByRecordZoneID:@{zoneID: zoneConfiguration}];
    CHECK(changes.recordZoneIDs.count == 1 && changes.configurationsByRecordZoneID.count == 1 && changes.fetchAllChanges);
    CHECK([[[CKFetchRecordZoneChangesOperation alloc] init] configurationsByRecordZoneID] == nil);

    CKFetchDatabaseChangesOperation *databaseChanges = [[CKFetchDatabaseChangesOperation alloc] initWithPreviousServerChangeToken:nil];
    CHECK(databaseChanges.previousServerChangeToken == nil && databaseChanges.resultsLimit == 0 && databaseChanges.fetchAllChanges);

    CHECK([[CKFetchSubscriptionsOperation fetchAllSubscriptionsOperation] subscriptionIDs] == nil);
    CKModifySubscriptionsOperation *modifySubscriptions = [[CKModifySubscriptionsOperation alloc] init];
    CHECK(modifySubscriptions.subscriptionsToSave == nil && modifySubscriptions.subscriptionIDsToDelete == nil);

    CKFetchShareMetadataOperation *metadata = [[CKFetchShareMetadataOperation alloc] init];
    CHECK(metadata.shareURLs == nil && !metadata.shouldFetchRootRecord && metadata.rootRecordDesiredKeys == nil);
    CHECK([[CKAcceptSharesOperation alloc] init].shareMetadatas == nil);
    CHECK([[CKFetchShareParticipantsOperation alloc] init].userIdentityLookupInfos == nil);
    CHECK([[CKDiscoverUserIdentitiesOperation alloc] init].userIdentityLookupInfos.count == 0);
    CHECK([[CKDiscoverAllUserIdentitiesOperation alloc] init].userIdentityDiscoveredBlock == nil);
}

#ifndef CK_SKIP_NO_ACCOUNT_PATH

// Waits for the dispatch group; returns NO on a timeout.
static BOOL waitFor(dispatch_semaphore_t semaphore)
{
    return dispatch_semaphore_wait(semaphore, dispatch_time(DISPATCH_TIME_NOW, 10 * NSEC_PER_SEC)) == 0;
}

static BOOL isNotAuthenticated(NSError *error)
{
    return error != nil && [error.domain isEqualToString:CKErrorDomain] && error.code == CKErrorNotAuthenticated;
}

static void testContainer(void)
{
    CKContainer *container = [CKContainer containerWithIdentifier:@"iCloud.org.example.nosuchcontainer"];
    CHECK([container.containerIdentifier isEqualToString:@"iCloud.org.example.nosuchcontainer"]);
    CHECK(container == [CKContainer containerWithIdentifier:@"iCloud.org.example.nosuchcontainer"]);
    CHECK(container != [CKContainer containerWithIdentifier:@"iCloud.org.example.other"]);
    CHECK([CKContainer defaultContainer].containerIdentifier.length > 0);
    CHECK([[CKContainer defaultContainer].containerIdentifier hasPrefix:@"iCloud."]);

    CHECK(container.publicCloudDatabase.databaseScope == CKDatabaseScopePublic);
    CHECK(container.privateCloudDatabase.databaseScope == CKDatabaseScopePrivate);
    CHECK(container.sharedCloudDatabase.databaseScope == CKDatabaseScopeShared);
    CHECK([container databaseWithDatabaseScope:CKDatabaseScopePrivate] == container.privateCloudDatabase);
    CHECK([container databaseWithDatabaseScope:CKDatabaseScopeShared] == container.sharedCloudDatabase);
    CHECK(container.publicCloudDatabase != container.privateCloudDatabase);
}

static void testNoAccount(void)
{
    CKContainer *container = [CKContainer containerWithIdentifier:@"iCloud.org.example.nosuchcontainer"];
    CKDatabase *database = container.privateCloudDatabase;
    dispatch_semaphore_t done = dispatch_semaphore_create(0);

    // Without an account the status is reported as such, not as an error.
    __block CKAccountStatus status = CKAccountStatusAvailable;
    __block NSError *statusError = (id)@"unset";
    [container accountStatusWithCompletionHandler:^(CKAccountStatus accountStatus, NSError *error) {
        status = accountStatus;
        statusError = error;
        dispatch_semaphore_signal(done);
    }];
    CHECK(waitFor(done) && status == CKAccountStatusNoAccount && statusError == nil);

    // Every operation completes once, with CKErrorNotAuthenticated and no results.
    CKRecordID *recordID = [[CKRecordID alloc] initWithRecordName:@"a"];
    CKRecord *record = [[CKRecord alloc] initWithRecordType:@"T" recordID:recordID];
    CKRecordZone *zone = [CKRecordZone defaultRecordZone];
    CKQuery *query = [[CKQuery alloc] initWithRecordType:@"T" predicate:[NSPredicate predicateWithValue:YES]];

    __block NSError *error = nil;
    __block id results = (id)@"unset";
    __block int calls = 0;
    void (^reset)(void) = ^{ error = nil; results = (id)@"unset"; calls = 0; };

    CKFetchRecordsOperation *fetch = [[CKFetchRecordsOperation alloc] initWithRecordIDs:@[recordID]];
    fetch.fetchRecordsCompletionBlock = ^(NSDictionary *records, NSError *e) { results = records; error = e; calls++; dispatch_semaphore_signal(done); };
    [database addOperation:fetch];
    CHECK(waitFor(done) && isNotAuthenticated(error) && results == nil && calls == 1);
    CHECK(fetch.database == database);
    CHECK(fetch.configuration.container == container);

    // The operation is finished afterwards and runs its NSOperation completion block too.
    dispatch_semaphore_t finished = dispatch_semaphore_create(0);
    reset();
    CKModifyRecordsOperation *modify = [[CKModifyRecordsOperation alloc] initWithRecordsToSave:@[record] recordIDsToDelete:nil];
    modify.modifyRecordsCompletionBlock = ^(NSArray *saved, NSArray *deleted, NSError *e) { results = saved; error = e; calls++; };
    modify.completionBlock = ^{ dispatch_semaphore_signal(finished); };
    [database addOperation:modify];
    CHECK(waitFor(finished) && isNotAuthenticated(error) && results == nil && calls == 1);
    CHECK(modify.isFinished && !modify.isExecuting);

    reset();
    CKQueryOperation *queryOperation = [[CKQueryOperation alloc] initWithQuery:query];
    __block int fetchedRecords = 0;
    queryOperation.recordFetchedBlock = ^(CKRecord *r) { fetchedRecords++; };
    queryOperation.queryCompletionBlock = ^(CKQueryCursor *cursor, NSError *e) { results = cursor; error = e; calls++; dispatch_semaphore_signal(done); };
    [database addOperation:queryOperation];
    CHECK(waitFor(done) && isNotAuthenticated(error) && results == nil && fetchedRecords == 0 && calls == 1);

    reset();
    CKFetchRecordZonesOperation *zones = [CKFetchRecordZonesOperation fetchAllRecordZonesOperation];
    zones.fetchRecordZonesCompletionBlock = ^(NSDictionary *z, NSError *e) { results = z; error = e; calls++; dispatch_semaphore_signal(done); };
    [database addOperation:zones];
    CHECK(waitFor(done) && isNotAuthenticated(error) && results == nil && calls == 1);

    reset();
    CKModifyRecordZonesOperation *modifyZones = [[CKModifyRecordZonesOperation alloc] initWithRecordZonesToSave:@[zone] recordZoneIDsToDelete:nil];
    modifyZones.modifyRecordZonesCompletionBlock = ^(NSArray *saved, NSArray *deleted, NSError *e) { results = saved; error = e; calls++; dispatch_semaphore_signal(done); };
    [database addOperation:modifyZones];
    CHECK(waitFor(done) && isNotAuthenticated(error) && results == nil && calls == 1);

    reset();
    CKFetchRecordZoneChangesOperation *changes = [[CKFetchRecordZoneChangesOperation alloc] initWithRecordZoneIDs:@[zone.zoneID] configurationsByRecordZoneID:nil];
    changes.fetchRecordZoneChangesCompletionBlock = ^(NSError *e) { error = e; calls++; dispatch_semaphore_signal(done); };
    [database addOperation:changes];
    CHECK(waitFor(done) && isNotAuthenticated(error) && calls == 1);

    reset();
    CKFetchDatabaseChangesOperation *databaseChanges = [[CKFetchDatabaseChangesOperation alloc] initWithPreviousServerChangeToken:nil];
    databaseChanges.fetchDatabaseChangesCompletionBlock = ^(CKServerChangeToken *token, BOOL moreComing, NSError *e) { results = token; error = e; calls++; dispatch_semaphore_signal(done); };
    [database addOperation:databaseChanges];
    CHECK(waitFor(done) && isNotAuthenticated(error) && results == nil && calls == 1);

    reset();
    CKFetchSubscriptionsOperation *fetchSubscriptions = [CKFetchSubscriptionsOperation fetchAllSubscriptionsOperation];
    fetchSubscriptions.fetchSubscriptionCompletionBlock = ^(NSDictionary *s, NSError *e) { results = s; error = e; calls++; dispatch_semaphore_signal(done); };
    [database addOperation:fetchSubscriptions];
    CHECK(waitFor(done) && isNotAuthenticated(error) && results == nil && calls == 1);

    reset();
    CKDatabaseSubscription *subscription = [[CKDatabaseSubscription alloc] initWithSubscriptionID:@"s"];
    CKModifySubscriptionsOperation *modifySubscriptions = [[CKModifySubscriptionsOperation alloc] initWithSubscriptionsToSave:@[subscription] subscriptionIDsToDelete:nil];
    modifySubscriptions.modifySubscriptionsCompletionBlock = ^(NSArray *saved, NSArray *deleted, NSError *e) { results = saved; error = e; calls++; dispatch_semaphore_signal(done); };
    [database addOperation:modifySubscriptions];
    CHECK(waitFor(done) && isNotAuthenticated(error) && results == nil && calls == 1);

    // Operations that are not tied to a database go through the container.
    reset();
    CKFetchShareMetadataOperation *metadata = [[CKFetchShareMetadataOperation alloc] initWithShareURLs:@[[NSURL URLWithString:@"https://example.com/share"]]];
    metadata.fetchShareMetadataCompletionBlock = ^(NSError *e) { error = e; calls++; dispatch_semaphore_signal(done); };
    [container addOperation:metadata];
    CHECK(waitFor(done) && isNotAuthenticated(error) && calls == 1);

    reset();
    CKAcceptSharesOperation *accept = [[CKAcceptSharesOperation alloc] initWithShareMetadatas:@[]];
    accept.acceptSharesCompletionBlock = ^(NSError *e) { error = e; calls++; dispatch_semaphore_signal(done); };
    [container addOperation:accept];
    CHECK(waitFor(done) && isNotAuthenticated(error) && calls == 1);

    reset();
    CKFetchShareParticipantsOperation *participants = [[CKFetchShareParticipantsOperation alloc] initWithUserIdentityLookupInfos:@[[[CKUserIdentityLookupInfo alloc] initWithEmailAddress:@"a@example.com"]]];
    participants.fetchShareParticipantsCompletionBlock = ^(NSError *e) { error = e; calls++; dispatch_semaphore_signal(done); };
    [container addOperation:participants];
    CHECK(waitFor(done) && isNotAuthenticated(error) && calls == 1);

    reset();
    CKDiscoverAllUserIdentitiesOperation *discoverAll = [[CKDiscoverAllUserIdentitiesOperation alloc] init];
    discoverAll.discoverAllUserIdentitiesCompletionBlock = ^(NSError *e) { error = e; calls++; dispatch_semaphore_signal(done); };
    [container addOperation:discoverAll];
    CHECK(waitFor(done) && isNotAuthenticated(error) && calls == 1);

    // An operation that is cancelled before it starts ends with CKErrorOperationCancelled.
    reset();
    CKFetchRecordsOperation *cancelled = [[CKFetchRecordsOperation alloc] initWithRecordIDs:@[recordID]];
    cancelled.fetchRecordsCompletionBlock = ^(NSDictionary *records, NSError *e) { error = e; calls++; dispatch_semaphore_signal(done); };
    [cancelled cancel];
    [cancelled start];
    CHECK(waitFor(done) && error != nil && [error.domain isEqualToString:CKErrorDomain] && error.code == CKErrorOperationCancelled && calls == 1);
}

static void testConvenienceMethods(void)
{
    // The convenience methods of the database and the container run the operations above.
    CKContainer *container = [CKContainer containerWithIdentifier:@"iCloud.org.example.nosuchcontainer"];
    CKDatabase *database = container.publicCloudDatabase;
    dispatch_semaphore_t done = dispatch_semaphore_create(0);
    __block NSError *error = nil;
    __block id result = nil;
    void (^finish)(id, NSError *) = ^(id r, NSError *e) { result = r; error = e; dispatch_semaphore_signal(done); };

    CKRecordID *recordID = [[CKRecordID alloc] initWithRecordName:@"a"];
    CKRecord *record = [[CKRecord alloc] initWithRecordType:@"T" recordID:recordID];
    CKRecordZone *zone = [CKRecordZone defaultRecordZone];
    CKQuery *query = [[CKQuery alloc] initWithRecordType:@"T" predicate:[NSPredicate predicateWithValue:YES]];
    CKQuerySubscription *subscription = [[CKQuerySubscription alloc] initWithRecordType:@"T" predicate:[NSPredicate predicateWithValue:YES]
                                                                                  options:CKQuerySubscriptionOptionsFiresOnRecordCreation];

#define EXPECT_NOT_AUTHENTICATED(call) do { error = nil; result = (id)@"unset"; call; \
    CHECK(waitFor(done) && isNotAuthenticated(error) && result == nil); } while (0)

    EXPECT_NOT_AUTHENTICATED([database fetchRecordWithID:recordID completionHandler:^(CKRecord *r, NSError *e) { finish(r, e); }]);
    EXPECT_NOT_AUTHENTICATED([database saveRecord:record completionHandler:^(CKRecord *r, NSError *e) { finish(r, e); }]);
    EXPECT_NOT_AUTHENTICATED([database deleteRecordWithID:recordID completionHandler:^(CKRecordID *r, NSError *e) { finish(r, e); }]);
    EXPECT_NOT_AUTHENTICATED([database performQuery:query inZoneWithID:nil completionHandler:^(NSArray *r, NSError *e) { finish(r, e); }]);
    EXPECT_NOT_AUTHENTICATED([database fetchRecordZoneWithID:zone.zoneID completionHandler:^(CKRecordZone *r, NSError *e) { finish(r, e); }]);
    EXPECT_NOT_AUTHENTICATED([database saveRecordZone:zone completionHandler:^(CKRecordZone *r, NSError *e) { finish(r, e); }]);
    EXPECT_NOT_AUTHENTICATED([database deleteRecordZoneWithID:zone.zoneID completionHandler:^(CKRecordZoneID *r, NSError *e) { finish(r, e); }]);
    EXPECT_NOT_AUTHENTICATED([database fetchSubscriptionWithID:@"s" completionHandler:^(CKSubscription *r, NSError *e) { finish(r, e); }]);
    EXPECT_NOT_AUTHENTICATED([database saveSubscription:subscription completionHandler:^(CKSubscription *r, NSError *e) { finish(r, e); }]);
    EXPECT_NOT_AUTHENTICATED([database deleteSubscriptionWithID:@"s" completionHandler:^(NSString *r, NSError *e) { finish(r, e); }]);

    EXPECT_NOT_AUTHENTICATED([container fetchUserRecordIDWithCompletionHandler:^(CKRecordID *r, NSError *e) { finish(r, e); }]);
    EXPECT_NOT_AUTHENTICATED([container discoverUserIdentityWithEmailAddress:@"a@example.com" completionHandler:^(CKUserIdentity *r, NSError *e) { finish(r, e); }]);
    EXPECT_NOT_AUTHENTICATED([container fetchShareParticipantWithEmailAddress:@"a@example.com" completionHandler:^(CKShareParticipant *r, NSError *e) { finish(r, e); }]);
    EXPECT_NOT_AUTHENTICATED([container fetchShareMetadataWithURL:[NSURL URLWithString:@"https://example.com/s"] completionHandler:^(CKShareMetadata *r, NSError *e) { finish(r, e); }]);

    // Lists come back empty with the error, not as nil, from the fetch-all conveniences of the database.
    error = nil;
    [database fetchAllRecordZonesWithCompletionHandler:^(NSArray *r, NSError *e) { finish(r, e); }];
    CHECK(waitFor(done) && isNotAuthenticated(error));
    error = nil;
    [database fetchAllSubscriptionsWithCompletionHandler:^(NSArray *r, NSError *e) { finish(r, e); }];
    CHECK(waitFor(done) && isNotAuthenticated(error));

    error = nil;
    [container statusForApplicationPermission:CKApplicationPermissionUserDiscoverability completionHandler:^(CKApplicationPermissionStatus s, NSError *e) { finish(nil, e); }];
    CHECK(waitFor(done) && isNotAuthenticated(error));
}

#endif

int main(void)
{
    @autoreleasepool {
        testEnums();
        testConfigurationAndGroup();
        testOperationDefaults();
#ifndef CK_SKIP_NO_ACCOUNT_PATH
        testContainer();
        testNoAccount();
        testConvenienceMethods();
#endif
        if (failures == 0)
            printf("PASS\n");
    }
    return failures;
}
