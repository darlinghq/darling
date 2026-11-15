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

NSString * const CKRecordTypeUserRecord = @"Users";
NSString * const CKRecordTypeShare = @"cloudkit.share";
NSString * const CKRecordParentKey = @"___parent";
NSString * const CKRecordShareKey = @"___share";

// Longest permitted field key.
#define CKMaxKeyLength 255

// Classes a field value may have; the index identifies the kind an array holds.
static NSInteger CKValueKind(id value)
{
    if ([value isKindOfClass:[NSString class]]) return 0;
    if ([value isKindOfClass:[NSNumber class]]) return 1;
    if ([value isKindOfClass:[NSData class]]) return 2;
    if ([value isKindOfClass:[NSDate class]]) return 3;
    if ([value isKindOfClass:[CKReference class]]) return 4;
    if ([value isKindOfClass:[CKAsset class]]) return 5;
    if ([value isKindOfClass:[CLLocation class]]) return 6;
    return -1;
}

static void CKValidateKey(NSString *key)
{
    if (key == nil)
        CKRaiseInvalid(NSInvalidArgumentException, @"recordKey can not be nil");
    if (key.length == 0)
        CKRaiseInvalid(NSInvalidArgumentException, @"recordKey can not be empty");
    if (key.length > CKMaxKeyLength)
        CKRaiseInvalid(NSInvalidArgumentException, [NSString stringWithFormat:@"recordKey (%@) is too long", key]);
    // ASCII letters, digits and underscores; the first character is not a digit.
    for (NSUInteger i = 0; i < key.length; i++) {
        unichar c = [key characterAtIndex:i];
        BOOL letter = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
        BOOL digit = c >= '0' && c <= '9';
        if (!(letter || (digit && i > 0)))
            CKRaiseInvalid(NSInvalidArgumentException, [NSString stringWithFormat:@"recordKey (%@) contains invalid characters", key]);
    }
}

// The share's documented system fields (title, thumbnail, type) use reserved keys that the ordinary
// key rules would reject; only a CKShare may set them.
static BOOL CKIsShareSystemKey(CKRecord *record, NSString *key)
{
    return [record isKindOfClass:[CKShare class]] &&
           ([key isEqualToString:CKShareTitleKey] || [key isEqualToString:CKShareThumbnailImageDataKey] ||
            [key isEqualToString:CKShareTypeKey]);
}

// Validates a value and returns the copy to store.
static id CKCopyValidatedValue(id value)
{
    if ([value isKindOfClass:[NSArray class]]) {
        NSInteger kind = -2;
        for (id element in (NSArray *)value) {
            if ([element isKindOfClass:[NSArray class]])
                CKRaiseInvalid(@"CKException", [NSString stringWithFormat:@"Nested arrays are not allowed as CKRecord value: %@", value]);
            NSInteger k = CKValueKind(element);
            if (k < 0)
                CKRaiseInvalid(NSInvalidArgumentException, [NSString stringWithFormat:@"Objects of class %@ cannot be set on CKRecordValueStore", [element class]]);
            if (kind == -2)
                kind = k;
            else if (k != kind)
                CKRaiseInvalid(@"CKException", [NSString stringWithFormat:@"Array members can't be mixed class: %@", value]);
        }
        return [value copy];
    }
    if (CKValueKind(value) < 0)
        CKRaiseInvalid(NSInvalidArgumentException, [NSString stringWithFormat:@"Objects of class %@ cannot be set on CKRecordValueStore", [value class]]);
    if ([value conformsToProtocol:@protocol(NSCopying)])
        return [value copy];
    return value;
}

@implementation CKRecord

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (instancetype)initWithRecordType:(CKRecordType)recordType
{
    return [self initWithRecordType:recordType recordID:[[CKRecordID alloc] initWithRecordName:[[NSUUID UUID] UUIDString]]];
}

