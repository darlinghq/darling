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

@implementation CKReference

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (instancetype)initWithRecordID:(CKRecordID *)recordID action:(CKReferenceAction)action
{
    if (recordID == nil)
        CKRaiseInvalid(@"CKException", @"recordID can not be nil");
    if ((self = [super init])) {
        _recordID = [recordID copy];
        _referenceAction = action;
    }
    return self;
}

- (instancetype)initWithRecord:(CKRecord *)record action:(CKReferenceAction)action
{
    if (record == nil)
        CKRaiseInvalid(@"CKException", @"record can not be nil");
    return [self initWithRecordID:record.recordID action:action];
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    CKRecordID *recordID = [coder decodeObjectOfClass:[CKRecordID class] forKey:@"RecordID"];
    if (recordID == nil) {
        return nil;
    }
    return [self initWithRecordID:recordID action:(CKReferenceAction)[coder decodeIntegerForKey:@"Action"]];
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_recordID forKey:@"RecordID"];
    [coder encodeInteger:(NSInteger)_referenceAction forKey:@"Action"];
}

- (id)copyWithZone:(NSZone *)zone
{
    // Immutable.
    return self;
}

- (CKRecordID *)recordID
{
    return _recordID;
}

- (CKReferenceAction)referenceAction
{
    return _referenceAction;
}

- (BOOL)isEqual:(id)object
{
    if (self == object)
        return YES;
    if (![object isKindOfClass:[CKReference class]])
        return NO;
    CKReference *other = object;
    return _referenceAction == other->_referenceAction && [_recordID isEqual:other->_recordID];
}

- (NSUInteger)hash
{
    return _recordID.hash ^ _referenceAction;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@: %p; recordID=%@>", [self class], self, _recordID];
}

@end
