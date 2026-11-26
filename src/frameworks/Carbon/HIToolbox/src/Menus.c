/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

// The Menu Manager: menu objects, items, the menu bar (root menu), the lookup of commands and key
// equivalents, and the command and menu events. How menus are shown and tracked is in
// MenuTracking.c.
//
// A menu is a Core Foundation object. It is retained by whoever created or retained it, by the
// menu bar list (InsertMenu) and by the item that has it as a submenu. The menu bar is a list of
// the inserted menus; the root menu (AcquireRootMenu) is a menu made from that list, whose items
// have the titles of the menus and the menus as their submenus.
//
// Behaviour of the lookup of commands, retain counts, the item 0 conventions and the errors for
// items out of range follows what the 10.14 Menu Manager does (black-box observation, tier 2;
// the probe results are recorded with the tests in tests/src/carbon_menus.c).

#include <HIToolbox/Menus.h>
#include <HIToolbox/CarbonEventsCore.h>
#include <HIToolbox/CarbonEvents.h>
#include <HIToolbox/Events.h>
#include <HIToolbox/TextInputSources.h>
#include <HIToolbox/MacApplication.h>
#include <CoreFoundation/CFRuntime.h>
#include <CarbonCore/MacErrors.h>
#include <CarbonCore/MacMemory.h>
#include <CarbonCore/Resources.h>
#include <CarbonCore/UnicodeUtilities.h>
#include <AE/AEDataModel.h>
#include <dispatch/dispatch.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "HIToolboxInternal.h"
#include "HIToolboxPrivate.h"
#include "MenusInternal.h"

// ---------------------------------------------------------------------------------------
// The objects
// ---------------------------------------------------------------------------------------

struct MenuItemProperty
{
	OSType creator;
	OSType tag;
	ByteCount size;
	void* data;
};

struct MenuItem
{
	CFStringRef title;
	MenuCommand command;
	MenuItemAttributes attributes;
	UInt8 modifiers;
	UInt16 commandKey;      // a character, or a virtual key code with kMenuItemAttrUseVirtualKey
	SInt16 glyph;
	UInt16 mark;
	MenuRef submenu;        // retained
	MenuID submenuID;       // the submenu named by ID (SetMenuItemHierarchicalID), 0 for none
	UInt32 refCon;
	UInt32 indent;
	SInt16 fontID;
	UInt8 style;
	struct MenuItemProperty* properties;
	ItemCount propertyCount;
};

struct OpaqueMenuRef
{
	CFRuntimeBase base;
	MenuID menuID;
	MenuAttributes attributes;
	CFStringRef title;
	Boolean enabled;
	struct MenuItem* items;
	ItemCount count;
	ItemCount capacity;
	SInt16 width;
	SInt16 height;
	EventTargetRef target;
	// The menu is part of the menu bar list
	bool inBar;
	// The standard Window menu (CreateStandardWindowMenu()): the window list is kept up to date when it opens
	bool isWindowMenu;
};

static pthread_mutex_t g_menuLock;
static pthread_once_t g_menuLockOnce = PTHREAD_ONCE_INIT;

static void initMenuLock(void)
{
	pthread_mutexattr_t attributes;

	pthread_mutexattr_init(&attributes);
	pthread_mutexattr_settype(&attributes, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&g_menuLock, &attributes);
	pthread_mutexattr_destroy(&attributes);
}

static void lockMenus(void)
{
	pthread_once(&g_menuLockOnce, initMenuLock);
	pthread_mutex_lock(&g_menuLock);
}

static void unlockMenus(void)
{
	pthread_mutex_unlock(&g_menuLock);
}

// The menus inserted with InsertMenu(), in the order of the calls. An entry holds a reference to
// the menu. The ones that are in the menu bar are in the order of the menu bar.
struct InsertedMenu
{
	MenuRef menu;
	bool inBar;
};
static struct InsertedMenu* g_inserted;
static ItemCount g_insertedCount;
static ItemCount g_insertedCapacity;

static MenuRef g_rootMenu;           // the root menu object, made by the first AcquireRootMenu()
static bool g_menuBarVisible = true;
static MenuID g_hilitedMenuID;

static CFTypeID g_menuTypeID;

static void freeItem(struct MenuItem* item)
{
	if (item->title)
		CFRelease(item->title);
	if (item->submenu)
		CFRelease(item->submenu);
	for (ItemCount i = 0; i < item->propertyCount; i++)
		free(item->properties[i].data);
	free(item->properties);
}

static void menuFinalize(CFTypeRef cf)
{
	MenuRef menu = (MenuRef) cf;

	for (ItemCount i = 0; i < menu->count; i++)
		freeItem(&menu->items[i]);
	free(menu->items);
	if (menu->title)
		CFRelease(menu->title);
	if (menu->target)
		_HIDestroyEventTarget(menu->target);
}

static CFStringRef menuCopyDescription(CFTypeRef cf)
{
	MenuRef menu = (MenuRef) cf;

	return CFStringCreateWithFormat(NULL, NULL, CFSTR("<Menu %p, ID %d, %lu items>"), cf, (int) menu->menuID, (unsigned long) menu->count);
}

static const CFRuntimeClass g_menuClass = {
	.version = 0,
	.className = "Menu",
	.finalize = menuFinalize,
	.copyDebugDesc = menuCopyDescription,
};

CFTypeID _HIMenuGetTypeID(void)
{
	static dispatch_once_t once;

	dispatch_once(&once, ^{
		g_menuTypeID = _CFRuntimeRegisterClass(&g_menuClass);
	});
	return g_menuTypeID;
}

static bool isMenu(MenuRef menu)
{
	return menu != NULL && CFGetTypeID((CFTypeRef) menu) == _HIMenuGetTypeID();
}

static bool isItem(MenuRef menu, MenuItemIndex item)
{
	return isMenu(menu) && item >= 1 && item <= menu->count;
}

// The status for a call whose menu or item is not valid: a menu that is none gives menuInvalidErr, an
// item out of range paramErr
static OSStatus itemFailure(MenuRef menu)
{
	return isMenu(menu) ? paramErr : menuInvalidErr;
}

static struct MenuItem* itemAt(MenuRef menu, MenuItemIndex item)
{
	return &menu->items[item - 1];
}

static MenuRef newMenu(MenuID menuID, MenuAttributes attributes)
{
	MenuRef menu = (MenuRef) _CFRuntimeCreateInstance(kCFAllocatorDefault, _HIMenuGetTypeID(), sizeof(struct OpaqueMenuRef) - sizeof(CFRuntimeBase), NULL);

	if (!menu)
		return NULL;
	memset((char*) menu + sizeof(CFRuntimeBase), 0, sizeof(struct OpaqueMenuRef) - sizeof(CFRuntimeBase));
	menu->menuID = menuID;
	menu->attributes = attributes;
	menu->enabled = true;
	return menu;
}

OSStatus CreateNewMenu(MenuID inMenuID, MenuAttributes inMenuAttributes, MenuRef* outMenuRef)
{
	if (!outMenuRef)
		return paramErr;

	*outMenuRef = newMenu(inMenuID, inMenuAttributes);
	return *outMenuRef ? noErr : memFullErr;
}

SInt16 GetMenuID(MenuRef menu)
{
	return isMenu(menu) ? menu->menuID : 0;
}

UInt16 CountMenuItems(MenuRef theMenu)
{
	return isMenu(theMenu) ? (UInt16) theMenu->count : 0;
}

OSStatus ChangeMenuAttributes(MenuRef menu, MenuAttributes setTheseAttributes, MenuAttributes clearTheseAttributes)
{
	if (!isMenu(menu))
		return paramErr;
	lockMenus();
	menu->attributes = (menu->attributes | setTheseAttributes) & ~clearTheseAttributes;
	unlockMenus();
	return noErr;
}

OSStatus GetMenuAttributes(MenuRef menu, MenuAttributes* outAttributes)
{
	if (!isMenu(menu) || !outAttributes)
		return paramErr;
	*outAttributes = menu->attributes;
	return noErr;
}

OSStatus SetMenuTitleWithCFString(MenuRef inMenu, CFStringRef inString)
{
	if (!isMenu(inMenu))
		return paramErr;

	CFStringRef copy = inString ? CFStringCreateCopy(NULL, inString) : NULL;

	lockMenus();
	if (inMenu->title)
		CFRelease(inMenu->title);
	inMenu->title = copy;
	unlockMenus();
	return noErr;
}

OSStatus CopyMenuTitleAsCFString(MenuRef inMenu, CFStringRef* outString)
{
	if (!isMenu(inMenu) || !outString)
		return paramErr;
	lockMenus();
	*outString = inMenu->title ? CFRetain(inMenu->title) : CFStringCreateWithCString(NULL, "", kCFStringEncodingUTF8);
	unlockMenus();
	return noErr;
}

OSStatus SetMenuTitleIcon(MenuRef inMenu, UInt32 inType, void* inIcon)
{
	// Icons are not drawn; the title text stays
	return isMenu(inMenu) ? noErr : paramErr;
}

UInt16 GetMenuWidth(MenuRef menu)
{
	return isMenu(menu) ? (UInt16) menu->width : 0;
}

UInt16 GetMenuHeight(MenuRef menu)
{
	return isMenu(menu) ? (UInt16) menu->height : 0;
}

