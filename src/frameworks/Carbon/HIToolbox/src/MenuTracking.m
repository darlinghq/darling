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

// Showing and tracking menus: the menu bar window at the top of the main display, the windows of
// the pull-down, pop-up and hierarchical menus, and the tracking loop of MenuSelect and
// PopUpMenuSelect.
//
// Menus are drawn with HITheme into a bitmap and shown in CoreGraphics windows (CGS), so the
// same code runs on every window system backend of CoreGraphics. Where a backend cannot place a
// window (Wayland), the window is a toplevel of the size of the menu that the compositor places;
// tracking works with the global coordinates the CoreGraphics events report, which are
// consistent for the windows of the process.
//
// The menu bar window is the one place the menu bar of a Carbon application is shown. AppKit, when it
// shows the menu inside a window or in a bar of its own, does so without this code.

#include <HIToolbox/Menus.h>
#include <HIToolbox/CarbonEventsCore.h>
#include <HIToolbox/CarbonEvents.h>
#include <HIToolbox/HITheme.h>
#include <HIToolbox/Events.h>
#include <HIToolbox/MacApplication.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreGraphics/CoreGraphicsPrivate.h>
#import <CoreGraphics/CGSNativeConnection.h>
#include <CoreText/CoreText.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLInternal.h>
#include <OpenGL/gl.h>
#include <AE/AEDataModel.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "HIToolboxInternal.h"
#include "HIToolboxPrivate.h"
#include "MenusInternal.h"

// ---------------------------------------------------------------------------------------
// Windows
// ---------------------------------------------------------------------------------------

struct MenuWindow
{
	CGSConnectionID connection;
	CGSWindowID window;
	CGRect frame;           // in global coordinates (origin at the top-left of the main display)
	void* native;
	CGLWindowRef glWindow;
	CGLContextObj gl;
	CGContextRef bitmap;
	void* pixels;
	size_t width, height;
	GLuint texture;
};

static bool g_registered;
// EGL_PLATFORM_WAYLAND_KHR
enum { kEGLPlatformWayland = 0x31D8 };

static void destroyMenuWindow(struct MenuWindow* win)
{
	if (!win)
		return;
	if (win->gl)
	{
		CGLSetCurrentContext(win->gl);
		if (win->texture)
			glDeleteTextures(1, &win->texture);
		CGLSetCurrentContext(NULL);
		CGLReleaseContext(win->gl);
	}
	if (win->glWindow)
		CGLDestroyWindow(win->glWindow);
	if (win->window)
	{
		CGSOrderWindow(win->connection, win->window, kCGSOrderOut, 0);
		CGSReleaseWindow(win->connection, win->window);
	}
	if (win->bitmap)
		CGContextRelease(win->bitmap);
	free(win->pixels);
	free(win);
}

// The window of the menu bar (level) or of a menu. Which window system shows them follows from the
// window level: a window system that lets a client place its windows takes the position as it is (X11, as
// a window that no window manager decorates or places), the others place the window by what it is,
// the menu bar along the top of the screen and a menu next to it (see -[CGSWindowWayland role]).
static struct MenuWindow* createMenuWindow(CGRect frame, CGWindowLevel level)
{
	CGSConnectionID connection = CGSMainConnectionID();

	if (connection == 0 || _CGSNativeDisplay(connection) == NULL)
		return NULL;

	struct MenuWindow* win = calloc(1, sizeof(*win));

	if (!win)
		return NULL;
	win->connection = connection;
	win->frame = frame;
	win->width = (size_t) ceil(frame.size.width);
	win->height = (size_t) ceil(frame.size.height);

	bool x11 = _CGSNativeConnectionKind() == CGSNativeConnectionKindX11;

	if (!g_registered)
	{
		// The GL layer has to be told which window system the connection is to. AppKit tells it when
		// it is loaded; an application with no AppKit has nobody else to do it. The platforms are
		// the EGL ones: X11 and Wayland.
		g_registered = true;
		if (x11)
			CGLRegisterNativeDisplay(_CGSNativeDisplay(connection));
		else
			CGLRegisterNativeDisplayForPlatform(_CGSNativeDisplay(connection), kEGLPlatformWayland);
	}
	if (x11)
	{
		// A window that no window manager decorates or places
		if (_CGSCreateDisplayOverlayWindow(connection, frame, &win->window, &win->native) != kCGErrorSuccess)
		{
			free(win);
			return NULL;
		}
		CGSSetWindowLevel(connection, win->window, level);
	}
	else
	{
		CGSRegionRef region = NULL;
		CGSSurfaceID surface = 0;

		if (CGSNewRegionWithRect(&frame, &region) != kCGErrorSuccess)
		{
			free(win);
			return NULL;
		}
		CGError error = CGSNewWindow(connection, 0, 0, 0, region, &win->window);

		CGSRegionRelease(region);
		// The level and the style decide what the window system makes of the window, so they are set
		// before it is first used
		if (error == kCGErrorSuccess)
			error = CGSSetWindowLevel(connection, win->window, level);
		if (error == kCGErrorSuccess)
			error = _CGSWindowSetStyle(connection, win->window,
				kCGSWindowStyleBorderless | kCGSWindowStyleNoFocus | kCGSWindowStyleUtility);
		if (error == kCGErrorSuccess)
			error = CGSAddSurface(connection, win->window, &surface);
		if (error == kCGErrorSuccess)
			error = CGSSetSurfaceBounds(connection, win->window, surface, CGRectMake(0, 0, frame.size.width, frame.size.height));
		if (error == kCGErrorSuccess)
		{
			win->native = _CGSNativeWindowForSurfaceID(connection, win->window, surface);
			CGSSetWindowTitle(connection, win->window, CFSTR(""));
			CGSOrderWindow(connection, win->window, kCGSOrderIn, 0);
		}
		if (error != kCGErrorSuccess || !win->native)
		{
			destroyMenuWindow(win);
			return NULL;
		}
	}

