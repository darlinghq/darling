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

// Only ever produced from a server response; the properties are read-only.
@implementation CKShareMetadata

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    if ((self = [super init])) {
        _containerIdentifier = [[coder decodeObjectOfClass:[NSString class] forKey:@"ContainerIdentifier"] copy];
        _share = [[coder decodeObjectOfClass:[CKShare class] forKey:@"Share"] copy];
        _rootRecordID = [[coder decodeObjectOfClass:[CKRecordID class] forKey:@"RootRecordID"] copy];
        _participantRole = (CKShareParticipantRole)[coder decodeIntegerForKey:@"ParticipantRole"];
        _participantStatus = (CKShareParticipantAcceptanceStatus)[coder decodeIntegerForKey:@"ParticipantStatus"];
        _participantPermission = (CKShareParticipantPermission)[coder decodeIntegerForKey:@"ParticipantPermission"];
        _ownerIdentity = [[coder decodeObjectOfClass:[CKUserIdentity class] forKey:@"OwnerIdentity"] copy];
        _rootRecord = [[coder decodeObjectOfClass:[CKRecord class] forKey:@"RootRecord"] copy];
    }
    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_containerIdentifier forKey:@"ContainerIdentifier"];
    [coder encodeObject:_share forKey:@"Share"];
    [coder encodeObject:_rootRecordID forKey:@"RootRecordID"];
    [coder encodeInteger:_participantRole forKey:@"ParticipantRole"];
    [coder encodeInteger:_participantStatus forKey:@"ParticipantStatus"];
    [coder encodeInteger:_participantPermission forKey:@"ParticipantPermission"];
    [coder encodeObject:_ownerIdentity forKey:@"OwnerIdentity"];
    [coder encodeObject:_rootRecord forKey:@"RootRecord"];
}

- (id)copyWithZone:(NSZone *)zone
{
    // Immutable.
    return self;
}

- (NSString *)containerIdentifier
{
    return _containerIdentifier;
}

- (CKShare *)share
{
    return _share;
}

- (CKRecordID *)rootRecordID
{
    return _rootRecordID;
}

- (CKShareParticipantRole)participantRole
{
    return _participantRole;
}

- (CKShareParticipantAcceptanceStatus)participantStatus
{
    return _participantStatus;
}

- (CKShareParticipantPermission)participantPermission
{
    return _participantPermission;
}

- (CKShareParticipantType)participantType
{
    return (CKShareParticipantType)_participantRole;
}

- (CKUserIdentity *)ownerIdentity
{
    return _ownerIdentity;
}

- (CKRecord *)rootRecord
{
    return _rootRecord;
}

@end
