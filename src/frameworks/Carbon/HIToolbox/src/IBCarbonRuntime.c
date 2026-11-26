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

// The Interface Builder Carbon runtime: nib references, and the menus and the menu bar of a nib.
// Reading the nib is in IBCarbonNib.c; windows are in IBCarbonWindows.c.

#include <HIToolbox/IBCarbonRuntime.h>
#include <HIToolbox/Events.h>
#include <CarbonCore/MacErrors.h>
#include <CarbonCore/MacMemory.h>
#include <stdlib.h>
#include <string.h>
#include "IBCarbonNib.h"
#include "MenusInternal.h"

static CFStringRef copyNibPath(CFBundleRef bundle, CFStringRef name)
{
	// The name is given without the extension (with it, the nib is not found)
	CFURLRef url = CFBundleCopyResourceURL(bundle, name, CFSTR("nib"), NULL);

	if (!url)
		return NULL;

	CFURLRef objects = CFURLCreateCopyAppendingPathComponent(NULL, url, CFSTR("objects.xib"), false);
	CFStringRef path = CFURLCopyFileSystemPath(objects, kCFURLPOSIXPathStyle);

	CFRelease(objects);
	CFRelease(url);
	return path;
}

OSStatus CreateNibReferenceWithCFBundle(CFBundleRef inBundle, CFStringRef inNibName, IBNibRef* outNibRef)
{
	if (!inBundle || !inNibName || !outNibRef)
		return paramErr;
	*outNibRef = NULL;

	CFStringRef path = copyNibPath(inBundle, inNibName);

	if (!path)
		return kIBCarbonRuntimeCantFindNibFile;

	IBNibRef nib;
	OSStatus status = _IBCarbonNibOpen(path, &nib);

	CFRelease(path);
	if (status != noErr)
		return status;
	// The nib is only opened: whether it can be read shows when an object is asked for
	if (!_IBCarbonNibExists(nib))
	{
		_IBCarbonNibClose(nib);
		return kIBCarbonRuntimeCantFindNibFile;
	}
	*outNibRef = nib;
	return noErr;
}

OSStatus CreateNibReference(CFStringRef inNibName, IBNibRef* outNibRef)
{
	if (!outNibRef)
		return paramErr;
	return CreateNibReferenceWithCFBundle(CFBundleGetMainBundle(), inNibName, outNibRef);
}

void DisposeNibReference(IBNibRef inNibRef)
{
	_IBCarbonNibClose(inNibRef);
}

// ---------------------------------------------------------------------------------------
// Menus
// ---------------------------------------------------------------------------------------

static bool isClass(CFDictionaryRef object, CFStringRef className)
{
	CFStringRef value = CFDictionaryGetValue(object, CFSTR("class"));

	return value && CFStringCompare(value, className, 0) == kCFCompareEqualTo;
}

static SInt64 numberOf(CFDictionaryRef object, CFStringRef key, SInt64 fallback)
{
	CFNumberRef number = CFDictionaryGetValue(object, key);
	SInt64 value;

	if (number && CFGetTypeID(number) == CFNumberGetTypeID() && CFNumberGetValue(number, kCFNumberSInt64Type, &value))
		return value;
	return fallback;
}

static bool flagOf(CFDictionaryRef object, CFStringRef key)
{
	CFBooleanRef value = CFDictionaryGetValue(object, key);

	return value && CFGetTypeID(value) == CFBooleanGetTypeID() && CFBooleanGetValue(value);
}

static MenuRef createMenuFromDescription(CFDictionaryRef description, int depth);