UInt16 GetMenuType(MenuRef theMenu)
{
	if (!isMenu(theMenu))
		return kMenuTypeInvalid;
	lockMenus();
	UInt16 type = kMenuTypePullDown;
	if (theMenu == g_rootMenu)
		type = kMenuTypeBar;
	else
	{
		// A menu that is the submenu of an item of a menu is hierarchical
		for (ItemCount i = 0; i < g_insertedCount && type != kMenuTypeHierarchical; i++)
		{
			if (g_inserted[i].menu == theMenu && !g_inserted[i].inBar)
				type = kMenuTypeHierarchical;
		}
	}
	unlockMenus();
	return type;
}

void CalcMenuSize(MenuRef theMenu)
{
	// The size is worked out when the menu is shown
}

// ---------------------------------------------------------------------------------------
// The menu bar
// ---------------------------------------------------------------------------------------

static void growInserted(void)
{
	if (g_insertedCount < g_insertedCapacity)
		return;
	ItemCount capacity = g_insertedCapacity ? g_insertedCapacity * 2 : 16;
	struct InsertedMenu* list = realloc(g_inserted, capacity * sizeof(*list));

	if (!list)
		abort();
	g_inserted = list;
	g_insertedCapacity = capacity;
}

static MenuRef findInserted(MenuID menuID)
{
	for (ItemCount i = 0; i < g_insertedCount; i++)
	{
		if (g_inserted[i].menu->menuID == menuID)
			return g_inserted[i].menu;
	}
	return NULL;
}

MenuRef GetMenuHandle(MenuID menuID)
{
	lockMenus();
	MenuRef menu = findInserted(menuID);
	unlockMenus();
	return menu;
}

void InsertMenu(MenuRef theMenu, MenuID beforeID)
{
	if (!isMenu(theMenu))
		return;

	lockMenus();

	// QUIRK: inserting a menu that is in the list already changes nothing: it is not held a second time
	// and keeps its place. Not documented. Evidence: black-box observation on macOS 10.14.4 (i386 and
	// x86_64) and 26.7.3 (tier 2): the retain count after a second InsertMenu() of the same menu
	// is the one after the first.
	// Test: test_quirk_insert_menu_twice in tests/src/carbon_menus.c
	for (ItemCount i = 0; i < g_insertedCount; i++)
	{
		if (g_inserted[i].menu == theMenu)
		{
			unlockMenus();
			return;
		}
	}
	growInserted();

	ItemCount position = g_insertedCount;
	bool inBar = beforeID != kInsertHierarchicalMenu;

	if (inBar && beforeID != 0)
	{
		for (ItemCount i = 0; i < g_insertedCount; i++)
		{
			if (g_inserted[i].inBar && g_inserted[i].menu->menuID == beforeID)
			{
				position = i;
				break;
			}
		}
	}
	memmove(&g_inserted[position + 1], &g_inserted[position], (g_insertedCount - position) * sizeof(*g_inserted));
	g_inserted[position].menu = (MenuRef) CFRetain((CFTypeRef) theMenu);
	g_inserted[position].inBar = inBar;
	g_insertedCount++;
	if (inBar)
		theMenu->inBar = true;
	unlockMenus();
	_HIMenuBarChanged();
}

static bool stillInBar(MenuRef menu)
{
	for (ItemCount i = 0; i < g_insertedCount; i++)
	{
		if (g_inserted[i].menu == menu && g_inserted[i].inBar)
			return true;
	}
	return false;
}

void DeleteMenu(MenuID menuID)
{
	MenuRef released[32];
	ItemCount releasedCount = 0;

	lockMenus();
	for (ItemCount i = 0; i < g_insertedCount;)
	{
		if (g_inserted[i].menu->menuID != menuID || releasedCount == 32)
		{
			i++;
			continue;
		}
		released[releasedCount++] = g_inserted[i].menu;
		memmove(&g_inserted[i], &g_inserted[i + 1], (g_insertedCount - i - 1) * sizeof(*g_inserted));
		g_insertedCount--;
	}
	for (ItemCount i = 0; i < releasedCount; i++)
		released[i]->inBar = stillInBar(released[i]);
	unlockMenus();

	for (ItemCount i = 0; i < releasedCount; i++)
		CFRelease((CFTypeRef) released[i]);
	if (releasedCount)
		_HIMenuBarChanged();
}

void ClearMenuBar(void)
{
	MenuRef released[64];
	ItemCount releasedCount = 0;

	lockMenus();
	for (ItemCount i = 0; i < g_insertedCount;)
	{
		if (!g_inserted[i].inBar || releasedCount == 64)
		{
			i++;
			continue;
		}
		released[releasedCount++] = g_inserted[i].menu;
		memmove(&g_inserted[i], &g_inserted[i + 1], (g_insertedCount - i - 1) * sizeof(*g_inserted));
		g_insertedCount--;
	}
	for (ItemCount i = 0; i < releasedCount; i++)
		released[i]->inBar = stillInBar(released[i]);
	unlockMenus();

	for (ItemCount i = 0; i < releasedCount; i++)
		CFRelease((CFTypeRef) released[i]);
	if (releasedCount)
		_HIMenuBarChanged();
}

// The menu bar menus, retained. The caller releases them and frees the array.
ItemCount _HICopyMenuBarMenus(MenuRef** outMenus)
{
	lockMenus();
	MenuRef* menus = malloc((g_insertedCount + 1) * sizeof(MenuRef));
	ItemCount count = 0;

	for (ItemCount i = 0; menus && i < g_insertedCount; i++)
	{
		if (g_inserted[i].inBar)
			menus[count++] = (MenuRef) CFRetain((CFTypeRef) g_inserted[i].menu);
	}
	unlockMenus();
	*outMenus = menus;
	return count;
}

static void rebuildRootMenu(void)
{
	if (!g_rootMenu)
		return;
	for (ItemCount i = 0; i < g_rootMenu->count; i++)
		freeItem(&g_rootMenu->items[i]);
	g_rootMenu->count = 0;
	for (ItemCount i = 0; i < g_insertedCount; i++)
	{
		if (!g_inserted[i].inBar)
			continue;
		MenuRef menu = g_inserted[i].menu;

		if (g_rootMenu->count == g_rootMenu->capacity)
		{
			ItemCount capacity = g_rootMenu->capacity ? g_rootMenu->capacity * 2 : 16;
			struct MenuItem* items = realloc(g_rootMenu->items, capacity * sizeof(*items));

			if (!items)
				abort();
			g_rootMenu->items = items;
			g_rootMenu->capacity = capacity;
		}
		struct MenuItem* item = &g_rootMenu->items[g_rootMenu->count++];

		memset(item, 0, sizeof(*item));
		item->title = menu->title ? CFRetain(menu->title) : NULL;
		item->submenu = (MenuRef) CFRetain((CFTypeRef) menu);
		item->attributes = menu->enabled ? 0 : kMenuItemAttrDisabled;
	}
}

MenuRef AcquireRootMenu(void)
{
	lockMenus();
	if (!g_rootMenu)
		g_rootMenu = newMenu(0, 0);
	rebuildRootMenu();
	MenuRef root = (MenuRef) CFRetain((CFTypeRef) g_rootMenu);
	unlockMenus();
	return root;
}

OSStatus SetRootMenu(MenuRef inMenu)
{
	if (!isMenu(inMenu))
		return paramErr;

	// The menu bar becomes the submenus of the items of the menu
	ClearMenuBar();
	lockMenus();
	ItemCount count = inMenu->count;
	MenuRef* menus = malloc((count + 1) * sizeof(MenuRef));
	ItemCount used = 0;

	for (ItemCount i = 0; menus && i < count; i++)
	{
		if (inMenu->items[i].submenu)
			menus[used++] = (MenuRef) CFRetain((CFTypeRef) inMenu->items[i].submenu);
	}
	unlockMenus();
	for (ItemCount i = 0; i < used; i++)
	{
		InsertMenu(menus[i], 0);
		CFRelease((CFTypeRef) menus[i]);
	}
	free(menus);
	return noErr;
}

SInt16 GetMBarHeight(void)
{
	// The height of the menu bar of 10.14, in points
	return g_menuBarVisible ? 22 : 0;
}

void HideMenuBar(void)
{
	g_menuBarVisible = false;
	_HIRedrawMenuBar();
}

void ShowMenuBar(void)
{
	g_menuBarVisible = true;
	_HIRedrawMenuBar();
}

Boolean IsMenuBarVisible(void)
{
	return g_menuBarVisible;
}

static SystemUIMode g_systemUIMode = kUIModeNormal;
static SystemUIOptions g_systemUIOptions;

OSStatus SetSystemUIMode(SystemUIMode inMode, SystemUIOptions inOptions)
{
	if (inMode > kUIModeAllSuppressed)
		return paramErr;

	g_systemUIMode = inMode;
	g_systemUIOptions = inOptions;
	// The modes that hide or suppress the menu bar take it off the screen; the options that let
	// the pointer bring it back are not acted on
	if (inMode == kUIModeNormal || inMode == kUIModeContentSuppressed)
		ShowMenuBar();
	else
		HideMenuBar();
	return noErr;
}

