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

@implementation CKQuery

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (instancetype)initWithRecordType:(CKRecordType)recordType predicate:(NSPredicate *)predicate
{
    if (recordType == nil || recordType.length == 0)
        CKRaiseInvalid(@"CKException", @"recordType can not be nil or empty");
    if (predicate == nil)
        CKRaiseInvalid(@"CKException", @"predicate can not be nil");
    if ((self = [super init])) {
        _recordType = [recordType copy];
        _predicate = [predicate copy];
    }
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    CKRecordType type = [coder decodeObjectOfClass:[NSString class] forKey:@"RecordType"];
    NSPredicate *predicate = [coder decodeObjectOfClass:[NSPredicate class] forKey:@"Predicate"];
    if (type.length == 0 || predicate == nil) {
        return nil;
    }
    if ((self = [self initWithRecordType:type predicate:predicate])) {
        NSSet *classes = [NSSet setWithObjects:[NSArray class], [NSSortDescriptor class], nil];
        _sortDescriptors = [[coder decodeObjectOfClasses:classes forKey:@"SortDescriptors"] copy];
    }
    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_recordType forKey:@"RecordType"];
    [coder encodeObject:_predicate forKey:@"Predicate"];
    [coder encodeObject:_sortDescriptors forKey:@"SortDescriptors"];
}

- (id)copyWithZone:(NSZone *)zone
{
    CKQuery *copy = [[[self class] allocWithZone:zone] initWithRecordType:_recordType predicate:_predicate];
    copy->_sortDescriptors = _sortDescriptors;
    return copy;
}

- (CKRecordType)recordType
{
    return _recordType;
}

- (NSPredicate *)predicate
{
    return _predicate;
}

- (NSArray<NSSortDescriptor *> *)sortDescriptors
{
    return _sortDescriptors;
}

- (void)setSortDescriptors:(NSArray<NSSortDescriptor *> *)sortDescriptors
{
    _sortDescriptors = [sortDescriptors copy];
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@: %p; recordType=%@, predicate=%@, sortDescriptors=%@>", [self class], self,
            _recordType, _predicate, _sortDescriptors];
}

@end
