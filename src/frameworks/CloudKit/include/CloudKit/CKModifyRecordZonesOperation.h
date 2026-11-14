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
#import <CloudKit/CKRecordZone.h>
#import <CloudKit/CKRecordZoneID.h>

NS_ASSUME_NONNULL_BEGIN

// Saves and deletes record zones.
@interface CKModifyRecordZonesOperation : CKDatabaseOperation {
@private
    NSArray<CKRecordZone *> *_recordZonesToSave;
    NSArray<CKRecordZoneID *> *_recordZoneIDsToDelete;
    void (^_modifyRecordZonesCompletionBlock)(NSArray<CKRecordZone *> * _Nullable savedRecordZones, NSArray<CKRecordZoneID *> * _Nullable deletedRecordZoneIDs, NSError * _Nullable operationError);
}

- (instancetype)initWithRecordZonesToSave:(nullable NSArray<CKRecordZone *> *)recordZonesToSave recordZoneIDsToDelete:(nullable NSArray<CKRecordZoneID *> *)recordZoneIDsToDelete;

@property (nonatomic, copy, nullable) NSArray<CKRecordZone *> *recordZonesToSave;
@property (nonatomic, copy, nullable) NSArray<CKRecordZoneID *> *recordZoneIDsToDelete;
@property (nonatomic, copy, nullable) void (^modifyRecordZonesCompletionBlock)(NSArray<CKRecordZone *> * _Nullable savedRecordZones, NSArray<CKRecordZoneID *> * _Nullable deletedRecordZoneIDs, NSError * _Nullable operationError);
@end

NS_ASSUME_NONNULL_END
