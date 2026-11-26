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

#import "SPWindowController.h"
#import "SPGridView.h"
#import <PreferencePanes/PreferencePanes.h>

// What a pane may implement to get the anchor of an x-apple.systempreferences URL
// (see -showPane:anchor: in the header)
@protocol SPAnchorPane <NSObject>
- (void)handleSystemPreferencesAnchor:(NSString *)anchor;
@end

static const CGFloat kBarHeight = 44;
static const CGFloat kMinimumWidth = 460;

@implementation SPWindowController {
    NSWindow *_window;
    SPPaneCatalog *_catalog;
    SPGridView *_grid;
    NSView *_container;
    NSView *_bar;
    NSButton *_backButton, *_forwardButton;
    NSSearchField *_search;

    SPPane *_currentPane;
    NSPreferencePane *_currentController;
    id _frameObserver;     // watches the main view of the shown pane for size changes
    NSMutableDictionary<NSString *, NSPreferencePane *> *_controllers;   // loaded panes by identifier

    // navigation history: pane identifiers, NSNull for the grid
    NSMutableArray *_history;
    NSInteger _historyIndex;
    BOOL _navigating;      // a leave request is waiting for its answer
}

@synthesize window = _window, catalog = _catalog, currentPane = _currentPane;

- (instancetype)initWithCatalog:(SPPaneCatalog *)catalog
{
    self = [super init];
    if (self == nil)
        return nil;
    _catalog = catalog;
    _controllers = [NSMutableDictionary dictionary];
    _history = [NSMutableArray array];
    _historyIndex = -1;

    _grid = [[SPGridView alloc] initWithCatalog:catalog];
    _grid.target = self;
    _grid.action = @selector(gridIconClicked:);

    CGFloat width = [SPGridView gridWidth];
    NSRect content = NSMakeRect(0, 0, width, kBarHeight + _grid.preferredHeight);
    _window = [[NSWindow alloc] initWithContentRect:content
                                          styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable
                                            backing:NSBackingStoreBuffered
                                              defer:NO];
    [_window setReleasedWhenClosed:NO];
    [_window setDelegate:self];
    [_window setTitle:NSLocalizedString(@"System Preferences", nil)];
    [_window center];

    NSView *root = [_window contentView];
    _container = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, width, _grid.preferredHeight)];
    [_container setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [root addSubview:_container];

    _bar = [[NSView alloc] initWithFrame:NSMakeRect(0, _grid.preferredHeight, width, kBarHeight)];
    [_bar setAutoresizingMask:NSViewWidthSizable | NSViewMinYMargin];
    [root addSubview:_bar];

    _backButton = [self barButton:@"◀" frame:NSMakeRect(10, 10, 30, 24) action:@selector(goBack:)];
    _forwardButton = [self barButton:@"▶" frame:NSMakeRect(40, 10, 30, 24) action:@selector(goForward:)];
    NSButton *showAll = [self barButton:NSLocalizedString(@"Show All", nil) frame:NSMakeRect(80, 10, 80, 24) action:@selector(showAllClicked:)];
    [showAll setToolTip:NSLocalizedString(@"Show All Preferences", nil)];
    for (NSButton *b in @[ _backButton, _forwardButton, showAll ])
        [_bar addSubview:b];

    _search = [[NSSearchField alloc] initWithFrame:NSMakeRect(width - 190, 11, 180, 22)];
    [_search setAutoresizingMask:NSViewMinXMargin];
    [_search setDelegate:(id)self];
    [_bar addSubview:_search];

    [_container addSubview:_grid];
    [_grid setFrame:NSMakeRect(0, 0, width, _grid.preferredHeight)];

    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(switchToPane:) name:NSPreferencePaneSwitchToPaneNotification object:nil];
    [center addObserver:self selector:@selector(updateHelpMenu:) name:NSPreferencePaneUpdateHelpMenuNotification object:nil];
    [self updateNavigationButtons];
    return self;
}

