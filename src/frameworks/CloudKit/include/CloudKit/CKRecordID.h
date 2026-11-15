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

NS_ASSUME_NONNULL_BEGIN

@class CKRecordZoneID;

// Identifies a record: its name (at most 255 characters) and the zone it lives in.
NS_SWIFT_NAME(CKRecord.ID)
@interface CKRecordID : NSObject <NSSecureCoding, NSCopying> {
@private
    NSString *_recordName;
    CKRecordZoneID *_zoneID;
}

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

// Uses the default zone of the current user.
- (instancetype)initWithRecordName:(NSString *)recordName NS_REFINED_FOR_SWIFT;
- (instancetype)initWithRecordName:(NSString *)recordName zoneID:(CKRecordZoneID *)zoneID NS_REFINED_FOR_SWIFT;

@property (nonatomic, readonly, copy) NSString *recordName;
@property (nonatomic, readonly, copy) CKRecordZoneID *zoneID;

@end

NS_ASSUME_NONNULL_END
