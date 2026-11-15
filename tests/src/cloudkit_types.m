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

// CloudKit data types: record, record ID, zone and zone ID, reference, asset, query, share, subscriptions, user
// identity lookup. Checks the constants and enum values, value semantics (copy, equality), field validation, and
// NSSecureCoding round trips. Prints PASS when everything holds; the exit code is the number of failed checks.
// Behaviour was observed on macOS 26.7.3 (x86_64); the test runs unchanged there.

#import <Foundation/Foundation.h>
#import <CloudKit/CloudKit.h>

// The tests pass nil to nonnull parameters on purpose.
#pragma clang diagnostic ignored "-Wnonnull"

static int failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)

static id roundTrip(id object)
{
    NSError *error = nil;
    NSData *data = [NSKeyedArchiver archivedDataWithRootObject:object requiringSecureCoding:YES error:&error];
    if (!data)
        return nil;
    return [NSKeyedUnarchiver unarchivedObjectOfClass:[object class] fromData:data error:&error];
}

static BOOL raises(void (^block)(void))
{
    @try {
        block();
    } @catch (NSException *exception) {
        return YES;
    }
    return NO;
}

static NSString *raisedName(void (^block)(void))
{
    @try {
        block();
    } @catch (NSException *exception) {
        return exception.name;
    }
    return nil;
}

static NSSet *keySet(NSArray *keys)
{
    return [NSSet setWithArray:keys];
}

static void testConstants(void)
{
    CHECK([CKRecordZoneDefaultName isEqualToString:@"_defaultZone"]);
    CHECK([CKCurrentUserDefaultName isEqualToString:@"__defaultOwner__"]);
    CHECK([CKRecordTypeUserRecord isEqualToString:@"Users"]);
    CHECK([CKRecordTypeShare isEqualToString:@"cloudkit.share"]);
    CHECK([CKRecordParentKey isEqualToString:@"___parent"]);
    CHECK([CKRecordShareKey isEqualToString:@"___share"]);
    CHECK([CKShareTitleKey isEqualToString:@"cloudkit.title"]);
    CHECK([CKShareThumbnailImageDataKey isEqualToString:@"cloudkit.thumbnailImageData"]);
    CHECK([CKShareTypeKey isEqualToString:@"cloudkit.type"]);

    CHECK(CKReferenceActionNone == 0 && CKReferenceActionDeleteSelf == 1);
    CHECK(CKRecordZoneCapabilityFetchChanges == 1 && CKRecordZoneCapabilityAtomic == 2
          && CKRecordZoneCapabilitySharing == 4 && CKRecordZoneCapabilityZoneWideSharing == 8);
    CHECK(CKSubscriptionTypeQuery == 1 && CKSubscriptionTypeRecordZone == 2 && CKSubscriptionTypeDatabase == 3);
    CHECK(CKNotificationTypeQuery == 1 && CKNotificationTypeRecordZone == 2
          && CKNotificationTypeReadNotification == 3 && CKNotificationTypeDatabase == 4);
    CHECK(CKQueryNotificationReasonRecordCreated == 1 && CKQueryNotificationReasonRecordUpdated == 2
          && CKQueryNotificationReasonRecordDeleted == 3);
    CHECK(CKDatabaseScopePublic == 1 && CKDatabaseScopePrivate == 2 && CKDatabaseScopeShared == 3);
    CHECK(CKQuerySubscriptionOptionsFiresOnRecordCreation == 1 && CKQuerySubscriptionOptionsFiresOnRecordUpdate == 2
          && CKQuerySubscriptionOptionsFiresOnRecordDeletion == 4 && CKQuerySubscriptionOptionsFiresOnce == 8);
    CHECK(CKShareParticipantAcceptanceStatusUnknown == 0 && CKShareParticipantAcceptanceStatusPending == 1
          && CKShareParticipantAcceptanceStatusAccepted == 2 && CKShareParticipantAcceptanceStatusRemoved == 3);
    CHECK(CKShareParticipantPermissionUnknown == 0 && CKShareParticipantPermissionNone == 1
          && CKShareParticipantPermissionReadOnly == 2 && CKShareParticipantPermissionReadWrite == 3);
    CHECK(CKShareParticipantRoleUnknown == 0 && CKShareParticipantRoleOwner == 1
          && CKShareParticipantRolePrivateUser == 3 && CKShareParticipantRolePublicUser == 4);
}

