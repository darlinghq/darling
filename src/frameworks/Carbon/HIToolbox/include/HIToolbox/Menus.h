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

#ifndef _Carbon_Menus_H_
#define _Carbon_Menus_H_

#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/MacTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

// The types other headers need (HICommand in CarbonEventsCore.h refers to MenuRef) come first. The
// event types are repeated, because the event headers include this one.
typedef struct OpaqueMenuRef* MenuRef;
typedef MenuRef MenuHandle;
typedef SInt16 MenuID;
typedef UInt16 MenuItemIndex;
typedef UInt32 MenuCommand;
typedef UInt32 MenuAttributes;
typedef UInt32 MenuItemAttributes;
typedef UInt32 MenuEventOptions;

typedef struct OpaqueEventRef* EventRef;
typedef void* EventTargetRef;
struct EventRecord;

enum {
	kMenuNoModifiers = 0,
	kMenuShiftModifier = 1 << 0,
	kMenuOptionModifier = 1 << 1,
	kMenuControlModifier = 1 << 2,
	kMenuNoCommandModifier = 1 << 3,
};

enum {
	kMenuAttrExcludesMarkColumn = 1 << 0,
	kMenuAttrAutoDisable = 1 << 2,
	kMenuAttrUsePencilGlyph = 1 << 3,
	kMenuAttrHidden = 1 << 4,
	kMenuAttrCondenseSeparators = 1 << 5,
	kMenuAttrDoNotCacheImage = 1 << 6,
	kMenuAttrDoNotUseUserCommandKeys = 1 << 7,
};

enum {
	kMenuItemAttrDisabled = 1 << 0,
	kMenuItemAttrIconDisabled = 1 << 1,
	kMenuItemAttrSubmenuParentChoosable = 1 << 2,
	kMenuItemAttrDynamic = 1 << 3,
	kMenuItemAttrNotPreviousAlternate = 1 << 4,
	kMenuItemAttrHidden = 1 << 5,
	kMenuItemAttrSeparator = 1 << 6,
	kMenuItemAttrSectionHeader = 1 << 7,
	kMenuItemAttrIgnoreMeta = 1 << 8,
	kMenuItemAttrAutoRepeat = 1 << 9,
	kMenuItemAttrUseVirtualKey = 1 << 10,
	kMenuItemAttrCustomDraw = 1 << 11,
	kMenuItemAttrIncludeInCmdKeyMatching = 1 << 12,
};

// Glyphs of the key equivalent column (SetMenuItemKeyGlyph)
enum {
	kMenuNullGlyph = 0x00,
	kMenuTabRightGlyph = 0x02,
	kMenuTabLeftGlyph = 0x03,
	kMenuEnterGlyph = 0x04,
	kMenuShiftGlyph = 0x05,
	kMenuControlGlyph = 0x06,
	kMenuOptionGlyph = 0x07,
	kMenuSpaceGlyph = 0x09,
	kMenuDeleteRightGlyph = 0x0A,
	kMenuReturnGlyph = 0x0B,
	kMenuReturnR2LGlyph = 0x0C,
	kMenuNonmarkingReturnGlyph = 0x0D,
	kMenuPencilGlyph = 0x0F,
	kMenuDownwardArrowDashedGlyph = 0x10,
	kMenuCommandGlyph = 0x11,
	kMenuCheckmarkGlyph = 0x12,
	kMenuDiamondGlyph = 0x13,
	kMenuAppleLogoFilledGlyph = 0x14,
	kMenuParagraphKoreanGlyph = 0x15,
	kMenuDeleteLeftGlyph = 0x17,
	kMenuLeftArrowDashedGlyph = 0x18,
	kMenuUpArrowDashedGlyph = 0x19,
	kMenuRightArrowDashedGlyph = 0x1A,
	kMenuEscapeGlyph = 0x1B,
	kMenuClearGlyph = 0x1C,
	kMenuBlankGlyph = 0x61,
	kMenuPageUpGlyph = 0x62,
	kMenuCapsLockGlyph = 0x63,
	kMenuLeftArrowGlyph = 0x64,
	kMenuRightArrowGlyph = 0x65,
	kMenuNorthwestArrowGlyph = 0x66,
	kMenuHelpGlyph = 0x67,
	kMenuUpArrowGlyph = 0x68,
	kMenuSoutheastArrowGlyph = 0x69,
	kMenuDownArrowGlyph = 0x6A,
	kMenuPageDownGlyph = 0x6B,
	kMenuContextualMenuGlyph = 0x6D,
	kMenuPowerGlyph = 0x6E,
	kMenuF1Glyph = 0x6F,
	kMenuF2Glyph = 0x70,
	kMenuF3Glyph = 0x71,
	kMenuF4Glyph = 0x72,
	kMenuF5Glyph = 0x73,
	kMenuF6Glyph = 0x74,
	kMenuF7Glyph = 0x75,
	kMenuF8Glyph = 0x76,
	kMenuF9Glyph = 0x77,
	kMenuF10Glyph = 0x78,
	kMenuF11Glyph = 0x79,
	kMenuF12Glyph = 0x7A,
	kMenuF13Glyph = 0x87,
	kMenuF14Glyph = 0x88,
	kMenuF15Glyph = 0x89,
};

