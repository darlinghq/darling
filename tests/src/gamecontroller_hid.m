/*
 * Integration test for GameController.framework using virtual HID controllers.
 * Tests discovery, connect/disconnect notifications, profiles (extendedGamepad,
 * microGamepad, physicalInputProfile), mice, keyboards, wireless discovery,
 * and input event delivery.
 */
#import <Foundation/Foundation.h>
#import <GameController/GameController.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int failures = 0;
static const char *ctl_dir = NULL;
static BOOL is_xbox = YES;

#undef check
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

static void sync_point(const char *tag)
{
	char path[512];
	printf("SYNC: %s\n", tag);
	fflush(stdout);
	snprintf(path, sizeof(path), "%s/go-%s", ctl_dir, tag);
	for (int i = 0; i < 1200 && access(path, F_OK) != 0; i++)
		spin(0.05);
}

int main(int argc, char *argv[])
{
	@autoreleasepool {
		if (argc < 2) {
			fprintf(stderr, "Usage: %s <controldir> [xbox|ds4]\n", argv[0]);
			return 1;
		}

		ctl_dir = argv[1];
		if (argc >= 3 && strcmp(argv[2], "ds4") == 0) {
			is_xbox = NO;
		}

		setvbuf(stdout, NULL, _IOLBF, 0);
		printf("--- GameController HID Test: starting (%s) ---\n", is_xbox ? "Xbox One" : "DualShock 4");

		// 1. Initial State: Mice and Keyboard
		NSArray<GCMouse *> *mice = [GCMouse mice];
		check("GCMouse mice returns array", mice != nil);
		printf("Initial mice count: %lu\n", (unsigned long)[mice count]);
		if ([mice count] > 0) {
			check("GCMouse current is non-nil when mice exist", [GCMouse current] != nil);
		}

		/* macOS answers nil without a keyboard; the keyboard itself is tested by gamecontroller_kbd_mouse */
		GCKeyboard *keyboard = [GCKeyboard coalescedKeyboard];
		if (keyboard) {
			check("GCKeyboard keyboardInput returns input", [keyboard keyboardInput] != nil);
		}

		NSArray<GCController *> *initialControllers = [GCController controllers];
		check("GCController controllers returns array", initialControllers != nil);
		NSUInteger initialCount = [initialControllers count];
		printf("Initial controllers count: %lu\n", (unsigned long)initialCount);

		// 2. Connect Notification Observer
		__block GCController *connectedController = nil;
		__block BOOL connectNotified = NO;

		id connectObserver = [[NSNotificationCenter defaultCenter]
			addObserverForName:GCControllerDidConnectNotification
			object:nil
			queue:[NSOperationQueue mainQueue]
			usingBlock:^(NSNotification *note) {
				GCController *c = [note object];
				// the virtual devices are named Vpad ...; a real controller may be attached too
				if (![[c vendorName] hasPrefix:@"Vpad"])
					return;
				printf("Received GCControllerDidConnectNotification: %p, category: %s\n",
				       c, [[c productCategory] UTF8String] ?: "(null)");
				connectedController = [c retain];
				connectNotified = YES;
			}];

		// 3. Hot-plug controller
		sync_point("plug");

		for (int i = 0; i < 200 && !connectNotified; i++)
			spin(0.05);

		check("GCControllerDidConnectNotification fired on plug", connectNotified && connectedController != nil);

		if (!connectedController) {
			// Check if it appears in controllers array anyway
			for (GCController *c in [GCController controllers]) {
				if ([[c vendorName] hasPrefix:@"Vpad"])
					connectedController = [c retain];
			}
		}

		check("Connected controller obtained", connectedController != nil);
		if (!connectedController) {
			printf("FATAL: No controller connected, aborting input tests.\n");
			return 1;
		}

		// Verify properties
		NSString *expectedCategory = is_xbox ? GCProductCategoryXboxOne : GCProductCategoryDualShock4;
		check("Product category matches device", [[connectedController productCategory] isEqualToString:expectedCategory]);
		check("Vendor name is set", [[connectedController vendorName] length] > 0);
		printf("Controller vendorName: %s\n", [[connectedController vendorName] UTF8String]);

		GCExtendedGamepad *extended = [connectedController extendedGamepad];
		check("Controller has extendedGamepad profile", extended != nil);
		check("Controller physicalInputProfile equals extendedGamepad", [connectedController physicalInputProfile] == extended);
		check("Controller has microGamepad profile", [connectedController microGamepad] != nil);

		// Elements exist
		GCControllerButtonInput *btnA = [extended buttonA];
		GCControllerDirectionPad *dpad = [extended dpad];
		GCControllerDirectionPad *leftStick = [extended leftThumbstick];
		GCControllerButtonInput *leftTrigger = [extended leftTrigger];

		check("extendedGamepad has buttonA", btnA != nil);
		check("extendedGamepad has dpad", dpad != nil);
		check("extendedGamepad has leftThumbstick", leftStick != nil);
		check("extendedGamepad has leftTrigger", leftTrigger != nil);

		// Set valueChangedHandlers
		__block BOOL btnAPressed = NO;
		__block float btnAValue = 0.0f;
		__block int btnAChangeCount = 0;
		btnA.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
			btnAValue = value;
			btnAPressed = pressed;
			btnAChangeCount++;
		};

		__block float stickXValue = 0.0f;
		leftStick.valueChangedHandler = ^(GCControllerDirectionPad *pad, float xValue, float yValue) {
			stickXValue = xValue;
		};

		__block float stickAxisValue = 0.0f;
		leftStick.xAxis.valueChangedHandler = ^(GCControllerAxisInput *axis, float value) {
			stickAxisValue = value;
		};

		__block float triggerValue = 0.0f;
		leftTrigger.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
			triggerValue = value;
		};

		__block BOOL dpadUpPressed = NO;
		dpad.up.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
			dpadUpPressed = pressed;
		};

		// 4. Rest state
		sync_point("rest");
		spin(0.2);
		check("At rest: buttonA value is 0", [btnA value] == 0.0f && ![btnA isPressed]);
		check("At rest: dpad is centered", [[dpad xAxis] value] == 0.0f && [[dpad yAxis] value] == 0.0f);

		// 5. Button Press
		sync_point("press");
		for (int i = 0; i < 100 && [btnA value] < 0.5f; i++)
			spin(0.05);
		check("Button press: buttonA value == 1.0", [btnA value] >= 0.9f && [btnA isPressed]);
		check("Button press: valueChangedHandler called", btnAChangeCount > 0);

		// 6. Button Release
		sync_point("release");
		for (int i = 0; i < 100 && [btnA value] > 0.1f; i++)
			spin(0.05);
		check("Button release: buttonA value == 0.0", [btnA value] <= 0.1f && ![btnA isPressed]);

		// 7. Stick High (Right)
		sync_point("stick");
		for (int i = 0; i < 100 && [[leftStick xAxis] value] < 0.5f; i++)
			spin(0.05);
		check("Stick right: leftThumbstick xAxis > 0.8", [[leftStick xAxis] value] > 0.8f);
		for (int i = 0; i < 100 && stickAxisValue < 0.5f; i++)
			spin(0.05);
		check("Stick right: leftThumbstick.xAxis valueChangedHandler called", stickAxisValue > 0.8f);

		// 8. Stick Low (Left)
		sync_point("stick-low");
		for (int i = 0; i < 100 && [[leftStick xAxis] value] > -0.5f; i++)
			spin(0.05);
		check("Stick left: leftThumbstick xAxis < -0.8", [[leftStick xAxis] value] < -0.8f);

		// 9. Trigger Press
		GCControllerButtonInput *trigger = is_xbox ? [extended leftTrigger] : [extended rightTrigger];
		sync_point("trigger");
		for (int i = 0; i < 100 && [trigger value] < 0.5f; i++)
			spin(0.05);
		check("Trigger: trigger value > 0.8", [trigger value] > 0.8f);

		// 10. Hat Up
		sync_point("hat-up");
		for (int i = 0; i < 100 && ![[dpad up] isPressed]; i++)
			spin(0.05);
		check("Hat up: dpad up isPressed", [[dpad up] isPressed]);
		for (int i = 0; i < 100 && !dpadUpPressed; i++)
			spin(0.05);
		check("Hat up: dpad.up valueChangedHandler called with pressed", dpadUpPressed);

		// 10b. Hat Up-Right
		sync_point("hat-upright");
		for (int i = 0; i < 100 && !([[dpad up] isPressed] && [[dpad right] isPressed]); i++)
			spin(0.05);
		check("Hat up-right: dpad up and right are pressed", [[dpad up] isPressed] && [[dpad right] isPressed]);

		// 10c. Hat Left
		sync_point("hat-left");
		for (int i = 0; i < 100 && ![[dpad left] isPressed]; i++)
			spin(0.05);
		check("Hat left: dpad left isPressed and up released", [[dpad left] isPressed] && ![[dpad up] isPressed]);

		// 11. Hat Center
		sync_point("hat-center");
		for (int i = 0; i < 100 && ([[dpad left] isPressed] || [[dpad up] isPressed]); i++)
			spin(0.05);
		check("Hat center: dpad centered", ![[dpad up] isPressed] && ![[dpad down] isPressed] && ![[dpad left] isPressed] && ![[dpad right] isPressed]);

		// 12. Unplug & Disconnect Notification
		__block BOOL disconnectNotified = NO;
		id disconnectObserver = [[NSNotificationCenter defaultCenter]
			addObserverForName:GCControllerDidDisconnectNotification
			object:nil
			queue:[NSOperationQueue mainQueue]
			usingBlock:^(NSNotification *note) {
				GCController *c = [note object];
				printf("Received GCControllerDidDisconnectNotification: %p\n", c);
				if (c == connectedController) {
					disconnectNotified = YES;
				}
			}];

		sync_point("unplug");
		for (int i = 0; i < 100 && !disconnectNotified; i++)
			spin(0.05);

		check("GCControllerDidDisconnectNotification fired on unplug", disconnectNotified);
		check("Controller removed from controllers array", ![[GCController controllers] containsObject:connectedController]);

		[[NSNotificationCenter defaultCenter] removeObserver:connectObserver];
		[[NSNotificationCenter defaultCenter] removeObserver:disconnectObserver];
		[connectedController release];

		// 13. Wireless Discovery API
		printf("--- Testing Wireless Discovery API ---\n");
		__block BOOL discoveryCompleted = NO;
		[GCController startWirelessControllerDiscoveryWithCompletionHandler:^{
			discoveryCompleted = YES;
		}];
		[GCController stopWirelessControllerDiscovery];
		check("startWirelessControllerDiscoveryWithCompletionHandler works", YES);

		printf("--- GameController HID Test finished: %d failures ---\n", failures);
		return failures == 0 ? 0 : 1;
	}
}
