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

#import "SPPaneCatalog.h"

// The grid of pane icons ("Show All"): the catalog's groups one under another,
// separated by lines, the catch-all group headed "Other".
@interface SPGridView : NSView

- (instancetype)initWithCatalog:(SPPaneCatalog *)catalog;

// The view height the current catalog needs at the fixed grid width.
@property (readonly) CGFloat preferredHeight;
+ (CGFloat)gridWidth;

// Called with the clicked icon's pane.
@property (weak) id target;
@property SEL action;
@property (readonly, strong) SPPane *clickedPane;

// Icons whose pane does not match the text are dimmed; empty text dims none.
@property (nonatomic, copy) NSString *searchText;

// Rebuilds the icons from the catalog.
- (void)reloadData;

@end
