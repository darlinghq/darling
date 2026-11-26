// Copyright (C) 2026 Lubos Dolezel
//
// A document of two pages printed to a PDF file (the destination type kPMDestinationFile of
// PMSessionSetDestination, which is how a job reaches no print queue): the calls that make a document in a
// session (begin and end document, begin and end page, the Core Graphics context of a page) and what the file
// has: the number of pages, and the size of each, which is the paper of the page format of the page in its
// orientation. The 32-bit API calls (PMSessionBeginDocument and the others, which call the dialog-less
// ones) are used in the 32-bit program.
#include <Carbon/Carbon.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void page(PMPrintSession session, PMPageFormat pageFormat, double gray)
{
	CGContextRef context = NULL;

#ifndef __LP64__
	CHECK(PMSessionBeginPage(session, pageFormat, NULL) == noErr);
#else
	CHECK(PMSessionBeginPageNoDialog(session, pageFormat, NULL) == noErr);
#endif
	CHECK(PMSessionGetCGGraphicsContext(session, &context) == noErr && context);
	if (context)
	{
		CGContextSetGrayFillColor(context, gray, 1);
		CGContextFillRect(context, CGRectMake(20, 20, 100, 100));
	}
#ifndef __LP64__
	CHECK(PMSessionEndPage(session) == noErr);
#else
	CHECK(PMSessionEndPageNoDialog(session) == noErr);
#endif
}

int main(void)
{
	PMPrintSession session = NULL;
	PMPageFormat portrait = NULL, landscape = NULL;
	PMPrintSettings settings = NULL;
	char dir[] = "/tmp/carbon_print_document.XXXXXX";
	char path[300];

	CHECK(mkdtemp(dir) != NULL);
	snprintf(path, sizeof(path), "%s/two-pages.pdf", dir);
	CHECK(PMCreateSession(&session) == noErr);
	CHECK(PMCreatePageFormat(&portrait) == noErr && PMSessionDefaultPageFormat(session, portrait) == noErr);
	CHECK(PMCreatePageFormat(&landscape) == noErr && PMSessionDefaultPageFormat(session, landscape) == noErr);
	CHECK(PMSetOrientation(landscape, kPMLandscape, false) == noErr);
	CHECK(PMSetScale(landscape, 50) == noErr);
	CHECK(PMSessionValidatePageFormat(session, landscape, NULL) == noErr);
	CHECK(PMCreatePrintSettings(&settings) == noErr && PMSessionDefaultPrintSettings(session, settings) == noErr);

	// the paper of the formats, which the pages of the document have
	PMPaper paper = NULL;
	double width = 0, height = 0;
	CHECK(PMGetPageFormatPaper(portrait, &paper) == noErr && paper);
	CHECK(PMPaperGetWidth(paper, &width) == noErr && PMPaperGetHeight(paper, &height) == noErr && width > 0 && height > width);

	CFURLRef url = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8*) path, strlen(path), false);
	CHECK(PMSessionSetDestination(session, settings, kPMDestinationFile, kPMDocumentFormatPDF, url) == noErr);
	PMDestinationType type = 0;
	CHECK(PMSessionGetDestinationType(session, settings, &type) == noErr && type == kPMDestinationFile);
	CFURLRef location = NULL;
	CFStringRef format = NULL;
	CHECK(PMSessionCopyDestinationLocation(session, settings, &location) == noErr && location && CFEqual(location, url));
	CHECK(PMSessionCopyDestinationFormat(session, settings, &format) == noErr && format && CFEqual(format, kPMDocumentFormatPDF));

	CGContextRef context = NULL;
	CHECK(PMSessionGetCGGraphicsContext(session, &context) == kPMOutOfScope);
#ifndef __LP64__
	CHECK(PMSessionBeginDocument(session, settings, portrait) == noErr);
	CHECK(PMSessionBeginDocument(session, settings, portrait) == kPMOutOfScope);
#else
	CHECK(PMSessionBeginCGDocumentNoDialog(session, settings, portrait) == noErr);
	CHECK(PMSessionBeginCGDocumentNoDialog(session, settings, portrait) == kPMOutOfScope);
#endif
	page(session, portrait, 0.2);
	page(session, landscape, 0.8);
#ifndef __LP64__
	CHECK(PMSessionEndDocument(session) == noErr);
#else
	CHECK(PMSessionEndDocumentNoDialog(session) == noErr);
#endif
	CHECK(PMSessionGetCGGraphicsContext(session, &context) == kPMOutOfScope);

	// the file
	CGPDFDocumentRef document = CGPDFDocumentCreateWithURL(url);
	CHECK(document != NULL);
	if (document)
	{
		CHECK(CGPDFDocumentGetNumberOfPages(document) == 2);
		CGRect first = CGPDFPageGetBoxRect(CGPDFDocumentGetPage(document, 1), kCGPDFMediaBox);
		CGRect second = CGPDFPageGetBoxRect(CGPDFDocumentGetPage(document, 2), kCGPDFMediaBox);
		CHECK(first.size.width == width && first.size.height == height);
		CHECK(second.size.width == height && second.size.height == width);
		CGPDFDocumentRelease(document);
	}

	// a job to a printer needs a queue to take it
	CHECK(PMSessionSetDestination(session, settings, kPMDestinationPrinter, NULL, NULL) == noErr);
#ifndef __LP64__
	CHECK(PMSessionBeginDocument(session, settings, portrait) == kPMNoDefaultPrinter);
#else
	CHECK(PMSessionBeginCGDocumentNoDialog(session, settings, portrait) == kPMNoDefaultPrinter);
#endif

	unlink(path);
	rmdir(dir);
	printf(g_failures ? "FAIL carbon_print_document\n" : "PASS carbon_print_document\n");
	return g_failures != 0;
}