- (NSButton *)barButton:(NSString *)title frame:(NSRect)frame action:(SEL)action
{
    NSButton *button = [[NSButton alloc] initWithFrame:frame];
    [button setTitle:title];
    [button setBezelStyle:NSBezelStyleRounded];
    [button setTarget:self];
    [button setAction:action];
    [button setAutoresizingMask:NSViewMinYMargin];
    return button;
}

- (void)showWindow
{
    [_window makeKeyAndOrderFront:nil];
}

#pragma mark Navigation

- (void)updateNavigationButtons
{
    [_backButton setEnabled:_historyIndex > 0];
    [_forwardButton setEnabled:_historyIndex >= 0 && _historyIndex + 1 < (NSInteger)[_history count]];
}

- (void)recordHistory:(SPPane *)pane
{
    id entry = pane != nil ? (id)pane.identifier : (id)[NSNull null];
    if (_historyIndex >= 0 && [_history[_historyIndex] isEqual:entry])
        return;
    [_history removeObjectsInRange:NSMakeRange(_historyIndex + 1, [_history count] - _historyIndex - 1)];
    [_history addObject:entry];
    _historyIndex = [_history count] - 1;
}

- (void)goBack:(id)sender
{
    [self goToHistoryIndex:_historyIndex - 1];
}

- (void)goForward:(id)sender
{
    [self goToHistoryIndex:_historyIndex + 1];
}

- (void)goToHistoryIndex:(NSInteger)index
{
    if (index < 0 || index >= (NSInteger)[_history count])
        return;
    id entry = _history[index];
    SPPane *pane = [entry isKindOfClass:[NSString class]] ? [_catalog paneWithIdentifier:entry] : nil;
    [self navigateToPane:pane recordHistory:NO completion:^{
        self->_historyIndex = index;
        [self updateNavigationButtons];
    }];
}

- (void)showAllClicked:(id)sender
{
    [self showAll];
}

- (void)showAll
{
    [self navigateToPane:nil recordHistory:YES completion:nil];
}

- (void)showPane:(SPPane *)pane
{
    [self navigateToPane:pane recordHistory:YES completion:nil];
}

- (void)showPane:(SPPane *)pane anchor:(NSString *)anchor
{
    [self navigateToPane:pane recordHistory:YES completion:^{
        [self deliverAnchor:anchor toPane:pane];
    }];
}

// The anchor of an x-apple.systempreferences URL (the text after the question mark) goes to
// the pane on show if it implements -handleSystemPreferencesAnchor:, see the header.
- (void)deliverAnchor:(NSString *)anchor toPane:(SPPane *)pane
{
    if ([anchor length] == 0 || pane == nil || pane != _currentPane)
        return;
    if ([_currentController respondsToSelector:@selector(handleSystemPreferencesAnchor:)])
        [(id<SPAnchorPane>)_currentController handleSystemPreferencesAnchor:anchor];
}

- (void)gridIconClicked:(SPGridView *)grid
{
    [self showPane:grid.clickedPane];
}

// Leaves what is on show (asking the pane) and shows the target.
- (void)navigateToPane:(SPPane *)pane recordHistory:(BOOL)record completion:(void (^)(void))completion
{
    if (_navigating)
        return;
    if (pane != nil && pane == _currentPane) {
        [_window makeKeyAndOrderFront:nil];
        if (completion != nil)
            completion();
        return;
    }
    if (pane == nil && _currentPane == nil) {
        [_window makeKeyAndOrderFront:nil];
        return;
    }

    _navigating = YES;
    [self requestLeavingCurrentPane:^(BOOL proceed) {
        self->_navigating = NO;
        if (!proceed)
            return;
        [self leaveCurrentPane];
        if (pane != nil)
            [self enterPane:pane];
        else
            [self enterGrid];
        if (record)
            [self recordHistory:pane];
        [self updateNavigationButtons];
        if (completion != nil)
            completion();
        [self->_window makeKeyAndOrderFront:nil];
    }];
}

- (void)switchToPane:(NSNotification *)notification
{
    // A pane asks for another pane by its identifier (nil or empty: the grid).
    id object = [notification object];
    NSString *identifier = [object isKindOfClass:[NSString class]] ? object : nil;
    SPPane *target = [identifier length] != 0 ? [_catalog paneWithIdentifier:identifier] : nil;
    if ([identifier length] != 0 && target == nil)
        return;
    [self navigateToPane:target recordHistory:YES completion:nil];
}

