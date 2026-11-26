// Copyright (C) 2026 Lubos Dolezel
//
// Navigation Services: the dialog that asks where to save a document (NavCreatePutFileDialog,
// NavDialogRun). The program runs the dialog twice, as an application does from its Save As command, and
// a harness (carbon_dialog_user.py) acts as the user: it accepts the first dialog and cancels the
// second. What is checked is what the documentation promises of the event procedure and the reply:
//   - the event procedure gets kNavCBStart, then kNavCBUserAction, then kNavCBTerminate;
//   - NavDialogRun returns at once (the dialog is not run inside the call) and a second call while the
//     dialog is running is an error (kNavWrongDialogStateErr);
//   - an accepted dialog has the user action kNavUserActionSaveAs and a valid reply, whose selection is
//     the folder to save in (it coerces to an FSRef that names an existing folder) and whose saveFileName
//     is the name that the dialog was given, as the user left it;
//   - a cancelled dialog has kNavUserActionCancel and a reply that is not valid;
//   - the calls refuse the arguments they cannot work with.
//
//   carbon_navigation_put_file
//
// Lines the harness reads:
//   DIALOG_READY n       the dialog n (1: to be accepted, 2: to be cancelled) is shown
//   PASS carbon_navigation_put_file / FAIL ...
//
// Needs a window system (X11 or Wayland) and a host GL driver; 32-bit only (Navigation Services' dialogs
// are not in the 64-bit API).
#include <Carbon/Carbon.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __LP64__
int main(void)
{
	printf("PASS carbon_navigation_put_file (no 64-bit API)\n");
	return 0;
}
#else

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

#define kName CFSTR("NavTestDocument.pdf")

static int g_events[8], g_eventCount;
static int g_dialogNumber;
static int g_done;
static Boolean g_replyValid;
static NavUserAction g_action;
static char g_folder[1024];
static char g_name[256];
static Boolean g_replacing;
static Boolean g_selectionIsFSRef;

static void on_alarm(int sig)
{
	printf("FAIL: timed out\n");
	fflush(stdout);
	_exit(1);
}

static void quitTimer(EventLoopTimerRef timer, void* userData) { QuitApplicationEventLoop(); }

static void navEvent(NavEventCallbackMessage message, NavCBRecPtr parameters, void* userData)
{
	if (g_eventCount < 8)
		g_events[g_eventCount++] = (int) message;
	if (message == kNavCBStart)
	{
		printf("DIALOG_READY %d\n", g_dialogNumber);
		fflush(stdout);
	}
	else if (message == kNavCBUserAction)
	{
		NavReplyRecord reply;
		OSStatus err = NavDialogGetReply(parameters->context, &reply);

		CHECK(err == noErr);
		g_action = NavDialogGetUserAction(parameters->context);
		g_replyValid = reply.validRecord;
		if (reply.validRecord)
		{
			AEDesc fsref;

			g_replacing = reply.replacing;
			CFStringGetCString(reply.saveFileName, g_name, sizeof(g_name), kCFStringEncodingUTF8);
			if (AECoerceDesc(&reply.selection, typeFSRef, &fsref) == noErr)
			{
				FSRef ref;

				g_selectionIsFSRef = true;
				if (AEGetDescData(&fsref, &ref, sizeof(ref)) == noErr)
					FSRefMakePath(&ref, (UInt8*) g_folder, sizeof(g_folder));
				AEDisposeDesc(&fsref);
			}
		}
		NavDisposeReply(&reply);
	}
	else if (message == kNavCBTerminate)
		g_done = 1;
}

static void runDialog(int number)
{
	NavDialogCreationOptions options;
	NavDialogRef dialog = NULL;
	OSStatus err;
	int spins = 0;

	g_dialogNumber = number;
	g_eventCount = 0;
	g_done = 0;
	g_replyValid = false;
	g_action = 0;
	g_folder[0] = g_name[0] = 0;
	g_selectionIsFSRef = false;
	NavGetDefaultDialogCreationOptions(&options);
	options.saveFileName = kName;
	options.modality = kWindowModalityNone;
	err = NavCreatePutFileDialog(&options, 'PDF ', 'prvw', NewNavEventUPP(navEvent), NULL, &dialog);
	CHECK(err == noErr && dialog != NULL);
	CHECK(NavDialogGetUserAction(dialog) == kNavUserActionNone);
	err = NavDialogRun(dialog);
	CHECK(err == noErr);
	// NavDialogRun returned before the dialog did anything
	CHECK(g_eventCount == 0);
	CHECK(NavDialogRun(dialog) == kNavWrongDialogStateErr);
	while (!g_done && spins++ < 600)
	{
		EventLoopTimerRef timer;

		InstallEventLoopTimer(GetMainEventLoop(), 0.1, 0, NewEventLoopTimerUPP(quitTimer), NULL, &timer);
		RunApplicationEventLoop();
		RemoveEventLoopTimer(timer);
	}
	CHECK(g_done);
	CHECK(g_eventCount == 3 && g_events[0] == kNavCBStart && g_events[1] == kNavCBUserAction && g_events[2] == kNavCBTerminate);
	if (number == 1)
	{
		CHECK(g_action == kNavUserActionSaveAs);
		CHECK(g_replyValid);
		CHECK(strcmp(g_name, "NavTestDocument.pdf") == 0);
		CHECK(g_selectionIsFSRef);
		CHECK(g_folder[0] == '/' && access(g_folder, X_OK) == 0);
		printf("saved as %s in %s\n", g_name, g_folder);
	}
	else
	{
		CHECK(g_action == kNavUserActionCancel);
		CHECK(!g_replyValid);
	}
	NavDialogDispose(dialog);
}

int main(void)
{
	Rect bounds = {100, 100, 300, 400};
	WindowRef window;
	NavDialogRef dialog;
	NavReplyRecord reply;

	signal(SIGALRM, on_alarm);
	alarm(100);

	// the calls refuse what they cannot work with
	CHECK(NavCreatePutFileDialog(NULL, 'PDF ', 'prvw', NULL, NULL, NULL) == paramErr);
	CHECK(NavDialogRun(NULL) == paramErr);
	CHECK(NavDialogGetReply(NULL, &reply) == paramErr);
	CHECK(NavDialogGetUserAction(NULL) == kNavUserActionNone);
	CHECK(NavDisposeReply(NULL) == paramErr);
	CHECK(NavCreatePutFileDialog(NULL, 'PDF ', 'prvw', NULL, NULL, &dialog) == noErr);
	NavDialogDispose(dialog);

	CHECK(CreateNewWindow(kDocumentWindowClass, kWindowStandardDocumentAttributes | kWindowStandardHandlerAttribute, &bounds, &window) == noErr);
	ShowWindow(window);
	runDialog(1);
	runDialog(2);
	if (g_failures)
		printf("FAIL carbon_navigation_put_file: %d check(s)\n", g_failures);
	else
		printf("PASS carbon_navigation_put_file\n");
	return g_failures != 0;
}

#endif
