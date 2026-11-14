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
#import <CloudKit/CKDatabase.h>

NS_ASSUME_NONNULL_BEGIN

@class CKOperation;
@class CKRecordID;
@class CKShare;
@class CKShareMetadata;
@class CKShareParticipant;
@class CKUserIdentity;

typedef NS_ENUM(NSInteger, CKAccountStatus) {
    CKAccountStatusCouldNotDetermine = 0,
    CKAccountStatusAvailable = 1,
    CKAccountStatusRestricted = 2,
    CKAccountStatusNoAccount = 3,
    CKAccountStatusTemporarilyUnavailable = 4,
};

typedef NS_OPTIONS(NSUInteger, CKApplicationPermissions) {
    CKApplicationPermissionUserDiscoverability = 1 << 0,
} NS_SWIFT_NAME(CKContainer_Application_Permissions);

typedef NS_ENUM(NSInteger, CKApplicationPermissionStatus) {
    CKApplicationPermissionStatusInitialState = 0,
    CKApplicationPermissionStatusCouldNotComplete = 1,
    CKApplicationPermissionStatusDenied = 2,
    CKApplicationPermissionStatusGranted = 3,
} NS_SWIFT_NAME(CKContainer_Application_PermissionStatus);

typedef void (^CKApplicationPermissionBlock)(CKApplicationPermissionStatus applicationPermissionStatus, NSError * _Nullable error) NS_SWIFT_NAME(CKContainer_Application_PermissionBlock);

// Posted when the iCloud account of the user changes.
extern NSNotificationName const CKAccountChangedNotification;

// The entry point to the data of an app in iCloud.
@interface CKContainer : NSObject {
@private
    NSString *_containerIdentifier;
    CKDatabase *_publicCloudDatabase;
    CKDatabase *_privateCloudDatabase;
    CKDatabase *_sharedCloudDatabase;
    NSOperationQueue *_operationQueue;
}

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

// The container named after the bundle identifier of the app, "iCloud." followed by the identifier.
+ (CKContainer *)defaultContainer;
+ (CKContainer *)containerWithIdentifier:(NSString *)containerIdentifier;

@property (nonatomic, readonly, copy) NSString *containerIdentifier;

@property (nonatomic, readonly, strong) CKDatabase *publicCloudDatabase;
@property (nonatomic, readonly, strong) CKDatabase *privateCloudDatabase;
@property (nonatomic, readonly, strong) CKDatabase *sharedCloudDatabase;
- (CKDatabase *)databaseWithDatabaseScope:(CKDatabaseScope)databaseScope;

// Runs an operation that is not tied to a database.
- (void)addOperation:(CKOperation *)operation;

// Reports CKAccountStatusNoAccount when there is no iCloud account, which is always the case here.
- (void)accountStatusWithCompletionHandler:(void (^)(CKAccountStatus accountStatus, NSError * _Nullable error))completionHandler;
- (void)statusForApplicationPermission:(CKApplicationPermissions)applicationPermission completionHandler:(CKApplicationPermissionBlock)completionHandler NS_SWIFT_NAME(status(forApplicationPermission:completionHandler:));
- (void)requestApplicationPermission:(CKApplicationPermissions)applicationPermission completionHandler:(CKApplicationPermissionBlock)completionHandler;

- (void)fetchUserRecordIDWithCompletionHandler:(void (^)(CKRecordID * _Nullable recordID, NSError * _Nullable error))completionHandler;
- (void)discoverAllIdentitiesWithCompletionHandler:(void (^)(NSArray<CKUserIdentity *> * _Nullable userIdentities, NSError * _Nullable error))completionHandler;
- (void)discoverUserIdentityWithEmailAddress:(NSString *)email completionHandler:(void (^)(CKUserIdentity * _Nullable userInfo, NSError * _Nullable error))completionHandler;
- (void)discoverUserIdentityWithPhoneNumber:(NSString *)phoneNumber completionHandler:(void (^)(CKUserIdentity * _Nullable userInfo, NSError * _Nullable error))completionHandler;
- (void)discoverUserIdentityWithUserRecordID:(CKRecordID *)userRecordID completionHandler:(void (^)(CKUserIdentity * _Nullable userInfo, NSError * _Nullable error))completionHandler;

- (void)fetchShareParticipantWithEmailAddress:(NSString *)emailAddress completionHandler:(void (^)(CKShareParticipant * _Nullable shareParticipant, NSError * _Nullable error))completionHandler;
- (void)fetchShareParticipantWithPhoneNumber:(NSString *)phoneNumber completionHandler:(void (^)(CKShareParticipant * _Nullable shareParticipant, NSError * _Nullable error))completionHandler;
- (void)fetchShareParticipantWithUserRecordID:(CKRecordID *)userRecordID completionHandler:(void (^)(CKShareParticipant * _Nullable shareParticipant, NSError * _Nullable error))completionHandler;
- (void)fetchShareMetadataWithURL:(NSURL *)url completionHandler:(void (^)(CKShareMetadata * _Nullable metadata, NSError * _Nullable error))completionHandler;
- (void)acceptShareMetadata:(CKShareMetadata *)metadata completionHandler:(void (^)(CKShare * _Nullable acceptedShare, NSError * _Nullable error))completionHandler;

// Operations marked long lived outlive the process; none are ever persisted here.
- (void)fetchAllLongLivedOperationIDsWithCompletionHandler:(void (^)(NSArray<CKOperationID> * _Nullable outstandingOperationIDs, NSError * _Nullable error))completionHandler NS_REFINED_FOR_SWIFT;
- (void)fetchLongLivedOperationWithID:(CKOperationID)operationID completionHandler:(void (^)(CKOperation * _Nullable outstandingOperation, NSError * _Nullable error))completionHandler NS_REFINED_FOR_SWIFT;

@end

NS_ASSUME_NONNULL_END
