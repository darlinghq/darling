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

#ifndef HITOOLBOX_MENUS_INTERNAL_H
#define HITOOLBOX_MENUS_INTERNAL_H

// The interface between the Menu Manager objects (Menus.c) and the code that shows and tracks them
// (MenuTracking.c). Not installed.

#include <HIToolbox/Menus.h>
#include <HIToolbox/CarbonEventsCore.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct HIMenuItemInfo
{
	CFStringRef title;
	MenuCommand command;
	MenuItemAttributes attributes;
	UInt8 modifiers;
	UInt16 commandKey;
	SInt16 glyph;
	UInt16 mark;
	MenuRef submenu;
	UInt32 indent;
	UInt8 style;
};

CFTypeID _HIMenuGetTypeID(void);
bool _HIIsMenu(MenuRef menu);
void _HILockMenus(void);
void _HIUnlockMenus(void);
ItemCount _HIMenuItemCount(MenuRef menu);
void _HIMenuGetItem(MenuRef menu, MenuItemIndex index, struct HIMenuItemInfo* info);
CFStringRef _HIMenuTitle(MenuRef menu);
Boolean _HIMenuIsEnabled(MenuRef menu);
MenuAttributes _HIMenuAttributes(MenuRef menu);
void _HISetMenuID(MenuRef menu, MenuID menuID);
void _HISetMenuSize(MenuRef menu, SInt16 width, SInt16 height);
bool _HIMenuContains(MenuRef menu, MenuRef inner);
MenuID _HIHilitedMenuID(void);

// The menu bar menus, retained; the caller releases every one and frees the array
ItemCount _HICopyMenuBarMenus(MenuRef** outMenus);

// The menu events and the command of an item
OSStatus _HISendMenuEvent(UInt32 kind, MenuRef menu, MenuItemIndex item, bool forKeyEvent, bool firstOpen);
void _HIUpdateMenuItems(MenuRef menu, bool forKeyEvent);
OSStatus _HIProcessMenuCommand(MenuRef menu, MenuItemIndex item);
OSStatus _HIHandleKeyEquivalent(EventRef event);

// The handle of a menu list that holds the menus (their references move to it)
Handle _HINewMenuListHandle(MenuRef* menus, ItemCount count);

// Called when the menu bar list or a menu changed: what is on screen is brought up to date by the
// next DrawMenuBar(), as in the Menu Manager of old
void _HIMenuBarChanged(void);
// Brings the menu bar window, if one is shown, up to date (the visibility or the highlight changed)
void _HIRedrawMenuBar(void);
OSStatus _HIHandleMenuBarMouseDown(EventRef event);
void _HIMenuChanged(MenuRef menu);
// The menu is a Window menu: the list of the application's windows is kept at the end of it (or after the
// list markers of the standard Window menu), made when the menu opens
void _HIMenuMakeWindowMenu(MenuRef menu);

#ifdef __cplusplus
}
#endif

#endif
