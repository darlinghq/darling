// Copyright (C) 2026 Lubos Dolezel
//
// The Menu Manager: menu objects, items, command lookup, the menu bar, key equivalents and the
// command and menu events. Behaviour of the documented calls follows the Menu Manager reference;
// where the reference is silent (lookup order of commands with the same ID, item 0, the errors for
// items out of range, retain counts) it follows black-box observation of macOS 10.14.4 (i386 and
// x86_64) and 26.7.3 (x86_64), which agree. The tests of those behaviours are named quirk_...
//
// The calls that exist in the 32-bit Menu Manager only (Pascal strings, MENU resources, handles,
// EnableMenuCommand, MenuKey, ...) are tested when the program is built for i386. Runs without a
// window system: nothing here draws the menu bar.
#include <Carbon/Carbon.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_EQ(a, b) do { long long _a = (long long) (a), _b = (long long) (b); if (_a != _b) { printf("FAIL line %d: %s = %lld, expected %lld\n", __LINE__, #a, _a, _b); g_failures++; } } while (0)

#define CC(a, b, c, d) ((UInt32) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d)))

static MenuRef newMenu(MenuID id, const char* title)
{
	MenuRef menu = NULL;
	CFStringRef string = CFStringCreateWithCString(NULL, title, kCFStringEncodingUTF8);

	CHECK_EQ(CreateNewMenu(id, 0, &menu), noErr);
	SetMenuTitleWithCFString(menu, string);
	CFRelease(string);
	return menu;
}

static MenuItemIndex addItem(MenuRef menu, const char* title, MenuCommand command)
{
	MenuItemIndex index = 0;
	CFStringRef string = CFStringCreateWithCString(NULL, title, kCFStringEncodingUTF8);

	CHECK_EQ(AppendMenuItemTextWithCFString(menu, string, 0, command, &index), noErr);
	CFRelease(string);
	return index;
}

static CFIndex retainCount(MenuRef menu)
{
	return CFGetRetainCount((CFTypeRef) menu);
}

static void test_menu_object(void)
{
	MenuRef menu = newMenu(1001, "A");

	CHECK(menu != NULL);
	CHECK(CFGetTypeID((CFTypeRef) menu) != 0);
	CHECK_EQ(retainCount(menu), 1);
	CHECK_EQ(GetMenuID(menu), 1001);
	CHECK_EQ(CountMenuItems(menu), 0);
	CFStringRef title = NULL;
	CHECK_EQ(CopyMenuTitleAsCFString(menu, &title), noErr);
	CHECK(title && CFEqual(title, CFSTR("A")));
	if (title)
		CFRelease(title);

	// A menu in the list of inserted menus is found by its ID, and the list holds a reference
	InsertMenu(menu, 0);
	CHECK_EQ(retainCount(menu), 2);
	CHECK(GetMenuHandle(1001) == menu);
	CHECK(GetMenuHandle(1002) == NULL);
	DeleteMenu(1001);
	CHECK(GetMenuHandle(1001) == NULL);
	CHECK_EQ(retainCount(menu), 1);
	CFRelease((CFTypeRef) menu);
}

static void test_quirk_insert_menu_twice(void)
{
	// QUIRK pinned: a menu that is in the list is not held a second time by another InsertMenu
	MenuRef menu = newMenu(1002, "Twice");

	InsertMenu(menu, 0);
	CHECK_EQ(retainCount(menu), 2);
	InsertMenu(menu, 0);
	CHECK_EQ(retainCount(menu), 2);
	DeleteMenu(1002);
	CHECK_EQ(retainCount(menu), 1);
	CFRelease((CFTypeRef) menu);
}

