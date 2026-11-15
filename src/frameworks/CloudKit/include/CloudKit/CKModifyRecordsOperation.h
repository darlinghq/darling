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

typedef NS_ENUM(NSInteger, CKRecordSavePolicy) {
    // Fails the save of a record when the server has a newer version of it.
    CKRecordSaveIfServerRecordUnchanged = 0,
    // Saves only the changed fields, overwriting whatever the server has for them.
    CKRecordSaveChangedKeys = 1,
    // Saves all fields, overwriting the server's record.
    CKRecordSaveAllKeys = 2,
} NS_SWIFT_NAME(CKModifyRecordsOperation.RecordSavePolicy);

// Saves and deletes records.
@interface CKModifyRecordsOperation : CKDatabaseOperation {
@private
    NSArray<CKRecord *> *_recordsToSave;
    NSArray<CKRecordID *> *_recordIDsToDelete;
    CKRecordSavePolicy _savePolicy;
    NSData *_clientChangeTokenData;
    BOOL _atomic;
    void (^_perRecordProgressBlock)(CKRecord *record, double progress);
    void (^_perRecordCompletionBlock)(CKRecord *record, NSError * _Nullable error);
    void (^_modifyRecordsCompletionBlock)(NSArray<CKRecord *> * _Nullable savedRecords, NSArray<CKRecordID *> * _Nullable deletedRecordIDs, NSError * _Nullable operationError);
}

- (instancetype)init;
- (instancetype)initWithRecordsToSave:(nullable NSArray<CKRecord *> *)records recordIDsToDelete:(nullable NSArray<CKRecordID *> *)recordIDs;

@property (nonatomic, copy, nullable) NSArray<CKRecord *> *recordsToSave;
@property (nonatomic, copy, nullable) NSArray<CKRecordID *> *recordIDsToDelete;
@property (nonatomic, assign) CKRecordSavePolicy savePolicy;
@property (nonatomic, copy, nullable) NSData *clientChangeTokenData;
// With YES (the default) either all changes in a zone are applied or none.
@property (nonatomic, assign, getter=isAtomic) BOOL atomic;
@property (nonatomic, copy, nullable) void (^perRecordProgressBlock)(CKRecord *record, double progress);
@property (nonatomic, copy, nullable) void (^perRecordCompletionBlock)(CKRecord *record, NSError * _Nullable error);
@property (nonatomic, copy, nullable) void (^modifyRecordsCompletionBlock)(NSArray<CKRecord *> * _Nullable savedRecords, NSArray<CKRecordID *> * _Nullable deletedRecordIDs, NSError * _Nullable operationError);
@end

NS_ASSUME_NONNULL_END