// InsertMenu: the menu is only a submenu of other menus, not a menu bar menu
enum {
	kInsertHierarchicalMenu = -1,
};

// The marks of CheckMenuItem and SetItemMark
enum {
	noMark = 0,
	commandMark = 0x11,
	checkMark = 0x12,
	diamondMark = 0x13,
	appleMark = 0x14,
};

// The options of IsMenuKeyEvent
enum {
	kMenuEventIncludeDisabledItems = 0x0001,
	kMenuEventQueryOnly = 0x0002,
	kMenuEventDontCheckSubmenus = 0x0004,
};

// The reason a menu is dismissed (CancelMenuTracking)
enum {
	kHIMenuDismissedBySelection = 1,
	kHIMenuDismissedByUserCancel = 2,
	kHIMenuDismissedByMouseDown = 3,
	kHIMenuDismissedByMouseUp = 4,
	kHIMenuDismissedByKeyEvent = 5,
	kHIMenuDismissedByAppSwitch = 6,
	kHIMenuDismissedByTimeout = 7,
	kHIMenuDismissedByCancelMenuTracking = 8,
	kHIMenuDismissedByActivationChange = 9,
	kHIMenuDismissedByFullscreenTransition = 10,
};

// GetMenuType
enum {
	kMenuTypeHierarchical = 1,
	kMenuTypePullDown = 2,
	kMenuTypePopUp = 3,
	kMenuTypeBar = 4,
	kMenuTypeInvalid = 5,
};

// Menu objects. A MenuRef is a Core Foundation object: CFRetain and CFRelease keep it, and the
// menu bar and the menus that have it as a submenu keep it as well.
OSStatus CreateNewMenu(MenuID inMenuID, MenuAttributes inMenuAttributes, MenuRef* outMenuRef);

enum {
	kWindowMenuIncludeRotate = 1 << 0
};
OSStatus CreateStandardWindowMenu(OptionBits inOptions, MenuRef* outMenu);
MenuRef GetMenuHandle(MenuID menuID);
void InsertMenu(MenuRef theMenu, MenuID beforeID);
void DeleteMenu(MenuID menuID);
SInt16 GetMenuID(MenuRef menu);
UInt16 CountMenuItems(MenuRef theMenu);
OSStatus ChangeMenuAttributes(MenuRef menu, MenuAttributes setTheseAttributes, MenuAttributes clearTheseAttributes);
OSStatus GetMenuAttributes(MenuRef menu, MenuAttributes* outAttributes);
UInt16 GetMenuWidth(MenuRef menu);
UInt16 GetMenuHeight(MenuRef menu);
UInt16 GetMenuType(MenuRef theMenu);
void CalcMenuSize(MenuRef theMenu);
OSStatus SetMenuTitleWithCFString(MenuRef inMenu, CFStringRef inString);
OSStatus CopyMenuTitleAsCFString(MenuRef inMenu, CFStringRef* outString);
OSStatus SetMenuTitleIcon(MenuRef inMenu, UInt32 inType, void* inIcon);

// The menu bar (the root menu is the menu whose items are the titles of the menu bar menus)
void ClearMenuBar(void);
void DrawMenuBar(void);
void InvalMenuBar(void);
MenuRef AcquireRootMenu(void);
OSStatus SetRootMenu(MenuRef inMenu);
SInt16 GetMBarHeight(void);
void HideMenuBar(void);
void ShowMenuBar(void);
Boolean IsMenuBarVisible(void);
void FlashMenuBar(MenuID menuID);