static void test_items(void)
{
	MenuRef menu = newMenu(1010, "Items");

	CHECK_EQ(addItem(menu, "One", 'tst1'), 1);
	CHECK_EQ(addItem(menu, "Two", 'tst2'), 2);
	CHECK_EQ(CountMenuItems(menu), 2);

	// An item after item 0 is the first; after one that does not exist, the last
	CFStringRef string = CFSTR("First");
	CHECK_EQ(InsertMenuItemTextWithCFString(menu, string, 0, 0, 'ins1'), noErr);
	CHECK_EQ(InsertMenuItemTextWithCFString(menu, CFSTR("Last"), 100, 0, 'ins2'), noErr);
	CHECK_EQ(CountMenuItems(menu), 4);

	CFStringRef text = NULL;
	CHECK_EQ(CopyMenuItemTextAsCFString(menu, 1, &text), noErr);
	CHECK(text && CFEqual(text, CFSTR("First")));
	if (text)
		CFRelease(text);
	CHECK_EQ(CopyMenuItemTextAsCFString(menu, 4, &text), noErr);
	CHECK(text && CFEqual(text, CFSTR("Last")));
	if (text)
		CFRelease(text);

	CHECK_EQ(SetMenuItemTextWithCFString(menu, 2, CFSTR("Renamed")), noErr);
	CHECK_EQ(CopyMenuItemTextAsCFString(menu, 2, &text), noErr);
	CHECK(text && CFEqual(text, CFSTR("Renamed")));
	if (text)
		CFRelease(text);

	MenuCommand command = 0;
	CHECK_EQ(GetMenuItemCommandID(menu, 1, &command), noErr);
	CHECK_EQ(command, 'ins1');
	CHECK_EQ(SetMenuItemCommandID(menu, 1, 'new1'), noErr);
	CHECK_EQ(GetMenuItemCommandID(menu, 1, &command), noErr);
	CHECK_EQ(command, 'new1');

	// Items out of range: the calls that return a status give paramErr, item 0 is not an item
	text = (CFStringRef) 1;
	CHECK_EQ(CopyMenuItemTextAsCFString(menu, 99, &text), paramErr);
	CHECK(text == (CFStringRef) 1);   // not written on an error
	CHECK_EQ(CopyMenuItemTextAsCFString(menu, 0, &text), paramErr);
	CHECK_EQ(GetMenuItemCommandID(menu, 99, &command), paramErr);
	CHECK_EQ(GetMenuItemCommandID(menu, 0, &command), paramErr);
	CHECK_EQ(SetMenuItemCommandID(menu, 99, 1), paramErr);

	// Deleting an item that does not exist does nothing
	DeleteMenuItem(menu, 99);
	CHECK_EQ(CountMenuItems(menu), 4);
	DeleteMenuItem(menu, 1);
	CHECK_EQ(CountMenuItems(menu), 3);
	CHECK_EQ(DeleteMenuItems(menu, 1, 2), noErr);
	CHECK_EQ(CountMenuItems(menu), 1);

	// Defaults of a new item
	MenuItemIndex item = addItem(menu, "Plain", 0);
	UInt8 modifiers = 99;
	UInt16 key = 99;
	MenuItemAttributes attributes = 99;
	CHECK_EQ(GetMenuItemModifiers(menu, item, &modifiers), noErr);
	CHECK_EQ(modifiers, 0);
	CHECK_EQ(GetMenuItemCommandKey(menu, item, false, &key), noErr);
	CHECK_EQ(key, 0);
	CHECK_EQ(GetMenuItemAttributes(menu, item, &attributes), noErr);
	CHECK_EQ(attributes, 0);
	CharParameter mark = 99;
	GetItemMark(menu, item, &mark);
	CHECK_EQ(mark, 0);
	UInt32 refCon = 99;
	CHECK_EQ(GetMenuItemRefCon(menu, item, &refCon), noErr);
	CHECK_EQ(refCon, 0);

	SetMenuItemCommandKey(menu, item, false, 'x');
	SetMenuItemModifiers(menu, item, kMenuShiftModifier | kMenuNoCommandModifier);
	CHECK_EQ(GetMenuItemCommandKey(menu, item, false, &key), noErr);
	CHECK_EQ(key, 'x');
	CHECK_EQ(GetMenuItemModifiers(menu, item, &modifiers), noErr);
	CHECK_EQ(modifiers, kMenuShiftModifier | kMenuNoCommandModifier);
	CheckMenuItem(menu, item, true);
	GetItemMark(menu, item, &mark);
	CHECK_EQ(mark, checkMark);
	CheckMenuItem(menu, item, false);
	GetItemMark(menu, item, &mark);
	CHECK_EQ(mark, noMark);
	CHECK_EQ(SetMenuItemRefCon(menu, item, 1234), noErr);
	CHECK_EQ(GetMenuItemRefCon(menu, item, &refCon), noErr);
	CHECK_EQ(refCon, 1234);

	CFRelease((CFTypeRef) menu);
}

static void test_quirk_separator_is_disabled(void)
{
	// QUIRK pinned: an item made a separator is disabled too
	MenuRef menu = newMenu(1011, "Sep");
	MenuItemIndex item = addItem(menu, "x", 0);
	MenuItemAttributes attributes = 0;

	CHECK_EQ(ChangeMenuItemAttributes(menu, item, kMenuItemAttrSeparator, 0), noErr);
	CHECK_EQ(GetMenuItemAttributes(menu, item, &attributes), noErr);
	CHECK_EQ(attributes, kMenuItemAttrSeparator | kMenuItemAttrDisabled);
	CHECK(!IsMenuItemEnabled(menu, item));

	MenuItemIndex made = 0;
	CHECK_EQ(AppendMenuItemTextWithCFString(menu, CFSTR("-"), kMenuItemAttrSeparator, 0, &made), noErr);
	CHECK(!IsMenuItemEnabled(menu, made));
	CFRelease((CFTypeRef) menu);
}

static void test_enable_and_item_zero(void)
{
	MenuRef menu = newMenu(1012, "Enable");
	MenuItemIndex a = addItem(menu, "a", 0), b = addItem(menu, "b", 0);

	CHECK(IsMenuItemEnabled(menu, a));
	DisableMenuItem(menu, b);
	CHECK(!IsMenuItemEnabled(menu, b));
	CHECK(IsMenuItemEnabled(menu, a));

	// Item 0 is the menu: disabling it leaves the items as they are
	CHECK(IsMenuItemEnabled(menu, 0));
	DisableMenuItem(menu, 0);
	CHECK(!IsMenuItemEnabled(menu, 0));
	CHECK(IsMenuItemEnabled(menu, a));
	EnableMenuItem(menu, 0);
	CHECK(IsMenuItemEnabled(menu, 0));
	CHECK(!IsMenuItemEnabled(menu, b));

	// Items that do not exist are not enabled, and changing them does nothing
	CHECK(!IsMenuItemEnabled(menu, 99));
	CHECK(!IsMenuItemEnabled(menu, 3));
	DisableMenuItem(menu, 99);
	EnableMenuItem(menu, 99);
	CHECK_EQ(CountMenuItems(menu), 2);
	CFRelease((CFTypeRef) menu);
}

static void test_hierarchical_menus(void)
{
	MenuRef menu = newMenu(1013, "Parent"), sub = newMenu(1014, "Sub");
	MenuItemIndex item = addItem(menu, "Has sub", 0);
	MenuRef found = (MenuRef) 1;

	CHECK_EQ(retainCount(sub), 1);
	CHECK_EQ(SetMenuItemHierarchicalMenu(menu, item, sub), noErr);
	// The item holds a reference to the submenu
	CHECK_EQ(retainCount(sub), 2);
	CHECK_EQ(GetMenuItemHierarchicalMenu(menu, item, &found), noErr);
	CHECK(found == sub);
	CHECK_EQ(retainCount(sub), 2);

	// An item without a submenu gives no error and a NULL menu
	MenuItemIndex plain = addItem(menu, "No sub", 0);
	CHECK_EQ(GetMenuItemHierarchicalMenu(menu, plain, &found), noErr);
	CHECK(found == NULL);

	CHECK_EQ(SetMenuItemHierarchicalMenu(menu, item, NULL), noErr);
	CHECK_EQ(retainCount(sub), 1);
	SetMenuItemHierarchicalMenu(menu, item, sub);
	CFRelease((CFTypeRef) menu);
	// The submenu is released with the parent
	CHECK_EQ(retainCount(sub), 1);
	CFRelease((CFTypeRef) sub);
}