static void testIdentifiers(void)
{
    CKRecordZoneID *zoneID = [[CKRecordZoneID alloc] initWithZoneName:@"Zone" ownerName:@"owner"];
    CHECK([zoneID.zoneName isEqualToString:@"Zone"] && [zoneID.ownerName isEqualToString:@"owner"]);
    CKRecordZoneID *same = [[CKRecordZoneID alloc] initWithZoneName:@"Zone" ownerName:@"owner"];
    CHECK([zoneID isEqual:same] && zoneID.hash == same.hash);
    CHECK(![zoneID isEqual:[[CKRecordZoneID alloc] initWithZoneName:@"Other" ownerName:@"owner"]]);
    CHECK(![zoneID isEqual:[[CKRecordZoneID alloc] initWithZoneName:@"Zone" ownerName:@"other"]]);
    CKRecordZoneID *zoneCopy = [zoneID copy];
    CHECK([zoneCopy isEqual:zoneID]);
    CHECK([roundTrip(zoneID) isEqual:zoneID]);
    CHECK(raises(^{ (void)[[CKRecordZoneID alloc] initWithZoneName:@"" ownerName:@"o"]; }));

    // The default zone belongs to the current user.
    CKRecordZone *defaultZone = [CKRecordZone defaultRecordZone];
    CHECK([defaultZone.zoneID.zoneName isEqualToString:CKRecordZoneDefaultName]);
    CHECK([defaultZone.zoneID.ownerName isEqualToString:CKCurrentUserDefaultName]);

    CKRecordID *recordID = [[CKRecordID alloc] initWithRecordName:@"rec" zoneID:zoneID];
    CHECK([recordID.recordName isEqualToString:@"rec"] && [recordID.zoneID isEqual:zoneID]);
    CKRecordID *plain = [[CKRecordID alloc] initWithRecordName:@"rec"];
    CHECK([plain.zoneID isEqual:defaultZone.zoneID]);
    CHECK([recordID isEqual:[[CKRecordID alloc] initWithRecordName:@"rec" zoneID:zoneID]]);
    CHECK(![recordID isEqual:plain]);
    CHECK([recordID hash] == [[[CKRecordID alloc] initWithRecordName:@"rec" zoneID:zoneID] hash]);
    CHECK([[recordID copy] isEqual:recordID]);
    CHECK([roundTrip(recordID) isEqual:recordID]);
    CHECK(raises(^{ (void)[[CKRecordID alloc] initWithRecordName:@""]; }));
    CHECK(raises(^{ (void)[[CKRecordID alloc] initWithRecordName:[@"" stringByPaddingToLength:300 withString:@"a" startingAtIndex:0]]; }));
    CHECK(!raises(^{ (void)[[CKRecordID alloc] initWithRecordName:[@"" stringByPaddingToLength:255 withString:@"a" startingAtIndex:0]]; }));

    CKRecordZone *zone = [[CKRecordZone alloc] initWithZoneName:@"Zone"];
    CHECK([zone.zoneID.zoneName isEqualToString:@"Zone"]);
    CHECK([zone.zoneID.ownerName isEqualToString:CKCurrentUserDefaultName]);
    CHECK(zone.capabilities == 0 && zone.share == nil);
    CKRecordZone *byID = [[CKRecordZone alloc] initWithZoneID:zoneID];
    CHECK([byID.zoneID isEqual:zoneID]);
    CKRecordZone *zoneCopy2 = [byID copy];
    CHECK(zoneCopy2.zoneID != nil && [zoneCopy2.zoneID isEqual:zoneID]);
    CKRecordZone *zoneRestored = roundTrip(byID);
    CHECK([zoneRestored.zoneID isEqual:zoneID]);
}