SystemUIMode GetSystemUIMode(SystemUIOptions* outOptions)
{
	if (outOptions)
		*outOptions = g_systemUIOptions;
	return g_systemUIMode;
}

void FlashMenuBar(MenuID menuID)
{
	// The title is highlighted for the moment of the flash, which no one can see between two calls
	_HIRedrawMenuBar();
}

void HiliteMenu(MenuID menuID)
{
	g_hilitedMenuID = menuID;
	_HIRedrawMenuBar();
}

MenuID _HIHilitedMenuID(void)
{
	return g_hilitedMenuID;
}

// ---------------------------------------------------------------------------------------
// Items
// ---------------------------------------------------------------------------------------

static OSStatus insertItem(MenuRef menu, ItemCount position, CFStringRef title, MenuItemAttributes attributes, MenuCommand command, MenuItemIndex* outNewItem)
{
	if (!isMenu(menu))
		return menuInvalidErr;

	CFStringRef copy = title ? CFStringCreateCopy(NULL, title) : CFStringCreateWithCString(NULL, "", kCFStringEncodingUTF8);

	lockMenus();
	if (menu->count == menu->capacity)
	{
		ItemCount capacity = menu->capacity ? menu->capacity * 2 : 8;
		struct MenuItem* items = realloc(menu->items, capacity * sizeof(*items));

		if (!items)
		{
			unlockMenus();
			CFRelease(copy);
			return memFullErr;
		}
		menu->items = items;
		menu->capacity = capacity;
	}
	if (position > menu->count)
		position = menu->count;
	memmove(&menu->items[position + 1], &menu->items[position], (menu->count - position) * sizeof(struct MenuItem));
	struct MenuItem* item = &menu->items[position];

	memset(item, 0, sizeof(*item));
	item->title = copy;
	item->attributes = attributes;
	item->command = command;
	if (attributes & kMenuItemAttrSeparator)
	{
		// A separator is a disabled item with the text "-"
		item->attributes |= kMenuItemAttrDisabled;
		CFRelease(item->title);
		item->title = CFSTR("-");
		CFRetain(item->title);
	}
	menu->count++;
	if (outNewItem)
		*outNewItem = (MenuItemIndex) (position + 1);
	unlockMenus();
	_HIMenuChanged(menu);
	return noErr;
}

OSStatus AppendMenuItemTextWithCFString(MenuRef inMenu, CFStringRef inString, MenuItemAttributes inAttributes, MenuCommand inCommandID, MenuItemIndex* outNewItem)
{
	if (!isMenu(inMenu))
		return menuInvalidErr;
	return insertItem(inMenu, inMenu->count, inString, inAttributes, inCommandID, outNewItem);
}

OSStatus InsertMenuItemTextWithCFString(MenuRef inMenu, CFStringRef inString, MenuItemIndex inAfterItem, MenuItemAttributes inAttributes, MenuCommand inCommandID)
{
	// An item after one that does not exist goes to the end
	return insertItem(inMenu, inAfterItem, inString, inAttributes, inCommandID, NULL);
}

OSStatus SetMenuItemTextWithCFString(MenuRef inMenu, MenuItemIndex inItem, CFStringRef inString)
{
	if (!isItem(inMenu, inItem) || !inString)
		return itemFailure(inMenu);

	CFStringRef copy = CFStringCreateCopy(NULL, inString);

	lockMenus();
	struct MenuItem* item = itemAt(inMenu, inItem);

	CFRelease(item->title);
	item->title = copy;
	unlockMenus();
	_HIMenuChanged(inMenu);
	return noErr;
}

OSStatus CopyMenuItemTextAsCFString(MenuRef inMenu, MenuItemIndex inItem, CFStringRef* outString)
{
	if (!outString)
		return paramErr;
	if (!isItem(inMenu, inItem))
		return itemFailure(inMenu);
	lockMenus();
	*outString = CFRetain(itemAt(inMenu, inItem)->title);
	unlockMenus();
	return noErr;
}

OSStatus DeleteMenuItems(MenuRef inMenu, MenuItemIndex inFirstItem, ItemCount inNumItems)
{
	if (!isMenu(inMenu))
		return paramErr;

	lockMenus();
	// Items that do not exist are not an error
	if (inFirstItem >= 1 && inFirstItem <= inMenu->count)
	{
		ItemCount available = inMenu->count - (inFirstItem - 1);
		ItemCount number = inNumItems < available ? inNumItems : available;

		for (ItemCount i = 0; i < number; i++)
			freeItem(&inMenu->items[inFirstItem - 1 + i]);
		memmove(&inMenu->items[inFirstItem - 1], &inMenu->items[inFirstItem - 1 + number], (available - number) * sizeof(struct MenuItem));
		inMenu->count -= number;
	}
	unlockMenus();
	_HIMenuChanged(inMenu);
	return noErr;
}

void DeleteMenuItem(MenuRef theMenu, MenuItemIndex item)
{
	DeleteMenuItems(theMenu, item, 1);
}

static void setEnabled(MenuRef menu, MenuItemIndex item, bool enabled)
{
	if (!isMenu(menu))
		return;
	lockMenus();
	if (item == 0)
		menu->enabled = enabled;
	else if (item <= menu->count)
	{
		struct MenuItem* entry = itemAt(menu, item);

		if (enabled)
			entry->attributes &= ~kMenuItemAttrDisabled;
		else
			entry->attributes |= kMenuItemAttrDisabled;
	}
	unlockMenus();
	_HIMenuChanged(menu);
}

void EnableMenuItem(MenuRef theMenu, MenuItemIndex item)
{
	setEnabled(theMenu, item, true);
}

void DisableMenuItem(MenuRef theMenu, MenuItemIndex item)
{
	setEnabled(theMenu, item, false);
}

Boolean IsMenuItemEnabled(MenuRef menu, MenuItemIndex item)
{
	if (!isMenu(menu))
		return false;
	if (item == 0)
		return menu->enabled;
	if (item > menu->count)
		return false;
	return (itemAt(menu, item)->attributes & kMenuItemAttrDisabled) == 0;
}

OSStatus SetMenuItemCommandID(MenuRef inMenu, MenuItemIndex inItem, MenuCommand inCommandID)
{
	if (!isItem(inMenu, inItem))
		return itemFailure(inMenu);
	itemAt(inMenu, inItem)->command = inCommandID;
	return noErr;
}

OSStatus GetMenuItemCommandID(MenuRef inMenu, MenuItemIndex inItem, MenuCommand* outCommandID)
{
	if (!isItem(inMenu, inItem) || !outCommandID)
		return itemFailure(inMenu);
	*outCommandID = itemAt(inMenu, inItem)->command;
	return noErr;
}

static MenuRef submenuOf(const struct MenuItem* item)
{
	if (item->submenu)
		return item->submenu;
	return item->submenuID ? findInserted(item->submenuID) : NULL;
}

OSStatus SetMenuItemHierarchicalMenu(MenuRef inMenu, MenuItemIndex inItem, MenuRef inHierMenu)
{
	if (!isItem(inMenu, inItem) || (inHierMenu && !isMenu(inHierMenu)))
		return itemFailure(inMenu);

	lockMenus();
	struct MenuItem* item = itemAt(inMenu, inItem);
	MenuRef previous = item->submenu;

	item->submenu = inHierMenu ? (MenuRef) CFRetain((CFTypeRef) inHierMenu) : NULL;
	item->submenuID = 0;
	unlockMenus();
	if (previous)
		CFRelease((CFTypeRef) previous);
	_HIMenuChanged(inMenu);
	return noErr;
}

OSStatus GetMenuItemHierarchicalMenu(MenuRef inMenu, MenuItemIndex inItem, MenuRef* outHierMenu)
{
	if (!isItem(inMenu, inItem) || !outHierMenu)
		return itemFailure(inMenu);
	lockMenus();
	*outHierMenu = submenuOf(itemAt(inMenu, inItem));
	unlockMenus();
	return noErr;
}

OSStatus SetMenuItemKeyGlyph(MenuRef inMenu, MenuItemIndex inItem, SInt16 inGlyph)
{
	if (!isItem(inMenu, inItem))
		return itemFailure(inMenu);
	itemAt(inMenu, inItem)->glyph = inGlyph;
	_HIMenuChanged(inMenu);
	return noErr;
}

OSStatus SetMenuItemModifiers(MenuRef inMenu, MenuItemIndex inItem, UInt8 inModifiers)
{
	if (!isItem(inMenu, inItem))
		return itemFailure(inMenu);
	itemAt(inMenu, inItem)->modifiers = inModifiers;
	_HIMenuChanged(inMenu);
	return noErr;
}

OSStatus GetMenuItemModifiers(MenuRef inMenu, MenuItemIndex inItem, UInt8* outModifiers)
{
	if (!isItem(inMenu, inItem) || !outModifiers)
		return itemFailure(inMenu);
	*outModifiers = itemAt(inMenu, inItem)->modifiers;
	return noErr;
}

OSStatus SetMenuItemCommandKey(MenuRef inMenu, MenuItemIndex inItem, Boolean inSetVirtualKey, UInt16 inKey)
{
	if (!isItem(inMenu, inItem))
		return itemFailure(inMenu);
	struct MenuItem* item = itemAt(inMenu, inItem);

	item->commandKey = inKey;
	if (inSetVirtualKey)
		item->attributes |= kMenuItemAttrUseVirtualKey;
	else
		item->attributes &= ~kMenuItemAttrUseVirtualKey;
	_HIMenuChanged(inMenu);
	return noErr;
}

