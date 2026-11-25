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

// Helpers of the GUI tests of the standard preference panes (General, Language & Region, Keyboard,
// Date & Time): each test loads the installed pane the way System Preferences does, operates its
// controls (found by their tool tips) and reads the result back with /usr/bin/defaults in another
// process, then checks what applications observe.
#import <Cocoa/Cocoa.h>
#import <PreferencePanes/PreferencePanes.h>

static int gFailures;
#define CHECK(cond, ...) do { \
	if (cond) printf("PASS: %s\n", #cond); \
	else { gFailures++; printf("FAIL: %s: ", #cond); printf(__VA_ARGS__); printf("\n"); } \
	fflush(stdout); \
} while (0)

static NSPreferencePane *gPane;
static NSWindow *gWindow;

static void Spin(double seconds)
{
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:seconds]];
}

static BOOL WaitUntil(BOOL (^condition)(void), double seconds)
{
	for (double waited = 0; waited < seconds; waited += 0.1) {
		if (condition())
			return YES;
		Spin(0.1);
	}
	return condition();
}

static void CollectByToolTip(NSView *root, NSString *tip, NSMutableArray *found)
{
	if ([[root toolTip] isEqualToString:tip])
		[found addObject:root];
	for (NSView *sub in [root subviews])
		CollectByToolTip(sub, tip, found);
	if ([root isKindOfClass:[NSScrollView class]])
		CollectByToolTip([(NSScrollView *)root documentView], tip, found);
}

static id Control(NSString *tip)
{
	NSMutableArray *found = [NSMutableArray array];
	CollectByToolTip([gPane mainView], tip, found);
	return [found firstObject];
}

// Loads the pane of the bundle at the path and puts it in a window
static BOOL LoadPane(NSString *path)
{
	NSBundle *bundle = [NSBundle bundleWithPath:path];
	if (bundle == nil || ![bundle load])
		return NO;
	gPane = [[[bundle principalClass] alloc] initWithBundle:bundle];
	NSView *view = [gPane loadMainView];
	gWindow = [[NSWindow alloc] initWithContentRect:NSMakeRect(100, 100, [view frame].size.width, [view frame].size.height)
	                                      styleMask:NSWindowStyleMaskTitled backing:NSBackingStoreBuffered defer:NO];
	[gWindow setTitle:[[bundle objectForInfoDictionaryKey:@"CFBundleName"] description]];
	[gWindow setContentView:view];
	[gWindow makeKeyAndOrderFront:nil];
	[gPane willSelect];
	[gPane didSelect];
	Spin(0.5);
	return YES;
}

static void Choose(NSString *tip, NSInteger index)
{
	NSPopUpButton *popup = Control(tip);
	[popup selectItemAtIndex:index];
	[NSApp sendAction:[popup action] to:[popup target] from:popup];
	Spin(0.3);
}

static void Toggle(NSString *tip, BOOL on)
{
	NSButton *check = Control(tip);
	[check setState:on ? NSControlStateValueOn : NSControlStateValueOff];
	[NSApp sendAction:[check action] to:[check target] from:check];
	Spin(0.3);
}

static void Press(NSString *tip)
{
	NSButton *button = Control(tip);
	[NSApp sendAction:[button action] to:[button target] from:button];
	Spin(0.3);
}

static void Slide(NSString *tip, NSInteger index)
{
	NSSlider *slider = Control(tip);
	[slider setIntegerValue:index];
	[NSApp sendAction:[slider action] to:[slider target] from:slider];
	Spin(0.3);
}

// `defaults read <args>` in another process: its output without the trailing newline, nil when it fails
static NSString *DefaultsRead(NSArray *args)
{
	NSTask *task = [[NSTask alloc] init];
	NSPipe *pipe = [NSPipe pipe];
	[task setLaunchPath:@"/usr/bin/defaults"];
	[task setArguments:[@[@"read"] arrayByAddingObjectsFromArray:args]];
	[task setStandardOutput:pipe];
	[task setStandardError:[NSFileHandle fileHandleWithNullDevice]];
	[task launch];
	NSData *data = [[pipe fileHandleForReading] readDataToEndOfFile];
	[task waitUntilExit];
	if ([task terminationStatus] != 0)
		return nil;
	NSString *text = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
	return [text stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
}

static NSString *GlobalRead(NSString *key)
{
	return DefaultsRead(@[@"-g", key]);
}

// The preferences a test changes are put back as they were
static NSMutableDictionary *gSaved;

static void Remember(NSString *key, NSString *domain)
{
	if (gSaved == nil)
		gSaved = [NSMutableDictionary dictionary];
	NSString *tag = [NSString stringWithFormat:@"%@\n%@", domain ?: @"", key];
	if (gSaved[tag] != nil)
		return;
	CFPropertyListRef value = CFPreferencesCopyValue((__bridge CFStringRef)key, domain ? (__bridge CFStringRef)domain : kCFPreferencesAnyApplication,
	                                                 kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
	gSaved[tag] = value != NULL ? CFBridgingRelease(value) : [NSNull null];
}

static void SetPref(NSString *key, NSString *domain, id value)
{
	Remember(key, domain);
	CFStringRef d = domain ? (__bridge CFStringRef)domain : kCFPreferencesAnyApplication;
	CFPreferencesSetValue((__bridge CFStringRef)key, (__bridge CFPropertyListRef)value, d, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
	CFPreferencesSynchronize(d, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
}

static void RestorePrefs(void)
{
	for (NSString *tag in gSaved) {
		NSArray *parts = [tag componentsSeparatedByString:@"\n"];
		NSString *domain = [parts[0] length] ? parts[0] : nil, *key = parts[1];
		id value = gSaved[tag];
		CFStringRef d = domain ? (__bridge CFStringRef)domain : kCFPreferencesAnyApplication;
		CFPreferencesSetValue((__bridge CFStringRef)key, [value isKindOfClass:[NSNull class]] ? NULL : (__bridge CFPropertyListRef)value, d,
		                      kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
		CFPreferencesSynchronize(d, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
	}
}

// Counts the distributed notifications of a name that reach this process
static NSMutableDictionary *gNotified;

static void NotifyCallback(CFNotificationCenterRef center, void *observer, CFNotificationName name, const void *object, CFDictionaryRef info)
{
	NSString *key = (__bridge NSString *)name;
	gNotified[key] = @([gNotified[key] intValue] + 1);
}

static void ObserveDistributed(NSString *name)
{
	if (gNotified == nil)
		gNotified = [NSMutableDictionary dictionary];
	CFNotificationCenterAddObserver(CFNotificationCenterGetDistributedCenter(), &gNotified, NotifyCallback, (__bridge CFStringRef)name, NULL,
	                                CFNotificationSuspensionBehaviorDeliverImmediately);
}

static int NotifiedCount(NSString *name)
{
	return [gNotified[name] intValue];
}
