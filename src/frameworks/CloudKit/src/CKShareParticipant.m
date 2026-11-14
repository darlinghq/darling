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

#import "CKInternal.h"

@implementation CKShareParticipant

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (instancetype)_initWithUserIdentity:(CKUserIdentity *)identity permission:(CKShareParticipantPermission)permission acceptanceStatus:(CKShareParticipantAcceptanceStatus)status role:(CKShareParticipantRole)role
{
    if ((self = [super init])) {
        _userIdentity = identity;
        _permission = permission;
        _acceptanceStatus = status;
        _role = role;
    }
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    CKUserIdentity *identity = [coder decodeObjectOfClass:[CKUserIdentity class] forKey:@"UserIdentity"];
    if (identity == nil) {
        return nil;
    }
    return [self _initWithUserIdentity:identity
                            permission:(CKShareParticipantPermission)[coder decodeIntegerForKey:@"Permission"]
                      acceptanceStatus:(CKShareParticipantAcceptanceStatus)[coder decodeIntegerForKey:@"AcceptanceStatus"]
                                  role:(CKShareParticipantRole)[coder decodeIntegerForKey:@"Role"]];
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_userIdentity forKey:@"UserIdentity"];
    [coder encodeInteger:_permission forKey:@"Permission"];
    [coder encodeInteger:_acceptanceStatus forKey:@"AcceptanceStatus"];
    [coder encodeInteger:_role forKey:@"Role"];
}

- (id)copyWithZone:(NSZone *)zone
{
    return [[[self class] allocWithZone:zone] _initWithUserIdentity:_userIdentity permission:_permission
                                                   acceptanceStatus:_acceptanceStatus role:_role];
}

- (CKUserIdentity *)userIdentity
{
    return _userIdentity;
}

- (CKShareParticipantPermission)permission
{
    return _permission;
}

- (void)setPermission:(CKShareParticipantPermission)permission
{
    _permission = permission;
}

- (CKShareParticipantAcceptanceStatus)acceptanceStatus
{
    return _acceptanceStatus;
}

- (CKShareParticipantRole)role
{
    return _role;
}

- (void)setRole:(CKShareParticipantRole)role
{
    _role = role;
}

// The deprecated type is the role under its older name; the values agree.
- (CKShareParticipantType)type
{
    return (CKShareParticipantType)_role;
}

- (void)setType:(CKShareParticipantType)type
{
    _role = (CKShareParticipantRole)type;
}

@end
