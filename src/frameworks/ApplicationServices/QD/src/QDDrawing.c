/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// Graphics state and drawing on the pixel memory of QuickDraw ports (32-bit only, as on macOS).
//
// Every port carries the QuickDraw graphics state (pen, colors, patterns, text settings, clip
// region). Drawing writes real pixels of the port's PixMap: 32-bit ARGB (depth 32/24), 32-bit
// BGRA (depth 0) and 16-bit 1-5-5-5. Text drawing and picture recording are not implemented
// here: DrawString, StringWidth, OpenCPicture and ClosePicture are absent on purpose.
#ifndef __LP64__

#include "QDPortPrivate.h"
#include <CoreGraphics/CGDirectDisplay.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#pragma pack(push, 2)
typedef struct QDPenState { Point pnLoc; Point pnSize; short pnMode; QDPattern pnPat; } QDPenState;
typedef struct QDCursor { SInt16 data[16]; SInt16 mask[16]; Point hotSpot; } QDCursor;
typedef struct QDHSVColor { UInt16 hue, saturation, value; } QDHSVColor;
#pragma pack(pop)

// Transfer modes (Imaging With QuickDraw, 3-10 and 4-34).
enum {
	kSrcCopy = 0, kSrcOr = 1, kSrcXor = 2, kSrcBic = 3, kNotSrcCopy = 4,
	kPatCopy = 8, kPatOr = 9, kPatXor = 10, kPatBic = 11,
	kNotPatCopy = 12, kNotPatOr = 13, kNotPatXor = 14, kNotPatBic = 15,
	kTransparent = 36, kAddOver = 34, kDitherCopy = 64
};

static const QDPattern kBlack = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};
static const QDPattern kWhite = {{0, 0, 0, 0, 0, 0, 0, 0}};
static const QDPattern kGray = {{0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55}};
static const QDPattern kLtGray = {{0x88, 0x22, 0x88, 0x22, 0x88, 0x22, 0x88, 0x22}};
static const QDPattern kDkGray = {{0x77, 0xDD, 0x77, 0xDD, 0x77, 0xDD, 0x77, 0xDD}};
static const RGBColor kRGBBlack = {0, 0, 0};
static const RGBColor kRGBWhite = {0xFFFF, 0xFFFF, 0xFFFF};

static struct QDPort *gPorts;

// ---------------------------------------------------------------- port registry and state

void QDPortInitState(struct QDPort *port)
{
	port->penLoc.h = port->portRect.left;
	port->penLoc.v = port->portRect.top;
	port->penSize.h = port->penSize.v = 1;
	port->penMode = kPatCopy;
	port->penVis = 0;
	port->penPat = kBlack;
	port->bkPat = kWhite;
	port->fgColor = kRGBBlack;
	port->bkColor = kRGBWhite;
	port->txFont = 0;
	port->txSize = 0;
	port->txMode = kSrcOr;
	port->txFace = 0;
	port->clipRgn = NULL;
	port->clipIsRect = false;
	port->globalOrigin.h = port->globalOrigin.v = 0;
	port->next = gPorts;
	gPorts = port;
}

void QDPortDisposeState(struct QDPort *port)
{
	for (struct QDPort **p = &gPorts; *p; p = &(*p)->next)
		if (*p == port) {
			*p = port->next;
			break;
		}
	if (port->clipRgn)
		DisposeRgn(port->clipRgn);
	port->clipRgn = NULL;
}

Boolean QDPortIsLive(const struct QDPort *port)
{
	for (struct QDPort *p = gPorts; p; p = p->next)
		if (p == port)
			return true;
	return false;
}

// The BitMap pointer CopyBits receives for a port is the address of its PixMap.
struct QDPort *QDPortFromPixMap(const void *bits)
{
	for (struct QDPort *p = gPorts; p; p = p->next)
		if ((const void *) &p->pixMap == bits)
			return p;
	return NULL;
}

static struct QDPort *current(void)
{
	struct QDPort *p = PORT_FROM(gCurrentPort);
	if (p == NULL || !QDPortIsLive(p)) {
		gQDError = paramErr;
		return NULL;
	}
	return p;
}

// Not an Apple API, and exported on purpose: the Window Manager (HIToolbox, MacWindows.c) records where a window port
// lies on the screen, which no public call of QuickDraw sets.
void QDPortSetGlobalOrigin(CGrafPtr port, short h, short v)
{
	struct QDPort *p = PORT_FROM(port);
	if (p && QDPortIsLive(p)) {
		p->globalOrigin.h = h;
		p->globalOrigin.v = v;
	}
}

