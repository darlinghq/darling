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

// The private interface between UIFoundation's collection view layouts and AppKit's NSCollectionView. AppKit sends
// and implements these messages by name; it declares them itself, as it doesn't link against this header.

#import <AppKit/NSCollectionViewLayout.h>
#import <AppKit/NSCollectionViewLayoutInvalidationContext.h>

@interface NSCollectionViewLayout (NSCollectionViewPrivate)
// Called by NSCollectionView when it takes the layout (with itself) and when it gives it up (with nil)
- (void) _setCollectionView: (NSCollectionView *) collectionView;
// What registerClass:/registerNib:forDecorationViewOfKind: registered for the kind: a Class, an NSNib or nil
- (id) _decorationViewSourceOfKind: (NSCollectionViewDecorationElementKind) elementKind;
@end

@interface NSCollectionViewLayoutInvalidationContext (NSCollectionViewPrivate)
// Set by NSCollectionView's reloadData
- (void) _setInvalidateEverything: (BOOL) invalidate;
- (void) _setInvalidateDataSourceCounts: (BOOL) invalidate;
@end

// Implemented by NSCollectionView: its layout became invalid, so it lays out its items again on its next layout pass
@interface NSObject (NSCollectionViewLayoutObserver)
- (void) _collectionViewLayout: (NSCollectionViewLayout *) layout
      didInvalidateWithContext: (NSCollectionViewLayoutInvalidationContext *) context;
@end
