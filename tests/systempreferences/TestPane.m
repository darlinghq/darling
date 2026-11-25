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

// A preference pane for the System Preferences tests (tests/src/systempreferences.m).
// It appends what the host does to it to the file named by the bundle's Info.plist key
// TestPaneLog, and takes commands from the file named by TestPaneMode (one word, read at
// each shouldUnselect and didSelect):
//   now, cancel, later, later-cancel : the answer to shouldUnselect
//   quit-on-select                   : -[NSApp terminate:] shortly after didSelect
//   showall-on-select                : ask the host for the grid shortly after didSelect
//   help-on-select                   : give the host two help menu items after didSelect
//   grow-on-select                   : make the main view 130 points taller (then 130 shorter again)
//                                      shortly after didSelect and log the window's content size
// The pane implements the host's anchor convention, -handleSystemPreferencesAnchor:, and logs it.
// After showall-on-select it describes the grid, after help-on-select the Help menu.
#import <Cocoa/Cocoa.h>
#import <PreferencePanes/PreferencePanes.h>

@interface TestPane : NSPreferencePane
@end

@implementation TestPane

- (NSString *)logPath { return [[[self bundle] infoDictionary] objectForKey:@"TestPaneLog"]; }
- (NSString *)modePath { return [[[self bundle] infoDictionary] objectForKey:@"TestPaneMode"]; }

- (void)note:(NSString *)line
{
	NSString *entry = [NSString stringWithFormat:@"%@ pid=%d %@\n", [[self bundle] bundleIdentifier], getpid(), line];
	NSFileHandle *h = [NSFileHandle fileHandleForWritingAtPath:[self logPath]];
	if (!h) {
		[[NSFileManager defaultManager] createFileAtPath:[self logPath] contents:nil attributes:nil];
		h = [NSFileHandle fileHandleForWritingAtPath:[self logPath]];
	}
	[h seekToEndOfFile];
	[h writeData:[entry dataUsingEncoding:NSUTF8StringEncoding]];
	[h closeFile];
}

- (NSString *)mode
{
	NSString *s = [NSString stringWithContentsOfFile:[self modePath] encoding:NSUTF8StringEncoding error:nil];
	return [s stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
}

- (void)mainViewDidLoad { [self note:@"mainViewDidLoad"]; }
- (void)willSelect { [self note:@"willSelect"]; }

- (void)didSelect
{
	[super didSelect];
	NSWindow *w = [[self mainView] window];
	NSRect cv = [[w contentView] frame], mv = [[self mainView] frame];
	[self note:[NSString stringWithFormat:@"didSelect window=%d title=%@ content=%gx%g mainView=%gx%g selected=%d key=%@", w != nil, [w title], cv.size.width, cv.size.height, mv.size.width, mv.size.height, [self isSelected], [self initialKeyView] ? @"set" : @"none"]];

	NSString *mode = [self mode];
	if ([mode isEqualToString:@"quit-on-select"])
		dispatch_after(dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC / 2), dispatch_get_main_queue(), ^{ [NSApp terminate:nil]; });
	else if ([mode isEqualToString:@"showall-on-select"])
		dispatch_after(dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC / 2), dispatch_get_main_queue(), ^{
			[[NSNotificationCenter defaultCenter] postNotificationName:NSPreferencePaneSwitchToPaneNotification object:nil];
			dispatch_after(dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC), dispatch_get_main_queue(), ^{ [self describeGrid]; });
		});
	else if ([mode isEqualToString:@"grow-on-select"])
		dispatch_after(dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC / 2), dispatch_get_main_queue(), ^{
			NSView *v = [self mainView];
			NSSize s = [v frame].size;
			[v setFrameSize:NSMakeSize(s.width, s.height + 130)];
			dispatch_after(dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC / 2), dispatch_get_main_queue(), ^{
				[self note:[NSString stringWithFormat:@"grown content=%g", [[[v window] contentView] frame].size.height]];
				[v setFrameSize:s];
				dispatch_after(dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC / 2), dispatch_get_main_queue(), ^{
					[self note:[NSString stringWithFormat:@"shrunk content=%g", [[[v window] contentView] frame].size.height]];
				});
			});
		});
	else if ([mode isEqualToString:@"help-on-select"])
		dispatch_after(dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC / 2), dispatch_get_main_queue(), ^{
			[self updateHelpMenuWithArray:@[ @{ NSPrefPaneHelpMenuTitleKey: @"Test Help One", NSPrefPaneHelpMenuAnchorKey: @"one" },
			                                  @{ NSPrefPaneHelpMenuTitleKey: @"Test Help Two", NSPrefPaneHelpMenuAnchorKey: @"two" } ]];
			NSMutableArray *titles = [NSMutableArray array];
			for (NSMenuItem *item in [[NSApp helpMenu] itemArray])
				[titles addObject:[item title]];
			[self note:[NSString stringWithFormat:@"helpmenu %@", [titles componentsJoinedByString:@","]]];
		});
}

- (void)handleSystemPreferencesAnchor:(NSString *)anchor
{
	[self note:[NSString stringWithFormat:@"anchor %@ selected=%d", anchor, [self isSelected]]];
}

- (NSPreferencePaneUnselectReply)shouldUnselect
{
	NSString *mode = [self mode];
	[self note:[NSString stringWithFormat:@"shouldUnselect mode=%@", mode]];
	if ([mode isEqualToString:@"cancel"])
		return NSUnselectCancel;
	if ([mode hasPrefix:@"later"]) {
		BOOL answer = ![mode isEqualToString:@"later-cancel"];
		dispatch_after(dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC / 2), dispatch_get_main_queue(), ^{ [self replyToShouldUnselect:answer]; });
		return NSUnselectLater;
	}
	return NSUnselectNow;
}

- (void)replyToShouldUnselect:(BOOL)b { [self note:[NSString stringWithFormat:@"replyToShouldUnselect:%d", b]]; [super replyToShouldUnselect:b]; }
- (void)willUnselect { [self note:@"willUnselect"]; }
- (void)didUnselect { [super didUnselect]; [self note:[NSString stringWithFormat:@"didUnselect selected=%d", [self isSelected]]]; }

// the labels of the icons in the application's window, in order (the grid's icons carry them as tool tips)
- (void)describeGrid
{
	NSMutableArray *labels = [NSMutableArray array];
	NSMutableArray *queue = [NSMutableArray array];
	for (NSWindow *w in [NSApp windows])
		[queue addObject:[w contentView]];
	while ([queue count]) {
		NSView *v = [queue objectAtIndex:0];
		[queue removeObjectAtIndex:0];
		if ([v toolTip] && [[v className] isEqualToString:@"SPIconView"])
			[labels addObject:[v toolTip]];
		[queue addObjectsFromArray:[v subviews]];
	}
	[self note:[NSString stringWithFormat:@"grid %@", [labels componentsJoinedByString:@","]]];
}

@end