static void test_command_lookup(void)
{
	MenuRef a = newMenu(2001, "A"), b = newMenu(2002, "B");
	MenuCommand dup = 'dup!';

	addItem(a, "one", 'one!');
	addItem(a, "dup in A", dup);
	addItem(a, "dup in A, again", dup);
	addItem(b, "dup in B", dup);
	InsertMenu(a, 0);
	InsertMenu(b, 0);

	MenuRef menu = NULL;
	MenuItemIndex index = 0;

	// The index is counted from 1; 0 is not an index
	CHECK_EQ(GetIndMenuItemWithCommandID(NULL, dup, 0, &menu, &index), paramErr);
	CHECK_EQ(GetIndMenuItemWithCommandID(NULL, dup, 1, &menu, &index), noErr);
	CHECK(menu == a && index == 2);
	CHECK_EQ(GetIndMenuItemWithCommandID(NULL, dup, 2, &menu, &index), noErr);
	CHECK(menu == a && index == 3);
	CHECK_EQ(GetIndMenuItemWithCommandID(NULL, dup, 3, &menu, &index), noErr);
	CHECK(menu == b && index == 1);
	CHECK_EQ(GetIndMenuItemWithCommandID(NULL, dup, 4, &menu, &index), menuItemNotFoundErr);
	CHECK_EQ(GetIndMenuItemWithCommandID(NULL, 'nope', 1, &menu, &index), menuItemNotFoundErr);
	CHECK_EQ(GetIndMenuItemWithCommandID(NULL, 0, 1, &menu, &index), paramErr);
	// Starting at a menu searches that menu (and its submenus), not the rest of the menu bar
	CHECK_EQ(GetIndMenuItemWithCommandID(a, dup, 3, &menu, &index), menuItemNotFoundErr);
	CHECK_EQ(GetIndMenuItemWithCommandID(b, dup, 1, &menu, &index), noErr);
	CHECK(menu == b && index == 1);

	// Only the first item with the command is changed by DisableMenuCommand, whether it is enabled or
	// not: a second call changes it again, not the next one
	DisableMenuCommand(NULL, dup);
	CHECK(!IsMenuItemEnabled(a, 2));
	CHECK(IsMenuItemEnabled(a, 3));
	CHECK(IsMenuItemEnabled(b, 1));
	DisableMenuCommand(a, dup);
	CHECK(IsMenuItemEnabled(a, 3));
	CHECK(IsMenuItemEnabled(b, 1));
	DisableMenuCommand(b, dup);
	CHECK(!IsMenuItemEnabled(b, 1));
	// Command 0 is not a command: nothing changes
	DisableMenuCommand(NULL, 0);
	CHECK(IsMenuItemEnabled(a, 1));

	DeleteMenu(2001);
	DeleteMenu(2002);
	CFRelease((CFTypeRef) a);
	CFRelease((CFTypeRef) b);
}

static void test_quirk_command_lookup_order(void)
{
	// QUIRK pinned: from the menu bar, the items of a submenu come right after the item that has
	// it (x1 xh y1 yh z1 y3 x3 w1); from a menu, its own items come first (x1 xh x3 y1 yh y3 z1)
	MenuRef x = newMenu(2010, "X"), y = newMenu(2011, "Y"), z = newMenu(2012, "Z"), w = newMenu(2013, "W");
	MenuCommand c = 'ord!';

	addItem(x, "x1", c);
	MenuItemIndex xh = addItem(x, "xh", c);
	addItem(x, "x3", c);
	addItem(y, "y1", c);
	MenuItemIndex yh = addItem(y, "yh", c);
	addItem(y, "y3", c);
	addItem(z, "z1", c);
	addItem(w, "w1", c);
	SetMenuItemHierarchicalMenu(x, xh, y);
	SetMenuItemHierarchicalMenu(y, yh, z);
	InsertMenu(x, 0);
	InsertMenu(w, 0);

	struct { MenuID menu; MenuItemIndex item; } fromBar[] = { { 2010, 1 }, { 2010, 2 }, { 2011, 1 }, { 2011, 2 }, { 2012, 1 }, { 2011, 3 }, { 2010, 3 }, { 2013, 1 } };
	struct { MenuID menu; MenuItemIndex item; } fromX[] = { { 2010, 1 }, { 2010, 2 }, { 2010, 3 }, { 2011, 1 }, { 2011, 2 }, { 2011, 3 }, { 2012, 1 } };

	for (UInt32 i = 0; i < 8; i++)
	{
		MenuRef menu = NULL;
		MenuItemIndex index = 0;

		CHECK_EQ(GetIndMenuItemWithCommandID(NULL, c, i + 1, &menu, &index), noErr);
		CHECK(menu && GetMenuID(menu) == fromBar[i].menu && index == fromBar[i].item);
	}
	for (UInt32 i = 0; i < 7; i++)
	{
		MenuRef menu = NULL;
		MenuItemIndex index = 0;

		CHECK_EQ(GetIndMenuItemWithCommandID(x, c, i + 1, &menu, &index), noErr);
		CHECK(menu && GetMenuID(menu) == fromX[i].menu && index == fromX[i].item);
	}
	// A menu that is not in the menu bar and not a submenu of one is not searched from NULL
	MenuRef lone = newMenu(2014, "Lone");
	MenuRef menu;
	MenuItemIndex index;

	addItem(lone, "lone", 'lon!');
	CHECK_EQ(GetIndMenuItemWithCommandID(NULL, 'lon!', 1, &menu, &index), menuItemNotFoundErr);
	CHECK_EQ(GetIndMenuItemWithCommandID(lone, 'lon!', 1, &menu, &index), noErr);
	CFRelease((CFTypeRef) lone);

	DeleteMenu(2010);
	DeleteMenu(2013);
	CFRelease((CFTypeRef) x);
	CFRelease((CFTypeRef) y);
	CFRelease((CFTypeRef) z);
	CFRelease((CFTypeRef) w);
}

