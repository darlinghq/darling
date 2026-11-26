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

// HITheme: metrics, geometry queries and drawing of the classic Aqua controls.
//
// Drawing is done with CoreGraphics (and CoreText for text) only. Every draw
// call works in a local frame whose origin is the visual top-left corner of the
// passed rectangle and whose y axis points down; the orientation argument
// selects how that frame maps onto the context.

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

static Boolean isActiveState(ThemeDrawState state)
{
	return state != kThemeStateInactive && state != kThemeStateUnavailableInactive;
}

static Boolean isDisabledState(ThemeDrawState state)
{
	return state == kThemeStateUnavailable || state == kThemeStateUnavailableInactive;
}

static Boolean isPressedState(ThemeDrawState state)
{
	return state == kThemeStatePressed || state == kThemeStatePressedUp || state == kThemeStatePressedDown;
}

// Enter the local frame of a rectangle: origin at its visual top-left corner, y down.
static void localBegin(CGContextRef c, const HIRect *r, HIThemeOrientation orientation)
{
	CGContextSaveGState(c);
	if (orientation == kHIThemeOrientationInverted)
	{
		CGContextTranslateCTM(c, r->origin.x, r->origin.y + r->size.height);
		CGContextScaleCTM(c, 1, -1);
	}
	else
	{
		CGContextTranslateCTM(c, r->origin.x, r->origin.y);
	}
}

static void localEnd(CGContextRef c)
{
	CGContextRestoreGState(c);
}

// Round rectangle with a chosen set of rounded corners (bit 0: top-left, 1: top-right, 2: bottom-right, 3: bottom-left).
static void addRoundRectCorners(CGContextRef c, CGRect r, CGFloat radius, unsigned corners)
{
	CGFloat maxRadius = fmin(r.size.width, r.size.height) / 2;
	if (radius > maxRadius)
		radius = maxRadius;
	CGFloat x0 = r.origin.x, y0 = r.origin.y, x1 = x0 + r.size.width, y1 = y0 + r.size.height;
	CGFloat rr[4];
	for (int i = 0; i < 4; i++)
		rr[i] = (corners & (1u << i)) ? radius : 0;
	CGContextMoveToPoint(c, x0 + rr[0], y0);
	CGContextAddLineToPoint(c, x1 - rr[1], y0);
	if (rr[1] > 0) CGContextAddArcToPoint(c, x1, y0, x1, y0 + rr[1], rr[1]);
	CGContextAddLineToPoint(c, x1, y1 - rr[2]);
	if (rr[2] > 0) CGContextAddArcToPoint(c, x1, y1, x1 - rr[2], y1, rr[2]);
	CGContextAddLineToPoint(c, x0 + rr[3], y1);
	if (rr[3] > 0) CGContextAddArcToPoint(c, x0, y1, x0, y1 - rr[3], rr[3]);
	CGContextAddLineToPoint(c, x0, y0 + rr[0]);
	if (rr[0] > 0) CGContextAddArcToPoint(c, x0, y0, x0 + rr[0], y0, rr[0]);
	CGContextClosePath(c);
}

