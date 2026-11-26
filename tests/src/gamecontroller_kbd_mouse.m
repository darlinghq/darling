/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * GCKeyboard and GCMouse with a virtual keyboard or mouse (evdev, through uinput) as seen through
 * IOKit HID: connect and disconnect notifications, +[GCKeyboard coalescedKeyboard], +[GCMouse mice]
 * and +[GCMouse current], key and button state and handlers, and the movement deltas.
 *
 * Mode "kbd" or "mouse". The device is created and removed by the driver script
 * tests/hid-gamepad/run.sh, in lockstep: the test prints "SYNC: <tag>" and waits for the file
 * <controldir>/go-<tag>.
 * Usage: tests/hid-gamepad/run.sh <this test binary> <kbd|mouse>
 * Link with -framework Foundation -framework GameController. 64-bit only (GameController is
 * 64-bit only on macOS).
 */
#import <Foundation/Foundation.h>
#import <GameController/GameController.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#undef check
static int failures;
static const char *ctl_dir;

static void check(const char *what, BOOL cond)
{
	printf("%s: %s\n", cond ? "PASS" : "FAIL", what);
	if (!cond)
		failures++;
	fflush(stdout);
}

static void spin(double seconds)
{
	CFRunLoopRunInMode(kCFRunLoopDefaultMode, seconds, false);
}

static BOOL wait_for(BOOL (^cond)(void))
{
	for (int i = 0; i < 100 && !cond(); i++)
		spin(0.05);
	return cond();
}

static void sync_point(const char *tag)
{
	char path[512];
	printf("SYNC: %s\n", tag);
	fflush(stdout);
	snprintf(path, sizeof(path), "%s/go-%s", ctl_dir, tag);
	for (int i = 0; i < 1200 && access(path, F_OK) != 0; i++)
		spin(0.05);
}

enum { kKeyA = 4, kLeftShift = 225 }; /* GCKeyCode is the usage on the Keyboard/Keypad page */

static void test_keyboard(void)
{
	BOOL hadKeyboard = [GCKeyboard coalescedKeyboard] != nil;
	__block int connected = 0, disconnected = 0;
	NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
	id o1 = [nc addObserverForName:GCKeyboardDidConnectNotification object:nil queue:[NSOperationQueue mainQueue]
		usingBlock:^(NSNotification *n) { connected++; }];
	id o2 = [nc addObserverForName:GCKeyboardDidDisconnectNotification object:nil queue:[NSOperationQueue mainQueue]
		usingBlock:^(NSNotification *n) { disconnected++; }];

	sync_point("plug");
	GCKeyboard *kb = nil;
	for (int i = 0; i < 100 && !(kb = [GCKeyboard coalescedKeyboard]); i++)
		spin(0.05);
	check("coalescedKeyboard exists after a keyboard is plugged in", kb != nil);
	if (!kb)
		return;
	if (!hadKeyboard)
		check("GCKeyboardDidConnectNotification posted", wait_for(^BOOL { return connected == 1; }));
	check("product category is Keyboard", [[kb productCategory] isEqualToString:GCProductCategoryKeyboard]);
	check("coalescedKeyboard is the same object each time", [GCKeyboard coalescedKeyboard] == kb);
	GCKeyboardInput *input = [kb keyboardInput];
	check("keyboardInput exists and is the physical input profile", input != nil && [kb physicalInputProfile] == input);
	check("no key pressed at rest", ![input isAnyKeyPressed]);

	__block GCKeyCode lastCode = -1;
	__block BOOL lastPressed = NO;
	__block int calls = 0;
	input.keyChangedHandler = ^(GCKeyboardInput *k, GCDeviceButtonInput *key, GCKeyCode code, BOOL pressed) {
		lastCode = code; lastPressed = pressed; calls++;
		(void)k; (void)key;
	};

	sync_point("key-a");
	check("key A pressed: button state", wait_for(^BOOL { return [[input buttonForKeyCode:kKeyA] isPressed]; }));
	check("key A pressed: handler gets the key code and the state", wait_for(^BOOL { return calls == 1; }) && lastCode == kKeyA && lastPressed);
	check("isAnyKeyPressed while A is down", [input isAnyKeyPressed]);
	sync_point("key-a-up");
	check("key A released: button state", wait_for(^BOOL { return ![[input buttonForKeyCode:kKeyA] isPressed]; }));
	check("key A released: handler", wait_for(^BOOL { return calls == 2; }) && lastCode == kKeyA && !lastPressed);
	check("no key pressed after release", ![input isAnyKeyPressed]);
	sync_point("shift");
	check("left shift pressed", wait_for(^BOOL { return calls == 3; }) && lastCode == kLeftShift && lastPressed
		&& [[input buttonForKeyCode:kLeftShift] isPressed]);
	sync_point("shift-up");
	check("left shift released", wait_for(^BOOL { return calls == 4; }) && lastCode == kLeftShift && !lastPressed);

	sync_point("unplug");
	if (!hadKeyboard) {
		check("GCKeyboardDidDisconnectNotification posted", wait_for(^BOOL { return disconnected == 1; }));
		check("coalescedKeyboard is nil without a keyboard", [GCKeyboard coalescedKeyboard] == nil);
	}
	[nc removeObserver:o1];
	[nc removeObserver:o2];
}

