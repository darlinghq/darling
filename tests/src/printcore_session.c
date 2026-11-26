// Copyright (C) 2026 Lubos Dolezel
//
// The Printing Manager objects: print session, page format and print settings, and a document made in
// a session to a PDF file. Values and error codes are from the contract of the calls (PrintCore headers)
// and from black-box observation on macOS 10.14.4 i386 (marked QUIRK).
#include <ApplicationServices/ApplicationServices.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

int main(void)
{
	PMPrintSession session = NULL;
	PMPageFormat format = NULL;
	PMPrintSettings settings = NULL;
	PMRect rect;
	UInt32 first, last, minPage, maxPage, copies;
	Boolean changed;
	char path[] = "/tmp/printcore_session.XXXXXX";
	int fd = mkstemp(path);

	close(fd);
	CHECK(PMCreateSession(&session) == noErr && session);
	CHECK(PMSessionError(session) == noErr);
	CHECK(PMCreateSession(NULL) == kPMInvalidParameter);

	CHECK(PMCreatePageFormat(&format) == noErr && format);
	// QUIRK: the rectangles of a page format that has not been made valid by PMSessionDefaultPageFormat
	// fail with -9583 and leave the rectangle alone. Not documented. Evidence: black-box observation on
	// macOS 10.14.4 i386 (tier 2). Test: this test (test_quirk_unvalidated_format).
	memset(&rect, 0, sizeof(rect));
	CHECK(PMGetAdjustedPageRect(format, &rect) == -9583 && rect.bottom == 0);
	PMPageFormat fresh;
	CHECK(PMCreatePageFormat(&fresh) == noErr && PMSessionValidatePageFormat(session, fresh, &changed) == -9583);
	PMRelease(fresh);
	CHECK(PMSessionDefaultPageFormat(session, format) == noErr);
	CHECK(PMGetAdjustedPageRect(format, &rect) == noErr && rect.top == 0 && rect.left == 0 && rect.bottom > rect.right && rect.right > 500);
	PMRect paper;
	CHECK(PMGetAdjustedPaperRect(format, &paper) == noErr && paper.bottom >= rect.bottom && paper.right >= rect.right);
	PMOrientation orientation = 0;
	double scale = 0;
	CHECK(PMGetOrientation(format, &orientation) == noErr && orientation == kPMPortrait);
	CHECK(PMGetScale(format, &scale) == noErr && scale == 100);
	changed = true;
	CHECK(PMSessionValidatePageFormat(session, format, &changed) == noErr && !changed);

	// Landscape swaps the sides, a scale of 50 percent doubles the page in the page's units
	PMRect portrait = rect, landscape;
	// QUIRK: the orientation and the scale show in the rectangles after PMSessionValidatePageFormat only (see
	// PMSession.c; test_quirk_orientation_after_validate)
	CHECK(PMSetOrientation(format, kPMLandscape, false) == noErr);
	CHECK(PMSetOrientation(format, 9, false) == paramErr);
	CHECK(PMGetAdjustedPageRect(format, &landscape) == noErr && landscape.right == portrait.right && landscape.bottom == portrait.bottom);
	CHECK(PMSessionValidatePageFormat(session, format, &changed) == noErr);
	CHECK(PMGetAdjustedPageRect(format, &landscape) == noErr && landscape.right == portrait.bottom && landscape.bottom == portrait.right);
	CHECK(PMSetOrientation(format, kPMPortrait, false) == noErr);
	CHECK(PMSetScale(format, 50) == noErr);
	CHECK(PMGetAdjustedPageRect(format, &rect) == noErr && rect.bottom == landscape.bottom);
	CHECK(PMSessionValidatePageFormat(session, format, &changed) == noErr);
	CHECK(PMGetAdjustedPageRect(format, &rect) == noErr && rect.bottom == portrait.bottom * 2);
	CHECK(PMGetUnadjustedPageRect(format, &rect) == noErr && rect.bottom == portrait.bottom);
	CHECK(PMSetScale(format, 100) == noErr);
	CHECK(PMSessionValidatePageFormat(session, format, &changed) == noErr);
	CHECK(PMGetAdjustedPageRect(NULL, &rect) == kPMInvalidPageFormat);

	CHECK(PMCreatePrintSettings(&settings) == noErr && settings);
	CHECK(PMSessionDefaultPrintSettings(session, settings) == noErr);
	CHECK(PMGetFirstPage(settings, &first) == noErr && first == 1);
	CHECK(PMGetLastPage(settings, &last) == noErr && last == 2147483647);
	CHECK(PMGetPageRange(settings, &minPage, &maxPage) == noErr && minPage == 1 && maxPage == 2147483647);
	CHECK(PMGetCopies(settings, &copies) == noErr && copies == 1);
	CHECK(PMSetPageRange(settings, 1, 5) == noErr);
	CHECK(PMGetPageRange(settings, &minPage, &maxPage) == noErr && minPage == 1 && maxPage == 5);
	// QUIRK: the first and the last page must be inside the page range, else kPMValueOutOfRange; setting the
	// page range does not move the last page. Not documented. Evidence: black-box observation on macOS 10.14.4
	// i386 (tier 2). Test: this test (test_quirk_first_last_in_range).
	CHECK(PMGetLastPage(settings, &last) == noErr && last == 2147483647);
	CHECK(PMSetFirstPage(settings, 2, false) == noErr && PMGetFirstPage(settings, &first) == noErr && first == 2);
	CHECK(PMSetLastPage(settings, 99, false) == kPMValueOutOfRange);
	CHECK(PMSetFirstPage(settings, 0, false) == kPMValueOutOfRange);
	CHECK(PMSetLastPage(settings, 4, false) == noErr && PMGetLastPage(settings, &last) == noErr && last == 4);
	CHECK(PMSetPageRange(settings, 3, 2) == kPMValueOutOfRange);
	CHECK(PMSetCopies(settings, 3, false) == noErr && PMGetCopies(settings, &copies) == noErr && copies == 3);

	CFStringRef name = NULL;
	CHECK(PMSetJobNameCFString(settings, CFSTR("a job")) == noErr);
	CHECK(PMPrintSettingsGetJobName(settings, &name) == noErr && name && CFStringCompare(name, CFSTR("a job"), 0) == kCFCompareEqualTo);

	// The default destination is a printer, and there is no queue to print to
	PMDestinationType destination = 99;
	CHECK(PMSessionGetDestinationType(session, settings, &destination) == noErr && destination == kPMDestinationPrinter);
	CHECK(PMSessionBeginCGDocumentNoDialog(session, settings, format) == kPMNoDefaultPrinter);
	// QUIRK: a failed call leaves the error of the session at noErr (test_quirk_session_error_stays)
	CHECK(PMSessionError(session) == noErr);
	CHECK(PMSessionBeginCGDocumentNoDialog(session, NULL, format) == kPMInvalidPrintSettings);
	CHECK(PMSessionSetDestination(session, settings, 77, kPMDocumentFormatPDF, NULL) == kPMValueOutOfRange);
	CHECK(PMSessionSetDocumentFormatGeneration(session, CFSTR("application/postscript"), NULL, NULL) == paramErr);

	// A document to a PDF file, on a session of its own
	PMPrintSession pdfSession;
	CHECK(PMCreateSession(&pdfSession) == noErr);
	CFURLRef url = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8*) path, strlen(path), false);
	CHECK(PMSessionSetDestination(pdfSession, settings, kPMDestinationFile, kPMDocumentFormatPDF, url) == noErr);
	CHECK(PMSessionGetDestinationType(pdfSession, settings, &destination) == noErr && destination == kPMDestinationFile);
	CFStringRef types[1] = { kPMGraphicsContextCoreGraphics };
	CFArrayRef typeArray = CFArrayCreate(NULL, (const void**) types, 1, &kCFTypeArrayCallBacks);
	CHECK(PMSessionSetDocumentFormatGeneration(pdfSession, kPMDocumentFormatPDF, typeArray, NULL) == noErr);
	CGContextRef context = NULL;
	CHECK(PMSessionGetCGGraphicsContext(pdfSession, &context) == kPMOutOfScope);
	CHECK(PMSessionBeginPageNoDialog(pdfSession, format, NULL) == kPMOutOfScope);
	PMPrintSession cleanSession;
	CHECK(PMCreateSession(&cleanSession) == noErr);
	CHECK(PMSessionEndDocumentNoDialog(cleanSession) == kPMOutOfScope);
	PMRelease(cleanSession);
	CHECK(PMSessionBeginCGDocumentNoDialog(pdfSession, settings, format) == noErr);
	CHECK(PMSessionBeginPageNoDialog(pdfSession, format, NULL) == noErr);
	CHECK(PMSessionGetCGGraphicsContext(pdfSession, &context) == noErr && context);
	void* generic = NULL;
	CHECK(PMSessionGetGraphicsContext(pdfSession, kPMGraphicsContextCoreGraphics, &generic) == noErr && generic == context);
	if (context)
		CGContextFillRect(context, CGRectMake(10, 10, 50, 50));
	CHECK(PMSessionEndPageNoDialog(pdfSession) == noErr);
	CHECK(PMSessionEndPageNoDialog(pdfSession) == kPMOutOfScope);
	CHECK(PMSessionEndDocumentNoDialog(pdfSession) == noErr);

	FILE* file = fopen(path, "rb");
	char magic[5] = { 0 };
	CHECK(file != NULL);
	if (file)
	{
		CHECK(fread(magic, 1, 4, file) == 4 && memcmp(magic, "%PDF", 4) == 0);
		fclose(file);
	}
	unlink(path);

	CHECK(PMRelease(NULL) == noErr);
	CHECK(PMRelease(settings) == noErr);
	CHECK(PMRelease(format) == noErr);
	CHECK(PMRelease(pdfSession) == noErr);
	CHECK(PMRelease(session) == noErr);
	CFRelease(url);
	CFRelease(typeArray);

	printf(g_failures ? "FAIL printcore_session\n" : "PASS printcore_session\n");
	return g_failures != 0;
}
