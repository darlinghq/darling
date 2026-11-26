// Copyright (C) 2026 Lubos Dolezel
//
// The standard menus of Carbon applications: the Window menu made by CreateStandardWindowMenu(), whose commands
// the application handles by default (Minimize, Minimize All, Zoom, Bring All to Front, Arrange in Front, cycling
// and the window list), and the application menu's Hide, Hide Others, Show All and Quit, which the program does
// not add to its own menu. Quit is the quit Apple event: a program that handles it gets it, otherwise the
// application event loop ends (Carbon Event Manager: the standard application handler calls the default quit
// Apple event handler).
//
// The Window menu has the items of the Mojave Menu Manager (black-box observation, macOS 10.14.4 i386); the
// application menu's system items are not items of the menu object (the Apple menu keeps its own three).
#include <Carbon/Carbon.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void quitTimer(EventLoopTimerRef timer, void* info)
{
	QuitApplicationEventLoop();
}

static void spin(double seconds)
{
	EventLoopTimerRef timer;

	InstallEventLoopTimer(GetMainEventLoop(), seconds, 0, NewEventLoopTimerUPP(quitTimer), NULL, &timer);
	RunApplicationEventLoop();
	RemoveEventLoopTimer(timer);
}

static int g_quitEvents;
static int g_quitFromLoop;

static OSErr quitHandler(const AppleEvent* event, AppleEvent* reply, SRefCon refcon)
{
	g_quitEvents++;
	return noErr;
}

static void commandTimer(EventLoopTimerRef timer, void* info)
{
	HICommand command;

	memset(&command, 0, sizeof(command));
	command.commandID = (OSType) (uintptr_t) info;
	ProcessHICommand(&command);
}

// Sends the command from inside the application event loop (so that it ends when the quit command works) and
// runs the loop for at most the given time; returns how long it ran
static double commandInLoop(OSType commandID, double timeout)
{
	EventLoopTimerRef sender, limit;
	CFAbsoluteTime start = CFAbsoluteTimeGetCurrent();

	InstallEventLoopTimer(GetMainEventLoop(), 0.1, 0, NewEventLoopTimerUPP(commandTimer), (void*) (uintptr_t) commandID, &sender);
	InstallEventLoopTimer(GetMainEventLoop(), timeout, 0, NewEventLoopTimerUPP(quitTimer), NULL, &limit);
	RunApplicationEventLoop();
	RemoveEventLoopTimer(sender);
	RemoveEventLoopTimer(limit);
	return CFAbsoluteTimeGetCurrent() - start;
}

static void sendCommand(OSType commandID, MenuRef menu, MenuItemIndex item)
{
	HICommand command;

	memset(&command, 0, sizeof(command));
	command.attributes = kHICommandFromMenu;
	command.commandID = commandID;
	command.menu.menuRef = menu;
	command.menu.menuItemIndex = item;
	ProcessHICommand(&command);
}

static WindowRef makeWindow(const char* title, int x, int y)
{
	WindowRef window = NULL;
	Rect bounds = { (short) y, (short) x, (short) (y + 150), (short) (x + 220) };
	WindowAttributes attributes = kWindowStandardDocumentAttributes | kWindowStandardHandlerAttribute;
#ifdef __LP64__
	attributes |= kWindowCompositingAttribute;
#endif
	if (CreateNewWindow(kDocumentWindowClass, attributes, &bounds, &window) != noErr)
		return NULL;

	CFStringRef name = CFStringCreateWithCString(NULL, title, kCFStringEncodingUTF8);

	SetWindowTitleWithCFString(window, name);
	CFRelease(name);
	ShowWindow(window);
	return window;
}

static OSType commandOf(MenuRef menu, MenuItemIndex item)
{
	MenuCommand command = 0;

	GetMenuItemCommandID(menu, item, &command);
	return command;
}

static bool titleIs(MenuRef menu, MenuItemIndex item, const char* expected)
{
	CFStringRef title = NULL;
	char buffer[128] = "";

	CopyMenuItemTextAsCFString(menu, item, &title);
	if (title)
	{
		CFStringGetCString(title, buffer, sizeof(buffer), kCFStringEncodingUTF8);
		CFRelease(title);
	}
	return strcmp(buffer, expected) == 0;
}

