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

// GUI test of Darling.prefPane (src/prefpanes/Darling): the test loads the installed pane like
// System Preferences does (NSBundle, principal class, loadMainView, willSelect, didSelect), puts
// its main view in a window of its own and operates the controls the way a user does (a popup
// item is chosen and its action sent, a text field is edited and ends editing, a button is
// clicked). What the pane wrote is read back through the defaults tool in other processes, so
// the test sees the files the way every application does. It changes keys of the test domain
// (org.darlinghq.test.app), org.darlinghq.AppKit and the system-wide platform and security
// domains, and puts every one of them back to unset. Needs a display (X11, or Wayland with
// DARLING_APPKIT_BACKEND=wayland).
// usage: darling_prefpane
#import <Cocoa/Cocoa.h>
#import <PreferencePanes/PreferencePanes.h>

static int gFailures;
#define CHECK(cond, ...) do { \
	if (cond) printf("PASS: %s\n", #cond); \
	else { gFailures++; printf("FAIL: %s: ", #cond); printf(__VA_ARGS__); printf("\n"); } \
} while (0)

static NSString * const kPanePath = @"/System/Library/PreferencePanes/Darling.prefPane";
static NSString * const kAppKit = @"org.darlinghq.AppKit";
static NSString * const kPlatformFile = @"/Library/Preferences/org.darlinghq.platform";
static NSString * const kSecurityFile = @"/Library/Preferences/org.darlinghq.security";
static NSString * const kTestApplication = @"org.darlinghq.test.app";

static void Spin(double seconds)
{
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:seconds]];
}

