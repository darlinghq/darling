// QuickDraw drawing on GWorld pixel memory (32-bit only). Every check reads back real pixels
// or state. Build for i386 against QD and run with `darling shell`. Exits non-zero on failure.
#include <QD/QD.h>
#include <CarbonCore/MacMemory.h>
#include <stdio.h>
#include <string.h>

#pragma pack(push, 2)
typedef struct { UInt8 pat[8]; } Pat;
typedef struct { Point pnLoc; Point pnSize; short pnMode; Pat pnPat; } PenState;
typedef struct { SInt16 data[16]; SInt16 mask[16]; Point hotSpot; } Cursor;
typedef struct { UInt16 hue, saturation, value; } HSV;
#pragma pack(pop)

void RGBForeColor(const RGBColor *); void RGBBackColor(const RGBColor *);
void PaintRect(const Rect *); void EraseRect(const Rect *); void FrameRect(const Rect *);
void InvertRect(const Rect *); void FillRect(const Rect *, const Pat *);
void PenSize(short, short); void PenNormal(void); void MoveTo(short, short); void LineTo(short, short);
void ClipRect(const Rect *); void GetCPixel(short, short, RGBColor *);
void GetPenState(PenState *); void HidePen(void); void ShowPen(void);
Pat *GetQDGlobalsGray(Pat *); Cursor *GetQDGlobalsArrow(Cursor *);
void HSV2RGB(const HSV *, RGBColor *);
const BitMap *GetPortBitMapForCopyBits(CGrafPtr);
void CopyBits(const BitMap *, const BitMap *, const Rect *, const Rect *, short, RgnHandle);
void ScrollRect(const Rect *, short, short, RgnHandle);
GWorldFlags UpdateGWorld(GWorldPtr *, short, const Rect *, CTabHandle, GDHandle, GWorldFlags);
void GetGWorld(CGrafPtr *, GDHandle *); void SetGWorld(CGrafPtr, GDHandle);
void ForeColor(long); void GetForeColor(RGBColor *);
void TextFont(short); short GetPortTextFont(CGrafPtr);

static int g_failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL %d: %s\n", __LINE__, #c); g_failures++; } } while (0)

static const RGBColor red = {0xFFFF, 0, 0}, blue = {0, 0, 0xFFFF}, white = {0xFFFF, 0xFFFF, 0xFFFF}, black = {0, 0, 0};

static int same(short h, short v, RGBColor want)
{
	RGBColor c;
	GetCPixel(h, v, &c);
	return (c.red >> 8) == (want.red >> 8) && (c.green >> 8) == (want.green >> 8) && (c.blue >> 8) == (want.blue >> 8);
}

static GWorldPtr world(short w, short h)
{
	Rect r; SetRect(&r, 0, 0, w, h);
	GWorldPtr g = NULL;
	CHECK(NewGWorld(&g, 32, &r, NULL, NULL, 0) == noErr && g != NULL);
	SetPort(g);
	RGBBackColor(&white);
	EraseRect(&r);
	return g;
}

