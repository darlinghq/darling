/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

// HITheme: theme metrics, brushes and geometry queries of the classic Aqua controls.

#include <HIToolbox/HITheme.h>
#include <CoreText/CoreText.h>
#include <math.h>
#include <string.h>
#include "HIThemeData.h"

#ifndef paramErr
#define paramErr (-50)
#endif
// QUIRK: HIThemeBrushCreateCGColor returns paramErr for brush 0 and resNotFound (-192)
// for any other brush that does not exist, and HIThemeGetTextColorForThemeBrush returns
// themeNoAppropriateBrushErr (-30568) for a brush without a text colour. The
// codes are not documented. Evidence: black-box observation on macOS 10.14.4
// and 26.7.3 (tier 2), mac-probes/V655-hitheme.c.
// Test: hitheme_quirk_brush_error_codes
#define kHIThemeNoBrushErr (-192)
#define kHIThemeNoTextColorErr (-30568)

#define HITHEME_NSIZES(a) (sizeof(a) / sizeof((a)[0]))

// ---------------------------------------------------------------------------
// Small helpers
// ---------------------------------------------------------------------------

static int floorHalf(int v)
{
	return v >= 0 ? v / 2 : -((-v + 1) / 2);
}

typedef struct { CGFloat v[4]; } Color;

static Color rgba(CGFloat r, CGFloat g, CGFloat b, CGFloat a)
{
	Color c = { { r, g, b, a } };
	return c;
}

static Color gray(CGFloat w)
{
	return rgba(w, w, w, 1);
}

static void setFill(CGContextRef c, Color col)
{
	CGContextSetRGBFillColor(c, col.v[0], col.v[1], col.v[2], col.v[3]);
}

// ---------------------------------------------------------------------------
// Metrics
// ---------------------------------------------------------------------------

OSStatus GetThemeMetric(ThemeMetric inMetric, SInt32 *outMetric)
{
	if (outMetric == NULL)
		return paramErr;
	if (inMetric >= HITHEME_NSIZES(kMetricTable))
		return paramErr;
	*outMetric = kMetricTable[inMetric];
	return noErr;
}

OSStatus CopyThemeIdentifier(CFStringRef *outIdentifier)
{
	if (outIdentifier == NULL)
		return paramErr;
	*outIdentifier = kThemeAppearanceAquaBlue;
	return noErr;
}

OSStatus GetThemeMenuSeparatorHeight(SInt16 *outHeight)
{
	if (outHeight == NULL)
		return paramErr;
	*outHeight = 12;
	return noErr;
}

OSStatus GetThemeMenuItemExtra(ThemeMenuItemType inItemType, SInt16 *outHeight, SInt16 *outWidth)
{
	if (outHeight == NULL || outWidth == NULL)
		return paramErr;
	*outHeight = 2;
	*outWidth = (inItemType & 0xff) == kThemeMenuItemHierarchical ? 39 : 0;
	return noErr;
}

OSStatus GetThemeMenuTitleExtra(SInt16 *outWidth, Boolean inIsSquished)
{
	if (outWidth == NULL)
		return paramErr;
	*outWidth = inIsSquished ? 10 : 18;
	return noErr;
}

// ---------------------------------------------------------------------------
// Brushes
// ---------------------------------------------------------------------------

static const BrushEntry *findBrush(ThemeBrush brush)
{
	for (size_t i = 0; i < HITHEME_NSIZES(kBrushTable); i++)
	{
		if (kBrushTable[i].brush == brush)
			return &kBrushTable[i];
	}
	return NULL;
}

OSStatus HIThemeBrushCreateCGColor(ThemeBrush inBrush, CGColorRef *outColor)
{
	if (outColor == NULL)
		return paramErr;
	*outColor = NULL;
	if (inBrush == 0)
		return paramErr;

	const BrushEntry *entry = findBrush(inBrush);
	if (entry == NULL)
		return kHIThemeNoBrushErr;

	if (entry->components == 2)
		*outColor = CGColorCreateGenericGray(entry->values[0], entry->values[1]);
	else
		*outColor = CGColorCreateGenericRGB(entry->values[0], entry->values[1], entry->values[2], entry->values[3]);
	return *outColor != NULL ? noErr : paramErr;
}

OSStatus HIThemeGetTextColorForThemeBrush(ThemeBrush inBrush, Boolean inWindowIsActive, ThemeTextColor *outColor)
{
	if (outColor == NULL)
		return paramErr;
	const BrushEntry *entry = findBrush(inBrush);
	int16_t text = entry != NULL ? (inWindowIsActive ? entry->textColorActive : entry->textColorInactive) : 0;
	if (text == 0)
	{
		*outColor = 0;
		return kHIThemeNoTextColorErr;
	}
	*outColor = text;
	return noErr;
}

static OSStatus applyBrush(ThemeBrush brush, CGContextRef c, Boolean stroke)
{
	if (c == NULL)
		return paramErr;
	CGColorRef color = NULL;
	OSStatus status = HIThemeBrushCreateCGColor(brush, &color);
	if (status != noErr)
		return status;
	if (stroke)
		CGContextSetStrokeColorWithColor(c, color);
	else
		CGContextSetFillColorWithColor(c, color);
	CGColorRelease(color);
	return noErr;
}

OSStatus HIThemeSetFill(ThemeBrush inBrush, void *inInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	return applyBrush(inBrush, inContext, false);
}

OSStatus HIThemeSetStroke(ThemeBrush inBrush, void *inInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	return applyBrush(inBrush, inContext, true);
}