OSStatus GetMenuItemCommandKey(MenuRef inMenu, MenuItemIndex inItem, Boolean inGetVirtualKey, UInt16* outKey)
{
	if (!isItem(inMenu, inItem) || !outKey)
		return itemFailure(inMenu);
	*outKey = itemAt(inMenu, inItem)->commandKey;
	return noErr;
}

void SetItemMark(MenuRef theMenu, MenuItemIndex item, CharParameter markChar)
{
	if (!isItem(theMenu, item))
		return;
	itemAt(theMenu, item)->mark = (UInt16) markChar;
	_HIMenuChanged(theMenu);
}

void GetItemMark(MenuRef theMenu, MenuItemIndex item, CharParameter* markChar)
{
	if (isItem(theMenu, item) && markChar)
		*markChar = (CharParameter) itemAt(theMenu, item)->mark;
}

void CheckMenuItem(MenuRef theMenu, MenuItemIndex item, Boolean checked)
{
	SetItemMark(theMenu, item, checked ? checkMark : noMark);
}

OSStatus SetMenuItemRefCon(MenuRef inMenu, MenuItemIndex inItem, UInt32 inRefCon)
{
	if (!isItem(inMenu, inItem))
		return itemFailure(inMenu);
	itemAt(inMenu, inItem)->refCon = inRefCon;
	return noErr;
}

OSStatus GetMenuItemRefCon(MenuRef inMenu, MenuItemIndex inItem, UInt32* outRefCon)
{
	if (!isItem(inMenu, inItem) || !outRefCon)
		return itemFailure(inMenu);
	*outRefCon = itemAt(inMenu, inItem)->refCon;
	return noErr;
}

static struct MenuItemProperty* findProperty(struct MenuItem* item, OSType creator, OSType tag)
{
	for (ItemCount i = 0; i < item->propertyCount; i++)
	{
		if (item->properties[i].creator == creator && item->properties[i].tag == tag)
			return &item->properties[i];
	}
	return NULL;
}

OSStatus SetMenuItemProperty(MenuRef menu, MenuItemIndex item, OSType propertyCreator, OSType propertyTag, ByteCount propertySize, const void* propertyData)
{
	if (!isItem(menu, item) || (propertySize && !propertyData))
		return itemFailure(menu);

	lockMenus();
	struct MenuItem* entry = itemAt(menu, item);
	struct MenuItemProperty* property = findProperty(entry, propertyCreator, propertyTag);
	void* data = malloc(propertySize ? propertySize : 1);

	if (!data)
	{
		unlockMenus();
		return memFullErr;
	}
	if (propertySize)
		memcpy(data, propertyData, propertySize);
	if (property)
	{
		free(property->data);
		property->data = data;
		property->size = propertySize;
	}
	else
	{
		struct MenuItemProperty* list = realloc(entry->properties, (entry->propertyCount + 1) * sizeof(*list));

		if (!list)
		{
			free(data);
			unlockMenus();
			return memFullErr;
		}
		entry->properties = list;
		list[entry->propertyCount].creator = propertyCreator;
		list[entry->propertyCount].tag = propertyTag;
		list[entry->propertyCount].size = propertySize;
		list[entry->propertyCount].data = data;
		entry->propertyCount++;
	}
	unlockMenus();
	return noErr;
}

OSStatus GetMenuItemProperty(MenuRef menu, MenuItemIndex item, OSType propertyCreator, OSType propertyTag, ByteCount bufferSize, ByteCount* actualSize, void* propertyBuffer)
{
	if (!isItem(menu, item))
		return itemFailure(menu);

	lockMenus();
	struct MenuItemProperty* property = findProperty(itemAt(menu, item), propertyCreator, propertyTag);

	if (!property)
	{
		unlockMenus();
		return menuPropertyNotFoundErr;
	}
	// QUIRK: a buffer smaller than the property is not an error: the bytes that fit are copied and
	// the actual size is the size of the property. Not documented. Evidence: black-box
	// observation on macOS 10.14.4 (i386) and 26.7.3 (tier 2).
	// Test: test_quirk_property_small_buffer in tests/src/carbon_menus.c
	if (propertyBuffer && bufferSize)
		memcpy(propertyBuffer, property->data, bufferSize < property->size ? bufferSize : property->size);
	if (actualSize)
		*actualSize = property->size;
	unlockMenus();
	return noErr;
}

OSStatus RemoveMenuItemProperty(MenuRef menu, MenuItemIndex item, OSType propertyCreator, OSType propertyTag)
{
	if (!isItem(menu, item))
		return itemFailure(menu);

	lockMenus();
	struct MenuItem* entry = itemAt(menu, item);
	struct MenuItemProperty* property = findProperty(entry, propertyCreator, propertyTag);

	if (!property)
	{
		unlockMenus();
		return menuPropertyNotFoundErr;
	}
	free(property->data);
	memmove(property, property + 1, (entry->propertyCount - 1 - (property - entry->properties)) * sizeof(*property));
	entry->propertyCount--;
	unlockMenus();
	return noErr;
}

OSStatus ChangeMenuItemAttributes(MenuRef menu, MenuItemIndex item, MenuItemAttributes setTheseAttributes, MenuItemAttributes clearTheseAttributes)
{
	if (!isItem(menu, item))
		return itemFailure(menu);

	lockMenus();
	struct MenuItem* entry = itemAt(menu, item);

	entry->attributes = (entry->attributes | setTheseAttributes) & ~clearTheseAttributes;
	// QUIRK: making an item a separator disables it as well. Not documented. Evidence:
	// black-box observation on macOS 10.14.4 (i386) and 26.7.3 (tier 2): the attributes of an item
	// made a separator read back as separator and disabled.
	// Test: test_quirk_separator_is_disabled in tests/src/carbon_menus.c
	if (setTheseAttributes & kMenuItemAttrSeparator)
		entry->attributes |= kMenuItemAttrDisabled;
	unlockMenus();
	_HIMenuChanged(menu);
	return noErr;
}

OSStatus GetMenuItemAttributes(MenuRef menu, MenuItemIndex item, MenuItemAttributes* outAttributes)
{
	if (!isItem(menu, item) || !outAttributes)
		return itemFailure(menu);
	*outAttributes = itemAt(menu, item)->attributes;
	return noErr;
}

OSStatus SetMenuItemIndent(MenuRef inMenu, MenuItemIndex inItem, UInt32 inIndent)
{
	if (!isItem(inMenu, inItem))
		return itemFailure(inMenu);
	itemAt(inMenu, inItem)->indent = inIndent;
	_HIMenuChanged(inMenu);
	return noErr;
}

void SetItemStyle(MenuRef theMenu, MenuItemIndex item, StyleParameter chStyle)
{
	if (!isItem(theMenu, item))
		return;
	itemAt(theMenu, item)->style = (UInt8) chStyle;
	_HIMenuChanged(theMenu);
}

// ---------------------------------------------------------------------------------------
// Looking up commands
// ---------------------------------------------------------------------------------------

struct CommandSearch
{
	MenuCommand command;
	UInt32 wanted;     // the number of the match to stop at (1 is the first)
	UInt32 found;
	MenuRef menu;
	MenuItemIndex index;
};

static bool searchItems(MenuRef menu, struct CommandSearch* search, bool inlineSubmenus, int depth);

static bool searchOneItem(MenuRef menu, MenuItemIndex index, struct CommandSearch* search, bool inlineSubmenus, int depth)
{
	struct MenuItem* item = itemAt(menu, index);

	if (item->command == search->command && ++search->found == search->wanted)
	{
		search->menu = menu;
		search->index = index;
		return true;
	}
	if (inlineSubmenus && depth < 16)
	{
		MenuRef sub = submenuOf(item);

		if (sub && searchItems(sub, search, inlineSubmenus, depth + 1))
			return true;
	}
	return false;
}

// With inlineSubmenus the items of the submenu of an item are visited right after the item, which is
// the order of the menu bar; without, the items of the menu come first and the submenus after them.
// QUIRK: the two orders. Not documented. Evidence: black-box observation on macOS 10.14.4 (i386
// and x86_64) and 26.7.3 (tier 2): with three bar menus X (items x1, xh with submenu Y, x3), Y (y1,
// yh with submenu Z, y3), Z (z1) and W, all with one command, the matches from the menu bar come in
// the order x1 xh y1 yh z1 y3 x3 w1, those from X in the order x1 xh x3 y1 yh y3 z1.
// Test: test_quirk_command_lookup_order in tests/src/carbon_menus.c
static bool searchItems(MenuRef menu, struct CommandSearch* search, bool inlineSubmenus, int depth)
{
	for (MenuItemIndex i = 1; i <= menu->count; i++)
	{
		if (searchOneItem(menu, i, search, inlineSubmenus, depth))
			return true;
	}
	if (!inlineSubmenus && depth < 16)
	{
		for (MenuItemIndex i = 1; i <= menu->count; i++)
		{
			MenuRef sub = submenuOf(itemAt(menu, i));

			if (sub && searchItems(sub, search, inlineSubmenus, depth + 1))
				return true;
		}
	}
	return false;
}