// ---------------------------------------------------------------- pixels

static uint8_t *pixel_at(struct QDPort *p, int x, int y)
{
	const Rect *b = &p->pixMap.bounds;
	if (x < b->left || x >= b->right || y < b->top || y >= b->bottom)
		return NULL;
	return (uint8_t *) p->pixels + (size_t) (y - b->top) * p->rowBytes
		+ (size_t) (x - b->left) * (p->pixMap.pixelSize / 8);
}

// The luminance of a color in 16 bits, with the weights of the NTSC primaries
static uint16_t luminance(RGBColor c)
{
	return (uint16_t) ((299u * c.red + 587u * c.green + 114u * c.blue) / 1000u);
}

static void store(struct QDPort *p, uint8_t *a, RGBColor c)
{
	if (p->pixMap.pixelSize == 8) {
		// 8-bit gray: 0 is white
		a[0] = (uint8_t) (0xFF - (luminance(c) >> 8));
	} else if (p->pixMap.pixelSize == 16) {
		uint16_t v = (uint16_t) (((c.red >> 11) << 10) | ((c.green >> 11) << 5) | (c.blue >> 11));
		if (p->pixMap.pixelFormat == 0x4C353535 /* L555 */) { a[1] = v >> 8; a[0] = v & 0xFF; }
		else { a[0] = v >> 8; a[1] = v & 0xFF; }
	} else if (p->pixMap.pixelFormat == 0x42475241 /* BGRA */) {
		a[0] = c.blue >> 8; a[1] = c.green >> 8; a[2] = c.red >> 8; a[3] = 0xFF;
	} else {
		a[0] = 0xFF; a[1] = c.red >> 8; a[2] = c.green >> 8; a[3] = c.blue >> 8;
	}
}

static uint16_t widen8(uint8_t v) { return (uint16_t) (v << 8 | v); }

static uint16_t widen5(unsigned v) { v &= 31; return (uint16_t) ((v << 11) | (v << 6) | (v << 1) | (v >> 4)); }

static RGBColor load(struct QDPort *p, const uint8_t *a)
{
	RGBColor c;
	if (p->pixMap.pixelSize == 8) {
		c.red = c.green = c.blue = widen8((uint8_t) (0xFF - a[0]));
	} else if (p->pixMap.pixelSize == 16) {
		unsigned v = p->pixMap.pixelFormat == 0x4C353535 ? ((unsigned) a[1] << 8 | a[0]) : ((unsigned) a[0] << 8 | a[1]);
		c.red = widen5(v >> 10); c.green = widen5(v >> 5); c.blue = widen5(v);
	} else if (p->pixMap.pixelFormat == 0x42475241) {
		c.red = widen8(a[2]); c.green = widen8(a[1]); c.blue = widen8(a[0]);
	} else {
		c.red = widen8(a[1]); c.green = widen8(a[2]); c.blue = widen8(a[3]);
	}
	return c;
}

static Boolean clipped_in(struct QDPort *p, int x, int y)
{
	if (p->clipRgn == NULL)
		return true;
	if (p->clipIsRect)
		return x >= p->clipRect.left && x < p->clipRect.right && y >= p->clipRect.top && y < p->clipRect.bottom;
	Point pt = {(short) y, (short) x};
	return PtInRgn(pt, p->clipRgn);
}

static Boolean pattern_bit(const QDPattern *pat, int x, int y)
{
	return (pat->pat[y & 7] >> (7 - (x & 7))) & 1;
}

// Pattern transfer of one pixel (Imaging With QuickDraw, Table 3-1, color behaviour 4-34).
static void pattern_pixel(struct QDPort *p, int x, int y, const QDPattern *pat, short mode,
		RGBColor fg, RGBColor bg)
{
	uint8_t *a = pixel_at(p, x, y);
	if (a == NULL || !clipped_in(p, x, y))
		return;
	Boolean bit = pattern_bit(pat, x, y);
	if (mode & 4)
		bit = !bit;
	switch (mode & 3) {
		case 0: store(p, a, bit ? fg : bg); break;
		case 1: if (bit) store(p, a, fg); break;
		case 2:
			if (bit) {
				RGBColor c = load(p, a);
				c.red = ~c.red; c.green = ~c.green; c.blue = ~c.blue;
				store(p, a, c);
			}
			break;
		case 3: if (bit) store(p, a, bg); break;
	}
}