OSStatus HIThemeSetTextFill(ThemeTextColor inColor, void *inInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	if (inContext == NULL)
		return paramErr;
	Color col;
	switch (inColor)
	{
		case kThemeTextColorBlack: col = gray(0); break;
		case kThemeTextColorWhite: col = gray(1); break;
		case kThemeTextColorDialogInactive:
		case kThemeTextColorAlertInactive:
		case kThemeTextColorModelessDialogInactive:
		case kThemeTextColorWindowHeaderInactive:
		case kThemeTextColorPlacardInactive:
		case kThemeTextColorPushButtonInactive:
		case kThemeTextColorBevelButtonInactive:
		case kThemeTextColorPopupButtonInactive:
		case kThemeTextColorDocumentWindowTitleInactive:
		case kThemeTextColorMovableModalWindowTitleInactive:
		case kThemeTextColorUtilityWindowTitleInactive:
		case kThemeTextColorPopupWindowTitleInactive:
		case kThemeTextColorRootMenuDisabled:
		case kThemeTextColorMenuItemDisabled:
		case kThemeTextColorPopupLabelInactive:
		case kThemeTextColorTabFrontInactive:
		case kThemeTextColorTabNonFrontInactive:
		case kThemeTextColorBevelButtonStickyInactive:
			col = rgba(0, 0, 0, 0.5);
			break;
		case kThemeTextColorRootMenuSelected:
		case kThemeTextColorMenuItemSelected:
		case kThemeTextColorPushButtonPressed:
			col = gray(1);
			break;
		default:
			if (inColor < kThemeTextColorBlack || inColor > kThemeTextColorSystemDetail)
				return paramErr;
			col = rgba(0, 0, 0, 0.85);
			break;
	}
	setFill(inContext, col);
	return noErr;
}

// ---------------------------------------------------------------------------
// Axis based geometry (table driven)
// ---------------------------------------------------------------------------

static void evalAxis(const AxisSpec *s, int extent, int *offset, int *size)
{
	switch (s->mode)
	{
		case AX_FIXED:
			*offset = s->a;
			*size = s->s;
			break;
		case AX_CENTER:
			*offset = floorHalf(extent - s->s + s->r) + s->a;
			*size = s->s;
			break;
		case AX_END:
			*offset = extent + s->a;
			*size = s->s;
			break;
		default: // AX_INSET
			if (s->t != 0 && extent < s->t)
			{
				*size = s->sc;
				*offset = floorHalf(extent - s->n + s->r);
			}
			else
			{
				*offset = s->a;
				*size = extent + s->s;
			}
			break;
	}
}

// The content and background rectangles never have a negative size; the label rectangle of a
// button smaller than its margins keeps the negative size. With shiftBack the position of a
// rectangle narrower than its margins also moves back by the shortfall.
static CGRect specRectClamped(const RectSpec *spec, const HIRect *in, Boolean clampSize, Boolean shiftBack)
{
	if (spec->x.mode == AX_EMPTY || spec->y.mode == AX_EMPTY)
		return CGRectZero;
	int ox, sx, oy, sy;
	evalAxis(&spec->x, (int) lround(in->size.width), &ox, &sx);
	evalAxis(&spec->y, (int) lround(in->size.height), &oy, &sy);
	if (clampSize)
	{
		if (sx < 0)
		{
			if (shiftBack)
				ox += sx;
			sx = 0;
		}
		if (sy < 0) sy = 0;
	}
	return CGRectMake(in->origin.x + ox, in->origin.y + oy, sx, sy);
}

// The rule of a button rectangle for the height of the passed rectangle: a height
// override of the kind if there is one, else the rule of the kind.
static CGRect buttonRect(const HIThemeButtonDrawInfo *info, int part, const RectSpec *rule, const HIRect *in, Boolean clampSize)
{
	// The content rectangle of the push button kinds (normal, textured and the plain kinds sharing
	// their geometry) moves back when the button is narrower than its margins; other kinds keep
	// their position.
	Boolean shiftBack = part == 0 && (info->kind == kThemePushButton || info->kind == kThemePushButtonNormal ||
		info->kind == kThemePushButtonTextured || info->kind == kThemePushButtonTexturedSmall);
	int height = (int) lround(in->size.height);
	for (size_t i = 0; i < HITHEME_NSIZES(kButtonHeightOverrides); i++)
	{
		const HeightOverride *o = &kButtonHeightOverrides[i];
		if (o->kind == info->kind && o->part == part && height >= o->hmin && height <= o->hmax)
			return specRectClamped(&o->spec, in, clampSize, shiftBack);
	}
	return specRectClamped(rule, in, clampSize, shiftBack);
}

// ---------------------------------------------------------------------------
// Buttons
// ---------------------------------------------------------------------------

static const ButtonKindSpec *buttonSpec(const HIThemeButtonDrawInfo *info)
{
	if (info->kind >= HITHEME_NSIZES(kButtonKinds))
		return NULL;
	return &kButtonKinds[info->kind];
}

OSStatus HIThemeGetButtonContentBounds(const HIRect *inBounds, const HIThemeButtonDrawInfo *inDrawInfo, HIRect *outBounds)
{
	if (inBounds == NULL || inDrawInfo == NULL || outBounds == NULL)
		return paramErr;
	const ButtonKindSpec *spec = buttonSpec(inDrawInfo);
	if (spec == NULL)
		return paramErr;
	*outBounds = buttonRect(inDrawInfo, 0, &spec->content, inBounds, true);
	return noErr;
}

OSStatus HIThemeGetButtonBackgroundBounds(const HIRect *inBounds, const HIThemeButtonDrawInfo *inDrawInfo, HIRect *outBounds)
{
	if (inBounds == NULL || inDrawInfo == NULL || outBounds == NULL)
		return paramErr;
	const ButtonKindSpec *spec = buttonSpec(inDrawInfo);
	if (spec == NULL)
		return paramErr;
	*outBounds = buttonRect(inDrawInfo, 1, &spec->background, inBounds, true);
	return noErr;
}

OSStatus HIThemeGetButtonShape(const HIRect *inBounds, const HIThemeButtonDrawInfo *inDrawInfo, HIShapeRef *outShape)
{
	if (inBounds == NULL || inDrawInfo == NULL || outShape == NULL)
		return paramErr;
	const ButtonKindSpec *spec = buttonSpec(inDrawInfo);
	if (spec == NULL)
		return paramErr;
	CGRect r = buttonRect(inDrawInfo, 1, &spec->background, inBounds, true);
	*outShape = HIShapeCreateWithRect(&r);
	return *outShape != NULL ? noErr : paramErr;
}