// The wanted-th item with the command, from the menu (NULL: the menu bar). Called with the lock held.
static OSStatus findCommand(MenuRef start, MenuCommand command, UInt32 wanted, MenuRef* outMenu, MenuItemIndex* outIndex)
{
	struct CommandSearch search = { command, wanted, 0, NULL, 0 };
	bool found = false;

	if (start && !isMenu(start))
		return paramErr;
	if (wanted == 0 || command == 0)
		return paramErr;

	if (start)
		found = searchItems(start, &search, false, 0);
	else
	{
		for (ItemCount i = 0; i < g_insertedCount && !found; i++)
		{
			if (g_inserted[i].inBar)
				found = searchItems(g_inserted[i].menu, &search, true, 0);
		}
	}
	if (!found)
		return menuItemNotFoundErr;
	if (outMenu)
		*outMenu = search.menu;
	if (outIndex)
		*outIndex = search.index;
	return noErr;
}

OSStatus GetIndMenuItemWithCommandID(MenuRef inMenu, MenuCommand inCommandID, UInt32 inItemIndex, MenuRef* outMenu, MenuItemIndex* outIndex)
{
	lockMenus();
	OSStatus result = findCommand(inMenu, inCommandID, inItemIndex, outMenu, outIndex);
	unlockMenus();
	return result;
}

static void setCommandEnabled(MenuRef menu, MenuCommand command, bool enabled)
{
	MenuRef found;
	MenuItemIndex index;

	// Only the first item with the command is changed
	lockMenus();
	OSStatus status = findCommand(menu, command, 1, &found, &index);
	unlockMenus();
	if (status == noErr)
		setEnabled(found, index, enabled);
}

void DisableMenuCommand(MenuRef inMenu, MenuCommand inCommandID)
{
	setCommandEnabled(inMenu, inCommandID, false);
}

#if !__LP64__
UInt32 CountMenuItemsWithCommandID(MenuRef inMenu, MenuCommand inCommandID)
{
	UInt32 count = 0;

	lockMenus();
	while (findCommand(inMenu, inCommandID, count + 1, NULL, NULL) == noErr)
		count++;
	unlockMenus();
	return count;
}

void EnableMenuCommand(MenuRef inMenu, MenuCommand inCommandID)
{
	setCommandEnabled(inMenu, inCommandID, true);
}

Boolean IsMenuCommandEnabled(MenuRef inMenu, MenuCommand inCommandID)
{
	MenuRef found;
	MenuItemIndex index;

	lockMenus();
	OSStatus status = findCommand(inMenu, inCommandID, 1, &found, &index);
	Boolean enabled = status == noErr && IsMenuItemEnabled(found, index);
	unlockMenus();
	return enabled;
}

Boolean MenuHasEnabledItems(MenuRef menu)
{
	if (!isMenu(menu) || !menu->enabled)
		return false;
	lockMenus();
	Boolean any = false;

	for (MenuItemIndex i = 1; i <= menu->count && !any; i++)
		any = (itemAt(menu, i)->attributes & (kMenuItemAttrDisabled | kMenuItemAttrSeparator)) == 0;
	unlockMenus();
	return any;
}

OSStatus RetainMenu(MenuRef inMenu)
{
	if (!isMenu(inMenu))
		return paramErr;
	CFRetain((CFTypeRef) inMenu);
	return noErr;
}

OSStatus ReleaseMenu(MenuRef inMenu)
{
	if (!isMenu(inMenu))
		return paramErr;
	CFRelease((CFTypeRef) inMenu);
	return noErr;
}

ItemCount GetMenuRetainCount(MenuRef inMenu)
{
	return isMenu(inMenu) ? (ItemCount) CFGetRetainCount((CFTypeRef) inMenu) : 0;
}

OSStatus GetMenuItemKeyGlyph(MenuRef inMenu, MenuItemIndex inItem, SInt16* outGlyph)
{
	if (!isItem(inMenu, inItem) || !outGlyph)
		return itemFailure(inMenu);
	*outGlyph = itemAt(inMenu, inItem)->glyph;
	return noErr;
}

OSStatus SetMenuItemHierarchicalID(MenuRef inMenu, MenuItemIndex inItem, MenuID inHierID)
{
	if (!isItem(inMenu, inItem))
		return itemFailure(inMenu);

	lockMenus();
	struct MenuItem* item = itemAt(inMenu, inItem);
	MenuRef previous = item->submenu;

	item->submenu = NULL;
	item->submenuID = inHierID;
	unlockMenus();
	if (previous)
		CFRelease((CFTypeRef) previous);
	_HIMenuChanged(inMenu);
	return noErr;
}

OSStatus GetMenuItemHierarchicalID(MenuRef inMenu, MenuItemIndex inItem, MenuID* outHierID)
{
	if (!isItem(inMenu, inItem) || !outHierID)
		return itemFailure(inMenu);
	lockMenus();
	MenuRef sub = submenuOf(itemAt(inMenu, inItem));

	*outHierID = sub ? sub->menuID : 0;
	unlockMenus();
	return noErr;
}

OSStatus GetMenuItemPropertySize(MenuRef menu, MenuItemIndex item, OSType propertyCreator, OSType propertyTag, ByteCount* size)
{
	if (!isItem(menu, item) || !size)
		return itemFailure(menu);
	lockMenus();
	struct MenuItemProperty* property = findProperty(itemAt(menu, item), propertyCreator, propertyTag);

	if (property)
		*size = property->size;
	unlockMenus();
	return property ? noErr : menuPropertyNotFoundErr;
}

OSStatus GetMenuItemIndent(MenuRef inMenu, MenuItemIndex inItem, UInt32* outIndent)
{
	if (!isItem(inMenu, inItem) || !outIndent)
		return itemFailure(inMenu);
	*outIndent = itemAt(inMenu, inItem)->indent;
	return noErr;
}

OSStatus SetMenuItemFontID(MenuRef inMenu, MenuItemIndex inItem, SInt16 inFontID)
{
	if (!isItem(inMenu, inItem))
		return itemFailure(inMenu);
	itemAt(inMenu, inItem)->fontID = inFontID;
	return noErr;
}

OSStatus GetMenuItemFontID(MenuRef inMenu, MenuItemIndex inItem, SInt16* outFontID)
{
	if (!isItem(inMenu, inItem) || !outFontID)
		return itemFailure(inMenu);
	*outFontID = itemAt(inMenu, inItem)->fontID;
	return noErr;
}

void GetItemStyle(MenuRef menu, MenuItemIndex item, Style* chStyle)
{
	if (isItem(menu, item) && chStyle)
		*chStyle = itemAt(menu, item)->style;
}
#endif

// ---------------------------------------------------------------------------------------
// Menu events and commands
// ---------------------------------------------------------------------------------------

EventTargetRef GetMenuEventTarget(MenuRef menu)
{
	if (!isMenu(menu))
		return NULL;

	lockMenus();
	if (!menu->target)
		menu->target = _HICreateEventTarget(_HIApplicationEventTarget());
	EventTargetRef target = menu->target;
	unlockMenus();
	return target;
}

// Sends a command event of the kind (process or update status) for an item of a menu, through the
// dispatcher target, which is where the front window's handlers (and then the application's) see it
static OSStatus sendCommandEvent(UInt32 kind, MenuCommand command, MenuRef menu, MenuItemIndex item, UInt32 attributes)
{
	EventRef event;
	HICommand hiCommand;

	hiCommand.attributes = attributes;
	hiCommand.commandID = command;
	hiCommand.menu.menuRef = menu;
	hiCommand.menu.menuItemIndex = item;
	if (CreateEvent(NULL, kEventClassCommand, kind, _HICurrentTime(), kEventAttributeNone, &event) != noErr)
		return memFullErr;
	SetEventParameter(event, kEventParamDirectObject, typeHICommand, sizeof(hiCommand), &hiCommand);
	OSStatus result = SendEventToEventTarget(event, GetEventDispatcherTarget());
	ReleaseEvent(event);
	return result;
}

OSStatus ProcessHICommand(const HICommand* inCommand)
{
	// QUIRK: no command is not an error of the arguments: nothing handles it. Not documented.
	// Evidence: black-box observation on macOS 10.14.4 (i386 and x86_64) and 26.7.3 (tier 2).
	// Test: test_quirk_process_null_command in tests/src/carbon_menus.c
	if (!inCommand)
		return eventNotHandledErr;

	EventRef event;

	if (CreateEvent(NULL, kEventClassCommand, kEventCommandProcess, _HICurrentTime(), kEventAttributeNone, &event) != noErr)
		return memFullErr;
	SetEventParameter(event, kEventParamDirectObject, typeHICommand, sizeof(*inCommand), inCommand);
	OSStatus result = SendEventToEventTarget(event, GetEventDispatcherTarget());
	ReleaseEvent(event);
	return result;
}

