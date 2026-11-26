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

// GUI test of the Sound section of Darling.prefPane. It loads the installed pane like System Preferences does and
// operates the controls the way a user does, against the PulseAudio test devices the harness provides: an output
// "DarlingTestSink" (a null sink, uid darling-test-sink) and an input "DarlingTestSource" (a source, uid
// darling-test-source) that carries a 440 Hz tone of amplitude 0.5 (or silence). The harness also records the
// sink's monitor while the test runs, to check that the test tone is really played, and creates and removes a
// second sink when the test asks for it with WAIT-NEW-SINK and WAIT-REMOVE-SINK on standard output.
// usage: darling_prefpane_sound signal|silence [keep]
//   keep: leave the test devices as the default devices (the harness reads PulseAudio's defaults then)
#import <Cocoa/Cocoa.h>
#import <CoreAudio/CoreAudio.h>
#import <PreferencePanes/PreferencePanes.h>

static int gFailures;
#define CHECK(cond, ...) do { \
	if (cond) printf("PASS: %s\n", #cond); \
	else { gFailures++; printf("FAIL: %s: ", #cond); printf(__VA_ARGS__); printf("\n"); } \
	fflush(stdout); \
} while (0)

@protocol AnchorPane <NSObject>
- (void)handleSystemPreferencesAnchor:(NSString *)anchor;
@end

static NSPreferencePane *gPane;

static void Spin(double seconds)
{
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:seconds]];
}

static void CollectByToolTip(NSView *root, NSString *tip, NSMutableArray *found)
{
	if ([[root toolTip] isEqualToString:tip])
		[found addObject:root];
	for (NSView *sub in [root subviews])
		CollectByToolTip(sub, tip, found);
}

static id Control(NSString *tip)
{
	NSMutableArray *found = [NSMutableArray array];
	CollectByToolTip([gPane mainView], tip, found);
	return [found firstObject];
}

static NSString *Cell(NSTableView *table, NSInteger row, NSString *column)
{
	NSTableColumn *c = [table tableColumnWithIdentifier:column];
	return [[table dataSource] tableView:table objectValueForTableColumn:c row:row];
}

static NSInteger RowNamed(NSTableView *table, NSString *name)
{
	for (NSInteger row = 0; row < [table numberOfRows]; row++)
		if ([Cell(table, row, @"name") isEqualToString:name])
			return row;
	return -1;
}

static void Select(NSTableView *table, NSInteger row)
{
	[table selectRowIndexes:[NSIndexSet indexSetWithIndex:row] byExtendingSelection:NO];
	Spin(0.7);
}

static AudioObjectPropertyAddress Addr(AudioObjectPropertySelector s, AudioObjectPropertyScope scope)
{
	AudioObjectPropertyAddress a = { s, scope, kAudioObjectPropertyElementMaster };
	return a;
}

static NSString *UIDOf(AudioObjectID device)
{
	AudioObjectPropertyAddress a = Addr(kAudioDevicePropertyDeviceUID, kAudioObjectPropertyScopeGlobal);
	CFStringRef uid = NULL;
	UInt32 size = sizeof(uid);
	if (AudioObjectGetPropertyData(device, &a, 0, NULL, &size, &uid) != noErr)
		return nil;
	return CFBridgingRelease(uid);
}

