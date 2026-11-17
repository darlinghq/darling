/*
 This file is part of Darling.

 Copyright (C) 2025 Darling Developers

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

#import <UIFoundation/NSCollectionViewFlowLayoutInvalidationContext.h>

@implementation NSCollectionViewFlowLayoutInvalidationContext

// QUIRK: a new context invalidates the delegate metrics and the attributes (both flags are YES), not nothing.
// Not documented. Evidence: black-box observation on macOS 26.7.3 and 27.0.1 (tier 2).
// Test: tests/appkit-collection-view-compositional-layout.m (darling-cocotron).
- (instancetype) init {
    if ((self = [super init]) != nil) {
        _invalidateFlowLayoutDelegateMetrics = YES;
        _invalidateFlowLayoutAttributes = YES;
    }
    return self;
}

- (BOOL) invalidateFlowLayoutDelegateMetrics {
    return _invalidateFlowLayoutDelegateMetrics;
}

- (void) setInvalidateFlowLayoutDelegateMetrics: (BOOL) invalidate {
    _invalidateFlowLayoutDelegateMetrics = invalidate;
}

- (BOOL) invalidateFlowLayoutAttributes {
    return _invalidateFlowLayoutAttributes;
}

- (void) setInvalidateFlowLayoutAttributes: (BOOL) invalidate {
    _invalidateFlowLayoutAttributes = invalidate;
}

@end
