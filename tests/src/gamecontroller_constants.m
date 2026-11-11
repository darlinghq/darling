// Copyright (C) 2026 Lubos Dolezel
//
// GameController exports the connect/disconnect notifications and the GCInput
// element names with the macOS values, and +[GCController controllers] answers
// an empty array when no controller is attached. Values observed on macOS 27
// (arm64); the program passes there with no controller attached.
#import <Foundation/Foundation.h>
#import <GameController/GameController.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_STR(sym, expected) do { if (![(sym) isEqualToString:(expected)]) { printf("FAIL %d: %s = %s\n", __LINE__, #sym, [(sym) UTF8String]); g_failures++; } } while (0)

int main(void)
{
	@autoreleasepool {
		CHECK_STR(GCControllerDidConnectNotification, @"GCControllerDidConnectNotification");
		CHECK_STR(GCControllerDidDisconnectNotification, @"GCControllerDidDisconnectNotification");

		CHECK_STR(GCInputButtonA, @"Button A");
		CHECK_STR(GCInputButtonB, @"Button B");
		CHECK_STR(GCInputButtonX, @"Button X");
		CHECK_STR(GCInputButtonY, @"Button Y");
		CHECK_STR(GCInputButtonMenu, @"Button Menu");
		CHECK_STR(GCInputButtonOptions, @"Button Options");
		CHECK_STR(GCInputLeftShoulder, @"Left Shoulder");
		CHECK_STR(GCInputRightShoulder, @"Right Shoulder");
		CHECK_STR(GCInputLeftTrigger, @"Left Trigger");
		CHECK_STR(GCInputRightTrigger, @"Right Trigger");
		CHECK_STR(GCInputLeftThumbstick, @"Left Thumbstick");
		CHECK_STR(GCInputDirectionPad, @"Direction Pad");

		NSArray *controllers = [GCController controllers];
		CHECK(controllers != nil);
		CHECK([controllers count] == 0);
	}

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
