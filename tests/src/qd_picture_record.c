// Copyright (C) 2026 fr4nck
//
// QuickDraw picture recording (OpenCPicture / ClosePicture), 32-bit only. A 96 x 64 image copied with CopyBits, a
// rectangle and a line are recorded; the version 2 bytes are checked (header, OpEndPic, picSize), then the picture is
// played back with DrawPicture and every pixel compared. Classic colors and the pen pattern are recorded; SetCPixel,
// ClipRect and regions are not, which the replay shows. One picture at a time; version -2 only.
#include <QD/QD.h>
#include <CarbonCore/MacMemory.h>
#include <stdio.h>
#include <string.h>

#ifdef __LP64__
int main(void)
{
	printf("PASSED (QuickDraw pictures: 32-bit only)\n");
	return 0;
}
#else

typedef OpenCPicParams CPParams;

void DrawPicture(PicHandle, const Rect *);
void RGBForeColor(const RGBColor *); void RGBBackColor(const RGBColor *);
void PaintRect(const Rect *); void EraseRect(const Rect *);
void MoveTo(short, short); void LineTo(short, short); void PenSize(short, short);
void SetCPixel(short, short, const RGBColor *); void GetCPixel(short, short, RGBColor *);
void SetGWorld(CGrafPtr, GDHandle);
const BitMap *GetPortBitMapForCopyBits(CGrafPtr);
void CopyBits(const BitMap *, const BitMap *, const Rect *, const Rect *, short, RgnHandle);
typedef struct { UInt8 pat[8]; } Pat;
void ForeColor(long); void PenPat(const Pat *); Pat *GetQDGlobalsGray(Pat *);
void ClipRect(const Rect *); void PaintRgn(RgnHandle);
short QDError(void);

static int g_failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL %d: %s\n", __LINE__, #c); g_failures++; } } while (0)
enum { W = 96, H = 64 };
static const RGBColor red = {0xFFFF, 0, 0}, blue = {0, 0, 0xFFFF}, black = {0, 0, 0}, white = {0xFFFF, 0xFFFF, 0xFFFF};

static RGBColor ramp(int f, int b)
{
	int fund = 4 + f / 12, level = (f * 7 + b * 13) % 23;
	if (b == fund || b == 2 * fund || b == 3 * fund) level = 255 - (f % 32);
	RGBColor c = {(unsigned short) (level * 257), (unsigned short) ((level > 128 ? 255 - level : level) * 2 * 257),
	              (unsigned short) ((255 - level) * 257)};
	return c;
}

static GWorldPtr world(short w, short h)
{
	Rect r; SetRect(&r, 0, 0, w, h);
	GWorldPtr g = NULL;
	if (NewGWorld(&g, 32, &r, NULL, NULL, 0) != noErr) return NULL;
	SetGWorld(g, NULL);
	RGBBackColor(&white);
	EraseRect(&r);
	return g;
}

static unsigned u16(const unsigned char *p) { return (p[0] << 8) | p[1]; }

