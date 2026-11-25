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

#import "SPAppDelegate.h"
#import "SPWindowController.h"
#import <PreferencePanes/PreferencePanes.h>

static NSString * const kURLScheme = @"x-apple.systempreferences";

@implementation SPAppDelegate {
    SPPaneCatalog *_catalog;
    SPWindowController *_controller;
    NSMenu *_viewMenu, *_helpMenu;
    BOOL _opened;     // something was asked for (a document or URL) since launch
}

- (void)applicationWillFinishLaunching:(NSNotification *)notification
{
    NSString *groups = [[NSBundle mainBundle] pathForResource:@"NSPrefPaneGroups" ofType:@"plist"];
    _catalog = [[SPPaneCatalog alloc] initWithSearchDirectories:[SPPaneCatalog defaultSearchDirectories] groupsPath:groups];
    _controller = [[SPWindowController alloc] initWithCatalog:_catalog];
    __weak SPAppDelegate *weakSelf = self;
    _controller.helpItemsChanged = ^(NSArray<NSDictionary *> *items) {
        [weakSelf rebuildHelpMenu:items];
    };
    [self buildMenus];
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    // what the launch itself asked for arrives around now; without any, the last pane is shown
    dispatch_async(dispatch_get_main_queue(), ^{
        if (!self->_opened) {
            NSString *last = [[NSUserDefaults standardUserDefaults] stringForKey:@"LastSelectedPane"];
            SPPane *pane = [self->_catalog paneWithIdentifier:last];
            if (pane != nil)
                [self->_controller showPane:pane];
        }
        [self->_controller showWindow];
    });
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    return YES;
}

- (BOOL)applicationShouldHandleReopen:(NSApplication *)sender hasVisibleWindows:(BOOL)flag
{
    [_controller showWindow];
    return YES;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
    // the shown pane is asked first; it may answer later
    __block BOOL answered = NO, asynchronous = NO;
    __block NSApplicationTerminateReply reply = NSTerminateCancel;
    [_controller requestLeavingCurrentPane:^(BOOL proceed) {
        if (proceed)
            [self->_controller leaveCurrentPane];
        if (asynchronous)
            [NSApp replyToApplicationShouldTerminate:proceed];
        else {
            answered = YES;
            reply = proceed ? NSTerminateNow : NSTerminateCancel;
        }
    }];
    if (answered)
        return reply;
    asynchronous = YES;
    return NSTerminateLater;
}

#pragma mark Opening documents and URLs

// x-apple.systempreferences:<pane identifier>[?<anchor>]; the anchor is a position
// inside a pane that the pane interprets (see -[SPWindowController showPane:anchor:])
static NSString *RestOfURL(NSURL *url)
{
    NSString *rest = [[url absoluteString] substringFromIndex:[kURLScheme length] + 1];
    if ([rest hasPrefix:@"//"])
        rest = [rest substringFromIndex:2];
    return rest;
}

static NSString *AnchorOfURL(NSURL *url)
{
    NSString *rest = RestOfURL(url);
    NSRange start = [rest rangeOfString:@"?"];
    if (start.location == NSNotFound)
        return nil;
    return [[rest substringFromIndex:NSMaxRange(start)] stringByRemovingPercentEncoding];
}

static NSString *PaneIdentifierOfURL(NSURL *url)
{
    NSString *rest = RestOfURL(url);
    NSRange cut = [rest rangeOfCharacterFromSet:[NSCharacterSet characterSetWithCharactersInString:@"?#"]];
    if (cut.location != NSNotFound)
        rest = [rest substringToIndex:cut.location];
    return [rest stringByRemovingPercentEncoding];
}

- (void)application:(NSApplication *)application openURLs:(NSArray<NSURL *> *)urls
{
    for (NSURL *url in urls) {
        SPPane *pane = nil;
        NSString *anchor = nil;
        if ([url isFileURL]) {
            NSString *path = [[url URLByStandardizingPath] path];
            if (![[path pathExtension] isEqualToString:@"prefPane"])
                continue;
            pane = [_catalog paneByAddingBundleAtPath:path];
            [self rebuildViewMenu];
        } else if ([[[url scheme] lowercaseString] isEqualToString:kURLScheme]) {
            NSString *identifier = PaneIdentifierOfURL(url);
            if ([identifier length] == 0) {
                // the bare scheme only brings the application forward
                _opened = YES;
                [_controller showWindow];
                continue;
            }
            pane = [_catalog paneWithIdentifier:identifier];
            anchor = AnchorOfURL(url);
        } else {
            continue;
        }
        _opened = YES;
        // an unknown identifier leaves the window as it is
        if (pane != nil)
            [_controller showPane:pane anchor:anchor];
        [_controller showWindow];
    }
}

#pragma mark Menus

- (NSMenuItem *)addItemTo:(NSMenu *)menu title:(NSString *)title action:(SEL)action key:(NSString *)key
{
    NSMenuItem *item = [menu addItemWithTitle:title action:action keyEquivalent:key];
    return item;
}

