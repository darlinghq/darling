// Copyright (C) 2026 Lubos Dolezel
//
// QuickDraw text while a picture is open (32-bit only): the picture has no text opcodes, so DrawString, DrawText and
// DrawChar are not recorded in the picture (it has no text opcodes) and nothing reaches the port, because the pen is
// hidden while a picture is recorded; the pen does not move. The picture holds the same bytes as one recorded without the
// text calls, and playing it back draws the rectangle that was recorded but no text. After ClosePicture, text draws
// again.
#include <QD/QD.h>
#include <CarbonCore/MacMemory.h>
#include <stdio.h>
#include <string.h>

#ifdef __LP64__
int main(void)
{
	printf("PASSED (QuickDraw text in pictures: 32-bit only)\n");
	return 0;
}
#else

typedef OpenCPicParams CPParams;
void DrawPicture(PicHandle, const Rect *); void PaintRect(const Rect *);
void SetGWorld(CGrafPtr, GDHandle);
void RGBForeColor(const RGBColor *); void RGBBackColor(const RGBColor *);
void EraseRect(const Rect *); void GetCPixel(short, short, RGBColor *);
void MoveTo(short, short); void GetPen(Point *);
void TextFont(short); void TextSize(short);

static int g_failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); g_failures++; } } while (0)

static int inked(short left, short top, short right, short bottom)
{
	int n = 0;
	for (short y = top; y < bottom; y++)
		for (short x = left; x < right; x++) { RGBColor c; GetCPixel(x, y, &c); n += c.red != 0xFFFF || c.green != 0xFFFF || c.blue != 0xFFFF; }
	return n;
}

int main(void)
{
	Rect r; SetRect(&r, 0, 0, 120, 40);
	GWorldPtr g = NULL;
	CHECK(NewGWorld(&g, 32, &r, NULL, NULL, 0) == noErr && g);
	if (!g) return 1;
	SetGWorld(g, NULL);
	RGBColor white = {0xFFFF, 0xFFFF, 0xFFFF}, black = {0, 0, 0};
	RGBBackColor(&white); RGBForeColor(&black); EraseRect(&r);
	TextFont(1); TextSize(12);

	CPParams cp = {r, 0x00480000, 0x00480000, -2, 0, 0};
	Rect mark; SetRect(&mark, 100, 2, 116, 10);
	PicHandle plain = OpenCPicture(&cp);     // reference: only the rectangle
	CHECK(plain != NULL);
	PaintRect(&mark);
	ClosePicture();

	PicHandle pic = OpenCPicture(&cp);
	CHECK(pic != NULL);
	MoveTo(10, 25);
	DrawString("\pHello");
	DrawText("abc", 0, 3);
	DrawChar(65);
	PaintRect(&mark);
	Point pen; GetPen(&pen);
	CHECK(pen.h == 10 && pen.v == 25);
	ClosePicture();
	CHECK(inked(0, 0, 120, 40) == 0);
	if (!pic || !plain) return 1;
	Size n = GetHandleSize((Handle) pic);
	CHECK(n == GetHandleSize((Handle) plain) && memcmp(*pic, *plain, (size_t) n) == 0);   // no text opcodes

	DrawPicture(pic, &r);
	CHECK(inked(100, 2, 116, 10) == 16 * 8);   // the recorded rectangle
	CHECK(inked(0, 0, 100, 40) == 0);          // and no text
	DisposeHandle((Handle) pic); DisposeHandle((Handle) plain);
	EraseRect(&r);

	MoveTo(10, 25);
	DrawString("\pHello");
	CHECK(inked(0, 0, 120, 40) > 0);

	DisposeGWorld(g);
	printf(g_failures ? "FAILED: %d\n" : "PASSED\n", g_failures);
	return g_failures != 0;
}

#endif