#pragma mark Leaving a pane

- (void)requestLeavingCurrentPane:(void (^)(BOOL))completion
{
    NSPreferencePane *controller = _currentController;
    if (controller == nil) {
        completion(YES);
        return;
    }

    // text fields that are being edited give up their contents first: the window takes
    // the first responder role (the field editor ends), which a field can refuse
    if ([controller autoSaveTextFields] && ![_window makeFirstResponder:_window]) {
        completion(NO);
        return;
    }

    switch ([controller shouldUnselect]) {
    case NSUnselectNow:
        completion(YES);
        break;
    case NSUnselectLater: {
        // the pane answers with replyToShouldUnselect:, which posts one of two notifications
        NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
        __block id doToken = nil, cancelToken = nil;
        void (^finish)(BOOL) = ^(BOOL proceed) {
            [center removeObserver:doToken];
            [center removeObserver:cancelToken];
            completion(proceed);
        };
        doToken = [center addObserverForName:NSPreferencePaneDoUnselectNotification object:controller queue:nil
                                  usingBlock:^(NSNotification *n) { finish(YES); }];
        cancelToken = [center addObserverForName:NSPreferencePaneCancelUnselectNotification object:controller queue:nil
                                      usingBlock:^(NSNotification *n) { finish(NO); }];
        break;
    }
    default:
        completion(NO);
        break;
    }
}

- (void)leaveCurrentPane
{
    NSPreferencePane *controller = _currentController;
    [self stopFollowingMainView];
    if (controller != nil) {
        [controller willUnselect];
        [[controller mainView] removeFromSuperview];
        [controller didUnselect];
    }
    _currentController = nil;
    _currentPane = nil;
    if (_helpItemsChanged != nil)
        _helpItemsChanged(nil);
}

#pragma mark Showing

- (void)resizeContentTo:(NSSize)size
{
    NSRect old = [_window frame];
    NSRect frame = [_window frameRectForContentRect:NSMakeRect(0, 0, size.width, size.height + kBarHeight)];
    frame.origin.x = old.origin.x;
    frame.origin.y = NSMaxY(old) - frame.size.height;
    [_window setFrame:frame display:YES animate:NO];
    [_container setFrame:NSMakeRect(0, 0, size.width, size.height)];
    [_bar setFrame:NSMakeRect(0, size.height, size.width, kBarHeight)];
}

// The window follows the main view of the shown pane when the pane changes its size, as it
// does when the pane is selected.
- (void)followMainView:(NSView *)view
{
    [self stopFollowingMainView];
    [view setPostsFrameChangedNotifications:YES];
    __weak SPWindowController *weakSelf = self;
    _frameObserver = [[NSNotificationCenter defaultCenter]
        addObserverForName:NSViewFrameDidChangeNotification object:view queue:nil
                usingBlock:^(NSNotification *n) { [weakSelf mainViewFrameChanged:[n object]]; }];
}

- (void)stopFollowingMainView
{
    if (_frameObserver != nil)
        [[NSNotificationCenter defaultCenter] removeObserver:_frameObserver];
    _frameObserver = nil;
}

- (void)mainViewFrameChanged:(NSView *)view
{
    if (_currentController == nil || [_currentController mainView] != view)
        return;
    NSSize size = [view frame].size;
    CGFloat width = MAX(size.width, kMinimumWidth);
    [self resizeContentTo:NSMakeSize(width, size.height)];
    [view setFrameOrigin:NSMakePoint(floor((width - size.width) / 2), 0)];
}

- (void)enterGrid
{
    [_grid reloadData];
    [_grid setFrame:NSMakeRect(0, 0, [SPGridView gridWidth], _grid.preferredHeight)];
    [self resizeContentTo:NSMakeSize([SPGridView gridWidth], _grid.preferredHeight)];
    [_container addSubview:_grid];
    [_window setTitle:NSLocalizedString(@"System Preferences", nil)];
    [_search setStringValue:@""];
    [_grid setSearchText:@""];
    [_window makeFirstResponder:_search];
    [[NSUserDefaults standardUserDefaults] removeObjectForKey:@"LastSelectedPane"];
}

