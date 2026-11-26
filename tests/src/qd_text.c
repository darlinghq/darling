// Copyright (C) 2026 fr4nck
//
// QuickDraw text (32-bit only): DrawString draws in the foreground color from the pen location, on the baseline, and
// moves the pen by StringWidth; nothing is drawn outside the text's box; srcCopy fills the box with the background
// color; bold text is wider; TextWidth, CharWidth, GetFontInfo and GetFNum agree. (QuickDraw Text: DrawText,
// TextWidth, GetFontInfo, GetFNum.)
#include <QD/QD.h>
#include <stdio.h>
#include <string.h>

#ifdef __LP64__
int main(void)
{
	printf("PASSED (QuickDraw text: 32-bit only)\n");
	return 0;
}
#else

void SetGWorld(CGrafPtr, GDHandle);
void RGBForeColor(const RGBColor *); void RGBBackColor(const RGBColor *);
void EraseRect(const Rect *); void GetCPixel(short, short, RGBColor *);
void MoveTo(short, short); void GetPen(Point *);
void TextFont(short); void TextSize(short); void TextFace(short); void TextMode(short);

static int g_failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); g_failures++; } } while (0)

static const RGBColor white = {0xFFFF, 0xFFFF, 0xFFFF}, black = {0, 0, 0}, red = {0xFFFF, 0, 0};

static int is(RGBColor c, RGBColor w) { return (c.red >> 8) == (w.red >> 8) && (c.green >> 8) == (w.green >> 8) && (c.blue >> 8) == (w.blue >> 8); }

// Pixels that are not white in [left,right) x [top,bottom)
static int inked(short left, short top, short right, short bottom)
{
	int n = 0;
	for (short y = top; y < bottom; y++)
		for (short x = left; x < right; x++) { RGBColor c; GetCPixel(x, y, &c); n += !is(c, white); }
	return n;
}

int main(void)
{
	Rect r; SetRect(&r, 0, 0, 200, 60);
	GWorldPtr g = NULL;
	CHECK(NewGWorld(&g, 32, &r, NULL, NULL, 0) == noErr && g);
	if (!g) return 1;
	SetGWorld(g, NULL);
	RGBBackColor(&white); EraseRect(&r);
	TextFont(1); TextSize(12);

	FontInfo fi; GetFontInfo(&fi);
	CHECK(fi.ascent > 0 && fi.descent > 0 && fi.widMax > 0);
	short w = StringWidth("\pHello");
	CHECK(w > 0);
	CHECK(TextWidth("Hello", 0, 5) == w);
	CHECK(StringWidth("\pWWWW") > StringWidth("\piiii"));
	CHECK(CharWidth('W') > 0);

	// Drawn in the foreground color, from the pen, and the pen moves by the width.
	RGBForeColor(&red);
	MoveTo(10, 30);
	DrawString("\pHello");
	Point pen; GetPen(&pen);
	CHECK(pen.h == 10 + w && pen.v == 30);
	int inside = inked(10, (short) (30 - fi.ascent), (short) (10 + w + 1), (short) (30 + fi.descent));
	CHECK(inside > 20);
	CHECK(inked(0, 0, 200, (short) (30 - fi.ascent - 1)) == 0);       // nothing above the line
	CHECK(inked((short) (10 + w + 2), 0, 200, 60) == 0);                // nothing after the text
	int reddish = 0;
	for (short y = (short) (30 - fi.ascent); y < 30 + fi.descent; y++)
		for (short x = 10; x < 10 + w; x++) { RGBColor c; GetCPixel(x, y, &c); reddish += c.red > 0xC000 && c.green < 0x4000 && c.blue < 0x4000; }
	CHECK(reddish > 5);

	// Bold is wider.
	TextFace(1);
	CHECK(StringWidth("\pHello") > w);
	TextFace(0);

	// srcCopy fills the text box with the background color.
	RGBBackColor(&black); RGBForeColor(&white);
	TextMode(0 /* srcCopy */);
	MoveTo(10, 55);
	DrawString("\pii");
	RGBColor c; GetCPixel(11, (short) (55 - 1), &c);
	int dark = 0;
	for (short x = 10; x < 10 + StringWidth("\pii"); x++) { GetCPixel(x, (short) (55 - fi.ascent + 1), &c); dark += is(c, black); }
	CHECK(dark > 0);

	short id = -1;
	GetFNum("\pMonaco", &id); CHECK(id == 4);
	GetFNum("\pUnknown Family", &id); CHECK(id == 0);

	DisposeGWorld(g);
	printf(g_failures ? "FAILED: %d\n" : "PASSED\n", g_failures);
	return g_failures != 0;
}

#endif