int main(void)
{
	GWorldPtr g = world(32, 32);
	Rect r;

	// Paint and erase.
	SetRect(&r, 4, 4, 8, 8);
	RGBForeColor(&red);
	PaintRect(&r);
	CHECK(same(4, 4, red) && same(7, 7, red));
	CHECK(same(8, 8, white) && same(3, 4, white));
	EraseRect(&r);
	CHECK(same(5, 5, white));

	// Frame with a 2x2 pen: the outline is inside the rectangle.
	PenSize(2, 2);
	SetRect(&r, 10, 10, 20, 20);
	FrameRect(&r);
	CHECK(same(10, 10, red) && same(11, 11, red) && same(19, 19, red));
	CHECK(same(12, 12, white) && same(20, 20, white));
	PenNormal();

	// A line draws every point; the pen hangs below and right.
	MoveTo(0, 25); LineTo(5, 30);
	CHECK(same(0, 25, red) && same(3, 28, red) && same(5, 30, red) && same(1, 25, white));
	HidePen(); MoveTo(0, 31); LineTo(5, 31); ShowPen();
	CHECK(same(2, 31, white));
	PenState ps; GetPenState(&ps);
	CHECK(ps.pnLoc.h == 5 && ps.pnLoc.v == 31 && ps.pnSize.h == 1 && ps.pnMode == 8);

	// Clipping restricts painting.
	SetRect(&r, 0, 0, 2, 2); ClipRect(&r);
	SetRect(&r, 0, 0, 32, 32); RGBForeColor(&blue); PaintRect(&r);
	CHECK(same(1, 1, blue) && same(2, 2, white) && same(10, 10, red));
	ClipRect(&r);

	// Patterns: gray alternates fore and back colors.
	Pat gray; GetQDGlobalsGray(&gray);
	SetRect(&r, 24, 0, 32, 8); FillRect(&r, &gray);
	CHECK(same(24, 0, blue) && same(25, 0, white) && same(24, 1, white) && same(25, 1, blue));

	// Invert, classic colors.
	SetRect(&r, 30, 30, 32, 32); InvertRect(&r);
	CHECK(same(30, 30, black));
	ForeColor(33); RGBColor fg; GetForeColor(&fg); CHECK(fg.red == 0 && fg.green == 0 && fg.blue == 0);

	// CopyBits between worlds, scaled 2x.
	GWorldPtr src = world(2, 2);
	SetRect(&r, 0, 0, 1, 1); RGBForeColor(&red); PaintRect(&r);
	SetPort(g);
	Rect from, to; SetRect(&from, 0, 0, 2, 2); SetRect(&to, 12, 0, 16, 4);
	CopyBits(GetPortBitMapForCopyBits(src), GetPortBitMapForCopyBits(g), &from, &to, 0 /* srcCopy */, NULL);
	CHECK(QDError() == noErr);
	CHECK(same(12, 0, red) && same(13, 1, red) && same(14, 0, white) && same(14, 2, white));
	CopyBits(GetPortBitMapForCopyBits(src), GetPortBitMapForCopyBits(g), &from, &to, 49 /* unsupported */, NULL);
	CHECK(QDError() == paramErr);

	// ScrollRect moves pixels and reports the uncovered strip.
	RgnHandle upd = NewRgn();
	SetRect(&r, 12, 0, 16, 4); ScrollRect(&r, 1, 0, upd);
	Rect ub; GetRegionBounds(upd, &ub);
	CHECK(same(13, 0, red) && same(12, 0, white));
	CHECK(ub.left == 12 && ub.right == 13 && ub.top == 0 && ub.bottom == 4);
	DisposeRgn(upd);

	// UpdateGWorld keeps the overlapping pixels with clipPix.
	GWorldPtr w = src;
	Rect big; SetRect(&big, 0, 0, 4, 4);
	GWorldFlags f = UpdateGWorld(&w, 32, &big, NULL, NULL, 1L << 28);
	CHECK(f == 1 && w != NULL);
	SetPort(w); CHECK(same(0, 0, red));
	CGrafPtr cur; GDHandle dev; GetGWorld(&cur, &dev); CHECK(cur == w);
	SetGWorld(g, NULL); GetGWorld(&cur, &dev); CHECK(cur == g);


	// State that has no drawing here is still recorded per port.
	TextFont(4); CHECK(GetPortTextFont(g) == 4);

	// HSV and the arrow cursor.
	HSV hsv = {0, 65535, 65535}; RGBColor out; HSV2RGB(&hsv, &out);
	CHECK(out.red == 0xFFFF && out.green == 0 && out.blue == 0);
	Cursor arrow; GetQDGlobalsArrow(&arrow); CHECK(arrow.hotSpot.h == 1 && arrow.hotSpot.v == 1 && arrow.data[1] == 0x4000);

	DisposeGWorld(w);
	DisposeGWorld(g);
	printf("%s\n", g_failures ? "FAILED" : "PASSED");
	return g_failures != 0;
}
