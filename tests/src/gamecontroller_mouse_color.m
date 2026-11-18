// CFLAGS: -framework Foundation -framework GameController
// Copyright (C) 2026 Lubos Dolezel
//
// GameController: GCMouse (no mice attached), GCColor, the mouse connect/disconnect
// notifications and the GCInput names for the thumbstick buttons, Xbox paddles and
// DualShock touchpad. Values observed on macOS 26.7.3 (arm64); the program passes there.
#import <Foundation/Foundation.h>
#import <GameController/GameController.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_STR(sym, expected) do { if (![(sym) isEqualToString:(expected)]) { printf("FAIL %d: %s = %s\n", __LINE__, #sym, [(sym) UTF8String]); g_failures++; } } while (0)

int main(void)
{
	@autoreleasepool {
		CHECK_STR(GCMouseDidConnectNotification, @"GCMouseDidConnectNotification");
		CHECK_STR(GCMouseDidDisconnectNotification, @"GCMouseDidDisconnectNotification");

		CHECK_STR(GCInputLeftThumbstickButton, @"Left Thumbstick Button");
		CHECK_STR(GCInputRightThumbstick, @"Right Thumbstick");
		CHECK_STR(GCInputRightThumbstickButton, @"Right Thumbstick Button");
		CHECK_STR(GCInputXboxPaddleOne, @"Paddle 1");
		CHECK_STR(GCInputXboxPaddleTwo, @"Paddle 2");
		CHECK_STR(GCInputXboxPaddleThree, @"Paddle 3");
		CHECK_STR(GCInputXboxPaddleFour, @"Paddle 4");
		CHECK_STR(GCInputDualShockTouchpadOne, @"Touchpad 1");
		CHECK_STR(GCInputDualShockTouchpadTwo, @"Touchpad 2");
		CHECK_STR(GCInputDualShockTouchpadButton, @"Touchpad Button");

		NSArray *mice = [GCMouse mice];
		CHECK(mice != nil && [mice count] == 0);
		CHECK([GCMouse current] == nil);
		CHECK([[GCController controllers] count] == 0);

		GCColor *color = [[GCColor alloc] initWithRed:0.25f green:0.5f blue:0.75f];
		CHECK(color != nil);
		CHECK(color.red == 0.25f && color.green == 0.5f && color.blue == 0.75f);
		CHECK([color conformsToProtocol:@protocol(NSCopying)]);
		CHECK([GCColor supportsSecureCoding]);
		GCColor *copy = [color copy];
		CHECK(copy.red == 0.25f && copy.green == 0.5f && copy.blue == 0.75f);

		// Components are stored as given, not clamped.
		GCColor *wide = [[GCColor alloc] initWithRed:2.0f green:-1.0f blue:0.5f];
		CHECK(wide.red == 2.0f && wide.green == -1.0f && wide.blue == 0.5f);
	}

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
