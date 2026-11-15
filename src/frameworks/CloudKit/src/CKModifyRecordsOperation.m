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

@implementation CKModifyRecordsOperation

@synthesize recordsToSave = _recordsToSave;
@synthesize recordIDsToDelete = _recordIDsToDelete;
@synthesize savePolicy = _savePolicy;
@synthesize clientChangeTokenData = _clientChangeTokenData;
@synthesize atomic = _atomic;
@synthesize perRecordProgressBlock = _perRecordProgressBlock;
@synthesize perRecordCompletionBlock = _perRecordCompletionBlock;
@synthesize modifyRecordsCompletionBlock = _modifyRecordsCompletionBlock;

- (instancetype)init
{
    if ((self = [super init]))
        _atomic = YES;
    return self;
}

- (instancetype)initWithRecordsToSave:(NSArray<CKRecord *> *)records recordIDsToDelete:(NSArray<CKRecordID *> *)recordIDs
{
    if ((self = [self init])) {
        _recordsToSave = [records copy];
        _recordIDsToDelete = [recordIDs copy];
    }
    return self;
}

- (void)_performWithError:(NSError *)error
{
    if (_modifyRecordsCompletionBlock)
        _modifyRecordsCompletionBlock(nil, nil, error);
}
@end
