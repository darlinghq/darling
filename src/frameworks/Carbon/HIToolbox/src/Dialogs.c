/*
 This file is part of Darling.

 Copyright (C) 2026 fr4nck

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

// The Dialog Manager (Macintosh Toolbox Essentials, chapter 6). A dialog is a window of the Window Manager made from
// a 'DLOG' resource, or an 'ALRT' resource for an alert, with the items of a 'DITL' resource. The dialog's
// reference is its window's reference, as on Mac OS X.
//
// The buttons, check boxes, radio buttons and the controls of 'CNTL' resources are controls of the Control Manager
// in the window: they draw themselves and are tracked by the window's standard handling of clicks. The Dialog Manager
// draws the other items into the window's QuickDraw port: static and edit text (word wrapped, with the dialog font),
// the frame of edit text, the outline of the default button, 'PICT' pictures, 'ICON' icons, and user items through
// the procedure SetDialogItem() gave them.
//
// ModalDialog() runs the Carbon event loop until an enabled item is hit; the events go to their targets as usual,
// except clicks outside the dialog. Return and Enter hit the default item, Escape and Command-period the cancel
// item; other keys edit the edit text item that has the focus (characters and backspace; there is no selection).
//
// Not available: the color tables of 'dctb' and 'actb' resources (dialogs are drawn with the theme's colors), the
// alert icons and sounds, and the later stages of an alert (each alert is at stage 1).

#include <HIToolbox/Dialogs.h>
#include <HIToolbox/CarbonEvents.h>
#include <HIToolbox/Controls.h>
#include <HIToolbox/MacWindows.h>
#include <CarbonCore/MacErrors.h>
#include <CarbonCore/MacMemory.h>
#include <CarbonCore/Resources.h>
#include <AE/AEDataModel.h>
#include <CoreText/CoreText.h>
#include <stdlib.h>
#include <string.h>

ModalFilterUPP NewModalFilterUPP(ModalFilterProcPtr userRoutine)
{
	return userRoutine;
}

void DisposeModalFilterUPP(ModalFilterUPP userUPP)
{
}

Boolean InvokeModalFilterUPP(DialogRef theDialog, EventRecord* theEvent, DialogItemIndex* itemHit, ModalFilterUPP userUPP)
{
	return userUPP ? userUPP(theDialog, theEvent, itemHit) : false;
}

UserItemUPP NewUserItemUPP(UserItemProcPtr userRoutine)
{
	return userRoutine;
}

void DisposeUserItemUPP(UserItemUPP userUPP)
{
}

void InvokeUserItemUPP(DialogRef theDialog, DialogItemIndex itemNo, UserItemUPP userUPP)
{
	if (userUPP)
		userUPP(theDialog, itemNo);
}

#ifndef __LP64__
// QuickDraw calls that the public QuickDraw header of this tree does not declare
void GetPort(GrafPtr* port);
void SetPort(GrafPtr port);
RgnHandle NewRgn(void);
void DisposeRgn(RgnHandle region);
void ClipRect(const Rect* rect);
void GetClip(RgnHandle region);
void SetClip(RgnHandle region);
void EraseRect(const Rect* rect);

enum
{
	kHiddenOffset = 16384,	// HideDialogItem() moves an item this far to the right
	kEditInset = 3,			// the frame of edit text is outside the item's rectangle
	kTextSize = 12,
};

typedef struct DialogItem
{
	DialogItemType type;	// with itemDisable
	Rect bounds;
	Handle text;			// static and edit text
	ControlRef control;		// buttons, check boxes, radio buttons and 'CNTL' controls
	SInt16 resourceID;		// pictures, icons and 'CNTL' controls
	UserItemUPP proc;		// user items
} DialogItem;

struct OpaqueDialogPtr;

typedef struct Dialog
{
	WindowRef window;
	DialogItemIndex count;
	DialogItem* items;
	DialogItemIndex defaultItem, cancelItem;
	DialogItemIndexZeroBased focus;	// the edit text item that gets the keys, or -1
	DialogItemIndex hit;			// the item ModalDialog() returns, 0 while none
	int modal;						// ModalDialog() is running for this dialog
	SInt16 font;
	EventHandlerRef handler;
	struct Dialog* next;
} Dialog;

static Dialog* g_dialogs;
static SInt16 g_dialogFont;
static unsigned char g_paramText[4][256];

static unsigned be16(const unsigned char* p)
{
	return (unsigned) p[0] << 8 | p[1];
}

static Rect rectAt(const unsigned char* p)
{
	Rect r = { (SInt16) be16(p), (SInt16) be16(p + 2), (SInt16) be16(p + 4), (SInt16) be16(p + 6) };

	return r;
}

static Dialog* dialogOf(DialogRef ref)
{
	for (Dialog* d = g_dialogs; d; d = d->next)
	{
		if ((DialogRef) d->window == ref)
			return d;
	}
	return NULL;
}

static DialogRef refOf(Dialog* d)
{
	return (DialogRef) d->window;
}

static DialogItem* itemOf(Dialog* d, DialogItemIndex itemNo)
{
	return d && itemNo >= 1 && itemNo <= d->count ? &d->items[itemNo - 1] : NULL;
}

// The item whose text or control is the handle GetDialogItem() returned
static DialogItem* itemOfHandle(Handle h, Dialog** owner)
{
	for (Dialog* d = g_dialogs; d && h; d = d->next)
	{
		for (int i = 0; i < d->count; i++)
		{
			if (d->items[i].text == h || (Handle) d->items[i].control == h)
			{
				if (owner)
					*owner = d;
				return &d->items[i];
			}
		}
	}
	return NULL;
}

static unsigned kindOf(const DialogItem* item)
{
	return item->type & ~itemDisable;
}

static bool isHidden(const DialogItem* item)
{
	return item->bounds.left >= kHiddenOffset / 2;
}

static bool isTextItem(const DialogItem* item)
{
	return kindOf(item) == statText || kindOf(item) == editText;
}

// A copy of a resource's data; the resource itself stays loaded for others that use it
static unsigned char* copyResource(ResType type, SInt16 id, Size* length)
{
	Handle h = GetResource(type, id);

	*length = 0;
	if (!h || !*h)
		return NULL;

	Size n = GetHandleSize(h);
	unsigned char* copy = malloc(n ? n : 1);

	if (!copy)
		return NULL;
	memcpy(copy, *h, n);
	*length = n;
	return copy;
}

#pragma mark Drawing

static CTFontRef copyDialogFont(SInt16 font)
{
	// The fonts of the classic font numbers; 0 and 1 are the system and application fonts
	static const struct { SInt16 number; CFStringRef name; } kFonts[] = {
		{ 2, CFSTR("New York") }, { 3, CFSTR("Geneva") }, { 4, CFSTR("Monaco") }, { 20, CFSTR("Times") },
		{ 21, CFSTR("Helvetica") }, { 22, CFSTR("Courier") },
	};
	CTFontRef result = NULL;

	for (size_t i = 0; i < sizeof(kFonts) / sizeof(kFonts[0]) && !result; i++)
	{
		if (kFonts[i].number == font)
			result = CTFontCreateWithName(kFonts[i].name, kTextSize, NULL);
	}
	if (!result)
		result = CTFontCreateUIFontForLanguage(font == 1 ? kCTFontUIFontApplication : kCTFontUIFontSystem, kTextSize, NULL);
	if (!result)
		result = CTFontCreateWithName(CFSTR("Helvetica"), kTextSize, NULL);
	return result;
}

static CTLineRef createLine(CTFontRef font, const unsigned char* bytes, CFIndex length)
{
	CFStringRef string = CFStringCreateWithBytes(NULL, bytes, length, kCFStringEncodingMacRoman, false);

	if (!string)
		return NULL;

	CGColorSpaceRef space = CGColorSpaceCreateDeviceGray();
	CGFloat comps[2] = { 0, 1 };
	CGColorRef color = CGColorCreate(space, comps);
	CFStringRef keys[2] = { kCTFontAttributeName, kCTForegroundColorAttributeName };
	CFTypeRef values[2] = { font, color };
	CFDictionaryRef attrs = CFDictionaryCreate(NULL, (const void**) keys, (const void**) values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFAttributedStringRef attributed = CFAttributedStringCreate(NULL, string, attrs);
	CTLineRef line = CTLineCreateWithAttributedString(attributed);

	CFRelease(attributed);
	CFRelease(attrs);
	CGColorRelease(color);
	CGColorSpaceRelease(space);
	CFRelease(string);
	return line;
}

static double lineWidth(CTFontRef font, const unsigned char* bytes, CFIndex length)
{
	CTLineRef line = createLine(font, bytes, length);
	double width = line ? CTLineGetTypographicBounds(line, NULL, NULL, NULL) : 0;

	if (line)
		CFRelease(line);
	return width;
}

// Draws text from the top left of the rectangle, wrapped at spaces and at returns. Returns the end of the text.
static CGPoint drawWrapped(CGContextRef ctx, CTFontRef font, const Rect* r, const unsigned char* s)
{
	CGFloat ascent = CTFontGetAscent(font), lineHeight = ascent + CTFontGetDescent(font) + CTFontGetLeading(font);
	CGFloat width = r->right - r->left, y = r->top + ascent;
	CGPoint end = CGPointMake(r->left, r->top);
	unsigned start = 1, length = s[0];

	if (lineHeight < 1)
		lineHeight = kTextSize + 3;
	while (start <= length)
	{
		unsigned stop = start, lastSpace = 0;

		// The longest run that fits, broken after a space when it does not fit
		while (stop <= length && s[stop] != '\r')
		{
			if (stop > start && lineWidth(font, s + start, stop - start + 1) > width)
			{
				if (lastSpace)
					stop = lastSpace + 1;
				break;
			}
			if (s[stop] == ' ')
				lastSpace = stop;
			stop++;
		}

		CTLineRef line = createLine(font, s + start, stop - start);

		end = CGPointMake(r->left, y - ascent);
		if (line)
		{
			CGContextSetTextMatrix(ctx, CGAffineTransformMakeScale(1, -1));
			CGContextSetTextPosition(ctx, r->left, y);
			CTLineDraw(line, ctx);
			end.x += CTLineGetTypographicBounds(line, NULL, NULL, NULL);
			CFRelease(line);
		}

		bool newline = stop <= length && s[stop] == '\r';
		unsigned next = newline ? stop + 1 : stop;

		if (newline || next <= length)
		{
			y += lineHeight;
			if (next > length)
				end = CGPointMake(r->left, y - ascent);
		}
		start = next;
	}
	return end;
}

// ParamText's ^0 to ^3 replaced in static text
static void substitute(const unsigned char* in, unsigned char* out)
{
	unsigned n = 0;

	for (unsigned i = 1; i <= in[0] && n < 255; i++)
	{
		if (in[i] == '^' && i < in[0] && in[i + 1] >= '0' && in[i + 1] <= '3')
		{
			const unsigned char* p = g_paramText[in[i + 1] - '0'];

			for (unsigned k = 1; k <= p[0] && n < 255; k++)
				out[++n] = p[k];
			i++;
		}
		else
			out[++n] = in[i];
	}
	out[0] = (unsigned char) n;
}

static void itemText(const DialogItem* item, unsigned char* out)
{
	Size n = item->text && *item->text ? GetHandleSize(item->text) : 0;

	if (n > 255)
		n = 255;
	out[0] = (unsigned char) n;
	if (n)
		memcpy(out + 1, *item->text, n);
}

// A QuickDraw context of the port in QuickDraw's coordinates (y down from the top left of portRect)
static CGContextRef beginContext(CGrafPtr port)
{
	CGContextRef ctx = NULL;
	Rect bounds;

	if (QDBeginCGContext(port, &ctx) != noErr || !ctx)
		return NULL;
	GetPortBounds(port, &bounds);
	CGContextSaveGState(ctx);
	CGContextTranslateCTM(ctx, -bounds.left, bounds.bottom);
	CGContextScaleCTM(ctx, 1, -1);
	return ctx;
}

static void endContext(CGrafPtr port, CGContextRef ctx)
{
	CGContextRestoreGState(ctx);
	QDEndCGContext(port, &ctx);
}

static CGRect cgRect(const Rect* r)
{
	return CGRectMake(r->left, r->top, r->right - r->left, r->bottom - r->top);
}

// An 'ICON' resource: 32 by 32 pixels, one bit each, scaled to the rectangle
static void drawIcon(CGContextRef ctx, const DialogItem* item)
{
	Handle icon = GetResource('ICON', item->resourceID);

	if (!icon || !*icon || GetHandleSize(icon) < 128)
		return;

	const unsigned char* bits = (const unsigned char*) *icon;
	CGFloat w = (item->bounds.right - item->bounds.left) / 32.0, h = (item->bounds.bottom - item->bounds.top) / 32.0;

	CGContextSetRGBFillColor(ctx, 0, 0, 0, 1);
	for (int y = 0; y < 32; y++)
	{
		for (int x = 0; x < 32; x++)
		{
			if (bits[y * 4 + x / 8] & (0x80 >> (x % 8)))
				CGContextFillRect(ctx, CGRectMake(item->bounds.left + x * w, item->bounds.top + y * h, w, h));
		}
	}
}

// The items the Dialog Manager draws (not the controls), into the dialog's port; only: one item, or -1 for all
static void drawItems(Dialog* d, int only)
{
	CGrafPtr port = GetWindowPort(d->window);
	GrafPtr saved = NULL;

	if (!port)
		return;
	GetPort(&saved);
	SetPort((GrafPtr) port);

	// What QuickDraw draws: pictures and user items, each clipped to its rectangle
	RgnHandle clip = NewRgn();

	GetClip(clip);
	for (int i = 0; i < d->count; i++)
	{
		DialogItem* item = &d->items[i];

		if ((only >= 0 && i != only) || isHidden(item))
			continue;
		if (kindOf(item) == picItem)
		{
			Handle picture = (Handle) GetPicture(item->resourceID);

			ClipRect(&item->bounds);
			if (picture)
				DrawPicture((PicHandle) picture, &item->bounds);
			SetClip(clip);
		}
		else if (kindOf(item) == userItem && item->proc)
			item->proc(refOf(d), (DialogItemIndex) (i + 1));
	}
	DisposeRgn(clip);

	// What Core Text and Core Graphics draw: text, frames and icons
	CGContextRef ctx = beginContext(port);

	if (ctx)
	{
		CTFontRef font = copyDialogFont(d->font);

		for (int i = 0; i < d->count; i++)
		{
			DialogItem* item = &d->items[i];
			unsigned char text[256], shown[256];

			if ((only >= 0 && i != only) || isHidden(item))
				continue;
			if (isTextItem(item))
			{
				itemText(item, text);
				if (kindOf(item) == statText)
					substitute(text, shown);
				else
					memcpy(shown, text, text[0] + 1);
				CGContextSaveGState(ctx);
				CGContextClipToRect(ctx, cgRect(&item->bounds));
				CGPoint end = font ? drawWrapped(ctx, font, &item->bounds, shown) : CGPointMake(item->bounds.left, item->bounds.top);
				CGContextRestoreGState(ctx);
				if (kindOf(item) == editText)
				{
					CGContextSetRGBStrokeColor(ctx, 0, 0, 0, 1);
					CGContextSetLineWidth(ctx, 1);
					CGContextStrokeRect(ctx, CGRectInset(cgRect(&item->bounds), -kEditInset + 0.5, -kEditInset + 0.5));
					// The insertion point, at the end of the text
					if (d->focus == i)
					{
						CGFloat height = font ? CTFontGetAscent(font) + CTFontGetDescent(font) : kTextSize;

						CGContextFillRect(ctx, CGRectMake(end.x, end.y, 1, height));
					}
				}
			}
			else if (kindOf(item) == iconItem)
				drawIcon(ctx, item);
			else if (i + 1 == d->defaultItem && item->control && kindOf(item) == kButtonDialogItem)
			{
				// The outline of the default button
				CGContextSetRGBStrokeColor(ctx, 0, 0, 0, 1);
				CGContextSetLineWidth(ctx, 3);
				CGContextStrokeRect(ctx, CGRectInset(cgRect(&item->bounds), -2.5, -2.5));
			}
		}
		if (font)
			CFRelease(font);
		endContext(port, ctx);
	}
	SetPort(saved);
}

// Draws an item again now: its area is erased, then drawn
static void redrawItem(Dialog* d, int index)
{
	DialogItem* item = &d->items[index];
	CGrafPtr port = GetWindowPort(d->window);
	GrafPtr saved = NULL;

	if (!port || !IsWindowVisible(d->window) || isHidden(item))
		return;
	if (item->control)
	{
		Draw1Control(item->control);
		return;
	}

	Rect area = item->bounds;

	if (kindOf(item) == editText)
		InsetRect(&area, -kEditInset, -kEditInset);
	GetPort(&saved);
	SetPort((GrafPtr) port);
	EraseRect(&area);
	SetPort(saved);
	drawItems(d, index);
	QDFlushPortBuffer(port, NULL);
}

void DrawDialog(DialogRef theDialog)
{
	Dialog* d = dialogOf(theDialog);

	if (!d)
		return;
	drawItems(d, -1);
	DrawControls(d->window);
	QDFlushPortBuffer(GetWindowPort(d->window), NULL);
}

#pragma mark Events

// The first item that contains the point (local coordinates), from 0, or -1
DialogItemIndexZeroBased FindDialogItem(DialogRef theDialog, Point thePt)
{
	Dialog* d = dialogOf(theDialog);

	for (int i = 0; d && i < d->count; i++)
	{
		const Rect* r = &d->items[i].bounds;

		if (!isHidden(&d->items[i]) && thePt.h >= r->left && thePt.h < r->right && thePt.v >= r->top && thePt.v < r->bottom)
			return (DialogItemIndexZeroBased) i;
	}
	return -1;
}

static bool isEnabled(Dialog* d, DialogItemIndex itemNo)
{
	DialogItem* item = itemOf(d, itemNo);

	return item && !(item->type & itemDisable) && !isHidden(item);
}

static void setFocus(Dialog* d, int index)
{
	int old = d->focus;

	if (old == index)
		return;
	d->focus = index;
	if (old >= 0)
		redrawItem(d, old);
	if (index >= 0)
		redrawItem(d, index);
}

// A click on an item that is not a control (local coordinates). Returns the item hit, or 0.
static DialogItemIndex clickItem(Dialog* d, Point local)
{
	int index = FindDialogItem(refOf(d), local);

	if (index < 0 || d->items[index].control)
		return 0;
	if (kindOf(&d->items[index]) == editText)
		setFocus(d, index);
	return isEnabled(d, (DialogItemIndex) (index + 1)) ? (DialogItemIndex) (index + 1) : 0;
}

// A key typed in the dialog. Returns the item it hits, 0 for none, or -1 when the dialog does not take the key.
// Return and Enter (the default item), Escape and Command-period (the cancel item) only in a modal dialog.
static DialogItemIndex keyItem(Dialog* d, unsigned char ch, UInt32 modifiers, bool modal)
{
	if (modal)
	{
		if ((ch == '\r' || ch == 3) && d->defaultItem)
			return isEnabled(d, d->defaultItem) ? d->defaultItem : 0;
		if ((ch == 27 || (ch == '.' && (modifiers & cmdKey))) && d->cancelItem)
			return isEnabled(d, d->cancelItem) ? d->cancelItem : 0;
	}
	if (modifiers & cmdKey)
		return -1;

	// Tab gives the focus to the next edit text item
	if (ch == '\t')
	{
		for (int k = 1; k <= d->count; k++)
		{
			int i = (d->focus + k) % d->count;

			if (kindOf(&d->items[i]) == editText && !isHidden(&d->items[i]))
			{
				setFocus(d, i);
				break;
			}
		}
		return 0;
	}
	if (d->focus < 0)
		return -1;

	DialogItem* item = &d->items[d->focus];
	Size n = GetHandleSize(item->text);

	if (ch == 8)
	{
		if (n > 0)
			SetHandleSize(item->text, n - 1);
	}
	else if (ch >= 32 && ch != 127 && n < 255)
	{
		SetHandleSize(item->text, n + 1);
		if (GetHandleSize(item->text) != n + 1)
			return -1;
		(*item->text)[n] = (char) ch;
	}
	else
		return -1;
	redrawItem(d, d->focus);
	return isEnabled(d, (DialogItemIndex) (d->focus + 1)) ? (DialogItemIndex) (d->focus + 1) : 0;
}

static Point localPoint(Dialog* d, Point global)
{
	Rect content;

	GetWindowBounds(d->window, kWindowContentRgn, &content);
	global.h -= content.left;
	global.v -= content.top;
	return global;
}

static bool containsGlobal(WindowRef window, Point global, WindowRegionCode region)
{
	Rect r;

	return GetWindowBounds(window, region, &r) == noErr && global.h >= r.left && global.h < r.right && global.v >= r.top && global.v < r.bottom;
}

// The handler of the dialog's window: what the Dialog Manager draws, item hits, and keys while ModalDialog() runs
static OSStatus dialogHandler(EventHandlerCallRef call, EventRef event, void* userData)
{
	Dialog* d = userData;
	UInt32 cls = GetEventClass(event), kind = GetEventKind(event);

	if (cls == kEventClassWindow && kind == kEventWindowDrawContent)
	{
		// The controls are drawn by the window's standard handling, after this
		drawItems(d, -1);
		return eventNotHandledErr;
	}
	if (cls == kEventClassControl && kind == kEventControlHit)
	{
		ControlRef control = NULL;
		ControlPartCode part = kControlNoPart;

		GetEventParameter(event, kEventParamDirectObject, typeControlRef, NULL, sizeof(control), NULL, &control);
		GetEventParameter(event, kEventParamControlPart, typeControlPartCode, NULL, sizeof(part), NULL, &part);
		for (int i = 0; i < d->count; i++)
		{
			if (d->items[i].control == control && part != kControlNoPart && isEnabled(d, (DialogItemIndex) (i + 1)))
				d->hit = (DialogItemIndex) (i + 1);
		}
		return eventNotHandledErr;
	}
	if (!d->modal)
		return eventNotHandledErr;
	if (cls == kEventClassWindow && kind == kEventWindowClickContentRgn)
	{
		Point global;

		if (GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(global), NULL, &global) == noErr)
		{
			DialogItemIndex hit = clickItem(d, localPoint(d, global));

			if (hit)
				d->hit = hit;
		}
		return eventNotHandledErr;
	}
	if (cls == kEventClassKeyboard && (kind == kEventRawKeyDown || kind == kEventRawKeyRepeat))
	{
		char ch = 0;
		UInt32 modifiers = 0;

		GetEventParameter(event, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof(ch), NULL, &ch);
		GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(modifiers), NULL, &modifiers);

		DialogItemIndex hit = keyItem(d, (unsigned char) ch, modifiers, true);

		if (hit < 0)
			return eventNotHandledErr;
		if (hit)
			d->hit = hit;
		return noErr;
	}
	return eventNotHandledErr;
}

// The classic event of a Carbon event, for a modal filter procedure
static bool eventRecordOf(EventRef event, EventRecord* r)
{
	UInt32 cls = GetEventClass(event), kind = GetEventKind(event), modifiers = 0;

	memset(r, 0, sizeof(*r));
	r->when = TickCount();
	GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(modifiers), NULL, &modifiers);
	r->modifiers = (EventModifiers) modifiers;
	if (cls == kEventClassMouse && (kind == kEventMouseDown || kind == kEventMouseUp))
	{
		r->what = kind == kEventMouseDown ? mouseDown : mouseUp;
		GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(r->where), NULL, &r->where);
		return true;
	}
	if (cls == kEventClassKeyboard && (kind == kEventRawKeyDown || kind == kEventRawKeyRepeat))
	{
		char ch = 0;
		UInt32 code = 0;

		GetEventParameter(event, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof(ch), NULL, &ch);
		GetEventParameter(event, kEventParamKeyCode, typeUInt32, NULL, sizeof(code), NULL, &code);
		r->what = kind == kEventRawKeyDown ? keyDown : autoKey;
		r->message = (unsigned char) ch | (code & 0xFF) << 8;
		return true;
	}
	if (cls == kEventClassWindow && (kind == kEventWindowUpdate || kind == kEventWindowActivated || kind == kEventWindowDeactivated))
	{
		WindowRef window = NULL;

		GetEventParameter(event, kEventParamDirectObject, typeWindowRef, NULL, sizeof(window), NULL, &window);
		r->what = kind == kEventWindowUpdate ? updateEvt : activateEvt;
		r->message = (unsigned long) window;
		if (kind == kEventWindowActivated)
			r->modifiers |= activeFlag;
		return true;
	}
	return false;
}

// Runs the dialog until an enabled item is hit; the filter procedure sees each event first, and null events
static DialogItemIndex runModal(Dialog* d, ModalFilterUPP filter)
{
	DialogItemIndex hit = 0;
	WindowRef window = d->window;

	d->hit = 0;
	d->modal++;
	while (!hit)
	{
		EventRef event = NULL;
		OSStatus status = ReceiveNextEvent(0, NULL, kEventDurationSecond / 10, true, &event);
		EventRecord record;

		if (status == eventLoopQuitErr)
			break;
		if (status != noErr && status != eventLoopTimedOutErr)
			break;
		if (filter)
		{
			bool classic = event ? eventRecordOf(event, &record) : false;

			if (!classic)
			{
				memset(&record, 0, sizeof(record));
				record.what = nullEvent;
				record.when = TickCount();
			}
			if (!event || classic)
			{
				DialogItemIndex filtered = 0;

				if (filter(refOf(d), &record, &filtered))
				{
					hit = filtered;
					if (event)
						ReleaseEvent(event);
					break;
				}
			}
		}
		if (event)
		{
			// The other windows do not take clicks while the dialog is modal
			bool outside = false;

			if (GetEventClass(event) == kEventClassMouse && GetEventKind(event) == kEventMouseDown)
			{
				Point global;

				if (GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(global), NULL, &global) == noErr)
					outside = !containsGlobal(window, global, kWindowStructureRgn);
			}
			if (!outside)
				SendEventToEventTarget(event, GetEventDispatcherTarget());
			ReleaseEvent(event);
		}
		// The dialog may have been disposed of by a handler or the filter procedure
		if (dialogOf((DialogRef) window) != d)
			return hit;
		if (d->hit)
			hit = d->hit;
	}
	d->modal--;
	d->hit = 0;
	return hit;
}

void ModalDialog(ModalFilterUPP modalFilter, DialogItemIndex* itemHit)
{
	Dialog* d = dialogOf((DialogRef) FrontWindow());

	// The front dialog; else the newest visible one
	for (Dialog* p = g_dialogs; p && !d; p = p->next)
	{
		if (IsWindowVisible(p->window))
			d = p;
	}
	if (itemHit)
		*itemHit = 0;
	if (!d)
		return;

	DialogItemIndex hit = runModal(d, modalFilter);

	if (itemHit)
		*itemHit = hit;
}

static Dialog* dialogAt(Point global)
{
	// The frontmost visible window that contains the point
	for (WindowRef w = FrontWindow(); w; w = GetNextWindow(w))
	{
		if (IsWindowVisible(w) && containsGlobal(w, global, kWindowStructureRgn))
			return containsGlobal(w, global, kWindowContentRgn) ? dialogOf((DialogRef) w) : NULL;
	}
	return NULL;
}

static Dialog* dialogOfEvent(const EventRecord* e)
{
	switch (e->what)
	{
		case mouseDown:
			return dialogAt(e->where);
		case updateEvt:
		case activateEvt:
			return dialogOf((DialogRef) e->message);
		case keyDown:
		case autoKey:
		case nullEvent:
			return dialogOf((DialogRef) FrontWindow());
	}
	return NULL;
}

Boolean IsDialogEvent(const EventRecord* theEvent)
{
	return theEvent && dialogOfEvent(theEvent) != NULL;
}

Boolean DialogSelect(const EventRecord* theEvent, DialogRef* theDialog, DialogItemIndex* itemHit)
{
	Dialog* d = theEvent ? dialogOfEvent(theEvent) : NULL;

	if (!d)
		return false;
	if (theDialog)
		*theDialog = refOf(d);

	DialogItemIndex hit = 0;

	switch (theEvent->what)
	{
		case updateEvt:
			BeginUpdate(d->window);
			DrawDialog(refOf(d));
			EndUpdate(d->window);
			return false;
		case mouseDown:
		{
			Point local = localPoint(d, theEvent->where);
			int index = FindDialogItem(refOf(d), local);

			if (index < 0)
				return false;
			if (d->items[index].control)
			{
				// The control tracks the mouse; a part released in it is a hit (dialogHandler)
				d->hit = 0;
				HandleControlClick(d->items[index].control, local, theEvent->modifiers, NULL);
				hit = d->hit;
				d->hit = 0;
			}
			else
				hit = clickItem(d, local);
			break;
		}
		case keyDown:
		case autoKey:
			hit = keyItem(d, (unsigned char) (theEvent->message & charCodeMask), theEvent->modifiers, false);
			if (hit < 0)
				hit = 0;
			break;
		default:
			return false;
	}
	if (itemHit && hit)
		*itemHit = hit;
	return hit != 0;
}

#pragma mark Making dialogs

static void disposeItems(Dialog* d)
{
	for (int i = 0; i < d->count; i++)
	{
		if (d->items[i].text)
			DisposeHandle(d->items[i].text);
	}
	free(d->items);
	d->items = NULL;
	d->count = 0;
}

static void discard(Dialog* d)
{
	if (d->handler)
		RemoveEventHandler(d->handler);
	if (d->window && IsValidWindowPtr(d->window))
		DisposeWindow(d->window);
	disposeItems(d);
	free(d);
}

// Reads the items of a 'DITL' resource and makes the controls of the control items
static OSStatus makeItems(Dialog* d, const unsigned char* raw, Size size)
{
	if (size < 2)
		return paramErr;

	int count = (SInt16) be16(raw) + 1;
	Size p = 2;

	if (count < 1 || count > 1024)
		return paramErr;
	d->items = calloc(count, sizeof(DialogItem));
	if (!d->items)
		return memFullErr;
	d->count = (DialogItemIndex) count;
	for (int i = 0; i < count; i++)
	{
		DialogItem* item = &d->items[i];

		if (size - p < 14)
			return paramErr;

		unsigned n = raw[p + 13];
		const unsigned char* data = raw + p + 14;

		if (n > (unsigned) (size - p - 14))
			return paramErr;
		item->bounds = rectAt(raw + p + 4);
		item->type = raw[p + 12];
		if ((kindOf(item) == picItem || kindOf(item) == iconItem || kindOf(item) == kResourceControlDialogItem) && n >= 2)
			item->resourceID = (SInt16) be16(data);

		CFStringRef title = NULL;
		OSStatus status = noErr;

		switch (kindOf(item))
		{
			case kButtonDialogItem:
			case kCheckBoxDialogItem:
			case kRadioButtonDialogItem:
				title = CFStringCreateWithBytes(NULL, data, n, kCFStringEncodingMacRoman, false);
				if (kindOf(item) == kButtonDialogItem)
					status = CreatePushButtonControl(d->window, &item->bounds, title, &item->control);
				else if (kindOf(item) == kCheckBoxDialogItem)
					status = CreateCheckBoxControl(d->window, &item->bounds, title, 0, false, &item->control);
				else
					status = CreateRadioButtonControl(d->window, &item->bounds, title, 0, false, &item->control);
				if (title)
					CFRelease(title);
				break;
			case kResourceControlDialogItem:
				item->control = GetNewControl(item->resourceID, d->window);
				if (!item->control)
					status = resNotFound;
				break;
			case statText:
			case editText:
				item->text = NewHandle(n);
				if (!item->text)
					return memFullErr;
				memcpy(*item->text, data, n);
				break;
		}
		if (status != noErr)
			return status;
		p += 14 + n;
		if (p & 1)
			p++;
	}
	return noErr;
}

static OSStatus windowClassOf(SInt16 procID, bool closeBox, WindowClass* cls, WindowAttributes* attributes)
{
	*attributes = closeBox ? kWindowCloseBoxAttribute : 0;
	switch (procID)
	{
		case 0: // documentProc
		case 8: // zoomDocProc
			*cls = kDocumentWindowClass;
			*attributes |= kWindowResizableAttribute | kWindowCollapseBoxAttribute | (procID == 8 ? kWindowFullZoomAttribute : 0);
			return noErr;
		case 4: // noGrowDocProc
		case 12: // zoomNoGrow
			*cls = kDocumentWindowClass;
			*attributes |= kWindowCollapseBoxAttribute | (procID == 12 ? kWindowFullZoomAttribute : 0);
			return noErr;
		case 1: // dBoxProc
			*cls = kModalWindowClass;
			*attributes = 0;
			return noErr;
		case 2: // plainDBox
		case 3: // altDBoxProc
			*cls = kPlainWindowClass;
			*attributes = 0;
			return noErr;
		case 5: // movableDBoxProc
			*cls = kMovableModalWindowClass;
			*attributes = 0;
			return noErr;
	}
	return paramErr;
}

// The position word that ends a 'DLOG' or 'ALRT' resource
static void positionWindow(WindowRef window, WindowRef parent, UInt16 position)
{
	static const struct { UInt16 position; WindowPositionMethod method; bool needsParent; } kPositions[] = {
		{ 0x280A, kWindowCenterOnMainScreen, false }, { 0x300A, kWindowAlertPositionOnMainScreen, false },
		{ 0x380A, kWindowCascadeOnMainScreen, false }, { 0xA80A, kWindowCenterOnParentWindow, true },
		{ 0xB00A, kWindowAlertPositionOnParentWindow, true }, { 0xB80A, kWindowCascadeOnParentWindow, true },
		{ 0x680A, kWindowCenterOnParentWindowScreen, true }, { 0x700A, kWindowAlertPositionOnParentWindowScreen, true },
		{ 0x780A, kWindowCascadeOnParentWindowScreen, true },
	};

	for (size_t i = 0; i < sizeof(kPositions) / sizeof(kPositions[0]); i++)
	{
		if (kPositions[i].position != position)
			continue;
		if (kPositions[i].needsParent && !parent)
			RepositionWindow(window, NULL, kPositions[i].method == kWindowAlertPositionOnParentWindow || kPositions[i].method == kWindowAlertPositionOnParentWindowScreen
				? kWindowAlertPositionOnMainScreen : kWindowCenterOnMainScreen);
		else
			RepositionWindow(window, parent, kPositions[i].method);
		return;
	}
}

static Dialog* makeDialog(const Rect* bounds, WindowClass cls, WindowAttributes attributes, const unsigned char* title, SInt16 itemsID,
	UInt16 position, bool visible, WindowRef behind, SRefCon refCon)
{
	Size size = 0;
	unsigned char* items = copyResource('DITL', itemsID, &size);
	WindowRef parent = FrontNonFloatingWindow();
	Dialog* d = items ? calloc(1, sizeof(Dialog)) : NULL;
	ControlRef root = NULL;

	if (!d)
	{
		free(items);
		return NULL;
	}
	d->focus = -1;
	d->defaultItem = ok;
	d->font = g_dialogFont;
	if (CreateNewWindow(cls, attributes, bounds, &d->window) != noErr || !d->window ||
		CreateRootControl(d->window, &root) != noErr || makeItems(d, items, size) != noErr)
	{
		free(items);
		discard(d);
		return NULL;
	}
	free(items);
	if (title && title[0])
	{
		CFStringRef string = CFStringCreateWithPascalString(NULL, title, kCFStringEncodingMacRoman);

		if (string)
		{
			SetWindowTitleWithCFString(d->window, string);
			CFRelease(string);
		}
	}
	SetWRefCon(d->window, refCon);

	// The first edit text item has the focus
	for (int i = 0; i < d->count && d->focus < 0; i++)
	{
		if (kindOf(&d->items[i]) == editText)
			d->focus = i;
	}

	static const EventTypeSpec kEvents[] = {
		{ kEventClassWindow, kEventWindowDrawContent }, { kEventClassWindow, kEventWindowClickContentRgn },
		{ kEventClassControl, kEventControlHit }, { kEventClassKeyboard, kEventRawKeyDown }, { kEventClassKeyboard, kEventRawKeyRepeat },
	};

	if (InstallEventHandler(GetWindowEventTarget(d->window), NewEventHandlerUPP(dialogHandler), sizeof(kEvents) / sizeof(kEvents[0]), kEvents, d, &d->handler) != noErr)
	{
		discard(d);
		return NULL;
	}
	positionWindow(d->window, parent, position);
	if (behind != (WindowRef) -1)
		SendBehind(d->window, behind);
	d->next = g_dialogs;
	g_dialogs = d;
	if (visible)
	{
		ShowWindow(d->window);
		if (behind == (WindowRef) -1)
			SelectWindow(d->window);
	}
	return d;
}

// A dialog from a 'DLOG' resource: bounds, procID, visibility, close box, reference constant, items, title and
// position (Macintosh Toolbox Essentials, 6-153). dStorage is not used on Mac OS X.
DialogRef GetNewDialog(SInt16 dialogID, void* dStorage, WindowRef behind)
{
	Size size = 0;
	unsigned char* raw = copyResource('DLOG', dialogID, &size);
	Dialog* d = NULL;

	if (raw && size >= 21 && raw[20] <= size - 21)
	{
		Rect bounds = rectAt(raw);
		WindowClass cls;
		WindowAttributes attributes;
		Size tail = 21 + raw[20];
		UInt16 position = 0;

		if (tail & 1)
			tail++;
		if (tail + 2 <= size)
			position = (UInt16) be16(raw + tail);
		if (bounds.bottom > bounds.top && bounds.right > bounds.left && windowClassOf((SInt16) be16(raw + 8), raw[12] != 0, &cls, &attributes) == noErr)
		{
			SRefCon refCon = (SRefCon) (uintptr_t) ((UInt32) be16(raw + 14) << 16 | be16(raw + 16));

			d = makeDialog(&bounds, cls, attributes, raw + 20, (SInt16) be16(raw + 18), position, raw[10] != 0, behind, refCon);
		}
	}
	free(raw);
	return d ? refOf(d) : NULL;
}

void DisposeDialog(DialogRef theDialog)
{
	for (Dialog** slot = &g_dialogs; *slot; slot = &(*slot)->next)
	{
		if ((DialogRef) (*slot)->window == theDialog)
		{
			Dialog* d = *slot;

			*slot = d->next;
			discard(d);
			return;
		}
	}
}

WindowRef GetDialogWindow(DialogRef dialog)
{
	Dialog* d = dialogOf(dialog);

	return d ? d->window : NULL;
}

CGrafPtr GetDialogPort(DialogRef dialog)
{
	Dialog* d = dialogOf(dialog);

	return d ? GetWindowPort(d->window) : NULL;
}

DialogRef GetDialogFromWindow(WindowRef window)
{
	return dialogOf((DialogRef) window) ? (DialogRef) window : NULL;
}

#pragma mark Items

void GetDialogItem(DialogRef theDialog, DialogItemIndex itemNo, DialogItemType* itemType, Handle* item, Rect* box)
{
	DialogItem* it = itemOf(dialogOf(theDialog), itemNo);

	if (itemType)
		*itemType = it ? it->type : 0;
	if (item)
		*item = !it ? NULL : it->control ? (Handle) it->control : kindOf(it) == userItem ? (Handle) it->proc : it->text;
	if (box)
	{
		if (it)
			*box = it->bounds;
		else
			memset(box, 0, sizeof(*box));
	}
}

// Changes an item's type (the itemDisable bit), its rectangle, and a user item's procedure
void SetDialogItem(DialogRef theDialog, DialogItemIndex itemNo, DialogItemType itemType, Handle item, const Rect* box)
{
	Dialog* d = dialogOf(theDialog);
	DialogItem* it = itemOf(d, itemNo);

	if (!it)
		return;
	if ((DialogItemType) (itemType & ~itemDisable) == kindOf(it))
		it->type = itemType;
	if (kindOf(it) == userItem)
		it->proc = (UserItemUPP) item;
	if (box)
	{
		it->bounds = *box;
		if (it->control)
			SetControlBounds(it->control, box);
	}
}

void GetDialogItemText(Handle item, Str255 text)
{
	Dialog* d = NULL;
	DialogItem* it = itemOfHandle(item, &d);

	if (!text)
		return;
	text[0] = 0;
	if (it && it->control)
		GetControlTitle(it->control, text);
	else if (item && *item)
	{
		Size n = GetHandleSize(item);

		if (n > 255)
			n = 255;
		text[0] = (unsigned char) n;
		memcpy(text + 1, *item, n);
	}
}

// The text is drawn again at once when the dialog is visible (Macintosh Toolbox Essentials, 6-125)
void SetDialogItemText(Handle item, ConstStr255Param text)
{
	Dialog* d = NULL;
	DialogItem* it = itemOfHandle(item, &d);

	if (!text)
		return;
	if (it && it->control)
	{
		SetControlTitle(it->control, text);
		return;
	}
	if (!item)
		return;
	SetHandleSize(item, text[0]);
	if (GetHandleSize(item) != text[0])
		return;
	if (text[0])
		memcpy(*item, text + 1, text[0]);
	if (it)
		redrawItem(d, (int) (it - d->items));
}

static void setHidden(DialogRef theDialog, DialogItemIndex itemNo, bool hidden)
{
	Dialog* d = dialogOf(theDialog);
	DialogItem* it = itemOf(d, itemNo);

	if (!it || isHidden(it) == hidden)
		return;

	Rect area = it->bounds;

	InsetRect(&area, -kEditInset - 1, -kEditInset - 1);
	if (hidden)
		InvalWindowRect(d->window, &area);
	it->bounds.left += hidden ? kHiddenOffset : -kHiddenOffset;
	it->bounds.right += hidden ? kHiddenOffset : -kHiddenOffset;
	if (it->control)
	{
		if (hidden)
			HideControl(it->control);
		else
			ShowControl(it->control);
	}
	if (!hidden)
		InvalWindowRect(d->window, &area);
}

void HideDialogItem(DialogRef theDialog, DialogItemIndex itemNo)
{
	setHidden(theDialog, itemNo, true);
}

void ShowDialogItem(DialogRef theDialog, DialogItemIndex itemNo)
{
	setHidden(theDialog, itemNo, false);
}

DialogItemIndex CountDITL(DialogRef theDialog)
{
	Dialog* d = dialogOf(theDialog);

	return d ? d->count : 0;
}

OSStatus SetDialogDefaultItem(DialogRef theDialog, DialogItemIndex newItem)
{
	Dialog* d = dialogOf(theDialog);

	if (!d || newItem < 0 || newItem > d->count)
		return paramErr;
	d->defaultItem = newItem;
	if (IsWindowVisible(d->window))
		InvalWindowRect(d->window, &(Rect) { -32768, -32768, 32767, 32767 });
	return noErr;
}

OSStatus SetDialogCancelItem(DialogRef theDialog, DialogItemIndex newItem)
{
	Dialog* d = dialogOf(theDialog);

	if (!d || newItem < 0 || newItem > d->count)
		return paramErr;
	d->cancelItem = newItem;
	return noErr;
}

DialogItemIndex GetDialogDefaultItem(DialogRef dialog)
{
	Dialog* d = dialogOf(dialog);

	return d ? d->defaultItem : 0;
}

DialogItemIndex GetDialogCancelItem(DialogRef dialog)
{
	Dialog* d = dialogOf(dialog);

	return d ? d->cancelItem : 0;
}

DialogItemIndex GetDialogKeyboardFocusItem(DialogRef dialog)
{
	Dialog* d = dialogOf(dialog);

	return d && d->focus >= 0 ? (DialogItemIndex) (d->focus + 1) : -1;
}

// The font of the static and edit text of the dialogs made afterwards
void SetDialogFont(SInt16 fontNum)
{
	g_dialogFont = fontNum;
}

#pragma mark Alerts

void ParamText(ConstStr255Param param0, ConstStr255Param param1, ConstStr255Param param2, ConstStr255Param param3)
{
	ConstStr255Param params[4] = { param0, param1, param2, param3 };

	for (int i = 0; i < 4; i++)
	{
		if (params[i])
			memcpy(g_paramText[i], params[i], params[i][0] + 1);
		else
			g_paramText[i][0] = 0;
	}
}

// An alert from an 'ALRT' resource: bounds, items, stages and position. Stage 1 says whether the alert is shown (else
// -1 is returned) and its default button, item 1 or 2; the other one of the two is the cancel item when it is a
// button.
DialogItemIndex Alert(SInt16 alertID, ModalFilterUPP modalFilter)
{
	Size size = 0;
	unsigned char* raw = copyResource('ALRT', alertID, &size);

	if (!raw || size < 12)
	{
		free(raw);
		return -1;
	}

	Rect bounds = rectAt(raw);
	SInt16 itemsID = (SInt16) be16(raw + 8);
	unsigned stage = be16(raw + 10) & 0xF;
	UInt16 position = size >= 14 ? (UInt16) be16(raw + 12) : 0;

	free(raw);
	if (!(stage & 4))
		return -1;

	Dialog* d = makeDialog(&bounds, kAlertWindowClass, 0, NULL, itemsID, position, false, (WindowRef) -1, 0);

	if (!d)
		return -1;
	d->defaultItem = (stage & 8) ? 2 : 1;
	if (d->defaultItem > d->count)
		d->defaultItem = 1;

	DialogItemIndex other = d->defaultItem == 1 ? 2 : 1;

	if (other <= d->count && kindOf(&d->items[other - 1]) == kButtonDialogItem)
		d->cancelItem = other;
	ShowWindow(d->window);
	SelectWindow(d->window);

	WindowRef window = d->window;
	DialogItemIndex hit = runModal(d, modalFilter);

	DisposeDialog((DialogRef) window);
	return hit;
}

DialogItemIndex StopAlert(SInt16 alertID, ModalFilterUPP modalFilter)
{
	return Alert(alertID, modalFilter);
}

DialogItemIndex NoteAlert(SInt16 alertID, ModalFilterUPP modalFilter)
{
	return Alert(alertID, modalFilter);
}

DialogItemIndex CautionAlert(SInt16 alertID, ModalFilterUPP modalFilter)
{
	return Alert(alertID, modalFilter);
}
#endif
