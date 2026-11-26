// Copyright (C) 2026 Lubos Dolezel
//
// Carbon command events (ProcessHICommand(), the commands of menu items) go to the user focus, the active window, and
// from it to the application: a handler installed on the window sees a command before the application's handler, and
// the application's handler gets those the window did not handle. Without an active window the commands go to the
// application. Needs a window system (X11 or Wayland).
#include <Carbon/Carbon.h>
#include <stdio.h>
#include <string.h>

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

enum { kHandled = 'hand', kPassed = 'pass' };

static int g_windowCommands, g_appCommands, g_windowUpdates, g_appFirst;
static UInt32 g_windowLast, g_appLast;

static OSStatus windowHandler(EventHandlerCallRef call, EventRef event, void* userData)
{
	HICommand command;

	if (GetEventParameter(event, kEventParamDirectObject, typeHICommand, NULL, sizeof(command), NULL, &command) != noErr)
		return eventNotHandledErr;
	if (GetEventKind(event) == kEventCommandUpdateStatus)
	{
		g_windowUpdates++;
		return noErr;
	}
	g_windowCommands++;
	g_windowLast = command.commandID;
	return command.commandID == kHandled ? noErr : eventNotHandledErr;
}

static OSStatus appHandler(EventHandlerCallRef call, EventRef event, void* userData)
{
	HICommand command;

	if (GetEventKind(event) != kEventCommandProcess || GetEventParameter(event, kEventParamDirectObject, typeHICommand, NULL, sizeof(command), NULL, &command) != noErr)
		return eventNotHandledErr;
	g_appCommands++;
	g_appLast = command.commandID;
	if (g_windowCommands == 0)
		g_appFirst++;
	return command.commandID == kPassed ? noErr : eventNotHandledErr;
}

static void quitTimer(EventLoopTimerRef t, void* u) { QuitApplicationEventLoop(); }

static void spin(int ms)
{
	EventLoopTimerRef timer;

	InstallEventLoopTimer(GetMainEventLoop(), ms / 1000.0, 0, NewEventLoopTimerUPP(quitTimer), NULL, &timer);
	RunApplicationEventLoop();
	RemoveEventLoopTimer(timer);
}

static OSStatus send(UInt32 id)
{
	HICommand command;

	memset(&command, 0, sizeof(command));
	command.commandID = id;
	return ProcessHICommand(&command);
}

#ifdef __LP64__
int main(void)
{
	printf("PASS carbon_command_focus_window (the Window Manager is 32-bit only)\n");
	return 0;
}
#else
int main(void)
{
	Rect bounds = { 100, 100, 220, 300 };
	WindowRef window = NULL;
	static const EventTypeSpec types[] = { { kEventClassCommand, kEventCommandProcess }, { kEventClassCommand, kEventCommandUpdateStatus } };
	static const EventTypeSpec appTypes[] = { { kEventClassCommand, kEventCommandProcess } };

	InstallApplicationEventHandler(NewEventHandlerUPP(appHandler), 1, appTypes, NULL, NULL);

	// No window: the application gets the command
	CHECK(send(kPassed) == noErr);
	CHECK(g_appCommands == 1 && g_windowCommands == 0);

	CHECK(CreateNewWindow(kDocumentWindowClass, kWindowStandardDocumentAttributes | kWindowStandardHandlerAttribute, &bounds, &window) == noErr);
	CHECK(InstallWindowEventHandler(window, NewEventHandlerUPP(windowHandler), 2, types, NULL, NULL) == noErr);
	ShowWindow(window);
	SelectWindow(window);
	for (int i = 0; i < 40 && !IsWindowActive(window); i++)
		spin(100);
	CHECK(IsWindowActive(window));

	// The active window handles a command: the application does not see it
	g_appCommands = g_windowCommands = 0;
	CHECK(send(kHandled) == noErr);
	CHECK(g_windowCommands == 1 && g_windowLast == kHandled);
	CHECK(g_appCommands == 0);

	// A command the window does not handle goes on to the application, after the window
	g_appCommands = g_windowCommands = g_appFirst = 0;
	CHECK(send(kPassed) == noErr);
	CHECK(g_windowCommands == 1 && g_windowLast == kPassed);
	CHECK(g_appCommands == 1 && g_appLast == kPassed && g_appFirst == 0);

	// The command status updates go to the window as well
	{
		EventRef event;
		HICommand command;

		memset(&command, 0, sizeof(command));
		command.commandID = kHandled;
		CHECK(CreateEvent(NULL, kEventClassCommand, kEventCommandUpdateStatus, 0, kEventAttributeNone, &event) == noErr);
		SetEventParameter(event, kEventParamDirectObject, typeHICommand, sizeof(command), &command);
		g_windowUpdates = 0;
		CHECK(SendEventToEventTarget(event, GetEventDispatcherTarget()) == noErr);
		CHECK(g_windowUpdates == 1);
		ReleaseEvent(event);
	}

	// A hidden window is no focus any more
	DisposeWindow(window);
	spin(300);
	g_appCommands = g_windowCommands = 0;
	CHECK(send(kPassed) == noErr);
	CHECK(g_appCommands == 1 && g_windowCommands == 0);

	printf(g_failures ? "FAIL carbon_command_focus_window\n" : "PASS carbon_command_focus_window\n");
	return g_failures != 0;
}
#endif