static void test_properties(void)
{
	MenuRef menu = newMenu(1020, "Props");
	MenuItemIndex item = addItem(menu, "p", 0);
	ByteCount size = 0;
	char buffer[8] = "";

	CHECK_EQ(GetMenuItemProperty(menu, item, 'tst ', 'prop', sizeof(buffer), &size, buffer), menuPropertyNotFoundErr);
	CHECK_EQ(SetMenuItemProperty(menu, item, 'tst ', 'prop', 3, "abc"), noErr);
	CHECK_EQ(GetMenuItemProperty(menu, item, 'tst ', 'prop', sizeof(buffer), &size, buffer), noErr);
	CHECK_EQ(size, 3);
	CHECK(memcmp(buffer, "abc", 3) == 0);
	CHECK_EQ(SetMenuItemProperty(menu, item, 'tst ', 'prop', 2, "xy"), noErr);
	CHECK_EQ(GetMenuItemProperty(menu, item, 'tst ', 'prop', sizeof(buffer), &size, buffer), noErr);
	CHECK_EQ(size, 2);
	CHECK_EQ(RemoveMenuItemProperty(menu, item, 'tst ', 'prop'), noErr);
	CHECK_EQ(GetMenuItemProperty(menu, item, 'tst ', 'prop', sizeof(buffer), &size, buffer), menuPropertyNotFoundErr);
	CHECK_EQ(RemoveMenuItemProperty(menu, item, 'tst ', 'prop'), menuPropertyNotFoundErr);
	CFRelease((CFTypeRef) menu);
}

static void test_quirk_property_small_buffer(void)
{
	// QUIRK pinned: a buffer smaller than the property is not an error; the actual size is the
	// size of the property
	MenuRef menu = newMenu(1021, "Props");
	MenuItemIndex item = addItem(menu, "p", 0);
	ByteCount size = 0;
	char buffer[8] = "";

	SetMenuItemProperty(menu, item, 'tst ', 'prop', 3, "abc");
	CHECK_EQ(GetMenuItemProperty(menu, item, 'tst ', 'prop', 2, &size, buffer), noErr);
	CHECK_EQ(size, 3);
	CHECK(buffer[0] == 'a' && buffer[1] == 'b');
	CFRelease((CFTypeRef) menu);
}

static void test_root_menu(void)
{
	MenuRef file = newMenu(3001, "File"), edit = newMenu(3002, "Edit");

	ClearMenuBar();
	InsertMenu(file, 0);
	InsertMenu(edit, 0);

	MenuRef root = AcquireRootMenu();

	CHECK(root != NULL);
	CHECK_EQ(CountMenuItems(root), 2);
	MenuRef sub = NULL;
	CFStringRef title = NULL;
	CHECK_EQ(GetMenuItemHierarchicalMenu(root, 1, &sub), noErr);
	CHECK(sub == file);
	CHECK_EQ(CopyMenuItemTextAsCFString(root, 2, &title), noErr);
	CHECK(title && CFEqual(title, CFSTR("Edit")));
	if (title)
		CFRelease(title);
	CFRelease((CFTypeRef) root);

	// A menu inserted before another one goes in front of it
	MenuRef view = newMenu(3003, "View");
	InsertMenu(view, 3002);
	root = AcquireRootMenu();
	CHECK_EQ(CountMenuItems(root), 3);
	CHECK_EQ(GetMenuItemHierarchicalMenu(root, 2, &sub), noErr);
	CHECK(sub == view);
	CFRelease((CFTypeRef) root);

	// A menu inserted as a hierarchical menu is found by ID but not in the menu bar
	MenuRef hier = newMenu(3004, "Hier");
	InsertMenu(hier, kInsertHierarchicalMenu);
	CHECK(GetMenuHandle(3004) == hier);
	root = AcquireRootMenu();
	CHECK_EQ(CountMenuItems(root), 3);
	CFRelease((CFTypeRef) root);

	// SetRootMenu makes the submenus of the items of a menu the menu bar
	MenuRef bar = newMenu(0, "");
	MenuItemIndex item = addItem(bar, "Only", 0);
	SetMenuItemHierarchicalMenu(bar, item, edit);
	CHECK_EQ(SetRootMenu(bar), noErr);
	root = AcquireRootMenu();
	CHECK_EQ(CountMenuItems(root), 1);
	CFRelease((CFTypeRef) root);
	CHECK(GetMenuHandle(3001) == NULL);
	CHECK(GetMenuHandle(3002) == edit);

	ClearMenuBar();
	CHECK(GetMenuHandle(3002) == NULL);
	DeleteMenu(3004);
	CFRelease((CFTypeRef) file);
	CFRelease((CFTypeRef) edit);
	CFRelease((CFTypeRef) view);
	CFRelease((CFTypeRef) hier);
	CFRelease((CFTypeRef) bar);
}

static void test_menu_bar_state(void)
{
	// The height of the menu bar of 10.14, in points
	CHECK_EQ(GetMBarHeight(), 22);
	CHECK(IsMenuBarVisible());
	HideMenuBar();
	CHECK(!IsMenuBarVisible());
	CHECK_EQ(GetMBarHeight(), 0);
	ShowMenuBar();
	CHECK(IsMenuBarVisible());
	SystemUIOptions options = 99;
	CHECK_EQ(GetSystemUIMode(&options), kUIModeNormal);
	CHECK_EQ(SetSystemUIMode(kUIModeAllHidden, 0), noErr);
	CHECK(!IsMenuBarVisible());
	CHECK_EQ(GetSystemUIMode(&options), kUIModeAllHidden);
	CHECK_EQ(SetSystemUIMode(kUIModeNormal, 0), noErr);
	CHECK(IsMenuBarVisible());
	CHECK_EQ(SetSystemUIMode(77, 0), paramErr);
}

// ---------------------------------------------------------------------------------------
// Commands and events
// ---------------------------------------------------------------------------------------

