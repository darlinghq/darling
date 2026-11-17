/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

// HITheme drawing: every draw call renders into a bitmap context, in both
// orientations, and has to draw something inside the bounds it was given (plus
// the documented shadow and focus ring margin); invalid arguments return
// paramErr; the label rectangles of the draw calls follow the orientation.

#include <Carbon/Carbon.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define W 240
#define H 160

static int g_failures;
static CGContextRef g_ctx;
static HIThemeOrientation g_o;
static unsigned char *g_pixels;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d (orientation %d): %s\n", __LINE__, (int) g_o, #cond); g_failures++; } } while (0)

static void clear_bitmap(void)
{
	CGContextClearRect(g_ctx, CGRectMake(0, 0, W, H));
}

// Bounding box of the pixels that are not transparent, in the coordinates the
// rectangles of the draw calls use (y grows with the orientation of the context).
static int drawn_bounds(int *x0, int *y0, int *x1, int *y1)
{
	int minx = W, miny = H, maxx = -1, maxy = -1;
	for (int row = 0; row < H; row++)
	{
		for (int x = 0; x < W; x++)
		{
			if (g_pixels[(row * W + x) * 4 + 3] != 0)
			{
				if (x < minx) minx = x;
				if (x > maxx) maxx = x;
				if (row < miny) miny = row;
				if (row > maxy) maxy = row;
			}
		}
	}
	if (maxx < 0)
		return 0;
	*x0 = minx;
	*x1 = maxx + 1;
	if (g_o == kHIThemeOrientationInverted)
	{
		*y0 = H - (maxy + 1);
		*y1 = H - miny;
	}
	else
	{
		*y0 = miny;
		*y1 = maxy + 1;
	}
	return 1;
}

static void check_drawn(const char *label, OSStatus st, CGRect r, int tol, int must_draw)
{
	if (st != noErr)
	{
		printf("FAIL %s (orientation %d): status %d\n", label, (int) g_o, (int) st);
		g_failures++;
		return;
	}
	int x0, y0, x1, y1;
	int any = drawn_bounds(&x0, &y0, &x1, &y1);
	if (!must_draw)
	{
		if (any)
		{
			printf("FAIL %s (orientation %d): drew %d,%d..%d,%d, expected nothing\n", label, (int) g_o, x0, y0, x1, y1);
			g_failures++;
		}
		return;
	}
	if (!any)
	{
		printf("FAIL %s (orientation %d): nothing drawn\n", label, (int) g_o);
		g_failures++;
		return;
	}
	if (x0 < r.origin.x - tol || y0 < r.origin.y - tol || x1 > r.origin.x + r.size.width + tol || y1 > r.origin.y + r.size.height + tol)
	{
		printf("FAIL %s (orientation %d): drew %d,%d..%d,%d outside %g,%g,%g,%g (+-%d)\n", label, (int) g_o, x0, y0, x1, y1,
			r.origin.x, r.origin.y, r.size.width, r.size.height, tol);
		g_failures++;
	}
}

#define DRAW(label, rect, tol, must, call) do { clear_bitmap(); OSStatus st_ = (call); check_drawn(label, st_, rect, tol, must); } while (0)

// Color of one pixel, addressed in the coordinates of the draw calls.
static void pixel_at(int x, int y, int *r, int *g, int *b, int *a)
{
	int row = g_o == kHIThemeOrientationInverted ? H - 1 - y : y;
	const unsigned char *p = g_pixels + (row * W + x) * 4;
	*r = p[0]; *g = p[1]; *b = p[2]; *a = p[3];
}

