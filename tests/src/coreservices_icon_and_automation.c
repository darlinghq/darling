// CFLAGS: -framework CoreServices
// Copyright (C) 2026 Darling developers
//
// AEDeterminePermissionToAutomateTarget and GetIconRefFromFileInfo are placeholders. The expected values
// below are the placeholder's own choices (the public pages give only the declarations and availability,
// not the results) and were not observed on a Mac. Darling enforces no Automation consent, so the
// automation function returns noErr (permitted) for any target, whatever askUserIfNeeded says, and
// paramErr for a NULL target. The icon function returns unimpErr, sets *outIconRef to NULL and *outLabel
// to 0, and tolerates NULL output pointers.
// Prints OK, or the failures with a non-zero exit status.
#include <CoreServices/CoreServices.h>
#include <stdbool.h>
#include <stdio.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

static int failures;

#define CHECK(cond, msg) do { if (!(cond)) { printf("FAIL: %s\n", msg); failures++; } } while (0)

int main(void)
{
	// A valid target address: the bundle identifier of an application, as AESendMessage accepts it.
	static const char bundleID[] = "com.example.target";
	AEAddressDesc target;
	AEInitializeDesc(&target);
	OSErr err = AECreateDesc(typeApplicationBundleID, bundleID, sizeof(bundleID) - 1, &target);
	CHECK(err == noErr, "could not create the target address");

	CHECK(AEDeterminePermissionToAutomateTarget(&target, typeWildCard, typeWildCard, false) == noErr,
		"automation permission, no prompt: not noErr");
	CHECK(AEDeterminePermissionToAutomateTarget(&target, kCoreEventClass, kAEOpenApplication, true) == noErr,
		"automation permission, may prompt: not noErr");
	CHECK(AEDeterminePermissionToAutomateTarget(NULL, typeWildCard, typeWildCard, false) == paramErr,
		"automation permission, NULL target: not paramErr");
	AEDisposeDesc(&target);

	// Output parameters are cleared, also when they held values.
	IconRef icon = (IconRef) 0x1;
	SInt16 label = 7;
	OSStatus status = GetIconRefFromFileInfo(NULL, 0, NULL, 0, NULL, 0, &icon, &label);
	CHECK(status == unimpErr, "GetIconRefFromFileInfo: not unimpErr");
	CHECK(icon == NULL, "GetIconRefFromFileInfo: IconRef not cleared");
	CHECK(label == 0, "GetIconRefFromFileInfo: label not cleared");

	// A NULL output pointer is skipped without a crash.
	CHECK(GetIconRefFromFileInfo(NULL, 0, NULL, 0, NULL, 0, NULL, NULL) == unimpErr,
		"GetIconRefFromFileInfo with NULL outputs: not unimpErr");
	icon = (IconRef) 0x1;
	CHECK(GetIconRefFromFileInfo(NULL, 0, NULL, 0, NULL, 0, &icon, NULL) == unimpErr && icon == NULL,
		"GetIconRefFromFileInfo with NULL label: result or IconRef wrong");
	label = 7;
	CHECK(GetIconRefFromFileInfo(NULL, 0, NULL, 0, NULL, 0, NULL, &label) == unimpErr && label == 0,
		"GetIconRefFromFileInfo with NULL IconRef: result or label wrong");

	if (failures == 0)
		printf("OK\n");
	return failures != 0;
}
