// QuickDraw version 2 picture playback into a GWorld (32-bit only). Every check reads back real pixels
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
void DrawPicture(Handle, const Rect *);
void ForeColor(long); void GetForeColor(RGBColor *);
void TextFont(short); short GetPortTextFont(CGrafPtr);

static int g_failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL %d: %s\n", __LINE__, #c); g_failures++; } } while (0)

static const RGBColor red = {0xFFFF, 0, 0}, blue = {0, 0, 0xFFFF}, white = {0xFFFF, 0xFFFF, 0xFFFF};

static int same(short h, short v, RGBColor want)
{
	RGBColor c;
	GetCPixel(h, v, &c);
	return (c.red >> 8) == (want.red >> 8) && (c.green >> 8) == (want.green >> 8) && (c.blue >> 8) == (want.blue >> 8);
}

int main(void)
{
	Rect r, b = {0, 0, 32, 32};
	GWorldPtr g = NULL;
	CHECK(NewGWorld(&g, 32, &b, NULL, NULL, 0) == noErr);
	SetPort(g);
	RGBBackColor(&white);
	EraseRect(&b);
	// A version 2 picture: one 2x1 DirectBitsRect, unpacked 32-bit (rowBytes < 8).
	static const unsigned char pict[] = {
		0x00, 0x00, 0, 0, 0, 0, 0, 1, 0, 2,            // size, frame (0,0,1,2)
		0x00, 0x11, 0x02, 0xFF,                         // version 2
		0x0C, 0x00, 0xFF, 0xFE, 0, 0, 0, 0x48, 0, 0, 0, 0x48, 0, 0, 0, 0, 0, 0, 0, 1, 0, 2, 0, 0, 0, 0,
		0x00, 0x9A, 0, 0, 0, 0xFF,                      // DirectBitsRect, baseAddr
		0x80, 0x08, 0, 0, 0, 0, 0, 1, 0, 2,            // rowBytes 8, bounds
		0, 0, 0, 1, 0, 0, 0, 0, 0, 0x48, 0, 0, 0, 0x48, 0, 0, 0, 16, 0, 32, 0, 3, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 2,                        // srcRect
		0, 0, 0, 0, 0, 1, 0, 2,                        // dstRect
		0, 0,                                           // mode
		0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, // red, blue (xRGB)
		0x00, 0xFF };
	Handle h = NewHandle(sizeof(pict));
	memcpy(*h, pict, sizeof(pict));
	SetRect(&r, 20, 20, 22, 21);
	DrawPicture(h, &r);
	CHECK(QDError() == noErr);
	CHECK(same(20, 20, red) && same(21, 20, blue));
	(*h)[sizeof(pict) - 1] = 0x77; // unknown opcode instead of OpEndPic
	(*h)[sizeof(pict) - 2] = 0x77;
	DrawPicture(h, &r);
	CHECK(QDError() == -11005);
	DisposeHandle(h);
	CHECK(same(19, 20, white));
	DisposeGWorld(g);
	printf("%s\n", g_failures ? "FAILED" : "PASSED");
	return g_failures != 0;
}