static void test_buttons(void)
{
	HIRect rect = CGRectMake(20, 20, 100, 40);
	char label[64];
	static const ThemeDrawState states[] = { kThemeStateInactive, kThemeStateActive, kThemeStatePressed, kThemeStateUnavailable, kThemeStateUnavailableInactive, kThemeStateRollover };
	for (int kind = 0; kind <= 34; kind++)
	{
		for (unsigned s = 0; s < sizeof(states) / sizeof(states[0]); s++)
		{
			for (int value = 0; value <= 2; value++)
			{
				HIThemeButtonDrawInfo info;
				memset(&info, 0, sizeof(info));
				info.state = states[s];
				info.kind = kind;
				info.value = value;
				snprintf(label, sizeof(label), "button kind %d state %d value %d", kind, (int) states[s], value);
				DRAW(label, rect, 2, 1, HIThemeDrawButton(&rect, &info, g_ctx, g_o, NULL));
			}
		}
		HIThemeButtonDrawInfo focus;
		memset(&focus, 0, sizeof(focus));
		focus.state = kThemeStateActive;
		focus.kind = kind;
		focus.adornment = kThemeAdornmentFocus | kThemeAdornmentDefault;
		snprintf(label, sizeof(label), "focused default button kind %d", kind);
		DRAW(label, rect, 5, 1, HIThemeDrawButton(&rect, &focus, g_ctx, g_o, NULL));
	}

	// the default push button is blue when active, the ordinary one is gray
	HIThemeButtonDrawInfo info;
	memset(&info, 0, sizeof(info));
	info.state = kThemeStateActive;
	info.kind = kThemePushButtonNormal;
	info.adornment = kThemeAdornmentDefault;
	DRAW("default push button", rect, 2, 1, HIThemeDrawButton(&rect, &info, g_ctx, g_o, NULL));
	int r, g, b, a;
	// the artwork is 22 pixels tall at the top of the rectangle
	pixel_at(70, g_o == kHIThemeOrientationInverted ? 60 - 11 : 20 + 11, &r, &g, &b, &a);
	CHECK(b > r + 40 && a == 255);
	info.adornment = kThemeAdornmentNone;
	DRAW("plain push button", rect, 2, 1, HIThemeDrawButton(&rect, &info, g_ctx, g_o, NULL));
	pixel_at(70, g_o == kHIThemeOrientationInverted ? 60 - 11 : 20 + 11, &r, &g, &b, &a);
	CHECK(abs(r - b) < 12 && r > 200);

	// check boxes and radio buttons differ between the states of the value
	HIRect box = CGRectMake(20, 20, 20, 20);
	for (int kind = kThemeCheckBox; kind <= kThemeRadioButton; kind++)
	{
		int count[3];
		for (int value = 0; value <= 2; value++)
		{
			HIThemeButtonDrawInfo ci;
			memset(&ci, 0, sizeof(ci));
			ci.state = kThemeStateActive;
			ci.kind = kind;
			ci.value = value;
			clear_bitmap();
			CHECK(HIThemeDrawButton(&box, &ci, g_ctx, g_o, NULL) == noErr);
			count[value] = 0;
			for (int y = 0; y < H; y++)
				for (int x = 0; x < W; x++)
					if (g_pixels[(y * W + x) * 4 + 2] > g_pixels[(y * W + x) * 4] + 60)
						count[value]++;
		}
		// blue only when the box is on or mixed
		CHECK(count[0] == 0 && count[1] > 50 && count[2] > 50);
	}

	// label rectangles follow the orientation: a list header button is 17 pixels tall at the top
	HIThemeButtonDrawInfo hi;
	memset(&hi, 0, sizeof(hi));
	hi.state = kThemeStateActive;
	hi.kind = kThemeListHeaderButton;
	HIRect label_rect = CGRectZero;
	clear_bitmap();
	CHECK(HIThemeDrawButton(&rect, &hi, g_ctx, g_o, &label_rect) == noErr);
	CHECK(label_rect.origin.x == 32 && label_rect.size.width == 88 && label_rect.size.height == 16);
	CHECK(label_rect.origin.y == (g_o == kHIThemeOrientationInverted ? 44 : 20));

	// kinds without a label
	hi.kind = kThemeArrowButton;
	CHECK(HIThemeDrawButton(&rect, &hi, g_ctx, g_o, &label_rect) == noErr);
	CHECK(label_rect.size.width == 0 && label_rect.size.height == 0);
}