// A raw key event of the key with the character, with the modifiers
static EventRef keyEvent(UInt32 keyCode, UInt8 character, UInt32 modifiers)
{
	EventRef event = NULL;

	CreateEvent(NULL, kEventClassKeyboard, kEventRawKeyDown, 0, kEventAttributeNone, &event);
	SetEventParameter(event, kEventParamKeyCode, typeUInt32, sizeof(keyCode), &keyCode);
	SetEventParameter(event, kEventParamKeyMacCharCodes, typeChar, sizeof(character), &character);
	SetEventParameter(event, kEventParamKeyModifiers, typeUInt32, sizeof(modifiers), &modifiers);
	return event;
}

int main(void)
{
	ProcessSerialNumber self = { 0, kCurrentProcess };

	TransformProcessType(&self, kProcessTransformToForegroundApplication);

	// The Window menu
	MenuRef window = NULL;

	CHECK(CreateStandardWindowMenu(0, NULL) == paramErr);
	CHECK(CreateStandardWindowMenu(0, &window) == noErr && window);

	static const struct { const char* title; OSType command; MenuItemAttributes attributes; } expected[] = {
		{ "Minimize", 'mini', kMenuItemAttrDynamic },
		{ "Minimize All", 'mina', kMenuItemAttrDynamic },
		{ "Zoom", 'zoom', 0 },
		{ "Cycle Through Windows", 'rotw', kMenuItemAttrHidden },
		{ "Cycle Backward", 'rotb', kMenuItemAttrHidden },
		{ NULL, 0, kMenuItemAttrSeparator | kMenuItemAttrDisabled },
		{ "Bring All to Front", 'bfrt', kMenuItemAttrDynamic },
		{ "Arrange in Front", 'frnt', kMenuItemAttrDynamic },
		{ "Window list separator", 'wldv', kMenuItemAttrHidden | kMenuItemAttrSeparator | kMenuItemAttrDisabled },
		{ "Window list terminator", 'wlst', kMenuItemAttrHidden | kMenuItemAttrSeparator | kMenuItemAttrDisabled },
	};

	CHECK(window && CountMenuItems(window) == 10);
	for (int i = 0; window && i < 10; i++)
	{
		MenuItemAttributes attributes = 0;
		UInt8 modifiers = 0;
		UInt16 key = 0;

		CHECK(commandOf(window, i + 1) == expected[i].command);
		GetMenuItemAttributes(window, i + 1, &attributes);
		CHECK(attributes == expected[i].attributes);
		if (expected[i].title)
			CHECK(titleIs(window, i + 1, expected[i].title));
		GetMenuItemModifiers(window, i + 1, &modifiers);
		GetMenuItemCommandKey(window, i + 1, false, &key);
		if (i == 0)
			CHECK(key == 'M' && modifiers == 0);
		if (i == 1)
			CHECK(key == 'M' && modifiers == kMenuOptionModifier);
	}
	MenuAttributes menuAttributes = 0;
	GetMenuAttributes(window, &menuAttributes);
	CHECK(menuAttributes & kMenuAttrAutoDisable);

	// The menu bar: the Apple menu keeps the items the program added, the Window menu goes after the others
	MenuRef apple, file;

	CreateNewMenu(128, 0, &apple);
	SetMenuTitleWithCFString(apple, CFSTR("App"));
	AppendMenuItemTextWithCFString(apple, CFSTR("About"), 0, 'abou', NULL);
	AppendMenuItemTextWithCFString(apple, CFSTR("-"), kMenuItemAttrSeparator, 0, NULL);
	AppendMenuItemTextWithCFString(apple, CFSTR("Preferences"), 0, 'pref', NULL);
	CreateNewMenu(129, 0, &file);
	SetMenuTitleWithCFString(file, CFSTR("File"));
	AppendMenuItemTextWithCFString(file, CFSTR("New"), 0, 'new ', NULL);
	InsertMenu(apple, 0);
	InsertMenu(file, 0);
	InsertMenu(window, 0);
	CHECK(CountMenuItems(apple) == 3);
	CHECK(GetIndMenuItemWithCommandID(NULL, kHICommandQuit, 1, NULL, NULL) == menuItemNotFoundErr);

	// The window commands, handled by default
	WindowRef a = makeWindow("Doc A", 40, 60), b = makeWindow("Doc B", 100, 120), c = makeWindow("Doc C", 160, 180);

	CHECK(a && b && c);
	if (!a || !b || !c)
		return 1;
	spin(0.3);
	SelectWindow(c);
	CHECK(FrontWindow() == c && IsWindowActive(c));

	sendCommand('mini', window, 1);
	CHECK(IsWindowCollapsed(c));
	CHECK(FrontWindow() == b && IsWindowActive(b));
	sendCommand('mina', window, 2);
	CHECK(IsWindowCollapsed(a) && IsWindowCollapsed(b) && IsWindowCollapsed(c));
	CHECK(CollapseWindow(a, false) == noErr && !IsWindowCollapsed(a));
	CHECK(FrontWindow() == a);
	CollapseAllWindows(false);
	CHECK(!IsWindowCollapsed(a) && !IsWindowCollapsed(b) && !IsWindowCollapsed(c));

	Rect before, zoomed, again;
	SelectWindow(a);
	GetWindowBounds(a, kWindowContentRgn, &before);
	sendCommand('zoom', window, 3);
	GetWindowBounds(a, kWindowContentRgn, &zoomed);
	CHECK((zoomed.right - zoomed.left) > (before.right - before.left));
	sendCommand('zoom', window, 3);
	GetWindowBounds(a, kWindowContentRgn, &again);
	CHECK(again.left == before.left && again.top == before.top && again.right == before.right && again.bottom == before.bottom);

	// Cycling: the front window goes to the back, and the other way round
	SelectWindow(a);
	CHECK(FrontWindow() == a);
	sendCommand('rotw', window, 4);
	CHECK(FrontWindow() != a);
	WindowRef next = FrontWindow();
	sendCommand('rotb', window, 5);
	CHECK(FrontWindow() == a && FrontWindow() != next);

	// Arrange in Front and Bring All to Front keep every window and make one active
	sendCommand('frnt', window, 8);
	CHECK(FrontWindow() != NULL && IsWindowVisible(a) && IsWindowVisible(b) && IsWindowVisible(c));
	sendCommand('bfrt', window, 7);
	CHECK(FrontWindow() != NULL);

	// The window list is made when the menu opens (the key equivalent lookup brings the items up to date)
	EventRef key = keyEvent(0x2E, 'm', cmdKey);
	MenuRef found = NULL;
	MenuItemIndex foundItem = 0;

	CHECK(IsMenuKeyEvent(NULL, key, 0, &found, &foundItem) && found == window && foundItem == 1);
	ReleaseEvent(key);
	// separator, markers, then the separator and three windows between the markers
	CHECK(CountMenuItems(window) == 14);
	CHECK(commandOf(window, 9) == 'wldv' && commandOf(window, 14) == 'wlst');
	CHECK(commandOf(window, 11) == 'swin' && commandOf(window, 12) == 'swin' && commandOf(window, 13) == 'swin');
	bool sawA = false;
	for (MenuItemIndex i = 11; i <= 13; i++)
		sawA = sawA || titleIs(window, i, "Doc A");
	CHECK(sawA);

	// Choosing a window of the list makes it the front one, whatever the state of the window
	MenuItemIndex itemOfB = 0;
	for (MenuItemIndex i = 11; i <= 13; i++)
		if (titleIs(window, i, "Doc B"))
			itemOfB = i;
	CHECK(itemOfB != 0);
	CollapseWindow(b, true);
	sendCommand('swin', window, itemOfB);
	CHECK(!IsWindowCollapsed(b) && FrontWindow() == b);

	// Hide and Show All: the application's own visibility, through the Process Manager
	CHECK(IsProcessVisible(&self));
	sendCommand('hide', apple, 4);
	spin(0.5);
	CHECK(!IsProcessVisible(&self));
	CHECK(ShowHideProcess(&self, true) == noErr);
	spin(0.5);
	CHECK(IsProcessVisible(&self));
	ProcessSerialNumber none = { 0, 0xFFFFFFFF };
	CHECK(ShowHideProcess(&none, false) != noErr);
	// Hide Others leaves this application alone, and Show All has nothing of its own to show
	sendCommand('hido', apple, 5);
	sendCommand('shal', apple, 6);
	spin(0.3);
	CHECK(IsProcessVisible(&self));

	// Quit: a program without a handler of the quit Apple event has the application event loop end
	double ran = commandInLoop(kHICommandQuit, 5.0);
	CHECK(ran < 3.0);
	// The key equivalent of the application menu's Quit is Command-Q
	EventRef quitKey = keyEvent(0x0C, 'q', cmdKey);
	CHECK(AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, quitHandler, 0, false) == noErr);
	SendEventToEventTarget(quitKey, GetEventDispatcherTarget());
	ReleaseEvent(quitKey);
	CHECK(g_quitEvents == 1);
	// With a handler the program decides: the loop is not ended
	ran = commandInLoop(kHICommandQuit, 1.0);
	CHECK(ran >= 0.9);
	CHECK(g_quitEvents == 2);
	(void) g_quitFromLoop;

	printf(g_failures ? "FAILED: %d\n" : "PASS\n", g_failures);
	return g_failures != 0;
}