- (instancetype)initWithRecordType:(CKRecordType)recordType zoneID:(CKRecordZoneID *)zoneID
{
    return [self initWithRecordType:recordType recordID:[[CKRecordID alloc] initWithRecordName:[[NSUUID UUID] UUIDString] zoneID:zoneID]];
}

- (instancetype)initWithRecordType:(CKRecordType)recordType recordID:(CKRecordID *)recordID
{
    if (recordType == nil)
        CKRaiseInvalid(@"CKException", @"recordType can not be nil");
    if (recordType.length == 0)
        CKRaiseInvalid(@"CKException", @"recordType can not be empty");
    if (recordID == nil)
        CKRaiseInvalid(@"CKException", @"recordID can not be nil");
    if ((self = [super init])) {
        _recordType = [recordType copy];
        _recordID = [recordID copy];
        _values = [NSMutableDictionary dictionary];
        _changedKeys = [NSMutableSet set];
    }
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    CKRecordType type = [coder decodeObjectOfClass:[NSString class] forKey:@"RecordType"];
    CKRecordID *recordID = [coder decodeObjectOfClass:[CKRecordID class] forKey:@"RecordID"];
    if (type.length == 0 || recordID == nil) {
        return nil;
    }
    if ((self = [self initWithRecordType:type recordID:recordID])) {
        _recordChangeTag = [[coder decodeObjectOfClass:[NSString class] forKey:@"RecordChangeTag"] copy];
        _creatorUserRecordID = [[coder decodeObjectOfClass:[CKRecordID class] forKey:@"CreatorUserRecordID"] copy];
        _creationDate = [[coder decodeObjectOfClass:[NSDate class] forKey:@"CreationDate"] copy];
        _lastModifiedUserRecordID = [[coder decodeObjectOfClass:[CKRecordID class] forKey:@"LastModifiedUserRecordID"] copy];
        _modificationDate = [[coder decodeObjectOfClass:[NSDate class] forKey:@"ModificationDate"] copy];
        _parent = [[coder decodeObjectOfClass:[CKReference class] forKey:@"Parent"] copy];
        _share = [[coder decodeObjectOfClass:[CKReference class] forKey:@"Share"] copy];

        NSSet *valueClasses = [NSSet setWithObjects:[NSString class], [NSNumber class], [NSData class], [NSDate class],
                               [NSArray class], [CKReference class], [CKAsset class], [CLLocation class], nil];
        NSDictionary *values = [coder decodeObjectOfClasses:[valueClasses setByAddingObject:[NSDictionary class]] forKey:@"Values"];
        for (NSString *key in values) {
            if (![key isKindOfClass:[NSString class]]) {
                return nil;
            }
            _values[key] = values[key];
        }
        NSSet *changed = [coder decodeObjectOfClasses:[NSSet setWithObjects:[NSSet class], [NSString class], nil] forKey:@"ChangedKeys"];
        for (NSString *key in changed) {
            if ([key isKindOfClass:[NSString class]])
                [_changedKeys addObject:key];
        }
    }
    return self;
}

- (void)encodeSystemFieldsWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_recordType forKey:@"RecordType"];
    [coder encodeObject:_recordID forKey:@"RecordID"];
    [coder encodeObject:_recordChangeTag forKey:@"RecordChangeTag"];
    [coder encodeObject:_creatorUserRecordID forKey:@"CreatorUserRecordID"];
    [coder encodeObject:_creationDate forKey:@"CreationDate"];
    [coder encodeObject:_lastModifiedUserRecordID forKey:@"LastModifiedUserRecordID"];
    [coder encodeObject:_modificationDate forKey:@"ModificationDate"];
    [coder encodeObject:_share forKey:@"Share"];
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [self encodeSystemFieldsWithCoder:coder];
    [coder encodeObject:_parent forKey:@"Parent"];
    [coder encodeObject:_values forKey:@"Values"];
    [coder encodeObject:_changedKeys forKey:@"ChangedKeys"];
}

