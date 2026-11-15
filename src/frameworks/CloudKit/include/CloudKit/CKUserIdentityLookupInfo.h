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

@class CKRecordID;

// Describes a user to look up by e-mail address, phone number or user record.
NS_SWIFT_NAME(CKUserIdentity.LookupInfo)
@interface CKUserIdentityLookupInfo : NSObject <NSSecureCoding, NSCopying> {
@private
    NSString *_emailAddress;
    NSString *_phoneNumber;
    CKRecordID *_userRecordID;
}

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

- (instancetype)initWithEmailAddress:(NSString *)emailAddress;
- (instancetype)initWithPhoneNumber:(NSString *)phoneNumber;
- (instancetype)initWithUserRecordID:(CKRecordID *)userRecordID;

@property (nonatomic, readonly, copy, nullable) NSString *emailAddress;
@property (nonatomic, readonly, copy, nullable) NSString *phoneNumber;
@property (nonatomic, readonly, copy, nullable) CKRecordID *userRecordID;

+ (NSArray<CKUserIdentityLookupInfo *> *)lookupInfosWithEmails:(NSArray<NSString *> *)emails NS_SWIFT_NAME(lookupInfos(withEmails:));
+ (NSArray<CKUserIdentityLookupInfo *> *)lookupInfosWithPhoneNumbers:(NSArray<NSString *> *)phoneNumbers NS_SWIFT_NAME(lookupInfos(withPhoneNumbers:));
+ (NSArray<CKUserIdentityLookupInfo *> *)lookupInfosWithRecordIDs:(NSArray<CKRecordID *> *)recordIDs NS_SWIFT_NAME(lookupInfos(withRecordIDs:));

@end

NS_ASSUME_NONNULL_END