static void addItemFromDescription(MenuRef menu, CFDictionaryRef item, int depth)
{
	MenuItemAttributes attributes = 0;
	CFStringRef title = CFDictionaryGetValue(item, CFSTR("title"));
	SInt64 command = numberOf(item, CFSTR("command"), 0);
	MenuItemIndex index = 0;

	if (flagOf(item, CFSTR("separator")))
		attributes |= kMenuItemAttrSeparator;
	if (flagOf(item, CFSTR("disabled")))
		attributes |= kMenuItemAttrDisabled;
	if (flagOf(item, CFSTR("dynamic")))
		attributes |= kMenuItemAttrDynamic;
	if (flagOf(item, CFSTR("hidden")))
		attributes |= kMenuItemAttrHidden;
	if (AppendMenuItemTextWithCFString(menu, title ? title : CFSTR(""), attributes, (MenuCommand) command, &index) != noErr || !index)
		return;

	// The key equivalent is the first character of the string. The modifiers are the flags of an
	// NSEvent: the Command flag is part of them, and an item with modifiers but no Command is one
	// that has no Command key
	CFStringRef key = CFDictionaryGetValue(item, CFSTR("keyEquivalent"));

	if (key && CFStringGetLength(key) > 0 && !(attributes & kMenuItemAttrSeparator))
	{
		UniChar character = CFStringGetCharacterAtIndex(key, 0);
		UInt8 modifiers = 0;

		// QUIRK: the key of an item made from a nib is the capital letter, whatever case the nib has. Not
		// documented. Evidence: black-box observation on macOS 10.14.4 (i386 and x86_64) and 26.7.3
		// (tier 2): GetMenuItemCommandKey() of items whose keyEquivalent is n, w, h and r gives
		// N, W, H and R (and S for S).
		// Test: test_nib in tests/src/carbon_menus_nib.c (the keys of the items)
		if (character >= 'a' && character <= 'z')
			character = (UniChar) (character - 'a' + 'A');
		if (character < 0x100)
			SetMenuItemCommandKey(menu, index, false, character);
		if (CFDictionaryContainsKey(item, CFSTR("keyEquivalentModifier")))
		{
			SInt64 flags = numberOf(item, CFSTR("keyEquivalentModifier"), 0);

			if (flags & 0x20000)
				modifiers |= kMenuShiftModifier;
			if (flags & 0x80000)
				modifiers |= kMenuOptionModifier;
			if (flags & 0x40000)
				modifiers |= kMenuControlModifier;
			if (!(flags & 0x100000) && flags != 0)
				modifiers |= kMenuNoCommandModifier;
			SetMenuItemModifiers(menu, index, modifiers);
		}
	}
	if (flagOf(item, CFSTR("checked")))
		CheckMenuItem(menu, index, true);

	CFDictionaryRef submenu = CFDictionaryGetValue(item, CFSTR("submenu"));

	if (submenu && CFGetTypeID(submenu) == CFDictionaryGetTypeID() && depth < 12)
	{
		MenuRef sub = createMenuFromDescription(submenu, depth + 1);

		if (sub)
		{
			SetMenuItemHierarchicalMenu(menu, index, sub);
			CFRelease((CFTypeRef) sub);
		}
	}
}

static MenuRef createMenuFromDescription(CFDictionaryRef description, int depth)
{
	MenuRef menu = NULL;
	CFStringRef title = CFDictionaryGetValue(description, CFSTR("title"));
	SInt64 id = numberOf(description, CFSTR("menuID"), 0);

	// A menu without an ID of its own has the ID 0
	CFTypeRef menuName = CFDictionaryGetValue(description, CFSTR("name"));

	if (CreateNewMenu((MenuID) id, 0, &menu) != noErr)
		return NULL;
	if (title)
		SetMenuTitleWithCFString(menu, title);

	// The Windows menu of the nib (named so by Interface Builder) gets the window list
	if (menuName && CFGetTypeID(menuName) == CFStringGetTypeID() && CFEqual(menuName, CFSTR("_NSWindowsMenu")))
		_HIMenuMakeWindowMenu(menu);

	CFArrayRef items = CFDictionaryGetValue(description, CFSTR("items"));

	for (CFIndex i = 0; items && i < CFArrayGetCount(items); i++)
	{
		CFDictionaryRef item = CFArrayGetValueAtIndex(items, i);

		if (CFGetTypeID(item) == CFDictionaryGetTypeID() && isClass(item, CFSTR("IBCarbonMenuItem")))
			addItemFromDescription(menu, item, depth);
	}
	return menu;
}