static void testRecordValues(void)
{
    CKRecordZoneID *zoneID = [[CKRecordZoneID alloc] initWithZoneName:@"Zone" ownerName:CKCurrentUserDefaultName];
    CKRecordID *recordID = [[CKRecordID alloc] initWithRecordName:@"rec" zoneID:zoneID];
    CKRecord *record = [[CKRecord alloc] initWithRecordType:@"Item" recordID:recordID];
    CHECK([record.recordType isEqualToString:@"Item"] && [record.recordID isEqual:recordID]);
    CHECK(record.allKeys.count == 0 && record.changedKeys.count == 0);
    CHECK(record.recordChangeTag == nil && record.creationDate == nil && record.modificationDate == nil);
    CHECK(record.creatorUserRecordID == nil && record.lastModifiedUserRecordID == nil);

    // Records without an ID get a random name in the default zone.
    CKRecord *anonymous = [[CKRecord alloc] initWithRecordType:@"Item"];
    CKRecord *anonymous2 = [[CKRecord alloc] initWithRecordType:@"Item"];
    CHECK(anonymous.recordID.recordName.length > 0 && ![anonymous.recordID isEqual:anonymous2.recordID]);
    CHECK([anonymous.recordID.zoneID.zoneName isEqualToString:CKRecordZoneDefaultName]);
    CKRecord *inZone = [[CKRecord alloc] initWithRecordType:@"Item" zoneID:zoneID];
    CHECK([inZone.recordID.zoneID isEqual:zoneID]);

    NSDate *date = [NSDate dateWithTimeIntervalSince1970:1000];
    NSData *blob = [NSData dataWithBytes:"ab" length:2];
    CKReference *reference = [[CKReference alloc] initWithRecordID:[[CKRecordID alloc] initWithRecordName:@"p" zoneID:zoneID]
                                                            action:CKReferenceActionDeleteSelf];
    record[@"name"] = @"text";
    record[@"count"] = @42;
    record[@"when"] = date;
    record[@"blob"] = blob;
    record[@"list"] = @[@"x", @"y"];
    record[@"ref"] = reference;
    [record setObject:@"viaMethod" forKey:@"method"];
    CHECK([record[@"name"] isEqual:@"text"] && [record[@"count"] isEqual:@42] && [record[@"when"] isEqual:date]);
    CHECK([record[@"blob"] isEqual:blob] && [record[@"list"] isEqual:(@[@"x", @"y"])] && [record[@"ref"] isEqual:reference]);
    CHECK([[record objectForKey:@"method"] isEqual:@"viaMethod"]);
    CHECK([record[@"missing"] isEqual:nil] || record[@"missing"] == nil);
    NSSet *expectedKeys = [NSSet setWithObjects:@"name", @"count", @"when", @"blob", @"list", @"ref", @"method", nil];
    CHECK([keySet(record.allKeys) isEqual:expectedKeys]);
    CHECK([keySet(record.changedKeys) isEqual:expectedKeys]);

    // Mutable values are copied when stored.
    NSMutableString *mutableString = [@"abc" mutableCopy];
    record[@"copied"] = mutableString;
    [mutableString appendString:@"d"];
    CHECK([record[@"copied"] isEqual:@"abc"]);

    // Removing a field takes it out of the values and counts as a change, even when it was never set.
    record[@"copied"] = nil;
    CHECK(![record.allKeys containsObject:@"copied"]);
    record[@"neverSet"] = nil;
    CHECK([record.changedKeys containsObject:@"neverSet"]);

    // Field validation.
    CHECK(raises(^{ record[@"bad"] = (id)[[NSObject alloc] init]; }));
    CHECK(raises(^{ record[@"bad"] = (id)@{@"a": @1}; }));
    CHECK(raises(^{ record[@"bad"] = @[@"x", @1]; }));
    CHECK(raises(^{ record[@"bad"] = @[@[@1]]; }));
    CHECK(![record.allKeys containsObject:@"bad"]);
    CHECK([raisedName(^{ record[@"a b"] = @1; }) isEqualToString:NSInvalidArgumentException]);
    CHECK(raises(^{ record[@"1a"] = @1; }));
    CHECK(raises(^{ record[@"a-b"] = @1; }));
    CHECK(raises(^{ record[@""] = @1; }));
    CHECK(raises(^{ record[[@"" stringByPaddingToLength:300 withString:@"a" startingAtIndex:0]] = @1; }));
    CHECK(!raises(^{ record[@"valid_key_2"] = @1; }));
    CHECK(!raises(^{ record[@"emptyList"] = @[]; }));

    // Copies are independent of the original.
    CKRecord *copy = [record copy];
    CHECK([copy.recordID isEqual:record.recordID] && [copy.recordType isEqualToString:record.recordType]);
    copy[@"name"] = @"changed";
    CHECK([record[@"name"] isEqual:@"text"] && [copy[@"name"] isEqual:@"changed"]);
    CHECK([keySet(copy.allKeys) isEqual:keySet(record.allKeys)]);

    // Full archiving keeps the fields; archiving only the system fields keeps none.
    CKRecord *restored = roundTrip(record);
    CHECK(restored != nil && [restored.recordType isEqualToString:@"Item"] && [restored.recordID isEqual:recordID]);
    CHECK([restored[@"name"] isEqual:@"text"] && [restored[@"count"] isEqual:@42] && [restored[@"when"] isEqual:date]);
    CHECK([restored[@"blob"] isEqual:blob] && [restored[@"list"] isEqual:(@[@"x", @"y"])] && [restored[@"ref"] isEqual:reference]);
    CHECK([keySet(restored.allKeys) isEqual:keySet(record.allKeys)]);

    NSKeyedArchiver *archiver = [[NSKeyedArchiver alloc] initRequiringSecureCoding:YES];
    [record encodeSystemFieldsWithCoder:archiver];
    [archiver finishEncoding];
    NSKeyedUnarchiver *unarchiver = [[NSKeyedUnarchiver alloc] initForReadingFromData:archiver.encodedData error:NULL];
    CKRecord *systemOnly = [[CKRecord alloc] initWithCoder:unarchiver];
    CHECK(systemOnly != nil && [systemOnly.recordID isEqual:recordID] && [systemOnly.recordType isEqualToString:@"Item"]);
    CHECK(systemOnly.allKeys.count == 0 && systemOnly.changedKeys.count == 0);

    // The parent must be in the record's own zone.
    CKRecord *parent = [[CKRecord alloc] initWithRecordType:@"Parent" zoneID:zoneID];
    CHECK(record.parent == nil);
    [record setParentReferenceFromRecord:parent];
    CHECK([record.parent.recordID isEqual:parent.recordID] && record.parent.referenceAction == CKReferenceActionNone);
    [record setParentReferenceFromRecord:nil];
    CHECK(record.parent == nil);
    [record setParentReferenceFromRecordID:parent.recordID];
    CHECK([record.parent.recordID isEqual:parent.recordID]);
    CKRecord *foreign = [[CKRecord alloc] initWithRecordType:@"Parent"];
    CHECK(raises(^{ [record setParentReferenceFromRecord:foreign]; }));
    CHECK(record.share == nil);

    CHECK(raises(^{ (void)[[CKRecord alloc] initWithRecordType:@"" recordID:recordID]; }));
}

