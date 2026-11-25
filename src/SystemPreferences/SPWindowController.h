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

// The application's window: a bar with Back, Forward, Show All and a search
// field above either the grid of panes or one hosted preference pane.
//
// A pane is hosted through the NSPreferencePane lifecycle: loadMainView, then
// willSelect, the view in the window (which is resized to it), didSelect; and
// on the way out shouldUnselect (possibly answered later through
// replyToShouldUnselect:), willUnselect, didUnselect.
@interface SPWindowController : NSObject <NSWindowDelegate>

- (instancetype)initWithCatalog:(SPPaneCatalog *)catalog;

@property (readonly, strong) NSWindow *window;
@property (readonly, strong) SPPaneCatalog *catalog;
// The pane on show, nil for the grid.
@property (readonly, strong) SPPane *currentPane;

- (void)showWindow;
- (void)showAll;
- (void)showPane:(SPPane *)pane;
// Shows the pane, then hands it the anchor of an x-apple.systempreferences URL (the text after
// the question mark). The public NSPreferencePane has no method for an anchor, so the host has a
// convention of its own: a pane that implements
//     - (void)handleSystemPreferencesAnchor:(NSString *)anchor;
// is sent the percent-decoded anchor after didSelect, or while it is on show when the URL is
// opened again. A pane without the method ignores the anchor. What an anchor names (a tab, a
// list item) is up to the pane.
- (void)showPane:(SPPane *)pane anchor:(NSString *)anchor;

// Asks the shown pane whether it may go away (see above) and calls the block
// with the answer; the block is called at once when no pane is shown.
- (void)requestLeavingCurrentPane:(void (^)(BOOL proceed))completion;
// Ends the shown pane's selection; call only after a positive answer.
- (void)leaveCurrentPane;

// The pane titles for the Help menu: the shown pane's help items.
@property (copy) void (^helpItemsChanged)(NSArray<NSDictionary *> *items);

@end