static void test_boxes_and_decorations(void)
{
	HIRect rect = CGRectMake(20, 20, 100, 40);
	char label[64];
	static const int frame_kinds[] = { kHIThemeFrameTextFieldSquare, kHIThemeFrameListBox, kHIThemeFrameTextFieldRound, kHIThemeFrameTextFieldRoundSmall, kHIThemeFrameTextFieldRoundMini };
	for (unsigned i = 0; i < sizeof(frame_kinds) / sizeof(frame_kinds[0]); i++)
	{
		for (int focused = 0; focused <= 1; focused++)
		{
			HIThemeFrameDrawInfo fi = { 0, frame_kinds[i], kThemeStateActive, focused };
			snprintf(label, sizeof(label), "frame kind %d focused %d", frame_kinds[i], focused);
			DRAW(label, rect, focused ? 5 : 1, 1, HIThemeDrawFrame(&rect, &fi, g_ctx, g_o));
		}
	}
	static const int group_kinds[] = { kHIThemeGroupBoxKindPrimary, kHIThemeGroupBoxKindSecondary, kHIThemeGroupBoxKindPrimaryOpaque, kHIThemeGroupBoxKindSecondaryOpaque };
	for (unsigned i = 0; i < 4; i++)
	{
		HIThemeGroupBoxDrawInfo gi = { 0, kThemeStateActive, group_kinds[i] };
		snprintf(label, sizeof(label), "group box kind %d", group_kinds[i]);
		DRAW(label, rect, 1, 1, HIThemeDrawGroupBox(&rect, &gi, g_ctx, g_o));
	}
	HIThemeButtonDrawInfo bi;
	memset(&bi, 0, sizeof(bi));
	bi.state = kThemeStateActive;
	DRAW("generic well", rect, 1, 1, HIThemeDrawGenericWell(&rect, &bi, g_ctx, g_o));

	HIThemeSplitterDrawInfo spi = { 0, kThemeStateActive, kHIThemeSplitterAdornmentNone };
	DRAW("splitter", rect, 1, 1, HIThemeDrawPaneSplitter(&rect, &spi, g_ctx, g_o));
	spi.adornment = kHIThemeSplitterAdornmentMetal;
	DRAW("metal splitter", rect, 1, 1, HIThemeDrawPaneSplitter(&rect, &spi, g_ctx, g_o));
	HIRect vertical = CGRectMake(20, 20, 40, 100);
	DRAW("vertical splitter", vertical, 1, 1, HIThemeDrawPaneSplitter(&vertical, &spi, g_ctx, g_o));
	HIThemeGrabberDrawInfo gri = { 0, kThemeStateActive };
	DRAW("grabber", rect, 1, 1, HIThemeDrawGrabber(&rect, &gri, g_ctx, g_o));
	HIThemePlacardDrawInfo pli = { 0, kThemeStateActive };
	DRAW("placard", rect, 1, 1, HIThemeDrawPlacard(&rect, &pli, g_ctx, g_o));
	pli.state = kThemeStatePressed;
	DRAW("pressed placard", rect, 1, 1, HIThemeDrawPlacard(&rect, &pli, g_ctx, g_o));
	for (int kind = kHIThemeHeaderKindWindow; kind <= kHIThemeHeaderKindList; kind++)
	{
		HIThemeHeaderDrawInfo hi = { 0, kThemeStateActive, kind };
		snprintf(label, sizeof(label), "header kind %d", kind);
		DRAW(label, rect, 1, 1, HIThemeDrawHeader(&rect, &hi, g_ctx, g_o));
	}
	DRAW("focus rect", rect, 5, 1, HIThemeDrawFocusRect(&rect, true, g_ctx, g_o));
	DRAW("focus rect without focus", rect, 0, 0, HIThemeDrawFocusRect(&rect, false, g_ctx, g_o));

	HIThemeSeparatorDrawInfo sep = { 0, kThemeStateActive };
	HIRect hline = CGRectMake(20, 20, 100, 1), vline = CGRectMake(20, 20, 1, 40);
	DRAW("horizontal separator", hline, 0, 1, HIThemeDrawSeparator(&hline, &sep, g_ctx, g_o));
	DRAW("vertical separator", vline, 0, 1, HIThemeDrawSeparator(&vline, &sep, g_ctx, g_o));

	for (int orient = kThemeArrowLeft; orient <= kThemeArrowUp; orient++)
	{
		for (int size = kThemeArrow3pt; size <= kThemeArrow9pt; size++)
		{
			HIThemePopupArrowDrawInfo pa = { 0, kThemeStateActive, orient, size };
			HIRect ar = CGRectMake(20, 20, 20, 20);
			snprintf(label, sizeof(label), "popup arrow orientation %d size %d", orient, size);
			DRAW(label, ar, 0, 1, HIThemeDrawPopupArrow(&ar, &pa, g_ctx, g_o));
		}
	}
	HIThemePopupArrowDrawInfo bad_arrow = { 0, kThemeStateActive, 9, 0 };
	HIRect ar = CGRectMake(20, 20, 20, 20);
	CHECK(HIThemeDrawPopupArrow(&ar, &bad_arrow, g_ctx, g_o) == paramErr);

	HIThemeChasingArrowsDrawInfo ca = { 0, kThemeStateActive, 3 };
	HIRect cr = CGRectMake(20, 20, 16, 16);
	DRAW("chasing arrows", cr, 0, 1, HIThemeDrawChasingArrows(&cr, &ca, g_ctx, g_o));

	HIPoint origin = CGPointMake(50, 50);
	HIThemeGrowBoxDrawInfo gi = { 0, kThemeStateActive, kHIThemeGrowBoxKindNormal, kThemeGrowRight | kThemeGrowDown, kHIThemeGrowBoxSizeNormal };
	HIRect gb = CGRectMake(50, 50, 15, 15);
	DRAW("grow box", gb, 0, 1, HIThemeDrawGrowBox(&origin, &gi, g_ctx, g_o));
	gi.kind = kHIThemeGrowBoxKindNone;
	DRAW("grow box of kind none", gb, 0, 0, HIThemeDrawGrowBox(&origin, &gi, g_ctx, g_o));

	HIThemeTickMarkDrawInfo ti = { 0, kThemeStateActive };
	HIRect tick = CGRectMake(20, 20, 1, 6);
	DRAW("tick mark", tick, 0, 1, HIThemeDrawTickMark(&tick, &ti, g_ctx, g_o));
	HIThemeScrollBarDelimitersDrawInfo di = { 0, kThemeStateActive, kThemeDocumentWindow, 0 };
	HIRect dr = CGRectMake(20, 20, 15, 15);
	DRAW("scroll bar delimiters", dr, 0, 1, HIThemeDrawScrollBarDelimiters(&dr, &di, g_ctx, g_o));

	for (int kind = kThemeBackgroundTabPane; kind <= kThemeBackgroundMetal; kind++)
	{
		HIThemeBackgroundDrawInfo bgi = { 0, kThemeStateActive, kind };
		snprintf(label, sizeof(label), "background kind %d", kind);
		DRAW(label, rect, 0, kind == kThemeBackgroundMetal, HIThemeDrawBackground(&rect, &bgi, g_ctx, g_o));
		DRAW(label, rect, 0, kind == kThemeBackgroundMetal, HIThemeApplyBackground(&rect, &bgi, g_ctx, g_o));
	}
	HIThemeBackgroundDrawInfo bad_bg = { 0, kThemeStateActive, 77 };
	CHECK(HIThemeDrawBackground(&rect, &bad_bg, g_ctx, g_o) == paramErr);
}

