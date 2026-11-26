// Copyright (C) 2026 Lubos Dolezel
//
// A PDF context that is released without CGPDFContextClose() finalizes its document: the documented way of finishing a PDF
// context is CGContextRelease() (CGPDFContextClose() is the explicit one). The data written is a complete PDF: it ends with
// %%EOF and has the pages that were drawn. Closing explicitly and then releasing also gives a complete document.
#include <ApplicationServices/ApplicationServices.h>
#include <stdio.h>
#include <string.h>

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static size_t putBytes(void* info, const void* buffer, size_t count)
{
	CFDataAppendBytes((CFMutableDataRef) info, buffer, (CFIndex) count);
	return count;
}

static void releaseInfo(void* info) {}

enum { kClose = 1, kEndPage = 2 };

static CFMutableDataRef makePDF(int flags)
{
	CFMutableDataRef data = CFDataCreateMutable(NULL, 0);
	CGDataConsumerCallbacks callbacks = { putBytes, releaseInfo };
	CGDataConsumerRef consumer = CGDataConsumerCreate(data, &callbacks);
	CGRect box = CGRectMake(0, 0, 100, 100);
	CGContextRef pdf = CGPDFContextCreate(consumer, &box, NULL);

	CGPDFContextBeginPage(pdf, NULL);
	CGContextSetRGBStrokeColor(pdf, 0, 0, 0, 1);
	CGContextSetLineWidth(pdf, 4);
	CGContextStrokeRect(pdf, CGRectMake(20, 20, 60, 40));
	if (flags & kEndPage)
		CGPDFContextEndPage(pdf);
	if (flags & kClose)
		CGPDFContextClose(pdf);
	CGContextRelease(pdf);
	CGDataConsumerRelease(consumer);
	return data;
}

static void runCase(int flags)
{
	CFMutableDataRef data = makePDF(flags);
	CFIndex length = CFDataGetLength(data);
	const char* bytes = (const char*) CFDataGetBytePtr(data);

	CHECK(length > 100);
	CHECK(length >= 6 && memcmp(bytes + length - 6, "%%EOF\n", 6) == 0);

	CGDataProviderRef provider = CGDataProviderCreateWithCFData(data);
	CGPDFDocumentRef document = CGPDFDocumentCreateWithProvider(provider);

	CHECK(document != NULL);
	if (document)
	{
		CHECK(CGPDFDocumentGetNumberOfPages(document) == 1);
		CGPDFDocumentRelease(document);
	}
	CGDataProviderRelease(provider);
	CFRelease(data);
}

int main(void)
{
	runCase(kEndPage);			// released after the page ended: finalized by the release
	runCase(kEndPage | kClose);	// closed explicitly, then released
	runCase(0);					// released with the page still open: the page is part of the document
	printf(g_failures ? "FAIL coregraphics_pdf_context_release\n" : "PASS coregraphics_pdf_context_release\n");
	return g_failures != 0;
}
