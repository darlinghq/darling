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
#import <CloudKit/CKDefines.h>
#import <CloudKit/CKRecordValue.h>
#import <CloudKit/CKRecordKeyValueSetting.h>
#import <CloudKit/CKRecordID.h>
#import <CloudKit/CKRecordZoneID.h>
#import <CloudKit/CKReference.h>
#import <CloudKit/CKAsset.h>

NS_ASSUME_NONNULL_BEGIN

// Record type of the record that describes a user, and of a share.
extern NSString * const CKRecordTypeUserRecord NS_REFINED_FOR_SWIFT;
extern NSString * const CKRecordTypeShare NS_REFINED_FOR_SWIFT;

// Names of fields every record has.
extern NSString * const CKRecordParentKey NS_REFINED_FOR_SWIFT;
extern NSString * const CKRecordShareKey NS_REFINED_FOR_SWIFT;

// A set of named fields of a record type, stored in a record zone.
@interface CKRecord : NSObject <NSSecureCoding, NSCopying> {
@private
    CKRecordType _recordType;
    CKRecordID *_recordID;
    NSMutableDictionary<NSString *, id> *_values;
    NSMutableSet<NSString *> *_changedKeys;
    NSString *_recordChangeTag;
    CKRecordID *_creatorUserRecordID;
    NSDate *_creationDate;
    CKRecordID *_lastModifiedUserRecordID;
    NSDate *_modificationDate;
    CKReference *_parent;
    CKReference *_share;
}

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

// The record gets a random name in the default zone when no record ID is given.
- (instancetype)initWithRecordType:(CKRecordType)recordType;
- (instancetype)initWithRecordType:(CKRecordType)recordType recordID:(CKRecordID *)recordID NS_REFINED_FOR_SWIFT;
- (instancetype)initWithRecordType:(CKRecordType)recordType zoneID:(CKRecordZoneID *)zoneID NS_SWIFT_UNAVAILABLE("Use init(recordType:recordID:) with a CKRecord.ID");

@property (nonatomic, readonly, copy) CKRecordType recordType NS_REFINED_FOR_SWIFT;
@property (nonatomic, readonly, copy) CKRecordID *recordID;

// Server-assigned metadata; nil until the record has been saved.
@property (nonatomic, readonly, copy, nullable) NSString *recordChangeTag;
@property (nonatomic, readonly, copy, nullable) CKRecordID *creatorUserRecordID;
@property (nonatomic, readonly, copy, nullable) NSDate *creationDate;
@property (nonatomic, readonly, copy, nullable) CKRecordID *lastModifiedUserRecordID;
@property (nonatomic, readonly, copy, nullable) NSDate *modificationDate;

// Field access. Values must be of one of the CKRecordValue classes; an array must not mix classes or nest.
- (nullable id<CKRecordValue>)objectForKey:(CKRecordFieldKey)key NS_REFINED_FOR_SWIFT;
- (void)setObject:(nullable id<CKRecordValue>)object forKey:(CKRecordFieldKey)key NS_REFINED_FOR_SWIFT;
- (nullable id<CKRecordValue>)objectForKeyedSubscript:(CKRecordFieldKey)key;
- (void)setObject:(nullable id<CKRecordValue>)object forKeyedSubscript:(CKRecordFieldKey)key;
- (NSArray<CKRecordFieldKey> *)allKeys NS_REFINED_FOR_SWIFT;
- (NSArray<CKRecordFieldKey> *)changedKeys NS_REFINED_FOR_SWIFT;

// Archives only the server-assigned fields, so that a later save can detect conflicts.
- (void)encodeSystemFieldsWithCoder:(NSCoder *)coder;

// Parent in the record hierarchy that defines what a share covers. Both must be in the record's zone.
@property (nonatomic, copy, nullable) CKReference *parent;
- (void)setParentReferenceFromRecord:(nullable CKRecord *)parentRecord;
- (void)setParentReferenceFromRecordID:(nullable CKRecordID *)parentRecordID;

// Reference to the share that covers this record.
@property (nonatomic, readonly, copy, nullable) CKReference *share;

@end

@interface CKRecord (CKRecordKeyValueSetting) <CKRecordKeyValueSetting>
@end

NS_ASSUME_NONNULL_END
