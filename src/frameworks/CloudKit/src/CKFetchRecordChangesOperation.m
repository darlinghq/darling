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

@implementation CKFetchRecordChangesOperation

@synthesize recordZoneID = _recordZoneID;
@synthesize previousServerChangeToken = _previousServerChangeToken;
@synthesize resultsLimit = _resultsLimit;
@synthesize desiredKeys = _desiredKeys;
@synthesize moreComing = _moreComing;
@synthesize recordChangedBlock = _recordChangedBlock;
@synthesize recordWithIDWasDeletedBlock = _recordWithIDWasDeletedBlock;
@synthesize fetchRecordChangesCompletionBlock = _fetchRecordChangesCompletionBlock;

- (instancetype)initWithRecordZoneID:(CKRecordZoneID *)recordZoneID previousServerChangeToken:(CKServerChangeToken *)previousServerChangeToken
{
    if ((self = [super init])) {
        _recordZoneID = [recordZoneID copy];
        _previousServerChangeToken = [previousServerChangeToken copy];
    }
    return self;
}

- (void)_performWithError:(NSError *)error
{
    if (_fetchRecordChangesCompletionBlock)
        _fetchRecordChangesCompletionBlock(nil, nil, error);
}
@end