// Sends a menu event to the target of the menu (and so to the application); the key parameter is
// set for the events made for a key equivalent
OSStatus _HISendMenuEvent(UInt32 kind, MenuRef menu, MenuItemIndex item, bool forKeyEvent, bool firstOpen)
{
	EventRef event;

	if (!isMenu(menu))
		return paramErr;
	if (CreateEvent(NULL, kEventClassMenu, kind, _HICurrentTime(), kEventAttributeNone, &event) != noErr)
		return memFullErr;
	SetEventParameter(event, kEventParamDirectObject, typeMenuRef, sizeof(menu), &menu);
	if (item)
		SetEventParameter(event, kEventParamMenuItemIndex, typeMenuItemIndex, sizeof(item), &item);
	if (kind == kEventMenuEnableItems)
	{
		Boolean key = forKeyEvent;

		SetEventParameter(event, kEventParamEnableMenuForKeyEvent, typeBoolean, sizeof(key), &key);
	}
	if (kind == kEventMenuOpening)
	{
		Boolean first = firstOpen;

		SetEventParameter(event, kEventParamMenuFirstOpen, typeBoolean, sizeof(first), &first);
	}
	OSStatus result = SendEventToEventTarget(event, GetMenuEventTarget(menu));
	ReleaseEvent(event);
	return result;
}

static void populateWindowMenu(MenuRef menu);

void _HIMenuMakeWindowMenu(MenuRef menu)
{
	if (isMenu(menu))
		menu->isWindowMenu = true;
}

// Brings the items of the menu up to date: handlers of kEventMenuEnableItems do it; without one,
// every item with a command gets an update status command event. Items of a menu with
// kMenuAttrAutoDisable that no handler answers for are disabled.
void _HIUpdateMenuItems(MenuRef menu, bool forKeyEvent)
{
	if (!isMenu(menu))
		return;

	CFRetain((CFTypeRef) menu);
	if (menu->isWindowMenu)
		populateWindowMenu(menu);
	if (_HISendMenuEvent(kEventMenuEnableItems, menu, 0, forKeyEvent, false) == eventNotHandledErr)
	{
		for (MenuItemIndex i = 1; i <= menu->count; i++)
		{
			MenuCommand command = itemAt(menu, i)->command;

			if (command == 0 || (itemAt(menu, i)->attributes & kMenuItemAttrSeparator))
				continue;
			OSStatus result = sendCommandEvent(kEventCommandUpdateStatus, command, menu, i, kHICommandFromMenu);

			if (result == eventNotHandledErr && (menu->attributes & kMenuAttrAutoDisable))
				DisableMenuItem(menu, i);
		}
	}
	CFRelease((CFTypeRef) menu);
}

static MenuCommand systemItemCommand(MenuRef menu, MenuItemIndex item);

OSStatus _HIProcessMenuCommand(MenuRef menu, MenuItemIndex item)
{
	MenuCommand command = 0;

	// An item the system added to the application menu
	if (isMenu(menu) && item > menu->count && (command = systemItemCommand(menu, item)) != 0)
		return sendCommandEvent(kEventCommandProcess, command, menu, item, kHICommandFromMenu);
	if (!isItem(menu, item))
		return itemFailure(menu);
	command = itemAt(menu, item)->command;
	if (command == 0)
		return eventNotHandledErr;
	return sendCommandEvent(kEventCommandProcess, command, menu, item, kHICommandFromMenu);
}

// ---------------------------------------------------------------------------------------
// The standard items of the application menu and the Window menu
// ---------------------------------------------------------------------------------------

// The application menu of a Carbon application has more than the program put in it: Services, Hide,
// Hide Others, Show All and Quit are the system's (kHICommandHide, kHICommandHideOthers,
// kHICommandShowAll, kHICommandQuit), and so is the standard Window menu's list of windows. They are
// not items of the menu object: CountMenuItems() and the other item calls see only what the program
// added, as the Menu Manager of Mojave shows (black-box observation, tier 2, macOS 10.14.4 i386: the
// items of the Apple menu were the program's three, with the standard Window menu's list not in the
// menu). The menu tracking shows them after the program's own items. A menu whose own items have the
// command (a Quit item that the program added, for instance) does not get the system's.
// Evidence for the content: Apple's Carbon porting guide, where a program that runs with the Aqua
// menu layout drops its own Quit item and "the standard application event handler calls the default
// Quit Apple event handler"; the Human Interface Guidelines' application menu.
struct SystemItem
{
	const char* title;      // NULL for a separator
	bool appName;           // "<title> <application name>"
	MenuCommand command;
	MenuItemAttributes attributes;
	UInt8 modifiers;
	UInt16 key;
	int group;              // 1: left out when the menu has Hide, 2: left out when it has Quit, 3: needs an item before it
	bool services;
};

static const struct SystemItem kSystemItems[] = {
	{ NULL, false, 0, kMenuItemAttrSeparator | kMenuItemAttrDisabled, 0, 0, 3, false },
	{ "Services", false, 0, 0, 0, 0, 0, true },
	{ NULL, false, 0, kMenuItemAttrSeparator | kMenuItemAttrDisabled, 0, 0, 1, false },
	{ "Hide", true, kHICommandHide, 0, 0, 'H', 1, false },
	{ "Hide Others", false, kHICommandHideOthers, 0, kMenuOptionModifier, 'H', 1, false },
	{ "Show All", false, kHICommandShowAll, 0, 0, 0, 1, false },
	{ NULL, false, 0, kMenuItemAttrSeparator | kMenuItemAttrDisabled, 0, 0, 2, false },
	{ "Quit", true, kHICommandQuit, 0, 0, 'Q', 2, false },
};
#define kSystemItemTotal (sizeof(kSystemItems) / sizeof(kSystemItems[0]))
#define kAppleMenuID 128

static CFStringRef applicationName(void)
{
	static CFStringRef name;
	static dispatch_once_t once;

	dispatch_once(&once, ^{
		CFTypeRef value = CFBundleGetValueForInfoDictionaryKey(CFBundleGetMainBundle(), kCFBundleNameKey);

		if (value && CFGetTypeID(value) == CFStringGetTypeID())
			name = CFRetain(value);
		else
			name = CFStringCreateWithCString(NULL, getprogname(), kCFStringEncodingUTF8);
	});
	return name;
}

static CFStringRef systemItemTitle(const struct SystemItem* item)
{
	static CFStringRef titles[kSystemItemTotal];
	static dispatch_once_t once;

	dispatch_once(&once, ^{
		for (size_t i = 0; i < kSystemItemTotal; i++)
		{
			if (!kSystemItems[i].title)
				titles[i] = CFSTR("-");
			else if (kSystemItems[i].appName)
				titles[i] = CFStringCreateWithFormat(NULL, NULL, CFSTR("%s %@"), kSystemItems[i].title, applicationName());
			else
				titles[i] = CFStringCreateWithCString(NULL, kSystemItems[i].title, kCFStringEncodingUTF8);
		}
	});
	return titles[item - kSystemItems];
}

static MenuRef servicesMenu(void)
{
	static MenuRef menu;
	static dispatch_once_t once;

	dispatch_once(&once, ^{
		if (CreateNewMenu(0, 0, &menu) == noErr)
			AppendMenuItemTextWithCFString(menu, CFSTR("No Services Apply"), kMenuItemAttrDisabled, 0, NULL);
	});
	return menu;
}

// The application menu is the one with the Apple menu's ID, or else the first of the menu bar
static bool isApplicationMenu(MenuRef menu)
{
	MenuRef first = NULL, apple = NULL;

	lockMenus();
	for (ItemCount i = 0; i < g_insertedCount; i++)
	{
		if (!g_inserted[i].inBar)
			continue;
		if (!first)
			first = g_inserted[i].menu;
		if (!apple && g_inserted[i].menu->menuID == kAppleMenuID)
			apple = g_inserted[i].menu;
	}
	unlockMenus();
	return menu == (apple ? apple : first);
}

static ItemCount systemItemsOf(MenuRef menu, const struct SystemItem** out)
{
	if (!isApplicationMenu(menu))
		return 0;

	lockMenus();
	bool hasHide = findCommand(menu, kHICommandHide, 1, NULL, NULL) == noErr;
	bool hasQuit = findCommand(menu, kHICommandQuit, 1, NULL, NULL) == noErr;
	ItemCount count = 0;

	for (size_t i = 0; i < kSystemItemTotal; i++)
	{
		const struct SystemItem* item = &kSystemItems[i];

		if ((item->group == 1 && hasHide) || (item->group == 2 && hasQuit) || (item->group == 3 && menu->count == 0))
			continue;
		if (out)
			out[count] = item;
		count++;
	}
	unlockMenus();
	return count;
}

static MenuCommand systemItemCommand(MenuRef menu, MenuItemIndex item)
{
	const struct SystemItem* items[kSystemItemTotal];
	ItemCount count = systemItemsOf(menu, items);
	ItemCount wanted = item - menu->count;

	return wanted >= 1 && wanted <= count ? items[wanted - 1]->command : 0;
}

// The standard Window menu, with the items and commands the Menu Manager of Mojave gives it (black-box
// observation, tier 2, macOS 10.14.4 i386: the ten items below, in this order, with these commands,
// key equivalents and attributes; the menu auto-disables its items). The window list goes between the two
// hidden list markers, and is made when the menu opens.
// Test: tests/src/carbon_standard_menus.c
#define kStandardWindowMenuID (-21405)