	win->glWindow = CGLGetWindow(win->native);
	if (!win->glWindow || CGLCreateContextForWindow(win->glWindow, &win->gl) != kCGLNoError)
	{
		destroyMenuWindow(win);
		return NULL;
	}

	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();

	win->pixels = calloc(win->width * 4, win->height);
	if (win->pixels)
		win->bitmap = CGBitmapContextCreate(win->pixels, win->width, win->height, 8, win->width * 4, colorSpace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little);
	CGColorSpaceRelease(colorSpace);
	if (!win->bitmap)
	{
		destroyMenuWindow(win);
		return NULL;
	}
	// The drawing code works with the origin at the top-left
	CGContextTranslateCTM(win->bitmap, 0, win->height);
	CGContextScaleCTM(win->bitmap, 1, -1);
	return win;
}

// Puts the bitmap in the window
static void presentMenuWindow(struct MenuWindow* win)
{
	CGLContextObj previous = CGLGetCurrentContext();

	if (CGLContextMakeCurrentAndAttachToWindow(win->gl, win->glWindow) != kCGLNoError)
		return;
	if (!win->texture)
		glGenTextures(1, &win->texture);
	glViewport(0, 0, (GLsizei) win->width, (GLsizei) win->height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, win->texture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei) win->width, (GLsizei) win->height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, win->pixels);
	glDrawBuffer(GL_BACK);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glColor4f(1, 1, 1, 1);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex2f(0, 1);
	glTexCoord2f(1, 0); glVertex2f(1, 1);
	glTexCoord2f(1, 1); glVertex2f(1, 0);
	glTexCoord2f(0, 1); glVertex2f(0, 0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	CGLFlushDrawable(win->gl);
	CGLSetCurrentContext(previous);
}

// ---------------------------------------------------------------------------------------
// Layout and drawing
// ---------------------------------------------------------------------------------------

enum {
	kBarHeight = 22,
	kItemHeight = 19,
	kMenuPadding = 4,
	kMarkColumn = 22,
	kTitleExtra = 18,
	kMaxLevels = 10,
};

static CTFontRef menuFont(void)
{
	static CTFontRef font;

	if (!font)
	{
		font = CTFontCreateUIFontForLanguage(kCTFontUIFontMenuItem, 14, NULL);
		if (!font)
			font = CTFontCreateWithName(CFSTR("Helvetica"), 14, NULL);
	}
	return font;
}

