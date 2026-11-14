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
#import <CloudKit/CKQuery.h>
#import <CloudKit/CKQueryCursor.h>
#import <CloudKit/CKRecordZoneID.h>

NS_ASSUME_NONNULL_BEGIN

// Passed as the results limit to ask for as many results as the server returns in one batch.
extern const NSUInteger CKQueryOperationMaximumResults NS_SWIFT_NAME(CKQueryOperation.maximumResults);

// Runs a query and delivers the matching records.
@interface CKQueryOperation : CKDatabaseOperation {
@private
    CKQuery *_query;
    CKQueryCursor *_cursor;
    CKRecordZoneID *_zoneID;
    NSUInteger _resultsLimit;
    NSArray<CKRecordFieldKey> *_desiredKeys;
    void (^_recordFetchedBlock)(CKRecord *record);
    void (^_queryCompletionBlock)(CKQueryCursor * _Nullable cursor, NSError * _Nullable operationError);
}

- (instancetype)initWithQuery:(CKQuery *)query;
// Continues a query where an earlier operation stopped.
- (instancetype)initWithCursor:(CKQueryCursor *)cursor;

@property (nonatomic, copy, nullable) CKQuery *query;
@property (nonatomic, copy, nullable) CKQueryCursor *cursor;
// Zone to search; nil searches all the zones of the database (private database only).
@property (nonatomic, copy, nullable) CKRecordZoneID *zoneID;
@property (nonatomic, assign) NSUInteger resultsLimit;
@property (nonatomic, copy, nullable) NSArray<CKRecordFieldKey> *desiredKeys NS_REFINED_FOR_SWIFT;
@property (nonatomic, copy, nullable) void (^recordFetchedBlock)(CKRecord *record);
// Called once; a non-nil cursor means more results are available.
@property (nonatomic, copy, nullable) void (^queryCompletionBlock)(CKQueryCursor * _Nullable cursor, NSError * _Nullable operationError);
@end

NS_ASSUME_NONNULL_END