- (id)copyWithZone:(NSZone *)zone
{
    CKRecord *copy = [[[self class] allocWithZone:zone] initWithRecordType:_recordType recordID:_recordID];
    copy->_values = [_values mutableCopy];
    copy->_changedKeys = [_changedKeys mutableCopy];
    copy->_recordChangeTag = _recordChangeTag;
    copy->_creatorUserRecordID = _creatorUserRecordID;
    copy->_creationDate = _creationDate;
    copy->_lastModifiedUserRecordID = _lastModifiedUserRecordID;
    copy->_modificationDate = _modificationDate;
    copy->_parent = _parent;
    copy->_share = _share;
    return copy;
}

- (CKRecordType)recordType
{
    return _recordType;
}

- (CKRecordID *)recordID
{
    return _recordID;
}

- (NSString *)recordChangeTag
{
    return _recordChangeTag;
}

- (CKRecordID *)creatorUserRecordID
{
    return _creatorUserRecordID;
}

- (NSDate *)creationDate
{
    return _creationDate;
}

- (CKRecordID *)lastModifiedUserRecordID
{
    return _lastModifiedUserRecordID;
}

- (NSDate *)modificationDate
{
    return _modificationDate;
}

- (id<CKRecordValue>)objectForKey:(CKRecordFieldKey)key
{
    return _values[key];
}

- (void)setObject:(id<CKRecordValue>)object forKey:(CKRecordFieldKey)key
{
    if (!CKIsShareSystemKey(self, key))
        CKValidateKey(key);
    if (object == nil) {
        [_values removeObjectForKey:key];
    } else {
        _values[key] = CKCopyValidatedValue(object);
    }
    // Removing a field is a change too, even when the field was not there.
    [_changedKeys addObject:key];
}

- (id<CKRecordValue>)objectForKeyedSubscript:(CKRecordFieldKey)key
{
    return [self objectForKey:key];
}

- (void)setObject:(id<CKRecordValue>)object forKeyedSubscript:(CKRecordFieldKey)key
{
    [self setObject:object forKey:key];
}

- (NSArray<CKRecordFieldKey> *)allKeys
{
    return [_values allKeys];
}

- (NSArray<CKRecordFieldKey> *)changedKeys
{
    return [_changedKeys allObjects];
}

- (CKReference *)parent
{
    return _parent;
}

- (void)setParent:(CKReference *)parent
{
    if (parent != nil && ![parent.recordID.zoneID isEqual:_recordID.zoneID])
        CKRaiseInvalid(NSInternalInconsistencyException, @"Parent record must be in the same zone as the current record");
    _parent = [parent copy];
}

- (void)setParentReferenceFromRecord:(CKRecord *)parentRecord
{
    [self setParentReferenceFromRecordID:parentRecord.recordID];
}

- (void)setParentReferenceFromRecordID:(CKRecordID *)parentRecordID
{
    if (parentRecordID == nil) {
        self.parent = nil;
        return;
    }
    self.parent = [[CKReference alloc] initWithRecordID:parentRecordID action:CKReferenceActionNone];
}

- (CKReference *)share
{
    return _share;
}

- (void)_setRecordChangeTag:(NSString *)tag creatorUserRecordID:(CKRecordID *)creator creationDate:(NSDate *)creationDate lastModifiedUserRecordID:(CKRecordID *)modifier modificationDate:(NSDate *)modificationDate
{
    _recordChangeTag = [tag copy];
    _creatorUserRecordID = [creator copy];
    _creationDate = [creationDate copy];
    _lastModifiedUserRecordID = [modifier copy];
    _modificationDate = [modificationDate copy];
}

- (void)_clearChangedKeys
{
    [_changedKeys removeAllObjects];
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@: %p; recordType=%@, recordID=%@:(%@:%@), values=%@>", [self class], self,
            _recordType, _recordID.recordName, _recordID.zoneID.zoneName, _recordID.zoneID.ownerName, _values];
}

@end