OSStatus HIThemeGetGrowBoxBounds(const HIPoint *inOrigin, const HIThemeGrowBoxDrawInfo *inDrawInfo, HIRect *outBounds)
{
	if (inOrigin == NULL || inDrawInfo == NULL || outBounds == NULL)
		return paramErr;
	SInt32 size;
	OSStatus status = GetThemeMetric(inDrawInfo->size == kHIThemeGrowBoxSizeSmall ? kThemeMetricSmallResizeControlHeight : kThemeMetricResizeControlHeight, &size);
	if (status != noErr)
		return status;
	*outBounds = CGRectMake(inOrigin->x, inOrigin->y, size, size);
	return noErr;
}

static OSStatus tabPaneShape(const HIRect *inRect, ThemeTabDirection direction, HIShapeRef *outShape, Boolean content)
{
	if (inRect == NULL || outShape == NULL || direction > kThemeTabWest)
		return paramErr;
	CGRect r = *inRect;
	if (content)
	{
		switch (direction)
		{
			case kThemeTabNorth: r = CGRectMake(r.origin.x, r.origin.y + 10, r.size.width, r.size.height - 10); break;
			case kThemeTabSouth: r.size.height -= 10; break;
			case kThemeTabEast: r.size.width -= 10; break;
			default: r = CGRectMake(r.origin.x + 10, r.origin.y, r.size.width - 10, r.size.height); break;
		}
	}
	else
	{
		r = CGRectMake(r.origin.x - 8, r.origin.y - 6, r.size.width + 17, r.size.height + 14);
	}
	*outShape = HIShapeCreateWithRect(&r);
	return *outShape != NULL ? noErr : paramErr;
}

OSStatus HIThemeGetTabPaneDrawShape(const HIRect *inRect, ThemeTabDirection inDirection, HIThemeTabSize inTabSize, HIShapeRef *outShape)
{
	return tabPaneShape(inRect, inDirection, outShape, false);
}

OSStatus HIThemeGetTabPaneContentShape(const HIRect *inRect, ThemeTabDirection inDirection, HIThemeTabSize inTabSize, HIShapeRef *outShape)
{
	return tabPaneShape(inRect, inDirection, outShape, true);
}

OSStatus HIThemeGetTabDrawShape(const HIRect *inRect, const HIThemeTabDrawInfo *inDrawInfo, HIShapeRef *outShape)
{
	if (inRect == NULL || inDrawInfo == NULL || outShape == NULL)
		return paramErr;
	if (inDrawInfo->direction > kThemeTabWest)
		return paramErr;
	*outShape = HIShapeCreateWithRect(inRect);
	return *outShape != NULL ? noErr : paramErr;
}

OSStatus HIThemeGetTabShape(const HIRect *inRect, const HIThemeTabDrawInfo *inDrawInfo, HIShapeRef *outShape)
{
	return HIThemeGetTabDrawShape(inRect, inDrawInfo, outShape);
}

static CGRect menuBackgroundRect(const HIRect *menuRect)
{
	return CGRectMake(menuRect->origin.x, menuRect->origin.y - 4, menuRect->size.width, menuRect->size.height + 8);
}

OSStatus HIThemeGetMenuBackgroundShape(const HIRect *inMenuRect, const HIThemeMenuDrawInfo *inMenuDrawInfo, HIShapeRef *outShape)
{
	if (inMenuRect == NULL || inMenuDrawInfo == NULL || outShape == NULL)
		return paramErr;
	CGRect r = menuBackgroundRect(inMenuRect);
	*outShape = HIShapeCreateWithRect(&r);
	return *outShape != NULL ? noErr : paramErr;
}

// ---------------------------------------------------------------------------
// Windows
// ---------------------------------------------------------------------------

typedef struct {
	CGFloat title;       // height of the title bar above the content
	CGFloat side;        // border to the left and right of the content
	CGFloat widget;      // width of a title bar widget box
	CGFloat widgetStep;  // distance between widgets
	CGFloat widgetLeft;  // first widget from the left edge
	Boolean hasTitle;
} WindowMetrics;

static Boolean windowMetricsFor(ThemeWindowType type, WindowMetrics *m)
{
	memset(m, 0, sizeof(*m));
	switch (type)
	{
		case kThemeDocumentWindow:
			m->title = 22; m->widget = 19; m->widgetStep = 22; m->widgetLeft = 6; m->hasTitle = true;
			return true;
		case kThemeMovableDialogWindow:
		case kThemeMovableAlertWindow:
			m->title = 22; m->side = 4; m->widget = 19; m->widgetStep = 22; m->widgetLeft = 6; m->hasTitle = true;
			return true;
		case kThemeUtilityWindow:
			m->title = 16; m->widget = 11; m->widgetStep = 18; m->widgetLeft = 6; m->hasTitle = true;
			return true;
		case kThemeUtilitySideWindow:
			m->side = 16; m->widget = 11; m->widgetStep = 18; m->widgetLeft = 6;
			return true;
		case kThemeDialogWindow:
		case kThemeAlertWindow:
		case kThemeSheetWindow:
		case kThemeDrawerWindow:
			return true;
		default:
			// The plain dialog, shadow dialog and popup window types are rejected with paramErr.
			return false;
	}
}

// The queries have no orientation and place the title bar above the content in the
// y-down frame; for an inverted context the bar lies above the content in the y-up
// frame, that is, at larger y.
static CGRect windowStructure(const HIRect *content, const WindowMetrics *m, HIThemeOrientation orientation)
{
	CGFloat y = orientation == kHIThemeOrientationInverted ? content->origin.y : content->origin.y - m->title;
	return CGRectMake(content->origin.x - m->side, y, content->size.width + 2 * m->side, content->size.height + m->title);
}