static AudioObjectID DefaultDevice(BOOL input)
{
	AudioObjectPropertyAddress a = Addr(input ? kAudioHardwarePropertyDefaultInputDevice : kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal);
	AudioObjectID d = 0;
	UInt32 size = sizeof(d);
	AudioObjectGetPropertyData(kAudioObjectSystemObject, &a, 0, NULL, &size, &d);
	return d;
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

int main(int argc, char **argv)
{
	@autoreleasepool {
		BOOL silence = argc > 1 && !strcmp(argv[1], "silence");
		BOOL keep = argc > 2 && !strcmp(argv[2], "keep");
		[NSApplication sharedApplication];
		[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

		NSBundle *bundle = [NSBundle bundleWithPath:@"/System/Library/PreferencePanes/Darling.prefPane"];
		CHECK(bundle != nil && [bundle load], "the pane loads");
		gPane = [[[bundle principalClass] alloc] initWithBundle:bundle];
		NSView *view = [gPane loadMainView];
		NSWindow *window = [[NSWindow alloc] initWithContentRect:NSMakeRect(100, 100, [view frame].size.width, [view frame].size.height)
		                                               styleMask:NSWindowStyleMaskTitled backing:NSBackingStoreBuffered defer:NO];
		[window setContentView:view];
		[window makeKeyAndOrderFront:nil];
		[gPane willSelect];
		[gPane didSelect];
		Spin(0.5);

		AudioObjectID oldOut = DefaultDevice(NO), oldIn = DefaultDevice(YES);
		[(id<AnchorPane>)gPane handleSystemPreferencesAnchor:@"sound"];
		Spin(1);
		NSTableView *outputs = Control(@"SoundOutputList"), *inputs = Control(@"SoundInputList");
		CHECK(outputs != nil && inputs != nil, "the Sound section has an output and an input list");
		NSInteger sinkRow = RowNamed(outputs, @"DarlingTestSink"), sourceRow = RowNamed(inputs, @"DarlingTestSource");
		CHECK(sinkRow >= 0, "the test sink is in the output list (%ld rows)", (long)[outputs numberOfRows]);
		CHECK(sourceRow >= 0, "the test source is in the input list (%ld rows)", (long)[inputs numberOfRows]);
		if (sinkRow < 0 || sourceRow < 0)
			return 1;
		CHECK([Cell(outputs, sinkRow, @"channels") isEqualToString:@"2"] && [Cell(outputs, sinkRow, @"manufacturer") length] > 0
		      && [Cell(outputs, sinkRow, @"rate") hasSuffix:@"Hz"] && [Cell(outputs, sinkRow, @"transport") length] > 0,
		      "an output row shows channels, manufacturer, sample rate and transport: %s / %s / %s / %s",
		      [Cell(outputs, sinkRow, @"channels") UTF8String], [Cell(outputs, sinkRow, @"manufacturer") UTF8String],
		      [Cell(outputs, sinkRow, @"rate") UTF8String], [Cell(outputs, sinkRow, @"transport") UTF8String]);
		BOOL standIns = NO;
		for (NSInteger row = 0; row < [outputs numberOfRows]; row++)
			if ([[Cell(outputs, row, @"name") description] isEqualToString:@"PulseAudio"])
				standIns = YES;
		CHECK(!standIns, "the objects that stand for the default device are not listed");

		// the default devices: the selected rows, and choosing a row makes it the default
		Select(outputs, sinkRow);
		CHECK([UIDOf(DefaultDevice(NO)) isEqualToString:@"darling-test-sink"], "choosing the sink makes it the default output: %s", [UIDOf(DefaultDevice(NO)) UTF8String]);
		Select(inputs, sourceRow);
		CHECK([UIDOf(DefaultDevice(YES)) isEqualToString:@"darling-test-source"], "choosing the source makes it the default input: %s", [UIDOf(DefaultDevice(YES)) UTF8String]);

		// the choice sticks: another instance of the pane shows the same rows selected
		NSPreferencePane *second = [[[bundle principalClass] alloc] initWithBundle:bundle];
		NSView *secondView = [second loadMainView];
		NSMutableArray *found = [NSMutableArray array];
		CollectByToolTip(secondView, @"SoundOutputList", found);
		[(id<AnchorPane>)second handleSystemPreferencesAnchor:@"sound"];
		[found removeAllObjects];
		CollectByToolTip(secondView, @"SoundOutputList", found);
		NSWindow *secondWindow = [[NSWindow alloc] initWithContentRect:NSMakeRect(300, 300, 100, 100) styleMask:NSWindowStyleMaskTitled backing:NSBackingStoreBuffered defer:NO];
		[secondWindow setContentView:secondView];
		[second willSelect];
		[second didSelect];
		Spin(0.7);
		NSTableView *secondOutputs = [found firstObject];
		CHECK(secondOutputs != nil && [[secondOutputs dataSource] numberOfRowsInTableView:secondOutputs] > 0
		      && [secondOutputs selectedRow] >= 0 && [Cell(secondOutputs, [secondOutputs selectedRow], @"name") isEqualToString:@"DarlingTestSink"],
		      "a second instance of the pane shows the sink selected");
		[second willUnselect];
		[second didUnselect];

		// volume and mute of the default output
		NSSlider *volume = Control(@"SoundVolume");
		NSButton *mute = Control(@"SoundMute");
		CHECK([volume isEnabled] && [mute isEnabled], "the volume and mute controls are enabled for the sink");
		[volume setDoubleValue:0.5];
		[NSApp sendAction:[volume action] to:[volume target] from:volume];
		Spin(0.5);
		AudioObjectID sink = DefaultDevice(NO);
		AudioObjectPropertyAddress va = Addr(kAudioDevicePropertyVolumeScalar, kAudioObjectPropertyScopeOutput);
		Float32 v = 0;
		UInt32 size = sizeof(v);
		AudioObjectGetPropertyData(sink, &va, 0, NULL, &size, &v);
		CHECK(fabs(v - 0.5) < 0.02, "the volume slider sets the device volume (%g)", v);
		[mute setState:NSControlStateValueOn];
		[NSApp sendAction:[mute action] to:[mute target] from:mute];
		Spin(0.5);
		AudioObjectPropertyAddress ma = Addr(kAudioDevicePropertyMute, kAudioObjectPropertyScopeOutput);
		UInt32 m = 9;
		size = sizeof(m);
		AudioObjectGetPropertyData(sink, &ma, 0, NULL, &size, &m);
		CHECK(m == 1, "the mute checkbox mutes the device (%u)", m);
		[mute setState:NSControlStateValueOff];
		[NSApp sendAction:[mute action] to:[mute target] from:mute];
		Spin(0.5);
		size = sizeof(m);
		AudioObjectGetPropertyData(sink, &ma, 0, NULL, &size, &m);
		CHECK(m == 0, "and unmutes it (%u)", m);
		[volume setDoubleValue:0.8];
		[NSApp sendAction:[volume action] to:[volume target] from:volume];
		Spin(0.3);

		// the output test: the button is busy while the tone plays and free again afterwards; the harness checks the sound
		NSButton *tone = Control(@"SoundTestTone");
		printf("TONE-NOW\n");
		fflush(stdout);
		[NSApp sendAction:[tone action] to:[tone target] from:tone];
		Spin(0.3);
		CHECK(![tone isEnabled], "the test tone button is busy while the tone plays");
		CHECK(WaitUntil(^BOOL{ return [tone isEnabled]; }, 6), "and is available again when it has ended");

		// the input test: the level meter follows the signal on the selected input
		NSButton *test = Control(@"SoundInputTest");
		id level = Control(@"SoundInputLevel");
		CHECK([[level class] instancesRespondToSelector:@selector(doubleValue)] && [level doubleValue] == 0, "the meter is at rest before the test");
		for (int round = 0; round < 3; round++) {
			[test setState:NSControlStateValueOn];
			[NSApp sendAction:[test action] to:[test target] from:test];
			double top = 0;
			for (int i = 0; i < 30; i++) {
				Spin(0.1);
				top = MAX(top, [level doubleValue]);
			}
			if (silence)
				CHECK(top < 0.01, "round %d: no signal, the meter stays low (%g)", round, top);
			else
				CHECK(top > 0.3 && top < 0.75, "round %d: a tone of amplitude 0.5 reads about 0.5 (%g)", round, top);
			[test setState:NSControlStateValueOff];
			[NSApp sendAction:[test action] to:[test target] from:test];
			Spin(0.5);
			CHECK([level doubleValue] == 0, "round %d: stopping the test puts the meter at rest", round);
		}

		// the list follows the devices: the harness adds a sink and removes it again
		printf("WAIT-NEW-SINK\n");
		fflush(stdout);
		CHECK(WaitUntil(^BOOL{ return RowNamed(outputs, @"DarlingTestSink2") >= 0; }, 20), "a sink that appears is listed");
		printf("WAIT-REMOVE-SINK\n");
		fflush(stdout);
		CHECK(WaitUntil(^BOOL{ return RowNamed(outputs, @"DarlingTestSink2") < 0; }, 20), "and is dropped from the list when it goes away");
		CHECK([outputs selectedRow] >= 0 && [Cell(outputs, [outputs selectedRow], @"name") isEqualToString:@"DarlingTestSink"], "the default stays selected");

		[gPane willUnselect];
		[gPane didUnselect];
		if (!keep) {
			AudioObjectPropertyAddress oa = Addr(kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal);
			AudioObjectPropertyAddress ia = Addr(kAudioHardwarePropertyDefaultInputDevice, kAudioObjectPropertyScopeGlobal);
			AudioObjectSetPropertyData(kAudioObjectSystemObject, &oa, 0, NULL, sizeof(oldOut), &oldOut);
			AudioObjectSetPropertyData(kAudioObjectSystemObject, &ia, 0, NULL, sizeof(oldIn), &oldIn);
		}
	}
	printf(gFailures ? "FAILED: %d\n" : "ALL PASSED\n", gFailures);
	return gFailures != 0;
}
