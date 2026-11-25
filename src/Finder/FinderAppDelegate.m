/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

#import "FinderAppDelegate.h"
#import "FinderDirectory.h"
#import "FinderWindowController.h"

// A folder's path as windows are looked up by it: standardized, without a trailing slash
static NSString *FinderFolderKey(NSURL *url)
{
    return [[url URLByStandardizingPath] path];
}

@implementation FinderAppDelegate {
    NSMutableArray<FinderWindowController *> *_controllers;
    NSPoint _cascadePoint;
}

- (instancetype)init
{
    self = [super init];
    if (self != nil)
        _controllers = [NSMutableArray array];
    return self;
}

- (NSArray<FinderWindowController *> *)windowControllers
{
    NSMutableArray<FinderWindowController *> *ordered = [NSMutableArray array];
    for (NSWindow *window in [NSApp orderedWindows]) {
        for (FinderWindowController *controller in _controllers) {
            if (controller.window == window)
                [ordered addObject:controller];
        }
    }
    for (FinderWindowController *controller in _controllers) {
        if (![ordered containsObject:controller])
            [ordered addObject:controller];
    }
    return ordered;
}

#pragma mark Windows

- (FinderWindowController *)windowForFolder:(NSURL *)folder create:(BOOL)create
{
    NSString *key = FinderFolderKey(folder);
    for (FinderWindowController *controller in _controllers) {
        if ([FinderFolderKey(controller.folderURL) isEqual:key])
            return controller;
    }
    if (!create)
        return nil;

    NSError *error = nil;
    FinderDirectory *directory = [[FinderDirectory alloc] initWithURL:folder];
    if (![directory reload:&error]) {
        [self reportFolder:folder error:error];
        return nil;
    }

    FinderWindowController *controller = [[FinderWindowController alloc] initWithFolderURL:folder];
    __weak FinderAppDelegate *weakSelf = self;
    __weak FinderWindowController *weakController = controller;
    controller.openItem = ^(NSURL *item) {
        [weakSelf openItem:item inWindow:weakController];
    };
    [_controllers addObject:controller];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(windowWillClose:)
                                                 name:NSWindowWillCloseNotification
                                               object:controller.window];
    if (NSEqualPoints(_cascadePoint, NSZeroPoint))
        [controller.window center];
    _cascadePoint = [controller.window cascadeTopLeftFromPoint:_cascadePoint];
    [controller showWindow:nil];
    return controller;
}

- (void)windowWillClose:(NSNotification *)notification
{
    NSWindow *window = [notification object];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSWindowWillCloseNotification object:window];
    for (FinderWindowController *controller in [_controllers copy]) {
        if (controller.window == window)
            [_controllers removeObject:controller];
    }
}

- (void)reportFolder:(NSURL *)folder error:(NSError *)error
{
    NSAlert *alert = [FinderWindowController alertForFolder:folder error:error];
    [NSApp activateIgnoringOtherApps:YES];
    [alert runModal];
}

#pragma mark Opening

// Folders and volumes open in their own window; packages, like other files, are items to reveal
- (BOOL)opensAsFolder:(NSURL *)url
{
    BOOL directory = NO;
    if (![[NSFileManager defaultManager] fileExistsAtPath:[url path] isDirectory:&directory] || !directory)
        return NO;
    return ![[NSWorkspace sharedWorkspace] isFilePackageAtPath:[url path]];
}

