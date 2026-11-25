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

// The Menu Manager calls of the 32-bit interface: menus made from Pascal strings, MENU and MBAR
// resources, the menu list handle, MenuKey and the contextual menu. They are built on the menu
// objects of Menus.c and exist only where the 32-bit Menu Manager does.

#include <HIToolbox/Menus.h>

#if !__LP64__

#include <HIToolbox/CarbonEventsCore.h>
#include <HIToolbox/Events.h>
#include <CarbonCore/MacErrors.h>
#include <CarbonCore/MacMemory.h>
#include <CarbonCore/Resources.h>
#include <stdlib.h>
#include <string.h>
#include "MenusInternal.h"

enum {
	kMenuResourceType = 'MENU',
	kMenuBarResourceType = 'MBAR',
};

// The contents of the handle of a menu list: the menus it holds a reference to
struct MenuList
{
	UInt32 count;
	MenuRef menus[1];
};

static CFStringRef copyString(ConstStr255Param string)
{
	if (!string)
		return CFStringCreateWithCString(NULL, "", kCFStringEncodingUTF8);
	return CFStringCreateWithPascalString(NULL, string, kCFStringEncodingMacRoman);
}

static void pascalFromString(CFStringRef string, Str255 out)
{
	if (!CFStringGetPascalString(string, out, 256, kCFStringEncodingMacRoman))
		out[0] = 0;
}

MenuRef NewMenu(MenuID menuID, ConstStr255Param menuTitle)
{
	MenuRef menu = NULL;
	CFStringRef title = copyString(menuTitle);

	CreateNewMenu(menuID, 0, &menu);
	if (menu)
		SetMenuTitleWithCFString(menu, title);
	CFRelease(title);
	return menu;
}

void DisposeMenu(MenuRef theMenu)
{
	// A menu is disposed of by dropping the reference the creator holds
	if (theMenu)
		CFRelease((CFTypeRef) theMenu);
}

void SetMenuID(MenuRef menu, MenuID menuID)
{
	if (_HIIsMenu(menu))
	{
		// The menu may be in the list of the inserted menus, which finds menus by ID: the ID is
		// the menu's own
		_HILockMenus();
		_HISetMenuID(menu, menuID);
		_HIUnlockMenus();
	}
}

void SetMenuWidth(MenuRef menu, SInt16 width)
{
	if (_HIIsMenu(menu))
		_HISetMenuSize(menu, width, -1);
}

void SetMenuHeight(MenuRef menu, SInt16 height)
{
	if (_HIIsMenu(menu))
		_HISetMenuSize(menu, -1, height);
}

void GetMenuTitle(MenuRef menu, Str255 title)
{
	CFStringRef text = NULL;

	title[0] = 0;
	if (CopyMenuTitleAsCFString(menu, &text) == noErr && text)
	{
		pascalFromString(text, title);
		CFRelease(text);
	}
}

void SetMenuTitle(MenuRef menu, ConstStr255Param title)
{
	CFStringRef text = copyString(title);

	SetMenuTitleWithCFString(menu, text);
	CFRelease(text);
}

// The item strings of AppendMenu() and InsertMenuItem(): items are separated by a semicolon or a
// return; after the text of an item may come the meta characters ^n (an icon), !c (the mark
// character c), <X (a style: B, I, U, O or S), /c (the command key c) and (, as the first
// character, disables the item. An item with the text "-" is a separator.
struct ParsedItem
{
	CFStringRef text;
	bool disabled;
	bool separator;
	UInt16 mark;
	UInt8 style;
	UInt16 commandKey;
};

static ItemCount parseItemStrings(ConstStr255Param data, struct ParsedItem* items, ItemCount capacity)
{
	if (!data)
		return 0;

	const unsigned char* bytes = data + 1;
	size_t length = data[0], position = 0;
	ItemCount count = 0;

	while (position <= length && count < capacity)
	{
		struct ParsedItem item;
		char text[256];
		size_t textLength = 0;
		bool started = true;

		memset(&item, 0, sizeof(item));
		while (position < length && bytes[position] != ';' && bytes[position] != '\r')
		{
			unsigned char c = bytes[position];

			if (c == '(' && started && textLength == 0)
			{
				item.disabled = true;
				position++;
				started = false;
				continue;
			}
			started = false;
			if (c == '^' || c == '!' || c == '<' || c == '/')
			{
				// A meta character takes the character after it
				if (position + 1 >= length)
					break;
				unsigned char argument = bytes[position + 1];

				switch (c)
				{
					case '!': item.mark = argument; break;
					case '/': item.commandKey = argument; break;
					case '<':
						switch (argument)
						{
							case 'B': item.style |= 1; break;
							case 'I': item.style |= 2; break;
							case 'U': item.style |= 4; break;
							case 'O': item.style |= 8; break;
							case 'S': item.style |= 16; break;
							default: break;
						}
						break;
					default: break;   // an icon is not shown
				}
				position += 2;
				continue;
			}
			text[textLength++] = (char) c;
			position++;
		}
		item.text = CFStringCreateWithBytes(NULL, (const UInt8*) text, textLength, kCFStringEncodingMacRoman, false);
		item.separator = textLength == 1 && text[0] == '-';
		items[count++] = item;
		position++;   // the separator character
		if (position > length)
			break;
	}
	return count;
}