static void fill_rect(struct QDPort *p, const Rect *r, const QDPattern *pat, short mode, RGBColor fg, RGBColor bg)
{
	Rect area;
	if (r == NULL || !SectRect(r, &p->pixMap.bounds, &area))
		return;
	for (int y = area.top; y < area.bottom; y++)
		for (int x = area.left; x < area.right; x++)
			pattern_pixel(p, x, y, pat, mode, fg, bg);
}

// ---------------------------------------------------------------- pen and colors

void PenNormal(void)
{
	struct QDPort *p = current();
	if (!p) return;
	p->penSize.h = p->penSize.v = 1;
	p->penMode = kPatCopy;
	p->penPat = kBlack;
}

void PenSize(short width, short height)
{
	struct QDPort *p = current();
	if (!p) return;
	p->penSize.h = width;
	p->penSize.v = height;
}

void PenMode(short mode)
{
	struct QDPort *p = current();
	if (!p) return;
	// The pen uses pattern modes; a source mode selects the matching pattern mode.
	p->penMode = (short) (mode < kPatCopy ? (mode & 7) + kPatCopy : mode);
}

void PenPat(const QDPattern *pat)
{
	struct QDPort *p = current();
	if (p && pat) { p->penPat = *pat; QDRecordPenPat(p, pat); }
}

void BackPat(const QDPattern *pat)
{
	struct QDPort *p = current();
	if (p && pat) p->bkPat = *pat;   // BkPat is not recorded in a picture
}

void HidePen(void) { struct QDPort *p = current(); if (p) p->penVis--; }
void ShowPen(void) { struct QDPort *p = current(); if (p) p->penVis++; }

void GetPen(Point *pt) { struct QDPort *p = current(); if (p && pt) *pt = p->penLoc; }

void GetPenState(QDPenState *s)
{
	struct QDPort *p = current();
	if (!p || !s) return;
	s->pnLoc = p->penLoc; s->pnSize = p->penSize; s->pnMode = p->penMode; s->pnPat = p->penPat;
}

void SetPenState(const QDPenState *s)
{
	struct QDPort *p = current();
	if (!p || !s) return;
	p->penLoc = s->pnLoc; p->penSize = s->pnSize; p->penMode = s->pnMode; p->penPat = s->pnPat;
}

void RGBForeColor(const RGBColor *c) { struct QDPort *p = current(); if (p && c) { p->fgColor = *c; QDRecordColor(p, true, c); } }
void RGBBackColor(const RGBColor *c) { struct QDPort *p = current(); if (p && c) { p->bkColor = *c; QDRecordColor(p, false, c); } }
void GetForeColor(RGBColor *c) { struct QDPort *p = current(); if (p && c) *c = p->fgColor; }
void GetBackColor(RGBColor *c) { struct QDPort *p = current(); if (p && c) *c = p->bkColor; }

// The eight classic colors (Imaging With QuickDraw, 4-61).
static RGBColor classic(long color)
{
	switch (color) {
		case 30: return kRGBWhite;                                  // whiteColor
		case 69: return (RGBColor) {0xFC00, 0xF37D, 0x052F};       // yellowColor
		case 137: return (RGBColor) {0xF2D7, 0x0856, 0x84EC};      // magentaColor
		case 205: return (RGBColor) {0xDD6B, 0x08C2, 0x06A2};      // redColor
		case 273: return (RGBColor) {0x0241, 0xAB54, 0xEAFF};      // cyanColor
		case 341: return (RGBColor) {0x0000, 0x8000, 0x11B0};      // greenColor
		case 409: return (RGBColor) {0x0000, 0x0000, 0xD400};      // blueColor
		default: return kRGBBlack;                                  // blackColor (33)
	}
}

void ForeColor(long color) { struct QDPort *p = current(); if (p) { p->fgColor = classic(color); QDRecordColor(p, true, &p->fgColor); } }
void BackColor(long color) { struct QDPort *p = current(); if (p) { p->bkColor = classic(color); QDRecordColor(p, false, &p->bkColor); } }

// ---------------------------------------------------------------- text state