// Rectangle of the title bar widget with the given index (0 close, 1 collapse, 2 zoom).
static CGRect windowWidgetRect(const HIRect *content, const WindowMetrics *m, int index, HIThemeOrientation orientation)
{
	CGFloat h = m->widget == 19 ? (index == 1 ? 19 : 18) : 12;
	CGFloat x = content->origin.x + m->widgetLeft + index * m->widgetStep;
	CGFloat y = orientation == kHIThemeOrientationInverted ? content->origin.y + content->size.height + m->title - 2 - h
	                                                       : content->origin.y - m->title + 2;
	return CGRectMake(x, y, m->widget, h);
}

OSStatus HIThemeGetWindowShape(const HIRect *inContRect, const HIThemeWindowDrawInfo *inDrawInfo, WindowRegionCode inWinRegion, HIShapeRef *outShape)
{
	if (inContRect == NULL || inDrawInfo == NULL || outShape == NULL)
		return paramErr;
	WindowMetrics m;
	if (!windowMetricsFor(inDrawInfo->windowType, &m))
		return paramErr;
	CGRect r = CGRectZero;
	switch (inWinRegion)
	{
		case kAppearanceRegionTitleBar:
		case kAppearanceRegionDrag:
			if (m.title > 0)
				r = CGRectMake(inContRect->origin.x - m.side, inContRect->origin.y - m.title, inContRect->size.width + 2 * m.side, m.title);
			break;
		// The widget regions do not depend on the window attributes.
		case kAppearanceRegionCloseBox:
			if (m.title > 0)
				r = windowWidgetRect(inContRect, &m, 0, kHIThemeOrientationNormal);
			break;
		case kAppearanceRegionCollapseBox:
			if (m.title > 0)
				r = windowWidgetRect(inContRect, &m, 1, kHIThemeOrientationNormal);
			break;
		case kAppearanceRegionZoomBox:
			if (m.title > 0)
				r = windowWidgetRect(inContRect, &m, 2, kHIThemeOrientationNormal);
			break;
		case kAppearanceRegionTitleProxyIcon:
		case kAppearanceRegionStructure:
			r = windowStructure(inContRect, &m, kHIThemeOrientationNormal);
			break;
		case kAppearanceRegionContent:
			r = *inContRect;
			break;
		default:
			break;
	}
	*outShape = HIShapeCreateWithRect(&r);
	return *outShape != NULL ? noErr : paramErr;
}

Boolean HIThemeGetWindowRegionHit(const HIRect *inContRect, const HIThemeWindowDrawInfo *inDrawInfo, const HIPoint *inPoint, WindowRegionCode *outRegionHit)
{
	if (inContRect == NULL || inDrawInfo == NULL || inPoint == NULL || outRegionHit == NULL)
		return false;
	// The small regions come first because they lie inside the larger ones.
	static const WindowRegionCode order[] = {
		kAppearanceRegionCloseBox, kAppearanceRegionZoomBox, kAppearanceRegionCollapseBox,
		kAppearanceRegionTitleBar, kAppearanceRegionContent, kAppearanceRegionStructure
	};
	for (size_t i = 0; i < HITHEME_NSIZES(order); i++)
	{
		HIShapeRef shape = NULL;
		if (HIThemeGetWindowShape(inContRect, inDrawInfo, order[i], &shape) != noErr)
			return false;
		Boolean hit = HIShapeContainsPoint(shape, inPoint);
		CFRelease(shape);
		if (hit)
		{
			*outRegionHit = order[i];
			return true;
		}
	}
	return false;
}

// ---------------------------------------------------------------------------
// Tracks: scroll bars, sliders and progress bars
//
// The geometry is computed in the coordinate space of the track bounds (the
// value grows with the coordinate along the track); the drawing code converts
// to the local frame afterwards.
// ---------------------------------------------------------------------------

typedef struct {
	Boolean horizontal;
	Boolean scrollBar;
	Boolean slider;
	CGRect bounds;    // overall rectangle including shadows
	CGRect body;      // the bar itself at its natural thickness
	CGRect track;     // rectangle the thumb moves in
	CGRect groove;    // visible groove of a slider or progress bar
	CGRect thumb;
	CGRect pageA, pageB, arrowA, arrowB;
	CGFloat travel;   // distance the thumb can move along the track
	Boolean hasThumb;
	Boolean hasArrows;
} TrackLayout;

// Rectangle taking its position and size across the track from "cross" and its start and length along it from the arguments.
static CGRect alongRect(CGRect cross, Boolean horizontal, CGFloat start, CGFloat length)
{
	if (length < 0)
		length = 0;
	return horizontal ? CGRectMake(start, cross.origin.y, length, cross.size.height) : CGRectMake(cross.origin.x, start, cross.size.width, length);
}

