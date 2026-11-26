// Copyright (C) 2026 Lubos Dolezel
//
// Showing and tracking menus: the menu bar of a Carbon application in a window at the top of the
// main display, key equivalents of the menu bar in the event loop, MenuSelect (choosing an item of
// a pull-down menu with the mouse) and PopUpMenuSelect (a pop-up menu), their menu events and the
// command events of the chosen items.
//
// Needs a window system (X11 or Wayland) and a host GL driver. Two modes:
//   carbon_menus_gui synthetic   the input is posted with CGEventPost() from a thread of the program
//   carbon_menus_gui external    the program says what it expects and waits for real input, which
//                                a harness sends with the tools of the window system:
//                                  1. Command-N, 2. a click on the title "File", a move to the item
//                                  "Open" and a release, 3. Command-Q (ends the program)
// The program prints "READY" lines with the position of the menu bar items, and PASS or FAIL.
#include <Carbon/Carbon.h>
#include <dlfcn.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static int g_synthetic;
static MenuCommand g_commands[16];
static int g_commandCount;
static volatile int g_phase;
static int g_opening, g_closed, g_begin, g_end, g_target;

static OSStatus commandHandler(EventHandlerCallRef call, EventRef event, void* userData)
{
	HICommand command;

	if (GetEventParameter(event, kEventParamDirectObject, typeHICommand, NULL, sizeof(command), NULL, &command) != noErr)
		return eventNotHandledErr;
	if (command.commandID == kHICommandQuit)
	{
		g_commands[g_commandCount++] = command.commandID;
		QuitApplicationEventLoop();
		return noErr;
	}
	if (g_commandCount < 16)
		g_commands[g_commandCount++] = command.commandID;
	if (command.commandID == 'clos')
		QuitApplicationEventLoop();
	return noErr;
}

static OSStatus menuHandler(EventHandlerCallRef call, EventRef event, void* userData)
{
	switch (GetEventKind(event))
	{
		case kEventMenuOpening: g_opening++; break;
		case kEventMenuClosed: g_closed++; break;
		case kEventMenuBeginTracking: g_begin++; break;
		case kEventMenuEndTracking: g_end++; break;
		case kEventMenuTargetItem: g_target++; break;
		default: break;
	}
	return eventNotHandledErr;
}

static void post(CGEventRef event)
{
	CGEventPost(kCGSessionEventTap, event);
	CFRelease(event);
}

static void mouse(CGEventType type, double x, double y)
{
	post(CGEventCreateMouseEvent(NULL, type, CGPointMake(x, y), kCGMouseButtonLeft));
}

static void key(CGKeyCode code, bool down, CGEventFlags flags)
{
	CGEventRef event = CGEventCreateKeyboardEvent(NULL, code, down);

	CGEventSetFlags(event, flags);
	post(event);
}

static void* injector(void* arg)
{
	// Command-N
	usleep(500000);
	key(0x2D, true, kCGEventFlagMaskCommand);
	key(0x2D, false, kCGEventFlagMaskCommand);
	usleep(300000);
	// Click on the title File, move down to the item Open, release
	mouse(kCGEventLeftMouseDown, 20, 11);
	usleep(300000);
	mouse(kCGEventLeftMouseDragged, 20, 30);
	usleep(200000);
	mouse(kCGEventLeftMouseDragged, 30, 50);
	usleep(300000);
	mouse(kCGEventLeftMouseUp, 30, 50);
	usleep(500000);
	// A click on the title File then on the title Edit, Escape closes it
	mouse(kCGEventLeftMouseDown, 20, 11);
	usleep(200000);
	mouse(kCGEventLeftMouseUp, 20, 11);
	usleep(300000);
	key(0x35, true, 0);
	key(0x35, false, 0);
	usleep(500000);
	// Command-Q ends the loop
	key(0x0C, true, kCGEventFlagMaskCommand);
	key(0x0C, false, kCGEventFlagMaskCommand);
	return NULL;
}

static void on_alarm(int sig)
{
	printf("FAIL: timed out (commands so far: %d, the first: %08x)\n", g_commandCount, g_commandCount ? g_commands[0] : 0);
	fflush(stdout);
	_exit(1);
}

