/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// Playback of version 2 pictures ('PICT' resources) into the current port (32-bit only).
//
// Supported opcodes: the header and no-op records, comments, clip, colors, pen and pattern
// state, rectangle and line drawing, and the pixel opcodes PackBitsRect, PackBitsRgn,
// DirectBitsRect and DirectBitsRgn (1-8 bit indexed with a color table, 16 and 32 bit direct;
// the region of the *Rgn forms is skipped), and the QuickTime opcodes CompressedQuickTime and
// UncompressedQuickTime for images that ImageIO decodes (TIFF, JPEG, PNG, GIF, BMP; the matrix
// is taken as the identity, the matte and the mask are skipped). Text opcodes are skipped and counted, since this
// QuickDraw draws no text. Any other opcode stops the playback with QDError set to
// pictureDataErr; what was drawn before it stays drawn. Version 1 pictures are refused.
// Reference: Imaging With QuickDraw, Appendix A.
#ifndef __LP64__

#include "QDPortPrivate.h"
#include <CarbonCore/MacMemory.h>
#include <CarbonCore/Resources.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum { kQDPictUnsupportedOpcode = -11005 /* pictureDataErr */ };

typedef struct { SInt16 picSize; Rect picFrame; } QDPicture;
typedef QDPicture **QDPicHandle;

void PenSize(short, short);
void PenMode(short);
void PenPat(const QDPattern *);
void RGBForeColor(const RGBColor *);
void RGBBackColor(const RGBColor *);
void MoveTo(short, short);
void LineTo(short, short);
void FrameRect(const Rect *);
void PaintRect(const Rect *);
void EraseRect(const Rect *);
void InvertRect(const Rect *);
void FillRect(const Rect *, const QDPattern *);
void TextFont(short);
void TextSize(short);
void TextMode(short);
void TextFace(StyleParameter);

struct reader { const uint8_t *p, *end; Boolean bad; };

static unsigned u8(struct reader *r) { if (r->p + 1 > r->end) { r->bad = true; return 0; } return *r->p++; }
static unsigned u16(struct reader *r) { unsigned a = u8(r); return a << 8 | u8(r); }
static uint32_t u32(struct reader *r) { uint32_t a = u16(r); return a << 16 | u16(r); }
static void skip(struct reader *r, size_t n) { if ((size_t) (r->end - r->p) < n) { r->bad = true; r->p = r->end; } else r->p += n; }
static Rect rect(struct reader *r) { Rect x; x.top = (short) u16(r); x.left = (short) u16(r); x.bottom = (short) u16(r); x.right = (short) u16(r); return x; }
static Point point(struct reader *r) { Point x; x.v = (short) u16(r); x.h = (short) u16(r); return x; }

struct mapping { Rect frame, dst; };

static short mx(const struct mapping *m, int h)
{
	int fw = m->frame.right - m->frame.left, dw = m->dst.right - m->dst.left;
	return (short) (m->dst.left + (fw ? (int64_t) (h - m->frame.left) * dw / fw : 0));
}

static short my(const struct mapping *m, int v)
{
	int fh = m->frame.bottom - m->frame.top, dh = m->dst.bottom - m->dst.top;
	return (short) (m->dst.top + (fh ? (int64_t) (v - m->frame.top) * dh / fh : 0));
}

static Rect map_rect(const struct mapping *m, Rect r)
{
	Rect o;
	SetRect(&o, mx(m, r.left), my(m, r.top), mx(m, r.right), my(m, r.bottom));
	return o;
}

// Unpacks one PackBits row of `want` bytes (`unit` 1: bytes, 2: 16-bit words).
static Boolean unpack_row(struct reader *r, size_t rowBytes, uint8_t *out, size_t want, int unit)
{
	size_t count = rowBytes > 250 ? u16(r) : u8(r);
	const uint8_t *end = r->p + count;
	if (end > r->end) { r->bad = true; return false; }
	size_t n = 0;
	while (r->p < end && n < want) {
		int8_t flag = (int8_t) *r->p++;
		if (flag >= 0) {
			size_t len = ((size_t) flag + 1) * unit;
			if (r->p + len > end || n + len > want) return false;
			memcpy(out + n, r->p, len);
			r->p += len;
			n += len;
		} else if (flag != -128) {
			size_t len = (size_t) (1 - flag);
			if (r->p + unit > end || n + len * unit > want) return false;
			for (size_t i = 0; i < len; i++, n += unit)
				memcpy(out + n, r->p, unit);
			r->p += unit;
		}
	}
	r->p = end;
	return n == want;
}

