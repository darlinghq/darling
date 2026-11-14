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
#import <CloudKit/CKSubscription.h>
#import <CloudKit/CKRecordZoneID.h>

NS_ASSUME_NONNULL_BEGIN

// Fires when anything changes in a record zone.
@interface CKRecordZoneSubscription : CKSubscription <NSSecureCoding, NSCopying> {
@private
    CKRecordZoneID *_zoneID;
    CKRecordType _recordType;
}

- (instancetype)initWithZoneID:(CKRecordZoneID *)zoneID;
- (instancetype)initWithZoneID:(CKRecordZoneID *)zoneID subscriptionID:(CKSubscriptionID)subscriptionID NS_REFINED_FOR_SWIFT;

@property (nonatomic, copy) CKRecordZoneID *zoneID;
// Restricts the subscription to one record type.
@property (nonatomic, copy, nullable) CKRecordType recordType NS_REFINED_FOR_SWIFT;

@end

NS_ASSUME_NONNULL_END