static void test_tabs_segments_menus(void)
{
	char label[96];
	HIRect rect = CGRectMake(20, 20, 100, 40);
	for (int pos = kHIThemeSegmentPositionFirst; pos <= kHIThemeSegmentPositionOnly; pos++)
	{
		for (int value = 0; value <= 1; value++)
		{
			HIThemeSegmentDrawInfo si = { 0, kThemeStateActive, value, kHIThemeSegmentSizeNormal, kHIThemeSegmentKindNormal, pos, 0 };
			snprintf(label, sizeof(label), "segment position %d value %d", pos, value);
			DRAW(label, rect, 1, 1, HIThemeDrawSegment(&rect, &si, g_ctx, g_o));
		}
	}
	HIThemeSegmentDrawInfo bad_seg = { 0, kThemeStateActive, 0, 0, 0, 9, 0 };
	CHECK(HIThemeDrawSegment(&rect, &bad_seg, g_ctx, g_o) == paramErr);

	for (int dir = kThemeTabNorth; dir <= kThemeTabWest; dir++)
	{
		HIThemeTabPaneDrawInfo pi;
		memset(&pi, 0, sizeof(pi));
		pi.version = 1;
		pi.state = kThemeStateActive;
		pi.direction = dir;
		snprintf(label, sizeof(label), "tab pane direction %d", dir);
		DRAW(label, rect, 1, 1, HIThemeDrawTabPane(&rect, &pi, g_ctx, g_o));
		for (int style = kThemeTabNonFront; style <= kThemeTabFrontUnavailable; style++)
		{
			HIThemeTabDrawInfo ti;
			memset(&ti, 0, sizeof(ti));
			ti.version = 1;
			ti.style = style;
			ti.direction = dir;
			ti.kind = kHIThemeTabKindNormal;
			ti.position = kHIThemeTabPositionMiddle;
			HIRect label_rect = CGRectZero;
			snprintf(label, sizeof(label), "tab direction %d style %d", dir, style);
			DRAW(label, rect, 1, 1, HIThemeDrawTab(&rect, &ti, g_ctx, g_o, &label_rect));
			CHECK(label_rect.size.width > 0 && label_rect.size.height > 0);
			CHECK(label_rect.origin.x >= rect.origin.x && label_rect.origin.x + label_rect.size.width <= rect.origin.x + rect.size.width);
			CHECK(label_rect.origin.y >= rect.origin.y && label_rect.origin.y + label_rect.size.height <= rect.origin.y + rect.size.height);
		}
	}
	HIThemeTabDrawInfo bad_tab;
	memset(&bad_tab, 0, sizeof(bad_tab));
	bad_tab.direction = 9;
	CHECK(HIThemeDrawTab(&rect, &bad_tab, g_ctx, g_o, NULL) == paramErr);

	// menus
	HIRect bar = CGRectMake(0, 0, W, 22);
	HIThemeMenuBarDrawInfo mb = { 0, kThemeMenuBarNormal, 0 };
	DRAW("menu bar", bar, 0, 1, HIThemeDrawMenuBarBackground(&bar, &mb, g_ctx, g_o));
	HIThemeMenuTitleDrawInfo mt = { 0, kThemeMenuSelected, 0, 0 };
	HIRect title = CGRectMake(20, 0, 50, 22), title_label = CGRectZero;
	DRAW("selected menu title", title, 0, 1, HIThemeDrawMenuTitle(&bar, &title, &mt, g_ctx, g_o, &title_label));
	CHECK(title_label.origin.x == 28 && title_label.size.width == 34 && title_label.size.height == 22);
	mt.state = kThemeMenuActive;
	DRAW("menu title", title, 0, 0, HIThemeDrawMenuTitle(&bar, &title, &mt, g_ctx, g_o, &title_label));
	CHECK(title_label.origin.x == 28 && title_label.size.width == 34);

	HIRect menu = CGRectMake(10, 20, 120, 80), menu_bg = CGRectMake(10, 16, 120, 88);
	for (int type = kThemeMenuTypePullDown; type <= kThemeMenuTypeHierarchical; type++)
	{
		HIThemeMenuDrawInfo mi;
		memset(&mi, 0, sizeof(mi));
		mi.menuType = type;
		snprintf(label, sizeof(label), "menu background type %d", type);
		DRAW(label, menu_bg, 0, 1, HIThemeDrawMenuBackground(&menu, &mi, g_ctx, g_o));
	}
	HIRect item = CGRectMake(10, 40, 120, 19);
	for (int state = kThemeMenuActive; state <= kThemeMenuDisabled; state += state == kThemeMenuSelected ? 2 : 1)
	{
		for (int type = kThemeMenuItemPlain; type <= kThemeMenuItemHierarchical; type++)
		{
			HIThemeMenuItemDrawInfo ii = { 0, type, state };
			HIRect content = CGRectZero;
			snprintf(label, sizeof(label), "menu item type %d state %d", type, state);
			DRAW(label, item, 0, 1, HIThemeDrawMenuItem(&menu, &item, &ii, g_ctx, g_o, &content));
			CHECK(content.origin.x == 10 && content.origin.y == 40 && content.size.width == (type == kThemeMenuItemHierarchical ? 81 : 120) && content.size.height == 17);
		}
	}
	HIThemeMenuItemDrawInfo sep_info = { 0, kThemeMenuItemPlain, kThemeMenuActive };
	HIRect sep_item = CGRectMake(10, 40, 120, 12);
	DRAW("menu separator", sep_item, 0, 1, HIThemeDrawMenuSeparator(&menu, &sep_item, &sep_info, g_ctx, g_o));
}

