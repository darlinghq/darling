// SPDX-License-Identifier: GPL-3.0-or-later
// Picture recording for OpenCPicture / ClosePicture (included at the end of QDDrawing.c, which owns the pixel accessors).
// While a picture is open, drawing in the port that was current when it was opened is appended to the picture handle as
// version 2 opcodes (Inside Macintosh: Imaging With QuickDraw, appendix A). Recorded: RGBFgCol 0x1A and RGBBkCol 0x1B
// (RGBForeColor, RGBBackColor, ForeColor, BackColor), PnPat 0x09, PnSize 0x07, PnMode 0x08, Line 0x20,
// frameRect..fillRect 0x30-0x34 (FillPat 0x0A before fillRect), and CopyBits into the port as DirectBitsRect 0x9A
// (32-bit pixels, packType 4: PackBits component planes; packType 1 when rowBytes < 8).
// Not recorded: clip changes (ClipRect, SetClip), BackPat, regions (FillRgn,
// PaintRgn, EraseRgn), ScrollRect, SetCPixel, FillCRect, CopyBits wider than 4095 pixels. This QuickDraw draws no
// text, ovals, arcs, round rectangles or polygons, so pictures contain none of them.
#include <CarbonCore/MacMemory.h>

static struct {
	CGrafPtr port;          // port being recorded (NULL when no picture is open)
	Handle pic;
	Boolean failed;
} g_rec;

static void rec_bytes(const void *data, size_t n)
{
	if (!g_rec.port || g_rec.failed || !n) return;
	Size old = GetHandleSize(g_rec.pic);
	SetHandleSize(g_rec.pic, old + (Size) n);
	if (GetHandleSize(g_rec.pic) != old + (Size) n) { g_rec.failed = true; return; }   // the resize did not happen
	memcpy(*g_rec.pic + old, data, n);
}

static void rec_u16(unsigned v) { uint8_t b[2] = {(uint8_t) (v >> 8), (uint8_t) v}; rec_bytes(b, 2); }
static void rec_u32(uint32_t v) { rec_u16(v >> 16); rec_u16(v & 0xFFFF); }
static void rec_rect(const Rect *r) { rec_u16((uint16_t) r->top); rec_u16((uint16_t) r->left); rec_u16((uint16_t) r->bottom); rec_u16((uint16_t) r->right); }
static void rec_align(void) { if (g_rec.port && !g_rec.failed && (GetHandleSize(g_rec.pic) & 1)) { uint8_t z = 0; rec_bytes(&z, 1); } }

static Boolean recording(struct QDPort *p) { return g_rec.port && p && (CGrafPtr) p == g_rec.port; }

void QDRecordColor(struct QDPort *p, Boolean fore, const RGBColor *c)
{
	if (!recording(p) || !c) return;
	rec_u16(fore ? 0x001A : 0x001B); rec_u16(c->red); rec_u16(c->green); rec_u16(c->blue);
}

void QDRecordRect(struct QDPort *p, unsigned op, const Rect *r, const QDPattern *pat)
{
	if (!recording(p) || !r) return;
	if (op == 0x34 && pat) { rec_u16(0x000A); rec_bytes(pat->pat, 8); }   // FillPat, then fillRect
	rec_u16(op); rec_rect(r);
}

void QDRecordLine(struct QDPort *p, Point from, Point to)
{
	if (!recording(p)) return;
	rec_u16(0x0007); rec_u16((uint16_t) p->penSize.v); rec_u16((uint16_t) p->penSize.h);   // PnSize (v, h)
	rec_u16(0x0008); rec_u16((uint16_t) p->penMode);                                    // PnMode
	rec_u16(0x0020); rec_u16((uint16_t) from.v); rec_u16((uint16_t) from.h); rec_u16((uint16_t) to.v); rec_u16((uint16_t) to.h);
}

void QDRecordPenPat(struct QDPort *p, const QDPattern *pat)
{
	if (!recording(p) || !pat) return;
	rec_u16(0x0009); rec_bytes(pat->pat, 8);                       // PnPat
}

// PackBits (Macintosh Technical Note 1023): runs of 2+ equal bytes as (1 - n, byte), others literal (n - 1, bytes).
static size_t packbits(const uint8_t *in, size_t n, uint8_t *out)
{
	size_t i = 0, o = 0;
	while (i < n) {
		size_t run = 1;
		while (i + run < n && run < 128 && in[i + run] == in[i]) run++;
		if (run >= 2) { out[o++] = (uint8_t) (1 - (int) run); out[o++] = in[i]; i += run; continue; }
		size_t start = i, len = 0;
		while (i < n && len < 128 && !(i + 1 < n && in[i + 1] == in[i])) { i++; len++; }
		if (len == 0) { i++; len = 1; }    // a single byte before a run
		out[o++] = (uint8_t) (len - 1);
		memcpy(out + o, in + start, len); o += len;
	}
	return o;
}

