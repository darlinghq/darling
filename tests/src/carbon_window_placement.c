// Copyright (C) 2026 Lubos Dolezel
//
// Where the windows of a Carbon application are put by the window system: the menu bar, a drop-down
// menu, a document window and a floating window. The program creates them and prints what it asked for;
// a harness (carbon_window_placement.py) asks the compositor or the window manager where they are and
// compares: the menu bar along the top of the screen, the drop-down menu below its title, the floating
// window in front of the document window (also when it was shown first), the document window with the
// frame the window system draws, and each window where it was asked to be (where the window system
// lets a client say so).
//
//   carbon_window_placement
//
// Lines the harness reads:
//   WINDOWS_READY doc=L,T,R,B palette=L,T,R,B bar=H   the two windows are shown (content rectangles)
//   MENU_READY menu=X,Y                               a menu at that place (below the bar) is going to be open
//   PASS carbon_window_placement / FAIL ...
//
// Needs a window system (X11 or Wayland) and a host GL driver.
#include <Carbon/Carbon.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Every 64-bit window is a compositing window
#ifdef __LP64__
#define COMP kWindowCompositingAttribute
#else
#define COMP 0
#endif

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void quitTimer(EventLoopTimerRef timer, void* userData) { QuitApplicationEventLoop(); }

static void spin(int milliseconds)
{
	EventLoopTimerRef timer;

	InstallEventLoopTimer(GetMainEventLoop(), milliseconds / 1000.0, 0, NewEventLoopTimerUPP(quitTimer), NULL, &timer);
	RunApplicationEventLoop();
	RemoveEventLoopTimer(timer);
}

static void on_alarm(int sig)
{
	printf("FAIL: timed out\n");
	fflush(stdout);
	_exit(1);
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

static MenuRef g_file;
static int g_menuReturned;

// A pop-up menu at a place of the program's choosing, below the title File of the menu bar; the program
// says so once the menu is going to be open, and the call returns when it is dismissed
static void openMenu(EventLoopTimerRef timer, void* userData)
{
	printf("MENU_READY menu=20,22\n");
	PopUpMenuSelect(g_file, 22, 20, 0);
	g_menuReturned = 1;
	QuitApplicationEventLoop();
}

// A click away from the menu dismisses it
static void* dismiss(void* arg)
{
	usleep(9000000);
	mouse(kCGEventLeftMouseDown, 900, 700);
	usleep(100000);
	mouse(kCGEventLeftMouseUp, 900, 700);
	return NULL;
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

static void item(MenuRef menu, const char* title, MenuCommand command)
{
	MenuItemIndex index;
	CFStringRef string = CFStringCreateWithCString(NULL, title, kCFStringEncodingUTF8);

	AppendMenuItemTextWithCFString(menu, string, 0, command, &index);
	CFRelease(string);
}

static WindowRef makeWindow(WindowClass cls, WindowAttributes attributes, Rect rect, const char* title)
{
	WindowRef window = NULL;
	CFStringRef string = CFStringCreateWithCString(NULL, title, kCFStringEncodingUTF8);

	CHECK(CreateNewWindow(cls, attributes | COMP, &rect, &window) == noErr);
	if (window)
		SetWindowTitleWithCFString(window, string);
	CFRelease(string);
	return window;
}

int main(int argc, char** argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	if (!getenv("DISPLAY") && !getenv("WAYLAND_DISPLAY"))
	{
		printf("SKIP: no window system\n");
		return 0;
	}
	signal(SIGALRM, on_alarm);
	alarm(90);

	MenuRef file = menuWith(6001, "File"), edit = menuWith(6002, "Edit");

	item(file, "New", 'new ');
	item(file, "Open", 'open');
	item(edit, "Copy", 'copy');
	InsertMenu(file, 0);
	InsertMenu(edit, 0);
	DrawMenuBar();

	// The floating window is shown before the document window it floats over
	Rect paletteRect = { 120, 40, 360, 200 };
	Rect docRect = { 240, 320, 520, 720 };
	WindowRef palette = makeWindow(kFloatingWindowClass, kWindowStandardFloatingAttributes, paletteRect, "PlacementPalette");
	WindowRef doc = makeWindow(kDocumentWindowClass, kWindowStandardDocumentAttributes | kWindowStandardHandlerAttribute, docRect, "PlacementDoc");

	if (!palette || !doc)
	{
		printf("FAIL: no windows\n");
		return 1;
	}
	ShowWindow(palette);
	spin(800);
	ShowWindow(doc);
	SelectWindow(doc);
	spin(2500);
	printf("WINDOWS_READY doc=%d,%d,%d,%d palette=%d,%d,%d,%d bar=22\n", docRect.left, docRect.top, docRect.right, docRect.bottom,
		paletteRect.left, paletteRect.top, paletteRect.right, paletteRect.bottom);
	spin(6000);

	pthread_t thread;
	EventLoopTimerRef timer;

	g_file = file;
	pthread_create(&thread, NULL, dismiss, NULL);
	InstallEventLoopTimer(GetMainEventLoop(), 0.5, 0, NewEventLoopTimerUPP(openMenu), NULL, &timer);
	RunApplicationEventLoop();
	CHECK(g_menuReturned);

	CHECK(IsWindowVisible(doc) && IsWindowVisible(palette));
	if (g_failures)
	{
		printf("FAIL: %d checks failed\n", g_failures);
		return 1;
	}
	printf("PASS carbon_window_placement\n");
	return 0;
}