static void test_windows(void)
{
	char label[96];
	HIRect content = CGRectMake(40, 50, 100, 60);
	static const struct { int type; double sx, sy, sw, sh; } windows[] = {
		{ kThemeDocumentWindow, 40, 28, 100, 82 },
		{ kThemeMovableDialogWindow, 36, 28, 108, 82 },
		{ kThemeUtilityWindow, 40, 34, 100, 76 },
		{ kThemeUtilitySideWindow, 24, 50, 132, 60 },
	};
	for (unsigned i = 0; i < sizeof(windows) / sizeof(windows[0]); i++)
	{
		HIThemeWindowDrawInfo wi;
		memset(&wi, 0, sizeof(wi));
		wi.state = kThemeStateActive;
		wi.windowType = windows[i].type;
		wi.attributes = kThemeWindowHasCloseBox | kThemeWindowHasFullZoom | kThemeWindowHasCollapseBox | kThemeWindowHasTitleText;
		wi.titleWidth = 40;
		HIRect title_rect = CGRectZero;
		// the title bar lies above the content: at smaller y in a top-left context, at larger y in an inverted one
		HIRect structure = CGRectMake(windows[i].sx, g_o == kHIThemeOrientationInverted && windows[i].type != kThemeUtilitySideWindow ? 50 : windows[i].sy, windows[i].sw, windows[i].sh);
		snprintf(label, sizeof(label), "window frame type %d", windows[i].type);
		DRAW(label, structure, 0, 1, HIThemeDrawWindowFrame(&content, &wi, g_ctx, g_o, &title_rect));
		if (windows[i].type == kThemeDocumentWindow)
		{
			CHECK(title_rect.size.width == 40 && title_rect.origin.x == 70 && title_rect.size.height == 21);
			if (g_o == kHIThemeOrientationInverted)
				CHECK(title_rect.origin.y == 111 && title_rect.origin.y + title_rect.size.height <= 132);
			else
				CHECK(title_rect.origin.y == 29 && title_rect.origin.y + title_rect.size.height <= 50);
		}
		for (int widget = kThemeWidgetCloseBox; widget <= kThemeWidgetCollapseBox; widget++)
		{
			HIThemeWindowWidgetDrawInfo wd;
			memset(&wd, 0, sizeof(wd));
			wd.widgetState = kThemeStateActive;
			wd.widgetType = widget;
			wd.windowState = kThemeStateActive;
			wd.windowType = windows[i].type;
			wd.attributes = wi.attributes;
			snprintf(label, sizeof(label), "title bar widget %d of window type %d", widget, windows[i].type);
			if (windows[i].type == kThemeUtilitySideWindow)
				CHECK(HIThemeDrawTitleBarWidget(&content, &wd, g_ctx, g_o) == paramErr);
			else
				DRAW(label, CGRectMake(40, g_o == kHIThemeOrientationInverted ? 110 : 28, 100, 22), 0, 1, HIThemeDrawTitleBarWidget(&content, &wd, g_ctx, g_o));
		}
	}
	// dialogs have no frame of their own
	HIThemeWindowDrawInfo di;
	memset(&di, 0, sizeof(di));
	di.state = kThemeStateActive;
	di.windowType = kThemeDialogWindow;
	DRAW("dialog window frame", content, 0, 0, HIThemeDrawWindowFrame(&content, &di, g_ctx, g_o, NULL));
	di.windowType = 99;
	CHECK(HIThemeDrawWindowFrame(&content, &di, g_ctx, g_o, NULL) == paramErr);
}

