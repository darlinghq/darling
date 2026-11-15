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

// Fetches record zones by ID, or all of them.
@interface CKFetchRecordZonesOperation : CKDatabaseOperation {
@private
    NSArray<CKRecordZoneID *> *_recordZoneIDs;
    void (^_fetchRecordZonesCompletionBlock)(NSDictionary<CKRecordZoneID *, CKRecordZone *> * _Nullable recordZonesByZoneID, NSError * _Nullable operationError);
}

- (instancetype)initWithRecordZoneIDs:(NSArray<CKRecordZoneID *> *)zoneIDs;
+ (instancetype)fetchAllRecordZonesOperation NS_SWIFT_NAME(fetchAllRecordZonesOperation());

// nil for the operation that fetches all zones.
@property (nonatomic, copy, nullable) NSArray<CKRecordZoneID *> *recordZoneIDs;
@property (nonatomic, copy, nullable) void (^fetchRecordZonesCompletionBlock)(NSDictionary<CKRecordZoneID *, CKRecordZone *> * _Nullable recordZonesByZoneID, NSError * _Nullable operationError);
@end

NS_ASSUME_NONNULL_END