static void addRoundRect(CGContextRef c, CGRect r, CGFloat radius)
{
	addRoundRectCorners(c, r, radius, 15u);
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

static void setStroke(CGContextRef c, Color col)
{
	CGContextSetRGBStrokeColor(c, col.v[0], col.v[1], col.v[2], col.v[3]);
}

static void fillGradientRect(CGContextRef c, CGRect r, Color top, Color bottom)
{
	CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
	CGFloat comps[8];
	memcpy(comps, top.v, sizeof(top.v));
	memcpy(comps + 4, bottom.v, sizeof(bottom.v));
	CGGradientRef g = CGGradientCreateWithColorComponents(cs, comps, NULL, 2);
	if (g != NULL)
	{
		CGContextSaveGState(c);
		CGContextClipToRect(c, r);
		CGContextDrawLinearGradient(c, g, CGPointMake(r.origin.x, r.origin.y), CGPointMake(r.origin.x, r.origin.y + r.size.height), 0);
		CGContextRestoreGState(c);
		CGGradientRelease(g);
	}
	else
	{
		setFill(c, top);
		CGContextFillRect(c, r);
	}
	CGColorSpaceRelease(cs);
}

// Gradient fill and 1 pixel outline of a round rectangle.
static void paintRoundRect(CGContextRef c, CGRect r, CGFloat radius, Color top, Color bottom, Color border)
{
	CGContextSaveGState(c);
	addRoundRect(c, r, radius);
	CGContextClip(c);
	fillGradientRect(c, r, top, bottom);
	CGContextRestoreGState(c);
	if (border.v[3] > 0)
	{
		setStroke(c, border);
		CGContextSetLineWidth(c, 1);
		addRoundRect(c, CGRectInset(r, 0.5, 0.5), radius > 0.5 ? radius - 0.5 : 0);
		CGContextStrokePath(c);
	}
}

static void drawLine(CGContextRef c, CGFloat x0, CGFloat y0, CGFloat x1, CGFloat y1, Color col, CGFloat width)
{
	setStroke(c, col);
	CGContextSetLineWidth(c, width);
	CGContextMoveToPoint(c, x0, y0);
	CGContextAddLineToPoint(c, x1, y1);
	CGContextStrokePath(c);
}

// Filled triangle; the tip is at (tx, ty), the base is centred on (bx, by) and extends "half" to both sides.
static void fillArrow(CGContextRef c, CGFloat tx, CGFloat ty, CGFloat bx, CGFloat by, CGFloat half, Color col)
{
	CGFloat dx = bx - tx, dy = by - ty;
	CGFloat len = sqrt(dx * dx + dy * dy);
	if (len == 0)
		return;
	CGFloat px = -dy / len * half, py = dx / len * half;
	setFill(c, col);
	CGContextMoveToPoint(c, tx, ty);
	CGContextAddLineToPoint(c, bx + px, by + py);
	CGContextAddLineToPoint(c, bx - px, by - py);
	CGContextClosePath(c);
	CGContextFillPath(c);
}

static Color accentTop(void) { return rgba(0.42, 0.71, 0.99, 1); }
static Color accentBottom(void) { return rgba(0.10, 0.50, 0.97, 1); }
static Color accentBorder(void) { return rgba(0.05, 0.36, 0.85, 1); }

static Color faceTop(ThemeDrawState state)
{
	if (isPressedState(state)) return gray(0.86);
	return isActiveState(state) ? gray(1.0) : gray(0.97);
}

static Color faceBottom(ThemeDrawState state)
{
	if (isPressedState(state)) return gray(0.78);
	return isActiveState(state) ? gray(0.94) : gray(0.95);
}

static Color frameColor(ThemeDrawState state)
{
	return isActiveState(state) ? gray(0.67) : gray(0.78);
}

static Color textColorForState(ThemeDrawState state)
{
	if (isDisabledState(state)) return rgba(0, 0, 0, 0.3);
	return isActiveState(state) ? rgba(0, 0, 0, 0.85) : rgba(0, 0, 0, 0.5);
}

static void paintFocusRing(CGContextRef c, CGRect r, CGFloat radius)
{
	setStroke(c, rgba(0.30, 0.60, 0.95, 0.65));
	CGContextSetLineWidth(c, 3);
	addRoundRect(c, CGRectInset(r, -1.5, -1.5), radius + 1.5);
	CGContextStrokePath(c);
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

static CGRect specRect(const RectSpec *spec, const HIRect *in)
{
	return specRectClamped(spec, in, true, false);
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

// Rectangle of the artwork in the local frame of the passed rectangle.
static CGRect artworkRect(const DrawSpec *d, CGFloat w, CGFloat h)
{
	CGFloat aw = d->w != 0 ? d->w : w;
	CGFloat ah = d->h != 0 ? d->h : h;
	CGFloat x = d->w == 0 ? 0 : (d->xCenter ? floor((w - aw) / 2) : d->dx);
	CGFloat y = d->h == 0 ? 0 : (d->yCenter ? floor((h - ah) / 2) : 0);
	return CGRectMake(x, y, aw, ah);
}

static void drawCheckMark(CGContextRef c, CGRect r, Color col)
{
	setStroke(c, col);
	CGContextSetLineWidth(c, fmax(1.5, r.size.width / 8));
	CGContextSetLineCap(c, kCGLineCapRound);
	CGContextSetLineJoin(c, kCGLineJoinRound);
	CGContextMoveToPoint(c, r.origin.x + r.size.width * 0.24, r.origin.y + r.size.height * 0.52);
	CGContextAddLineToPoint(c, r.origin.x + r.size.width * 0.43, r.origin.y + r.size.height * 0.72);
	CGContextAddLineToPoint(c, r.origin.x + r.size.width * 0.77, r.origin.y + r.size.height * 0.26);
	CGContextStrokePath(c);
}

static void drawDash(CGContextRef c, CGRect r, Color col)
{
	drawLine(c, r.origin.x + r.size.width * 0.25, r.origin.y + r.size.height / 2, r.origin.x + r.size.width * 0.75, r.origin.y + r.size.height / 2, col, fmax(1.5, r.size.width / 8));
}

static Boolean isAccented(const HIThemeButtonDrawInfo *info)
{
	return info->value != kThemeButtonOff && isActiveState(info->state) && !isDisabledState(info->state);
}

static void paintCheckBox(CGContextRef c, CGRect r, const HIThemeButtonDrawInfo *info)
{
	Boolean on = info->value != kThemeButtonOff;
	if (isAccented(info))
		paintRoundRect(c, r, 3, accentTop(), accentBottom(), accentBorder());
	else
		paintRoundRect(c, r, 3, faceTop(info->state), faceBottom(info->state), frameColor(info->state));
	if (!on)
		return;
	Color mark = isAccented(info) ? gray(1) : textColorForState(info->state);
	if (info->value == kThemeButtonMixed)
		drawDash(c, r, mark);
	else
		drawCheckMark(c, r, mark);
}

static void paintRadioButton(CGContextRef c, CGRect r, const HIThemeButtonDrawInfo *info)
{
	Boolean on = info->value != kThemeButtonOff;
	CGContextSaveGState(c);
	CGContextAddEllipseInRect(c, r);
	CGContextClip(c);
	if (isAccented(info))
		fillGradientRect(c, r, accentTop(), accentBottom());
	else
		fillGradientRect(c, r, faceTop(info->state), faceBottom(info->state));
	CGContextRestoreGState(c);
	setStroke(c, isAccented(info) ? accentBorder() : frameColor(info->state));
	CGContextSetLineWidth(c, 1);
	CGContextAddEllipseInRect(c, CGRectInset(r, 0.5, 0.5));
	CGContextStrokePath(c);
	if (!on)
		return;
	Color mark = isAccented(info) ? gray(1) : textColorForState(info->state);
	if (info->value == kThemeButtonMixed)
		drawDash(c, r, mark);
	else
	{
		setFill(c, mark);
		CGContextAddEllipseInRect(c, CGRectInset(r, r.size.width * 0.3, r.size.height * 0.3));
		CGContextFillPath(c);
	}
}

static void paintPushButton(CGContextRef c, CGRect r, CGFloat radius, const HIThemeButtonDrawInfo *info)
{
	Boolean isDefault = (info->adornment & kThemeAdornmentDefault) != 0;
	Boolean active = isActiveState(info->state) && !isDisabledState(info->state);
	if (isDefault && active)
	{
		Color top = accentTop(), bottom = accentBottom();
		if (isPressedState(info->state))
		{
			top = rgba(0.25, 0.55, 0.92, 1);
			bottom = rgba(0.05, 0.38, 0.82, 1);
		}
		paintRoundRect(c, r, radius, top, bottom, accentBorder());
	}
	else
	{
		paintRoundRect(c, r, radius, faceTop(info->state), faceBottom(info->state), frameColor(info->state));
	}
}

// Double chevron of a pop-up button, in the right end of the artwork rectangle.
static void paintPopupArrows(CGContextRef c, CGRect r, const HIThemeButtonDrawInfo *info)
{
	CGFloat cx = r.origin.x + r.size.width - 10;
	CGFloat cy = r.origin.y + r.size.height / 2;
	Boolean isDefault = (info->adornment & kThemeAdornmentDefault) != 0;
	Color col = (isDefault && isActiveState(info->state) && !isDisabledState(info->state)) ? gray(1) : textColorForState(info->state);
	fillArrow(c, cx, cy - 5, cx, cy - 2, 3.5, col);
	fillArrow(c, cx, cy + 5, cx, cy + 2, 3.5, col);
}

static void paintBevel(CGContextRef c, CGRect r, CGFloat radius, const HIThemeButtonDrawInfo *info)
{
	Boolean on = info->value == kThemeButtonOn;
	Color top = on ? gray(0.78) : faceTop(info->state);
	Color bottom = on ? gray(0.84) : faceBottom(info->state);
	paintRoundRect(c, r, radius, top, bottom, frameColor(info->state));
}

static void paintDisclosure(CGContextRef c, CGRect r, const HIThemeButtonDrawInfo *info)
{
	Color col = textColorForState(info->state);
	CGFloat cx = r.origin.x + r.size.width / 2, cy = r.origin.y + r.size.height / 2;
	switch (info->value)
	{
		case kThemeDisclosureDown:
			fillArrow(c, cx, cy + 3, cx, cy - 2, 4, col);
			break;
		case kThemeDisclosureLeft:
			fillArrow(c, cx - 3, cy, cx + 2, cy, 4, col);
			break;
		default:
			fillArrow(c, cx + 3, cy, cx - 2, cy, 4, col);
			break;
	}
}

static void paintArrowButton(CGContextRef c, CGRect r, const HIThemeButtonDrawInfo *info)
{
	paintRoundRect(c, r, 4, faceTop(info->state), faceBottom(info->state), frameColor(info->state));
	Color col = textColorForState(info->state);
	CGFloat cx = r.origin.x + r.size.width / 2, cy = r.origin.y + r.size.height / 2;
	UInt16 a = info->adornment;
	if (a & kThemeAdornmentArrowDoubleArrow)
	{
		fillArrow(c, cx, cy - 4, cx, cy - 1, 3, col);
		fillArrow(c, cx, cy + 4, cx, cy + 1, 3, col);
	}
	else if (a & kThemeAdornmentArrowLeftArrow)
		fillArrow(c, cx - 3, cy, cx + 2, cy, 3.5, col);
	else if (a & kThemeAdornmentArrowUpArrow)
		fillArrow(c, cx, cy - 3, cx, cy + 2, 3.5, col);
	else if (a & kThemeAdornmentArrowRightArrow)
		fillArrow(c, cx + 3, cy, cx - 2, cy, 3.5, col);
	else
		fillArrow(c, cx, cy + 3, cx, cy - 2, 3.5, col);
}

static void paintIncDec(CGContextRef c, CGRect r, const HIThemeButtonDrawInfo *info)
{
	CGFloat half = floor(r.size.height / 2);
	CGRect upper = CGRectMake(r.origin.x, r.origin.y, r.size.width, half + 1);
	CGRect lower = CGRectMake(r.origin.x, r.origin.y + half, r.size.width, r.size.height - half);
	CGContextSaveGState(c);
	addRoundRectCorners(c, upper, 4, 3u);
	CGContextClip(c);
	fillGradientRect(c, upper, faceTop(info->state), faceBottom(info->state));
	CGContextRestoreGState(c);
	CGContextSaveGState(c);
	addRoundRectCorners(c, lower, 4, 12u);
	CGContextClip(c);
	fillGradientRect(c, lower, faceTop(info->state), faceBottom(info->state));
	CGContextRestoreGState(c);
	setStroke(c, frameColor(info->state));
	CGContextSetLineWidth(c, 1);
	addRoundRect(c, CGRectInset(r, 0.5, 0.5), 3.5);
	CGContextStrokePath(c);
	drawLine(c, r.origin.x, r.origin.y + half, r.origin.x + r.size.width, r.origin.y + half, frameColor(info->state), 1);
	Color col = textColorForState(info->state);
	CGFloat cx = r.origin.x + r.size.width / 2;
	fillArrow(c, cx, r.origin.y + half / 2 - 2.5, cx, r.origin.y + half / 2 + 1.5, 3, col);
	fillArrow(c, cx, r.origin.y + half + (r.size.height - half) / 2 + 2.5, cx, r.origin.y + half + (r.size.height - half) / 2 - 1.5, 3, col);
}

static void paintRoundButton(CGContextRef c, CGRect r, const HIThemeButtonDrawInfo *info)
{
	CGContextSaveGState(c);
	CGContextAddEllipseInRect(c, r);
	CGContextClip(c);
	fillGradientRect(c, r, faceTop(info->state), faceBottom(info->state));
	CGContextRestoreGState(c);
	setStroke(c, frameColor(info->state));
	CGContextSetLineWidth(c, 1);
	CGContextAddEllipseInRect(c, CGRectInset(r, 0.5, 0.5));
	CGContextStrokePath(c);
}

static void paintListHeader(CGContextRef c, CGRect r, const HIThemeButtonDrawInfo *info)
{
	Color top = isPressedState(info->state) ? gray(0.84) : gray(0.98);
	Color bottom = isPressedState(info->state) ? gray(0.78) : gray(0.92);
	fillGradientRect(c, r, top, bottom);
	drawLine(c, r.origin.x, r.origin.y + r.size.height - 0.5, r.origin.x + r.size.width, r.origin.y + r.size.height - 0.5, gray(0.72), 1);
	drawLine(c, r.origin.x + r.size.width - 0.5, r.origin.y, r.origin.x + r.size.width - 0.5, r.origin.y + r.size.height, gray(0.82), 1);
	if (!(info->adornment & kThemeAdornmentHeaderButtonNoSortArrow) && info->value != kThemeButtonOff)
	{
		CGFloat cx = r.origin.x + r.size.width - 12, cy = r.origin.y + r.size.height / 2;
		Color col = rgba(0, 0, 0, 0.5);
		if (info->adornment & kThemeAdornmentHeaderButtonSortUp)
			fillArrow(c, cx, cy - 3, cx, cy + 2, 4, col);
		else
			fillArrow(c, cx, cy + 3, cx, cy - 2, 4, col);
	}
}

static void paintComboBox(CGContextRef c, CGRect r, const HIThemeButtonDrawInfo *info)
{
	paintRoundRect(c, r, 4, gray(1), gray(1), frameColor(info->state));
	CGFloat bw = fmin(r.size.width, 17);
	CGRect arrowBox = CGRectMake(r.origin.x + r.size.width - bw, r.origin.y, bw, r.size.height);
	CGContextSaveGState(c);
	addRoundRectCorners(c, arrowBox, 4, 6u);
	CGContextClip(c);
	fillGradientRect(c, arrowBox, faceTop(info->state), faceBottom(info->state));
	CGContextRestoreGState(c);
	drawLine(c, arrowBox.origin.x + 0.5, r.origin.y, arrowBox.origin.x + 0.5, r.origin.y + r.size.height, frameColor(info->state), 1);
	CGFloat cx = arrowBox.origin.x + bw / 2, cy = r.origin.y + r.size.height / 2;
	fillArrow(c, cx, cy + 3, cx, cy - 2, 3.5, textColorForState(info->state));
}

static OSStatus drawButtonArtwork(CGContextRef c, CGRect art, const HIThemeButtonDrawInfo *info)
{
	switch (info->kind)
	{
		case kThemePushButton:
		case kThemePushButtonNormal:
		case kThemePushButtonSmall:
		case kThemePushButtonMini:
		case kThemePushButtonTextured:
		case kThemePushButtonTexturedSmall:
			paintPushButton(c, art, art.size.height >= 20 ? 5 : 4, info);
			break;
		case kThemeCheckBox:
		case kThemeCheckBoxSmall:
		case kThemeCheckBoxMini:
			paintCheckBox(c, art, info);
			break;
		case kThemeRadioButton:
		case kThemeRadioButtonSmall:
		case kThemeRadioButtonMini:
			paintRadioButton(c, art, info);
			break;
		case kThemeBevelButton:
		case kThemeBevelButtonSmall:
		case kThemeBevelButtonLarge:
		case kThemeBevelButtonInset:
			paintBevel(c, art, 3, info);
			break;
		case kThemeRoundedBevelButton:
			paintBevel(c, art, 5, info);
			break;
		case kThemeArrowButton:
		case kThemeArrowButtonSmall:
		case kThemeArrowButtonMini:
			paintArrowButton(c, art, info);
			break;
		case kThemePopupButton:
		case kThemePopupButtonNormal:
		case kThemePopupButtonSmall:
		case kThemePopupButtonMini:
			paintPushButton(c, art, 5, info);
			paintPopupArrows(c, art, info);
			break;
		case kThemeDisclosureTriangle:
			paintDisclosure(c, art, info);
			break;
		case kThemeIncDecButton:
		case kThemeIncDecButtonSmall:
		case kThemeIncDecButtonMini:
			paintIncDec(c, art, info);
			break;
		case kThemeListHeaderButton:
			paintListHeader(c, art, info);
			break;
		case kThemeRoundButton:
		case kThemeRoundButtonLarge:
		case kThemeRoundButtonHelp:
			paintRoundButton(c, art, info);
			break;
		case kThemeComboBox:
		case kThemeComboBoxSmall:
		case kThemeComboBoxMini:
			paintComboBox(c, art, info);
			break;
		default:
			return paramErr;
	}
	return noErr;
}

OSStatus HIThemeDrawButton(const HIRect *inBounds, const HIThemeButtonDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation, HIRect *outLabelRect)
{
	if (inBounds == NULL || inDrawInfo == NULL || inContext == NULL)
		return paramErr;
	const ButtonKindSpec *spec = buttonSpec(inDrawInfo);
	if (spec == NULL)
		return paramErr;

	CGFloat w = inBounds->size.width, h = inBounds->size.height;
	CGRect art = artworkRect(&spec->draw, w, h);

	if (outLabelRect != NULL)
	{
		// The label table is expressed from the visual top; flip it for an inverted context.
		CGRect label = buttonRect(inDrawInfo, 2, &spec->label, inBounds, false);
		if (inOrientation == kHIThemeOrientationInverted && spec->label.x.mode != AX_EMPTY)
			label.origin.y = inBounds->origin.y + (inBounds->size.height - (label.origin.y - inBounds->origin.y) - label.size.height);
		*outLabelRect = label;
	}

	localBegin(inContext, inBounds, inOrientation);
	if ((inDrawInfo->adornment & kThemeAdornmentFocus) && inDrawInfo->state != kThemeStateInactive)
		paintFocusRing(inContext, art, 4);
	OSStatus status = drawButtonArtwork(inContext, art, inDrawInfo);
	localEnd(inContext);
	return status;
}

OSStatus HIThemeDrawGenericWell(const HIRect *inRect, const HIThemeButtonDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	if (inRect == NULL || inDrawInfo == NULL || inContext == NULL)
		return paramErr;
	localBegin(inContext, inRect, inOrientation);
	CGRect r = CGRectMake(0, 0, inRect->size.width, inRect->size.height);
	paintRoundRect(inContext, r, 3, gray(0.88), gray(0.93), frameColor(inDrawInfo->state));
	localEnd(inContext);
	return noErr;
}

// ---------------------------------------------------------------------------
// Frames, boxes and small decorations
// ---------------------------------------------------------------------------

OSStatus HIThemeDrawFrame(const HIRect *inRect, const HIThemeFrameDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	if (inRect == NULL || inDrawInfo == NULL || inContext == NULL)
		return paramErr;
	localBegin(inContext, inRect, inOrientation);
	CGRect r = CGRectMake(0, 0, inRect->size.width, inRect->size.height);
	CGFloat radius = 0;
	switch (inDrawInfo->kind)
	{
		case kHIThemeFrameTextFieldRound: radius = fmin(r.size.height / 2, 11); break;
		case kHIThemeFrameTextFieldRoundSmall: radius = fmin(r.size.height / 2, 9); break;
		case kHIThemeFrameTextFieldRoundMini: radius = fmin(r.size.height / 2, 8); break;
		default: break;
	}
	if (inDrawInfo->isFocused)
		paintFocusRing(inContext, r, radius);
	Color border = isActiveState(inDrawInfo->state) ? gray(0.62) : gray(0.78);
	if (radius > 0)
		paintRoundRect(inContext, r, radius, gray(1), gray(1), border);
	else
	{
		setStroke(inContext, border);
		CGContextSetLineWidth(inContext, 1);
		CGContextStrokeRect(inContext, CGRectInset(r, 0.5, 0.5));
		drawLine(inContext, 1, 1.5, r.size.width - 1, 1.5, rgba(0, 0, 0, 0.12), 1);
	}
	localEnd(inContext);
	return noErr;
}

OSStatus HIThemeDrawGroupBox(const HIRect *inRect, const HIThemeGroupBoxDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	if (inRect == NULL || inDrawInfo == NULL || inContext == NULL)
		return paramErr;
	localBegin(inContext, inRect, inOrientation);
	CGRect r = CGRectMake(0, 0, inRect->size.width, inRect->size.height);
	Boolean secondary = inDrawInfo->kind == kHIThemeGroupBoxKindSecondary || inDrawInfo->kind == kHIThemeGroupBoxKindSecondaryOpaque;
	Boolean opaque = inDrawInfo->kind == kHIThemeGroupBoxKindPrimaryOpaque || inDrawInfo->kind == kHIThemeGroupBoxKindSecondaryOpaque;
	Color fill = opaque ? gray(secondary ? 0.95 : 0.9) : rgba(0, 0, 0, secondary ? 0.04 : 0.07);
	setFill(inContext, fill);
	addRoundRect(inContext, r, 5);
	CGContextFillPath(inContext);
	setStroke(inContext, rgba(0, 0, 0, 0.12));
	CGContextSetLineWidth(inContext, 1);
	addRoundRect(inContext, CGRectInset(r, 0.5, 0.5), 4.5);
	CGContextStrokePath(inContext);
	localEnd(inContext);
	return noErr;
}

OSStatus HIThemeDrawPaneSplitter(const HIRect *inRect, const HIThemeSplitterDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	if (inRect == NULL || inDrawInfo == NULL || inContext == NULL)
		return paramErr;
	localBegin(inContext, inRect, inOrientation);
	CGFloat w = inRect->size.width, h = inRect->size.height;
	Boolean horizontalBar = w >= h;
	CGFloat thickness = 10;
	CGRect band = horizontalBar ? CGRectMake(0, floor((h - thickness) / 2), w, fmin(thickness, h))
	                            : CGRectMake(floor((w - thickness) / 2), 0, fmin(thickness, w), h);
	Boolean metal = inDrawInfo->adornment == kHIThemeSplitterAdornmentMetal;
	fillGradientRect(inContext, band, metal ? gray(0.82) : gray(0.97), metal ? gray(0.74) : gray(0.90));
	Color edge = gray(0.7);
	if (horizontalBar)
	{
		drawLine(inContext, band.origin.x, band.origin.y + 0.5, band.origin.x + band.size.width, band.origin.y + 0.5, edge, 1);
		drawLine(inContext, band.origin.x, band.origin.y + band.size.height - 0.5, band.origin.x + band.size.width, band.origin.y + band.size.height - 0.5, edge, 1);
	}
	else
	{
		drawLine(inContext, band.origin.x + 0.5, band.origin.y, band.origin.x + 0.5, band.origin.y + band.size.height, edge, 1);
		drawLine(inContext, band.origin.x + band.size.width - 0.5, band.origin.y, band.origin.x + band.size.width - 0.5, band.origin.y + band.size.height, edge, 1);
	}
	setFill(inContext, gray(0.55));
	for (int i = -1; i <= 1; i++)
	{
		CGFloat cx = horizontalBar ? w / 2 + i * 5 : band.origin.x + band.size.width / 2;
		CGFloat cy = horizontalBar ? band.origin.y + band.size.height / 2 : h / 2 + i * 5;
		CGContextFillEllipseInRect(inContext, CGRectMake(cx - 1, cy - 1, 2, 2));
	}
	localEnd(inContext);
	return noErr;
}

OSStatus HIThemeDrawGrabber(const HIRect *inRect, const HIThemeGrabberDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	if (inRect == NULL || inDrawInfo == NULL || inContext == NULL)
		return paramErr;
	localBegin(inContext, inRect, inOrientation);
	CGRect r = CGRectMake(0, 0, inRect->size.width, fmin(inRect->size.height, 5));
	fillGradientRect(inContext, r, gray(0.9), gray(0.78));
	drawLine(inContext, 0, r.size.height - 0.5, r.size.width, r.size.height - 0.5, gray(0.62), 1);
	localEnd(inContext);
	return noErr;
}

OSStatus HIThemeDrawPlacard(const HIRect *inRect, const HIThemePlacardDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	if (inRect == NULL || inDrawInfo == NULL || inContext == NULL)
		return paramErr;
	localBegin(inContext, inRect, inOrientation);
	CGRect r = CGRectMake(0, 0, inRect->size.width, inRect->size.height);
	Color top = isPressedState(inDrawInfo->state) ? gray(0.82) : gray(0.97);
	Color bottom = isPressedState(inDrawInfo->state) ? gray(0.76) : gray(0.88);
	fillGradientRect(inContext, r, top, bottom);
	setStroke(inContext, gray(0.65));
	CGContextSetLineWidth(inContext, 1);
	CGContextStrokeRect(inContext, CGRectInset(r, 0.5, 0.5));
	localEnd(inContext);
	return noErr;
}

OSStatus HIThemeDrawHeader(const HIRect *inRect, const HIThemeHeaderDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	if (inRect == NULL || inDrawInfo == NULL || inContext == NULL)
		return paramErr;
	localBegin(inContext, inRect, inOrientation);
	CGRect r = CGRectMake(0, 0, inRect->size.width, inRect->size.height);
	Boolean active = isActiveState(inDrawInfo->state);
	if (inDrawInfo->kind == kHIThemeHeaderKindList)
		fillGradientRect(inContext, r, gray(0.98), gray(0.92));
	else
		fillGradientRect(inContext, r, active ? gray(0.93) : gray(0.95), active ? gray(0.84) : gray(0.92));
	drawLine(inContext, 0, r.size.height - 0.5, r.size.width, r.size.height - 0.5, gray(active ? 0.62 : 0.75), 1);
	localEnd(inContext);
	return noErr;
}

OSStatus HIThemeDrawFocusRect(const HIRect *inRect, Boolean inHasFocus, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	if (inRect == NULL || inContext == NULL)
		return paramErr;
	// Without focus there is nothing to draw: the ring belongs to the focused state only.
	if (!inHasFocus)
		return noErr;
	localBegin(inContext, inRect, inOrientation);
	paintFocusRing(inContext, CGRectMake(0, 0, inRect->size.width, inRect->size.height), 3);
	localEnd(inContext);
	return noErr;
}

OSStatus HIThemeBeginFocus(CGContextRef inContext, HIThemeFocusRing inRing, void *inReserved)
{
	if (inContext == NULL)
		return paramErr;
	if (inRing > kHIThemeFocusRingBelow)
		return paramErr;
	CGContextSaveGState(inContext);
	CGColorRef ring = CGColorCreateGenericRGB(0.30, 0.60, 0.95, 0.75);
	CGContextSetShadowWithColor(inContext, CGSizeZero, 3, ring);
	CGColorRelease(ring);
	return noErr;
}

OSStatus HIThemeEndFocus(CGContextRef inContext)
{
	if (inContext == NULL)
		return paramErr;
	CGContextRestoreGState(inContext);
	return noErr;
}

OSStatus HIThemeDrawSeparator(const HIRect *inRect, const HIThemeSeparatorDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	if (inRect == NULL || inDrawInfo == NULL || inContext == NULL)
		return paramErr;
	localBegin(inContext, inRect, inOrientation);
	CGFloat w = inRect->size.width, h = inRect->size.height;
	Color col = isActiveState(inDrawInfo->state) ? gray(0.74) : gray(0.84);
	if (w >= h)
		drawLine(inContext, 0, 0.5, w, 0.5, col, 1);
	else
		drawLine(inContext, 0.5, 0, 0.5, h, col, 1);
	localEnd(inContext);
	return noErr;
}

OSStatus HIThemeDrawPopupArrow(const HIRect *inBounds, const HIThemePopupArrowDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	if (inBounds == NULL || inDrawInfo == NULL || inContext == NULL)
		return paramErr;
	if (inDrawInfo->orientation > kThemeArrowUp || inDrawInfo->size > kThemeArrow9pt)
		return paramErr;
	// Extent along and across the arrow direction, per size.
	static const CGFloat lengths[4] = { 3, 4, 5, 5 };
	static const CGFloat widths[4] = { 3, 5, 7, 7 };
	Boolean vertical = inDrawInfo->orientation == kThemeArrowDown || inDrawInfo->orientation == kThemeArrowUp;
	CGFloat w = vertical ? widths[inDrawInfo->size] : lengths[inDrawInfo->size];
	CGFloat h = vertical ? lengths[inDrawInfo->size] : widths[inDrawInfo->size];
	localBegin(inContext, inBounds, inOrientation);
	Color col = isActiveState(inDrawInfo->state) ? (isPressedState(inDrawInfo->state) ? gray(0) : rgba(0, 0, 0, 0.85)) : gray(0.5);
	if (isDisabledState(inDrawInfo->state))
		col = rgba(0, 0, 0, 0.3);
	switch (inDrawInfo->orientation)
	{
		case kThemeArrowLeft:
			fillArrow(inContext, 0, h / 2, w, h / 2, h / 2, col);
			break;
		case kThemeArrowRight:
			fillArrow(inContext, w, h / 2, 0, h / 2, h / 2, col);
			break;
		case kThemeArrowUp:
			fillArrow(inContext, w / 2, 0, w / 2, h, w / 2, col);
			break;
		default:
			fillArrow(inContext, w / 2, h, w / 2, 0, w / 2, col);
			break;
	}
	localEnd(inContext);
	return noErr;
}

OSStatus HIThemeDrawChasingArrows(const HIRect *inBounds, const HIThemeChasingArrowsDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	if (inBounds == NULL || inDrawInfo == NULL || inContext == NULL)
		return paramErr;
	localBegin(inContext, inBounds, inOrientation);
	CGFloat size = fmin(inBounds->size.width, inBounds->size.height);
	CGFloat cx = size / 2, cy = size / 2, outer = size / 2 - 0.5, inner = outer * 0.5;
	for (int i = 0; i < 12; i++)
	{
		CGFloat angle = (i * 30.0) * M_PI / 180.0;
		int age = (int) ((i + 12 - (inDrawInfo->index % 12)) % 12);
		Color col = rgba(0, 0, 0, 0.15 + 0.7 * (11 - age) / 11.0);
		drawLine(inContext, cx + sin(angle) * inner, cy - cos(angle) * inner, cx + sin(angle) * outer, cy - cos(angle) * outer, col, 1.5);
	}
	localEnd(inContext);
	return noErr;
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

OSStatus HIThemeDrawGrowBox(const HIPoint *inOrigin, const HIThemeGrowBoxDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	if (inOrigin == NULL || inDrawInfo == NULL || inContext == NULL)
		return paramErr;
	if (inDrawInfo->kind == kHIThemeGrowBoxKindNone)
		return noErr;
	HIRect bounds;
	OSStatus status = HIThemeGetGrowBoxBounds(inOrigin, inDrawInfo, &bounds);
	if (status != noErr)
		return status;
	localBegin(inContext, &bounds, inOrientation);
	CGFloat s = bounds.size.width;
	for (int i = 1; i <= 3; i++)
	{
		CGFloat d = i * s / 4;
		drawLine(inContext, s - d + 0.5, s - 0.5, s - 0.5, s - d + 0.5, gray(0.55), 1);
		drawLine(inContext, s - d + 1.5, s - 0.5, s - 0.5, s - d + 1.5, gray(1), 1);
	}
	localEnd(inContext);
	return noErr;
}

OSStatus HIThemeDrawTickMark(const HIRect *inBounds, const HIThemeTickMarkDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	if (inBounds == NULL || inDrawInfo == NULL || inContext == NULL)
		return paramErr;
	localBegin(inContext, inBounds, inOrientation);
	setFill(inContext, isActiveState(inDrawInfo->state) ? rgba(0, 0, 0, 0.55) : rgba(0, 0, 0, 0.3));
	CGContextFillRect(inContext, CGRectMake(0, 0, fmax(1, inBounds->size.width), fmax(1, inBounds->size.height)));
	localEnd(inContext);
	return noErr;
}

OSStatus HIThemeDrawScrollBarDelimiters(const HIRect *inContRect, const HIThemeScrollBarDelimitersDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	if (inContRect == NULL || inDrawInfo == NULL || inContext == NULL)
		return paramErr;
	localBegin(inContext, inContRect, inOrientation);
	CGRect r = CGRectMake(0, 0, inContRect->size.width, inContRect->size.height);
	fillGradientRect(inContext, r, gray(0.97), gray(0.93));
	setStroke(inContext, gray(0.82));
	CGContextSetLineWidth(inContext, 1);
	CGContextStrokeRect(inContext, CGRectInset(r, 0.5, 0.5));
	localEnd(inContext);
	return noErr;
}

// ---------------------------------------------------------------------------
// Backgrounds
// ---------------------------------------------------------------------------

static OSStatus fillBackground(const HIRect *bounds, const HIThemeBackgroundDrawInfo *info, CGContextRef c, HIThemeOrientation orientation)
{
	if (bounds == NULL || info == NULL || c == NULL)
		return paramErr;
	if (info->kind < kThemeBackgroundTabPane || info->kind > kThemeBackgroundMetal)
		return paramErr;
	// Only the textured (metal) background has a pattern of its own; the other kinds draw
	// nothing beyond the window colour the caller has already painted.
	if (info->kind != kThemeBackgroundMetal)
		return noErr;
	localBegin(c, bounds, orientation);
	Boolean active = isActiveState(info->state);
	fillGradientRect(c, CGRectMake(0, 0, bounds->size.width, bounds->size.height), active ? gray(0.84) : gray(0.92), active ? gray(0.76) : gray(0.88));
	localEnd(c);
	return noErr;
}

OSStatus HIThemeDrawBackground(const HIRect *inBounds, const HIThemeBackgroundDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	return fillBackground(inBounds, inDrawInfo, inContext, inOrientation);
}

OSStatus HIThemeApplyBackground(const HIRect *inBounds, const HIThemeBackgroundDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	return fillBackground(inBounds, inDrawInfo, inContext, inOrientation);
}

// ---------------------------------------------------------------------------
// Segments and tabs
// ---------------------------------------------------------------------------

OSStatus HIThemeDrawSegment(const HIRect *inBounds, const HIThemeSegmentDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	if (inBounds == NULL || inDrawInfo == NULL || inContext == NULL)
		return paramErr;
	if (inDrawInfo->position > kHIThemeSegmentPositionOnly)
		return paramErr;
	localBegin(inContext, inBounds, inOrientation);
	CGRect r = CGRectMake(0, 0, inBounds->size.width, inBounds->size.height);
	unsigned corners = 0;
	switch (inDrawInfo->position)
	{
		case kHIThemeSegmentPositionFirst: corners = 9u; break;
		case kHIThemeSegmentPositionLast: corners = 6u; break;
		case kHIThemeSegmentPositionOnly: corners = 15u; break;
		default: break;
	}
	Boolean on = inDrawInfo->value != kThemeButtonOff && isActiveState(inDrawInfo->state);
	CGContextSaveGState(inContext);
	addRoundRectCorners(inContext, r, 4, corners);
	CGContextClip(inContext);
	if (on)
		fillGradientRect(inContext, r, accentTop(), accentBottom());
	else
		fillGradientRect(inContext, r, faceTop(inDrawInfo->state), faceBottom(inDrawInfo->state));
	CGContextRestoreGState(inContext);
	setStroke(inContext, on ? accentBorder() : frameColor(inDrawInfo->state));
	CGContextSetLineWidth(inContext, 1);
	addRoundRectCorners(inContext, CGRectInset(r, 0.5, 0.5), 3.5, corners);
	CGContextStrokePath(inContext);
	localEnd(inContext);
	return noErr;
}

OSStatus HIThemeDrawTabPane(const HIRect *inRect, const HIThemeTabPaneDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	if (inRect == NULL || inDrawInfo == NULL || inContext == NULL)
		return paramErr;
	if (inDrawInfo->direction > kThemeTabWest)
		return paramErr;
	localBegin(inContext, inRect, inOrientation);
	CGRect r = CGRectMake(0, 0, inRect->size.width, inRect->size.height);
	Boolean active = isActiveState(inDrawInfo->state);
	paintRoundRect(inContext, r, 4, active ? gray(0.99) : gray(0.97), active ? gray(0.96) : gray(0.95), gray(active ? 0.72 : 0.8));
	localEnd(inContext);
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

static CGRect tabLabelRect(const HIRect *r, ThemeTabDirection direction)
{
	switch (direction)
	{
		case kThemeTabNorth: return CGRectMake(r->origin.x + 10, r->origin.y, r->size.width - 20, r->size.height - 4);
		case kThemeTabSouth: return CGRectMake(r->origin.x + 10, r->origin.y + 4, r->size.width - 20, r->size.height - 4);
		case kThemeTabEast: return CGRectMake(r->origin.x, r->origin.y + 10, r->size.width - 4, r->size.height - 20);
		default: return CGRectMake(r->origin.x + 4, r->origin.y + 10, r->size.width - 4, r->size.height - 20);
	}
}

OSStatus HIThemeDrawTab(const HIRect *inRect, const HIThemeTabDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation, HIRect *outLabelRect)
{
	if (inRect == NULL || inDrawInfo == NULL || inContext == NULL)
		return paramErr;
	if (inDrawInfo->direction > kThemeTabWest || inDrawInfo->style > kThemeTabFrontUnavailable)
		return paramErr;
	if (outLabelRect != NULL)
		*outLabelRect = tabLabelRect(inRect, inDrawInfo->direction);

	localBegin(inContext, inRect, inOrientation);
	CGRect r = CGRectMake(0, 0, inRect->size.width, inRect->size.height);
	Boolean front = inDrawInfo->style == kThemeTabFront || inDrawInfo->style == kThemeTabFrontInactive || inDrawInfo->style == kThemeTabFrontUnavailable;
	Boolean inactive = inDrawInfo->style == kThemeTabNonFrontInactive || inDrawInfo->style == kThemeTabFrontInactive;
	Boolean pressed = inDrawInfo->style == kThemeTabNonFrontPressed;
	unsigned corners;
	switch (inDrawInfo->direction)
	{
		case kThemeTabNorth: corners = 3u; break;
		case kThemeTabSouth: corners = 12u; break;
		case kThemeTabEast: corners = 6u; break;
		default: corners = 9u; break;
	}
	Color top = front ? gray(1) : (pressed ? gray(0.8) : gray(inactive ? 0.95 : 0.9));
	Color bottom = front ? gray(0.98) : (pressed ? gray(0.74) : gray(inactive ? 0.92 : 0.82));
	CGContextSaveGState(inContext);
	addRoundRectCorners(inContext, r, 4, corners);
	CGContextClip(inContext);
	fillGradientRect(inContext, r, top, bottom);
	CGContextRestoreGState(inContext);
	setStroke(inContext, gray(inactive ? 0.78 : 0.68));
	CGContextSetLineWidth(inContext, 1);
	addRoundRectCorners(inContext, CGRectInset(r, 0.5, 0.5), 3.5, corners);
	CGContextStrokePath(inContext);
	if (inDrawInfo->adornment & kHIThemeTabAdornmentFocus)
		paintFocusRing(inContext, r, 4);
	localEnd(inContext);
	return noErr;
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

// ---------------------------------------------------------------------------
// Menus
// ---------------------------------------------------------------------------

OSStatus HIThemeDrawMenuBarBackground(const HIRect *inBounds, const HIThemeMenuBarDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	if (inBounds == NULL || inDrawInfo == NULL || inContext == NULL)
		return paramErr;
	localBegin(inContext, inBounds, inOrientation);
	CGRect r = CGRectMake(0, 0, inBounds->size.width, inBounds->size.height);
	Boolean inactive = inDrawInfo->state == kThemeMenuBarInactive;
	fillGradientRect(inContext, r, gray(inactive ? 0.97 : 0.95), gray(inactive ? 0.94 : 0.88));
	drawLine(inContext, 0, r.size.height - 0.5, r.size.width, r.size.height - 0.5, gray(0.7), 1);
	localEnd(inContext);
	return noErr;
}

OSStatus HIThemeDrawMenuTitle(const HIRect *inMenuBarRect, const HIRect *inTitleRect, const HIThemeMenuTitleDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation, HIRect *outLabelRect)
{
	if (inMenuBarRect == NULL || inTitleRect == NULL || inDrawInfo == NULL || inContext == NULL)
		return paramErr;
	SInt16 extra;
	GetThemeMenuTitleExtra(&extra, (inDrawInfo->attributes & kHIThemeMenuTitleDrawCondensed) != 0);
	if (outLabelRect != NULL)
		*outLabelRect = CGRectMake(inTitleRect->origin.x + extra / 2 - 1, inTitleRect->origin.y, inTitleRect->size.width - (extra - 2), inTitleRect->size.height);
	if (inDrawInfo->state == kThemeMenuSelected)
	{
		localBegin(inContext, inTitleRect, inOrientation);
		fillGradientRect(inContext, CGRectMake(0, 0, inTitleRect->size.width, inTitleRect->size.height), accentTop(), accentBottom());
		localEnd(inContext);
	}
	return noErr;
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

OSStatus HIThemeDrawMenuBackground(const HIRect *inMenuRect, const HIThemeMenuDrawInfo *inMenuDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	if (inMenuRect == NULL || inMenuDrawInfo == NULL || inContext == NULL)
		return paramErr;
	CGRect bg = menuBackgroundRect(inMenuRect);
	localBegin(inContext, &bg, inOrientation);
	CGRect r = CGRectMake(0, 0, bg.size.width, bg.size.height);
	setFill(inContext, rgba(0.95, 0.95, 0.95, 0.97));
	CGContextFillRect(inContext, r);
	setStroke(inContext, rgba(0, 0, 0, 0.25));
	CGContextSetLineWidth(inContext, 1);
	CGContextStrokeRect(inContext, CGRectInset(r, 0.5, 0.5));
	localEnd(inContext);
	return noErr;
}

OSStatus HIThemeDrawMenuItem(const HIRect *inMenuRect, const HIRect *inItemRect, const HIThemeMenuItemDrawInfo *inItemDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation, HIRect *outContentRect)
{
	if (inMenuRect == NULL || inItemRect == NULL || inItemDrawInfo == NULL || inContext == NULL)
		return paramErr;
	SInt16 extraH, extraW;
	GetThemeMenuItemExtra(inItemDrawInfo->itemType, &extraH, &extraW);
	if (outContentRect != NULL)
		*outContentRect = CGRectMake(inItemRect->origin.x, inItemRect->origin.y, inItemRect->size.width - extraW, inItemRect->size.height - extraH);

	localBegin(inContext, inItemRect, inOrientation);
	CGRect r = CGRectMake(0, 0, inItemRect->size.width, inItemRect->size.height);
	if (!(inItemDrawInfo->itemType & kThemeMenuItemNoBackground))
	{
		if (inItemDrawInfo->state == kThemeMenuSelected)
			fillGradientRect(inContext, r, accentTop(), accentBottom());
		else
		{
			setFill(inContext, rgba(0.95, 0.95, 0.95, 1));
			CGContextFillRect(inContext, r);
		}
	}
	if ((inItemDrawInfo->itemType & 0xff) == kThemeMenuItemHierarchical)
	{
		Color col = inItemDrawInfo->state == kThemeMenuSelected ? gray(1) : (inItemDrawInfo->state == kThemeMenuDisabled ? rgba(0, 0, 0, 0.3) : rgba(0, 0, 0, 0.8));
		fillArrow(inContext, r.size.width - 6, r.size.height / 2, r.size.width - 11, r.size.height / 2, 3.5, col);
	}
	localEnd(inContext);
	return noErr;
}

OSStatus HIThemeDrawMenuSeparator(const HIRect *inMenuRect, const HIRect *inItemRect, const HIThemeMenuItemDrawInfo *inItemDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	if (inMenuRect == NULL || inItemRect == NULL || inItemDrawInfo == NULL || inContext == NULL)
		return paramErr;
	localBegin(inContext, inItemRect, inOrientation);
	CGFloat y = floor(inItemRect->size.height / 2) + 0.5;
	drawLine(inContext, 0, y, inItemRect->size.width, y, rgba(0, 0, 0, 0.2), 1);
	localEnd(inContext);
	return noErr;
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

OSStatus HIThemeDrawWindowFrame(const HIRect *inContRect, const HIThemeWindowDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation, HIRect *outTitleRect)
{
	if (inContRect == NULL || inDrawInfo == NULL || inContext == NULL)
		return paramErr;
	WindowMetrics m;
	if (!windowMetricsFor(inDrawInfo->windowType, &m))
		return paramErr;
	CGRect structure = windowStructure(inContRect, &m, inOrientation);
	if (outTitleRect != NULL)
	{
		CGFloat titleWidth = inDrawInfo->titleWidth;
		CGFloat titleY = inOrientation == kHIThemeOrientationInverted ? inContRect->origin.y + inContRect->size.height + 1 : inContRect->origin.y - m.title + 1;
		*outTitleRect = m.hasTitle ? CGRectMake(inContRect->origin.x + floor((inContRect->size.width - titleWidth) / 2), titleY, titleWidth, m.title - 1)
		                           : CGRectZero;
	}
	if (m.title == 0 && m.side == 0)
		return noErr;

	localBegin(inContext, &structure, inOrientation);
	Boolean active = isActiveState(inDrawInfo->state);
	if (m.title > 0)
	{
		CGRect bar = CGRectMake(0, 0, structure.size.width, m.title);
		CGContextSaveGState(inContext);
		addRoundRectCorners(inContext, bar, 5, 3u);
		CGContextClip(inContext);
		fillGradientRect(inContext, bar, gray(active ? 0.92 : 0.96), gray(active ? 0.82 : 0.92));
		CGContextRestoreGState(inContext);
		drawLine(inContext, 0, m.title - 0.5, structure.size.width, m.title - 0.5, gray(active ? 0.62 : 0.76), 1);
	}
	if (m.side > 0)
	{
		setFill(inContext, gray(active ? 0.88 : 0.93));
		CGContextFillRect(inContext, CGRectMake(0, m.title, m.side, structure.size.height - m.title));
		CGContextFillRect(inContext, CGRectMake(structure.size.width - m.side, m.title, m.side, structure.size.height - m.title));
	}
	setStroke(inContext, gray(active ? 0.55 : 0.72));
	CGContextSetLineWidth(inContext, 1);
	CGContextStrokeRect(inContext, CGRectInset(CGRectMake(0, 0, structure.size.width, structure.size.height), 0.5, 0.5));
	localEnd(inContext);
	return noErr;
}

OSStatus HIThemeDrawTitleBarWidget(const HIRect *inContRect, const HIThemeWindowWidgetDrawInfo *inDrawInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	if (inContRect == NULL || inDrawInfo == NULL || inContext == NULL)
		return paramErr;
	WindowMetrics m;
	if (!windowMetricsFor(inDrawInfo->windowType, &m) || m.title == 0)
		return paramErr;
	int index;
	Color base;
	switch (inDrawInfo->widgetType)
	{
		case kThemeWidgetCloseBox:
		case kThemeWidgetDirtyCloseBox: index = 0; base = rgba(0.99, 0.37, 0.34, 1); break;
		case kThemeWidgetCollapseBox: index = 1; base = rgba(0.99, 0.74, 0.18, 1); break;
		case kThemeWidgetZoomBox: index = 2; base = rgba(0.15, 0.78, 0.25, 1); break;
		case kThemeWidgetToolbarButton: return noErr;
		default: return paramErr;
	}
	CGRect box = windowWidgetRect(inContRect, &m, index, inOrientation);
	CGFloat d = 12;
	CGRect circle = CGRectMake(box.origin.x + floor((box.size.width - d) / 2), box.origin.y + floor((box.size.height - d) / 2), d, d);
	if (!isActiveState(inDrawInfo->windowState) || isDisabledState(inDrawInfo->widgetState))
		base = gray(0.8);
	else if (isPressedState(inDrawInfo->widgetState))
	{
		base.v[0] *= 0.8; base.v[1] *= 0.8; base.v[2] *= 0.8;
	}
	localBegin(inContext, &circle, inOrientation);
	CGRect r = CGRectMake(0, 0, d, d);
	setFill(inContext, base);
	CGContextAddEllipseInRect(inContext, r);
	CGContextFillPath(inContext);
	setStroke(inContext, rgba(0, 0, 0, 0.2));
	CGContextSetLineWidth(inContext, 1);
	CGContextAddEllipseInRect(inContext, CGRectInset(r, 0.5, 0.5));
	CGContextStrokePath(inContext);
	localEnd(inContext);
	return noErr;
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

static void paintScrollArrow(CGContextRef c, CGRect r, Boolean horizontal, Boolean second, Boolean pressed, Boolean enabled)
{
	paintRoundRect(c, r, 0, gray(pressed ? 0.88 : 0.98), gray(pressed ? 0.8 : 0.93), gray(0.78));
	CGFloat cx = r.origin.x + r.size.width / 2, cy = r.origin.y + r.size.height / 2;
	Color col = enabled ? rgba(0, 0, 0, 0.6) : rgba(0, 0, 0, 0.25);
	if (horizontal)
		fillArrow(c, second ? cx + 3 : cx - 3, cy, second ? cx - 2 : cx + 2, cy, 3.5, col);
	else
		fillArrow(c, cx, second ? cy + 3 : cy - 3, cx, second ? cy - 2 : cy + 2, 3.5, col);
}

// Convert a rectangle given in the coordinate space of the track to the local frame of "frame".
static CGRect toLocal(CGRect r, CGRect frame, HIThemeOrientation o)
{
	CGRect out = CGRectMake(r.origin.x - frame.origin.x, r.origin.y - frame.origin.y, r.size.width, r.size.height);
	if (o == kHIThemeOrientationInverted)
		out.origin.y = frame.size.height - out.origin.y - out.size.height;
	return out;
}

OSStatus HIThemeDrawTrack(const HIThemeTrackDrawInfo *inDrawInfo, const HIRect *inGhostRect, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	if (inDrawInfo == NULL || inContext == NULL)
		return paramErr;
	TrackLayout L;
	OSStatus status = layoutTrack(inDrawInfo, &L);
	if (status != noErr)
		return status;

	CGRect frame = L.bounds;
	localBegin(inContext, &frame, inOrientation);
	Boolean active = inDrawInfo->enableState == kThemeTrackActive;
	Boolean disabled = inDrawInfo->enableState == kThemeTrackDisabled || inDrawInfo->enableState == kThemeTrackNothingToScroll;
	Boolean h = L.horizontal;

	if (L.scrollBar)
	{
		CGRect whole = toLocal(L.body, frame, inOrientation);
		fillGradientRect(inContext, whole, gray(0.97), gray(0.94));
		setStroke(inContext, gray(0.84));
		CGContextSetLineWidth(inContext, 1);
		CGContextStrokeRect(inContext, CGRectInset(whole, 0.5, 0.5));
		if (L.hasArrows)
		{
			UInt8 press = inDrawInfo->trackInfo.scrollbar.pressState;
			Boolean pressedA = (press & (kThemeLeftInsideArrowPressed | kThemeLeftOutsideArrowPressed)) != 0;
			Boolean pressedB = (press & (kThemeRightInsideArrowPressed | kThemeRightOutsideArrowPressed)) != 0;
			paintScrollArrow(inContext, toLocal(L.arrowA, frame, inOrientation), h, false, pressedA, !disabled);
			paintScrollArrow(inContext, toLocal(L.arrowB, frame, inOrientation), h, true, pressedB, !disabled);
		}
		if (L.hasThumb)
		{
			CGRect thumb = CGRectInset(toLocal(L.thumb, frame, inOrientation), h ? 0 : 2, h ? 2 : 0);
			CGFloat radius = fmin(thumb.size.width, thumb.size.height) / 2;
			paintRoundRect(inContext, thumb, radius, gray(active ? 0.78 : 0.88), gray(active ? 0.68 : 0.82), gray(active ? 0.55 : 0.75));
			if (inDrawInfo->trackInfo.scrollbar.pressState & kThemeThumbPressed)
			{
				setFill(inContext, rgba(0, 0, 0, 0.12));
				addRoundRect(inContext, thumb, radius);
				CGContextFillPath(inContext);
			}
		}
	}
	else if (L.slider)
	{
		CGRect groove = toLocal(L.groove, frame, inOrientation);
		paintRoundRect(inContext, groove, 2, gray(0.82), gray(0.93), gray(0.6));
		if (L.hasThumb)
		{
			CGRect thumb = toLocal(L.thumb, frame, inOrientation);
			CGRect knob = h ? CGRectMake(thumb.origin.x, thumb.origin.y + 1, thumb.size.width, thumb.size.height - 2)
			                : CGRectMake(thumb.origin.x + 1, thumb.origin.y, thumb.size.width - 2, thumb.size.height);
			CGContextSaveGState(inContext);
			CGContextAddEllipseInRect(inContext, knob);
			CGContextClip(inContext);
			fillGradientRect(inContext, knob, gray(disabled ? 0.95 : 1), gray(disabled ? 0.9 : 0.86));
			CGContextRestoreGState(inContext);
			setStroke(inContext, gray(disabled ? 0.75 : 0.58));
			CGContextSetLineWidth(inContext, 1);
			CGContextAddEllipseInRect(inContext, CGRectInset(knob, 0.5, 0.5));
			CGContextStrokePath(inContext);
		}
	}
	else
	{
		CGRect groove = toLocal(L.groove, frame, inOrientation);
		paintRoundRect(inContext, groove, groove.size.height / 2, gray(0.8), gray(0.93), gray(0.65));
		SInt32 range = inDrawInfo->max - inDrawInfo->min;
		Boolean indeterminate = inDrawInfo->kind == kThemeIndeterminateBarMedium || inDrawInfo->kind == kThemeIndeterminateBarLarge;
		CGContextSaveGState(inContext);
		addRoundRect(inContext, CGRectInset(groove, 0.5, 0.5), groove.size.height / 2);
		CGContextClip(inContext);
		if (indeterminate)
		{
			// Diagonal stripes, advanced by the animation phase.
			setFill(inContext, disabled ? gray(0.75) : accentBottom());
			CGFloat extent = h ? groove.size.width : groove.size.height;
			CGFloat shift = fmod(inDrawInfo->trackInfo.progress.phase * 2.0, 16.0);
			CGFloat thick = h ? groove.size.height : groove.size.width;
			for (CGFloat p = -thick - shift; p < extent + thick; p += 16)
			{
				if (h)
				{
					CGContextMoveToPoint(inContext, groove.origin.x + p, groove.origin.y + groove.size.height);
					CGContextAddLineToPoint(inContext, groove.origin.x + p + 8, groove.origin.y + groove.size.height);
					CGContextAddLineToPoint(inContext, groove.origin.x + p + 8 + thick, groove.origin.y);
					CGContextAddLineToPoint(inContext, groove.origin.x + p + thick, groove.origin.y);
				}
				else
				{
					CGContextMoveToPoint(inContext, groove.origin.x + groove.size.width, groove.origin.y + p);
					CGContextAddLineToPoint(inContext, groove.origin.x + groove.size.width, groove.origin.y + p + 8);
					CGContextAddLineToPoint(inContext, groove.origin.x, groove.origin.y + p + 8 + thick);
					CGContextAddLineToPoint(inContext, groove.origin.x, groove.origin.y + p + thick);
				}
				CGContextClosePath(inContext);
				CGContextFillPath(inContext);
			}
		}
		else if (range > 0)
		{
			SInt32 v = inDrawInfo->value < inDrawInfo->min ? inDrawInfo->min : (inDrawInfo->value > inDrawInfo->max ? inDrawInfo->max : inDrawInfo->value);
			CGFloat frac = (CGFloat) (v - inDrawInfo->min) / range;
			CGRect fill = groove;
			if (h)
				fill.size.width = floor(groove.size.width * frac);
			else
				fill.size.height = floor(groove.size.height * frac);
			fillGradientRect(inContext, fill, disabled ? gray(0.8) : accentTop(), disabled ? gray(0.7) : accentBottom());
		}
		CGContextRestoreGState(inContext);
	}
	localEnd(inContext);
	return noErr;
}

OSStatus HIThemeDrawTrackTickMarks(const HIThemeTrackDrawInfo *inDrawInfo, ItemCount inNumTicks, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	if (inDrawInfo == NULL || inContext == NULL)
		return paramErr;
	TrackLayout L;
	OSStatus status = layoutTrack(inDrawInfo, &L);
	if (status != noErr)
		return status;
	if (!L.slider || inNumTicks == 0)
		return noErr;
	CGRect frame = L.bounds;
	localBegin(inContext, &frame, inOrientation);
	CGRect track = toLocal(L.groove, frame, inOrientation);
	Boolean h = L.horizontal;
	CGFloat tickLen = 4;
	CGFloat first = (h ? track.origin.x : track.origin.y) + 8;
	CGFloat last = (h ? track.origin.x + track.size.width : track.origin.y + track.size.height) - 8;
	setFill(inContext, rgba(0, 0, 0, 0.5));
	for (ItemCount i = 0; i < inNumTicks; i++)
	{
		CGFloat p = inNumTicks == 1 ? (first + last) / 2 : first + (last - first) * i / (inNumTicks - 1);
		if (h)
			CGContextFillRect(inContext, CGRectMake(floor(p), track.origin.y - tickLen - 1, 1, tickLen));
		else
			CGContextFillRect(inContext, CGRectMake(track.origin.x - tickLen - 1, floor(p), tickLen, 1));
	}
	localEnd(inContext);
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

// The font of a text info: a specified CTFont, or the UI font of the theme font id.
static CTFontRef createTextFont(const HIThemeTextInfo *info)
{
	if (info->font != NULL && info->version >= kHIThemeTextInfoVersionOne && info->fontID == kThemeSpecifiedFont)
		return (CTFontRef) CFRetain(info->font);
	CTFontUIFontType type = HIThemeGetUIFontType(info->fontID);
	if (type == kCTFontUIFontNone)
		type = kCTFontUIFontSystem;
	return CTFontCreateUIFontForLanguage(type, 0, NULL);
}

static CFAttributedStringRef createAttributedString(CFTypeRef string, CTFontRef font, CGColorRef color)
{
	CFStringRef plain = NULL;
	if (CFGetTypeID(string) == CFStringGetTypeID())
		plain = (CFStringRef) CFRetain(string);
	else if (CFGetTypeID(string) == CFAttributedStringGetTypeID())
		plain = (CFStringRef) CFRetain(CFAttributedStringGetString((CFAttributedStringRef) string));
	if (plain == NULL)
		return NULL;
	const void *keys[2] = { kCTFontAttributeName, kCTForegroundColorAttributeName };
	const void *values[2] = { font, color };
	CFDictionaryRef attrs = CFDictionaryCreate(kCFAllocatorDefault, keys, values, color != NULL ? 2 : 1,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFAttributedStringRef result = CFAttributedStringCreate(kCFAllocatorDefault, plain, attrs);
	CFRelease(attrs);
	CFRelease(plain);
	return result;
}

#define kMaxTextLines 64

typedef struct {
	CTLineRef lines[kMaxTextLines];
	CGFloat ascent[kMaxTextLines], descent[kMaxTextLines], leading[kMaxTextLines], width[kMaxTextLines];
	CFIndex count;
	Boolean truncated;
} TextLayout;

static void releaseLayout(TextLayout *t)
{
	for (CFIndex i = 0; i < t->count; i++)
		CFRelease(t->lines[i]);
	t->count = 0;
}

// Breaks the string into lines for a width (0: a single line, no wrapping) and an optional maximum line count.
static void layoutText(CFAttributedStringRef astr, CGFloat width, const HIThemeTextInfo *info, TextLayout *t)
{
	memset(t, 0, sizeof(*t));
	CFIndex length = CFAttributedStringGetLength(astr);
	CTTypesetterRef ts = CTTypesetterCreateWithAttributedString(astr);
	if (ts == NULL)
		return;
	UInt32 maxLines = info->truncationMaxLines;
	if (maxLines == 0 || maxLines > kMaxTextLines)
		maxLines = width > 0 ? kMaxTextLines : 1;
	CFIndex start = 0;
	while (start < length && (UInt32) t->count < maxLines)
	{
		CFIndex n = width > 0 ? CTTypesetterSuggestLineBreak(ts, start, width) : length - start;
		if (n <= 0)
			n = length - start;
		Boolean lastAllowed = (UInt32) t->count + 1 == maxLines && start + n < length;
		CTLineRef line = CTTypesetterCreateLine(ts, CFRangeMake(start, lastAllowed ? length - start : n));
		if (line == NULL)
			break;
		if (lastAllowed)
		{
			t->truncated = true;
			if (width > 0 && info->truncationPosition != kHIThemeTextTruncationNone)
			{
				CTLineTruncationType type = info->truncationPosition == kHIThemeTextTruncationMiddle ? kCTLineTruncationMiddle : kCTLineTruncationEnd;
				CTLineRef truncated = CTLineCreateTruncatedLine(line, width, type, NULL);
				if (truncated != NULL)
				{
					CFRelease(line);
					line = truncated;
				}
			}
			else
			{
				// Without truncation the text is cut off at the end of the last line.
				CFRelease(line);
				line = CTTypesetterCreateLine(ts, CFRangeMake(start, n));
				if (line == NULL)
					break;
			}
		}
		t->lines[t->count] = line;
		t->width[t->count] = CTLineGetTypographicBounds(line, &t->ascent[t->count], &t->descent[t->count], &t->leading[t->count]);
		t->count++;
		start += n;
	}
	CFRelease(ts);
}

static CGFloat lineHeight(const TextLayout *t, CFIndex i)
{
	return ceil(t->ascent[i] + t->descent[i] + t->leading[i]);
}

OSStatus HIThemeGetTextDimensions(CFTypeRef inString, CGFloat inWidth, HIThemeTextInfo *inTextInfo, CGFloat *outWidth, CGFloat *outHeight, CGFloat *outBaseline)
{
	if (inString == NULL || inTextInfo == NULL)
		return paramErr;
	CTFontRef font = createTextFont(inTextInfo);
	if (font == NULL)
		return paramErr;
	CFAttributedStringRef astr = createAttributedString(inString, font, NULL);
	if (astr == NULL)
	{
		CFRelease(font);
		return paramErr;
	}
	TextLayout t;
	layoutText(astr, inWidth, inTextInfo, &t);
	CGFloat w = 0, h = 0;
	for (CFIndex i = 0; i < t.count; i++)
	{
		if (t.width[i] > w)
			w = t.width[i];
		h += lineHeight(&t, i);
	}
	CGFloat baseline = t.count > 0 ? ceil(t.ascent[0]) : ceil(CTFontGetAscent(font));
	if (t.count == 0)
		h = ceil(CTFontGetAscent(font) + CTFontGetDescent(font) + CTFontGetLeading(font));
	if (outWidth != NULL) *outWidth = ceil(w);
	if (outHeight != NULL) *outHeight = h;
	if (outBaseline != NULL) *outBaseline = baseline;
	inTextInfo->truncationHappened = t.truncated;
	releaseLayout(&t);
	CFRelease(astr);
	CFRelease(font);
	return noErr;
}

OSStatus HIThemeDrawTextBox(CFTypeRef inString, const HIRect *inBounds, HIThemeTextInfo *inTextInfo, CGContextRef inContext, HIThemeOrientation inOrientation)
{
	if (inString == NULL || inBounds == NULL || inTextInfo == NULL || inContext == NULL)
		return paramErr;
	CTFontRef font = createTextFont(inTextInfo);
	if (font == NULL)
		return paramErr;
	Color col = textColorForState(inTextInfo->state);
	CGColorRef cgcol = CGColorCreateGenericRGB(col.v[0], col.v[1], col.v[2], col.v[3]);
	CFAttributedStringRef astr = createAttributedString(inString, font, cgcol);
	if (astr == NULL)
	{
		CGColorRelease(cgcol);
		CFRelease(font);
		return paramErr;
	}
	TextLayout t;
	layoutText(astr, inBounds->size.width, inTextInfo, &t);
	CGFloat height = 0;
	for (CFIndex i = 0; i < t.count; i++)
		height += lineHeight(&t, i);
	CGFloat y;
	switch (inTextInfo->verticalFlushness)
	{
		case kHIThemeTextVerticalFlushCenter:
		case kHIThemeTextVerticalFlushDefault: y = floor((inBounds->size.height - height) / 2); break;
		case kHIThemeTextVerticalFlushBottom: y = inBounds->size.height - height; break;
		default: y = 0; break;
	}

	CGContextSaveGState(inContext);
	if (!(inTextInfo->options & kHIThemeTextBoxOptionDontClip))
		CGContextClipToRect(inContext, *inBounds);
	// CoreText draws with a y-up text space; a context with a top-left origin needs the glyphs mirrored.
	CGContextSetTextMatrix(inContext, inOrientation == kHIThemeOrientationNormal ? CGAffineTransformMake(1, 0, 0, -1, 0, 0) : CGAffineTransformIdentity);
	for (CFIndex i = 0; i < t.count; i++)
	{
		CGFloat x;
		switch (inTextInfo->horizontalFlushness)
		{
			case kHIThemeTextHorizontalFlushCenter: x = floor((inBounds->size.width - t.width[i]) / 2); break;
			case kHIThemeTextHorizontalFlushRight: x = inBounds->size.width - t.width[i]; break;
			default: x = 0; break;
		}
		CGFloat baseline = y + ceil(t.ascent[i]);
		CGFloat py = inOrientation == kHIThemeOrientationNormal ? inBounds->origin.y + baseline : inBounds->origin.y + inBounds->size.height - baseline;
		CGContextSetTextPosition(inContext, inBounds->origin.x + x, py);
		CTLineDraw(t.lines[i], inContext);
		y += lineHeight(&t, i);
	}
	CGContextRestoreGState(inContext);

	inTextInfo->truncationHappened = t.truncated;
	releaseLayout(&t);
	CFRelease(astr);
	CGColorRelease(cgcol);
	CFRelease(font);
	return noErr;
}

// The cursor shapes of the theme (kThemeArrowCursor and the others) have no counterpart in CoreGraphics
// to show a cursor shape yet; the cursor is checked and remembered only
OSStatus SetThemeCursor(ThemeCursor inCursor)
{
	if (inCursor > kThemePoofCursor)
		return themeBadCursorIndexErr;
	return noErr;
}