static void test_tracks(void)
{
	char label[96];
	static const struct { int kind; double x, y, w, h; } tracks[] = {
		{ kThemeScrollBarMedium, 10, 20, 120, 20 }, { kThemeScrollBarSmall, 10, 20, 120, 20 },
		{ kThemeSliderMedium, 10, 20, 120, 20 }, { kThemeSliderSmall, 10, 20, 120, 20 }, { kThemeSliderMini, 10, 20, 120, 20 },
		{ kThemeProgressBarMedium, 10, 20, 120, 20 }, { kThemeProgressBarLarge, 10, 20, 120, 24 },
		{ kThemeIndeterminateBarMedium, 10, 20, 120, 20 }, { kThemeIndeterminateBarLarge, 10, 20, 120, 24 },
		{ kThemeRelevanceBar, 10, 20, 120, 24 },
	};
	for (unsigned i = 0; i < sizeof(tracks) / sizeof(tracks[0]); i++)
	{
		for (int horizontal = 0; horizontal <= 1; horizontal++)
		{
			HIThemeTrackDrawInfo ti;
			memset(&ti, 0, sizeof(ti));
			ti.kind = tracks[i].kind;
			ti.bounds = horizontal ? CGRectMake(tracks[i].x, tracks[i].y, tracks[i].w, tracks[i].h) : CGRectMake(tracks[i].y, tracks[i].x, tracks[i].h, tracks[i].w);
			ti.min = 0;
			ti.max = 100;
			ti.value = 40;
			ti.attributes = (horizontal ? kThemeTrackHorizontal : 0) | kThemeTrackShowThumb;
			ti.enableState = kThemeTrackActive;
			if (ti.kind == kThemeScrollBarMedium || ti.kind == kThemeScrollBarSmall)
				ti.trackInfo.scrollbar.viewsize = 25;
			HIRect bounds;
			CHECK(HIThemeGetTrackBounds(&ti, &bounds) == noErr);
			for (int enable = kThemeTrackActive; enable <= kThemeTrackInactive; enable++)
			{
				ti.enableState = enable;
				snprintf(label, sizeof(label), "track kind %d horizontal %d enable state %d", tracks[i].kind, horizontal, enable);
				DRAW(label, bounds, 1, 1, HIThemeDrawTrack(&ti, NULL, g_ctx, g_o));
			}
			ti.enableState = kThemeTrackActive;
			if (ti.kind == kThemeSliderMedium || ti.kind == kThemeSliderSmall || ti.kind == kThemeSliderMini)
			{
				snprintf(label, sizeof(label), "tick marks of track kind %d horizontal %d", tracks[i].kind, horizontal);
				DRAW(label, CGRectInset(bounds, -8, -8), 0, 1, HIThemeDrawTrackTickMarks(&ti, 5, g_ctx, g_o));
			}
			else
			{
				snprintf(label, sizeof(label), "no tick marks on track kind %d", tracks[i].kind);
				DRAW(label, bounds, 0, 0, HIThemeDrawTrackTickMarks(&ti, 5, g_ctx, g_o));
			}
		}
	}
	// a progress bar fills in proportion to its value
	HIThemeTrackDrawInfo pb;
	memset(&pb, 0, sizeof(pb));
	pb.kind = kThemeProgressBarMedium;
	pb.bounds = CGRectMake(10, 20, 120, 20);
	pb.max = 100;
	pb.attributes = kThemeTrackHorizontal;
	pb.enableState = kThemeTrackActive;
	int widths[2];
	for (int i = 0; i < 2; i++)
	{
		pb.value = i == 0 ? 20 : 80;
		clear_bitmap();
		CHECK(HIThemeDrawTrack(&pb, NULL, g_ctx, g_o) == noErr);
		widths[i] = 0;
		// the groove is thinner than the track bounds: take the row with the most fill pixels
		for (int y = 20; y < 40; y++)
		{
			int count = 0;
			for (int x = 0; x < W; x++)
			{
				int r, g, b, a;
				pixel_at(x, y, &r, &g, &b, &a);
				if (a != 0 && b > r + 60)
					count++;
			}
			if (count > widths[i])
				widths[i] = count;
		}
	}
	CHECK(widths[0] > 10 && widths[1] > widths[0] * 2);

	HIThemeTrackDrawInfo bad;
	memset(&bad, 0, sizeof(bad));
	bad.kind = kThemeScrollBarMini;
	bad.bounds = CGRectMake(10, 10, 20, 80);
	CHECK(HIThemeDrawTrack(&bad, NULL, g_ctx, g_o) == paramErr);
}

