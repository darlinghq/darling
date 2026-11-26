// Copyright (C) 2026 Lubos Dolezel
//
// The raw key events of the window system in a Carbon application: the parameters they carry (key code,
// modifiers, the Mac Roman character, the Unicode text and the keyboard type) and the Command-key
// equivalents of the menu bar.
//
// Needs a window system (X11 or Wayland) and a host GL driver. Two modes:
//   carbon_key_events synthetic   the keys are posted with CGEventPost() from a thread of the program
//   carbon_key_events external    the program waits for real keys, which a harness types into the window
//                                 "carbon-key-events" with the tools of the window system:
//                                   1. a, 2. Shift-b, 3. Backspace, 4. Command-a, 5. Command-q
// The program prints "READY" when its window is up, and PASS or FAIL.
#include <Carbon/Carbon.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

struct Seen
{
	UInt32 code, modifiers, keyboardType;
	OSStatus codeStatus, modifiersStatus, charStatus, unicodeStatus, typeStatus;
	UInt8 macChar;
	UniChar unicode;
	ByteCount unicodeSize;
};

static struct Seen g_seen[16];
static int g_seenCount, g_selectAll, g_quit;

// The window handler takes every raw key down as handled (it returns the status of its last call, as the
// handlers of some applications do): the menu commands have to run all the same, see
// test_quirk_menu_key_before_window
static int g_swallow = 1;

static int g_windowKeys;

// The dispatcher target sees every key before the standard handlers (the menu bar, the windows) do
static OSStatus dispatcherKey(EventHandlerCallRef call, EventRef event, void* userData)
{
	struct Seen* seen = &g_seen[g_seenCount < 16 ? g_seenCount : 15];

	memset(seen, 0, sizeof(*seen));
	seen->codeStatus = GetEventParameter(event, kEventParamKeyCode, typeUInt32, NULL, sizeof(seen->code), NULL, &seen->code);
	seen->modifiersStatus = GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(seen->modifiers), NULL, &seen->modifiers);
	seen->charStatus = GetEventParameter(event, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof(seen->macChar), NULL, &seen->macChar);
	seen->unicodeStatus = GetEventParameter(event, kEventParamKeyUnicodes, typeUnicodeText, NULL, sizeof(seen->unicode), &seen->unicodeSize, &seen->unicode);
	seen->typeStatus = GetEventParameter(event, kEventParamKeyboardType, typeUInt32, NULL, sizeof(seen->keyboardType), NULL, &seen->keyboardType);
	printf("key: code %u modifiers 0x%x chr %d(0x%02x) uni %d(0x%04x, %lu bytes) type %d(%u)\n", (unsigned) seen->code, (unsigned) seen->modifiers,
		(int) seen->charStatus, seen->macChar, (int) seen->unicodeStatus, seen->unicode, (unsigned long) seen->unicodeSize, (int) seen->typeStatus, (unsigned) seen->keyboardType);
	g_seenCount++;
	return eventNotHandledErr;
}

static OSStatus windowKey(EventHandlerCallRef call, EventRef event, void* userData)
{
	g_windowKeys++;
	return g_swallow ? noErr : eventNotHandledErr;
}

static OSStatus commandHandler(EventHandlerCallRef call, EventRef event, void* userData)
{
	HICommand command;

	if (GetEventParameter(event, kEventParamDirectObject, typeHICommand, NULL, sizeof(command), NULL, &command) != noErr)
		return eventNotHandledErr;
	if (command.commandID == 'sall')
		g_selectAll++;
	else if (command.commandID == kHICommandQuit)
	{
		g_quit++;
		QuitApplicationEventLoop();
	}
	return noErr;
}

static void key(CGKeyCode code, bool down, CGEventFlags flags, UniChar text)
{
	CGEventRef event = CGEventCreateKeyboardEvent(NULL, code, down);

	CGEventSetFlags(event, flags);
	if (text)
		CGEventKeyboardSetUnicodeString(event, 1, &text);
	CGEventPost(kCGSessionEventTap, event);
	CFRelease(event);
}

static void typeKey(CGKeyCode code, CGEventFlags flags, UniChar text)
{
	key(code, true, flags, text);
	key(code, false, flags, text);
	usleep(300000);
}

static void* injector(void* arg)
{
	usleep(1000000);
	typeKey(0x00, 0, 'a');
	typeKey(0x0B, kCGEventFlagMaskShift, 'B');
	typeKey(0x33, 0, 0x08);
	typeKey(0x00, kCGEventFlagMaskCommand, 'a');
	typeKey(0x0C, kCGEventFlagMaskCommand, 'q');
	return NULL;
}