// Menu items
OSStatus AppendMenuItemTextWithCFString(MenuRef inMenu, CFStringRef inString, MenuItemAttributes inAttributes, MenuCommand inCommandID, MenuItemIndex* outNewItem);
OSStatus InsertMenuItemTextWithCFString(MenuRef inMenu, CFStringRef inString, MenuItemIndex inAfterItem, MenuItemAttributes inAttributes, MenuCommand inCommandID);
OSStatus SetMenuItemTextWithCFString(MenuRef inMenu, MenuItemIndex inItem, CFStringRef inString);
OSStatus CopyMenuItemTextAsCFString(MenuRef inMenu, MenuItemIndex inItem, CFStringRef* outString);
void DeleteMenuItem(MenuRef theMenu, MenuItemIndex item);
OSStatus DeleteMenuItems(MenuRef inMenu, MenuItemIndex inFirstItem, ItemCount inNumItems);
void EnableMenuItem(MenuRef theMenu, MenuItemIndex item);
void DisableMenuItem(MenuRef theMenu, MenuItemIndex item);
Boolean IsMenuItemEnabled(MenuRef menu, MenuItemIndex item);
void DisableMenuCommand(MenuRef inMenu, MenuCommand inCommandID);
OSStatus SetMenuItemCommandID(MenuRef inMenu, MenuItemIndex inItem, MenuCommand inCommandID);
OSStatus GetMenuItemCommandID(MenuRef inMenu, MenuItemIndex inItem, MenuCommand* outCommandID);
OSStatus GetIndMenuItemWithCommandID(MenuRef inMenu, MenuCommand inCommandID, UInt32 inItemIndex, MenuRef* outMenu, MenuItemIndex* outIndex);
OSStatus SetMenuItemHierarchicalMenu(MenuRef inMenu, MenuItemIndex inItem, MenuRef inHierMenu);
OSStatus GetMenuItemHierarchicalMenu(MenuRef inMenu, MenuItemIndex inItem, MenuRef* outHierMenu);
OSStatus SetMenuItemKeyGlyph(MenuRef inMenu, MenuItemIndex inItem, SInt16 inGlyph);
OSStatus SetMenuItemModifiers(MenuRef inMenu, MenuItemIndex inItem, UInt8 inModifiers);
OSStatus GetMenuItemModifiers(MenuRef inMenu, MenuItemIndex inItem, UInt8* outModifiers);
OSStatus SetMenuItemCommandKey(MenuRef inMenu, MenuItemIndex inItem, Boolean inSetVirtualKey, UInt16 inKey);
OSStatus GetMenuItemCommandKey(MenuRef inMenu, MenuItemIndex inItem, Boolean inGetVirtualKey, UInt16* outKey);
void SetItemMark(MenuRef theMenu, MenuItemIndex item, CharParameter markChar);
void GetItemMark(MenuRef theMenu, MenuItemIndex item, CharParameter* markChar);
void CheckMenuItem(MenuRef theMenu, MenuItemIndex item, Boolean checked);
OSStatus SetMenuItemRefCon(MenuRef inMenu, MenuItemIndex inItem, UInt32 inRefCon);
OSStatus GetMenuItemRefCon(MenuRef inMenu, MenuItemIndex inItem, UInt32* outRefCon);
OSStatus SetMenuItemProperty(MenuRef menu, MenuItemIndex item, OSType propertyCreator, OSType propertyTag, ByteCount propertySize, const void* propertyData);
OSStatus GetMenuItemProperty(MenuRef menu, MenuItemIndex item, OSType propertyCreator, OSType propertyTag, ByteCount bufferSize, ByteCount* actualSize, void* propertyBuffer);
OSStatus RemoveMenuItemProperty(MenuRef menu, MenuItemIndex item, OSType propertyCreator, OSType propertyTag);
OSStatus ChangeMenuItemAttributes(MenuRef menu, MenuItemIndex item, MenuItemAttributes setTheseAttributes, MenuItemAttributes clearTheseAttributes);
OSStatus GetMenuItemAttributes(MenuRef menu, MenuItemIndex item, MenuItemAttributes* outAttributes);
OSStatus SetMenuItemIndent(MenuRef inMenu, MenuItemIndex inItem, UInt32 inIndent);
void SetItemStyle(MenuRef theMenu, MenuItemIndex item, StyleParameter chStyle);