static void testReferenceAndAsset(void)
{
    CKRecordID *recordID = [[CKRecordID alloc] initWithRecordName:@"target"];
    CKReference *byID = [[CKReference alloc] initWithRecordID:recordID action:CKReferenceActionDeleteSelf];
    CHECK([byID.recordID isEqual:recordID] && byID.referenceAction == CKReferenceActionDeleteSelf);
    CKRecord *record = [[CKRecord alloc] initWithRecordType:@"T" recordID:recordID];
    CKReference *byRecord = [[CKReference alloc] initWithRecord:record action:CKReferenceActionNone];
    CHECK([byRecord.recordID isEqual:recordID] && byRecord.referenceAction == CKReferenceActionNone);
    CHECK(![byID isEqual:byRecord]);
    CHECK([[byID copy] isEqual:byID]);
    CKReference *restored = roundTrip(byID);
    CHECK([restored isEqual:byID] && restored.referenceAction == CKReferenceActionDeleteSelf);
    CHECK(raises(^{ (void)[[CKReference alloc] initWithRecordID:(id)nil action:CKReferenceActionNone]; }));

    NSURL *url = [NSURL fileURLWithPath:@"/tmp/cloudkit-types-test-file"];
    CKAsset *asset = [[CKAsset alloc] initWithFileURL:url];
    CHECK([asset.fileURL isEqual:url]);
    CKAsset *restoredAsset = roundTrip(asset);
    CHECK([restoredAsset.fileURL isEqual:url]);
    CHECK(raises(^{ (void)[[CKAsset alloc] initWithFileURL:(id)nil]; }));
    CHECK(raises(^{ (void)[[CKAsset alloc] initWithFileURL:[NSURL URLWithString:@"http://example.com/x"]]; }));

    // References and assets are record values, alone and inside arrays.
    CKRecord *holder = [[CKRecord alloc] initWithRecordType:@"Holder"];
    holder[@"ref"] = byID;
    holder[@"asset"] = asset;
    holder[@"refs"] = @[byID, byRecord];
    CHECK([holder[@"ref"] isEqual:byID] && [((CKAsset *)holder[@"asset"]).fileURL isEqual:url]);
    CHECK(((NSArray *)holder[@"refs"]).count == 2);
}