static void test_focus_and_errors(void)
{
	HIRect rect = CGRectMake(20, 20, 100, 40);
	CHECK(HIThemeBeginFocus(g_ctx, kHIThemeFocusRingOnly, NULL) == noErr);
	CHECK(HIThemeEndFocus(g_ctx) == noErr);
	CHECK(HIThemeBeginFocus(g_ctx, 77, NULL) == paramErr);
	CHECK(HIThemeBeginFocus(NULL, kHIThemeFocusRingOnly, NULL) == paramErr);
	CHECK(HIThemeEndFocus(NULL) == paramErr);

	HIThemeButtonDrawInfo bi;
	memset(&bi, 0, sizeof(bi));
	HIThemeFrameDrawInfo fi = { 0, 0, kThemeStateActive, 0 };
	HIThemeGroupBoxDrawInfo gi = { 0, kThemeStateActive, 0 };
	HIThemeSplitterDrawInfo sp = { 0, kThemeStateActive, 0 };
	HIThemeSeparatorDrawInfo se = { 0, kThemeStateActive };
	HIThemePlacardDrawInfo pl = { 0, kThemeStateActive };
	CHECK(HIThemeDrawButton(&rect, &bi, NULL, g_o, NULL) == paramErr);
	CHECK(HIThemeDrawButton(&rect, NULL, g_ctx, g_o, NULL) == paramErr);
	bi.kind = 999;
	CHECK(HIThemeDrawButton(&rect, &bi, g_ctx, g_o, NULL) == paramErr);
	CHECK(HIThemeDrawFrame(&rect, &fi, NULL, g_o) == paramErr);
	CHECK(HIThemeDrawFrame(&rect, NULL, g_ctx, g_o) == paramErr);
	CHECK(HIThemeDrawGroupBox(&rect, &gi, NULL, g_o) == paramErr);
	CHECK(HIThemeDrawGroupBox(&rect, NULL, g_ctx, g_o) == paramErr);
	CHECK(HIThemeDrawPaneSplitter(&rect, &sp, NULL, g_o) == paramErr);
	CHECK(HIThemeDrawSeparator(&rect, &se, NULL, g_o) == paramErr);
	CHECK(HIThemeDrawSeparator(&rect, NULL, g_ctx, g_o) == paramErr);
	CHECK(HIThemeDrawPlacard(&rect, &pl, NULL, g_o) == paramErr);
	CHECK(HIThemeDrawFocusRect(NULL, true, g_ctx, g_o) == paramErr);
	HIThemeTrackDrawInfo ti;
	memset(&ti, 0, sizeof(ti));
	CHECK(HIThemeDrawTrack(NULL, NULL, g_ctx, g_o) == paramErr);
	CHECK(HIThemeDrawTrack(&ti, NULL, NULL, g_o) == paramErr);
}

