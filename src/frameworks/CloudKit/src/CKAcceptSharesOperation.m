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

@implementation CKAcceptSharesOperation

@synthesize shareMetadatas = _shareMetadatas;
@synthesize perShareCompletionBlock = _perShareCompletionBlock;
@synthesize acceptSharesCompletionBlock = _acceptSharesCompletionBlock;

- (instancetype)initWithShareMetadatas:(NSArray<CKShareMetadata *> *)shareMetadatas
{
    if ((self = [super init]))
        _shareMetadatas = [shareMetadatas copy];
    return self;
}

- (void)_performWithError:(NSError *)error
{
    if (_acceptSharesCompletionBlock)
        _acceptSharesCompletionBlock(error);
}
@end