static OSStatus layoutTrack(const HIThemeTrackDrawInfo *info, TrackLayout *L)
{
	memset(L, 0, sizeof(*L));
	Boolean h = (info->attributes & kThemeTrackHorizontal) != 0;
	L->horizontal = h;
	CGRect r = info->bounds;
	CGFloat thickness, minThumb = 0, thumbLength = 0, grooveThickness = 0;
	CGFloat outsetAlong = 0, outsetAcross = 0;
	CGFloat crossShift = 0;   // offset of the bar from the start of the rectangle across the track
	CGFloat thumbInset = 0;   // the thumb is this much thinner than the bar
	SInt32 metric = 0;

	switch (info->kind)
	{
		case kThemeScrollBarMedium:
		case kThemeScrollBarSmall:
		{
			L->scrollBar = true;
			Boolean small = info->kind == kThemeScrollBarSmall;
			GetThemeMetric(small ? kThemeMetricSmallScrollBarWidth : kThemeMetricScrollBarWidth, &metric);
			thickness = metric;
			GetThemeMetric(small ? (h ? kThemeMetricSmallScrollBarMinThumbWidth : kThemeMetricSmallScrollBarMinThumbHeight)
			                     : (h ? kThemeMetricScrollBarMinThumbWidth : kThemeMetricScrollBarMinThumbHeight), &metric);
			minThumb = metric;
			// The x extent of every bar is outset by one pixel, whatever its direction.
			outsetAlong = h ? 1 : 0;
			outsetAcross = h ? 0 : 1;
			L->hasArrows = !(info->attributes & kThemeTrackNoScrollBarArrows);
			break;
		}
		case kThemeSliderMedium:
		case kThemeSliderSmall:
		case kThemeSliderMini:
		{
			L->slider = true;
			static const CGFloat sizes[3][3] = { { 21, 23, 7 }, { 16, 17, 5 }, { 12, 14, 4 } };
			int s = info->kind == kThemeSliderMedium ? 0 : (info->kind == kThemeSliderSmall ? 1 : 2);
			thickness = sizes[s][0];
			if (s == 1)
			{
				// The small slider sits one pixel down and its thumb is one pixel thinner than the bar.
				crossShift = 1;
				thumbInset = 1;
			}
			thumbLength = sizes[s][1];
			grooveThickness = sizes[s][2];
			CGFloat outset = s == 0 ? 3 : (s == 1 ? 2 : 1);
			outsetAlong = h ? outset : 1;
			outsetAcross = h ? 0 : outset;
			break;
		}
		case kThemeProgressBarMedium:
		case kThemeIndeterminateBarMedium:
		case kThemeProgressBarLarge:
		case kThemeIndeterminateBarLarge:
		case kThemeRelevanceBar:
		{
			Boolean large = info->kind == kThemeProgressBarLarge || info->kind == kThemeIndeterminateBarLarge;
			Boolean relevance = info->kind == kThemeRelevanceBar;
			GetThemeMetric(large ? kThemeMetricLargeProgressBarThickness : kThemeMetricNormalProgressBarThickness, &metric);
			thickness = (relevance || large) ? 20 : metric;
			grooveThickness = relevance ? 20 : 6;
			outsetAlong = h ? 1 : 0;
			outsetAcross = h ? 0 : 1;
			break;
		}
		default:
			// The mini scroll bar and the mini progress bars do not exist in the classic theme.
			return paramErr;
	}

	CGFloat start = h ? r.origin.x : r.origin.y;
	CGFloat length = h ? r.size.width : r.size.height;
	CGRect cross = r;
	if (h)
	{
		cross.origin.y += crossShift;
		cross.size.height = thickness;
	}
	else
	{
		cross.origin.x += crossShift;
		cross.size.width = thickness;
	}
	L->body = cross;
	L->bounds = h ? CGRectMake(start - outsetAlong, cross.origin.y - outsetAcross, length + 2 * outsetAlong, cross.size.height + 2 * outsetAcross)
	              : CGRectMake(cross.origin.x - outsetAcross, start - outsetAlong, cross.size.width + 2 * outsetAcross, length + 2 * outsetAlong);

	// The groove of a slider or progress bar is centred in its thickness.
	CGRect grooveCross = cross;
	if (!L->scrollBar)
	{
		if (h)
		{
			grooveCross.origin.y = cross.origin.y + floor((thickness - grooveThickness) / 2);
			grooveCross.size.height = grooveThickness;
		}
		else
		{
			grooveCross.origin.x = cross.origin.x + floor((thickness - grooveThickness) / 2);
			grooveCross.size.width = grooveThickness;
		}
	}

	SInt32 range = info->max - info->min;
	CGFloat fraction = 0;
	if (range > 0)
	{
		SInt32 v = info->value < info->min ? info->min : (info->value > info->max ? info->max : info->value);
		fraction = (CGFloat) (v - info->min) / range;
	}

	if (L->scrollBar)
	{
		// Distance of the arrow buttons from the end of the bar, and their length.
		static const CGFloat arrowAFromEnd[2] = { 44, 34 }, arrowALen[2] = { 27, 21 };
		static const CGFloat arrowBFromEnd[2] = { 17, 13 }, arrowBLen[2] = { 17, 13 };
		static const CGFloat trailing[2] = { 30, 24 };
		int s = info->kind == kThemeScrollBarSmall ? 1 : 0;
		CGFloat end = start + length;
		CGFloat trackStart = start + 4;
		CGFloat trackEnd = L->hasArrows ? end - trailing[s] : end - 4;
		if (trackEnd < trackStart)
			trackEnd = trackStart;
		L->track = alongRect(cross, h, trackStart, trackEnd - trackStart);
		L->groove = L->track;
		if (L->hasArrows)
		{
			L->arrowA = alongRect(cross, h, end - arrowAFromEnd[s], arrowALen[s]);
			L->arrowB = alongRect(cross, h, end - arrowBFromEnd[s], arrowBLen[s]);
		}
		CGFloat view = info->trackInfo.scrollbar.viewsize;
		CGFloat trackLen = trackEnd - trackStart;
		CGFloat len = (range + view) > 0 ? trackLen * view / (range + view) : trackLen;
		if (len < minThumb)
			len = minThumb;
		if (len > trackLen)
			len = trackLen;
		L->hasThumb = (info->attributes & kThemeTrackShowThumb) != 0 && info->enableState != kThemeTrackNothingToScroll;
		L->travel = trackLen - len;
		CGFloat thumbPos = floor(L->travel * fraction + 0.5);
		L->thumb = alongRect(cross, h, trackStart + thumbPos, len);
		L->pageA = alongRect(cross, h, trackStart, thumbPos);
		L->pageB = alongRect(cross, h, trackStart + thumbPos + len, trackLen - thumbPos - len);
	}
	else if (L->slider)
	{
		L->track = alongRect(grooveCross, h, start, length);
		L->groove = L->track;
		L->travel = length - thumbLength - 3;
		if (L->travel < 0)
			L->travel = 0;
		CGFloat thumbPos = floor(L->travel * fraction);
		CGRect thumbCross = cross;
		if (h)
		{
			thumbCross.origin.y += thumbInset;
			thumbCross.size.height -= thumbInset;
		}
		else
		{
			thumbCross.origin.x += thumbInset;
			thumbCross.size.width -= thumbInset;
		}
		L->thumb = alongRect(thumbCross, h, start + thumbPos, thumbLength);
		L->hasThumb = (info->attributes & kThemeTrackShowThumb) != 0;
		L->pageA = alongRect(grooveCross, h, start, thumbPos);
		L->pageB = alongRect(grooveCross, h, start + thumbPos + thumbLength, length - thumbPos - thumbLength);
	}
	else
	{
		L->track = alongRect(grooveCross, h, start, length);
		L->groove = L->track;
		L->travel = length;
	}
	return noErr;
}