void TextFont(short font) { struct QDPort *p = current(); if (p) p->txFont = font; }
void TextSize(short size) { struct QDPort *p = current(); if (p) p->txSize = size; }
void TextMode(short mode) { struct QDPort *p = current(); if (p) p->txMode = mode; }
void TextFace(StyleParameter face) { struct QDPort *p = current(); if (p) p->txFace = (Style) face; }
short GetPortTextFont(CGrafPtr port) { struct QDPort *p = PORT_FROM(port); return QDPortIsLive(p) ? p->txFont : 0; }
short GetPortTextSize(CGrafPtr port) { struct QDPort *p = PORT_FROM(port); return QDPortIsLive(p) ? p->txSize : 0; }
short GetPortTextMode(CGrafPtr port) { struct QDPort *p = PORT_FROM(port); return QDPortIsLive(p) ? p->txMode : 0; }
Style GetPortTextFace(CGrafPtr port) { struct QDPort *p = PORT_FROM(port); return QDPortIsLive(p) ? p->txFace : 0; }

// ---------------------------------------------------------------- clip region

void ClipRect(const Rect *r)
{
	struct QDPort *p = current();
	if (!p || !r) return;
	if (p->clipRgn == NULL)
		p->clipRgn = NewRgn();
	RectRgn(p->clipRgn, r);
	p->clipIsRect = true;
	p->clipRect = *r;
}

void SetClip(RgnHandle rgn)
{
	struct QDPort *p = current();
	if (!p || !rgn) return;
	if (p->clipRgn == NULL)
		p->clipRgn = NewRgn();
	CopyRgn(rgn, p->clipRgn);
	Rect b;
	GetRegionBounds(rgn, &b);
	RgnHandle probe = NewRgn();
	RectRgn(probe, &b);
	p->clipIsRect = EqualRgn(probe, rgn);
	DisposeRgn(probe);
	p->clipRect = b;
}

void GetClip(RgnHandle rgn)
{
	struct QDPort *p = current();
	if (!p || !rgn) return;
	if (p->clipRgn)
		CopyRgn(p->clipRgn, rgn);
	else
		SetRectRgn(rgn, -32767, -32767, 32767, 32767);
}

// ---------------------------------------------------------------- shapes

void PaintRect(const Rect *r) { struct QDPort *p = current(); QDRecordRect(p, 0x31, r, NULL); if (p && p->penVis >= 0) fill_rect(p, r, &p->penPat, p->penMode, p->fgColor, p->bkColor); }
void EraseRect(const Rect *r) { struct QDPort *p = current(); QDRecordRect(p, 0x32, r, NULL); if (p) fill_rect(p, r, &p->bkPat, kPatCopy, p->fgColor, p->bkColor); }
void InvertRect(const Rect *r) { struct QDPort *p = current(); QDRecordRect(p, 0x33, r, NULL); if (p) fill_rect(p, r, &kBlack, kPatXor, p->fgColor, p->bkColor); }
void FillRect(const Rect *r, const QDPattern *pat) { struct QDPort *p = current(); QDRecordRect(p, 0x34, r, pat); if (p && pat) fill_rect(p, r, pat, kPatCopy, p->fgColor, p->bkColor); }

// The frame lies inside the rectangle, pen width on the sides and pen height on top and bottom.
void FrameRect(const Rect *r)
{
	struct QDPort *p = current();
	QDRecordRect(p, 0x30, r, NULL);
	if (!p || !r || p->penVis < 0 || r->right <= r->left || r->bottom <= r->top) return;
	short pw = p->penSize.h, ph = p->penSize.v;
	if (pw <= 0 || ph <= 0) return;
	if (r->right - r->left <= 2 * pw || r->bottom - r->top <= 2 * ph) {
		fill_rect(p, r, &p->penPat, p->penMode, p->fgColor, p->bkColor);
		return;
	}
	Rect side;
	SetRect(&side, r->left, r->top, r->right, r->top + ph); fill_rect(p, &side, &p->penPat, p->penMode, p->fgColor, p->bkColor);
	SetRect(&side, r->left, r->bottom - ph, r->right, r->bottom); fill_rect(p, &side, &p->penPat, p->penMode, p->fgColor, p->bkColor);
	SetRect(&side, r->left, r->top + ph, r->left + pw, r->bottom - ph); fill_rect(p, &side, &p->penPat, p->penMode, p->fgColor, p->bkColor);
	SetRect(&side, r->right - pw, r->top + ph, r->right, r->bottom - ph); fill_rect(p, &side, &p->penPat, p->penMode, p->fgColor, p->bkColor);
}

void MoveTo(short h, short v) { struct QDPort *p = current(); if (p) { p->penLoc.h = h; p->penLoc.v = v; } }
void Move(short dh, short dv) { struct QDPort *p = current(); if (p) { p->penLoc.h += dh; p->penLoc.v += dv; } }

