// Copyright (C) 2026 Lubos Dolezel
//
// The Printing Manager objects: papers, page formats and print settings. The values and the error codes
// are the ones the calls of the PrintCore headers promise or that black-box observation of the calls on
// macOS 10.14.4 and 26.7.3 gave (QUIRK in PrintCore, marked in the checks): the page and paper rectangles
// that have the margins of the paper, turned with the orientation and scaled; the generic printer and its
// papers; the copies of the objects; their data; the keys of the print settings.
#include <ApplicationServices/ApplicationServices.h>
#include <stdio.h>
#include <string.h>

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static int same(PMRect r, double t, double l, double b, double rt)
{
	return r.top == t && r.left == l && r.bottom == b && r.right == rt;
}

static int equal(CFStringRef s, const char* c)
{
	return s && CFStringCompare(s, CFStringCreateWithCString(NULL, c, kCFStringEncodingUTF8), 0) == kCFCompareEqualTo;
}

int main(void)
{
	PMPrintSession session = NULL;
	PMPageFormat format = NULL, copy = NULL;
	PMPrintSettings settings = NULL, settingsCopy = NULL;
	PMPaper paper = NULL, custom = NULL;
	PMPrinter printer = NULL;
	PMRect rect;
	PMPaperMargins margins;
	CFArrayRef papers = NULL;
	CFStringRef string = NULL;
	double value;
	Boolean changed, flag;
	UInt32 u, u2;

	CHECK(PMCreateSession(&session) == noErr);

	// ---- the generic printer and its papers
	CHECK(PMSessionGetCurrentPrinter(session, &printer) == noErr && printer);
	CHECK(PMSessionGetCurrentPrinter(NULL, &printer) == kPMInvalidPrintSession);
	CHECK(PMSessionGetCurrentPrinter(session, NULL) == kPMInvalidParameter);
	CHECK(PMPrinterGetPaperList(printer, &papers) == noErr && papers && CFArrayGetCount(papers) == 14);
	CHECK(PMPrinterGetPaperList(NULL, &papers) == kPMInvalidParameter);
	CHECK(!PMPrinterIsDefault(printer));
	CFArrayRef printers = NULL;
	CFIndex current = 7;
	CHECK(PMSessionCreatePrinterList(session, &printers, &current, NULL) == noErr && printers && CFArrayGetCount(printers) == 0 && current == -1);
	// The first papers of the list, with the margins that they have on the printer without a queue
	paper = (PMPaper) CFArrayGetValueAtIndex(papers, 0);
	CHECK(PMPaperGetID(paper, &string) == noErr && equal(string, "na-letter"));
	CHECK(PMPaperGetPPDPaperName(paper, &string) == noErr && equal(string, "Letter"));
	CHECK(PMPaperGetWidth(paper, &value) == noErr && value == 612);
	CHECK(PMPaperGetHeight(paper, &value) == noErr && value == 792);
	CHECK(PMPaperGetMargins(paper, &margins) == noErr && margins.top == 18 && margins.left == 18 && margins.bottom == 40 && margins.right == 18);
	CHECK(!PMPaperIsCustom(paper));
	CHECK(PMPaperCreateLocalizedName(paper, printer, &string) == noErr && equal(string, "US Letter"));
	CHECK(PMPaperCreateLocalizedName(paper, NULL, &string) == kPMInvalidParameter);
	paper = (PMPaper) CFArrayGetValueAtIndex(papers, 2);
	CHECK(PMPaperGetID(paper, &string) == noErr && equal(string, "iso-a4"));
	CHECK(PMPaperGetMargins(paper, &margins) == noErr && margins.bottom == 41);
	CHECK(PMPaperGetWidth(NULL, &value) == kPMInvalidParameter);
	CHECK(PMPaperGetWidth(paper, NULL) == kPMInvalidParameter);

	// ---- a custom paper has no name in the list of a printer
	PMPaperMargins cm = { 10, 20, 30, 40 };
	CHECK(PMPaperCreateCustom(NULL, CFSTR("my.custom"), CFSTR("My Paper"), 300, 400, &cm, &custom) == noErr && custom);
	CHECK(PMPaperGetWidth(custom, &value) == noErr && value == 300);
	CHECK(PMPaperGetHeight(custom, &value) == noErr && value == 400);
	CHECK(PMPaperGetMargins(custom, &margins) == noErr && margins.top == 10 && margins.left == 20 && margins.bottom == 30 && margins.right == 40);
	CHECK(PMPaperGetID(custom, &string) == noErr && equal(string, "my.custom"));
	CHECK(PMPaperGetPPDPaperName(custom, &string) == kPMNoSuchEntry);
	CHECK(PMPaperIsCustom(custom));
	// QUIRK: the sizes of a custom paper are not checked (test_quirk_custom_paper_unchecked)
	CHECK(PMPaperCreateCustom(NULL, CFSTR("a"), CFSTR("b"), 0, -5, &cm, &paper) == noErr);

	// ---- page formats: the rectangles have the margins, the orientation and the scale
	CHECK(PMCreatePageFormat(&format) == noErr);
	CHECK(PMGetPageFormatPaper(format, &paper) == noErr);
	CHECK(PMGetPageFormatPaper(NULL, &paper) == kPMInvalidParameter);
	CHECK(PMGetPageFormatPaper(format, NULL) == kPMInvalidParameter);
	CHECK(PMSessionDefaultPageFormat(session, format) == noErr);
	CHECK(PMGetPageFormatPaper(format, &paper) == noErr && paper);
	CHECK(PMPaperGetMargins(paper, &margins) == noErr && margins.top == 18 && margins.left == 18);
	CHECK(PMPaperGetWidth(paper, &value) == noErr && (value == 612 || value == 595));
	CHECK(PMGetUnadjustedPaperRect(format, &rect) == noErr && rect.top == -18 && rect.left == -18);
	CHECK(PMGetUnadjustedPageRect(format, &rect) == noErr && rect.top == 0 && rect.left == 0);

	CHECK(PMCreatePageFormatWithPMPaper(&copy, custom) == noErr && copy);
	CHECK(PMCreatePageFormatWithPMPaper(NULL, custom) == kPMInvalidParameter);
	CHECK(PMCreatePageFormatWithPMPaper(&format, NULL) == kPMInvalidParameter);
	PMRelease(format);
	format = copy;
	copy = NULL;
	PMOrientation orientation = 0;
	CHECK(PMGetOrientation(format, &orientation) == noErr && orientation == kPMPortrait);
	CHECK(PMGetScale(format, &value) == noErr && value == 100);
	// portrait: the page is the paper without the margins, which the paper rectangle goes out by
	CHECK(PMGetUnadjustedPaperRect(format, &rect) == noErr && same(rect, -10, -20, 390, 280));
	CHECK(PMGetUnadjustedPageRect(format, &rect) == noErr && same(rect, 0, 0, 360, 240));
	CHECK(PMGetAdjustedPaperRect(format, &rect) == noErr && same(rect, -10, -20, 390, 280));
	CHECK(PMGetAdjustedPageRect(format, &rect) == noErr && same(rect, 0, 0, 360, 240));

	// The orientation turns the margins with the page. QUIRK: it shows in the adjusted rectangles after the
	// validation only (test_quirk_orientation_after_validate); the unadjusted ones do not turn.
	CHECK(PMSetOrientation(format, kPMLandscape, false) == noErr);
	CHECK(PMGetAdjustedPageRect(format, &rect) == noErr && same(rect, 0, 0, 360, 240));
	CHECK(PMSessionValidatePageFormat(session, format, &changed) == noErr && !changed);
	CHECK(PMGetAdjustedPaperRect(format, &rect) == noErr && same(rect, -40, -10, 260, 390));
	CHECK(PMGetAdjustedPageRect(format, &rect) == noErr && same(rect, 0, 0, 240, 360));
	CHECK(PMGetUnadjustedPaperRect(format, &rect) == noErr && same(rect, -10, -20, 390, 280));
	CHECK(PMSetOrientation(format, kPMReversePortrait, false) == noErr && PMSessionValidatePageFormat(session, format, NULL) == noErr);
	CHECK(PMGetAdjustedPaperRect(format, &rect) == noErr && same(rect, -30, -40, 370, 260));
	CHECK(PMGetAdjustedPageRect(format, &rect) == noErr && same(rect, 0, 0, 360, 240));
	CHECK(PMSetOrientation(format, kPMReverseLandscape, false) == noErr && PMSessionValidatePageFormat(session, format, NULL) == noErr);
	CHECK(PMGetAdjustedPaperRect(format, &rect) == noErr && same(rect, -20, -30, 280, 370));
	CHECK(PMGetAdjustedPageRect(format, &rect) == noErr && same(rect, 0, 0, 240, 360));
	CHECK(PMSetOrientation(format, 0, false) == kPMInvalidParameter);
	CHECK(PMSetOrientation(format, 9, false) == kPMInvalidParameter);
	CHECK(PMGetOrientation(format, &orientation) == noErr && orientation == kPMReverseLandscape);

	// the scale divides the page units: at 50 percent the page has twice the points
	CHECK(PMSetOrientation(format, kPMLandscape, false) == noErr && PMSetScale(format, 50) == noErr);
	CHECK(PMSessionValidatePageFormat(session, format, NULL) == noErr);
	CHECK(PMGetAdjustedPaperRect(format, &rect) == noErr && same(rect, -80, -20, 520, 780));
	CHECK(PMGetAdjustedPageRect(format, &rect) == noErr && same(rect, 0, 0, 480, 720));
	CHECK(PMGetUnadjustedPageRect(format, &rect) == noErr && same(rect, 0, 0, 360, 240));
	// a scale that is no percentage is kept
	CHECK(PMSetScale(format, 5000) == noErr && PMGetScale(format, &value) == noErr && value == 5000);
	CHECK(PMSessionValidatePageFormat(session, format, &changed) == noErr && !changed && PMGetScale(format, &value) == noErr && value == 5000);

	// ---- copies of a page format
	CHECK(PMSetScale(format, 50) == noErr && PMSessionValidatePageFormat(session, format, NULL) == noErr);
	CHECK(PMCreatePageFormat(&copy) == noErr);
	CHECK(PMCopyPageFormat(format, copy) == noErr);
	CHECK(PMGetOrientation(copy, &orientation) == noErr && orientation == kPMLandscape);
	CHECK(PMGetAdjustedPageRect(copy, &rect) == noErr && same(rect, 0, 0, 480, 720));
	CHECK(PMGetPageFormatPaper(copy, &paper) == noErr && PMPaperGetWidth(paper, &value) == noErr && value == 300);
	CHECK(PMSetOrientation(copy, kPMPortrait, false) == noErr && PMGetOrientation(format, &orientation) == noErr && orientation == kPMLandscape);
	CHECK(PMCopyPageFormat(NULL, copy) == kPMInvalidPageFormat);
	CHECK(PMCopyPageFormat(format, NULL) == kPMInvalidPageFormat);

	// the data of a page format and of the application stored with it
	CFDataRef data = NULL;
	PMPageFormat back = NULL;
	CHECK(PMPageFormatCreateDataRepresentation(format, &data, kPMDataFormatXMLDefault) == noErr && data);
	CHECK(PMPageFormatCreateWithDataRepresentation(data, &back) == noErr && back);
	CHECK(PMGetOrientation(back, &orientation) == noErr && orientation == kPMLandscape);
	CHECK(PMGetAdjustedPageRect(back, &rect) == noErr && same(rect, 0, 0, 480, 720));
	CHECK(PMPageFormatCreateWithDataRepresentation(NULL, &back) == kPMInvalidParameter);
	CFDataRef junk = CFDataCreate(NULL, (const UInt8*) "junk", 4);
	PMPageFormat none = NULL;
	CHECK(PMPageFormatCreateWithDataRepresentation(junk, &none) == kPMInvalidObject && !none);
	UInt32 stored = 0x12345678, got = 0, size = sizeof(got);
	CHECK(PMSetPageFormatExtendedData(format, 'TEST', sizeof(stored), &stored) == noErr);
	CHECK(PMGetPageFormatExtendedData(format, 'TEST', &size, &got) == noErr && size == 4 && got == 0x12345678);
	size = 8;
	CHECK(PMGetPageFormatExtendedData(format, 'TEST', &size, &got) == noErr && size == 4);
	CHECK(PMGetPageFormatExtendedData(format, 'NONE', &size, &got) == -9589);
#ifndef __LP64__
	Handle flat = NULL;
	PMPageFormat unflat = NULL;
	CHECK(PMFlattenPageFormat(format, &flat) == noErr && flat && GetHandleSize(flat) > 0);
	CHECK(PMUnflattenPageFormat(flat, &unflat) == noErr && unflat && PMGetAdjustedPageRect(unflat, &rect) == noErr && same(rect, 0, 0, 480, 720));
	DisposeHandle(flat);
#endif

	// ---- print settings: the keys that the settings have
	CHECK(PMCreatePrintSettings(&settings) == noErr);
	// QUIRK: settings that no session has made default have none of the keys: the getters fail with
	// -9589 and zero the value (test_quirk_settings_keys)
	u = 7;
	CHECK(PMGetCopies(settings, &u) == -9589 && u == 0);
	CHECK(PMGetFirstPage(settings, &u) == -9589);
	CHECK(PMGetPageRange(settings, &u, &u2) == -9589);
	CHECK(PMGetCollate(settings, &flag) == -9589);
	PMDuplexMode duplex = 5;
	CHECK(PMGetDuplex(settings, &duplex) == -9589);
	CHECK(PMSessionDefaultPrintSettings(session, settings) == noErr);
	CHECK(PMGetCopies(settings, &u) == noErr && u == 1);
	CHECK(PMGetFirstPage(settings, &u) == noErr && u == 1);
	CHECK(PMGetLastPage(settings, &u) == noErr && u == 2147483647);
	CHECK(PMGetPageRange(settings, &u, &u2) == noErr && u == 1 && u2 == 2147483647);
	CHECK(PMGetCollate(settings, &flag) == noErr && flag);
	CHECK(PMGetDuplex(settings, &duplex) == -9589);
	CHECK(PMSetCollate(settings, false) == noErr && PMGetCollate(settings, &flag) == noErr && !flag);
	CHECK(PMSetDuplex(settings, kPMDuplexNoTumble) == noErr && PMGetDuplex(settings, &duplex) == noErr && duplex == kPMDuplexNoTumble);
	CHECK(PMGetCopies(NULL, &u) == kPMInvalidPrintSettings);
	CHECK(PMGetCopies(settings, NULL) == kPMInvalidParameter);
	// QUIRK: no copies is accepted, and the validation of the settings puts one in its place; so is a first
	// page after the last (test_quirk_validate_print_settings)
	CHECK(PMSetCopies(settings, 0, false) == noErr && PMGetCopies(settings, &u) == noErr && u == 0);
	CHECK(PMSessionValidatePrintSettings(session, settings, &changed) == noErr && changed && PMGetCopies(settings, &u) == noErr && u == 1);
	CHECK(PMSetPageRange(settings, 1, 10) == noErr && PMSetFirstPage(settings, 8, false) == noErr && PMSetLastPage(settings, 4, false) == noErr);
	CHECK(PMSessionValidatePrintSettings(session, settings, &changed) == noErr && changed);
	CHECK(PMGetFirstPage(settings, &u) == noErr && u == 1 && PMGetLastPage(settings, &u) == noErr && u == 2147483647);
	CHECK(PMSetFirstPage(settings, 20, false) == kPMValueOutOfRange);
	CHECK(PMSetPageRange(settings, 5, 3) == kPMValueOutOfRange);
	CHECK(PMSetPageRange(settings, 0, 3) == noErr);
	CHECK(PMSetPageRange(settings, 1, 10) == noErr);
	CHECK(PMSetCopies(settings, 3, false) == noErr && PMSetCollate(settings, true) == noErr);
	CHECK(PMPrintSettingsSetJobName(settings, CFSTR("a job")) == noErr);

	CHECK(PMCreatePrintSettings(&settingsCopy) == noErr);
	CHECK(PMCopyPrintSettings(settings, settingsCopy) == noErr);
	CHECK(PMGetCopies(settingsCopy, &u) == noErr && u == 3);
	CHECK(PMGetCollate(settingsCopy, &flag) == noErr && flag);
	CHECK(PMGetDuplex(settingsCopy, &duplex) == noErr && duplex == kPMDuplexNoTumble);
	CHECK(PMPrintSettingsGetJobName(settingsCopy, &string) == noErr && equal(string, "a job"));
	CHECK(PMSetCopies(settingsCopy, 9, false) == noErr && PMGetCopies(settings, &u) == noErr && u == 3);
	CHECK(PMCopyPrintSettings(NULL, settingsCopy) == kPMInvalidPrintSettings);

	CHECK(PMPrintSettingsSetValue(settings, CFSTR("key"), CFSTR("value"), false) == noErr);
	CFTypeRef found = NULL;
	CHECK(PMPrintSettingsGetValue(settings, CFSTR("key"), &found) == noErr && found && CFEqual(found, CFSTR("value")));
	CHECK(PMPrintSettingsGetValue(settings, CFSTR("other"), &found) == -9589);
	CHECK(PMSetPrintSettingsExtendedData(settings, 'TEST', sizeof(stored), &stored) == noErr);
	size = 4;
	got = 0;
	CHECK(PMGetPrintSettingsExtendedData(settings, 'TEST', &size, &got) == noErr && got == 0x12345678);

	CFDataRef settingsData = NULL;
	PMPrintSettings settingsBack = NULL;
	CHECK(PMPrintSettingsCreateDataRepresentation(settings, &settingsData, kPMDataFormatXMLDefault) == noErr && settingsData);
	CHECK(PMPrintSettingsCreateWithDataRepresentation(settingsData, &settingsBack) == noErr && settingsBack);
	CHECK(PMGetCopies(settingsBack, &u) == noErr && u == 3);
	CHECK(PMGetCollate(settingsBack, &flag) == noErr && flag);
	CHECK(PMPrintSettingsGetJobName(settingsBack, &string) == noErr && equal(string, "a job"));
	CHECK(PMPrintSettingsCreateWithDataRepresentation(junk, &settingsBack) == kPMInvalidObject);

	// ---- the objects of the other kind are refused
	CHECK(PMGetCopies((PMPrintSettings) format, &u) == kPMInvalidPrintSettings);
	CHECK(PMGetOrientation((PMPageFormat) settings, &orientation) == kPMInvalidPageFormat);
	CHECK(PMSessionDefaultPageFormat(session, (PMPageFormat) settings) == kPMInvalidPageFormat);
	CHECK(PMSessionDefaultPrintSettings((PMPrintSession) format, settings) == kPMInvalidPrintSession);

	printf(g_failures ? "FAIL carbon_print_objects\n" : "PASS carbon_print_objects\n");
	return g_failures != 0;
}
