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

@class CKUserIdentity;

typedef NS_ENUM(NSInteger, CKShareParticipantAcceptanceStatus) {
    CKShareParticipantAcceptanceStatusUnknown = 0,
    CKShareParticipantAcceptanceStatusPending = 1,
    CKShareParticipantAcceptanceStatusAccepted = 2,
    CKShareParticipantAcceptanceStatusRemoved = 3,
} NS_SWIFT_NAME(CKShare_Participant_AcceptanceStatus);

typedef NS_ENUM(NSInteger, CKShareParticipantPermission) {
    CKShareParticipantPermissionUnknown = 0,
    CKShareParticipantPermissionNone = 1,
    CKShareParticipantPermissionReadOnly = 2,
    CKShareParticipantPermissionReadWrite = 3,
} NS_SWIFT_NAME(CKShare_Participant_Permission);

typedef NS_ENUM(NSInteger, CKShareParticipantRole) {
    CKShareParticipantRoleUnknown = 0,
    CKShareParticipantRoleOwner = 1,
    CKShareParticipantRolePrivateUser = 3,
    CKShareParticipantRolePublicUser = 4,
} NS_SWIFT_NAME(CKShare_Participant_Role);

typedef NS_ENUM(NSInteger, CKShareParticipantType) {
    CKShareParticipantTypeUnknown = 0,
    CKShareParticipantTypeOwner = 1,
    CKShareParticipantTypePrivateUser = 3,
    CKShareParticipantTypePublicUser = 4,
} NS_SWIFT_NAME(CKShare_Participant_ParticipantType) API_DEPRECATED_WITH_REPLACEMENT("CKShareParticipantRole", macos(10.12, 10.14));

// A person a share was offered to, or its owner.
NS_SWIFT_NAME(CKShare.Participant)
@interface CKShareParticipant : NSObject <NSSecureCoding, NSCopying> {
@private
    CKUserIdentity *_userIdentity;
    CKShareParticipantPermission _permission;
    CKShareParticipantAcceptanceStatus _acceptanceStatus;
    CKShareParticipantRole _role;
}

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@property (nonatomic, readonly, strong) CKUserIdentity *userIdentity;
@property (nonatomic, assign) CKShareParticipantPermission permission;
@property (nonatomic, readonly, assign) CKShareParticipantAcceptanceStatus acceptanceStatus;
@property (nonatomic, assign) CKShareParticipantRole role;
@property (nonatomic, assign) CKShareParticipantType type API_DEPRECATED_WITH_REPLACEMENT("role", macos(10.12, 10.14));

@end

NS_ASSUME_NONNULL_END