static int g_processed;
static HICommand g_lastCommand;
static int g_updates;
static MenuCommand g_enableOnUpdate;
static int g_menuEnableEvents;
static Boolean g_enableForKey;

static OSStatus commandHandler(EventHandlerCallRef call, EventRef event, void* userData)
{
	HICommand command;

	if (GetEventParameter(event, kEventParamDirectObject, typeHICommand, NULL, sizeof(command), NULL, &command) != noErr)
		return eventNotHandledErr;
	if (GetEventKind(event) == kEventCommandProcess)
	{
		if (command.commandID != 'hand')
			return eventNotHandledErr;
		g_processed++;
		g_lastCommand = command;
		return noErr;
	}
	if (GetEventKind(event) == kEventCommandUpdateStatus)
	{
		g_updates++;
		if (command.commandID == g_enableOnUpdate && command.menu.menuRef)
		{
			EnableMenuItem(command.menu.menuRef, command.menu.menuItemIndex);
			return noErr;
		}
	}
	return eventNotHandledErr;
}

static OSStatus menuHandler(EventHandlerCallRef call, EventRef event, void* userData)
{
	if (GetEventKind(event) == kEventMenuEnableItems)
	{
		g_menuEnableEvents++;
		g_enableForKey = false;
		GetEventParameter(event, kEventParamEnableMenuForKeyEvent, typeBoolean, NULL, sizeof(g_enableForKey), NULL, &g_enableForKey);
		MenuRef menu = NULL;
		GetEventParameter(event, kEventParamDirectObject, typeMenuRef, NULL, sizeof(menu), NULL, &menu);
		if (!menu)
			return eventNotHandledErr;
		// The handler handles the update: the command events are not sent
		return noErr;
	}
	return eventNotHandledErr;
}

static EventRef keyEvent(UInt32 kind, UInt32 keyCode, UInt32 modifiers, char character)
{
	EventRef event = NULL;
	UInt8 c = (UInt8) character;

	CreateEvent(NULL, kEventClassKeyboard, kind, 0, kEventAttributeNone, &event);
	SetEventParameter(event, kEventParamKeyCode, typeUInt32, sizeof(keyCode), &keyCode);
	SetEventParameter(event, kEventParamKeyModifiers, typeUInt32, sizeof(modifiers), &modifiers);
	SetEventParameter(event, kEventParamKeyMacCharCodes, typeChar, sizeof(c), &c);
	return event;
}

static void test_process_command(void)
{
	EventTypeSpec types[2] = { { kEventClassCommand, kEventCommandProcess }, { kEventClassCommand, kEventCommandUpdateStatus } };
	EventHandlerRef handler;

	CHECK_EQ(InstallEventHandler(GetApplicationEventTarget(), commandHandler, 2, types, NULL, &handler), noErr);

	MenuRef menu = newMenu(4001, "Cmd");
	MenuItemIndex item = addItem(menu, "Handled", 'hand');
	HICommand command = { kHICommandFromMenu, 'hand', { menu, item } };

	g_processed = 0;
	CHECK_EQ(ProcessHICommand(&command), noErr);
	CHECK_EQ(g_processed, 1);
	CHECK_EQ(g_lastCommand.commandID, 'hand');
	CHECK_EQ(g_lastCommand.attributes, kHICommandFromMenu);
	CHECK(g_lastCommand.menu.menuRef == menu);
	CHECK_EQ(g_lastCommand.menu.menuItemIndex, item);

	command.commandID = 'nobd';
	CHECK_EQ(ProcessHICommand(&command), eventNotHandledErr);
	CHECK_EQ(g_processed, 1);

	RemoveEventHandler(handler);
	CFRelease((CFTypeRef) menu);
}