OSStatus HIThemeGetTrackBounds(const HIThemeTrackDrawInfo *inDrawInfo, HIRect *outBounds)
{
	if (inDrawInfo == NULL || outBounds == NULL)
		return paramErr;
	TrackLayout L;
	OSStatus status = layoutTrack(inDrawInfo, &L);
	if (status != noErr)
		return status;
	*outBounds = L.bounds;
	return noErr;
}

OSStatus HIThemeGetTrackThumbShape(const HIThemeTrackDrawInfo *inDrawInfo, HIShapeRef *outThumbShape)
{
	if (inDrawInfo == NULL || outThumbShape == NULL)
		return paramErr;
	TrackLayout L;
	OSStatus status = layoutTrack(inDrawInfo, &L);
	if (status != noErr)
		return status;
	CGRect thumb = L.hasThumb ? L.thumb : CGRectZero;
	*outThumbShape = HIShapeCreateWithRect(&thumb);
	return *outThumbShape != NULL ? noErr : paramErr;
}

OSStatus HIThemeGetTrackDragRect(const HIThemeTrackDrawInfo *inDrawInfo, HIRect *outDragRect)
{
	if (inDrawInfo == NULL || outDragRect == NULL)
		return paramErr;
	TrackLayout L;
	OSStatus status = layoutTrack(inDrawInfo, &L);
	if (status != noErr)
		return status;
	*outDragRect = L.scrollBar ? L.body : L.groove;
	return noErr;
}

static Boolean partRect(const TrackLayout *L, ControlPartCode part, CGRect *out)
{
	switch (part)
	{
		case kAppearancePartUpButton: if (!L->hasArrows) return false; *out = L->arrowA; return true;
		case kAppearancePartDownButton: if (!L->hasArrows) return false; *out = L->arrowB; return true;
		case kAppearancePartPageUpArea: if (!L->scrollBar && !L->slider) return false; *out = L->pageA; return true;
		case kAppearancePartPageDownArea: if (!L->scrollBar && !L->slider) return false; *out = L->pageB; return true;
		case kAppearancePartIndicator: if (!L->hasThumb) return false; *out = L->thumb; return true;
		default: return false;
	}
}

OSStatus HIThemeGetTrackPartBounds(const HIThemeTrackDrawInfo *inDrawInfo, ControlPartCode inPartCode, HIRect *outPartBounds)
{
	if (inDrawInfo == NULL || outPartBounds == NULL)
		return paramErr;
	TrackLayout L;
	OSStatus status = layoutTrack(inDrawInfo, &L);
	if (status != noErr)
		return status;
	CGRect r;
	if (partRect(&L, inPartCode, &r))
	{
		*outPartBounds = r;
		return noErr;
	}
	// QUIRK: for a scroll bar, a part code the bar does not have returns noErr with the
	// rectangle of the whole bar instead of an error; other tracks return an error. Not
	// documented. Evidence: black-box observation on macOS 10.14.4 and 26.7.3 (tier 2),
	// mac-probes/V655-hitheme.c.
	// Test: hitheme_quirk_unknown_scrollbar_part_is_whole_bar
	if (L.scrollBar)
	{
		*outPartBounds = L.body;
		return noErr;
	}
	return paramErr;
}

OSStatus HIThemeGetTrackParts(const HIThemeTrackDrawInfo *inDrawInfo, UInt32 *outNumberOfParts, UInt32 inMaxParts, ControlPartCode *ioPartsBuffer)
{
	if (inDrawInfo == NULL || outNumberOfParts == NULL)
		return paramErr;
	TrackLayout L;
	OSStatus status = layoutTrack(inDrawInfo, &L);
	if (status != noErr)
		return status;
	// Only scroll bars enumerate their parts.
	if (!L.scrollBar)
	{
		*outNumberOfParts = 0;
		return paramErr;
	}
	ControlPartCode parts[5];
	UInt32 n = 0;
	parts[n++] = kAppearancePartPageUpArea;
	if (L.hasThumb)
		parts[n++] = kAppearancePartIndicator;
	parts[n++] = kAppearancePartPageDownArea;
	if (L.hasArrows)
	{
		parts[n++] = kAppearancePartUpButton;
		parts[n++] = kAppearancePartDownButton;
	}
	*outNumberOfParts = n;
	if (ioPartsBuffer != NULL)
	{
		for (UInt32 i = 0; i < n && i < inMaxParts; i++)
			ioPartsBuffer[i] = parts[i];
	}
	return noErr;
}

Boolean HIThemeHitTestTrack(const HIThemeTrackDrawInfo *inDrawInfo, const HIPoint *inMousePoint, ControlPartCode *outPartHit)
{
	if (outPartHit != NULL)
		*outPartHit = 0;
	if (inDrawInfo == NULL || inMousePoint == NULL || outPartHit == NULL)
		return false;
	TrackLayout L;
	if (layoutTrack(inDrawInfo, &L) != noErr)
		return false;
	// The arrows and the thumb lie over the page areas.
	static const ControlPartCode order[] = {
		kAppearancePartUpButton, kAppearancePartDownButton, kAppearancePartIndicator,
		kAppearancePartPageUpArea, kAppearancePartPageDownArea
	};
	for (size_t i = 0; i < HITHEME_NSIZES(order); i++)
	{
		CGRect r;
		if (partRect(&L, order[i], &r) && CGRectContainsPoint(r, *inMousePoint))
		{
			*outPartHit = order[i];
			return true;
		}
	}
	return false;
}