- (void)openURLs:(NSArray<NSURL *> *)urls
{
    NSFileManager *fm = [NSFileManager defaultManager];
    NSMutableArray<NSURL *> *folders = [NSMutableArray array];
    NSMutableArray<NSString *> *revealFolders = [NSMutableArray array];
    NSMutableDictionary<NSString *, NSMutableArray<NSURL *> *> *revealed = [NSMutableDictionary dictionary];

    for (NSURL *url in urls) {
        if (![url isFileURL] || ![fm fileExistsAtPath:[url path]])
            continue;
        NSURL *item = [NSURL fileURLWithPath:FinderFolderKey(url)];
        if ([self opensAsFolder:item]) {
            [folders addObject:item];
            continue;
        }
        NSString *parent = FinderFolderKey([item URLByDeletingLastPathComponent]);
        if (revealed[parent] == nil) {
            revealed[parent] = [NSMutableArray array];
            [revealFolders addObject:parent];
        }
        [revealed[parent] addObject:item];
    }

    // A window that is open already is read again: what is revealed may have appeared since
    for (NSURL *folder in folders)
        [[self windowForFolder:folder create:NO] reloadFolder];
    for (NSString *parent in revealFolders)
        [[self windowForFolder:[NSURL fileURLWithPath:parent isDirectory:YES] create:NO] reloadFolder];

    FinderWindowController *last = nil;
    for (NSURL *folder in folders) {
        FinderWindowController *controller = [self windowForFolder:folder create:YES];
        if (controller != nil)
            last = controller;
    }
    for (NSString *parent in revealFolders) {
        FinderWindowController *controller = [self windowForFolder:[NSURL fileURLWithPath:parent isDirectory:YES]
                                                            create:YES];
        if (controller == nil)
            continue;
        [controller selectItems:revealed[parent]];
        last = controller;
    }

    if (last != nil) {
        [last showWindow:nil];
        [NSApp activateIgnoringOtherApps:YES];
    } else if ([_controllers count] == 0 && [urls count] > 0) {
        // Nothing could be shown, and no window is left to keep the Finder running
        [NSApp terminate:nil];
    }
}

- (void)openItem:(NSURL *)item inWindow:(FinderWindowController *)controller
{
    if ([self opensAsFolder:item]) {
        FinderWindowController *existing = [self windowForFolder:item create:NO];
        if (existing != nil && existing != controller) {
            [existing showWindow:nil];
            return;
        }
        [controller showFolder:item];
        return;
    }
    [[NSWorkspace sharedWorkspace] openURL:item];
}

#pragma mark NSApplicationDelegate

- (void)applicationWillFinishLaunching:(NSNotification *)notification
{
    [NSApp setMainMenu:[self mainMenu]];
    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self
                                                           selector:@selector(volumeDidUnmount:)
                                                               name:NSWorkspaceDidUnmountNotification
                                                             object:nil];
}

// A volume went away, whoever unmounted it: its windows, and those of folders on it, close
- (void)volumeDidUnmount:(NSNotification *)notification
{
    NSURL *volume = notification.userInfo[NSWorkspaceVolumeURLKey];
    NSString *path = volume != nil ? FinderFolderKey(volume) : notification.userInfo[@"NSDevicePath"];
    if (path == nil)
        return;
    NSString *prefix = [path hasSuffix:@"/"] ? path : [path stringByAppendingString:@"/"];
    for (FinderWindowController *controller in [_controllers copy]) {
        NSString *folder = FinderFolderKey(controller.folderURL);
        if ([folder isEqual:path] || [folder hasPrefix:prefix])
            [controller close];
    }
}

- (void)application:(NSApplication *)application openURLs:(NSArray<NSURL *> *)urls
{
    [self openURLs:urls];
}

// Launched without documents, or reopened with no window: a window on the home folder
- (BOOL)applicationOpenUntitledFile:(NSApplication *)sender
{
    return [self windowForFolder:[NSURL fileURLWithPath:NSHomeDirectory() isDirectory:YES] create:YES] != nil;
}

- (BOOL)applicationShouldHandleReopen:(NSApplication *)sender hasVisibleWindows:(BOOL)flag
{
    return YES;
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    return YES;
}

#pragma mark Menus

