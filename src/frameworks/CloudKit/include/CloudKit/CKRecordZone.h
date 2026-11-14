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
#import <CloudKit/CKRecordZoneID.h>

NS_ASSUME_NONNULL_BEGIN

@class CKReference;

typedef NS_OPTIONS(NSUInteger, CKRecordZoneCapabilities) {
    CKRecordZoneCapabilityFetchChanges = 1 << 0,
    CKRecordZoneCapabilityAtomic = 1 << 1,
    CKRecordZoneCapabilitySharing = 1 << 2,
    CKRecordZoneCapabilityZoneWideSharing = 1 << 3,
} NS_SWIFT_NAME(CKRecordZone.Capabilities);

@interface CKRecordZone : NSObject <NSSecureCoding, NSCopying> {
@private
    CKRecordZoneID *_zoneID;
    CKRecordZoneCapabilities _capabilities;
    CKReference *_share;
}

+ (CKRecordZone *)defaultRecordZone;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

- (instancetype)initWithZoneName:(NSString *)zoneName;
- (instancetype)initWithZoneID:(CKRecordZoneID *)zoneID;

@property (nonatomic, readonly, copy) CKRecordZoneID *zoneID;
@property (nonatomic, readonly, assign) CKRecordZoneCapabilities capabilities;

// Reference to the share of a zone that is shared as a whole.
@property (nonatomic, readonly, copy, nullable) CKReference *share;

@end

NS_ASSUME_NONNULL_END