// A 32-bit CopyBits is recorded as QuickDraw records it: DirectBitsRect, packType 4 (each row as red, green and blue
// planes compressed with PackBits, preceded by its byte count: 1 byte when rowBytes <= 250, else 2), cmpCount 3.
// Rows narrower than 8 bytes are stored unpacked (packType 1, X R G B), as the format requires.
static void rec_copybits(struct QDPort *s, const Rect *src, const Rect *dst, short mode)
{
	int w = src->right - src->left, h = src->bottom - src->top;
	if (w <= 0 || h <= 0) return;
	if ((size_t) w * 4 > 0x3FFF) return;   // rowBytes field holds 14 bits
	size_t rowBytes = (size_t) w * 4;
	Boolean packed = rowBytes >= 8;
	rec_u16(0x009A);
	rec_u32(0x000000FF);                       // baseAddr (not used in pictures)
	rec_u16(0x8000 | (unsigned) rowBytes);     // rowBytes, PixMap flag
	Rect bounds = *src;
	rec_rect(&bounds);
	rec_u16(0);                                // pmVersion
	rec_u16(packed ? 4 : 1);                   // packType
	rec_u32(0);                                // packSize
	rec_u32(0x00480000); rec_u32(0x00480000);  // hRes, vRes: 72 dpi
	rec_u16(16);                               // pixelType RGBDirect
	rec_u16(32); rec_u16(3); rec_u16(8);       // pixelSize, cmpCount, cmpSize
	rec_u32(0); rec_u32(0); rec_u32(0);        // planeBytes, pmTable, pmReserved
	rec_rect(src); rec_rect(dst);
	rec_u16((uint16_t) mode);
	uint8_t *row = malloc(rowBytes), *pk = malloc((size_t) w * 3 + (size_t) w * 3 / 128 + 8);
	if (!row || !pk) { free(row); free(pk); g_rec.failed = true; return; }
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			uint8_t *a = pixel_at(s, src->left + x, src->top + y);
			RGBColor c = a ? load(s, a) : (RGBColor) {0, 0, 0};
			if (packed) { row[x] = c.red >> 8; row[w + x] = c.green >> 8; row[2 * w + x] = c.blue >> 8; }
			else { row[4 * x] = 0; row[4 * x + 1] = c.red >> 8; row[4 * x + 2] = c.green >> 8; row[4 * x + 3] = c.blue >> 8; }
		}
		if (!packed) { rec_bytes(row, rowBytes); continue; }
		size_t n = packbits(row, (size_t) w * 3, pk);
		if (rowBytes > 250) rec_u16((unsigned) n); else { uint8_t b = (uint8_t) n; rec_bytes(&b, 1); }
		rec_bytes(pk, n);
	}
	free(row); free(pk);
	rec_align();
}

// Starts recording in the current port; returns the picture handle (header written) or NULL with QDError set.
Handle QDRecordOpen(const Rect *srcRect, Fixed hRes, Fixed vRes, short version)
{
	struct QDPort *p = current();
	if (!p || !srcRect) { gQDError = paramErr; return NULL; }
	if (g_rec.port) { gQDError = paramErr; return NULL; }         // one picture open at a time
	Handle h = NewHandle(0);
	if (!h) { gQDError = cNoMemErr; return NULL; }
	g_rec.port = (CGrafPtr) p; g_rec.pic = h; g_rec.failed = false;
	Rect frame = *srcRect;
	if (version == -2 && hRes && hRes != 0x00480000) {           // picFrame is at 72 dpi
		frame.right = (short) (frame.left + (int32_t) ((int64_t) (srcRect->right - srcRect->left) * 0x00480000 / hRes));
		frame.bottom = (short) (frame.top + (int32_t) ((int64_t) (srcRect->bottom - srcRect->top) * 0x00480000 / vRes));
	}
	rec_u16(0); rec_rect(&frame);              // picSize (set by ClosePicture), picFrame
	rec_u16(0x0011); rec_u16(0x02FF);          // VersionOp, version 2
	rec_u16(0x0C00);                           // HeaderOp
	rec_u16((uint16_t) version); rec_u16(0);
	rec_u32((uint32_t) hRes); rec_u32((uint32_t) vRes);
	rec_rect(srcRect); rec_u32(0);
	rec_u16(0x001E);                           // DefHilite
	if (g_rec.failed) { DisposeHandle(h); g_rec.port = NULL; gQDError = cNoMemErr; return NULL; }
	p->penVis--;                               // HidePen: picture drawing is not shown while recording
	gQDError = noErr;
	return h;
}

// Ends the picture: OpEndPic, picSize (low 16 bits of the size, as version 2 pictures do).
void QDRecordClose(void)
{
	if (!g_rec.port) { gQDError = paramErr; return; }
	struct QDPort *p = PORT_FROM(g_rec.port);
	rec_align();
	rec_u16(0x00FF);
	Size n = GetHandleSize(g_rec.pic);
	if (!g_rec.failed && n >= 2) { (*g_rec.pic)[0] = (char) (n >> 8); (*g_rec.pic)[1] = (char) n; }
	if (QDPortIsLive(p)) p->penVis++;          // ShowPen
	gQDError = g_rec.failed ? cNoMemErr : noErr;
	g_rec.port = NULL;
}
