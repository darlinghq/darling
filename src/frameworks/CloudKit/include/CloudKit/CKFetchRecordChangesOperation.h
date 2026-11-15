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

NS_ASSUME_NONNULL_BEGIN

// Fetches the changes of one zone since a change token. Superseded by CKFetchRecordZoneChangesOperation.
@interface CKFetchRecordChangesOperation : CKDatabaseOperation {
@private
    CKRecordZoneID *_recordZoneID;
    CKServerChangeToken *_previousServerChangeToken;
    NSUInteger _resultsLimit;
    NSArray<CKRecordFieldKey> *_desiredKeys;
    BOOL _moreComing;
    void (^_recordChangedBlock)(CKRecord *record);
    void (^_recordWithIDWasDeletedBlock)(CKRecordID *recordID);
    void (^_fetchRecordChangesCompletionBlock)(CKServerChangeToken * _Nullable serverChangeToken, NSData * _Nullable clientChangeTokenData, NSError * _Nullable operationError);
}

- (instancetype)initWithRecordZoneID:(CKRecordZoneID *)recordZoneID previousServerChangeToken:(nullable CKServerChangeToken *)previousServerChangeToken;

@property (nonatomic, copy, nullable) CKRecordZoneID *recordZoneID;
@property (nonatomic, copy, nullable) CKServerChangeToken *previousServerChangeToken;
@property (nonatomic, assign) NSUInteger resultsLimit;
@property (nonatomic, copy, nullable) NSArray<CKRecordFieldKey> *desiredKeys;
@property (nonatomic, assign) BOOL moreComing;
@property (nonatomic, copy, nullable) void (^recordChangedBlock)(CKRecord *record);
@property (nonatomic, copy, nullable) void (^recordWithIDWasDeletedBlock)(CKRecordID *recordID);
@property (nonatomic, copy, nullable) void (^fetchRecordChangesCompletionBlock)(CKServerChangeToken * _Nullable serverChangeToken, NSData * _Nullable clientChangeTokenData, NSError * _Nullable operationError);
@end

NS_ASSUME_NONNULL_END