static void insertParsed(MenuRef menu, const struct ParsedItem* parsed, ItemCount afterItem)
{
	MenuItemAttributes attributes = 0;

	if (parsed->separator)
		attributes |= kMenuItemAttrSeparator;
	if (parsed->disabled)
		attributes |= kMenuItemAttrDisabled;
	InsertMenuItemTextWithCFString(menu, parsed->text, (MenuItemIndex) afterItem, attributes, 0);
	if (afterItem >= CountMenuItems(menu))
		afterItem = CountMenuItems(menu) - 1;
	MenuItemIndex index = (MenuItemIndex) (afterItem + 1);

	if (parsed->mark)
		SetItemMark(menu, index, parsed->mark);
	if (parsed->style)
		SetItemStyle(menu, index, parsed->style);
	if (parsed->commandKey)
		SetMenuItemCommandKey(menu, index, false, parsed->commandKey);
}

void AppendMenu(MenuRef menu, ConstStr255Param data)
{
	struct ParsedItem items[64];
	ItemCount count = parseItemStrings(data, items, 64);

	for (ItemCount i = 0; i < count; i++)
	{
		insertParsed(menu, &items[i], CountMenuItems(menu));
		CFRelease(items[i].text);
	}
}

void InsertMenuItem(MenuRef theMenu, ConstStr255Param itemString, MenuItemIndex afterItem)
{
	struct ParsedItem items[64];
	ItemCount count = parseItemStrings(itemString, items, 64);
	ItemCount after = afterItem > CountMenuItems(theMenu) ? CountMenuItems(theMenu) : afterItem;

	// QUIRK: the items of the string are inserted one after the other right after the item, so they end up
	// in the reverse order: "A;B" after item 1 gives item 1, B, A. Not documented. Evidence: black-box
	// observation on macOS 10.14.4 (i386) (tier 2).
	// Test: test_quirk_insert_menu_item_reverses_items in tests/src/carbon_menus.c
	for (ItemCount i = 0; i < count; i++)
	{
		insertParsed(theMenu, &items[i], after);
		CFRelease(items[i].text);
	}
}

OSStatus AppendMenuItemText(MenuRef menu, ConstStr255Param inString)
{
	CFStringRef text = copyString(inString);
	OSStatus status = AppendMenuItemTextWithCFString(menu, text, 0, 0, NULL);

	CFRelease(text);
	return status;
}

OSStatus InsertMenuItemText(MenuRef menu, ConstStr255Param inString, MenuItemIndex afterItem)
{
	CFStringRef text = copyString(inString);
	OSStatus status = InsertMenuItemTextWithCFString(menu, text, afterItem, 0, 0);

	CFRelease(text);
	return status;
}

void SetMenuItemText(MenuRef theMenu, MenuItemIndex item, ConstStr255Param itemString)
{
	CFStringRef text = copyString(itemString);

	SetMenuItemTextWithCFString(theMenu, item, text);
	CFRelease(text);
}

void GetMenuItemText(MenuRef theMenu, MenuItemIndex item, Str255 itemString)
{
	CFStringRef text = NULL;

	itemString[0] = 0;
	if (CopyMenuItemTextAsCFString(theMenu, item, &text) == noErr && text)
	{
		pascalFromString(text, itemString);
		CFRelease(text);
	}
}

void SetItemCmd(MenuRef theMenu, MenuItemIndex item, CharParameter cmdChar)
{
	SetMenuItemCommandKey(theMenu, item, false, (UInt16) (cmdChar & 0xff));
}

void GetItemCmd(MenuRef theMenu, MenuItemIndex item, CharParameter* cmdChar)
{
	UInt16 key = 0;

	if (cmdChar && GetMenuItemCommandKey(theMenu, item, false, &key) == noErr)
		*cmdChar = (CharParameter) key;
}

UInt32 MenuKey(CharParameter ch)
{
	struct EventRecord event;

	memset(&event, 0, sizeof(event));
	event.what = keyDown;
	event.message = (unsigned long) (ch & charCodeMask);
	event.modifiers = cmdKey;

	UInt32 choice = MenuEvent(&event);

	// The title of the menu is highlighted until the application takes the highlight off
	if (choice)
		HiliteMenu((MenuID) (choice >> 16));
	return choice;
}

OSStatus ContextualMenuSelect(MenuRef inMenu, Point inGlobalLocation, Boolean inReserved, UInt32 inHelpType, ConstStr255Param inHelpItemString, const void* inSelection, UInt32* outUserSelectionType, SInt16* outMenuID, UInt16* outMenuItem)
{
	enum { kCMNothingSelected = 1, kCMMenuItemSelected = 2 };

	if (!_HIIsMenu(inMenu))
		return paramErr;

	UInt32 choice = PopUpMenuSelect(inMenu, inGlobalLocation.v, inGlobalLocation.h, 0);

	if (outUserSelectionType)
		*outUserSelectionType = choice ? kCMMenuItemSelected : kCMNothingSelected;
	if (outMenuID)
		*outMenuID = choice ? (SInt16) (choice >> 16) : 0;
	if (outMenuItem)
		*outMenuItem = choice ? (UInt16) (choice & 0xffff) : 0;
	return noErr;
}

