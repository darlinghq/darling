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
@class CKUserIdentityLookupInfo;

// A user that takes part in sharing.
@interface CKUserIdentity : NSObject <NSSecureCoding, NSCopying> {
@private
    CKUserIdentityLookupInfo *_lookupInfo;
    NSPersonNameComponents *_nameComponents;
    CKRecordID *_userRecordID;
    BOOL _hasiCloudAccount;
}

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@property (nonatomic, readonly, copy, nullable) CKUserIdentityLookupInfo *lookupInfo;
@property (nonatomic, readonly, copy, nullable) NSPersonNameComponents *nameComponents;
@property (nonatomic, readonly, copy, nullable) CKRecordID *userRecordID;
@property (nonatomic, readonly, assign) BOOL hasiCloudAccount;

@end

NS_ASSUME_NONNULL_END