OSStatus _HICreateMenuFromNibDescription(CFDictionaryRef description, MenuRef* outMenu)
{
	if (!description || !outMenu || !isClass(description, CFSTR("IBCarbonMenu")))
		return paramErr;
	*outMenu = createMenuFromDescription(description, 0);
	return *outMenu ? noErr : memFullErr;
}

static OSStatus copyMenuDescription(IBNibRef inNibRef, CFStringRef inName, CFDictionaryRef* outDescription)
{
	if (!inNibRef || !inName)
		return paramErr;

	CFDictionaryRef description = _IBCarbonNibCopyObjectNamed(inNibRef, inName);

	if (!description)
		return kIBCarbonRuntimeCantFindObject;
	if (!isClass(description, CFSTR("IBCarbonMenu")))
	{
		CFRelease(description);
		return kIBCarbonRuntimeObjectNotOfRequestedType;
	}
	*outDescription = description;
	return noErr;
}

OSStatus CreateMenuFromNib(IBNibRef inNibRef, CFStringRef inName, MenuRef* outMenuRef)
{
	CFDictionaryRef description;

	if (!outMenuRef)
		return paramErr;
	*outMenuRef = NULL;

	OSStatus status = copyMenuDescription(inNibRef, inName, &description);

	if (status != noErr)
		return status;
	*outMenuRef = createMenuFromDescription(description, 0);
	CFRelease(description);
	return *outMenuRef ? noErr : memFullErr;
}

OSStatus _HICreateMenuBarMenusFromNib(IBNibRef nib, CFStringRef name, MenuRef** outMenus, ItemCount* outCount)
{
	CFDictionaryRef description;
	OSStatus status = copyMenuDescription(nib, name, &description);

	if (status != noErr)
		return status;

	// The menus of the menu bar are the submenus of the items of the bar menu
	CFArrayRef items = CFDictionaryGetValue(description, CFSTR("items"));
	CFIndex count = items ? CFArrayGetCount(items) : 0;
	MenuRef* menus = calloc((size_t) count + 1, sizeof(MenuRef));
	ItemCount made = 0;

	for (CFIndex i = 0; menus && i < count; i++)
	{
		CFDictionaryRef item = CFArrayGetValueAtIndex(items, i);
		CFDictionaryRef submenu = CFGetTypeID(item) == CFDictionaryGetTypeID() ? CFDictionaryGetValue(item, CFSTR("submenu")) : NULL;

		if (submenu && CFGetTypeID(submenu) == CFDictionaryGetTypeID())
		{
			MenuRef menu = createMenuFromDescription(submenu, 1);

			if (menu)
				menus[made++] = menu;
		}
	}
	CFRelease(description);
	if (!menus)
		return memFullErr;
	*outMenus = menus;
	*outCount = made;
	return noErr;
}

OSStatus SetMenuBarFromNib(IBNibRef inNibRef, CFStringRef inName)
{
	MenuRef* menus;
	ItemCount count;
	OSStatus status = _HICreateMenuBarMenusFromNib(inNibRef, inName, &menus, &count);

	if (status != noErr)
		return status;
	ClearMenuBar();
	for (ItemCount i = 0; i < count; i++)
	{
		InsertMenu(menus[i], 0);
		CFRelease((CFTypeRef) menus[i]);
	}
	free(menus);
	DrawMenuBar();
	return noErr;
}

#if !__LP64__
OSStatus CreateMenuBarFromNib(IBNibRef inNibRef, CFStringRef inName, Handle* outMenuBar)
{
	MenuRef* menus;
	ItemCount count;

	if (!outMenuBar)
		return paramErr;
	*outMenuBar = NULL;

	OSStatus status = _HICreateMenuBarMenusFromNib(inNibRef, inName, &menus, &count);

	if (status != noErr)
		return status;
	*outMenuBar = _HINewMenuListHandle(menus, count);
	free(menus);
	return *outMenuBar ? noErr : memFullErr;
}
#endif