static void test_key_equivalents(void)
{
	EventTypeSpec types[2] = { { kEventClassCommand, kEventCommandProcess }, { kEventClassCommand, kEventCommandUpdateStatus } };
	EventHandlerRef handler;

	CHECK_EQ(InstallEventHandler(GetApplicationEventTarget(), commandHandler, 2, types, NULL, &handler), noErr);

	MenuRef file = newMenu(4101, "File"), edit = newMenu(4102, "Edit");
	MenuItemIndex save = addItem(file, "Save", 'save');
	MenuItemIndex saveAs = addItem(file, "Save As", 'svas');
	MenuItemIndex disabled = addItem(file, "Disabled", 'dsbl');
	MenuItemIndex plain = addItem(edit, "Plain F1", 'f1  ');
	MenuItemIndex ctrl = addItem(edit, "Ctrl-Opt-K", 'ctrk');

	SetMenuItemCommandKey(file, save, false, 's');
	SetMenuItemCommandKey(file, saveAs, false, 'S');
	SetMenuItemModifiers(file, saveAs, kMenuShiftModifier);
	SetMenuItemCommandKey(file, disabled, false, 'd');
	DisableMenuItem(file, disabled);
	SetMenuItemCommandKey(edit, plain, true, 0x7A);   // the virtual key of F1
	SetMenuItemModifiers(edit, plain, kMenuNoCommandModifier);
	SetMenuItemCommandKey(edit, ctrl, false, 'k');
	SetMenuItemModifiers(edit, ctrl, kMenuControlModifier | kMenuOptionModifier | kMenuNoCommandModifier);
	InsertMenu(file, 0);
	InsertMenu(edit, 0);

	MenuRef menu = NULL;
	MenuItemIndex index = 0;
	EventRef event;

	// Command-S
	event = keyEvent(kEventRawKeyDown, 0x01, cmdKey, 's');
	CHECK(IsMenuKeyEvent(NULL, event, 0, &menu, &index));
	CHECK(menu == file && index == save);
	ReleaseEvent(event);
	// The key of the event can come as a capital letter
	event = keyEvent(kEventRawKeyDown, 0x01, cmdKey, 'S');
	CHECK(IsMenuKeyEvent(NULL, event, 0, &menu, &index));
	CHECK(menu == file && index == save);
	ReleaseEvent(event);
	// Command-Shift-S is the other item; without Command nothing matches
	event = keyEvent(kEventRawKeyDown, 0x01, cmdKey | shiftKey, 'S');
	CHECK(IsMenuKeyEvent(NULL, event, 0, &menu, &index));
	CHECK(menu == file && index == saveAs);
	ReleaseEvent(event);
	event = keyEvent(kEventRawKeyDown, 0x01, 0, 's');
	CHECK(!IsMenuKeyEvent(NULL, event, 0, &menu, &index));
	ReleaseEvent(event);
	// Key up and other classes are not key equivalents
	event = keyEvent(kEventRawKeyUp, 0x01, cmdKey, 's');
	CHECK(!IsMenuKeyEvent(NULL, event, 0, &menu, &index));
	ReleaseEvent(event);
	CHECK(!IsMenuKeyEvent(NULL, NULL, 0, &menu, &index));
	// A disabled item matches only when disabled items are asked for
	event = keyEvent(kEventRawKeyDown, 0x02, cmdKey, 'd');
	CHECK(!IsMenuKeyEvent(NULL, event, 0, &menu, &index));
	CHECK(IsMenuKeyEvent(NULL, event, kMenuEventIncludeDisabledItems, &menu, &index));
	CHECK(menu == file && index == disabled);
	ReleaseEvent(event);
	// A virtual key without Command, and Control-Option
	event = keyEvent(kEventRawKeyDown, 0x7A, 0, 0);
	CHECK(IsMenuKeyEvent(NULL, event, 0, &menu, &index));
	CHECK(menu == edit && index == plain);
	ReleaseEvent(event);
	event = keyEvent(kEventRawKeyDown, 0x28, controlKey | optionKey, 'k');
	CHECK(IsMenuKeyEvent(NULL, event, 0, &menu, &index));
	CHECK(menu == edit && index == ctrl);
	ReleaseEvent(event);
	event = keyEvent(kEventRawKeyDown, 0x28, controlKey, 'k');
	CHECK(!IsMenuKeyEvent(NULL, event, 0, &menu, &index));
	ReleaseEvent(event);
	// Without kMenuNoCommandModifier the item needs Command as well
	event = keyEvent(kEventRawKeyDown, 0x28, cmdKey | controlKey | optionKey, 'k');
	CHECK(!IsMenuKeyEvent(NULL, event, 0, &menu, &index));
	ReleaseEvent(event);
	// Starting at a menu searches that menu only
	event = keyEvent(kEventRawKeyDown, 0x01, cmdKey, 's');
	CHECK(IsMenuKeyEvent(file, event, 0, &menu, &index));
	CHECK(!IsMenuKeyEvent(edit, event, 0, &menu, &index));
	ReleaseEvent(event);

	// The items are brought up to date before the match: an item that its command's handler enables
	// matches, one that nothing enables does not
	DisableMenuItem(file, save);
	g_enableOnUpdate = 'save';
	g_updates = 0;
	event = keyEvent(kEventRawKeyDown, 0x01, cmdKey, 's');
	CHECK(IsMenuKeyEvent(NULL, event, 0, &menu, &index));
	CHECK(menu == file && index == save);
	CHECK(g_updates > 0);
	g_enableOnUpdate = 0;
	DisableMenuItem(file, save);
	CHECK(!IsMenuKeyEvent(NULL, event, 0, &menu, &index));
	ReleaseEvent(event);
	EnableMenuItem(file, save);

	// A handler of the menu's own enable-items event does the update instead of the command events,
	// and is told that it is for a key event
	EventTypeSpec menuTypes[1] = { { kEventClassMenu, kEventMenuEnableItems } };
	EventHandlerRef menuRef;

	CHECK_EQ(InstallEventHandler(GetMenuEventTarget(file), menuHandler, 1, menuTypes, NULL, &menuRef), noErr);
	g_updates = 0;
	g_menuEnableEvents = 0;
	event = keyEvent(kEventRawKeyDown, 0x01, cmdKey, 's');
	CHECK(IsMenuKeyEvent(NULL, event, 0, &menu, &index));
	CHECK(g_menuEnableEvents >= 1);
	CHECK(g_enableForKey);
	CHECK_EQ(g_updates, 0);
	ReleaseEvent(event);
	RemoveEventHandler(menuRef);

	// MenuEvent takes an EventRecord (the character in the message, the modifiers of the record)
	EventRecord record;

	memset(&record, 0, sizeof(record));
	record.what = keyDown;
	record.message = 's';
	record.modifiers = cmdKey;
	CHECK_EQ(MenuEvent(&record), ((UInt32) 4101 << 16) | save);
	// the character is the one without Shift (see test_quirk_menu_event_character_case)
	record.modifiers = cmdKey | shiftKey;
	record.message = 's';
	CHECK_EQ(MenuEvent(&record), ((UInt32) 4101 << 16) | saveAs);
	record.what = keyUp;
	CHECK_EQ(MenuEvent(&record), 0);
	record.what = keyDown;
	record.modifiers = 0;
	CHECK_EQ(MenuEvent(&record), 0);
	CHECK_EQ(MenuEvent(NULL), 0);

	RemoveEventHandler(handler);
	ClearMenuBar();
	CFRelease((CFTypeRef) file);
	CFRelease((CFTypeRef) edit);
}

