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

@implementation CKFetchRecordZoneChangesOptions

@synthesize previousServerChangeToken = _previousServerChangeToken;
@synthesize resultsLimit = _resultsLimit;
@synthesize desiredKeys = _desiredKeys;

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    if ((self = [self init])) {
        _previousServerChangeToken = [[coder decodeObjectOfClass:[CKServerChangeToken class] forKey:@"PreviousServerChangeToken"] copy];
        _resultsLimit = (NSUInteger)[coder decodeIntegerForKey:@"ResultsLimit"];
        _desiredKeys = [[coder decodeObjectOfClasses:[NSSet setWithObjects:[NSArray class], [NSString class], nil] forKey:@"DesiredKeys"] copy];
    }
    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_previousServerChangeToken forKey:@"PreviousServerChangeToken"];
    [coder encodeInteger:(NSInteger)_resultsLimit forKey:@"ResultsLimit"];
    [coder encodeObject:_desiredKeys forKey:@"DesiredKeys"];
}

- (id)copyWithZone:(NSZone *)zone
{
    CKFetchRecordZoneChangesOptions *copy = [[[self class] allocWithZone:zone] init];
    copy->_previousServerChangeToken = _previousServerChangeToken;
    copy->_resultsLimit = _resultsLimit;
    copy->_desiredKeys = _desiredKeys;
    return copy;
}
@end
