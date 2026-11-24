/*
 This file is part of Darling.

 Copyright (C) 2021-2026 Lubos Dolezel

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

#import <AppKit/NSCollectionViewLayoutInvalidationContext.h>
#import "NSCollectionViewLayoutPrivate.h"

@implementation NSCollectionViewLayoutInvalidationContext

- (void) dealloc {
    [_invalidatedItemIndexPaths release];
    [super dealloc];
}

- (BOOL) invalidateEverything {
    return _invalidateEverything;
}

- (BOOL) invalidateDataSourceCounts {
    return _invalidateDataSourceCounts;
}

- (void) _setInvalidateEverything: (BOOL) invalidate {
    _invalidateEverything = invalidate;
}

- (void) _setInvalidateDataSourceCounts: (BOOL) invalidate {
    _invalidateDataSourceCounts = invalidate;
}

- (void) invalidateItemsAtIndexPaths: (NSSet<NSIndexPath *> *) indexPaths {
    if (_invalidatedItemIndexPaths == nil)
        _invalidatedItemIndexPaths = [[NSMutableSet alloc] init];
    [_invalidatedItemIndexPaths unionSet: indexPaths];
}

- (NSSet<NSIndexPath *> *) invalidatedItemIndexPaths {
    return [[_invalidatedItemIndexPaths copy] autorelease];
}

- (NSPoint) contentOffsetAdjustment {
    return _contentOffsetAdjustment;
}

- (void) setContentOffsetAdjustment: (NSPoint) adjustment {
    _contentOffsetAdjustment = adjustment;
}

- (NSSize) contentSizeAdjustment {
    return _contentSizeAdjustment;
}

- (void) setContentSizeAdjustment: (NSSize) adjustment {
    _contentSizeAdjustment = adjustment;
}

@end