static void test_quirk_menu_event_character_case(void)
{
	// QUIRK pinned: MenuEvent compares the character of the record with the lower case of the key of
	// the item. Items: s, T with Shift, u with Option, X. (The observed results of macOS 10.14.4 and 26.7.3.)
	MenuRef menu = newMenu(4301, "Case");
	EventRecord record;

	ClearMenuBar();
	SetMenuItemCommandKey(menu, addItem(menu, "s", 'cs01'), false, 's');
	MenuItemIndex t = addItem(menu, "T", 'cs02');
	SetMenuItemCommandKey(menu, t, false, 'T');
	SetMenuItemModifiers(menu, t, kMenuShiftModifier);
	MenuItemIndex u = addItem(menu, "u", 'cs03');
	SetMenuItemCommandKey(menu, u, false, 'u');
	SetMenuItemModifiers(menu, u, kMenuOptionModifier);
	SetMenuItemCommandKey(menu, addItem(menu, "X", 'cs04'), false, 'X');
	InsertMenu(menu, 0);

	memset(&record, 0, sizeof(record));
	record.what = keyDown;
	record.message = 's';
	record.modifiers = cmdKey;
	CHECK_EQ(MenuEvent(&record), ((UInt32) 4301 << 16) | 1);
	record.message = 'S';
	CHECK_EQ(MenuEvent(&record), 0);
	record.message = 't';
	record.modifiers = cmdKey | shiftKey;
	CHECK_EQ(MenuEvent(&record), ((UInt32) 4301 << 16) | t);
	record.message = 'T';
	CHECK_EQ(MenuEvent(&record), 0);
	record.message = 'T';
	record.modifiers = cmdKey;
	CHECK_EQ(MenuEvent(&record), 0);
	record.message = 'u';
	record.modifiers = cmdKey | optionKey;
	CHECK_EQ(MenuEvent(&record), ((UInt32) 4301 << 16) | u);
	record.message = 'x';
	record.modifiers = cmdKey;
	CHECK_EQ(MenuEvent(&record), ((UInt32) 4301 << 16) | 4);
	record.message = 'X';
	CHECK_EQ(MenuEvent(&record), 0);

	DeleteMenu(4301);
	CFRelease((CFTypeRef) menu);
}

static void test_quirk_process_null_command(void)
{
	// QUIRK pinned: no command is not handled, and not an error of the argument
	CHECK_EQ(ProcessHICommand(NULL), eventNotHandledErr);
}

static void test_menu_event_target(void)
{
	MenuRef menu = newMenu(4201, "Target");

	EventTargetRef target = GetMenuEventTarget(menu);
	CHECK(target != NULL);
	CHECK(GetMenuEventTarget(menu) == target);
	CHECK(GetMenuEventTarget(NULL) == NULL);

	// An event sent to the menu's target that its handlers do not handle goes on to the application
	EventTypeSpec types[1] = { { kEventClassCommand, kEventCommandProcess } };
	EventHandlerRef handler;

	CHECK_EQ(InstallEventHandler(GetApplicationEventTarget(), commandHandler, 1, types, NULL, &handler), noErr);
	EventRef event = NULL;
	HICommand command = { 0, 'hand', { NULL, 0 } };

	CreateEvent(NULL, kEventClassCommand, kEventCommandProcess, 0, kEventAttributeNone, &event);
	SetEventParameter(event, kEventParamDirectObject, typeHICommand, sizeof(command), &command);
	g_processed = 0;
	CHECK_EQ(SendEventToEventTarget(event, target), noErr);
	CHECK_EQ(g_processed, 1);
	ReleaseEvent(event);
	RemoveEventHandler(handler);
	CFRelease((CFTypeRef) menu);
}

static void test_ids_and_arguments(void)
{
	// Things that are not menus are refused with menuInvalidErr
	CHECK_EQ(SetMenuItemCommandID(NULL, 1, 1), menuInvalidErr);
	CHECK_EQ(CountMenuItems(NULL), 0);
	CHECK_EQ(GetMenuID(NULL), 0);
	CHECK(!IsMenuItemEnabled(NULL, 1));
	CHECK_EQ(AppendMenuItemTextWithCFString(NULL, CFSTR("x"), 0, 0, NULL), menuInvalidErr);
	DisableMenuItem(NULL, 1);
	InsertMenu(NULL, 0);
}

#if !__LP64__
static void pstr(Str255 out, const char* text)
{
	size_t length = strlen(text);

	out[0] = (unsigned char) length;
	memcpy(out + 1, text, length);
}

static void test_quirk_insert_menu_item_reverses_items(void)
{
	// QUIRK pinned: the items of the string end up in the reverse order after the item
	Str255 title, items, text;

	pstr(title, "Menu");
	MenuRef menu = NewMenu(5100, title);

	pstr(items, "One;Two");
	AppendMenu(menu, items);
	pstr(items, "A;B");
	InsertMenuItem(menu, items, 1);
	CHECK_EQ(CountMenuItems(menu), 4);
	GetMenuItemText(menu, 2, text);
	CHECK(text[0] == 1 && text[1] == 'B');
	GetMenuItemText(menu, 3, text);
	CHECK(text[0] == 1 && text[1] == 'A');
	GetMenuItemText(menu, 4, text);
	CHECK(text[0] == 3 && text[1] == 'T');
	DisposeMenu(menu);
}

