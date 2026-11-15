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
#import <CloudKit/CKShareParticipant.h>

NS_ASSUME_NONNULL_BEGIN

@class CKShare;
@class CKRecord;
@class CKRecordID;
@class CKUserIdentity;

// What a share's URL tells about the share, before it is accepted.
NS_SWIFT_NAME(CKShare.Metadata)
@interface CKShareMetadata : NSObject <NSSecureCoding, NSCopying> {
@private
    NSString *_containerIdentifier;
    CKShare *_share;
    CKRecordID *_rootRecordID;
    CKShareParticipantRole _participantRole;
    CKShareParticipantAcceptanceStatus _participantStatus;
    CKShareParticipantPermission _participantPermission;
    CKUserIdentity *_ownerIdentity;
    CKRecord *_rootRecord;
}

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@property (nonatomic, readonly, copy) NSString *containerIdentifier;
@property (nonatomic, readonly, copy) CKShare *share;
@property (nonatomic, readonly, copy) CKRecordID *rootRecordID;
@property (nonatomic, readonly, assign) CKShareParticipantRole participantRole;
@property (nonatomic, readonly, assign) CKShareParticipantAcceptanceStatus participantStatus;
@property (nonatomic, readonly, assign) CKShareParticipantPermission participantPermission;
@property (nonatomic, readonly, assign) CKShareParticipantType participantType API_DEPRECATED_WITH_REPLACEMENT("participantRole", macos(10.12, 10.14));
@property (nonatomic, readonly, copy) CKUserIdentity *ownerIdentity;
// Present only when the metadata was fetched with the root record.
@property (nonatomic, readonly, copy, nullable) CKRecord *rootRecord;

@end

NS_ASSUME_NONNULL_END