static void testQuery(void)
{
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"name == %@", @"x"];
    CKQuery *query = [[CKQuery alloc] initWithRecordType:@"Item" predicate:predicate];
    CHECK([query.recordType isEqualToString:@"Item"] && [query.predicate isEqual:predicate]);
    CHECK(query.sortDescriptors == nil);
    query.sortDescriptors = @[[NSSortDescriptor sortDescriptorWithKey:@"name" ascending:YES]];
    CHECK(query.sortDescriptors.count == 1);
    CKQuery *copy = [query copy];
    CHECK([copy.recordType isEqualToString:@"Item"] && [copy.predicate isEqual:predicate] && copy.sortDescriptors.count == 1);
    CKQuery *restored = roundTrip(query);
    CHECK([restored.recordType isEqualToString:@"Item"] && [restored.predicate isEqual:predicate]);
    CHECK(restored.sortDescriptors.count == 1 && [restored.sortDescriptors[0].key isEqualToString:@"name"]);
    CHECK(raises(^{ (void)[[CKQuery alloc] initWithRecordType:@"Item" predicate:(id)nil]; }));
    CKQuery *everything = [[CKQuery alloc] initWithRecordType:@"Item" predicate:[NSPredicate predicateWithValue:YES]];
    CHECK(everything != nil);

    // Cursors and change tokens come from the server and cannot be created by an application.
    CHECK([CKQueryCursor conformsToProtocol:@protocol(NSSecureCoding)] && [CKQueryCursor supportsSecureCoding]);
    CHECK([CKServerChangeToken conformsToProtocol:@protocol(NSSecureCoding)] && [CKServerChangeToken supportsSecureCoding]);
    CHECK([CKServerChangeToken conformsToProtocol:@protocol(NSCopying)] && [CKQueryCursor conformsToProtocol:@protocol(NSCopying)]);
}

static void testShare(void)
{
    CKRecordZoneID *zoneID = [[CKRecordZoneID alloc] initWithZoneName:@"Zone" ownerName:CKCurrentUserDefaultName];
    CKRecord *root = [[CKRecord alloc] initWithRecordType:@"Root" recordID:[[CKRecordID alloc] initWithRecordName:@"root" zoneID:zoneID]];
    CKShare *share = [[CKShare alloc] initWithRootRecord:root];
    CHECK([share.recordType isEqualToString:CKRecordTypeShare]);
    CHECK([share isKindOfClass:[CKRecord class]]);
    CHECK([share.recordID.zoneID isEqual:zoneID] && share.recordID.recordName.length > 0);
    CHECK(share.publicPermission == CKShareParticipantPermissionNone);
    CHECK(share.URL == nil);
    CHECK(share.allKeys.count == 0);

    // The owner is a participant from the start.
    CHECK(share.participants.count == 1 && share.participants[0] == share.owner);
    CKShareParticipant *owner = share.owner;
    CHECK(owner.role == CKShareParticipantRoleOwner && owner.permission == CKShareParticipantPermissionReadWrite);
    CHECK(owner.acceptanceStatus == CKShareParticipantAcceptanceStatusAccepted);
    CHECK(owner.userIdentity != nil && owner.userIdentity.hasiCloudAccount);
    CHECK([owner.userIdentity.userRecordID.recordName isEqualToString:CKCurrentUserDefaultName]);
    CHECK(share.currentUserParticipant != nil);

    share.publicPermission = CKShareParticipantPermissionReadOnly;
    CHECK(share.publicPermission == CKShareParticipantPermissionReadOnly);
    share[CKShareTitleKey] = @"Title";
    CHECK([share[CKShareTitleKey] isEqual:@"Title"]);

    CHECK(raises(^{ [share addParticipant:share.owner]; }));
    CHECK(raises(^{ [share removeParticipant:share.owner]; }));

    CKRecordID *shareID = [[CKRecordID alloc] initWithRecordName:@"mine" zoneID:zoneID];
    CKShare *named = [[CKShare alloc] initWithRootRecord:root shareID:shareID];
    CHECK([named.recordID isEqual:shareID]);
    CHECK(raises(^{ (void)[[CKShare alloc] initWithRootRecord:root shareID:[[CKRecordID alloc] initWithRecordName:@"x"]]; }));
    CKShare *zoneShare = [[CKShare alloc] initWithRecordZoneID:zoneID];
    CHECK([zoneShare.recordID.recordName isEqualToString:@"cloudkit.zoneshare"] && [zoneShare.recordID.zoneID isEqual:zoneID]);

    CKShare *copy = [share copy];
    CHECK([copy isKindOfClass:[CKShare class]] && [copy.recordID isEqual:share.recordID]);
    CHECK(copy.publicPermission == CKShareParticipantPermissionReadOnly && copy.participants.count == 1);
    CKShare *restored = roundTrip(share);
    CHECK(restored != nil && [restored.recordID isEqual:share.recordID]);
    CHECK(restored.publicPermission == CKShareParticipantPermissionReadOnly && restored.participants.count == 1);
    CHECK(restored.owner.role == CKShareParticipantRoleOwner);
    CHECK([restored[CKShareTitleKey] isEqual:@"Title"]);
}