static Boolean pixels(struct reader *r, unsigned op, struct QDPort *port, const struct mapping *m)
{
	Boolean direct = op == 0x9A || op == 0x9B;
	if (direct) skip(r, 4); // baseAddr
	unsigned rowBytesField = u16(r);
	Boolean isPixMap = direct || (rowBytesField & 0x8000);
	size_t rowBytes = rowBytesField & 0x3FFF;
	Rect bounds = rect(r);
	unsigned packType = 0, pixelSize = 1, cmpCount = 1;
	RGBColor *ctab = NULL;
	unsigned ctabSize = 0;
	if (isPixMap) {
		u16(r); packType = u16(r); u32(r); u32(r); u32(r); u16(r);
		pixelSize = u16(r); cmpCount = u16(r); u16(r); skip(r, 12);
		if (!direct) {
			u32(r);
			unsigned flags = u16(r);
			ctabSize = u16(r) + 1;
			ctab = calloc(ctabSize < 256 ? 256 : ctabSize, sizeof(RGBColor));
			if (!ctab) return false;
			for (unsigned i = 0; i < ctabSize; i++) {
				unsigned value = u16(r);
				RGBColor c; c.red = u16(r); c.green = u16(r); c.blue = u16(r);
				unsigned index = (flags & 0x8000) ? i : value;
				if (index < 256) ctab[index] = c;
			}
		}
	}
	Rect src = rect(r), dst = rect(r);
	u16(r); // transfer mode: the picture is copied
	if (op == 0x99 || op == 0x9B) skip(r, u16(r) - 2); // mask region
	int width = bounds.right - bounds.left, height = bounds.bottom - bounds.top;
	if (r->bad || width <= 0 || height <= 0 || rowBytes == 0) { free(ctab); return false; }

	GWorldPtr tmp = NULL;
	if (NewGWorld(&tmp, 32, &bounds, NULL, NULL, 0) != noErr) { free(ctab); return false; }
	struct QDPort *t = PORT_FROM(tmp);
	uint8_t *row = malloc(rowBytes * 4 + 8);
	Boolean ok = row != NULL;
	for (int y = 0; ok && y < height; y++) {
		size_t want = rowBytes;
		if (direct && packType == 2) want = (size_t) width * 3;
		if (rowBytes < 8 || packType == 1 || (direct && packType == 2)) {
			if ((size_t) (r->end - r->p) < want) { ok = false; break; }
			memcpy(row, r->p, want);
			r->p += want;
		} else if (pixelSize == 16) {
			ok = unpack_row(r, rowBytes, row, rowBytes, 2);
		} else if (pixelSize == 32) {
			want = (size_t) width * cmpCount;
			ok = unpack_row(r, rowBytes, row, want, 1);
		} else {
			ok = unpack_row(r, rowBytes, row, rowBytes, 1);
		}
		if (!ok) break;
		uint8_t *out = (uint8_t *) t->pixels + (size_t) y * t->rowBytes;
		for (int x = 0; x < width; x++) {
			RGBColor c;
			if (!isPixMap) {
				Boolean bit = (row[x >> 3] >> (7 - (x & 7))) & 1;
				c = bit ? port->fgColor : port->bkColor;
			} else if (pixelSize <= 8) {
				unsigned per = 8 / pixelSize, shift = (per - 1 - x % per) * pixelSize;
				unsigned index = (row[x / per] >> shift) & ((1u << pixelSize) - 1);
				c = ctab[index];
			} else if (pixelSize == 16) {
				unsigned v = (unsigned) row[2 * x] << 8 | row[2 * x + 1];
				c.red = (UInt16) (((v >> 10) & 31) * 0x0842); c.green = (UInt16) (((v >> 5) & 31) * 0x0842); c.blue = (UInt16) ((v & 31) * 0x0842);
			} else if (packType == 2 || (direct && packType == 1 && rowBytes < (size_t) width * 4)) {
				c.red = row[3 * x] * 0x101; c.green = row[3 * x + 1] * 0x101; c.blue = row[3 * x + 2] * 0x101;
			} else if (rowBytes < 8 || packType == 1) {
				c.red = row[4 * x + 1] * 0x101; c.green = row[4 * x + 2] * 0x101; c.blue = row[4 * x + 3] * 0x101;
			} else { // component planes, alpha first when there are four
				unsigned base = cmpCount == 4 ? (unsigned) width : 0;
				c.red = row[base + x] * 0x101; c.green = row[base + width + x] * 0x101; c.blue = row[base + 2 * width + x] * 0x101;
			}
			out[0] = 0xFF; out[1] = c.red >> 8; out[2] = c.green >> 8; out[3] = c.blue >> 8;
			out += 4;
		}
	}
	free(row);
	free(ctab);
	if (ok) {
		Rect target = map_rect(m, dst);
		CopyBits((const BitMap *) &t->pixMap, (const BitMap *) &port->pixMap, &src, &target, 0, NULL);
	}
	DisposeGWorld(tmp);
	return ok;
}


