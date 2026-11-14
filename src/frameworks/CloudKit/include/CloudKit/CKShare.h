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
#import <CloudKit/CKRecord.h>
#import <CloudKit/CKShareParticipant.h>

NS_ASSUME_NONNULL_BEGIN

// Names of the fields a share has.
extern NSString * const CKShareTitleKey NS_REFINED_FOR_SWIFT;
extern NSString * const CKShareThumbnailImageDataKey NS_REFINED_FOR_SWIFT;
extern NSString * const CKShareTypeKey NS_REFINED_FOR_SWIFT;

// A record that controls who can see a record hierarchy or a whole zone.
@interface CKShare : CKRecord <NSSecureCoding, NSCopying> {
@private
    CKShareParticipantPermission _publicPermission;
    NSURL *_URL;
    NSMutableArray<CKShareParticipant *> *_participants;
    CKShareParticipant *_owner;
}

// Permission of people who have the share's URL but were not invited. Starts as CKShareParticipantPermissionNone.
@property (nonatomic, assign) CKShareParticipantPermission publicPermission;

// Set by the server once the share has been saved.
@property (nonatomic, readonly, copy, nullable) NSURL *URL;

// Always includes the owner.
@property (nonatomic, readonly, copy) NSArray<CKShareParticipant *> *participants;
@property (nonatomic, readonly, copy) CKShareParticipant *owner;
@property (nonatomic, readonly, copy, nullable) CKShareParticipant *currentUserParticipant;

// The root record and the share must be in the same zone.
- (instancetype)initWithRootRecord:(CKRecord *)rootRecord;
- (instancetype)initWithRootRecord:(CKRecord *)rootRecord shareID:(CKRecordID *)shareID;
// Shares all the records of a zone.
- (instancetype)initWithRecordZoneID:(CKRecordZoneID *)recordZoneID;

- (instancetype)initWithRecordType:(CKRecordType)recordType NS_UNAVAILABLE;
- (instancetype)initWithRecordType:(CKRecordType)recordType recordID:(CKRecordID *)recordID NS_UNAVAILABLE;
- (instancetype)initWithRecordType:(CKRecordType)recordType zoneID:(CKRecordZoneID *)zoneID NS_UNAVAILABLE;

- (void)addParticipant:(CKShareParticipant *)participant;
- (void)removeParticipant:(CKShareParticipant *)participant;

@end

NS_ASSUME_NONNULL_END