// ---------------------------------------------------------------------------------------
// Menu bars made of resources
// ---------------------------------------------------------------------------------------

static bool readInt16(const UInt8* data, size_t size, size_t offset, SInt16* out)
{
	if (offset + 2 > size)
		return false;
	*out = (SInt16) ((data[offset] << 8) | data[offset + 1]);
	return true;
}

// A 'MENU' resource: the menu ID, the width and height, the definition procedure (ID and a filler),
// a 32-bit mask of the enabled items (bit 0 is the menu, bit n the item n), the title, then for
// each item its text, an icon number, the command key, the mark and the style; an empty text ends
// the list.
static MenuRef menuFromResourceData(const UInt8* data, size_t size)
{
	SInt16 id, width, height;
	size_t position = 14;

	if (!readInt16(data, size, 0, &id) || !readInt16(data, size, 2, &width) || !readInt16(data, size, 4, &height) || size < 15)
		return NULL;

	UInt32 enableFlags = ((UInt32) data[10] << 24) | ((UInt32) data[11] << 16) | ((UInt32) data[12] << 8) | data[13];
	size_t titleLength = data[position];

	if (position + 1 + titleLength > size)
		return NULL;

	Str255 title;

	memcpy(title, data + position, titleLength + 1);
	MenuRef menu = NewMenu(id, title);

	if (!menu)
		return NULL;
	position += 1 + titleLength;
	if (!(enableFlags & 1))
		DisableMenuItem(menu, 0);
	for (MenuItemIndex item = 1; position < size && data[position] != 0; item++)
	{
		size_t length = data[position];

		if (position + 1 + length + 4 > size)
			break;

		Str255 text;
		UInt8 key, mark, style;

		memcpy(text, data + position, length + 1);
		key = data[position + 1 + length + 1];
		mark = data[position + 1 + length + 2];
		style = data[position + 1 + length + 3];
		position += 1 + length + 4;

		CFStringRef string = copyString(text);
		bool separator = length == 1 && text[1] == '-';

		AppendMenuItemTextWithCFString(menu, string, separator ? kMenuItemAttrSeparator : 0, 0, NULL);
		CFRelease(string);
		if (key)
			SetMenuItemCommandKey(menu, item, false, key);
		if (mark)
			SetItemMark(menu, item, mark);
		if (style)
			SetItemStyle(menu, item, style);
		if (!separator && item < 32 && !(enableFlags & (1u << item)))
			DisableMenuItem(menu, item);
	}
	return menu;
}

MenuRef GetMenu(SInt16 resourceID)
{
	Handle resource = GetResource(kMenuResourceType, resourceID);

	if (!resource)
		return NULL;

	long size = GetHandleSize(resource);
	MenuRef menu = size > 0 ? menuFromResourceData((const UInt8*) *resource, (size_t) size) : NULL;

	ReleaseResource(resource);
	return menu;
}

Handle _HINewMenuListHandle(MenuRef* menus, ItemCount count)
{
	Handle list = NewHandle((long) (sizeof(UInt32) + count * sizeof(MenuRef)));

	if (!list)
		return NULL;
	struct MenuList* contents = (struct MenuList*) *list;

	contents->count = (UInt32) count;
	for (ItemCount i = 0; i < count; i++)
		contents->menus[i] = menus[i];
	return list;
}

Handle GetNewMBar(SInt16 menuBarID)
{
	Handle resource = GetResource(kMenuBarResourceType, menuBarID);

	if (!resource)
		return NULL;

	long size = GetHandleSize(resource);
	SInt16 count = 0;
	Handle list = NULL;

	if (size >= 2 && readInt16((const UInt8*) *resource, (size_t) size, 0, &count) && count >= 0)
	{
		MenuRef* menus = calloc((size_t) count + 1, sizeof(MenuRef));
		ItemCount made = 0;

		for (SInt16 i = 0; menus && i < count; i++)
		{
			SInt16 id;

			if (!readInt16((const UInt8*) *resource, (size_t) size, 2 + 2 * (size_t) i, &id))
				break;
			MenuRef menu = GetMenu(id);

			if (menu)
				menus[made++] = menu;
		}
		list = _HINewMenuListHandle(menus, made);
		free(menus);
	}
	ReleaseResource(resource);
	return list;
}

Handle GetMenuBar(void)
{
	MenuRef* menus;
	ItemCount count = _HICopyMenuBarMenus(&menus);
	Handle list = _HINewMenuListHandle(menus, count);

	// The menus' references move to the list
	free(menus);
	return list;
}

void SetMenuBar(Handle menuList)
{
	if (!menuList)
		return;

	struct MenuList* contents = (struct MenuList*) *menuList;
	UInt32 count = contents->count;

	ClearMenuBar();
	for (UInt32 i = 0; i < count; i++)
		InsertMenu(contents->menus[i], 0);
}

#endif