// Tracking and key equivalents. The choice returned by MenuSelect, MenuEvent and PopUpMenuSelect
// has the menu ID in the high word and the item in the low word, zero when nothing was chosen.
UInt32 MenuSelect(Point startPt);
UInt32 MenuEvent(const struct EventRecord* inEvent);
Boolean IsMenuKeyEvent(MenuRef inStartMenu, EventRef inEvent, MenuEventOptions inOptions, MenuRef* outMenu, MenuItemIndex* outMenuItem);
UInt32 PopUpMenuSelect(MenuRef menu, SInt16 top, SInt16 left, SInt16 popUpItem);
void HiliteMenu(MenuID menuID);
OSStatus CancelMenuTracking(MenuRef inRootMenu, Boolean inImmediate, UInt32 inDismissalReason);
EventTargetRef GetMenuEventTarget(MenuRef menu);

#if !__LP64__
// The calls that exist in the 32-bit Menu Manager only: Pascal strings, MENU and MBAR resources,
// handles
MenuRef NewMenu(MenuID menuID, ConstStr255Param menuTitle);
MenuRef GetMenu(SInt16 resourceID);
void DisposeMenu(MenuRef theMenu);
Handle GetMenuBar(void);
void SetMenuBar(Handle menuList);
Handle GetNewMBar(SInt16 menuBarID);
void SetMenuID(MenuRef menu, MenuID menuID);
void SetMenuWidth(MenuRef menu, SInt16 width);
void SetMenuHeight(MenuRef menu, SInt16 height);
void GetMenuTitle(MenuRef menu, Str255 title);
void SetMenuTitle(MenuRef menu, ConstStr255Param title);
void AppendMenu(MenuRef menu, ConstStr255Param data);
OSStatus AppendMenuItemText(MenuRef menu, ConstStr255Param inString);
void InsertMenuItem(MenuRef theMenu, ConstStr255Param itemString, MenuItemIndex afterItem);
OSStatus InsertMenuItemText(MenuRef menu, ConstStr255Param inString, MenuItemIndex afterItem);
void SetMenuItemText(MenuRef theMenu, MenuItemIndex item, ConstStr255Param itemString);
void GetMenuItemText(MenuRef theMenu, MenuItemIndex item, Str255 itemString);
void SetItemCmd(MenuRef theMenu, MenuItemIndex item, CharParameter cmdChar);
void GetItemCmd(MenuRef theMenu, MenuItemIndex item, CharParameter* cmdChar);
void GetItemStyle(MenuRef menu, MenuItemIndex item, Style* chStyle);
OSStatus GetMenuItemKeyGlyph(MenuRef inMenu, MenuItemIndex inItem, SInt16* outGlyph);
OSStatus SetMenuItemHierarchicalID(MenuRef inMenu, MenuItemIndex inItem, MenuID inHierID);
OSStatus GetMenuItemHierarchicalID(MenuRef inMenu, MenuItemIndex inItem, MenuID* outHierID);
OSStatus GetMenuItemPropertySize(MenuRef menu, MenuItemIndex item, OSType propertyCreator, OSType propertyTag, ByteCount* size);
OSStatus GetMenuItemIndent(MenuRef inMenu, MenuItemIndex inItem, UInt32* outIndent);
OSStatus SetMenuItemFontID(MenuRef inMenu, MenuItemIndex inItem, SInt16 inFontID);
OSStatus GetMenuItemFontID(MenuRef inMenu, MenuItemIndex inItem, SInt16* outFontID);
UInt32 MenuKey(CharParameter ch);
UInt32 CountMenuItemsWithCommandID(MenuRef inMenu, MenuCommand inCommandID);
Boolean MenuHasEnabledItems(MenuRef menu);
void EnableMenuCommand(MenuRef inMenu, MenuCommand inCommandID);
Boolean IsMenuCommandEnabled(MenuRef inMenu, MenuCommand inCommandID);
OSStatus RetainMenu(MenuRef inMenu);
OSStatus ReleaseMenu(MenuRef inMenu);
ItemCount GetMenuRetainCount(MenuRef inMenu);
OSStatus ContextualMenuSelect(MenuRef inMenu, Point inGlobalLocation, Boolean inReserved, UInt32 inHelpType, ConstStr255Param inHelpItemString, const void* inSelection, UInt32* outUserSelectionType, SInt16* outMenuID, UInt16* outMenuItem);
#endif

#ifdef __cplusplus
}
#endif

#endif