- (void)buildMenus
{
    NSMenu *bar = [[NSMenu alloc] init];

    NSMenuItem *appItem = [bar addItemWithTitle:@"" action:NULL keyEquivalent:@""];
    NSMenu *app = [[NSMenu alloc] initWithTitle:NSLocalizedString(@"System Preferences", nil)];
    [self addItemTo:app title:NSLocalizedString(@"About System Preferences", nil) action:@selector(orderFrontStandardAboutPanel:) key:@""];
    [app addItem:[NSMenuItem separatorItem]];
    [self addItemTo:app title:NSLocalizedString(@"Hide System Preferences", nil) action:@selector(hide:) key:@"h"];
    [app addItem:[NSMenuItem separatorItem]];
    [self addItemTo:app title:NSLocalizedString(@"Quit System Preferences", nil) action:@selector(terminate:) key:@"q"];
    [appItem setSubmenu:app];

    NSMenuItem *editItem = [bar addItemWithTitle:NSLocalizedString(@"Edit", nil) action:NULL keyEquivalent:@""];
    NSMenu *edit = [[NSMenu alloc] initWithTitle:NSLocalizedString(@"Edit", nil)];
    [self addItemTo:edit title:NSLocalizedString(@"Undo", nil) action:@selector(undo:) key:@"z"];
    [self addItemTo:edit title:NSLocalizedString(@"Redo", nil) action:@selector(redo:) key:@"Z"];
    [edit addItem:[NSMenuItem separatorItem]];
    [self addItemTo:edit title:NSLocalizedString(@"Cut", nil) action:@selector(cut:) key:@"x"];
    [self addItemTo:edit title:NSLocalizedString(@"Copy", nil) action:@selector(copy:) key:@"c"];
    [self addItemTo:edit title:NSLocalizedString(@"Paste", nil) action:@selector(paste:) key:@"v"];
    [self addItemTo:edit title:NSLocalizedString(@"Select All", nil) action:@selector(selectAll:) key:@"a"];
    [editItem setSubmenu:edit];

    NSMenuItem *viewItem = [bar addItemWithTitle:NSLocalizedString(@"View", nil) action:NULL keyEquivalent:@""];
    _viewMenu = [[NSMenu alloc] initWithTitle:NSLocalizedString(@"View", nil)];
    [viewItem setSubmenu:_viewMenu];
    [self rebuildViewMenu];

    NSMenuItem *windowItem = [bar addItemWithTitle:NSLocalizedString(@"Window", nil) action:NULL keyEquivalent:@""];
    NSMenu *window = [[NSMenu alloc] initWithTitle:NSLocalizedString(@"Window", nil)];
    [self addItemTo:window title:NSLocalizedString(@"Minimize", nil) action:@selector(performMiniaturize:) key:@"m"];
    [self addItemTo:window title:NSLocalizedString(@"Close", nil) action:@selector(performClose:) key:@"w"];
    [windowItem setSubmenu:window];
    [NSApp setWindowsMenu:window];

    NSMenuItem *helpItem = [bar addItemWithTitle:NSLocalizedString(@"Help", nil) action:NULL keyEquivalent:@""];
    _helpMenu = [[NSMenu alloc] initWithTitle:NSLocalizedString(@"Help", nil)];
    [helpItem setSubmenu:_helpMenu];
    [NSApp setHelpMenu:_helpMenu];
    [self rebuildHelpMenu:nil];

    [NSApp setMainMenu:bar];
}

- (void)rebuildViewMenu
{
    [_viewMenu removeAllItems];
    NSMenuItem *showAll = [_viewMenu addItemWithTitle:NSLocalizedString(@"Show All Preferences", nil) action:@selector(showAllPreferences:) keyEquivalent:@"l"];
    [showAll setTarget:self];
    [_viewMenu addItem:[NSMenuItem separatorItem]];
    NSArray *panes = [[_catalog allPanes] sortedArrayUsingComparator:^NSComparisonResult(SPPane *a, SPPane *b) {
        return [a.label localizedCaseInsensitiveCompare:b.label];
    }];
    for (SPPane *pane in panes) {
        NSMenuItem *item = [_viewMenu addItemWithTitle:pane.label action:@selector(showPaneFromMenu:) keyEquivalent:@""];
        [item setTarget:self];
        [item setRepresentedObject:pane.identifier];
    }
}

- (void)rebuildHelpMenu:(NSArray<NSDictionary *> *)items
{
    [_helpMenu removeAllItems];
    for (NSDictionary *entry in items) {
        NSString *title = entry[NSPrefPaneHelpMenuTitleKey], *anchor = entry[NSPrefPaneHelpMenuAnchorKey];
        if (![title isKindOfClass:[NSString class]] || ![anchor isKindOfClass:[NSString class]])
            continue;
        NSMenuItem *item = [_helpMenu addItemWithTitle:title action:@selector(openHelpAnchorFromMenu:) keyEquivalent:@""];
        [item setTarget:self];
        [item setRepresentedObject:anchor];
    }
    if ([_helpMenu numberOfItems] == 0) {
        NSMenuItem *none = [_helpMenu addItemWithTitle:NSLocalizedString(@"No Help Available", nil) action:NULL keyEquivalent:@""];
        [none setEnabled:NO];
    }
}

- (void)showAllPreferences:(id)sender
{
    [_controller showAll];
}

- (void)showPaneFromMenu:(NSMenuItem *)sender
{
    SPPane *pane = [_catalog paneWithIdentifier:[sender representedObject]];
    if (pane != nil)
        [_controller showPane:pane];
}

- (void)openHelpAnchorFromMenu:(NSMenuItem *)sender
{
    NSBundle *bundle = _controller.currentPane.bundle;
    NSString *book = [bundle objectForInfoDictionaryKey:@"CFBundleHelpBookName"];
    if ([book isKindOfClass:[NSString class]])
        [[NSHelpManager sharedHelpManager] openHelpAnchor:[sender representedObject] inBook:book];
}

@end
