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

NSString * const CKShareTitleKey = @"cloudkit.title";
NSString * const CKShareThumbnailImageDataKey = @"cloudkit.thumbnailImageData";
NSString * const CKShareTypeKey = @"cloudkit.type";

// Record name of a share that covers a whole zone.
static NSString * const CKZoneShareRecordName = @"cloudkit.zoneshare";

@implementation CKShare

+ (BOOL)supportsSecureCoding
{
    return YES;
}

// A share starts with its owner, the current user, who can read and write.
- (instancetype)_initWithRecordID:(CKRecordID *)recordID
{
    if ((self = [super initWithRecordType:CKRecordTypeShare recordID:recordID])) {
        _publicPermission = CKShareParticipantPermissionNone;
        CKRecordID *ownerRecordID = [[CKRecordID alloc] initWithRecordName:CKCurrentUserDefaultName];
        CKUserIdentity *identity = [[CKUserIdentity alloc] _initWithUserRecordID:ownerRecordID lookupInfo:nil nameComponents:nil hasiCloudAccount:YES];
        _owner = [[CKShareParticipant alloc] _initWithUserIdentity:identity
                                                        permission:CKShareParticipantPermissionReadWrite
                                                  acceptanceStatus:CKShareParticipantAcceptanceStatusAccepted
                                                              role:CKShareParticipantRoleOwner];
        _participants = [NSMutableArray arrayWithObject:_owner];
    }
    return self;
}

- (instancetype)initWithRootRecord:(CKRecord *)rootRecord
{
    if (rootRecord == nil)
        CKRaiseInvalid(@"CKException", @"rootRecord can not be nil");
    CKRecordID *shareID = [[CKRecordID alloc] initWithRecordName:[@"Share-" stringByAppendingString:[[NSUUID UUID] UUIDString]]
                                                          zoneID:rootRecord.recordID.zoneID];
    return [self initWithRootRecord:rootRecord shareID:shareID];
}

- (instancetype)initWithRootRecord:(CKRecord *)rootRecord shareID:(CKRecordID *)shareID
{
    if (rootRecord == nil)
        CKRaiseInvalid(@"CKException", @"rootRecord can not be nil");
    if (shareID == nil)
        CKRaiseInvalid(@"CKException", @"shareID can not be nil");
    if (![shareID.zoneID isEqual:rootRecord.recordID.zoneID])
        CKRaiseInvalid(NSInternalInconsistencyException, @"Share must be in the same zone as the current record");
    return [self _initWithRecordID:shareID];
}

- (instancetype)initWithRecordZoneID:(CKRecordZoneID *)recordZoneID
{
    if (recordZoneID == nil)
        CKRaiseInvalid(@"CKException", @"recordZoneID can not be nil");
    return [self _initWithRecordID:[[CKRecordID alloc] initWithRecordName:CKZoneShareRecordName zoneID:recordZoneID]];
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    if ((self = [super initWithCoder:coder])) {
        _publicPermission = (CKShareParticipantPermission)[coder decodeIntegerForKey:@"PublicPermission"];
        _URL = [[coder decodeObjectOfClass:[NSURL class] forKey:@"URL"] copy];
        NSSet *classes = [NSSet setWithObjects:[NSArray class], [CKShareParticipant class], nil];
        NSArray *participants = [coder decodeObjectOfClasses:classes forKey:@"Participants"];
        _participants = [NSMutableArray array];
        for (CKShareParticipant *participant in participants) {
            if (![participant isKindOfClass:[CKShareParticipant class]]) {
                return nil;
            }
            [_participants addObject:participant];
            if (participant.role == CKShareParticipantRoleOwner)
                _owner = participant;
        }
        if (_owner == nil) {
            return nil;
        }
    }
    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [super encodeWithCoder:coder];
    [coder encodeInteger:_publicPermission forKey:@"PublicPermission"];
    [coder encodeObject:_URL forKey:@"URL"];
    [coder encodeObject:_participants forKey:@"Participants"];
}

- (id)copyWithZone:(NSZone *)zone
{
    CKShare *copy = [super copyWithZone:zone];
    copy->_publicPermission = _publicPermission;
    copy->_URL = _URL;
    copy->_participants = [_participants mutableCopy];
    copy->_owner = _owner;
    return copy;
}

- (CKShareParticipantPermission)publicPermission
{
    return _publicPermission;
}

- (void)setPublicPermission:(CKShareParticipantPermission)publicPermission
{
    _publicPermission = publicPermission;
}

- (NSURL *)URL
{
    return _URL;
}

- (NSArray<CKShareParticipant *> *)participants
{
    return [_participants copy];
}

- (CKShareParticipant *)owner
{
    return _owner;
}

// Everything is created by the current user, so the owner is also the current user's participant.
- (CKShareParticipant *)currentUserParticipant
{
    return _owner;
}

- (void)addParticipant:(CKShareParticipant *)participant
{
    if (participant == nil)
        CKRaiseInvalid(@"CKException", @"participant can not be nil");
    if (participant.role == CKShareParticipantRoleOwner)
        CKRaiseInvalid(NSInternalInconsistencyException, @"You can have only one owner on a share");
    if (![_participants containsObject:participant])
        [_participants addObject:participant];
}

- (void)removeParticipant:(CKShareParticipant *)participant
{
    if (participant == nil)
        CKRaiseInvalid(@"CKException", @"participant can not be nil");
    if (participant.role == CKShareParticipantRoleOwner)
        CKRaiseInvalid(@"CKException", @"You cannot remove the owner from a share");
    [_participants removeObject:participant];
}

@end