// Runs a tool and returns what it wrote to standard output (and its status).
static NSString *Run(NSString *tool, NSArray *args, int *status)
{
	NSTask *t = [[NSTask alloc] init];
	NSPipe *out = [NSPipe pipe];
	[t setLaunchPath:tool];
	[t setArguments:args];
	[t setStandardOutput:out];
	[t setStandardError:[NSFileHandle fileHandleWithNullDevice]];
	[t launch];
	NSData *data = [[out fileHandleForReading] readDataToEndOfFile];
	[t waitUntilExit];
	if (status) *status = [t terminationStatus];
	NSString *s = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
	return [s stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
}

// `defaults read <domain> <key>`; nil when the key is not set
static NSString *DefaultsRead(NSString *domain, NSString *key)
{
	int status;
	NSString *value = Run(@"/usr/bin/defaults", @[ @"read", domain, key ], &status);
	return status == 0 ? value : nil;
}

static NSString *DefaultsType(NSString *domain, NSString *key)
{
	int status;
	NSString *value = Run(@"/usr/bin/defaults", @[ @"read-type", domain, key ], &status);
	return status == 0 ? value : nil;
}

// hw.model as a newly started process sees it, once it is `expected` (waits up to ten seconds)
static NSString *WaitForModel(NSString *expected)
{
	NSString *model = nil;
	for (int i = 0; i < 40; i++) {
		model = Run(@"/usr/sbin/sysctl", @[ @"-n", @"hw.model" ], NULL);
		if ([model isEqualToString:expected]) break;
		Spin(0.25);
	}
	return model;
}

static NSView *FindByToolTip(NSView *root, NSString *tip, Class kind)
{
	if ([[root toolTip] isEqualToString:tip] && (kind == Nil || [root isKindOfClass:kind]))
		return root;
	for (NSView *sub in [root subviews]) {
		NSView *found = FindByToolTip(sub, tip, kind);
		if (found) return found;
	}
	return nil;
}

// The n-th (from 0) control carrying the tool tip
static void CollectByToolTip(NSView *root, NSString *tip, NSMutableArray *found)
{
	if ([[root toolTip] isEqualToString:tip])
		[found addObject:root];
	for (NSView *sub in [root subviews])
		CollectByToolTip(sub, tip, found);
}

// the host's convention for the anchor of an x-apple.systempreferences URL
@protocol AnchorPane <NSObject>
- (void)handleSystemPreferencesAnchor:(NSString *)anchor;
@end

static NSWindow *gWindow;
static NSPreferencePane *gPane;

static NSView *Control(NSString *key, NSUInteger index)
{
	NSMutableArray *found = [NSMutableArray array];
	CollectByToolTip([gPane mainView], key, found);
	return index < [found count] ? found[index] : nil;
}

// the list of sections on the left
static NSTableView *Sidebar(void)
{
	NSMutableArray *queue = [NSMutableArray arrayWithObject:[gPane mainView]];
	while ([queue count]) {
		NSView *v = queue[0];
		[queue removeObjectAtIndex:0];
		if ([v isKindOfClass:[NSTableView class]])
			return (NSTableView *)v;
		[queue addObjectsFromArray:[v subviews]];
	}
	return nil;
}

static NSString *SectionTitleAtRow(NSInteger row)
{
	NSTableView *list = Sidebar();
	return [[list dataSource] tableView:list objectValueForTableColumn:[[list tableColumns] objectAtIndex:0] row:row];
}

// the identifier (the lower-case title) of the section the list has selected
static NSString *SelectedSection(void)
{
	return [SectionTitleAtRow([Sidebar() selectedRow]) lowercaseString];
}

static void Anchor(NSString *anchor)
{
	[(id<AnchorPane>)gPane handleSystemPreferencesAnchor:anchor];
	Spin(0.3);
}

// What a user does with a popup: choose an item, and the control sends its action
static void ChooseItem(NSPopUpButton *popup, NSInteger index)
{
	[popup selectItemAtIndex:index];
	[NSApp sendAction:[popup action] to:[popup target] from:popup];
	Spin(0.3);
}

static void TypeInto(NSTextField *field, NSString *text)
{
	[gWindow makeFirstResponder:field];
	NSText *editor = [field currentEditor];
	if (editor == nil) {
		[field selectText:nil];
		editor = [field currentEditor];
	}
	[editor setString:text];
	[gWindow makeFirstResponder:gWindow];
	Spin(0.4);
}

static void Click(NSButton *button)
{
	[NSApp sendAction:[button action] to:[button target] from:button];
	Spin(1);
}

static NSButton *ButtonTitled(NSString *title)
{
	NSMutableArray *queue = [NSMutableArray arrayWithObject:[gPane mainView]];
	while ([queue count]) {
		NSView *v = queue[0];
		[queue removeObjectAtIndex:0];
		if ([v isKindOfClass:[NSButton class]] && ![v isKindOfClass:[NSPopUpButton class]] && [[(NSButton *)v title] isEqualToString:title])
			return (NSButton *)v;
		[queue addObjectsFromArray:[v subviews]];
	}
	return nil;
}

int main(int argc, char **argv)
{
	@autoreleasepool {
		[NSApplication sharedApplication];
		[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

		NSBundle *bundle = [NSBundle bundleWithPath:kPanePath];
		CHECK(bundle != nil, "the pane is installed at %s", [kPanePath UTF8String]);
		if (!bundle) return 1;
		CHECK([[bundle bundleIdentifier] isEqualToString:@"org.darlinghq.preference.darling"], "identifier %s", [[bundle bundleIdentifier] UTF8String]);
		CHECK([[bundle objectForInfoDictionaryKey:@"NSPrefPaneIconLabel"] isEqualToString:@"Darling"], "label");
		// System Preferences shows the bundle name as the window title and the icon label in the grid
		CHECK([[bundle objectForInfoDictionaryKey:@"CFBundleName"] isEqualToString:@"Darling System Preferences"], "the bundle name is the window title: %s", [[bundle objectForInfoDictionaryKey:@"CFBundleName"] UTF8String]);
		NSString *icon = [bundle objectForInfoDictionaryKey:@"NSPrefPaneIconFile"];
		NSImage *iconImage = icon ? [[NSImage alloc] initWithContentsOfFile:[bundle pathForResource:icon ofType:@"icns"]] : nil;
		CHECK(iconImage != nil && [iconImage size].width > 0, "NSPrefPaneIconFile names an icon that loads");
		CHECK([[bundle objectForInfoDictionaryKey:@"NSPrefPaneSearchParameters"] length] > 0, "search parameters");
		CHECK([bundle load] && [[bundle principalClass] isSubclassOfClass:[NSPreferencePane class]], "the principal class is an NSPreferencePane");

		// a clean start: none of the keys the test changes is set
		for (NSString *key in @[ @"ControlKeyActsAsCommand", @"DisplayBackend", @"X11BackingScale", @"DarlingMenuBarStyle", @"DarlingInWindowMenuBar", @"TerminateAfterLastWindowClosed" ])
			Run(@"/usr/bin/defaults", @[ @"delete", kAppKit, key ], NULL);
		Run(@"/usr/bin/defaults", @[ @"delete", kTestApplication ], NULL);

		gPane = [[[bundle principalClass] alloc] initWithBundle:bundle];
		NSView *view = [gPane loadMainView];
		CHECK(view != nil && view == [gPane mainView], "loadMainView builds the main view without a nib");
		gWindow = [[NSWindow alloc] initWithContentRect:NSMakeRect(100, 100, [view frame].size.width, [view frame].size.height)
		                                      styleMask:NSWindowStyleMaskTitled backing:NSBackingStoreBuffered defer:NO];
		[gWindow setContentView:view];
		[gWindow makeKeyAndOrderFront:nil];
		[gPane willSelect];
		[gPane didSelect];
		Spin(1);
		CHECK([gPane isSelected], "didSelect");

		// sections and anchors
		CHECK(Sidebar() != nil && [Sidebar() numberOfRows] == 9, "the list has nine sections");
		NSMutableArray *ids = [NSMutableArray array];
		for (NSInteger row = 0; row < [Sidebar() numberOfRows]; row++) [ids addObject:SectionTitleAtRow(row)];
		CHECK([ids isEqualToArray:(@[ @"Keyboard", @"Display", @"Files", @"Graphics", @"Sound", @"Identity", @"Security", @"Metal", @"Applications" ])], "section order %s", [[ids description] UTF8String]);
		CHECK([SelectedSection() isEqualToString:@"keyboard"] && Control(@"ControlKeyActsAsCommand", 0) != nil, "the first section is shown at the start");
		// every row of the list has its own icon of the size of a source list's
		NSMutableSet *icons = [NSMutableSet set];
		BOOL iconsOK = YES;
		for (NSInteger row = 0; row < [Sidebar() numberOfRows]; row++) {
			NSTableColumn *column = [[Sidebar() tableColumns] objectAtIndex:0];
			NSCell *cell = [column dataCellForRow:row];
			[(id<NSTableViewDelegate>)[Sidebar() delegate] tableView:Sidebar() willDisplayCell:cell forTableColumn:column row:row];
			NSImage *icon = [cell image];
			NSData *tiff = [icon TIFFRepresentation];
			if (icon == nil || [icon size].width < 16 || [icon size].width > 18 || [icon size].height < 16 || [icon size].height > 18 || [tiff length] == 0) iconsOK = NO;
			else [icons addObject:tiff];
		}
		CHECK(iconsOK && [icons count] == 9, "each of the nine rows has an icon of 16 to 18 points, and they differ (%lu distinct)", (unsigned long)[icons count]);
		// the user picks rows in the list
		[Sidebar() selectRowIndexes:[NSIndexSet indexSetWithIndex:7] byExtendingSelection:NO];
		Spin(0.3);
		CHECK([SelectedSection() isEqualToString:@"metal"] && Control(@"LogStatistics", 0) != nil && Control(@"ControlKeyActsAsCommand", 0) == nil, "selecting a row shows that section only");
		[Sidebar() selectRowIndexes:[NSIndexSet indexSetWithIndex:0] byExtendingSelection:NO];
		Spin(0.3);
		Anchor(@"Security");
		CHECK([SelectedSection() isEqualToString:@"security"], "anchor Security selects the security section: %s", [SelectedSection() UTF8String]);
		Anchor(@"DesktopFileChooser");
		CHECK([SelectedSection() isEqualToString:@"files"], "the key of a setting selects its section: %s", [SelectedSection() UTF8String]);
		Anchor(@"sound");
		CHECK([SelectedSection() isEqualToString:@"sound"] && Control(@"SoundOutputList", 0) != nil, "the sound anchor shows the Sound section");
		Anchor(@"Application lifecycle");
		CHECK([SelectedSection() isEqualToString:@"applications"], "the issue's group name works: %s", [SelectedSection() UTF8String]);
		Anchor(@"no-such-anchor");
		CHECK([SelectedSection() isEqualToString:@"applications"], "an unknown anchor leaves the section as it is");

		// the pane's height follows the section (the host resizes its window to it)
		Anchor(@"keyboard");
		double keyboardHeight = [view frame].size.height;
		Anchor(@"identity");
		double identityHeight = [view frame].size.height;
		CHECK(identityHeight > keyboardHeight + 100, "the main view is taller for the identity section (%g) than for the keyboard section (%g)", identityHeight, keyboardHeight);

		// a boolean user setting: shows the documented default, writes, and removes the key for Default
		Anchor(@"keyboard");
		NSPopUpButton *control = (NSPopUpButton *)Control(@"ControlKeyActsAsCommand", 0);
		CHECK([control isKindOfClass:[NSPopUpButton class]], "ControlKeyActsAsCommand is a popup");
		CHECK([[[control itemAtIndex:0] title] isEqualToString:@"Default (Off)"] && [control indexOfSelectedItem] == 0, "unset shows Default (Off): %s", [[control titleOfSelectedItem] UTF8String]);
		ChooseItem(control, 1);
		CHECK([DefaultsRead(kAppKit, @"ControlKeyActsAsCommand") isEqualToString:@"1"] && [DefaultsType(kAppKit, @"ControlKeyActsAsCommand") isEqualToString:@"Type is boolean"], "On writes a boolean YES: %s / %s", [DefaultsRead(kAppKit, @"ControlKeyActsAsCommand") UTF8String], [DefaultsType(kAppKit, @"ControlKeyActsAsCommand") UTF8String]);
		ChooseItem(control, 2);
		CHECK([DefaultsRead(kAppKit, @"ControlKeyActsAsCommand") isEqualToString:@"0"], "Off writes NO (different from unset)");
		ChooseItem(control, 0);
		CHECK(DefaultsRead(kAppKit, @"ControlKeyActsAsCommand") == nil, "Default removes the key");

		// a value written elsewhere shows up when the pane is selected again
		Run(@"/usr/bin/defaults", @[ @"write", kAppKit, @"ControlKeyActsAsCommand", @"-bool", @"YES" ], NULL);
		[gPane willSelect];
		CHECK([control indexOfSelectedItem] == 1, "a key written with the defaults tool is shown");
		ChooseItem(control, 0);

		// a choice
		Anchor(@"display");
		NSPopUpButton *backend = (NSPopUpButton *)Control(@"DisplayBackend", 0);
		ChooseItem(backend, 2);
		CHECK([DefaultsRead(kAppKit, @"DisplayBackend") isEqualToString:@"wayland"], "DisplayBackend wayland: %s", [DefaultsRead(kAppKit, @"DisplayBackend") UTF8String]);
		ChooseItem(backend, 0);
		CHECK(DefaultsRead(kAppKit, @"DisplayBackend") == nil, "DisplayBackend back to Default removes the key");

		// a number: validated, written as a real, empty removes the key
		NSTextField *scale = (NSTextField *)Control(@"X11BackingScale", 0);
		TypeInto(scale, @"2");
		CHECK([DefaultsRead(kAppKit, @"X11BackingScale") doubleValue] == 2 && [DefaultsType(kAppKit, @"X11BackingScale") isEqualToString:@"Type is float"], "X11BackingScale 2 written as a real: %s / %s", [DefaultsRead(kAppKit, @"X11BackingScale") UTF8String], [DefaultsType(kAppKit, @"X11BackingScale") UTF8String]);
		TypeInto(scale, @"0.5");
		CHECK([DefaultsRead(kAppKit, @"X11BackingScale") doubleValue] == 2 && [[scale stringValue] isEqualToString:@"2"], "a scale below 1 is refused and the field shows the stored value (%s)", [[scale stringValue] UTF8String]);
		TypeInto(scale, @"abc");
		CHECK([DefaultsRead(kAppKit, @"X11BackingScale") doubleValue] == 2, "text that is no number is refused");
		TypeInto(scale, @"");
		CHECK(DefaultsRead(kAppKit, @"X11BackingScale") == nil, "an empty field removes the key");

		// the menu style replaces the older boolean
		Run(@"/usr/bin/defaults", @[ @"write", kAppKit, @"DarlingInWindowMenuBar", @"-bool", @"YES" ], NULL);
		[gPane willSelect];
		NSPopUpButton *style = (NSPopUpButton *)Control(@"DarlingMenuBarStyle", 0);
		CHECK([[style titleOfSelectedItem] isEqualToString:@"Inside each window"], "the older boolean shows as Inside each window: %s", [[style titleOfSelectedItem] UTF8String]);
		ChooseItem(style, 3);
		CHECK([DefaultsRead(kAppKit, @"DarlingMenuBarStyle") isEqualToString:@"auto"] && DefaultsRead(kAppKit, @"DarlingInWindowMenuBar") == nil, "choosing a style removes the older boolean");
		ChooseItem(style, 0);
		CHECK(DefaultsRead(kAppKit, @"DarlingMenuBarStyle") == nil, "Default removes the style");

		// per-application overrides
		Anchor(@"applications");
		NSPopUpButton *terminate = (NSPopUpButton *)Control(@"TerminateAfterLastWindowClosed", 1);
		CHECK(![terminate isEnabled], "the application's popups wait for an application");
		NSTextField *bundleField = (NSTextField *)Control(@"ApplicationIdentifier", 0);
		TypeInto(bundleField, @"bad id!");
		CHECK(![terminate isEnabled], "an invalid bundle identifier selects nothing");
		TypeInto(bundleField, kTestApplication);
		CHECK([terminate isEnabled] && [[[terminate itemAtIndex:0] title] isEqualToString:@"Same as for all applications"], "a bundle identifier enables the application's popups");
		ChooseItem(terminate, 1);
		CHECK([DefaultsRead(kTestApplication, @"TerminateAfterLastWindowClosed") isEqualToString:@"1"] && DefaultsRead(kAppKit, @"TerminateAfterLastWindowClosed") == nil, "the application's own domain gets the value, the global one stays unset");
		NSPopUpButton *global = (NSPopUpButton *)Control(@"TerminateAfterLastWindowClosed", 0);
		ChooseItem(global, 1);
		CHECK([DefaultsRead(kAppKit, @"TerminateAfterLastWindowClosed") isEqualToString:@"1"] && [DefaultsRead(kTestApplication, @"TerminateAfterLastWindowClosed") isEqualToString:@"1"], "the global value is separate");
		ChooseItem(terminate, 2);
		CHECK([DefaultsRead(kTestApplication, @"TerminateAfterLastWindowClosed") isEqualToString:@"0"], "an application can opt out with NO");
		ChooseItem(terminate, 0);
		CHECK(DefaultsRead(kTestApplication, @"TerminateAfterLastWindowClosed") == nil, "Same as for all applications removes the application's key");
		ChooseItem(global, 0);
		CHECK(DefaultsRead(kAppKit, @"TerminateAfterLastWindowClosed") == nil, "and the global one");

		// system-wide settings: edited first, written by Apply with administrator rights
		Anchor(@"identity");
		NSTextField *model = (NSTextField *)Control(@"Model", 0);
		NSButton *apply = ButtonTitled(@"Apply");
		CHECK(apply != nil && ![apply isEnabled], "Apply is disabled while nothing is edited");
		CHECK(DefaultsRead(kPlatformFile, @"Model") == nil, "no Model is set to begin with");
		TypeInto(model, @"TestBook9,9");
		CHECK([apply isEnabled] && DefaultsRead(kPlatformFile, @"Model") == nil, "an edited identity is not written before Apply");
		TypeInto(model, @"bad\x01model");
		CHECK(DefaultsRead(kPlatformFile, @"Model") == nil, "a value darling-prefsd would refuse is not accepted");
		NSString *defaultModel = Run(@"/usr/sbin/sysctl", @[ @"-n", @"hw.model" ], NULL);
		TypeInto(model, @"TestBook9,9");
		Click(apply);
		CHECK([DefaultsRead(kPlatformFile, @"Model") isEqualToString:@"TestBook9,9"], "Apply writes the Model: %s", [DefaultsRead(kPlatformFile, @"Model") UTF8String]);
		NSDictionary *attributes = [[NSFileManager defaultManager] attributesOfItemAtPath:[kPlatformFile stringByAppendingString:@".plist"] error:nil];
		CHECK(attributes != nil && ([attributes[NSFilePosixPermissions] intValue] & 022) == 0, "the file in /Library/Preferences is not writable by group or others, which darling-prefsd requires (mode %o)", [attributes[NSFilePosixPermissions] intValue]);
		CHECK([WaitForModel(@"TestBook9,9") isEqualToString:@"TestBook9,9"], "darling-prefsd pushes the Model to the running system: hw.model is now %s", [WaitForModel(@"TestBook9,9") UTF8String]);
		CHECK(![apply isEnabled], "nothing is pending afterwards");

		// the memory is typed in GiB and stored in bytes; the UUIDs must agree
		NSTextField *memory = (NSTextField *)Control(@"MemorySize", 0);
		NSTextField *platformUUID = (NSTextField *)Control(@"PlatformUUID", 0);
		NSTextField *udid = (NSTextField *)Control(@"ProvisioningUDID", 0);
		TypeInto(memory, @"6");
		TypeInto(platformUUID, @"0F8F1A52-6B77-4E58-9A0B-2D3C4E5F6071");
		TypeInto(udid, @"1F8F1A52-6B77-4E58-9A0B-2D3C4E5F6071");
		Click(apply);
		CHECK(DefaultsRead(kPlatformFile, @"MemorySize") == nil && DefaultsRead(kPlatformFile, @"PlatformUUID") == nil, "different UUIDs: nothing is written");
		TypeInto(udid, @"0f8f1a52-6b77-4e58-9a0b-2d3c4e5f6071");
		Click(apply);
		CHECK([DefaultsRead(kPlatformFile, @"MemorySize") isEqualToString:@"6442450944"], "6 GiB is written in bytes: %s", [DefaultsRead(kPlatformFile, @"MemorySize") UTF8String]);
		CHECK([DefaultsRead(kPlatformFile, @"PlatformUUID") isEqualToString:@"0F8F1A52-6B77-4E58-9A0B-2D3C4E5F6071"] && [DefaultsRead(kPlatformFile, @"ProvisioningUDID") length] == 36, "both UUIDs are written");

		// data and a bad hex value
		NSTextField *rom = (NSTextField *)Control(@"ROM", 0);
		TypeInto(rom, @"00:11:22:33");
		CHECK([apply isEnabled] == NO, "a ROM of the wrong length is refused");
		TypeInto(rom, @"00:11:22:33:44:55");
		Click(apply);
		CHECK([DefaultsType(kPlatformFile, @"ROM") isEqualToString:@"Type is data"], "the ROM is written as data: %s", [DefaultsType(kPlatformFile, @"ROM") UTF8String]);

		// back to the defaults: clearing a field removes the key
		for (NSString *key in @[ @"Model", @"MemorySize", @"PlatformUUID", @"ProvisioningUDID", @"ROM" ])
			TypeInto((NSTextField *)Control(key, 0), @"");
		CHECK([apply isEnabled], "clearing fields is pending");
		Click(apply);
		for (NSString *key in @[ @"Model", @"MemorySize", @"PlatformUUID", @"ProvisioningUDID", @"ROM" ])
			CHECK(DefaultsRead(kPlatformFile, key) == nil, "%s is removed", [key UTF8String]);

		CHECK([WaitForModel(defaultModel) isEqualToString:defaultModel], "removing the Model puts hw.model back to %s", [defaultModel UTF8String]);

		// Revert drops what was edited
		TypeInto(model, @"Edited1,1");
		NSButton *revert = ButtonTitled(@"Revert");
		Click(revert);
		CHECK([[model stringValue] length] == 0 && ![apply isEnabled], "Revert restores the stored values");

		// the security domain: a choice and a boolean, nothing is set by default
		Anchor(@"security");
		NSPopUpButton *policy = (NSPopUpButton *)Control(@"CodeSigningPolicy", 0);
		CHECK([[[policy itemAtIndex:0] title] isEqualToString:@"Default (off)"] && [policy indexOfSelectedItem] == 0, "unset code signing policy shows Default (off)");
		NSPopUpButton *import = (NSPopUpButton *)Control(@"ImportHostCertificates", 0);
		CHECK([[[import itemAtIndex:0] title] isEqualToString:@"Default (On)"], "the host certificate import shows its default On");
		ChooseItem(policy, 2);
		apply = ButtonTitled(@"Apply");
		CHECK([apply isEnabled] && DefaultsRead(kSecurityFile, @"CodeSigningPolicy") == nil, "a security change waits for Apply");
		Click(apply);
		CHECK([DefaultsRead(kSecurityFile, @"CodeSigningPolicy") isEqualToString:@"warn"], "warn is written: %s", [DefaultsRead(kSecurityFile, @"CodeSigningPolicy") UTF8String]);
		ChooseItem(policy, 0);
		Click(apply);
		CHECK(DefaultsRead(kSecurityFile, @"CodeSigningPolicy") == nil, "Default removes the policy again");

		[gPane willUnselect];
		[gPane didUnselect];
		CHECK(![gPane isSelected], "didUnselect");
	}
	printf(gFailures ? "FAILED: %d\n" : "ALL PASSED\n", gFailures);
	return gFailures != 0;
}