static void on_alarm(int sig)
{
	printf("FAIL: timed out (keys seen %d, select all %d, quit %d)\n", g_seenCount, g_selectAll, g_quit);
	fflush(stdout);
	_exit(1);
}

static void item(MenuRef menu, const char* title, MenuCommand command, char key)
{
	MenuItemIndex index;
	CFStringRef string = CFStringCreateWithCString(NULL, title, kCFStringEncodingUTF8);

	AppendMenuItemTextWithCFString(menu, string, 0, command, &index);
	CFRelease(string);
	SetMenuItemCommandKey(menu, index, false, key);
}

static void checkParameters(const char* what, int index, UInt32 code, UInt32 modifiers, UInt8 macChar, UniChar unicode)
{
	if (index >= g_seenCount)
	{
		printf("FAIL: %s: the event dispatcher saw no such key\n", what);
		g_failures++;
		return;
	}

	struct Seen* seen = &g_seen[index];

	CHECK(seen->codeStatus == noErr && seen->code == code);
	CHECK(seen->modifiersStatus == noErr && seen->modifiers == modifiers);
	// The documented parameters of a raw key event
	CHECK(seen->charStatus == noErr && seen->macChar == macChar);
	CHECK(seen->unicodeStatus == noErr && seen->unicodeSize == sizeof(UniChar) && seen->unicode == unicode);
	CHECK(seen->typeStatus == noErr && seen->keyboardType != 0);
}

int main(int argc, char** argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	if (!getenv("DISPLAY") && !getenv("WAYLAND_DISPLAY"))
	{
		printf("SKIP: no window system\n");
		return 0;
	}

	int synthetic = argc < 2 || strcmp(argv[1], "external") != 0;

	signal(SIGALRM, on_alarm);
	alarm(40);

	MenuRef edit;

	CreateNewMenu(7001, 0, &edit);
	SetMenuTitleWithCFString(edit, CFSTR("Edit"));
	item(edit, "Select All", 'sall', 'a');
	item(edit, "Quit", kHICommandQuit, 'q');
	InsertMenu(edit, 0);
	DrawMenuBar();

	WindowRef window;
	Rect bounds = { 100, 100, 300, 400 };

	CHECK(CreateNewWindow(kDocumentWindowClass, kWindowStandardDocumentAttributes | kWindowCompositingAttribute, &bounds, &window) == noErr);
	SetWindowTitleWithCFString(window, CFSTR("carbon-key-events"));

	EventTypeSpec keyTypes[1] = { { kEventClassKeyboard, kEventRawKeyDown } };
	EventTypeSpec commandTypes[1] = { { kEventClassCommand, kEventCommandProcess } };

	InstallEventHandler(GetEventDispatcherTarget(), dispatcherKey, 1, keyTypes, NULL, NULL);
	InstallEventHandler(GetWindowEventTarget(window), windowKey, 1, keyTypes, NULL, NULL);
	InstallEventHandler(GetApplicationEventTarget(), commandHandler, 1, commandTypes, NULL, NULL);
	ShowWindow(window);
	SelectWindow(window);
	printf("READY\n");

	pthread_t thread;

	if (synthetic)
		pthread_create(&thread, NULL, injector, NULL);
	RunApplicationEventLoop();

	// Key codes of the ANSI layout: A 0, B 11, Delete (Backspace) 51; Command is cmdKey, Shift shiftKey
	checkParameters("a", 0, 0, 0, 'a', 'a');
	checkParameters("Shift-b", 1, 11, shiftKey, 'B', 'B');
	checkParameters("Backspace", 2, 51, 0, 0x08, 0x08);
	checkParameters("Command-a", 3, 0, cmdKey, 'a', 'a');
	// test_quirk_menu_key_before_window: Command-a and Command-q reach the menu bar's commands although the
	// window handler took the keys as handled
	CHECK(g_selectAll == 1);
	CHECK(g_windowKeys == 3);   // a, Shift-b and Backspace; the Command keys went to the menu bar
	CHECK(g_quit == 1);
	if (g_failures)
	{
		printf("FAIL: %d checks failed\n", g_failures);
		return 1;
	}
	printf("PASS carbon_key_events (%s)\n", synthetic ? "synthetic" : "external");
	return 0;
}