static void testSubscriptions(void)
{
    NSPredicate *predicate = [NSPredicate predicateWithValue:YES];
    CKQuerySubscription *query = [[CKQuerySubscription alloc] initWithRecordType:@"Item" predicate:predicate
                                                                         options:CKQuerySubscriptionOptionsFiresOnRecordCreation];
    CHECK(query.subscriptionID.length == 36);
    CHECK(query.subscriptionType == CKSubscriptionTypeQuery);
    CHECK([query.recordType isEqualToString:@"Item"] && [query.predicate isEqual:predicate]);
    CHECK(query.querySubscriptionOptions == CKQuerySubscriptionOptionsFiresOnRecordCreation);
    CHECK(query.notificationInfo == nil);
    CHECK(raises(^{ (void)[[CKQuerySubscription alloc] initWithRecordType:@"Item" predicate:predicate options:0]; }));
    CHECK(raises(^{ (void)[[CKQuerySubscription alloc] initWithRecordType:@"Item" predicate:predicate
                                                                  options:CKQuerySubscriptionOptionsFiresOnce]; }));

    CKNotificationInfo *info = [[CKNotificationInfo alloc] init];
    CHECK(info.alertBody == nil && info.desiredKeys == nil && info.alertLocalizationArgs == nil);
    CHECK(!info.shouldBadge && !info.shouldSendContentAvailable && !info.shouldSendMutableContent);
    info.alertBody = @"hello";
    info.title = @"Title";
    info.soundName = @"sound";
    info.category = @"cat";
    info.collapseIDKey = @"collapse";
    info.desiredKeys = @[@"a", @"b"];
    info.shouldBadge = YES;
    info.shouldSendMutableContent = YES;
    CKNotificationInfo *infoCopy = [info copy];
    CHECK([infoCopy.alertBody isEqual:@"hello"] && [infoCopy.title isEqual:@"Title"] && [infoCopy.soundName isEqual:@"sound"]);
    CHECK([infoCopy.category isEqual:@"cat"] && [infoCopy.collapseIDKey isEqual:@"collapse"]);
    CHECK([infoCopy.desiredKeys isEqual:(@[@"a", @"b"])] && infoCopy.shouldBadge && infoCopy.shouldSendMutableContent);
    CHECK(!infoCopy.shouldSendContentAvailable);

    CKQuerySubscription *named = [[CKQuerySubscription alloc] initWithRecordType:@"Item" predicate:predicate subscriptionID:@"sub1"
                                                                         options:CKQuerySubscriptionOptionsFiresOnRecordUpdate | CKQuerySubscriptionOptionsFiresOnRecordDeletion];
    CHECK([named.subscriptionID isEqual:@"sub1"]);
    named.notificationInfo = info;
    CHECK([named.notificationInfo.alertBody isEqual:@"hello"]);
    info.alertBody = @"changed later";
    CHECK([named.notificationInfo.alertBody isEqual:@"hello"]);
    CKQuerySubscription *namedCopy = [named copy];
    CHECK([namedCopy.subscriptionID isEqual:@"sub1"] && [namedCopy isEqual:named] && namedCopy.notificationInfo != nil);
    CKQuerySubscription *restored = roundTrip(named);
    CHECK([restored.subscriptionID isEqual:@"sub1"] && [restored.recordType isEqual:@"Item"]);
    CHECK(restored.querySubscriptionOptions == named.querySubscriptionOptions);
    CHECK([restored.notificationInfo.alertBody isEqual:@"hello"] && [restored.notificationInfo.desiredKeys isEqual:(@[@"a", @"b"])]);
    CHECK(restored.notificationInfo.shouldBadge);

    CKRecordZoneID *zoneID = [[CKRecordZoneID alloc] initWithZoneName:@"Zone" ownerName:CKCurrentUserDefaultName];
    CKRecordZoneSubscription *zone = [[CKRecordZoneSubscription alloc] initWithZoneID:zoneID];
    CHECK(zone.subscriptionID.length == 36 && zone.subscriptionType == CKSubscriptionTypeRecordZone);
    CHECK([zone.zoneID isEqual:zoneID] && zone.recordType == nil);
    zone.recordType = @"Item";
    CKRecordZoneSubscription *zoneRestored = roundTrip(zone);
    CHECK([zoneRestored.zoneID isEqual:zoneID] && [zoneRestored.recordType isEqual:@"Item"]);
    CKRecordZoneSubscription *zoneNamed = [[CKRecordZoneSubscription alloc] initWithZoneID:zoneID subscriptionID:@"zs"];
    CHECK([zoneNamed.subscriptionID isEqual:@"zs"]);

    CKDatabaseSubscription *database = [[CKDatabaseSubscription alloc] initWithSubscriptionID:@"ds"];
    CHECK([database.subscriptionID isEqual:@"ds"] && database.subscriptionType == CKSubscriptionTypeDatabase);
    CKDatabaseSubscription *generated = [[CKDatabaseSubscription alloc] init];
    CHECK(generated.subscriptionID.length == 36);
    CKDatabaseSubscription *databaseRestored = roundTrip(database);
    CHECK([databaseRestored.subscriptionID isEqual:@"ds"]);
    CHECK(![database isEqual:zoneNamed]);

    // Notifications are made by the system from push payloads; an empty payload is not a CloudKit notification.
    CHECK([CKNotification notificationFromRemoteNotificationDictionary:@{}] == nil);
}

