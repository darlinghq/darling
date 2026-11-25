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

#import <PreferencePanes/NSPreferencePane.h>

NSString * const NSPreferencePaneDoUnselectNotification = @"NSPreferencePaneDoUnselectNotification";
NSString * const NSPreferencePaneCancelUnselectNotification = @"NSPreferencePaneCancelUnselectNotification";
NSString * const NSPreferencePaneSwitchToPaneNotification = @"NSPreferencePaneSwitchToPaneNotification";
NSString * const NSPreferencePrefPaneIsAvailableNotification = @"NSPreferencePrefPaneIsAvailableNotification";
NSString * const NSPreferencePaneUpdateHelpMenuNotification = @"NSPreferencePaneUpdateHelpMenuNotification";

NSString * const NSPrefPaneHelpMenuInfoPListKey = @"NSPrefPaneHelpAnchors";
NSString * const NSPrefPaneHelpMenuTitleKey = @"title";
NSString * const NSPrefPaneHelpMenuAnchorKey = @"anchor";

// The nib name used when the bundle's Info.plist has no NSMainNibFile.
static NSString * const kDefaultMainNibName = @"Main";

@implementation NSPreferencePane

- (instancetype)initWithBundle:(NSBundle *)bundle
{
	if ((self = [super init])) {
		_bundle = [bundle retain];
	}
	return self;
}

- (void)dealloc
{
	[_window release];
	[_initialKeyView release];
	[_firstKeyView release];
	[_lastKeyView release];
	[_mainView release];
	[_bundle release];
	[_reserved1 release];
	[super dealloc];
}

- (NSBundle *)bundle
{
	return _bundle;
}

- (NSString *)mainNibName
{
	id name = [_bundle objectForInfoDictionaryKey:@"NSMainNibFile"];

	// QUIRK: a bundle without an NSMainNibFile entry gets the nib name "Main"
	// instead of nil, and an empty entry is passed on as the empty name.
	// Evidence: black-box observation (tier 2) on macOS 10.14.4 and 26.7.3,
	// with a probe that instantiates a test pane in a bundle without the key.
	// Test: test_quirk_main_nib_name_defaults_to_main (tests/src/preferencepanes.m)
	if (![name isKindOfClass:[NSString class]])
		return kDefaultMainNibName;
	return name;
}

- (NSView *)loadMainView
{
	NSString *path = [_bundle pathForResource:[self mainNibName] ofType:@"nib"];

	if (!path || ![NSBundle loadNibFile:path externalNameTable:[NSDictionary dictionaryWithObject:self forKey:NSNibOwner] withZone:nil]) {
		NSLog(@"[NSPreferencePane loadMainView]: Could not load nib at \"%@\"", path);
		// the declared result is nonnull: an empty view stands in for the missing one,
		// while the main view stays unset and nothing is assigned
		return [[[NSView alloc] init] autorelease];
	}

	[self assignMainView];
	[self mainViewDidLoad];
	return [self mainView];
}

- (void)mainViewDidLoad
{
}

- (void)assignMainView
{
	if (!_window) {
		NSLog(@"The _window outlet is not connected");
		return;
	}

	// take the content view out of the window; the window then has no use
	NSView *content = [[[_window contentView] retain] autorelease];
	[content removeFromSuperview];
	[self setMainView:content];

	[_window release];
	_window = nil;
}

- (void)willSelect
{
}

- (void)didSelect
{
	[_reserved1 release];
	_reserved1 = [[NSNumber numberWithBool:YES] retain];
}

- (NSPreferencePaneUnselectReply)shouldUnselect
{
	return NSUnselectNow;
}

- (void)replyToShouldUnselect:(BOOL)shouldUnselect
{
	[[NSNotificationCenter defaultCenter] postNotificationName:(shouldUnselect ? NSPreferencePaneDoUnselectNotification : NSPreferencePaneCancelUnselectNotification) object:self];
}

- (void)willUnselect
{
}

- (void)didUnselect
{
	[_reserved1 release];
	_reserved1 = nil;
}

- (NSView *)mainView
{
	return _mainView;
}

- (void)setMainView:(NSView *)view
{
	if (view == _mainView)
		return;
	[view retain];
	[_mainView release];
	_mainView = view;
}

- (NSView *)initialKeyView
{
	return _initialKeyView;
}

- (void)setInitialKeyView:(NSView *)view
{
	if (view == _initialKeyView)
		return;
	[view retain];
	[_initialKeyView release];
	_initialKeyView = view;
}

- (NSView *)firstKeyView
{
	return _firstKeyView;
}

- (void)setFirstKeyView:(NSView *)view
{
	if (view == _firstKeyView)
		return;
	[view retain];
	[_firstKeyView release];
	_firstKeyView = view;
}

- (NSView *)lastKeyView
{
	return _lastKeyView;
}

- (void)setLastKeyView:(NSView *)view
{
	if (view == _lastKeyView)
		return;
	[view retain];
	[_lastKeyView release];
	_lastKeyView = view;
}

- (BOOL)autoSaveTextFields
{
	return YES;
}

- (BOOL)isSelected
{
	return [_reserved1 boolValue];
}

- (void)updateHelpMenuWithArray:(NSArray *)items
{
	// QUIRK: the notification's object is the array of help items itself
	// (nil for a nil array), not the pane, and it has no user info.
	// Evidence: black-box observation (tier 2) on macOS 10.14.4 and 26.7.3,
	// with a probe that observes the default notification center.
	// Test: test_quirk_update_help_menu_notification_object (tests/src/preferencepanes.m)
	[[NSNotificationCenter defaultCenter] postNotificationName:NSPreferencePaneUpdateHelpMenuNotification object:items];
}

@end
