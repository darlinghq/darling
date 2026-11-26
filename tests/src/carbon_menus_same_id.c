// Copyright (C) 2026 Lubos Dolezel
//
// The commands of the items of the menu bar when several menus have the same menu ID (the menus of a nib have the
// ID 0 when they have none of their own): the command of the item that was chosen is processed, not that of the item
// with the same index in the first menu that has the ID.
//
// Needs a window system (X11 or Wayland); the input is posted with CGEventPost() from a thread of the program.
#include <Carbon/Carbon.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static MenuCommand g_commands[8];
static int g_commandCount;

static OSStatus commandHandler(EventHandlerCallRef call, EventRef event, void* userData)
{
	HICommand command;

	if (GetEventParameter(event, kEventParamDirectObject, typeHICommand, NULL, sizeof(command), NULL, &command) != noErr)
		return eventNotHandledErr;
	if (command.commandID == kHICommandQuit)
	{
		QuitApplicationEventLoop();
		return noErr;
	}
	if (g_commandCount < 8)
		g_commands[g_commandCount++] = command.commandID;
	return noErr;
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

// Titles of the menus in the bar, from the left: Edit, File. The item to choose is in the second menu.
static void* injector(void* arg)
{
	usleep(800000);
	mouse(kCGEventLeftMouseDown, 70, 11);
	usleep(300000);
	mouse(kCGEventLeftMouseDragged, 70, 30);
	usleep(200000);
	mouse(kCGEventLeftMouseDragged, 75, 50);
	usleep(300000);
	mouse(kCGEventLeftMouseUp, 75, 50);
	usleep(500000);
	key(0x0C, true, kCGEventFlagMaskCommand);
	key(0x0C, false, kCGEventFlagMaskCommand);
	return NULL;
}

static void on_alarm(int sig)
{
	printf("FAIL: timed out (commands %d)\n", g_commandCount);
	fflush(stdout);
	_exit(1);
}

static MenuRef menuWith(const char* title)
{
	MenuRef menu;
	CFStringRef string = CFStringCreateWithCString(NULL, title, kCFStringEncodingUTF8);

	CreateNewMenu(0, 0, &menu);
	SetMenuTitleWithCFString(menu, string);
	CFRelease(string);
	return menu;
}

static void item(MenuRef menu, const char* title, MenuCommand command, char key)
{
	MenuItemIndex index;
	CFStringRef string = CFStringCreateWithCString(NULL, title, kCFStringEncodingUTF8);

	AppendMenuItemTextWithCFString(menu, string, 0, command, &index);
	CFRelease(string);
	if (key)
		SetMenuItemCommandKey(menu, index, false, key);
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	if (!getenv("DISPLAY") && !getenv("WAYLAND_DISPLAY"))
	{
		printf("SKIP: no window system\n");
		return 0;
	}
	signal(SIGALRM, on_alarm);
	alarm(60);

	MenuRef edit = menuWith("Edit"), file = menuWith("File");

	item(edit, "Cut", 'cut ', 'x');
	item(edit, "Copy", 'copy', 'c');
	item(file, "New", 'new ', 'n');
	item(file, "Open", 'open', 'o');
	item(file, "Quit", kHICommandQuit, 'q');
	InsertMenu(edit, 0);
	InsertMenu(file, 0);

	EventTypeSpec commandTypes[1] = { { kEventClassCommand, kEventCommandProcess } };

	InstallEventHandler(GetApplicationEventTarget(), commandHandler, 1, commandTypes, NULL, NULL);
	DrawMenuBar();

	pthread_t thread;

	pthread_create(&thread, NULL, injector, NULL);
	RunApplicationEventLoop();

	// Open, the second item of the second menu (the second item of the first menu is Copy)
	CHECK(g_commandCount == 1);
	if (g_commandCount == 1)
		CHECK(g_commands[0] == 'open');
	printf(g_failures ? "FAIL carbon_menus_same_id\n" : "PASS carbon_menus_same_id\n");
	return g_failures != 0;
}
