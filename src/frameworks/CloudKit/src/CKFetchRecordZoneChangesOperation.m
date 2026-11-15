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

@implementation CKFetchRecordZoneChangesOperation

@synthesize recordZoneIDs = _recordZoneIDs;
@synthesize configurationsByRecordZoneID = _configurationsByRecordZoneID;
@synthesize optionsByRecordZoneID = _optionsByRecordZoneID;
@synthesize fetchAllChanges = _fetchAllChanges;
@synthesize recordChangedBlock = _recordChangedBlock;
@synthesize recordWithIDWasDeletedBlock = _recordWithIDWasDeletedBlock;
@synthesize recordZoneChangeTokensUpdatedBlock = _recordZoneChangeTokensUpdatedBlock;
@synthesize recordZoneFetchCompletionBlock = _recordZoneFetchCompletionBlock;
@synthesize fetchRecordZoneChangesCompletionBlock = _fetchRecordZoneChangesCompletionBlock;

- (instancetype)init
{
    if ((self = [super init]))
        _fetchAllChanges = YES;
    return self;
}

- (instancetype)initWithRecordZoneIDs:(NSArray<CKRecordZoneID *> *)recordZoneIDs configurationsByRecordZoneID:(NSDictionary<CKRecordZoneID *, CKFetchRecordZoneChangesConfiguration *> *)configurationsByRecordZoneID
{
    if ((self = [self init])) {
        _recordZoneIDs = [recordZoneIDs copy];
        _configurationsByRecordZoneID = [configurationsByRecordZoneID copy];
    }
    return self;
}

- (instancetype)initWithRecordZoneIDs:(NSArray<CKRecordZoneID *> *)recordZoneIDs optionsByRecordZoneID:(NSDictionary<CKRecordZoneID *, CKFetchRecordZoneChangesOptions *> *)optionsByRecordZoneID
{
    if ((self = [self init])) {
        _recordZoneIDs = [recordZoneIDs copy];
        _optionsByRecordZoneID = [optionsByRecordZoneID copy];
    }
    return self;
}

- (void)_performWithError:(NSError *)error
{
    if (_fetchRecordZoneChangesCompletionBlock)
        _fetchRecordZoneChangesCompletionBlock(error);
}
@end