// The pen rectangle hangs below and to the right of each point of the line.
void LineTo(short h, short v)
{
	struct QDPort *p = current();
	if (!p) return;
	int x0 = p->penLoc.h, y0 = p->penLoc.v, x1 = h, y1 = v;
	{ Point from = {(short) y0, (short) x0}, to = {v, h}; QDRecordLine(p, from, to); }
	p->penLoc.h = h;
	p->penLoc.v = v;
	if (p->penVis < 0 || p->penSize.h <= 0 || p->penSize.v <= 0) return;
	int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1, dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1, err = dx + dy;
	for (;;) {
		Rect pen;
		SetRect(&pen, (short) x0, (short) y0, (short) (x0 + p->penSize.h), (short) (y0 + p->penSize.v));
		fill_rect(p, &pen, &p->penPat, p->penMode, p->fgColor, p->bkColor);
		if (x0 == x1 && y0 == y1) break;
		int e2 = 2 * err;
		if (e2 >= dy) { err += dy; x0 += sx; }
		if (e2 <= dx) { err += dx; y0 += sy; }
	}
}

void Line(short dh, short dv) { struct QDPort *p = current(); if (p) LineTo(p->penLoc.h + dh, p->penLoc.v + dv); }

void FillRgn(RgnHandle rgn, const QDPattern *pat)
{
	struct QDPort *p = current();
	if (!p || !rgn || !pat) return;
	Rect b, area;
	GetRegionBounds(rgn, &b);
	if (!SectRect(&b, &p->pixMap.bounds, &area)) return;
	for (int y = area.top; y < area.bottom; y++)
		for (int x = area.left; x < area.right; x++) {
			Point pt = {(short) y, (short) x};
			if (PtInRgn(pt, rgn))
				pattern_pixel(p, x, y, pat, kPatCopy, p->fgColor, p->bkColor);
		}
}

void PaintRgn(RgnHandle rgn) { struct QDPort *p = current(); if (p) FillRgn(rgn, &p->penPat); }
void EraseRgn(RgnHandle rgn)
{
	struct QDPort *p = current();
	if (!p) return;
	RGBColor fg = p->fgColor;
	p->fgColor = p->bkColor; // the background pattern is drawn in the background color
	FillRgn(rgn, &kBlack);
	p->fgColor = fg;
}

// Moves the bits of r by (dh, dv) inside r; the uncovered part is erased and returned.
void ScrollRect(const Rect *r, short dh, short dv, RgnHandle updateRgn)
{
	struct QDPort *p = current();
	if (updateRgn) SetEmptyRgn(updateRgn);
	if (!p || !r) return;
	Rect area;
	if (!SectRect(r, &p->pixMap.bounds, &area)) return;
	int w = area.right - area.left, h = area.bottom - area.top, bpp = p->pixMap.pixelSize / 8;
	uint8_t *tmp = malloc((size_t) w * h * bpp);
	if (!tmp) { gQDError = cNoMemErr; return; }
	for (int y = 0; y < h; y++)
		memcpy(tmp + (size_t) y * w * bpp, pixel_at(p, area.left, area.top + y), (size_t) w * bpp);
	for (int y = 0; y < h; y++)
		for (int x = 0; x < w; x++) {
			int sx = x - dh, sy = y - dv;
			uint8_t *dst = pixel_at(p, area.left + x, area.top + y);
			if (!clipped_in(p, area.left + x, area.top + y)) continue;
			if (sx >= 0 && sx < w && sy >= 0 && sy < h)
				memcpy(dst, tmp + ((size_t) sy * w + sx) * bpp, bpp);
			else
				pattern_pixel(p, area.left + x, area.top + y, &p->bkPat, kPatCopy, p->fgColor, p->bkColor);
		}
	free(tmp);
	if (updateRgn) {
		RgnHandle moved = NewRgn(), all = NewRgn();
		Rect m = area;
		OffsetRect(&m, dh, dv);
		SectRect(&m, &area, &m);
		RectRgn(all, &area);
		RectRgn(moved, &m);
		DiffRgn(all, moved, updateRgn);
		DisposeRgn(moved);
		DisposeRgn(all);
	}
}

// ---------------------------------------------------------------- pixels and color conversion

void GetCPixel(short h, short v, RGBColor *c)
{
	struct QDPort *p = current();
	if (!p || !c) return;
	uint8_t *a = pixel_at(p, h, v);
	*c = a ? load(p, a) : kRGBWhite;
}