int main(void)
{
	GWorldPtr off = world(W, H), win = world(W + 40, H + 30);
	CHECK(off && win);
	if (!off || !win) return 1;
	SetGWorld(off, NULL);
	for (int b = 0; b < H; b++) for (int f = 0; f < W; f++) { RGBColor c = ramp(f, b); SetCPixel(f, b, &c); }

	Rect src; SetRect(&src, 0, 0, W, H);
	CPParams cp = {src, 0x00480000, 0x00480000, -2, 0, 0};
	SetGWorld(win, NULL);
	ClosePicture();
	CHECK(QDError() == paramErr);
	PicHandle pic = OpenCPicture(&cp);
	CHECK(pic && QDError() == noErr);
	CHECK(OpenCPicture(&cp) == NULL && QDError() == paramErr);
	RGBForeColor(&black); RGBBackColor(&white);
	CopyBits(GetPortBitMapForCopyBits(off), GetPortBitMapForCopyBits(win), &src, &src, 0 /* srcCopy */, NULL);
	Rect mark; SetRect(&mark, 5, 5, 25, 15);
	RGBColor before, after; GetCPixel(10, 10, &before);
	RGBForeColor(&red); PaintRect(&mark);
	GetCPixel(10, 10, &after);
	CHECK(memcmp(&before, &after, sizeof(before)) == 0);   // the pen is hidden while recording
	RGBForeColor(&blue); PenSize(2, 2); MoveTo(0, H - 4); LineTo(W - 1, H - 4);
	{ RGBColor g = {0x8000, 0x8000, 0x8000}; SetCPixel(50, 30, &g); }   // not recorded: the replay shows the ramp there
	RGBForeColor(&black);
	ClosePicture();
	CHECK(QDError() == noErr);
	if (!pic) return 1;

	Size n = GetHandleSize((Handle) pic);
	const unsigned char *p = (const unsigned char *) *pic;
	CHECK(u16(p) == (unsigned) (n & 0xFFFF));
	CHECK(u16(p + 6) == H && u16(p + 8) == W);
	CHECK(u16(p + 10) == 0x0011 && u16(p + 12) == 0x02FF && u16(p + 14) == 0x0C00 && u16(p + 16) == 0xFFFE);
	CHECK(u16(p + n - 2) == 0x00FF);

	GWorldPtr view = world(W + 40, H + 30);
	Rect dst = {(short) u16(p + 2), (short) u16(p + 4), (short) u16(p + 6), (short) u16(p + 8)};   // big-endian picFrame
	DrawPicture(pic, &dst);
	CHECK(QDError() == noErr);
	int bad = 0;
	for (int b = 0; b < H; b++)
		for (int f = 0; f < W; f++) {
			if ((f >= 5 && f < 25 && b >= 5 && b < 15) || b == H - 4 || b == H - 3) continue;
			RGBColor c, w = ramp(f, b);
			GetCPixel(f, b, &c);
			if ((c.red >> 8) != (w.red >> 8) || (c.green >> 8) != (w.green >> 8) || (c.blue >> 8) != (w.blue >> 8)) bad++;
		}
	CHECK(bad == 0);
	RGBColor c;
	GetCPixel(10, 10, &c); CHECK(c.red == 0xFFFF && c.green == 0 && c.blue == 0);
	GetCPixel(W / 2, H - 4, &c); CHECK(c.red == 0 && c.green == 0 && c.blue == 0xFFFF);

	DisposeHandle((Handle) pic);
	DisposeGWorld(view); DisposeGWorld(off); DisposeGWorld(win);

	// Classic colours and the pen pattern are recorded; clip changes and regions are not recorded, so the replay does not show them.
	GWorldPtr g = world(32, 16);
	Rect all; SetRect(&all, 0, 0, 32, 16);
	CPParams cp2 = {all, 0x00480000, 0x00480000, -2, 0, 0};
	PicHandle pic2 = OpenCPicture(&cp2);
	CHECK(pic2 != NULL);
	ForeColor(205 /* redColor */);
	Rect a; SetRect(&a, 0, 0, 8, 8); PaintRect(&a);
	Pat gray; GetQDGlobalsGray(&gray);
	ForeColor(33 /* blackColor */); PenPat(&gray);
	Rect b2; SetRect(&b2, 8, 0, 16, 8); PaintRect(&b2);
	ClipRect(&all);
	RgnHandle rgn = NewRgn(); RectRgn(rgn, &a); PaintRgn(rgn); DisposeRgn(rgn);
	ClosePicture();
	if (pic2) {
		GWorldPtr v2 = world(32, 16);
		DrawPicture(pic2, &all);
		RGBColor c2;
		GetCPixel(2, 2, &c2); CHECK(c2.red >> 8 == 0xDD && c2.green >> 8 == 0x08 && c2.blue >> 8 == 0x06);   // Apple's redColor
		int blackCount = 0;
		for (int y = 0; y < 8; y++) for (int x = 8; x < 16; x++) { GetCPixel(x, y, &c2); blackCount += c2.red == 0 && c2.green == 0 && c2.blue == 0; }
		CHECK(blackCount == 32);   // 50% gray pattern
		int redCount = 0;   // PaintRgn over the red square is not recorded: the square stays red
		for (int y = 0; y < 8; y++) for (int x = 0; x < 8; x++) { GetCPixel(x, y, &c2); redCount += (c2.red >> 8) == 0xDD; }
		CHECK(redCount == 64);
		DisposeGWorld(v2);
		DisposeHandle((Handle) pic2);
	}
	DisposeGWorld(g);
	printf(g_failures ? "FAILED %d\n" : "PASSED\n", g_failures);
	return g_failures != 0;
}

#endif