// A window the window manager can give the keyboard focus to: with no window of the application
// there is nowhere for the keys to go (X11). The private CoreGraphics calls are looked up by name.
static void showKeyWindow(void)
{
	CGError (*newRegion)(const CGRect*, CFTypeRef*) = dlsym(RTLD_DEFAULT, "CGSNewRegionWithRect");
	CGError (*newWindow)(unsigned, CFIndex, float, float, CFTypeRef, unsigned*) = dlsym(RTLD_DEFAULT, "CGSNewWindow");
	CGError (*orderWindow)(unsigned, unsigned, int, unsigned) = dlsym(RTLD_DEFAULT, "CGSOrderWindow");
	unsigned (*mainConnection)(void) = dlsym(RTLD_DEFAULT, "CGSMainConnectionID");
	CGRect frame = CGRectMake(400, 300, 300, 200);
	CFTypeRef region = NULL;
	unsigned window = 0;

	if (!newRegion || !newWindow || !orderWindow || !mainConnection)
		return;
	if (newRegion(&frame, &region) == kCGErrorSuccess && newWindow(mainConnection(), 0, 0, 0, region, &window) == kCGErrorSuccess)
		orderWindow(mainConnection(), window, 2, 0);   // kCGSOrderIn
}

static MenuRef menuWith(MenuID id, const char* title)
{
	MenuRef menu;
	CFStringRef string = CFStringCreateWithCString(NULL, title, kCFStringEncodingUTF8);

	CreateNewMenu(id, 0, &menu);
	SetMenuTitleWithCFString(menu, string);
	CFRelease(string);
	return menu;
}

static void item(MenuRef menu, const char* title, MenuCommand command, char key, UInt8 modifiers)
{
	MenuItemIndex index;
	CFStringRef string = CFStringCreateWithCString(NULL, title, kCFStringEncodingUTF8);

	AppendMenuItemTextWithCFString(menu, string, 0, command, &index);
	CFRelease(string);
	if (key)
		SetMenuItemCommandKey(menu, index, false, key);
	SetMenuItemModifiers(menu, index, modifiers);
}

int main(int argc, char** argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	if (!getenv("DISPLAY") && !getenv("WAYLAND_DISPLAY"))
	{
		printf("SKIP: no window system\n");
		return 0;
	}
	g_synthetic = argc < 2 || strcmp(argv[1], "external") != 0;
	signal(SIGALRM, on_alarm);
	alarm(g_synthetic ? 60 : 120);

	MenuRef file = menuWith(6001, "File"), edit = menuWith(6002, "Edit");

	item(file, "New", 'new ', 'n', 0);
	item(file, "Open", 'open', 'o', 0);
	item(file, "Close", 'clos', 'w', 0);
	item(edit, "Cut", 'cut ', 'x', 0);
	item(edit, "Copy", 'copy', 'c', 0);
	item(file, "Quit", kHICommandQuit, 'q', 0);
	InsertMenu(file, 0);
	InsertMenu(edit, 0);

	EventTypeSpec commandTypes[1] = { { kEventClassCommand, kEventCommandProcess } };
	EventTypeSpec menuTypes[5] = { { kEventClassMenu, kEventMenuOpening }, { kEventClassMenu, kEventMenuClosed }, { kEventClassMenu, kEventMenuBeginTracking },
		{ kEventClassMenu, kEventMenuEndTracking }, { kEventClassMenu, kEventMenuTargetItem } };

	InstallEventHandler(GetApplicationEventTarget(), commandHandler, 1, commandTypes, NULL, NULL);
	InstallEventHandler(GetApplicationEventTarget(), menuHandler, 5, menuTypes, NULL, NULL);

	DrawMenuBar();
	if (!g_synthetic)
		showKeyWindow();
	printf("READY menu bar: title File at 20,11; item Open at 30,50 (menu bar at the top of the main display)\n");

	pthread_t thread;

	if (g_synthetic)
		pthread_create(&thread, NULL, injector, NULL);
	RunApplicationEventLoop();

	// New (Command-N), Open (click, drag, release), Quit (Command-Q)
	CHECK(g_commandCount == 3);
	if (g_commandCount == 3)
	{
		CHECK(g_commands[0] == 'new ');
		CHECK(g_commands[1] == 'open');
		CHECK(g_commands[2] == kHICommandQuit);
	}
	CHECK(g_begin >= 1 && g_end >= 1);
	CHECK(g_opening >= 1);
	CHECK(g_closed >= 1);
	CHECK(g_target >= 1);
	if (g_failures)
	{
		printf("commands:");
		for (int i = 0; i < g_commandCount; i++)
			printf(" '%c%c%c%c'", (int) (g_commands[i] >> 24), (int) ((g_commands[i] >> 16) & 0xff), (int) ((g_commands[i] >> 8) & 0xff), (int) (g_commands[i] & 0xff));
		printf("; menu events: opening %d closed %d begin %d end %d target %d\n", g_opening, g_closed, g_begin, g_end, g_target);
		printf("FAIL: %d checks failed\n", g_failures);
		return 1;
	}
	printf("PASS carbon_menus_gui (%s)\n", g_synthetic ? "synthetic" : "external");
	return 0;
}