void SetCPixel(short h, short v, const RGBColor *c)
{
	struct QDPort *p = current();
	if (!p || !c) return;
	uint8_t *a = pixel_at(p, h, v);
	if (a && clipped_in(p, h, v)) store(p, a, *c);
}

// SmallFract components (0..65535); Color Picker Manager, HSV2RGB and RGB2HSV.
void HSV2RGB(const QDHSVColor *hsv, RGBColor *rgb)
{
	if (!hsv || !rgb) return;
	double hh = hsv->hue / 65536.0 * 6.0, s = hsv->saturation / 65535.0, v = hsv->value / 65535.0;
	int i = (int) hh;
	double f = hh - i, pp = v * (1 - s), q = v * (1 - s * f), t = v * (1 - s * (1 - f)), r, g, b;
	switch (i % 6) {
		case 0: r = v; g = t; b = pp; break;
		case 1: r = q; g = v; b = pp; break;
		case 2: r = pp; g = v; b = t; break;
		case 3: r = pp; g = q; b = v; break;
		case 4: r = t; g = pp; b = v; break;
		default: r = v; g = pp; b = q; break;
	}
	rgb->red = (UInt16) (r * 65535 + 0.5);
	rgb->green = (UInt16) (g * 65535 + 0.5);
	rgb->blue = (UInt16) (b * 65535 + 0.5);
}

void RGB2HSV(const RGBColor *rgb, QDHSVColor *hsv)
{
	if (!hsv || !rgb) return;
	double r = rgb->red / 65535.0, g = rgb->green / 65535.0, b = rgb->blue / 65535.0;
	double mx = r > g ? (r > b ? r : b) : (g > b ? g : b), mn = r < g ? (r < b ? r : b) : (g < b ? g : b);
	double d = mx - mn, h = 0;
	if (d > 0) {
		if (mx == r) h = (g - b) / d;
		else if (mx == g) h = 2 + (b - r) / d;
		else h = 4 + (r - g) / d;
		if (h < 0) h += 6;
	}
	hsv->hue = (UInt16) (h / 6.0 * 65536.0 + 0.5);
	hsv->saturation = (UInt16) (mx > 0 ? d / mx * 65535 + 0.5 : 0);
	hsv->value = (UInt16) (mx * 65535 + 0.5);
}

// ---------------------------------------------------------------- pixel patterns

struct QDPixPat { RGBColor color; Boolean solid; };
typedef struct QDPixPat **QDPixPatHandle;

QDPixPatHandle NewPixPat(void)
{
	struct QDPixPat **h = malloc(sizeof(*h));
	if (!h) { gQDError = cNoMemErr; return NULL; }
	*h = calloc(1, sizeof(struct QDPixPat));
	if (!*h) { free(h); gQDError = cNoMemErr; return NULL; }
	return h;
}

void DisposePixPat(QDPixPatHandle pp) { if (pp) { free(*pp); free(pp); } }

// Direct pixels: the pattern is the exact color.
void MakeRGBPat(QDPixPatHandle pp, const RGBColor *c)
{
	if (!pp || !*pp || !c) return;
	(*pp)->color = *c;
	(*pp)->solid = true;
}

void FillCRect(const Rect *r, QDPixPatHandle pp)
{
	struct QDPort *p = current();
	if (!p || !pp || !*pp || !(*pp)->solid) { gQDError = paramErr; return; }
	fill_rect(p, r, &kBlack, kPatCopy, (*pp)->color, p->bkColor);
}

// ---------------------------------------------------------------- globals

QDPattern *GetQDGlobalsBlack(QDPattern *pat) { if (pat) *pat = kBlack; return pat; }
QDPattern *GetQDGlobalsWhite(QDPattern *pat) { if (pat) *pat = kWhite; return pat; }
QDPattern *GetQDGlobalsGray(QDPattern *pat) { if (pat) *pat = kGray; return pat; }
QDPattern *GetQDGlobalsLightGray(QDPattern *pat) { if (pat) *pat = kLtGray; return pat; }
QDPattern *GetQDGlobalsDarkGray(QDPattern *pat) { if (pat) *pat = kDkGray; return pat; }