// Thumb offset, in pixels from the start of the track, for a coordinate along the track.
static OSStatus thumbOffsetFromCoordinate(const HIThemeTrackDrawInfo *info, CGFloat coordinate, CGFloat *out)
{
	TrackLayout L;
	OSStatus status = layoutTrack(info, &L);
	if (status != noErr)
		return status;
	CGFloat trackStart = L.horizontal ? L.track.origin.x : L.track.origin.y;
	CGFloat v = coordinate - trackStart;
	if (v < 0) v = 0;
	if (v > L.travel) v = L.travel;
	*out = v;
	return noErr;
}

OSStatus HIThemeGetTrackThumbPositionFromOffset(const HIThemeTrackDrawInfo *inDrawInfo, const HIPoint *inThumbOffset, CGFloat *outRelativePosition)
{
	if (inDrawInfo == NULL || inThumbOffset == NULL || outRelativePosition == NULL)
		return paramErr;
	Boolean h = (inDrawInfo->attributes & kThemeTrackHorizontal) != 0;
	return thumbOffsetFromCoordinate(inDrawInfo, h ? inThumbOffset->x : inThumbOffset->y, outRelativePosition);
}

OSStatus HIThemeGetTrackThumbPositionFromBounds(const HIThemeTrackDrawInfo *inDrawInfo, const HIRect *inThumbBounds, CGFloat *outRelativePosition)
{
	if (inDrawInfo == NULL || inThumbBounds == NULL || outRelativePosition == NULL)
		return paramErr;
	Boolean h = (inDrawInfo->attributes & kThemeTrackHorizontal) != 0;
	return thumbOffsetFromCoordinate(inDrawInfo, h ? inThumbBounds->origin.x : inThumbBounds->origin.y, outRelativePosition);
}

OSStatus HIThemeGetTrackLiveValue(const HIThemeTrackDrawInfo *inDrawInfo, CGFloat inRelativePosition, SInt32 *outValue)
{
	if (inDrawInfo == NULL || outValue == NULL)
		return paramErr;
	TrackLayout L;
	OSStatus status = layoutTrack(inDrawInfo, &L);
	if (status != noErr)
		return status;
	SInt32 range = inDrawInfo->max - inDrawInfo->min;
	if (L.travel <= 0 || range <= 0)
	{
		*outValue = inDrawInfo->min;
		return noErr;
	}
	CGFloat p = inRelativePosition < 0 ? 0 : (inRelativePosition > L.travel ? L.travel : inRelativePosition);
	*outValue = inDrawInfo->min + (SInt32) floor(p * range / L.travel + 0.5);
	return noErr;
}

// Track info of a scroll bar given by its bounds.
static void scrollBarInfo(const HIRect *bounds, const HIScrollBarTrackInfo *trackInfo, Boolean horizontal, HIThemeTrackDrawInfo *info)
{
	memset(info, 0, sizeof(*info));
	CGFloat thickness = horizontal ? bounds->size.height : bounds->size.width;
	info->kind = thickness <= 12 ? kThemeScrollBarSmall : kThemeScrollBarMedium;
	info->bounds = *bounds;
	info->attributes = (horizontal ? kThemeTrackHorizontal : 0) | kThemeTrackShowThumb;
	info->enableState = trackInfo->enableState;
}

OSStatus HIThemeGetScrollBarTrackRect(const HIRect *inBounds, const HIScrollBarTrackInfo *inTrackInfo, Boolean inIsHoriz, HIRect *outTrackBounds)
{
	if (inBounds == NULL || inTrackInfo == NULL || outTrackBounds == NULL)
		return paramErr;
	HIThemeTrackDrawInfo info;
	scrollBarInfo(inBounds, inTrackInfo, inIsHoriz, &info);
	TrackLayout L;
	OSStatus status = layoutTrack(&info, &L);
	if (status != noErr)
		return status;
	*outTrackBounds = L.track;
	return noErr;
}

Boolean HIThemeHitTestScrollBarArrows(const HIRect *inScrollBarBounds, const HIScrollBarTrackInfo *inTrackInfo, Boolean inIsHoriz, const HIPoint *inPtHit, HIRect *outTrackBounds, ControlPartCode *outPartCode)
{
	if (outPartCode != NULL)
		*outPartCode = 0;
	if (inScrollBarBounds == NULL || inTrackInfo == NULL || inPtHit == NULL)
		return false;
	HIThemeTrackDrawInfo info;
	scrollBarInfo(inScrollBarBounds, inTrackInfo, inIsHoriz, &info);
	TrackLayout L;
	if (layoutTrack(&info, &L) != noErr)
		return false;
	if (outTrackBounds != NULL)
		*outTrackBounds = L.track;
	if (!L.hasArrows)
		return false;
	if (CGRectContainsPoint(L.arrowA, *inPtHit))
	{
		if (outPartCode != NULL) *outPartCode = kAppearancePartUpButton;
		return true;
	}
	if (CGRectContainsPoint(L.arrowB, *inPtHit))
	{
		if (outPartCode != NULL) *outPartCode = kAppearancePartDownButton;
		return true;
	}
	return false;
}

// ---------------------------------------------------------------------------
// Text
// ---------------------------------------------------------------------------