static const struct
{
	const char* title;
	MenuCommand command;
	MenuItemAttributes attributes;
	UInt8 modifiers;
	UInt16 key;
} kWindowMenuItems[] = {
	{ "Minimize", kHICommandMinimizeWindow, kMenuItemAttrDynamic, 0, 'M' },
	{ "Minimize All", kHICommandMinimizeAll, kMenuItemAttrDynamic, kMenuOptionModifier, 'M' },
	{ "Zoom", kHICommandZoomWindow, 0, 0, 0 },
	{ "Cycle Through Windows", kHICommandRotateWindowsForward, kMenuItemAttrHidden, 0, 0 },
	{ "Cycle Backward", kHICommandRotateWindowsBackward, kMenuItemAttrHidden, 0, 0 },
	{ "", 0, kMenuItemAttrSeparator, 0, 0 },
	{ "Bring All to Front", kHICommandBringAllToFront, kMenuItemAttrDynamic, 0, 0 },
	{ "Arrange in Front", kHICommandArrangeInFront, kMenuItemAttrDynamic, kMenuOptionModifier, 0 },
	{ "Window list separator", kHICommandWindowListSeparator, kMenuItemAttrHidden | kMenuItemAttrSeparator | kMenuItemAttrDisabled, 0, 0 },
	{ "Window list terminator", kHICommandWindowListTerminator, kMenuItemAttrHidden | kMenuItemAttrSeparator | kMenuItemAttrDisabled, 0, 0 },
};

OSStatus CreateStandardWindowMenu(OptionBits inOptions, MenuRef* outMenu)
{
	if (!outMenu)
		return paramErr;
	*outMenu = NULL;

	MenuRef menu = newMenu(kStandardWindowMenuID, kMenuAttrAutoDisable);

	if (!menu)
		return memFullErr;
	menu->isWindowMenu = true;
	SetMenuTitleWithCFString(menu, CFSTR("Window"));
	for (size_t i = 0; i < sizeof(kWindowMenuItems) / sizeof(kWindowMenuItems[0]); i++)
	{
		MenuItemIndex index;
		CFStringRef title = CFStringCreateWithCString(NULL, kWindowMenuItems[i].title, kCFStringEncodingUTF8);

		insertItem(menu, menu->count, title, kWindowMenuItems[i].attributes, kWindowMenuItems[i].command, &index);
		if (kWindowMenuItems[i].command && (kWindowMenuItems[i].attributes & kMenuItemAttrSeparator))
			SetMenuItemTextWithCFString(menu, index, title);
		CFRelease(title);
		if (kWindowMenuItems[i].key)
			SetMenuItemCommandKey(menu, index, false, kWindowMenuItems[i].key);
		if (kWindowMenuItems[i].modifiers)
			SetMenuItemModifiers(menu, index, kWindowMenuItems[i].modifiers);
	}
	*outMenu = menu;
	return noErr;
}

// The windows of the application between the list markers: a separator and one item for each, with the
// window as the item's property (the command of the item is kHICommandSelectWindow). The window
// that is active has a check mark and a minimized one a diamond.
static void populateWindowMenu(MenuRef menu)
{
	MenuItemIndex first = 0;

	// The list of the last time goes (its items have the window list property)
	for (MenuItemIndex i = menu->count; i >= 1; i--)
	{
		if (findProperty(itemAt(menu, i), 'dlng', 'wlst'))
			DeleteMenuItem(menu, i);
	}
	// It goes after the list marker of the standard menu, else at the end of the menu
	for (MenuItemIndex i = 1; i <= menu->count; i++)
	{
		if (itemAt(menu, i)->command == kHICommandWindowListSeparator)
			first = i;
	}

	WindowRef* windows;
	ItemCount count = _HICopyWindowList(&windows);
	ItemCount position = first ? first : menu->count;

	if (count)
	{
		MenuItemIndex separator;

		if (insertItem(menu, position++, CFSTR("-"), kMenuItemAttrSeparator, 0, &separator) == noErr)
			SetMenuItemProperty(menu, separator, 'dlng', 'wlst', 0, NULL);
	}
	for (ItemCount i = 0; i < count; i++)
	{
		CFStringRef title = NULL;
		MenuItemIndex index;

		CopyWindowTitleAsCFString(windows[i], &title);
		if (insertItem(menu, position++, title, 0, kHICommandSelectWindow, &index) == noErr)
		{
			SetMenuItemProperty(menu, index, 'dlng', 'wind', sizeof(WindowRef), &windows[i]);
			SetMenuItemProperty(menu, index, 'dlng', 'wlst', 0, NULL);
			if (IsWindowCollapsed(windows[i]))
				SetItemMark(menu, index, 0x13);
			else if (IsWindowActive(windows[i]))
				SetItemMark(menu, index, 0x12);
		}
		if (title)
			CFRelease(title);
	}
	free(windows);
}

// ---------------------------------------------------------------------------------------
// Key equivalents
// ---------------------------------------------------------------------------------------

// The character a virtual key produces without Command in the current ASCII-capable layout, with and
// without Shift: 0 when it produces none
static UniChar characterOfKey(UInt32 keyCode, bool shift)
{
	UniChar result = 0;
	TISInputSourceRef source = TISCopyCurrentASCIICapableKeyboardLayoutInputSource();

	if (!source)
		return 0;
	CFDataRef layoutData = TISGetInputSourceProperty(source, kTISPropertyUnicodeKeyLayoutData);

	if (layoutData)
	{
		UInt32 deadKeyState = 0;
		UniChar chars[4];
		UniCharCount length = 0;

		if (UCKeyTranslate((const UCKeyboardLayout*) CFDataGetBytePtr(layoutData), (UInt16) keyCode, kUCKeyActionDown, shift ? (shiftKey >> 8) & 0xff : 0, 0, kUCKeyTranslateNoDeadKeysMask, &deadKeyState, 4, &length, chars) == noErr && length == 1)
			result = chars[0];
	}
	CFRelease(source);
	return result;
}

static UniChar upperCase(UniChar c)
{
	return c < 0x80 ? (UniChar) toupper(c) : c;
}

// What a key event is, for the key equivalents: the virtual key, the characters it may stand for
// and the modifiers (Carbon event modifier bits)
struct KeyEquivalentQuery
{
	UInt32 keyCode;
	UniChar characters[2];
	UInt32 modifiers;
	// The character is compared as it is with the lower case of the item's key (see MenuEvent)
	bool exactCase;
};

static bool itemMatchesKey(const struct MenuItem* item, const struct KeyEquivalentQuery* query)
{
	if (item->commandKey == 0 && !(item->attributes & kMenuItemAttrUseVirtualKey))
		return false;
	if (item->attributes & kMenuItemAttrSeparator)
		return false;

	// The modifiers the item needs: Command unless the item says otherwise
	UInt32 needed = (item->modifiers & kMenuNoCommandModifier) ? 0 : cmdKey;

	if (item->modifiers & kMenuShiftModifier)
		needed |= shiftKey;
	if (item->modifiers & kMenuOptionModifier)
		needed |= optionKey;
	if (item->modifiers & kMenuControlModifier)
		needed |= controlKey;

	UInt32 given = query->modifiers & (cmdKey | shiftKey | optionKey | controlKey);

	if (item->attributes & kMenuItemAttrUseVirtualKey)
		return item->commandKey == query->keyCode && given == needed;
	if (given != needed)
		return false;

	if (query->exactCase)
		return query->characters[0] == (item->commandKey < 0x80 ? (UniChar) tolower((int) item->commandKey) : item->commandKey);

	UniChar wanted = upperCase(item->commandKey);

	return upperCase(query->characters[0]) == wanted || (query->characters[1] && upperCase(query->characters[1]) == wanted);
}

// The first enabled (unless disabled ones are wanted) item that has the key equivalent, in the menu
// and, unless they are not wanted, its submenus
static bool findKeyEquivalent(MenuRef menu, const struct KeyEquivalentQuery* query, MenuEventOptions options, MenuRef* outMenu, MenuItemIndex* outItem, int depth)
{
	if (depth > 16 || !menu->enabled)
		return false;
	for (MenuItemIndex i = 1; i <= menu->count; i++)
	{
		struct MenuItem* item = itemAt(menu, i);

		if (!itemMatchesKey(item, query))
			continue;
		if ((item->attributes & kMenuItemAttrDisabled) && !(options & kMenuEventIncludeDisabledItems))
			continue;
		if (item->attributes & kMenuItemAttrHidden)
			continue;
		*outMenu = menu;
		*outItem = i;
		return true;
	}
	if (!(options & kMenuEventDontCheckSubmenus))
	{
		for (MenuItemIndex i = 1; i <= menu->count; i++)
		{
			MenuRef sub = submenuOf(itemAt(menu, i));

			if (sub && findKeyEquivalent(sub, query, options, outMenu, outItem, depth + 1))
				return true;
		}
	}
	return false;
}

// Whether the key equivalent is a candidate in the menu tree at root, and where. The menu is not
// updated yet, so disabled items count.
static bool matchInTree(MenuRef root, const struct KeyEquivalentQuery* query, MenuEventOptions options, MenuRef* outMenu, MenuItemIndex* outItem)
{
	MenuRef candidate = NULL;
	MenuItemIndex candidateItem = 0;

	lockMenus();
	bool any = findKeyEquivalent(root, query, options | kMenuEventIncludeDisabledItems, &candidate, &candidateItem, 0);
	if (any)
		CFRetain((CFTypeRef) candidate);
	unlockMenus();
	if (!any)
		return false;

	// The items are brought up to date before the match is looked at again
	_HIUpdateMenuItems(candidate, true);
	CFRelease((CFTypeRef) candidate);

	lockMenus();
	bool matched = findKeyEquivalent(root, query, options, outMenu, outItem, 0);
	unlockMenus();
	return matched;
}