// The standard arrow cursor ('CURS' 0 data, hot spot at 1,1).
QDCursor *GetQDGlobalsArrow(QDCursor *arrow)
{
	static const QDCursor kArrow = {
		{0x0000, 0x4000, 0x6000, 0x7000, 0x7800, 0x7C00, 0x7E00, 0x7F00,
		 0x7F80, 0x7C00, 0x6C00, 0x4600, 0x0600, 0x0300, 0x0300, 0x0000},
		{(SInt16) 0xC000, (SInt16) 0xE000, (SInt16) 0xF000, (SInt16) 0xF800, (SInt16) 0xFC00, (SInt16) 0xFE00, (SInt16) 0xFF00, (SInt16) 0xFF80,
		 (SInt16) 0xFFC0, (SInt16) 0xFFE0, (SInt16) 0xFE00, (SInt16) 0xEF00, (SInt16) 0xCF00, (SInt16) 0x8780, 0x0780, 0x0380},
		{1, 1}};
	if (arrow) *arrow = kArrow;
	return arrow;
}

// The bounds of the main screen; there is no addressable screen memory.
BitMap *GetQDGlobalsScreenBits(BitMap *bits)
{
	if (!bits) return NULL;
	CGRect r = CGDisplayBounds(CGMainDisplayID());
	memset(bits, 0, sizeof(*bits));
	SetRect(&bits->bounds, 0, 0, (short) r.size.width, (short) r.size.height);
	return bits;
}

// ---------------------------------------------------------------- coordinates

void GlobalToLocal(Point *pt)
{
	struct QDPort *p = current();
	if (!p || !pt) return;
	pt->h -= p->globalOrigin.h - p->portRect.left;
	pt->v -= p->globalOrigin.v - p->portRect.top;
}

void LocalToGlobal(Point *pt)
{
	struct QDPort *p = current();
	if (!p || !pt) return;
	pt->h += p->globalOrigin.h - p->portRect.left;
	pt->v += p->globalOrigin.v - p->portRect.top;
}

// ---------------------------------------------------------------- GWorld state

void GetGWorld(CGrafPtr *port, GDHandle *gdh)
{
	if (port) *port = gCurrentPort;
	if (gdh) *gdh = GetMainDevice();
}

void SetGWorld(CGrafPtr port, GDHandle gdh)
{
	SetPort(port);
}

const BitMap *GetPortBitMapForCopyBits(CGrafPtr port)
{
	struct QDPort *p = PORT_FROM(port);
	if (!p || !QDPortIsLive(p)) { gQDError = paramErr; return NULL; }
	return (const BitMap *) &p->pixMap;
}

// ---------------------------------------------------------------- CopyBits

static Boolean copy_mode_supported(short mode)
{
	switch (mode & ~kDitherCopy) {
		case kSrcCopy: case kSrcOr: case kSrcXor: case kSrcBic: case kNotSrcCopy:
		case kTransparent: case kAddOver:
			return true;
		default:
			return false;
	}
}

static Boolean is_white(RGBColor c) { return c.red == 0xFFFF && c.green == 0xFFFF && c.blue == 0xFFFF; }

// Copies between port pixel maps with nearest-neighbour scaling. Unsupported sources (plain
// BitMaps not owned by a port) or modes leave the destination untouched and set QDError.
void CopyBits(const BitMap *srcBits, const BitMap *dstBits, const Rect *srcRect, const Rect *dstRect,
		short mode, RgnHandle maskRgn)
{
	struct QDPort *s = QDPortFromPixMap(srcBits), *d = QDPortFromPixMap(dstBits);
	if (!s || !d || !srcRect || !dstRect || !copy_mode_supported(mode)) {
		gQDError = paramErr;
		return;
	}
	gQDError = noErr;
	QDRecordCopyBits(s, d, srcRect, dstRect, mode);
	int sw = srcRect->right - srcRect->left, sh = srcRect->bottom - srcRect->top;
	int dw = dstRect->right - dstRect->left, dh = dstRect->bottom - dstRect->top;
	if (sw <= 0 || sh <= 0 || dw <= 0 || dh <= 0) return;
	// The destination clip applies when copying into the current port.
	Boolean useClip = (GrafPtr) d == gCurrentPort;
	short m = mode & ~kDitherCopy;
	uint8_t *tmp = NULL;
	if (s == d) { // overlapping copies read a snapshot of the source
		tmp = malloc(s->length);
		if (!tmp) { gQDError = cNoMemErr; return; }
		memcpy(tmp, s->pixels, s->length);
	}
	for (int y = 0; y < dh; y++)
		for (int x = 0; x < dw; x++) {
			int dx = dstRect->left + x, dy = dstRect->top + y;
			uint8_t *da = pixel_at(d, dx, dy);
			if (!da) continue;
			if (useClip && !clipped_in(d, dx, dy)) continue;
			if (maskRgn) { Point pt = {(short) dy, (short) dx}; if (!PtInRgn(pt, maskRgn)) continue; }
			int sx = srcRect->left + (int) ((int64_t) x * sw / dw), sy = srcRect->top + (int) ((int64_t) y * sh / dh);
			uint8_t *sa = pixel_at(s, sx, sy);
			if (!sa) continue;
			if (tmp) sa = tmp + (sa - (uint8_t *) s->pixels);
			RGBColor c = load(s, sa), o = load(d, da);
			switch (m) {
				case kSrcCopy: break;
				case kNotSrcCopy: c.red = ~c.red; c.green = ~c.green; c.blue = ~c.blue; break;
				case kSrcOr: if (is_white(c)) continue; c = d->fgColor; break;        // black source bits paint
				case kSrcBic: if (is_white(c)) continue; c = d->bkColor; break;
				case kSrcXor: if (is_white(c)) continue; c.red = ~o.red; c.green = ~o.green; c.blue = ~o.blue; break;
				case kTransparent: if (c.red == d->bkColor.red && c.green == d->bkColor.green && c.blue == d->bkColor.blue) continue; break;
				case kAddOver: c.red = (UInt16) (c.red + o.red); c.green = (UInt16) (c.green + o.green); c.blue = (UInt16) (c.blue + o.blue); break;
			}
			store(d, da, c);
		}
	free(tmp);
}

