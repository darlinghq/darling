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

// Declarations shared by the implementation files of the public CloudKit data types. Not installed.

#import <Foundation/Foundation.h>
#import <CloudKit/CKRecord.h>
#import <CloudKit/CKRecordID.h>
#import <CloudKit/CKRecordZoneID.h>
#import <CloudKit/CKRecordZone.h>
#import <CloudKit/CKReference.h>
#import <CloudKit/CKAsset.h>
#import <CloudKit/CKQuery.h>
#import <CloudKit/CKQueryCursor.h>
#import <CloudKit/CKServerChangeToken.h>
#import <CloudKit/CKShare.h>
#import <CloudKit/CKShareParticipant.h>
#import <CloudKit/CKShareMetadata.h>
#import <CloudKit/CKUserIdentity.h>
#import <CloudKit/CKUserIdentityLookupInfo.h>
#import <CloudKit/CKSubscription.h>
#import <CloudKit/CKQuerySubscription.h>
#import <CloudKit/CKRecordZoneSubscription.h>
#import <CloudKit/CKDatabaseSubscription.h>
#import <CloudKit/CKNotificationInfo.h>

// Name of the exceptions raised for invalid arguments the documentation calls out (names, keys, values).
static inline void CKRaiseInvalid(NSString *name, NSString *reason)
{
    @throw [NSException exceptionWithName:name reason:reason userInfo:nil];
}

// Maximum length of record names and zone names.
#define CKMaxNameLength 255

// Raises when the name is nil, empty or too long. what is "recordName" or "zoneName".
void CKValidateName(NSString *name, NSString *what);

@interface CKServerChangeToken ()
- (instancetype)_initWithData:(NSData *)data;
@property (nonatomic, readonly, copy) NSData *_data;
@end

@interface CKQueryCursor ()
- (instancetype)_initWithData:(NSData *)data;
@property (nonatomic, readonly, copy) NSData *_data;
@end

@interface CKUserIdentity ()
- (instancetype)_initWithUserRecordID:(CKRecordID *)userRecordID lookupInfo:(CKUserIdentityLookupInfo *)lookupInfo nameComponents:(NSPersonNameComponents *)nameComponents hasiCloudAccount:(BOOL)hasiCloudAccount;
@end

@interface CKShareParticipant ()
- (instancetype)_initWithUserIdentity:(CKUserIdentity *)identity permission:(CKShareParticipantPermission)permission acceptanceStatus:(CKShareParticipantAcceptanceStatus)status role:(CKShareParticipantRole)role;
@end

@interface CKRecord ()
// Sets the fields the server assigns.
- (void)_setRecordChangeTag:(NSString *)tag creatorUserRecordID:(CKRecordID *)creator creationDate:(NSDate *)creationDate lastModifiedUserRecordID:(CKRecordID *)modifier modificationDate:(NSDate *)modificationDate;
// Clears the list of changed keys, as after a save.
- (void)_clearChangedKeys;
@end

@interface CKSubscription ()
- (instancetype)_initWithSubscriptionID:(CKSubscriptionID)subscriptionID;
@end