static void test_mouse(void)
{
	NSUInteger before = [[GCMouse mice] count];
	__block GCMouse *connectedMouse = nil;
	__block GCMouse *disconnectedMouse = nil;
	__block int becameCurrent = 0;
	NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
	id o1 = [nc addObserverForName:GCMouseDidConnectNotification object:nil queue:[NSOperationQueue mainQueue]
		usingBlock:^(NSNotification *n) { connectedMouse = [n object]; }];
	id o2 = [nc addObserverForName:GCMouseDidDisconnectNotification object:nil queue:[NSOperationQueue mainQueue]
		usingBlock:^(NSNotification *n) { disconnectedMouse = [n object]; }];
	id o3 = [nc addObserverForName:GCMouseDidBecomeCurrentNotification object:nil queue:[NSOperationQueue mainQueue]
		usingBlock:^(NSNotification *n) { becameCurrent++; }];

	sync_point("plug");
	check("GCMouseDidConnectNotification posted with the mouse", wait_for(^BOOL { return connectedMouse != nil; }));
	GCMouse *mouse = connectedMouse;
	if (!mouse)
		return;
	check("+mice has the new mouse", [[GCMouse mice] count] == before + 1 && [[GCMouse mice] containsObject:mouse]);
	check("+current is non-nil", [GCMouse current] != nil);
	check("product category is Mouse", [[mouse productCategory] isEqualToString:GCProductCategoryMouse]);
	check("vendorName is the product name", [[mouse vendorName] hasPrefix:@"Vpad"]);
	GCMouseInput *input = [mouse mouseInput];
	check("mouseInput exists and is the physical input profile", input != nil && [mouse physicalInputProfile] == input);
	check("left, right and middle buttons exist", [input leftButton] && [input rightButton] && [input middleButton]);

	__block int leftChanges = 0;
	__block BOOL leftState = NO;
	input.leftButton.pressedChangedHandler = ^(GCControllerButtonInput *b, float v, BOOL pressed) { leftChanges++; leftState = pressed; };
	__block int profileChanges = 0;
	input.valueDidChangeHandler = ^(GCPhysicalInputProfile *p, GCControllerElement *e) { profileChanges++; };

	sync_point("left-down");
	check("left button pressed", wait_for(^BOOL { return [input.leftButton isPressed]; }) && input.leftButton.value == 1.0f);
	check("left button pressedChangedHandler called", wait_for(^BOOL { return leftChanges == 1; }) && leftState);
	check("profile valueDidChangeHandler called", profileChanges >= 1);
	check("the mouse that produced input is current", [GCMouse current] == mouse);
	sync_point("left-up");
	check("left button released", wait_for(^BOOL { return ![input.leftButton isPressed]; }));
	check("left button pressedChangedHandler called on release", wait_for(^BOOL { return leftChanges == 2; }) && !leftState);
	sync_point("right-down");
	check("right button pressed, left not", wait_for(^BOOL { return [input.rightButton isPressed]; }) && ![input.leftButton isPressed]);
	sync_point("right-up");
	check("right button released", wait_for(^BOOL { return ![input.rightButton isPressed]; }));

	__block float sumX = 0, sumY = 0;
	__block int moves = 0;
	input.mouseMovedHandler = ^(GCMouseInput *m, float dx, float dy) { sumX += dx; sumY += dy; moves++; };
	sync_point("move");
	/* the orientation of the vertical delta is not documented; only its size is checked */
	check("movement: one call with the deltas of the report", wait_for(^BOOL { return moves >= 1; }) && sumX == 5.0f && fabsf(sumY) == 3.0f);
	check("movement of one report arrives in one call", moves == 1);
	sync_point("move-x-big");
	check("large negative movement", wait_for(^BOOL { return sumX == 5.0f - 300.0f; }));
	sumX = 0; sumY = 0;
	sync_point("two-frames");
	check("two reports: all movement delivered", wait_for(^BOOL { return sumX == 3.0f; }));

	sync_point("unplug");
	check("GCMouseDidDisconnectNotification posted with the mouse", wait_for(^BOOL { return disconnectedMouse == mouse; }));
	check("+mice no longer has the mouse", ![[GCMouse mice] containsObject:mouse]);
	check("+mice count is back", [[GCMouse mice] count] == before);
	[nc removeObserver:o1];
	[nc removeObserver:o2];
	[nc removeObserver:o3];
}

int main(int argc, char *argv[])
{
	@autoreleasepool {
		if (argc < 3) {
			fprintf(stderr, "usage: %s <controldir> <kbd|mouse>\n", argv[0]);
			return 2;
		}
		ctl_dir = argv[1];
		setvbuf(stdout, NULL, _IOLBF, 0);
		if (strcmp(argv[2], "kbd") == 0)
			test_keyboard();
		else
			test_mouse();
		printf("%s\n", failures ? "FAILED" : "ALL PASSED");
		return failures ? 1 : 0;
	}
}