// The QuickTime picture opcodes 0x8200 (CompressedQuickTime) and 0x8201 (UncompressedQuickTime): a record with the
// transformation matrix, an optional matte, the transfer mode, the source rectangle, the accuracy, a mask region and
// then an image description followed by the image data. An image of a format ImageIO reads is drawn at the source
// rectangle; any other codec (or damage) makes the picture unsupported.
static Boolean quicktime_image(struct reader *r, struct QDPort *port, const struct mapping *m)
{
	uint32_t size = u32(r);
	if (r->bad || (size_t) (r->end - r->p) < size) return false;
	struct reader b = {r->p, r->p + size, false};
	skip(r, size);
	u16(&b);                    // version
	skip(&b, 36);               // matrix
	uint32_t matteSize = u32(&b);
	skip(&b, 8);                // matte rectangle
	u16(&b);                    // transfer mode
	Rect src = rect(&b);
	u32(&b);                    // accuracy
	uint32_t maskSize = u32(&b);
	skip(&b, matteSize);
	skip(&b, maskSize);
	uint32_t descriptionSize = u32(&b);
	if (b.bad || descriptionSize < 46 || (size_t) (b.end - b.p) < descriptionSize - 4) return false;
	const uint8_t *description = b.p - 4;
	b.p = description + 44;
	uint32_t dataSize = u32(&b);
	b.p = description + descriptionSize;
	size_t available = (size_t) (b.end - b.p);
	if (dataSize == 0 || dataSize > available) dataSize = (uint32_t) available;

	int width = src.right - src.left, height = src.bottom - src.top;
	if (width <= 0 || height <= 0 || dataSize == 0) return false;

	CFDataRef data = CFDataCreate(NULL, b.p, dataSize);
	CGImageSourceRef source = data ? CGImageSourceCreateWithData(data, NULL) : NULL;
	CGImageRef image = source ? CGImageSourceCreateImageAtIndex(source, 0, NULL) : NULL;
	if (source) CFRelease(source);
	if (data) CFRelease(data);
	if (!image) return false;

	GWorldPtr tmp = NULL;
	Rect bounds;
	SetRect(&bounds, 0, 0, width, height);
	if (NewGWorld(&tmp, 32, &bounds, NULL, NULL, 0) != noErr) { CGImageRelease(image); return false; }
	struct QDPort *t = PORT_FROM(tmp);
	CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
	// the pixels of a port are 0xFF, red, green, blue in memory; the image is drawn over white
	CGContextRef context = CGBitmapContextCreate(t->pixels, width, height, 8, t->rowBytes, space, kCGBitmapByteOrder32Big | kCGImageAlphaNoneSkipFirst);
	CGColorSpaceRelease(space);
	Boolean ok = context != NULL;
	if (ok) {
		CGContextSetRGBFillColor(context, 1, 1, 1, 1);
		CGContextFillRect(context, CGRectMake(0, 0, width, height));
		CGContextDrawImage(context, CGRectMake(0, 0, width, height), image);
		CGContextRelease(context);
		Rect target = map_rect(m, src), from = bounds;
		CopyBits((const BitMap *) &t->pixMap, (const BitMap *) &port->pixMap, &from, &target, 0, NULL);
	}
	DisposeGWorld(tmp);
	CGImageRelease(image);
	return ok;
}