static void testUserIdentityLookup(void)
{
    CKUserIdentityLookupInfo *email = [[CKUserIdentityLookupInfo alloc] initWithEmailAddress:@"a@example.com"];
    CHECK([email.emailAddress isEqual:@"a@example.com"] && email.phoneNumber == nil && email.userRecordID == nil);
    CKUserIdentityLookupInfo *phone = [[CKUserIdentityLookupInfo alloc] initWithPhoneNumber:@"+100"];
    CHECK([phone.phoneNumber isEqual:@"+100"] && phone.emailAddress == nil);
    CKRecordID *recordID = [[CKRecordID alloc] initWithRecordName:@"user"];
    CKUserIdentityLookupInfo *user = [[CKUserIdentityLookupInfo alloc] initWithUserRecordID:recordID];
    CHECK([user.userRecordID isEqual:recordID]);
    CHECK([email isEqual:[[CKUserIdentityLookupInfo alloc] initWithEmailAddress:@"a@example.com"]]);
    CHECK(![email isEqual:phone]);
    CHECK([[email copy] isEqual:email] && [roundTrip(email) isEqual:email] && [roundTrip(phone) isEqual:phone] && [roundTrip(user) isEqual:user]);
    NSArray *emails = @[@"a@example.com", @"b@example.com"];
    CHECK([CKUserIdentityLookupInfo lookupInfosWithEmails:emails].count == 2);
    CHECK([CKUserIdentityLookupInfo lookupInfosWithPhoneNumbers:@[@"+100"]].count == 1);
    NSArray<CKUserIdentityLookupInfo *> *infos = [CKUserIdentityLookupInfo lookupInfosWithRecordIDs:@[recordID]];
    CHECK([infos[0] isEqual:user]);
}

int main(void)
{
    @autoreleasepool {
        testConstants();
        testIdentifiers();
        testRecordValues();
        testReferenceAndAsset();
        testQuery();
        testShare();
        testSubscriptions();
        testUserIdentityLookup();
        if (failures == 0)
            printf("PASS\n");
    }
    return failures;
}
