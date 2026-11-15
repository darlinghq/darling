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

extern NSString * const CKRecordZoneDefaultName NS_REFINED_FOR_SWIFT;

// Owner name that stands for the current user.
extern NSString * const CKCurrentUserDefaultName;
extern NSString * const CKOwnerDefaultName API_DEPRECATED_WITH_REPLACEMENT("CKCurrentUserDefaultName", macos(10.10, 10.12));

// Identifies a record zone: its name and the name of the user that owns it.
NS_SWIFT_NAME(CKRecordZone.ID)
@interface CKRecordZoneID : NSObject <NSSecureCoding, NSCopying> {
@private
    NSString *_zoneName;
    NSString *_ownerName;
}

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

- (instancetype)initWithZoneName:(NSString *)zoneName ownerName:(NSString *)ownerName NS_REFINED_FOR_SWIFT;

@property (nonatomic, readonly, copy) NSString *zoneName;
@property (nonatomic, readonly, copy) NSString *ownerName;

@end

NS_ASSUME_NONNULL_END