static QDErr play(const uint8_t *data, size_t size, const Rect *dstRect, unsigned *skippedText)
{
	struct QDPort *port = PORT_FROM(gCurrentPort);
	if (!port || !QDPortIsLive(port)) return paramErr;
	struct reader r = {data, data + size, false};
	u16(&r);
	struct mapping m = {rect(&r), *dstRect};
	if (u16(&r) != 0x0011 || u16(&r) != 0x02FF) return kQDPictUnsupportedOpcode; // version 2 only
	Point pen = {0, 0};
	QDPattern fill;
	memset(&fill, 0xFF, sizeof(fill));
	while (!r.bad) {
		if ((r.p - data) & 1) r.p++; // opcodes are word aligned
		unsigned op = u16(&r);
		if (r.bad) break;
		switch (op) {
			case 0x0000: case 0x001E: break;                        // NOP, DefHilite
			case 0x001F: skip(&r, 6); break;                        // OpColor: the color of the blend modes
			case 0x0010: skip(&r, 8); break;                        // TxRatio: the scale of the text, which is skipped
			case 0x0006: case 0x000B: case 0x000E: case 0x000F: skip(&r, 4); break; // SpExtra, OvSize, the color constants: no effect on the playback
			case 0x00FF: return noErr;                              // OpEndPic
			case 0x0C00: skip(&r, 24); break;                      // HeaderOp
			case 0x0001: { unsigned n = u16(&r); skip(&r, n - 2); break; } // Clip: the frame clips
			case 0x0003: TextFont((short) u16(&r)); break;
			case 0x0004: TextFace((StyleParameter) u8(&r)); break;
			case 0x0005: TextMode((short) u16(&r)); break;
			case 0x000D: TextSize((short) u16(&r)); break;
			case 0x0007: { Point s = point(&r); PenSize(s.h, s.v); break; }
			case 0x0008: PenMode((short) u16(&r)); break;
			case 0x0009: { QDPattern pat; for (int i = 0; i < 8; i++) pat.pat[i] = (UInt8) u8(&r); PenPat(&pat); break; }
			case 0x000A: for (int i = 0; i < 8; i++) fill.pat[i] = (UInt8) u8(&r); break;
			case 0x001A: { RGBColor c; c.red = u16(&r); c.green = u16(&r); c.blue = u16(&r); RGBForeColor(&c); break; }
			case 0x001B: { RGBColor c; c.red = u16(&r); c.green = u16(&r); c.blue = u16(&r); RGBBackColor(&c); break; }
			case 0x0020: { Point a = point(&r), b = point(&r); MoveTo(mx(&m, a.h), my(&m, a.v)); LineTo(mx(&m, b.h), my(&m, b.v)); pen = b; break; }
			case 0x0021: { Point b = point(&r); MoveTo(mx(&m, pen.h), my(&m, pen.v)); LineTo(mx(&m, b.h), my(&m, b.v)); pen = b; break; }
			case 0x0022: { Point a = point(&r); int dh = (int8_t) u8(&r), dv = (int8_t) u8(&r); MoveTo(mx(&m, a.h), my(&m, a.v)); LineTo(mx(&m, a.h + dh), my(&m, a.v + dv)); pen.h = a.h + dh; pen.v = a.v + dv; break; }
			case 0x0023: { int dh = (int8_t) u8(&r), dv = (int8_t) u8(&r); MoveTo(mx(&m, pen.h), my(&m, pen.v)); LineTo(mx(&m, pen.h + dh), my(&m, pen.v + dv)); pen.h += dh; pen.v += dv; break; }
			case 0x0030: { Rect x = map_rect(&m, rect(&r)); FrameRect(&x); break; }
			case 0x0031: { Rect x = map_rect(&m, rect(&r)); PaintRect(&x); break; }
			case 0x0032: { Rect x = map_rect(&m, rect(&r)); EraseRect(&x); break; }
			case 0x0033: { Rect x = map_rect(&m, rect(&r)); InvertRect(&x); break; }
			case 0x0034: { Rect x = map_rect(&m, rect(&r)); FillRect(&x, &fill); break; }
			case 0x0028: skip(&r, 4); skip(&r, u8(&r)); ++*skippedText; break;  // LongText
			case 0x0029: case 0x002A: skip(&r, 1); skip(&r, u8(&r)); ++*skippedText; break; // DHText, DVText
			case 0x002B: skip(&r, 2); skip(&r, u8(&r)); ++*skippedText; break;  // DHDVText
			case 0x002C: { unsigned n = u16(&r); skip(&r, n); break; }        // fontName
			case 0x00A0: skip(&r, 2); break;                                  // ShortComment
			case 0x00A1: { skip(&r, 2); unsigned n = u16(&r); skip(&r, n); break; } // LongComment
			case 0x0098: case 0x0099: case 0x009A: case 0x009B:
				if (!pixels(&r, op, port, &m)) return kQDPictUnsupportedOpcode;
				break;
			case 0x8200: case 0x8201:
				if (!quicktime_image(&r, port, &m)) return kQDPictUnsupportedOpcode;
				break;
			default:
				return kQDPictUnsupportedOpcode;
		}
	}
	return kQDPictUnsupportedOpcode;
}

static unsigned gLastSkippedText;

// Text opcodes skipped by the last DrawPicture (diagnostic accessor, private).
unsigned QDLastPictureSkippedText(void) { return gLastSkippedText; }

QDPicHandle GetPicture(short pictureID)
{
	return (QDPicHandle) GetResource('PICT', pictureID);
}

void DrawPicture(QDPicHandle pic, const Rect *dstRect)
{
	gLastSkippedText = 0;
	if (!pic || !*pic || !dstRect) { gQDError = paramErr; return; }
	Size size = GetHandleSize((Handle) pic);
	gQDError = play((const uint8_t *) *pic, (size_t) size, dstRect, &gLastSkippedText);
}

#endif /* !__LP64__ */
