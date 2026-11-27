/*
 * Copyright (C) 2026 fr4nck
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// QuickDraw text (32-bit only): DrawText, DrawString, DrawChar, TextWidth, StringWidth, CharWidth, GetFontInfo and
// GetFNum, drawn with Core Text through a Quartz context over the port's pixels (QDBridge.c).
// The text of the current port's font family (txFont), size (txSize, 0: 12 points) and style (bold and italic of
// txFace) is drawn from the pen location, on the baseline, in the foreground color, and the pen moves by the text's
// width. Transfer modes: srcCopy and patCopy fill the text's box with the background color first; the other modes
// draw over what is there (srcOr). The clip rectangle of the port applies; non-rectangular clip regions clip to their
// bounding box. The bytes are Mac OS Roman. Family numbers map to the fonts of this system: 0 (system font) and 1
// (application font) to the user interface fonts, 4 (Monaco) and 22 (Courier) to a monospaced font, 20 (Times) to
// Times, 21 (Helvetica) to Helvetica; other families use the application font.
// Reference: Inside Macintosh: Text, chapter 3 (QuickDraw Text).
#ifndef __LP64__

#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>
#include <string.h>
#include "QDPortPrivate.h"

enum { kQDSystemFont = 0, kQDApplFont = 1, kQDNewYork = 2, kQDGeneva = 3, kQDMonaco = 4, kQDTimes = 20,
	kQDHelvetica = 21, kQDCourier = 22 };
enum { kQDBold = 1, kQDItalic = 2 };
enum { kQDSrcCopy = 0, kQDPatCopy = 8 };

static const struct { const char *name; short id; } kFamilies[] = {
	{"Chicago", kQDSystemFont}, {"Geneva", kQDGeneva}, {"New York", kQDNewYork}, {"Monaco", kQDMonaco},
	{"Times", kQDTimes}, {"Helvetica", kQDHelvetica}, {"Courier", kQDCourier},
};

void GetFNum(ConstStr255Param name, short *familyID)
{
	if (!familyID)
		return;
	*familyID = 0;
	if (!name)
		return;
	for (size_t i = 0; i < sizeof(kFamilies) / sizeof(kFamilies[0]); i++) {
		size_t n = strlen(kFamilies[i].name);
		if (name[0] == n && !strncasecmp((const char *) name + 1, kFamilies[i].name, n)) {
			*familyID = kFamilies[i].id;
			return;
		}
	}
}

// The name of a font family: the inverse of GetFNum. A family number that is not one of the known ones has no name:
// the result is the empty string.
void GetFontName(short familyID, Str255 name)
{
	if (!name)
		return;
	name[0] = 0;
	for (size_t i = 0; i < sizeof(kFamilies) / sizeof(kFamilies[0]); i++) {
		if (kFamilies[i].id == familyID) {
			size_t n = strlen(kFamilies[i].name);
			name[0] = (unsigned char) n;
			memcpy(name + 1, kFamilies[i].name, n);
			return;
		}
	}
}

static struct QDPort *text_port(void)
{
	struct QDPort *p = PORT_FROM(gCurrentPort);
	return p && QDPortIsLive(p) ? p : NULL;
}

static CTFontRef create_font(const struct QDPort *p)
{
	CGFloat size = p->txSize > 0 ? p->txSize : 12;
	CTFontRef font = NULL;
	switch (p->txFont) {
		case kQDSystemFont: font = CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, size, NULL); break;
		case kQDMonaco: case kQDCourier: font = CTFontCreateUIFontForLanguage(kCTFontUIFontUserFixedPitch, size, NULL); break;
		case kQDTimes: font = CTFontCreateWithName(CFSTR("Times"), size, NULL); break;
		case kQDHelvetica: font = CTFontCreateWithName(CFSTR("Helvetica"), size, NULL); break;
		default: font = CTFontCreateUIFontForLanguage(kCTFontUIFontApplication, size, NULL); break;
	}
	if (!font)
		font = CTFontCreateWithName(CFSTR("Helvetica"), size, NULL);
	CTFontSymbolicTraits traits = 0;
	if (p->txFace & kQDBold) traits |= kCTFontBoldTrait;
	if (p->txFace & kQDItalic) traits |= kCTFontItalicTrait;
	if (font && traits) {
		CTFontRef styled = CTFontCreateCopyWithSymbolicTraits(font, 0, NULL, traits, traits);
		if (styled) { CFRelease(font); font = styled; }
	}
	return font;
}

static CTLineRef create_line(const struct QDPort *p, const void *text, short firstByte, short byteCount, CGColorRef color)
{
	CFStringRef string = CFStringCreateWithBytes(NULL, (const UInt8 *) text + firstByte, byteCount, kCFStringEncodingMacRoman, false);
	if (!string)
		return NULL;
	CTFontRef font = create_font(p);
	CFTypeRef keys[2] = { kCTFontAttributeName, kCTForegroundColorAttributeName };
	CFTypeRef values[2] = { font, color };
	CFDictionaryRef attributes = CFDictionaryCreate(NULL, keys, values, color ? 2 : 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFAttributedStringRef attributed = CFAttributedStringCreate(NULL, string, attributes);
	CTLineRef line = attributed ? CTLineCreateWithAttributedString(attributed) : NULL;
	if (attributed) CFRelease(attributed);
	CFRelease(attributes);
	if (font) CFRelease(font);
	CFRelease(string);
	return line;
}

short TextWidth(const void *text, short firstByte, short byteCount)
{
	struct QDPort *p = text_port();
	if (!p || !text || byteCount <= 0)
		return 0;
	CTLineRef line = create_line(p, text, firstByte, byteCount, NULL);
	if (!line)
		return 0;
	double width = CTLineGetTypographicBounds(line, NULL, NULL, NULL);
	CFRelease(line);
	return (short) (width + 0.5);
}

short StringWidth(ConstStr255Param s) { return s ? TextWidth(s, 1, s[0]) : 0; }
short CharWidth(CharParameter ch) { unsigned char c = (unsigned char) ch; return TextWidth(&c, 0, 1); }

void GetFontInfo(FontInfo *info)
{
	struct QDPort *p = text_port();
	if (!info)
		return;
	memset(info, 0, sizeof(*info));
	if (!p)
		return;
	CTFontRef font = create_font(p);
	if (!font)
		return;
	CGRect box = CTFontGetBoundingBox(font);
	info->ascent = (short) (CTFontGetAscent(font) + 0.5);
	info->descent = (short) (CTFontGetDescent(font) + 0.5);
	info->leading = (short) (CTFontGetLeading(font) + 0.5);
	info->widMax = (short) (box.size.width + 0.5);
	CFRelease(font);
}

void DrawText(const void *text, short firstByte, short byteCount)
{
	struct QDPort *p = text_port();
	if (!p || !text || byteCount <= 0)
		return;
	if (p->penVis < 0)
		return;
	CGContextRef ctx = NULL;
	if (QDBeginCGContext((CGrafPtr) gCurrentPort, &ctx) != noErr || !ctx)
		return;
	CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
	CGFloat fg[4] = { p->fgColor.red / 65535.0, p->fgColor.green / 65535.0, p->fgColor.blue / 65535.0, 1 };
	CGColorRef color = CGColorCreate(space, fg);
	CTLineRef line = create_line(p, text, firstByte, byteCount, color);
	if (line) {
		CGFloat ascent, descent;
		double width = CTLineGetTypographicBounds(line, &ascent, &descent, NULL);
		short height = (short) (p->portRect.bottom - p->portRect.top);
		// The context's origin is the bottom left of the port: flip to QuickDraw's y down, from portRect's top left.
		CGContextSaveGState(ctx);
		CGContextTranslateCTM(ctx, -p->portRect.left, height + p->portRect.top);
		CGContextScaleCTM(ctx, 1, -1);
		Rect clip = p->clipRect;
		if (p->clipRgn || p->clipIsRect)
			CGContextClipToRect(ctx, CGRectMake(clip.left, clip.top, clip.right - clip.left, clip.bottom - clip.top));
		if (p->txMode == kQDSrcCopy || p->txMode == kQDPatCopy) {
			CGContextSetRGBFillColor(ctx, p->bkColor.red / 65535.0, p->bkColor.green / 65535.0, p->bkColor.blue / 65535.0, 1);
			CGContextFillRect(ctx, CGRectMake(p->penLoc.h, p->penLoc.v - ascent, width, ascent + descent));
		}
		CGContextSetRGBFillColor(ctx, fg[0], fg[1], fg[2], 1);   // the glyphs are filled with the foreground color
		CGContextSetTextMatrix(ctx, CGAffineTransformMakeScale(1, -1));
		CGContextSetTextPosition(ctx, p->penLoc.h, p->penLoc.v);
		CTLineDraw(line, ctx);
		CGContextRestoreGState(ctx);
		p->penLoc.h = (short) (p->penLoc.h + (short) (width + 0.5));
		CFRelease(line);
	}
	CGColorRelease(color);
	CGColorSpaceRelease(space);
	QDEndCGContext((CGrafPtr) gCurrentPort, &ctx);
}

void DrawString(ConstStr255Param s) { if (s) DrawText(s, 1, s[0]); }
void DrawChar(CharParameter ch) { unsigned char c = (unsigned char) ch; DrawText(&c, 0, 1); }

#endif /* !__LP64__ */