// ---------------------------------------------------------------- GWorld update

enum { kReallocPix = 1L << 0, kClipPix = 1L << 28, kStretchPix = 1L << 29, kGwFlagErr = 1L << 31 };

GWorldFlags UpdateGWorld(GWorldPtr *world, short depth, const Rect *bounds, CTabHandle ctab, GDHandle gd, GWorldFlags flags)
{
	if (!world || !*world || !bounds) { gQDError = paramErr; return kGwFlagErr; }
	struct QDPort *old = PORT_FROM(*world);
	short oldDepth = old->pixMap.pixelFormat == 0x42475241 ? 0 : old->pixMap.pixelSize;
	if (depth == 0 && old->pixMap.pixelSize == 32) depth = oldDepth;
	Rect ob = old->portRect;
	// The world keeps the byte order of its pixels: the replacement is little-endian when the world is, and a
	// world that is not becomes so when the flags ask for it. (Universal Binary Programming Guidelines, "GWorlds".)
	Boolean oldNative = old->pixMap.pixelFormat == k32BGRAPixelFormat || old->pixMap.pixelFormat == k16LE555PixelFormat;
	Boolean native = oldNative || (flags & kNativeEndianPixMap) != 0;
	if (depth == oldDepth && native == oldNative && bounds->right - bounds->left == ob.right - ob.left
			&& bounds->bottom - bounds->top == ob.bottom - ob.top) {
		gQDError = noErr;
		return 0;
	}
	GWorldPtr fresh = NULL;
	if (NewGWorld(&fresh, depth, bounds, ctab, gd, native ? kNativeEndianPixMap : 0) != noErr) return kGwFlagErr;
	if (flags & (kClipPix | kStretchPix)) {
		Rect src = old->portRect, dst = PORT_FROM(fresh)->portRect;
		if (flags & kClipPix) {
			int w = src.right - src.left < dst.right - dst.left ? src.right - src.left : dst.right - dst.left;
			int h = src.bottom - src.top < dst.bottom - dst.top ? src.bottom - src.top : dst.bottom - dst.top;
			src.right = src.left + w; src.bottom = src.top + h;
			dst.right = dst.left + w; dst.bottom = dst.top + h;
		}
		GrafPtr save = gCurrentPort;
		gCurrentPort = NULL; // no clip: the copy is not drawing into a port
		CopyBits((const BitMap *) &old->pixMap, (const BitMap *) &PORT_FROM(fresh)->pixMap, &src, &dst, kSrcCopy, NULL);
		gCurrentPort = save;
	}
	if (gCurrentPort == (GrafPtr) old) gCurrentPort = (GrafPtr) fresh;
	DisposeGWorld(*world);
	*world = fresh;
	gQDError = noErr;
	return kReallocPix;
}

#include "QDRecord.inc"

void QDRecordCopyBits(struct QDPort *s, struct QDPort *d, const Rect *src, const Rect *dst, short mode)
{
	if (recording(d) && s && src && dst) rec_copybits(s, src, dst, mode);
}

#endif /* !__LP64__ */