CTFontUIFontType HIThemeGetUIFontType(ThemeFontID inFontID)
{
	switch (inFontID)
	{
		case kThemeSystemFont: return kCTFontUIFontSystem;
		case kThemeSmallSystemFont: return kCTFontUIFontSmallSystem;
		case kThemeSmallEmphasizedSystemFont: return kCTFontUIFontSmallEmphasizedSystem;
		case kThemeViewsFont: return kCTFontUIFontViews;
		case kThemeEmphasizedSystemFont: return kCTFontUIFontEmphasizedSystem;
		case kThemeApplicationFont: return kCTFontUIFontApplication;
		case kThemeLabelFont: return kCTFontUIFontLabel;
		case kThemeSystemFontDetail: return (CTFontUIFontType) 19;
		case kThemeSystemFontDetailEmphasized: return (CTFontUIFontType) 20;
		case kThemeMenuTitleFont: return kCTFontUIFontMenuTitle;
		case kThemeMenuItemFont: return kCTFontUIFontMenuItem;
		case kThemeMenuItemMarkFont: return kCTFontUIFontMenuItemMark;
		case kThemeMenuItemCmdKeyFont: return kCTFontUIFontMenuItemCmdKey;
		case kThemeWindowTitleFont: return kCTFontUIFontWindowTitle;
		case kThemePushButtonFont: return kCTFontUIFontPushButton;
		case kThemeUtilityWindowTitleFont: return kCTFontUIFontUtilityWindowTitle;
		case kThemeAlertHeaderFont: return (CTFontUIFontType) 18;
		case kThemeToolbarFont: return (CTFontUIFontType) 21;
		case kThemeMiniSystemFont: return kCTFontUIFontMiniSystem;
		default: return kCTFontUIFontNone;
	}
}

// Drawing: not implemented in this change.
OSStatus HIThemeDrawButton(const HIRect *inBounds, const HIThemeButtonDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation, HIRect *outLabelRect)
{
	return unimpErr;
}

OSStatus HIThemeDrawGenericWell(const HIRect *inRect, const HIThemeButtonDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	return unimpErr;
}

OSStatus HIThemeDrawFrame(const HIRect *inRect, const HIThemeFrameDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	return unimpErr;
}

OSStatus HIThemeDrawGroupBox(const HIRect *inRect, const HIThemeGroupBoxDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	return unimpErr;
}

OSStatus HIThemeDrawPaneSplitter(const HIRect *inRect, const HIThemeSplitterDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	return unimpErr;
}

OSStatus HIThemeDrawGrabber(const HIRect *inRect, const HIThemeGrabberDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	return unimpErr;
}

OSStatus HIThemeDrawPlacard(const HIRect *inRect, const HIThemePlacardDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	return unimpErr;
}

OSStatus HIThemeDrawHeader(const HIRect *inRect, const HIThemeHeaderDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	return unimpErr;
}

OSStatus HIThemeDrawFocusRect(const HIRect *inRect, Boolean inHasFocus, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	return unimpErr;
}

OSStatus HIThemeBeginFocus(CGContextRef inContext, HIThemeFocusRing inRing, void *inReserved)
{
	return unimpErr;
}

OSStatus HIThemeEndFocus(CGContextRef inContext)
{
	return unimpErr;
}

OSStatus HIThemeDrawSeparator(const HIRect *inRect, const HIThemeSeparatorDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	return unimpErr;
}

OSStatus HIThemeDrawPopupArrow(const HIRect *inBounds, const HIThemePopupArrowDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	return unimpErr;
}

OSStatus HIThemeDrawChasingArrows(const HIRect *inBounds, const HIThemeChasingArrowsDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	return unimpErr;
}

OSStatus HIThemeDrawGrowBox(const HIPoint *inOrigin, const HIThemeGrowBoxDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	return unimpErr;
}

OSStatus HIThemeDrawTickMark(const HIRect *inBounds, const HIThemeTickMarkDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	return unimpErr;
}

OSStatus HIThemeDrawScrollBarDelimiters(const HIRect *inContRect, const HIThemeScrollBarDelimitersDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	return unimpErr;
}

OSStatus HIThemeDrawBackground(const HIRect *inBounds, const HIThemeBackgroundDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	return unimpErr;
}

OSStatus HIThemeApplyBackground(const HIRect *inBounds, const HIThemeBackgroundDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	return unimpErr;
}

OSStatus HIThemeDrawSegment(const HIRect *inBounds, const HIThemeSegmentDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	return unimpErr;
}

OSStatus HIThemeDrawTabPane(const HIRect *inRect, const HIThemeTabPaneDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	return unimpErr;
}

OSStatus HIThemeDrawTab(const HIRect *inRect, const HIThemeTabDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation, HIRect *outLabelRect)
{
	return unimpErr;
}

OSStatus HIThemeDrawMenuBarBackground(const HIRect *inBounds, const HIThemeMenuBarDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	return unimpErr;
}

OSStatus HIThemeDrawMenuTitle(const HIRect *inMenuBarRect, const HIRect *inTitleRect, const HIThemeMenuTitleDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation, HIRect *outLabelRect)
{
	return unimpErr;
}

OSStatus HIThemeDrawMenuBackground(const HIRect *inMenuRect, const HIThemeMenuDrawInfo *inMenuDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	return unimpErr;
}

OSStatus HIThemeDrawMenuItem(const HIRect *inMenuRect, const HIRect *inItemRect, const HIThemeMenuItemDrawInfo *inItemDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation, HIRect *outContentRect)
{
	return unimpErr;
}

OSStatus HIThemeDrawMenuSeparator(const HIRect *inMenuRect, const HIRect *inItemRect, const HIThemeMenuItemDrawInfo *inItemDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	return unimpErr;
}

OSStatus HIThemeDrawWindowFrame(const HIRect *inContRect, const HIThemeWindowDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation, HIRect *outTitleRect)
{
	return unimpErr;
}

OSStatus HIThemeDrawTitleBarWidget(const HIRect *inContRect, const HIThemeWindowWidgetDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	return unimpErr;
}

OSStatus HIThemeDrawTrack(const HIThemeTrackDrawInfo *inDrawInfo, const HIRect *inGhostRect, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	return unimpErr;
}

OSStatus HIThemeDrawTrackTickMarks(const HIThemeTrackDrawInfo *inDrawInfo, ItemCount inNumTicks, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	return unimpErr;
}

OSStatus HIThemeGetTextDimensions(CFTypeRef inString, CGFloat inWidth, HIThemeTextInfo *inTextInfo, CGFloat *outWidth, CGFloat *outHeight, CGFloat *outBaseline)
{
	return unimpErr;
}

OSStatus HIThemeDrawTextBox(CFTypeRef inString, const HIRect *inBounds, HIThemeTextInfo *inTextInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	return unimpErr;
}