static CTLineRef createLine(CFStringRef text, CGColorRef color)
{
	CFTypeRef keys[2] = { kCTFontAttributeName, kCTForegroundColorAttributeName };
	CFTypeRef values[2] = { menuFont(), color };
	CFDictionaryRef attributes = CFDictionaryCreate(NULL, keys, values, color ? 2 : 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFAttributedStringRef string = CFAttributedStringCreate(NULL, text, attributes);
	CTLineRef line = CTLineCreateWithAttributedString(string);

	CFRelease(string);
	CFRelease(attributes);
	return line;
}

static CGFloat textWidth(CFStringRef text)
{
	CTLineRef line = createLine(text, NULL);
	CGFloat width = (CGFloat) CTLineGetTypographicBounds(line, NULL, NULL, NULL);

	CFRelease(line);
	return width;
}

// Draws the text with its left edge at x and the vertical middle of the line at midY, in the context
// with the origin at the top-left
static void drawText(CGContextRef ctx, CFStringRef text, CGFloat x, CGFloat midY, CGFloat gray, CGFloat alpha, bool rightAligned)
{
	CGColorSpaceRef space = CGColorSpaceCreateDeviceGray();
	CGFloat components[2] = { gray, alpha };
	CGColorRef color = CGColorCreate(space, components);
	CTLineRef line = createLine(text, color);
	CGFloat ascent, descent;
	CGFloat width = (CGFloat) CTLineGetTypographicBounds(line, &ascent, &descent, NULL);

	CGContextSaveGState(ctx);
	CGContextSetTextMatrix(ctx, CGAffineTransformMakeScale(1, -1));
	CGContextSetTextPosition(ctx, rightAligned ? x - width : x, midY + (ascent - descent) / 2);
	CTLineDraw(line, ctx);
	CGContextRestoreGState(ctx);
	CFRelease(line);
	CGColorRelease(color);
	CGColorSpaceRelease(space);
}

struct ItemLayout
{
	CGFloat top;
	CGFloat height;
	bool visible;
};

struct MenuLayout
{
	CGFloat width;
	CGFloat height;       // of the window
	ItemCount count;
	struct ItemLayout* items;
};

static CFStringRef copyKeyEquivalentText(const struct HIMenuItemInfo* info)
{
	if (info->commandKey == 0 && info->glyph == 0)
		return NULL;

	CFMutableStringRef text = CFStringCreateMutable(NULL, 0);

	if (info->modifiers & kMenuControlModifier)
		CFStringAppend(text, CFSTR("⌃"));
	if (info->modifiers & kMenuOptionModifier)
		CFStringAppend(text, CFSTR("⌥"));
	if (info->modifiers & kMenuShiftModifier)
		CFStringAppend(text, CFSTR("⇧"));
	if (!(info->modifiers & kMenuNoCommandModifier))
		CFStringAppend(text, CFSTR("⌘"));

	UniChar key = 0;

	switch (info->glyph)
	{
		case kMenuReturnGlyph: key = 0x21A9; break;
		case kMenuDeleteLeftGlyph: key = 0x232B; break;
		case kMenuDeleteRightGlyph: key = 0x2326; break;
		case kMenuEscapeGlyph: key = 0x238B; break;
		case kMenuLeftArrowGlyph: key = 0x2190; break;
		case kMenuUpArrowGlyph: key = 0x2191; break;
		case kMenuRightArrowGlyph: key = 0x2192; break;
		case kMenuDownArrowGlyph: key = 0x2193; break;
		case kMenuSpaceGlyph: key = 0x2423; break;
		case kMenuTabRightGlyph: key = 0x21E5; break;
		case kMenuPageUpGlyph: key = 0x21DE; break;
		case kMenuPageDownGlyph: key = 0x21DF; break;
		default: break;
	}
	if (!key && info->glyph >= kMenuF1Glyph && info->glyph <= kMenuF12Glyph)
		CFStringAppendFormat(text, NULL, CFSTR("F%d"), info->glyph - kMenuF1Glyph + 1);
	else if (!key && !(info->attributes & kMenuItemAttrUseVirtualKey) && info->commandKey)
	{
		UniChar c = (UniChar) info->commandKey;

		if (c >= 'a' && c <= 'z')
			c = (UniChar) (c - 'a' + 'A');
		CFStringAppendCharacters(text, &c, 1);
	}
	else if (key)
		CFStringAppendCharacters(text, &key, 1);
	return text;
}

static void computeLayout(MenuRef menu, struct MenuLayout* layout)
{
	ItemCount count = _HIMenuItemCount(menu);
	CGFloat maxText = 0, maxKey = 0, y = kMenuPadding;
	SInt16 separatorHeight = 12;

	GetThemeMenuSeparatorHeight(&separatorHeight);
	layout->count = count;
	layout->items = calloc(count + 1, sizeof(struct ItemLayout));
	for (ItemCount i = 0; i < count; i++)
	{
		struct HIMenuItemInfo info;

		_HIMenuGetItem(menu, (MenuItemIndex) (i + 1), &info);
		struct ItemLayout* item = &layout->items[i];

		item->visible = !(info.attributes & kMenuItemAttrHidden);
		if (!item->visible)
			continue;
		item->top = y;
		if (info.attributes & kMenuItemAttrSeparator)
			item->height = separatorHeight;
		else
		{
			item->height = kItemHeight;
			CGFloat w = textWidth(info.title) + info.indent * 12;

			if (w > maxText)
				maxText = w;
			CFStringRef key = copyKeyEquivalentText(&info);

			if (key)
			{
				CGFloat kw = textWidth(key);

				if (kw > maxKey)
					maxKey = kw;
				CFRelease(key);
			}
		}
		y += item->height;
	}
	layout->width = ceil(kMarkColumn + maxText + (maxKey > 0 ? 24 + maxKey : 0) + 22);
	if (layout->width < 80)
		layout->width = 80;
	layout->height = y + kMenuPadding;
}

static void drawMenu(struct MenuWindow* win, MenuRef menu, const struct MenuLayout* layout, MenuItemIndex hilited)
{
	CGContextRef ctx = win->bitmap;
	CGRect menuRect = CGRectMake(0, kMenuPadding, layout->width, layout->height - 2 * kMenuPadding);
	HIThemeMenuDrawInfo menuInfo = { kHIThemeMenuDrawInfoVersionZero, kThemeMenuTypePullDown, 0, 0, 0, 0, 0 };

	CGContextClearRect(ctx, CGRectMake(0, 0, win->width, win->height));
	HIThemeDrawMenuBackground(&menuRect, &menuInfo, ctx, kHIThemeOrientationInverted);

	for (ItemCount i = 0; i < layout->count; i++)
	{
		struct HIMenuItemInfo info;
		const struct ItemLayout* position = &layout->items[i];

		if (!position->visible)
			continue;
		_HIMenuGetItem(menu, (MenuItemIndex) (i + 1), &info);
		CGRect itemRect = CGRectMake(0, position->top, layout->width, position->height);
		bool disabled = (info.attributes & kMenuItemAttrDisabled) != 0 || !_HIMenuIsEnabled(menu);
		HIThemeMenuItemDrawInfo itemInfo;

		itemInfo.version = 0;
		itemInfo.itemType = info.submenu ? kThemeMenuItemHierarchical : kThemeMenuItemPlain;
		itemInfo.state = disabled ? kThemeMenuDisabled : (hilited == i + 1 ? kThemeMenuSelected : kThemeMenuActive);

		if (info.attributes & kMenuItemAttrSeparator)
		{
			HIThemeDrawMenuSeparator(&menuRect, &itemRect, &itemInfo, ctx, kHIThemeOrientationInverted);
			continue;
		}
		HIThemeDrawMenuItem(&menuRect, &itemRect, &itemInfo, ctx, kHIThemeOrientationInverted, NULL);

		bool selected = itemInfo.state == kThemeMenuSelected;
		CGFloat gray = selected ? 1 : 0;
		CGFloat alpha = disabled ? 0.35 : (selected ? 1 : 0.9);
		CGFloat mid = position->top + position->height / 2;

		if (info.mark)
		{
			UniChar mark = info.mark == checkMark ? 0x2713 : (UniChar) info.mark;
			CFStringRef text = CFStringCreateWithCharacters(NULL, &mark, 1);

			drawText(ctx, text, 6, mid, gray, alpha, false);
			CFRelease(text);
		}
		drawText(ctx, info.title, kMarkColumn + info.indent * 12, mid, gray, alpha, false);
		CFStringRef key = copyKeyEquivalentText(&info);

		if (key)
		{
			drawText(ctx, key, layout->width - (info.submenu ? 28 : 12), mid, gray, alpha, true);
			CFRelease(key);
		}
	}
}

// ---------------------------------------------------------------------------------------
// The menu bar
// ---------------------------------------------------------------------------------------

struct BarTitle
{
	MenuRef menu;
	CGFloat left;
	CGFloat width;
	CFStringRef text;
};

static struct MenuWindow* g_barWindow;

static CGRect mainDisplayBounds(void)
{
	return CGDisplayBounds(CGMainDisplayID());
}

static ItemCount copyBarTitles(struct BarTitle** outTitles)
{
	MenuRef* menus;
	ItemCount count = _HICopyMenuBarMenus(&menus);
	struct BarTitle* titles = calloc(count + 1, sizeof(struct BarTitle));
	CGFloat x = 8;

	for (ItemCount i = 0; i < count; i++)
	{
		CFStringRef title = _HIMenuTitle(menus[i]);

		titles[i].menu = menus[i];
		titles[i].text = title ? CFRetain(title) : CFSTR("");
		titles[i].left = x;
		titles[i].width = ceil(textWidth(titles[i].text)) + kTitleExtra;
		x += titles[i].width;
	}
	free(menus);
	*outTitles = titles;
	return count;
}

static void freeBarTitles(struct BarTitle* titles, ItemCount count)
{
	for (ItemCount i = 0; i < count; i++)
	{
		CFRelease(titles[i].text);
		CFRelease((CFTypeRef) titles[i].menu);
	}
	free(titles);
}

static void drawBar(struct MenuWindow* win, struct BarTitle* titles, ItemCount count, MenuRef selected)
{
	CGContextRef ctx = win->bitmap;
	HIRect bounds = CGRectMake(0, 0, win->width, win->height);
	HIThemeMenuBarDrawInfo barInfo = { 0, kThemeMenuBarNormal, 0 };

	CGContextClearRect(ctx, bounds);
	HIThemeDrawMenuBarBackground(&bounds, &barInfo, ctx, kHIThemeOrientationInverted);
	for (ItemCount i = 0; i < count; i++)
	{
		HIRect titleRect = CGRectMake(titles[i].left, 0, titles[i].width, kBarHeight);
		bool isSelected = titles[i].menu == selected || (!selected && _HIHilitedMenuID() != 0 && GetMenuID(titles[i].menu) == _HIHilitedMenuID());
		HIThemeMenuTitleDrawInfo titleInfo = { 0, isSelected ? kThemeMenuSelected : kThemeMenuActive, 0, 0 };
		HIRect label;
		bool disabled = !_HIMenuIsEnabled(titles[i].menu);

		HIThemeDrawMenuTitle(&bounds, &titleRect, &titleInfo, ctx, kHIThemeOrientationInverted, &label);
		drawText(ctx, titles[i].text, titles[i].left + kTitleExtra / 2, kBarHeight / 2, isSelected ? 1 : 0, disabled ? 0.35 : 1, false);
	}
}

static void redrawBar(MenuRef selected)
{
	if (!g_barWindow)
		return;

	struct BarTitle* titles;
	ItemCount count = copyBarTitles(&titles);

	drawBar(g_barWindow, titles, count, selected);
	freeBarTitles(titles, count);
	presentMenuWindow(g_barWindow);
}

// Shows, updates or hides the menu bar window
static void updateBarWindow(bool create)
{
	if (!IsMenuBarVisible())
	{
		if (g_barWindow)
		{
			destroyMenuWindow(g_barWindow);
			g_barWindow = NULL;
		}
		return;
	}
	if (!g_barWindow)
	{
		if (!create)
			return;
		CGRect display = mainDisplayBounds();

		g_barWindow = createMenuWindow(CGRectMake(display.origin.x, display.origin.y, display.size.width, kBarHeight), kCGMainMenuWindowLevel);
		if (!g_barWindow)
			return;
	}
	redrawBar(NULL);
}

void _HIMenuBarChanged(void)
{
	// What is on screen is brought up to date by DrawMenuBar(), as the Menu Manager always did
}

void _HIRedrawMenuBar(void)
{
	// Only a menu bar that was drawn once follows; DrawMenuBar() starts it
	if (g_barWindow || !IsMenuBarVisible())
		updateBarWindow(false);
}

void _HIMenuChanged(MenuRef menu)
{
	// As _HIMenuBarChanged(): the bar and the menus that are open follow at DrawMenuBar()
}

void DrawMenuBar(void)
{
	updateBarWindow(true);
}

void InvalMenuBar(void)
{
	_HIRedrawMenuBar();
}

// ---------------------------------------------------------------------------------------
// Tracking
// ---------------------------------------------------------------------------------------

struct Level
{
	MenuRef menu;
	struct MenuWindow* window;
	struct MenuLayout layout;
	CGPoint origin;          // of the window, global
	MenuItemIndex hilited;   // 0 for none
};

struct Tracking
{
	struct Level levels[kMaxLevels];
	int depth;               // the number of open menus
	bool fromBar;
	struct BarTitle* titles;
	ItemCount titleCount;
	MenuRef barSelected;     // the bar menu that is open
	bool sticky;             // the mouse was released over a title: the menus stay open
	bool popUp;              // PopUpMenuSelect(): the menu opened where a control or the program asked
	bool done;
	bool chosen;
	MenuRef chosenMenu;
	MenuItemIndex chosenItem;
	MenuRef root;            // the menu that gets the begin and end tracking events, retained
};

static void redrawLevel(struct Level* level)
{
	drawMenu(level->window, level->menu, &level->layout, level->hilited);
	presentMenuWindow(level->window);
}

static void closeLevels(struct Tracking* tracking, int toDepth)
{
	while (tracking->depth > toDepth)
	{
		struct Level* level = &tracking->levels[--tracking->depth];

		destroyMenuWindow(level->window);
		free(level->layout.items);
		_HISendMenuEvent(kEventMenuClosed, level->menu, 0, false, false);
		CFRelease((CFTypeRef) level->menu);
		memset(level, 0, sizeof(*level));
	}
}

static CGRect clampToDisplay(CGRect frame)
{
	CGRect display = mainDisplayBounds();

	if (frame.origin.x + frame.size.width > display.origin.x + display.size.width)
		frame.origin.x = display.origin.x + display.size.width - frame.size.width;
	if (frame.origin.y + frame.size.height > display.origin.y + display.size.height)
		frame.origin.y = display.origin.y + display.size.height - frame.size.height;
	if (frame.origin.x < display.origin.x)
		frame.origin.x = display.origin.x;
	if (frame.origin.y < display.origin.y)
		frame.origin.y = display.origin.y;
	return frame;
}

// Opens the menu as the next level, with the top-left corner of the window at origin; popUpItem
// moves the window so that the item is at origin
static bool openLevel(struct Tracking* tracking, MenuRef menu, CGPoint origin, MenuItemIndex popUpItem)
{
	if (tracking->depth >= kMaxLevels)
		return false;

	struct Level* level = &tracking->levels[tracking->depth];

	memset(level, 0, sizeof(*level));
	CFRetain((CFTypeRef) menu);
	level->menu = menu;
	_HISendMenuEvent(kEventMenuOpening, menu, 0, false, true);
	_HIUpdateMenuItems(menu, false);
	computeLayout(menu, &level->layout);
	if (popUpItem >= 1 && popUpItem <= level->layout.count)
		origin.y -= level->layout.items[popUpItem - 1].top;

	CGRect frame = clampToDisplay(CGRectMake(origin.x, origin.y, level->layout.width, level->layout.height));

	level->origin = frame.origin;
	level->window = createMenuWindow(frame, kCGPopUpMenuWindowLevel);
	if (!level->window)
	{
		free(level->layout.items);
		CFRelease((CFTypeRef) menu);
		memset(level, 0, sizeof(*level));
		return false;
	}
	tracking->depth++;
	redrawLevel(level);
	return true;
}

static bool itemSelectable(const struct HIMenuItemInfo* info, MenuRef menu)
{
	if (info->attributes & (kMenuItemAttrSeparator | kMenuItemAttrHidden | kMenuItemAttrSectionHeader))
		return false;
	return !(info->attributes & kMenuItemAttrDisabled) && _HIMenuIsEnabled(menu);
}

// The item of the level at the global point, or 0
static MenuItemIndex itemAtPoint(const struct Level* level, CGPoint point)
{
	CGFloat x = point.x - level->origin.x, y = point.y - level->origin.y;

	if (x < 0 || x >= level->layout.width || y < 0 || y >= level->layout.height)
		return 0;
	for (ItemCount i = 0; i < level->layout.count; i++)
	{
		const struct ItemLayout* position = &level->layout.items[i];

		if (position->visible && y >= position->top && y < position->top + position->height)
			return (MenuItemIndex) (i + 1);
	}
	return 0;
}

static ItemCount titleAtPoint(const struct Tracking* tracking, CGPoint point)
{
	CGRect display = mainDisplayBounds();

	if (!tracking->fromBar || point.y < display.origin.y || point.y >= display.origin.y + kBarHeight)
		return (ItemCount) -1;
	for (ItemCount i = 0; i < tracking->titleCount; i++)
	{
		if (point.x >= tracking->titles[i].left && point.x < tracking->titles[i].left + tracking->titles[i].width)
			return i;
	}
	return (ItemCount) -1;
}

static void openBarMenu(struct Tracking* tracking, ItemCount title)
{
	closeLevels(tracking, 0);
	tracking->barSelected = tracking->titles[title].menu;
	redrawBar(tracking->barSelected);
	if (_HIMenuIsEnabled(tracking->barSelected))
		openLevel(tracking, tracking->barSelected, CGPointMake(tracking->titles[title].left, mainDisplayBounds().origin.y + kBarHeight - kMenuPadding), 0);
}

static void setHilite(struct Tracking* tracking, int depth, MenuItemIndex item)
{
	struct Level* level = &tracking->levels[depth];

	if (level->hilited == item)
		return;
	level->hilited = item;
	redrawLevel(level);
	if (item)
		_HISendMenuEvent(kEventMenuTargetItem, level->menu, item, false, false);
}

// The pointer is over the point (or the keyboard moved to an item): updates the highlight, and
// opens or closes the submenus
static void trackPoint(struct Tracking* tracking, CGPoint point)
{
	ItemCount title = titleAtPoint(tracking, point);

	if (title != (ItemCount) -1)
	{
		if (tracking->titles[title].menu != tracking->barSelected)
			openBarMenu(tracking, title);
		else if (tracking->depth > 0)
			setHilite(tracking, 0, 0);
		return;
	}

	// The deepest level the point is in
	for (int depth = tracking->depth - 1; depth >= 0; depth--)
	{
		struct Level* level = &tracking->levels[depth];
		MenuItemIndex item = itemAtPoint(level, point);

		if (item == 0)
			continue;

		struct HIMenuItemInfo info;

		_HIMenuGetItem(level->menu, item, &info);
		if (!itemSelectable(&info, level->menu) && !(info.submenu && !(info.attributes & kMenuItemAttrDisabled)))
		{
			closeLevels(tracking, depth + 1);
			setHilite(tracking, depth, 0);
			return;
		}
		if (tracking->depth > depth + 1 && level->hilited != item)
			closeLevels(tracking, depth + 1);
		setHilite(tracking, depth, item);
		if (info.submenu && tracking->depth == depth + 1)
		{
			CGPoint origin = CGPointMake(level->origin.x + level->layout.width - 4, level->origin.y + level->layout.items[item - 1].top - kMenuPadding);

			openLevel(tracking, info.submenu, origin, 0);
		}
		else if (!info.submenu)
			closeLevels(tracking, depth + 1);
		return;
	}

	// Outside every menu: nothing is highlighted in the deepest level
	if (tracking->depth > 0)
		setHilite(tracking, tracking->depth - 1, 0);
}

static void chooseItem(struct Tracking* tracking, int depth)
{
	struct Level* level = &tracking->levels[depth];
	struct HIMenuItemInfo info;

	if (level->hilited == 0)
		return;
	_HIMenuGetItem(level->menu, level->hilited, &info);
	if (!itemSelectable(&info, level->menu) || info.submenu)
		return;
	tracking->chosen = true;
	tracking->chosenMenu = level->menu;
	tracking->chosenItem = level->hilited;
	CFRetain((CFTypeRef) level->menu);
	tracking->done = true;
}

static MenuItemIndex nextItem(MenuRef menu, MenuItemIndex from, int direction)
{
	ItemCount count = _HIMenuItemCount(menu);

	if (count == 0)
		return 0;
	MenuItemIndex item = from;

	for (ItemCount tries = 0; tries <= count; tries++)
	{
		if (item == 0)
			item = direction > 0 ? 1 : (MenuItemIndex) count;
		else
		{
			int next = (int) item + direction;

			item = next < 1 || next > (int) count ? 0 : (MenuItemIndex) next;
			if (item == 0)
				item = direction > 0 ? 1 : (MenuItemIndex) count;
		}
		struct HIMenuItemInfo info;

		_HIMenuGetItem(menu, item, &info);
		if (itemSelectable(&info, menu) || (info.submenu && !(info.attributes & kMenuItemAttrDisabled)))
			return item;
	}
	return 0;
}

static void trackKey(struct Tracking* tracking, UInt32 keyCode)
{
	enum { kKeyReturn = 0x24, kKeyEnter = 0x4C, kKeyEscape = 0x35, kKeyLeft = 0x7B, kKeyRight = 0x7C, kKeyDown = 0x7D, kKeyUp = 0x7E };

	switch (keyCode)
	{
		case kKeyEscape:
			tracking->done = true;
			break;
		case kKeyDown:
		case kKeyUp:
			if (tracking->depth > 0)
			{
				int depth = tracking->depth - 1;

				setHilite(tracking, depth, nextItem(tracking->levels[depth].menu, tracking->levels[depth].hilited, keyCode == kKeyDown ? 1 : -1));
			}
			else if (tracking->fromBar && tracking->titleCount > 0)
				openBarMenu(tracking, 0);
			break;
		case kKeyRight:
		case kKeyLeft:
		{
			int depth = tracking->depth - 1;

			if (depth < 0)
				break;
			struct Level* level = &tracking->levels[depth];

			if (keyCode == kKeyRight && level->hilited)
			{
				struct HIMenuItemInfo info;

				_HIMenuGetItem(level->menu, level->hilited, &info);
				if (info.submenu && !(info.attributes & kMenuItemAttrDisabled))
				{
					CGPoint origin = CGPointMake(level->origin.x + level->layout.width - 4, level->origin.y + level->layout.items[level->hilited - 1].top - kMenuPadding);

					if (openLevel(tracking, info.submenu, origin, 0))
						setHilite(tracking, tracking->depth - 1, nextItem(info.submenu, 0, 1));
					break;
				}
			}
			if (keyCode == kKeyLeft && depth > 0)
			{
				closeLevels(tracking, depth);
				break;
			}
			if (tracking->fromBar && depth == 0)
			{
				for (ItemCount i = 0; i < tracking->titleCount; i++)
				{
					if (tracking->titles[i].menu != tracking->barSelected)
						continue;
					ItemCount next = keyCode == kKeyRight ? (i + 1) % tracking->titleCount : (i + tracking->titleCount - 1) % tracking->titleCount;

					openBarMenu(tracking, next);
					break;
				}
			}
			break;
		}
		case kKeyReturn:
		case kKeyEnter:
			if (tracking->depth > 0)
				chooseItem(tracking, tracking->depth - 1);
			break;
		default:
			break;
	}
}

static Boolean trackingFilter(EventRef event, void* info)
{
	OSType eventClass = GetEventClass(event);

	return eventClass == kEventClassMouse || eventClass == kEventClassKeyboard;
}

// Runs the tracking until a choice or a cancellation. The mouse button is down when it starts
// (mouseDown), or the menus were started by the keyboard.
static void runTracking(struct Tracking* tracking, bool mouseDown, CGPoint start)
{
	// Where the pointer was when the first mouse event came, and whether it moved since: a click that did
	// not move the pointer is a click, not a drag
	CGPoint first = CGPointZero;
	bool haveFirst = false, moved = false;

	_HISendMenuEvent(kEventMenuBeginTracking, tracking->root, 0, false, false);
	trackPoint(tracking, start);

	while (!tracking->done)
	{
		EventRef event;
		OSStatus status = _HIReceiveNextEventMatching(trackingFilter, NULL, NULL, kEventDurationForever, true, &event);

		if (status != noErr)
		{
			if (status == eventLoopQuitErr)
				break;
			continue;
		}

		OSType eventClass = GetEventClass(event);
		UInt32 kind = GetEventKind(event);

		if (eventClass == kEventClassMouse)
		{
			HIPoint location;

			if (GetEventParameter(event, kEventParamMouseLocation, typeHIPoint, NULL, sizeof(location), NULL, &location) == noErr)
			{
				CGPoint point = CGPointMake(location.x, location.y);

				if (kind == kEventMouseMoved || kind == kEventMouseDragged || kind == kEventMouseUp)
				{
					if (!haveFirst)
					{
						first = point;
						haveFirst = true;
					}
					else if (fabs(point.x - first.x) > 3 || fabs(point.y - first.y) > 3)
						moved = true;
				}
				switch (kind)
				{
					case kEventMouseDown:
						tracking->sticky = false;
						// A click outside every menu ends the tracking
						{
							bool inside = titleAtPoint(tracking, point) != (ItemCount) -1;

							for (int depth = 0; depth < tracking->depth && !inside; depth++)
								inside = itemAtPoint(&tracking->levels[depth], point) != 0 || (point.x >= tracking->levels[depth].origin.x && point.x < tracking->levels[depth].origin.x + tracking->levels[depth].layout.width && point.y >= tracking->levels[depth].origin.y && point.y < tracking->levels[depth].origin.y + tracking->levels[depth].layout.height);
							if (!inside)
								tracking->done = true;
							else
								trackPoint(tracking, point);
						}
						break;
					case kEventMouseMoved:
					case kEventMouseDragged:
						trackPoint(tracking, point);
						break;
					case kEventMouseUp:
						if (titleAtPoint(tracking, point) != (ItemCount) -1 && !tracking->sticky && mouseDown)
						{
							// A click on a title leaves the menu open until the next click
							tracking->sticky = true;
							mouseDown = false;
						}
						else if (tracking->popUp && mouseDown && !moved && !tracking->sticky && tracking->depth > 0 &&
							itemAtPoint(&tracking->levels[tracking->depth - 1], point) == 0 && titleAtPoint(tracking, point) == (ItemCount) -1)
						{
							// A quick click on what opened a pop-up menu (the pointer is not on one of its items and
							// did not move) leaves the menu open until the next click, as a click on the title of a
							// menu bar menu does
							tracking->sticky = true;
							mouseDown = false;
						}
						else if (tracking->depth > 0 && itemAtPoint(&tracking->levels[tracking->depth - 1], point) == 0 && titleAtPoint(tracking, point) == (ItemCount) -1 && !tracking->sticky)
							tracking->done = true;
						else if (!tracking->sticky || itemAtPoint(&tracking->levels[tracking->depth > 0 ? tracking->depth - 1 : 0], point) != 0)
						{
							trackPoint(tracking, point);
							if (tracking->depth > 0)
								chooseItem(tracking, tracking->depth - 1);
							if (!tracking->done && !tracking->sticky)
								tracking->done = true;
						}
						break;
					default:
						break;
				}
			}
		}
		else if (kind == kEventRawKeyDown || kind == kEventRawKeyRepeat)
		{
			UInt32 keyCode;

			if (GetEventParameter(event, kEventParamKeyCode, typeUInt32, NULL, sizeof(keyCode), NULL, &keyCode) == noErr)
				trackKey(tracking, keyCode);
		}
		ReleaseEvent(event);
	}
}

// (outChosen, when given, gets the menu of the chosen item, retained: menus can share a menu ID)
static UInt32 finishTracking(struct Tracking* tracking, bool highlightResult, MenuRef* outChosen)
{
	UInt32 choice = 0;
	MenuRef chosenMenu = tracking->chosenMenu;
	MenuID barMenuID = tracking->barSelected ? GetMenuID(tracking->barSelected) : 0;

	closeLevels(tracking, 0);
	if (outChosen)
	{
		*outChosen = tracking->chosen ? chosenMenu : NULL;
		if (*outChosen)
			CFRetain((CFTypeRef) *outChosen);
	}
	if (tracking->chosen)
	{
		choice = ((UInt32) (UInt16) GetMenuID(chosenMenu) << 16) | tracking->chosenItem;
		if (highlightResult)
			HiliteMenu(barMenuID ? barMenuID : GetMenuID(chosenMenu));
	}
	if (tracking->fromBar)
	{
		redrawBar(NULL);
		freeBarTitles(tracking->titles, tracking->titleCount);
	}
	_HISendMenuEvent(kEventMenuEndTracking, tracking->root, 0, false, false);
	if (chosenMenu)
		CFRelease((CFTypeRef) chosenMenu);
	CFRelease((CFTypeRef) tracking->root);
	return choice;
}

static UInt32 menuSelect(Point startPt, MenuRef* outChosen)
{
	struct Tracking tracking;

	if (outChosen)
		*outChosen = NULL;

	memset(&tracking, 0, sizeof(tracking));
	updateBarWindow(true);
	if (!g_barWindow)
		return 0;
	tracking.fromBar = true;
	tracking.root = AcquireRootMenu();
	tracking.titleCount = copyBarTitles(&tracking.titles);
	runTracking(&tracking, true, CGPointMake(startPt.h, startPt.v));
	return finishTracking(&tracking, true, outChosen);
}

UInt32 MenuSelect(Point startPt)
{
	return menuSelect(startPt, NULL);
}

UInt32 PopUpMenuSelect(MenuRef menu, SInt16 top, SInt16 left, SInt16 popUpItem)
{
	struct Tracking tracking;

	if (!_HIIsMenu(menu))
		return 0;
	memset(&tracking, 0, sizeof(tracking));
	CFRetain((CFTypeRef) menu);
	tracking.root = menu;
	if (!openLevel(&tracking, menu, CGPointMake(left, top), popUpItem > 0 ? (MenuItemIndex) popUpItem : 0))
	{
		CFRelease((CFTypeRef) menu);
		return 0;
	}
	if (popUpItem > 0)
		setHilite(&tracking, 0, (MenuItemIndex) popUpItem);
	// The pointer is where the menu was asked to appear, usually; the click that opened the pop-up has
	// been released when the user does not hold the button, hence the first release does not choose
	tracking.sticky = false;
	tracking.popUp = true;
	runTracking(&tracking, true, CGPointMake(left, top));
	return finishTracking(&tracking, false, NULL);
}

OSStatus CancelMenuTracking(MenuRef inRootMenu, Boolean inImmediate, UInt32 inDismissalReason)
{
	// The tracking loop of a menu that is open runs below the caller (it is called from a handler of the
	// events of the tracking): a cancel key event ends it
	EventRef event = NULL;
	UInt32 modifiers = 0;
	UInt32 keyCode = 0x35;

	if (CreateEvent(NULL, kEventClassKeyboard, kEventRawKeyDown, _HICurrentTime(), kEventAttributeNone, &event) != noErr)
		return memFullErr;
	SetEventParameter(event, kEventParamKeyCode, typeUInt32, sizeof(keyCode), &keyCode);
	SetEventParameter(event, kEventParamKeyModifiers, typeUInt32, sizeof(modifiers), &modifiers);
	PostEventToQueue(GetMainEventQueue(), event, kEventPriorityHigh);
	ReleaseEvent(event);
	return noErr;
}

// ---------------------------------------------------------------------------------------
// The standard handling of the menu bar
// ---------------------------------------------------------------------------------------

// A mouse down in the menu bar window that the application did not handle: the menus are tracked
// and the command of the chosen item processed
OSStatus _HIHandleMenuBarMouseDown(EventRef event)
{
	HIPoint location;
	EventMouseButton button = kEventMouseButtonPrimary;

	if (!g_barWindow || GetEventParameter(event, kEventParamMouseLocation, typeHIPoint, NULL, sizeof(location), NULL, &location) != noErr)
		return eventNotHandledErr;
	GetEventParameter(event, kEventParamMouseButton, typeMouseButton, NULL, sizeof(button), NULL, &button);
	if (button != kEventMouseButtonPrimary)
		return eventNotHandledErr;

	CGRect display = mainDisplayBounds();

	if (location.y < display.origin.y || location.y >= display.origin.y + kBarHeight)
		return eventNotHandledErr;

	Point point = { (short) location.y, (short) location.x };
	MenuRef menu = NULL;
	UInt32 choice = menuSelect(point, &menu);

	// The menu is the one that was chosen from: its ID is not enough, menus may share one
	if (choice && menu)
		_HIProcessMenuCommand(menu, (MenuItemIndex) (choice & 0xffff));
	if (menu)
		CFRelease((CFTypeRef) menu);
	HiliteMenu(0);
	return noErr;
}