- (NSPreferencePane *)controllerForPane:(SPPane *)pane
{
    NSPreferencePane *controller = _controllers[pane.identifier];
    if (controller != nil)
        return controller;

    NSBundle *bundle = pane.bundle;
    Class principal = [bundle load] ? [bundle principalClass] : Nil;
    if (principal == Nil || ![principal isSubclassOfClass:[NSPreferencePane class]]) {
        NSAlert *alert = [[NSAlert alloc] init];
        [alert setAlertStyle:NSAlertStyleWarning];
        [alert setMessageText:[NSString stringWithFormat:NSLocalizedString(@"The “%@” preferences can’t be opened.", nil), pane.label]];
        [alert setInformativeText:NSLocalizedString(@"The preference pane could not be loaded.", nil)];
        [alert runModal];
        return nil;
    }
    controller = [[principal alloc] initWithBundle:bundle];
    _controllers[pane.identifier] = controller;
    return controller;
}

- (void)enterPane:(SPPane *)pane
{
    NSPreferencePane *controller = [self controllerForPane:pane];
    if (controller == nil) {
        [self enterGrid];
        return;
    }

    NSView *view = [controller mainView];
    if (view == nil)
        view = [controller loadMainView];
    [controller willSelect];

    [_grid removeFromSuperview];
    NSSize size = [view frame].size;
    CGFloat width = MAX(size.width, kMinimumWidth);
    [self resizeContentTo:NSMakeSize(width, size.height)];
    [view setFrameOrigin:NSMakePoint(floor((width - size.width) / 2), 0)];
    [view setAutoresizingMask:NSViewMinXMargin | NSViewMaxXMargin];
    [_container addSubview:view];
    [self followMainView:view];

    _currentPane = pane;
    _currentController = controller;
    [_window setTitle:pane.title];
    [controller didSelect];

    NSView *first = [controller firstKeyView];
    if (first != nil)
        [_window setInitialFirstResponder:first];
    NSView *initial = [controller initialKeyView];
    if (initial != nil)
        [_window makeFirstResponder:initial];

    [[NSUserDefaults standardUserDefaults] setObject:pane.identifier forKey:@"LastSelectedPane"];
    [self publishStaticHelpItems:pane];
}

#pragma mark Search

- (void)controlTextDidChange:(NSNotification *)notification
{
    if ([notification object] != _search)
        return;
    NSString *text = [_search stringValue];
    if (_currentPane != nil && [text length] != 0) {
        // searching from a pane goes back to the grid, where the matches are marked
        [self navigateToPane:nil recordHistory:YES completion:^{
            [self->_search setStringValue:text];
            [self->_grid setSearchText:text];
        }];
        return;
    }
    [_grid setSearchText:text];
}

#pragma mark Help menu

- (void)publishStaticHelpItems:(SPPane *)pane
{
    id items = [pane.bundle objectForInfoDictionaryKey:NSPrefPaneHelpMenuInfoPListKey];
    if (_helpItemsChanged != nil)
        _helpItemsChanged([items isKindOfClass:[NSArray class]] ? items : nil);
}

- (void)updateHelpMenu:(NSNotification *)notification
{
    // the notification's object is the array of items, see -[NSPreferencePane updateHelpMenuWithArray:]
    id items = [notification object];
    if (_helpItemsChanged != nil)
        _helpItemsChanged([items isKindOfClass:[NSArray class]] ? items : nil);
}

#pragma mark NSWindowDelegate

- (BOOL)windowShouldClose:(NSWindow *)sender
{
    // closing leaves the pane like any other way out; the application ends with its window
    if (_currentController == nil)
        return YES;
    if (_navigating)
        return NO;
    _navigating = YES;
    [self requestLeavingCurrentPane:^(BOOL proceed) {
        self->_navigating = NO;
        if (proceed) {
            [self leaveCurrentPane];
            [self->_window close];
        }
    }];
    return NO;
}

@end
