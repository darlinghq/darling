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
#import <CloudKit/CKRecord.h>
#import <CloudKit/CKRecordID.h>
#import <CloudKit/CKRecordZoneID.h>
#import <CloudKit/CKServerChangeToken.h>
#import <CloudKit/CKFetchRecordZoneChangesConfiguration.h>
#import <CloudKit/CKFetchRecordZoneChangesOptions.h>

NS_ASSUME_NONNULL_BEGIN

// Fetches the changes made to the records of one or more zones since a change token.
@interface CKFetchRecordZoneChangesOperation : CKDatabaseOperation {
@private
    NSArray<CKRecordZoneID *> *_recordZoneIDs;
    NSDictionary<CKRecordZoneID *, CKFetchRecordZoneChangesConfiguration *> *_configurationsByRecordZoneID;
    NSDictionary<CKRecordZoneID *, CKFetchRecordZoneChangesOptions *> *_optionsByRecordZoneID;
    BOOL _fetchAllChanges;
    void (^_recordChangedBlock)(CKRecord *record);
    void (^_recordWithIDWasDeletedBlock)(CKRecordID *recordID, CKRecordType recordType);
    void (^_recordZoneChangeTokensUpdatedBlock)(CKRecordZoneID *recordZoneID, CKServerChangeToken * _Nullable serverChangeToken, NSData * _Nullable clientChangeTokenData);
    void (^_recordZoneFetchCompletionBlock)(CKRecordZoneID *recordZoneID, CKServerChangeToken * _Nullable serverChangeToken, NSData * _Nullable clientChangeTokenData, BOOL moreComing, NSError * _Nullable recordZoneError);
    void (^_fetchRecordZoneChangesCompletionBlock)(NSError * _Nullable operationError);
}

- (instancetype)init;
- (instancetype)initWithRecordZoneIDs:(NSArray<CKRecordZoneID *> *)recordZoneIDs configurationsByRecordZoneID:(nullable NSDictionary<CKRecordZoneID *, CKFetchRecordZoneChangesConfiguration *> *)configurationsByRecordZoneID;
- (instancetype)initWithRecordZoneIDs:(NSArray<CKRecordZoneID *> *)recordZoneIDs optionsByRecordZoneID:(nullable NSDictionary<CKRecordZoneID *, CKFetchRecordZoneChangesOptions *> *)optionsByRecordZoneID API_DEPRECATED_WITH_REPLACEMENT("initWithRecordZoneIDs:configurationsByRecordZoneID:", macos(10.12, 10.14));

@property (nonatomic, copy, nullable) NSArray<CKRecordZoneID *> *recordZoneIDs;
@property (nonatomic, copy, nullable) NSDictionary<CKRecordZoneID *, CKFetchRecordZoneChangesConfiguration *> *configurationsByRecordZoneID;
@property (nonatomic, copy, nullable) NSDictionary<CKRecordZoneID *, CKFetchRecordZoneChangesOptions *> *optionsByRecordZoneID API_DEPRECATED_WITH_REPLACEMENT("configurationsByRecordZoneID", macos(10.12, 10.14));
// With YES (the default) the operation keeps fetching until the server has no more changes.
@property (nonatomic, assign) BOOL fetchAllChanges;
@property (nonatomic, copy, nullable) void (^recordChangedBlock)(CKRecord *record);
@property (nonatomic, copy, nullable) void (^recordWithIDWasDeletedBlock)(CKRecordID *recordID, CKRecordType recordType) NS_REFINED_FOR_SWIFT;
@property (nonatomic, copy, nullable) void (^recordZoneChangeTokensUpdatedBlock)(CKRecordZoneID *recordZoneID, CKServerChangeToken * _Nullable serverChangeToken, NSData * _Nullable clientChangeTokenData);
@property (nonatomic, copy, nullable) void (^recordZoneFetchCompletionBlock)(CKRecordZoneID *recordZoneID, CKServerChangeToken * _Nullable serverChangeToken, NSData * _Nullable clientChangeTokenData, BOOL moreComing, NSError * _Nullable recordZoneError);
@property (nonatomic, copy, nullable) void (^fetchRecordZoneChangesCompletionBlock)(NSError * _Nullable operationError);
@end

NS_ASSUME_NONNULL_END