- (NSMenu *)mainMenu
{
    NSMenu *main = [[NSMenu alloc] initWithTitle:@"Main"];

    NSMenu *finder = [[NSMenu alloc] initWithTitle:@"Finder"];
    [finder addItemWithTitle:@"About Finder" action:@selector(orderFrontStandardAboutPanel:) keyEquivalent:@""];
    [finder addItem:[NSMenuItem separatorItem]];
    [finder addItemWithTitle:@"Hide Finder" action:@selector(hide:) keyEquivalent:@"h"];
    [finder addItemWithTitle:@"Quit Finder" action:@selector(terminate:) keyEquivalent:@"q"];
    [[main addItemWithTitle:@"Finder" action:NULL keyEquivalent:@""] setSubmenu:finder];

    NSMenu *file = [[NSMenu alloc] initWithTitle:@"File"];
    NSMenuItem *newFolder = [file addItemWithTitle:@"New Folder" action:@selector(newFolder:) keyEquivalent:@"n"];
    newFolder.keyEquivalentModifierMask = NSEventModifierFlagShift | NSEventModifierFlagCommand;
    [file addItemWithTitle:@"Open" action:@selector(openSelection:) keyEquivalent:@"o"];
    [file addItemWithTitle:@"Close Window" action:@selector(performClose:) keyEquivalent:@"w"];
    [file addItem:[NSMenuItem separatorItem]];
    [file addItemWithTitle:@"Get Info" action:@selector(getInfo:) keyEquivalent:@"i"];
    [file addItemWithTitle:@"Rename" action:@selector(rename:) keyEquivalent:@""];
    [file addItem:[NSMenuItem separatorItem]];
    [file addItemWithTitle:@"Move to Trash" action:@selector(moveToTrash:)
             keyEquivalent:[NSString stringWithFormat:@"%C", (unichar) NSBackspaceCharacter]];
    [file addItem:[NSMenuItem separatorItem]];
    [file addItemWithTitle:@"Eject" action:@selector(eject:) keyEquivalent:@"e"];
    [[main addItemWithTitle:@"File" action:NULL keyEquivalent:@""] setSubmenu:file];

    NSMenu *edit = [[NSMenu alloc] initWithTitle:@"Edit"];
    [edit addItemWithTitle:@"Copy" action:@selector(copy:) keyEquivalent:@"c"];
    [edit addItemWithTitle:@"Paste" action:@selector(paste:) keyEquivalent:@"v"];
    // Paste's alternate: Option shows it in its place
    NSMenuItem *move = [edit addItemWithTitle:@"Move Item Here" action:@selector(moveItemHere:) keyEquivalent:@"v"];
    move.keyEquivalentModifierMask = NSEventModifierFlagOption | NSEventModifierFlagCommand;
    move.alternate = YES;
    [edit addItemWithTitle:@"Select All" action:@selector(selectAll:) keyEquivalent:@"a"];
    [[main addItemWithTitle:@"Edit" action:NULL keyEquivalent:@""] setSubmenu:edit];

    NSMenu *view = [[NSMenu alloc] initWithTitle:@"View"];
    [view addItemWithTitle:@"as Icons" action:@selector(showAsIcons:) keyEquivalent:@"1"];
    [view addItemWithTitle:@"as List" action:@selector(showAsList:) keyEquivalent:@"2"];
    [view addItem:[NSMenuItem separatorItem]];
    NSMenuItem *toolbar = [view addItemWithTitle:@"Hide Toolbar" action:@selector(toggleToolbar:) keyEquivalent:@"t"];
    toolbar.keyEquivalentModifierMask = NSEventModifierFlagOption | NSEventModifierFlagCommand;
    NSMenuItem *sidebar = [view addItemWithTitle:@"Hide Sidebar" action:@selector(toggleSidebar:) keyEquivalent:@"s"];
    sidebar.keyEquivalentModifierMask = NSEventModifierFlagControl | NSEventModifierFlagCommand;
    [[main addItemWithTitle:@"View" action:NULL keyEquivalent:@""] setSubmenu:view];

    NSMenu *go = [[NSMenu alloc] initWithTitle:@"Go"];
    [go addItemWithTitle:@"Back" action:@selector(goBack:) keyEquivalent:@"["];
    [go addItemWithTitle:@"Forward" action:@selector(goForward:) keyEquivalent:@"]"];
    [go addItemWithTitle:@"Enclosing Folder"
                  action:@selector(goToEnclosingFolder:)
           keyEquivalent:[NSString stringWithFormat:@"%C", (unichar) NSUpArrowFunctionKey]];
    [go addItemWithTitle:@"Selected Item"
                  action:@selector(openSelection:)
           keyEquivalent:[NSString stringWithFormat:@"%C", (unichar) NSDownArrowFunctionKey]];
    [[main addItemWithTitle:@"Go" action:NULL keyEquivalent:@""] setSubmenu:go];

    NSMenu *window = [[NSMenu alloc] initWithTitle:@"Window"];
    [window addItemWithTitle:@"Minimize" action:@selector(performMiniaturize:) keyEquivalent:@"m"];
    [window addItemWithTitle:@"Zoom" action:@selector(performZoom:) keyEquivalent:@""];
    [[main addItemWithTitle:@"Window" action:NULL keyEquivalent:@""] setSubmenu:window];
    [NSApp setWindowsMenu:window];

    return main;
}

@end
