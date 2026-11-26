// Copyright (C) 2026 Lubos Dolezel
//
// The opacity of a fill or stroke colour, and the constant alpha of a graphics state, survive a PDF context: a
// semi-transparent fill is drawn semi-transparent when the PDF page is drawn again (PDF 1.4: /ca and /CA entries of a
// graphics state parameter dictionary).
#include <ApplicationServices/ApplicationServices.h>
#include <stdio.h>
#include <stdlib.h>

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static size_t put(void* info, const void* buffer, size_t count)
{
	CFDataAppendBytes((CFMutableDataRef) info, buffer, count);
	return count;
}

static void release(void* info)
{
}

// the page: 100x100, left third a red fill of colour alpha 0.5, middle third a red fill with constant alpha 0.5, right
// third opaque red; below them (y 0..40) a blue stroked box of colour alpha 0.5
static CFDataRef makePDF(void)
{
	CFMutableDataRef data = CFDataCreateMutable(NULL, 0);
	CGDataConsumerCallbacks callbacks = { put, release };
	CGDataConsumerRef consumer = CGDataConsumerCreate(data, &callbacks);
	CGRect box = CGRectMake(0, 0, 90, 100);
	CGContextRef pdf = CGPDFContextCreate(consumer, &box, NULL);

	CGPDFContextBeginPage(pdf, NULL);
	CGContextSetRGBFillColor(pdf, 1, 0, 0, 0.5);
	CGContextFillRect(pdf, CGRectMake(0, 60, 30, 40));

	CGContextSaveGState(pdf);
	CGContextSetAlpha(pdf, 0.5);
	CGContextSetRGBFillColor(pdf, 1, 0, 0, 1);
	CGContextFillRect(pdf, CGRectMake(30, 60, 30, 40));
	CGContextRestoreGState(pdf);

	CGContextSetRGBFillColor(pdf, 1, 0, 0, 1);
	CGContextFillRect(pdf, CGRectMake(60, 60, 30, 40));

	CGContextSetRGBStrokeColor(pdf, 0, 0, 1, 0.5);
	CGContextSetLineWidth(pdf, 20);
	CGContextStrokeRect(pdf, CGRectMake(20, 20, 50, 1));
	CGPDFContextEndPage(pdf);
	CGPDFContextClose(pdf);
	CGContextRelease(pdf);
	CGDataConsumerRelease(consumer);
	return data;
}

static unsigned char* g_pixels;
static int g_stride;

// the bitmap is 90x100 with the row of y = 99 first in memory
static void pixel(int x, int y, int* r, int* g, int* b)
{
	unsigned char* p = g_pixels + (99 - y) * g_stride + x * 4;

	*r = p[0];
	*g = p[1];
	*b = p[2];
}

int main(void)
{
	CFDataRef data = makePDF();
	CGDataProviderRef provider = CGDataProviderCreateWithCFData(data);
	CGPDFDocumentRef doc = CGPDFDocumentCreateWithProvider(provider);
	CGPDFPageRef page = doc ? CGPDFDocumentGetPage(doc, 1) : NULL;

	CHECK(page != NULL);
	if (!page)
		return 1;

	CGColorSpaceRef rgb = CGColorSpaceCreateDeviceRGB();
	CGContextRef c = CGBitmapContextCreate(NULL, 90, 100, 8, 360, rgb, kCGImageAlphaPremultipliedLast);
	CGContextSetRGBFillColor(c, 1, 1, 1, 1);
	CGContextFillRect(c, CGRectMake(0, 0, 90, 100));
	CGContextDrawPDFPage(c, page);
	g_pixels = CGBitmapContextGetData(c);
	g_stride = (int) CGBitmapContextGetBytesPerRow(c);

	int r, g, b;
	pixel(15, 80, &r, &g, &b);
	printf("colour alpha fill: %d %d %d\n", r, g, b);
	CHECK(r > 245 && g > 120 && g < 136 && b > 120 && b < 136);
	pixel(45, 80, &r, &g, &b);
	printf("constant alpha fill: %d %d %d\n", r, g, b);
	CHECK(r > 245 && g > 120 && g < 136 && b > 120 && b < 136);
	pixel(75, 80, &r, &g, &b);
	printf("opaque fill: %d %d %d\n", r, g, b);
	CHECK(r > 245 && g < 10 && b < 10);
	pixel(45, 20, &r, &g, &b);
	printf("stroke alpha: %d %d %d\n", r, g, b);
	CHECK(r > 120 && r < 136 && g > 120 && g < 136 && b > 245);

	printf(g_failures ? "FAILED: %d\n" : "PASS\n", g_failures);
	return g_failures != 0;
}