static void test_text(void)
{
	// Fonts are given explicitly, so the test does not depend on which UI font types CoreText supports.
	CTFontRef regular = CTFontCreateWithName(CFSTR("Helvetica"), 13, NULL);
	CTFontRef small = CTFontCreateWithName(CFSTR("Helvetica"), 10, NULL);
	CHECK(regular != NULL && small != NULL);
	if (regular == NULL || small == NULL)
		return;
	HIThemeTextInfo ti;
	memset(&ti, 0, sizeof(ti));
	ti.version = kHIThemeTextInfoVersionOne;
	ti.state = kThemeStateActive;
	ti.fontID = kThemeSpecifiedFont;
	ti.font = regular;
	ti.horizontalFlushness = kHIThemeTextHorizontalFlushDefault;
	ti.verticalFlushness = kHIThemeTextVerticalFlushDefault;
	CGFloat w = -1, h = -1, baseline = -1;
	CFStringRef text = CFSTR("Hello World");
	CHECK(HIThemeGetTextDimensions(text, 0, &ti, &w, &h, &baseline) == noErr);
	CHECK(w > 20 && h >= 10 && baseline > 0 && baseline < h);

	CGFloat w2, h2, b2;
	ti.font = small;
	CHECK(HIThemeGetTextDimensions(text, 0, &ti, &w2, &h2, &b2) == noErr);
	CHECK(w2 > 0 && w2 < w && h2 <= h);

	// wrapping into a narrow width produces more lines and a taller box
	ti.font = regular;
	CFStringRef long_text = CFSTR("Hello World Hello World Hello World");
	CGFloat wide_w, wide_h, wrap_w, wrap_h, bl;
	CHECK(HIThemeGetTextDimensions(long_text, 0, &ti, &wide_w, &wide_h, &bl) == noErr);
	CHECK(HIThemeGetTextDimensions(long_text, 90, &ti, &wrap_w, &wrap_h, &bl) == noErr);
	CHECK(wrap_w <= 90 && wrap_h >= wide_h * 2);

	// a single line that does not fit is truncated and says so
	ti.truncationPosition = kHIThemeTextTruncationEnd;
	ti.truncationMaxLines = 1;
	ti.truncationHappened = 0;
	CHECK(HIThemeGetTextDimensions(long_text, 60, &ti, &wrap_w, &wrap_h, &bl) == noErr);
	CHECK(ti.truncationHappened);
	CHECK(wrap_w <= 60 && wrap_h < wide_h * 2);
	ti.truncationPosition = kHIThemeTextTruncationNone;
	ti.truncationMaxLines = 0;

	HIRect box = CGRectMake(20, 20, 140, 30);
	DRAW("text box", box, 0, 1, HIThemeDrawTextBox(text, &box, &ti, g_ctx, g_o));
	ti.state = kThemeStateInactive;
	DRAW("inactive text box", box, 0, 1, HIThemeDrawTextBox(text, &box, &ti, g_ctx, g_o));
	ti.horizontalFlushness = kHIThemeTextHorizontalFlushRight;
	ti.verticalFlushness = kHIThemeTextVerticalFlushBottom;
	DRAW("right bottom text box", box, 0, 1, HIThemeDrawTextBox(text, &box, &ti, g_ctx, g_o));
	int x0, y0, x1, y1;
	CHECK(drawn_bounds(&x0, &y0, &x1, &y1) && x1 > box.origin.x + box.size.width - 6);
	// text is clipped to the box
	HIRect narrow = CGRectMake(20, 20, 30, 30);
	ti.horizontalFlushness = kHIThemeTextHorizontalFlushLeft;
	ti.verticalFlushness = kHIThemeTextVerticalFlushTop;
	DRAW("clipped text box", narrow, 0, 1, HIThemeDrawTextBox(long_text, &narrow, &ti, g_ctx, g_o));

	// theme font ids use the UI fonts of CoreText: the small system font is narrower than the system font
	HIThemeTextInfo ui = ti;
	ui.fontID = kThemeSystemFont;
	ui.font = NULL;
	CGFloat uw = 0, uh = 0, ub = 0, sw = 0, sh = 0, sb = 0;
	CHECK(HIThemeGetTextDimensions(text, 0, &ui, &uw, &uh, &ub) == noErr);
	ui.fontID = kThemeSmallSystemFont;
	CHECK(HIThemeGetTextDimensions(text, 0, &ui, &sw, &sh, &sb) == noErr);
	CHECK(uw > 20 && sw > 10 && sw < uw && sh <= uh && ub > 0);
	ui.fontID = kThemeSystemFont;
	DRAW("ui font text box", box, 0, 1, HIThemeDrawTextBox(text, &box, &ui, g_ctx, g_o));

	CHECK(HIThemeGetTextDimensions(NULL, 0, &ti, &w, &h, &baseline) == paramErr);
	CHECK(HIThemeGetTextDimensions(text, 0, NULL, &w, &h, &baseline) == paramErr);
	CHECK(HIThemeDrawTextBox(NULL, &box, &ti, g_ctx, g_o) == paramErr);
	CHECK(HIThemeDrawTextBox(text, NULL, &ti, g_ctx, g_o) == paramErr);
	CHECK(HIThemeDrawTextBox(text, &box, NULL, g_ctx, g_o) == paramErr);
	CHECK(HIThemeDrawTextBox(text, &box, &ti, NULL, g_o) == paramErr);
	CFRelease(regular);
	CFRelease(small);
}

int main(void)
{
	CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
	for (int o = 0; o <= 1; o++)
	{
		g_o = o == 0 ? kHIThemeOrientationNormal : kHIThemeOrientationInverted;
		g_ctx = CGBitmapContextCreate(NULL, W, H, 8, W * 4, cs, kCGImageAlphaPremultipliedLast);
		if (g_ctx == NULL)
		{
			printf("FAIL: no bitmap context\n");
			return 1;
		}
		g_pixels = CGBitmapContextGetData(g_ctx);
		if (g_o == kHIThemeOrientationNormal)
		{
			// a context with a top-left origin, as HIToolbox hands to a view
			CGContextTranslateCTM(g_ctx, 0, H);
			CGContextScaleCTM(g_ctx, 1, -1);
		}
		test_buttons();
		test_boxes_and_decorations();
		test_tabs_segments_menus();
		test_windows();
		test_tracks();
		test_focus_and_errors();
		test_text();
		CGContextRelease(g_ctx);
	}
	CGColorSpaceRelease(cs);
	if (g_failures != 0)
	{
		printf("hitheme_draw: %d failure(s)\n", g_failures);
		return 1;
	}
	printf("PASS hitheme_draw\n");
	return 0;
}
