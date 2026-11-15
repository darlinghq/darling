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

NS_ASSUME_NONNULL_BEGIN

// Fetches records by ID.
@interface CKFetchRecordsOperation : CKDatabaseOperation {
@private
    NSArray<CKRecordID *> *_recordIDs;
    NSArray<CKRecordFieldKey> *_desiredKeys;
    void (^_perRecordProgressBlock)(CKRecordID *recordID, double progress);
    void (^_perRecordCompletionBlock)(CKRecord * _Nullable record, CKRecordID * _Nullable recordID, NSError * _Nullable error);
    void (^_fetchRecordsCompletionBlock)(NSDictionary<CKRecordID *, CKRecord *> * _Nullable recordsByRecordID, NSError * _Nullable operationError);
}

- (instancetype)initWithRecordIDs:(NSArray<CKRecordID *> *)recordIDs;
// An operation that fetches the record of the current user.
+ (instancetype)fetchCurrentUserRecordOperation NS_SWIFT_NAME(fetchCurrentUserRecordOperation());

@property (nonatomic, copy, nullable) NSArray<CKRecordID *> *recordIDs;
// Names of the fields to fetch; nil fetches all, an empty array fetches none.
@property (nonatomic, copy, nullable) NSArray<CKRecordFieldKey> *desiredKeys NS_REFINED_FOR_SWIFT;
@property (nonatomic, copy, nullable) void (^perRecordProgressBlock)(CKRecordID *recordID, double progress);
@property (nonatomic, copy, nullable) void (^perRecordCompletionBlock)(CKRecord * _Nullable record, CKRecordID * _Nullable recordID, NSError * _Nullable error);
// Called once at the end; the dictionary maps the IDs to the records that were fetched.
@property (nonatomic, copy, nullable) void (^fetchRecordsCompletionBlock)(NSDictionary<CKRecordID *, CKRecord *> * _Nullable recordsByRecordID, NSError * _Nullable operationError);
@end

NS_ASSUME_NONNULL_END
