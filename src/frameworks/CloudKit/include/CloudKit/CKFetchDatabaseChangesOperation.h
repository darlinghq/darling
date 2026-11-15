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
#import <CloudKit/CKRecordZoneID.h>
#import <CloudKit/CKServerChangeToken.h>

NS_ASSUME_NONNULL_BEGIN

// Fetches which record zones of a database changed since a change token.
@interface CKFetchDatabaseChangesOperation : CKDatabaseOperation {
@private
    CKServerChangeToken *_previousServerChangeToken;
    NSUInteger _resultsLimit;
    BOOL _fetchAllChanges;
    void (^_recordZoneWithIDChangedBlock)(CKRecordZoneID *zoneID);
    void (^_recordZoneWithIDWasDeletedBlock)(CKRecordZoneID *zoneID);
    void (^_recordZoneWithIDWasPurgedBlock)(CKRecordZoneID *zoneID);
    void (^_changeTokenUpdatedBlock)(CKServerChangeToken *newToken);
    void (^_fetchDatabaseChangesCompletionBlock)(CKServerChangeToken * _Nullable serverChangeToken, BOOL moreComing, NSError * _Nullable operationError);
}

- (instancetype)init;
- (instancetype)initWithPreviousServerChangeToken:(nullable CKServerChangeToken *)previousServerChangeToken;

@property (nonatomic, copy, nullable) CKServerChangeToken *previousServerChangeToken;
@property (nonatomic, assign) NSUInteger resultsLimit;
// With YES (the default) the operation keeps fetching until the server has no more changes.
@property (nonatomic, assign) BOOL fetchAllChanges;
@property (nonatomic, copy, nullable) void (^recordZoneWithIDChangedBlock)(CKRecordZoneID *zoneID);
@property (nonatomic, copy, nullable) void (^recordZoneWithIDWasDeletedBlock)(CKRecordZoneID *zoneID);
@property (nonatomic, copy, nullable) void (^recordZoneWithIDWasPurgedBlock)(CKRecordZoneID *zoneID);
@property (nonatomic, copy, nullable) void (^changeTokenUpdatedBlock)(CKServerChangeToken *newToken);
@property (nonatomic, copy, nullable) void (^fetchDatabaseChangesCompletionBlock)(CKServerChangeToken * _Nullable serverChangeToken, BOOL moreComing, NSError * _Nullable operationError);
@end

NS_ASSUME_NONNULL_END
