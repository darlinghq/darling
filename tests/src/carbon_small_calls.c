// Copyright (C) 2026 Lubos Dolezel
//
// Small Carbon calls and constants of the 32-bit API: TrackMouseLocationWithOptions with a timeout,
// GetEventTypeCount, HICommandExtended, AddPt, SetThemeCursor and the Navigation Services defaults.
#include <Carbon/Carbon.h>
#include <stdio.h>
#include <string.h>

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

int main(void)
{
	static const EventTypeSpec types[] = { { kEventClassWindow, kEventWindowDrawContent }, { kEventClassMouse, kEventMouseDown }, { kEventClassKeyboard, kEventRawKeyDown } };

	CHECK(GetEventTypeCount(types) == 3);
	CHECK(sizeof(HICommandExtended) >= sizeof(HICommand));
	HICommandExtended command;
	memset(&command, 0, sizeof(command));
	command.commandID = 'abcd';
	CHECK(command.legacy.commandID == 'abcd');
	CHECK(kBackspaceCharCode == 8 && kEscapeCharCode == 27 && kDeleteCharCode == 127 && kReturnCharCode == 13);

#ifndef __LP64__
	Point a = { 10, 20 }, b = { 1, 2 };
	AddPt(a, &b);
	CHECK(b.v == 11 && b.h == 22);

	// With nothing happening the tracking times out, with the position of the mouse
	MouseTrackingResult result = 0;
	Point where = { 0, 0 };
	UInt32 modifiers = 0xFFFF;
	CHECK(TrackMouseLocationWithOptions(NULL, 0, 0.05, &where, &modifiers, &result) == noErr);
	CHECK(result == kMouseTrackingTimedOut);
	CHECK(TrackMouseLocationWithOptions(NULL, 0, 0.05, NULL, &modifiers, &result) == paramErr);
	CHECK(kMouseTrackingMouseUp == 2 && kMouseTrackingMouseDragged == 5 && kMouseTrackingUserCancelled == 7);

	CHECK(SetThemeCursor(kThemeArrowCursor) == noErr);
	CHECK(SetThemeCursor(9999) == themeBadCursorIndexErr);

	NavDialogCreationOptions options;
	memset(&options, 0x55, sizeof(options));
	CHECK(NavGetDefaultDialogCreationOptions(&options) == noErr);
	CHECK(options.version == kNavDialogCreationOptionsVersion && options.optionFlags == kNavDefaultNavDlogOptions);
	CHECK(options.location.h == -1 && options.location.v == -1 && options.saveFileName == NULL && options.parentWindow == NULL);
	CHECK(NavGetDefaultDialogCreationOptions(NULL) == paramErr);
	NavDialogRef dialog = NULL;
	CHECK(NavCreatePutFileDialog(&options, 'PDF ', 'CSKt', NULL, NULL, &dialog) == noErr && dialog);
	CHECK(NavDialogGetUserAction(dialog) == kNavUserActionNone);
	NavDialogDispose(dialog);
#endif

	printf(g_failures ? "FAIL carbon_small_calls\n" : "PASS carbon_small_calls\n");
	return g_failures != 0;
}
