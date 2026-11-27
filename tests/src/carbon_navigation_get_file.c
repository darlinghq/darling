// Copyright (C) 2026 Lubos Dolezel
//
// Navigation Services of the first API (32-bit only, as in the 32-bit Carbon): NavGetDefaultDialogOptions, the
// type list, and NavChooseFile run to the end: a user who cancels the dialog gets userCanceledErr, a reply that is
// not valid, and an event procedure that is called with kNavCBStart, kNavCBUserAction (with the user action
// kNavUserActionCancel) and kNavCBTerminate. A harness (carbon_dialog_user.py) acts as the user and cancels the
// dialog. (Navigation Services Reference.)
//
//   carbon_navigation_get_file
//
// Needs a window system (X11 or Wayland).
#include <Carbon/Carbon.h>
#include <stdio.h>
#include <string.h>

#ifdef __LP64__
int main(void)
{
	printf("PASS carbon_navigation_get_file (no 64-bit API)\n");
	return 0;
}
#else

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static int g_events[8], g_count;
static NavUserAction g_action = 0xFFFF;

static void navEvent(NavEventCallbackMessage message, NavCBRecPtr parameters, void *userData)
{
	if (g_count < 8)
		g_events[g_count++] = (int) message;
	if (message == kNavCBUserAction)
		g_action = NavDialogGetUserAction(parameters->context);
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	CHECK(NavServicesAvailable());
	CHECK(NavLoad() == noErr);

	NavDialogOptions options;
	memset(&options, 0x55, sizeof(options));
	CHECK(NavGetDefaultDialogOptions(&options) == noErr);
	CHECK(options.version == kNavDialogOptionsVersion && options.dialogOptionFlags == kNavDefaultNavDlogOptions);
	CHECK(options.location.h == -1 && options.location.v == -1);
	CHECK(options.clientName[0] == 0 && options.windowTitle[0] == 0 && options.message[0] == 0 && options.savedFileName[0] == 0);
	CHECK(NavGetDefaultDialogOptions(NULL) == paramErr);

	NavTypeListHandle types = (NavTypeListHandle) NewHandleClear(sizeof(NavTypeList) + sizeof(OSType));
	(**types).osTypeCount = 2;
	(**types).osType[0] = 'TEXT';
	(**types).osType[1] = '????';

	NavReplyRecord reply;
	CHECK(NavChooseFile(NULL, NULL, &options, NULL, NULL, NULL, types, NULL) == paramErr);

	printf("DIALOG_READY 1\n");
	OSErr err = NavChooseFile(NULL, &reply, &options, NewNavEventUPP(navEvent), NULL, NULL, types, NULL);
	CHECK(err == userCanceledErr);
	CHECK(!reply.validRecord);
	CHECK(g_count == 3 && g_events[0] == kNavCBStart && g_events[1] == kNavCBUserAction && g_events[2] == kNavCBTerminate);
	CHECK(g_action == kNavUserActionCancel);
	CHECK(NavDisposeReply(&reply) == noErr);
	DisposeHandle((Handle) types);
	CHECK(NavUnload() == noErr);

	printf(g_failures ? "FAIL carbon_navigation_get_file\n" : "PASS carbon_navigation_get_file\n");
	return g_failures != 0;
}
#endif