static bool matchKey(MenuRef start, const struct KeyEquivalentQuery* query, MenuEventOptions options, MenuRef* outMenu, MenuItemIndex* outItem)
{
	MenuRef found = NULL;
	MenuItemIndex foundItem = 0;
	bool matched = false;

	if (start)
		matched = matchInTree(start, query, options, &found, &foundItem);
	else
	{
		MenuRef* menus;
		ItemCount count = _HICopyMenuBarMenus(&menus);

		for (ItemCount i = 0; i < count; i++)
		{
			if (!matched)
				matched = matchInTree(menus[i], query, options, &found, &foundItem);
			CFRelease((CFTypeRef) menus[i]);
		}
		free(menus);
	}
	if (matched)
	{
		if (outMenu)
			*outMenu = found;
		if (outItem)
			*outItem = foundItem;
	}
	return matched;
}

// The key equivalent of a system item, for the Command-key handling of the menu bar
static bool systemKeyEquivalent(const struct KeyEquivalentQuery* query, MenuRef* outMenu, MenuItemIndex* outItem)
{
	MenuRef* menus;
	ItemCount menuCount = _HICopyMenuBarMenus(&menus);
	bool found = false;

	for (ItemCount m = 0; m < menuCount; m++)
	{
		const struct SystemItem* items[kSystemItemTotal];
		ItemCount count = found ? 0 : systemItemsOf(menus[m], items);

		for (ItemCount i = 0; i < count; i++)
		{
			struct MenuItem probe;

			memset(&probe, 0, sizeof(probe));
			probe.commandKey = items[i]->key;
			probe.modifiers = items[i]->modifiers;
			if (items[i]->command && itemMatchesKey(&probe, query))
			{
				*outMenu = menus[m];
				*outItem = (MenuItemIndex) (menus[m]->count + i + 1);
				found = true;
				break;
			}
		}
	}
	for (ItemCount m = 0; m < menuCount; m++)
		CFRelease((CFTypeRef) menus[m]);
	free(menus);
	return found;
}

static bool makeKeyQuery(EventRef inEvent, struct KeyEquivalentQuery* outQuery)
{
	struct KeyEquivalentQuery query = { 0, { 0, 0 }, 0, false };
	UInt32 value;

	if (!inEvent || GetEventClass(inEvent) != kEventClassKeyboard)
		return false;
	if (GetEventKind(inEvent) != kEventRawKeyDown && GetEventKind(inEvent) != kEventRawKeyRepeat)
		return false;
	if (GetEventParameter(inEvent, kEventParamKeyCode, typeUInt32, NULL, sizeof(value), NULL, &value) != noErr)
		return false;
	query.keyCode = value;
	if (GetEventParameter(inEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(value), NULL, &value) != noErr)
		return false;
	query.modifiers = value;

	UInt8 macChar = 0;

	if (GetEventParameter(inEvent, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof(macChar), NULL, &macChar) == noErr && macChar)
		query.characters[0] = macChar;
	else
	{
		query.characters[0] = characterOfKey(query.keyCode, false);
		query.characters[1] = characterOfKey(query.keyCode, true);
	}
	*outQuery = query;
	return true;
}

Boolean IsMenuKeyEvent(MenuRef inStartMenu, EventRef inEvent, MenuEventOptions inOptions, MenuRef* outMenu, MenuItemIndex* outMenuItem)
{
	struct KeyEquivalentQuery query;

	if (!makeKeyQuery(inEvent, &query))
		return false;
	return matchKey(inStartMenu, &query, inOptions, outMenu, outMenuItem);
}

UInt32 MenuEvent(const struct EventRecord* inEvent)
{
	if (!inEvent || (inEvent->what != keyDown && inEvent->what != autoKey))
		return 0;

	struct KeyEquivalentQuery query;
	UInt32 modifiers = inEvent->modifiers;

	query.keyCode = (inEvent->message & keyCodeMask) >> 8;
	query.characters[0] = (UniChar) (inEvent->message & charCodeMask);
	query.characters[1] = 0;
	query.modifiers = modifiers;
	// QUIRK: the character of the event record is compared with the lower case of the item's key, as it
	// is: an upper case character in the record matches no item, and the key of an item given as a
	// capital matches the lower case character (the record has the character without Shift).
	// Not documented. Evidence: black-box observation on macOS 10.14.4 (i386 and x86_64) and 26.7.3
	// (tier 2): with items for s, T (Shift), u (Option) and X, MenuEvent finds them for the records s,
	// t with Shift, u with Option and x, and none for S, T with Shift, T and X.
	// Test: test_quirk_menu_event_character_case in tests/src/carbon_menus.c
	query.exactCase = true;

	MenuRef menu = NULL;
	MenuItemIndex item = 0;

	if (!matchKey(NULL, &query, 0, &menu, &item))
		return 0;
	return ((UInt32) (UInt16) menu->menuID << 16) | item;
}

// The Command-key handling of a raw key event nobody else handled: the menu bar's key equivalent, if
// any, is chosen as if the user had picked the item.
OSStatus _HIHandleKeyEquivalent(EventRef event)
{
	MenuRef menu = NULL;
	MenuItemIndex item = 0;

	if (!IsMenuKeyEvent(NULL, event, 0, &menu, &item))
	{
		// The program's items come first, then the system's of the application menu
		struct KeyEquivalentQuery query;

		if (!makeKeyQuery(event, &query) || !systemKeyEquivalent(&query, &menu, &item))
			return eventNotHandledErr;
	}

	CFRetain((CFTypeRef) menu);
	MenuID hilited = menu->menuID;
	MenuRef top = menu;

	// The title that is highlighted while the command runs is the menu bar menu that holds the item
	lockMenus();
	for (ItemCount i = 0; i < g_insertedCount; i++)
	{
		if (g_inserted[i].inBar && (g_inserted[i].menu == menu || _HIMenuContains(g_inserted[i].menu, menu)))
		{
			top = g_inserted[i].menu;
			break;
		}
	}
	hilited = top->menuID;
	unlockMenus();
	HiliteMenu(hilited);
	OSStatus result = _HIProcessMenuCommand(menu, item);

	HiliteMenu(0);
	CFRelease((CFTypeRef) menu);
	return result == eventNotHandledErr ? noErr : result;
}

static bool menuContains(MenuRef menu, MenuRef inner, int depth)
{
	bool found = false;

	if (depth > 16)
		return false;
	for (MenuItemIndex i = 1; i <= menu->count && !found; i++)
	{
		MenuRef sub = submenuOf(itemAt(menu, i));

		found = sub && (sub == inner || menuContains(sub, inner, depth + 1));
	}
	return found;
}

bool _HIMenuContains(MenuRef menu, MenuRef inner)
{
	return menuContains(menu, inner, 0);
}

// ---------------------------------------------------------------------------------------
// The accessors the menu window code uses
// ---------------------------------------------------------------------------------------

ItemCount _HIMenuItemCount(MenuRef menu)
{
	return menu->count + systemItemsOf(menu, NULL);
}

void _HIMenuGetItem(MenuRef menu, MenuItemIndex index, struct HIMenuItemInfo* info)
{
	if (index > menu->count)
	{
		const struct SystemItem* items[kSystemItemTotal];
		ItemCount count = systemItemsOf(menu, items);

		if (index - menu->count <= count)
		{
			const struct SystemItem* system = items[index - menu->count - 1];

			memset(info, 0, sizeof(*info));
			info->title = systemItemTitle(system);
			info->command = system->command;
			info->attributes = system->attributes;
			info->modifiers = system->modifiers;
			info->commandKey = system->key;
			info->submenu = system->services ? servicesMenu() : NULL;
			return;
		}
	}
	struct MenuItem* item = itemAt(menu, index);

	info->title = item->title;
	info->command = item->command;
	info->attributes = item->attributes;
	info->modifiers = item->modifiers;
	info->commandKey = item->commandKey;
	info->glyph = item->glyph;
	info->mark = item->mark;
	info->submenu = submenuOf(item);
	info->indent = item->indent;
	info->style = item->style;
}

CFStringRef _HIMenuTitle(MenuRef menu)
{
	return menu->title;
}

Boolean _HIMenuIsEnabled(MenuRef menu)
{
	return menu->enabled;
}

MenuAttributes _HIMenuAttributes(MenuRef menu)
{
	return menu->attributes;
}

void _HILockMenus(void)
{
	lockMenus();
}

void _HIUnlockMenus(void)
{
	unlockMenus();
}

bool _HIIsMenu(MenuRef menu)
{
	return isMenu(menu);
}

void _HISetMenuID(MenuRef menu, MenuID menuID)
{
	menu->menuID = menuID;
}

// A negative size is left as it is
void _HISetMenuSize(MenuRef menu, SInt16 width, SInt16 height)
{
	if (width >= 0)
		menu->width = width;
	if (height >= 0)
		menu->height = height;
}