static void test_classic_calls(void)
{
	test_quirk_insert_menu_item_reverses_items();

	Str255 title, text;
	MenuRef menu;

	pstr(title, "Classic");
	menu = NewMenu(5001, title);
	CHECK(menu != NULL);
	CHECK_EQ(GetMenuID(menu), 5001);
	GetMenuTitle(menu, text);
	CHECK(text[0] == 7 && memcmp(text + 1, "Classic", 7) == 0);
	pstr(title, "Renamed");
	SetMenuTitle(menu, title);
	GetMenuTitle(menu, text);
	CHECK(text[0] == 7 && memcmp(text + 1, "Renamed", 7) == 0);

	// Items from a string with meta characters: separators, disabled items, marks, styles, keys
	Str255 items;

	pstr(items, "Open/O;Close/W;(-;(Disabled;!\x12" "Marked;<BBold");
	AppendMenu(menu, items);
	CHECK_EQ(CountMenuItems(menu), 6);
	GetMenuItemText(menu, 1, text);
	CHECK(text[0] == 4 && memcmp(text + 1, "Open", 4) == 0);
	CHECK(IsMenuItemEnabled(menu, 1));
	CHECK(!IsMenuItemEnabled(menu, 3));
	MenuItemAttributes attributes = 0;
	GetMenuItemAttributes(menu, 3, &attributes);
	CHECK(attributes & kMenuItemAttrSeparator);
	CHECK(!IsMenuItemEnabled(menu, 4));
	GetMenuItemText(menu, 4, text);
	CHECK(text[0] == 8 && memcmp(text + 1, "Disabled", 8) == 0);
	CharParameter mark = 0;
	GetItemMark(menu, 5, &mark);
	CHECK_EQ(mark, 0x12);
	CharParameter key = 0;
	GetItemCmd(menu, 1, &key);
	CHECK_EQ(key, 'O');
	SetItemCmd(menu, 2, 'x');
	GetItemCmd(menu, 2, &key);
	CHECK_EQ(key, 'x');
	Style style = 0;
	GetItemStyle(menu, 6, &style);
	CHECK_EQ(style, 1);   // bold

	// Text of items in Pascal strings
	pstr(text, "Appended");
	CHECK_EQ(AppendMenuItemText(menu, text), noErr);
	CHECK_EQ(CountMenuItems(menu), 7);
	pstr(text, "Inserted;not;split");
	CHECK_EQ(InsertMenuItemText(menu, text, 0), noErr);
	CHECK_EQ(CountMenuItems(menu), 8);
	GetMenuItemText(menu, 1, text);
	CHECK(text[0] == 18);
	SetMenuItemText(menu, 1, title);
	GetMenuItemText(menu, 1, text);
	CHECK(text[0] == 7 && memcmp(text + 1, "Renamed", 7) == 0);
	pstr(items, "A;B");
	InsertMenuItem(menu, items, 1);
	CHECK_EQ(CountMenuItems(menu), 10);
	// QUIRK pinned (test_quirk_insert_menu_item_reverses_items): the items end up in reverse order
	GetMenuItemText(menu, 2, text);
	CHECK(text[0] == 1 && text[1] == 'B');
	GetMenuItemText(menu, 3, text);
	CHECK(text[0] == 1 && text[1] == 'A');

	// Counting and enabling by command, the retain count calls
	MenuItemIndex first = CountMenuItems(menu);
	SetMenuItemCommandID(menu, first, 'cmd1');
	SetMenuItemCommandID(menu, first - 1, 'cmd1');
	InsertMenu(menu, 0);
	CHECK_EQ(CountMenuItemsWithCommandID(NULL, 'cmd1'), 2);
	CHECK_EQ(CountMenuItemsWithCommandID(menu, 'cmd1'), 2);
	CHECK_EQ(CountMenuItemsWithCommandID(NULL, 'zzzz'), 0);
	DisableMenuCommand(NULL, 'cmd1');
	CHECK(!IsMenuCommandEnabled(NULL, 'cmd1'));
	EnableMenuCommand(NULL, 'cmd1');
	CHECK(IsMenuCommandEnabled(NULL, 'cmd1'));
	CHECK(!IsMenuCommandEnabled(NULL, 'zzzz'));
	CHECK(MenuHasEnabledItems(menu));

	ItemCount count = GetMenuRetainCount(menu);

	CHECK(count >= 2);
	CHECK_EQ(RetainMenu(menu), noErr);
	CHECK_EQ(GetMenuRetainCount(menu), count + 1);
	CHECK_EQ(ReleaseMenu(menu), noErr);
	CHECK_EQ(GetMenuRetainCount(menu), count);

	// MenuKey: the item with Command-O is chosen and the title is highlighted
	UInt32 choice = MenuKey('o');
	CHECK_EQ(choice, ((UInt32) 5001 << 16) | 4);
	HiliteMenu(0);
	CHECK_EQ(MenuKey('q'), 0);

	// The key glyph and the hierarchical ID calls
	CHECK_EQ(SetMenuItemKeyGlyph(menu, 1, kMenuF1Glyph), noErr);
	SInt16 glyph = 0;
	CHECK_EQ(GetMenuItemKeyGlyph(menu, 1, &glyph), noErr);
	CHECK_EQ(glyph, kMenuF1Glyph);
	MenuRef sub;
	pstr(title, "Sub");
	sub = NewMenu(5002, title);
	InsertMenu(sub, kInsertHierarchicalMenu);
	CHECK_EQ(SetMenuItemHierarchicalID(menu, 2, 5002), noErr);
	MenuID hierID = 0;
	CHECK_EQ(GetMenuItemHierarchicalID(menu, 2, &hierID), noErr);
	CHECK_EQ(hierID, 5002);
	MenuRef found = NULL;
	CHECK_EQ(GetMenuItemHierarchicalMenu(menu, 2, &found), noErr);
	CHECK(found == sub);

	// Menu bar handles
	Handle bar = GetMenuBar();
	CHECK(bar != NULL);
	ClearMenuBar();
	CHECK(GetMenuHandle(5001) == NULL);
	SetMenuBar(bar);
	CHECK(GetMenuHandle(5001) == menu);
	DisposeHandle(bar);

	// Resource menus that do not exist
	CHECK(GetMenu(31000) == NULL);
	CHECK(GetNewMBar(31000) == NULL);

	DeleteMenu(5001);
	DeleteMenu(5002);
	DisposeMenu(menu);
	DisposeMenu(sub);
	// the extra references of the handle and ReleaseMenu above
	ClearMenuBar();
}
#endif

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	test_menu_object();
	test_quirk_insert_menu_twice();
	test_items();
	test_quirk_separator_is_disabled();
	test_enable_and_item_zero();
	test_hierarchical_menus();
	test_command_lookup();
	test_quirk_command_lookup_order();
	test_properties();
	test_quirk_property_small_buffer();
	test_root_menu();
	test_menu_bar_state();
	test_process_command();
	test_key_equivalents();
	test_quirk_menu_event_character_case();
	test_quirk_process_null_command();
	test_menu_event_target();
	test_ids_and_arguments();
#if !__LP64__
	test_classic_calls();
#endif
	if (g_failures)
	{
		printf("FAIL: %d checks failed\n", g_failures);
		return 1;
	}
	printf("PASS carbon_menus (%s)\n", sizeof(void*) == 8 ? "x86_64" : "i386");
	return 0;
}
