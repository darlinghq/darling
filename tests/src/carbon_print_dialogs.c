// Copyright (C) 2026 Lubos Dolezel
//
// The dialogs of the Printing Manager (PMSessionPageSetupDialog, PMSessionPrintDialog and the sheet that
// PMSessionUseSheets asks for). A harness (carbon_dialog_user.py) acts as the user: it accepts the
// first dialog (Return), cancels the second (Escape), and so on:
//   1  Page Setup, accepted: accepted is true and the page format keeps what it had (the dialog is
//      shown with the values of the page format and gives them back)
//   2  Page Setup, cancelled: accepted is false and the page format is as it was
//   3  Print, accepted: accepted is true and the settings have what they had, the destination included
//      (the copies, the pages and the PDF file the job goes to)
//   4  Print, cancelled: accepted is false and the settings are as they were
//   5  Page Setup as a sheet, accepted: the call returns at once and the procedure of
//      PMSessionUseSheets gets accepted = true
// The settings are then used to print a page, which is a PDF file.
//
//   carbon_print_dialogs
//
// Lines the harness reads:
//   DIALOG_READY n       the dialog n is shown
//   PASS carbon_print_dialogs / FAIL ...
//
// Needs a window system (X11 or Wayland) and a host GL driver; 32-bit only (the dialogs are not in the
// 64-bit API).
#include <Carbon/Carbon.h>
#include <dispatch/dispatch.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __LP64__
int main(void)
{
	printf("PASS carbon_print_dialogs (no 64-bit API)\n");
	return 0;
}
#else

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void on_alarm(int sig)
{
	printf("FAIL: timed out\n");
	fflush(stdout);
	_exit(1);
}

// The harness types when the dialog has been up for a few seconds: say that it is about to be shown, and
// that is when the window system gets it
static void announce(int number)
{
	dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC), dispatch_get_global_queue(0, 0), ^{
		printf("DIALOG_READY %d\n", number);
		fflush(stdout);
	});
}

static int g_sheetDone;
static Boolean g_sheetAccepted;
static WindowRef g_sheetWindow;

static void sheetDone(PMPrintSession session, WindowRef window, Boolean accepted)
{
	g_sheetAccepted = accepted;
	g_sheetWindow = window;
	g_sheetDone = 1;
}

static int same(PMRect a, PMRect b)
{
	return a.top == b.top && a.left == b.left && a.bottom == b.bottom && a.right == b.right;
}

int main(void)
{
	PMPrintSession session = NULL;
	PMPageFormat format = NULL;
	PMPrintSettings settings = NULL;
	PMRect before, after;
	PMOrientation orientation;
	double scale;
	Boolean accepted;
	UInt32 u, u2;
	char dir[] = "/tmp/carbon_print_dialogs.XXXXXX";
	char path[300];

	signal(SIGALRM, on_alarm);
	alarm(240);
	CHECK(mkdtemp(dir) != NULL);
	snprintf(path, sizeof(path), "%s/job.pdf", dir);
	CHECK(PMCreateSession(&session) == noErr);
	CHECK(PMCreatePageFormat(&format) == noErr && PMSessionDefaultPageFormat(session, format) == noErr);
	CHECK(PMCreatePrintSettings(&settings) == noErr && PMSessionDefaultPrintSettings(session, settings) == noErr);
	CHECK(PMSetOrientation(format, kPMLandscape, false) == noErr && PMSetScale(format, 80) == noErr);
	CHECK(PMSessionValidatePageFormat(session, format, NULL) == noErr);
	CHECK(PMGetAdjustedPageRect(format, &before) == noErr);

	// the calls refuse what they cannot work with
	accepted = true;
	CHECK(PMSessionPageSetupDialog(NULL, format, &accepted) == kPMInvalidPrintSession);
	CHECK(PMSessionPageSetupDialog(session, NULL, &accepted) == kPMInvalidPageFormat);
	CHECK(PMSessionPrintDialog(NULL, settings, format, &accepted) == kPMInvalidPrintSession);
	CHECK(PMSessionPrintDialog(session, NULL, format, &accepted) == kPMInvalidPrintSettings);
	CHECK(PMSessionPrintDialog(session, settings, NULL, &accepted) == kPMInvalidPageFormat);

	// 1: Page Setup, accepted
	accepted = false;
	announce(1);
	CHECK(PMSessionPageSetupDialog(session, format, &accepted) == noErr);
	CHECK(accepted);
	CHECK(PMGetOrientation(format, &orientation) == noErr && orientation == kPMLandscape);
	CHECK(PMGetScale(format, &scale) == noErr && scale == 80);
	CHECK(PMGetAdjustedPageRect(format, &after) == noErr && same(before, after));

	// 2: Page Setup, cancelled
	accepted = true;
	announce(2);
	CHECK(PMSessionPageSetupDialog(session, format, &accepted) == noErr);
	CHECK(!accepted);
	CHECK(PMGetOrientation(format, &orientation) == noErr && orientation == kPMLandscape);
	CHECK(PMGetAdjustedPageRect(format, &after) == noErr && same(before, after));

	// 3: Print, accepted; the job is to be a PDF file, three copies of pages 2 to 4
	CFURLRef url = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8*) path, strlen(path), false);
	CHECK(PMSessionSetDestination(session, settings, kPMDestinationFile, kPMDocumentFormatPDF, url) == noErr);
	CHECK(PMSetPageRange(settings, 1, 10) == noErr && PMSetFirstPage(settings, 2, false) == noErr && PMSetLastPage(settings, 4, false) == noErr);
	CHECK(PMSetCopies(settings, 3, false) == noErr);
	accepted = false;
	announce(3);
	CHECK(PMSessionPrintDialog(session, settings, format, &accepted) == noErr);
	CHECK(accepted);
	CHECK(PMGetCopies(settings, &u) == noErr && u == 3);
	CHECK(PMGetFirstPage(settings, &u) == noErr && u == 2 && PMGetLastPage(settings, &u2) == noErr && u2 == 4);
	CHECK(PMGetPageRange(settings, &u, &u2) == noErr && u == 1 && u2 == 10);
	PMDestinationType destination = 0;
	CFURLRef location = NULL;
	CHECK(PMSessionGetDestinationType(session, settings, &destination) == noErr && destination == kPMDestinationFile);
	CHECK(PMSessionCopyDestinationLocation(session, settings, &location) == noErr && location && CFEqual(location, url));
	// the page format is the document's and is not changed by the print dialog
	CHECK(PMGetAdjustedPageRect(format, &after) == noErr && same(before, after));

	// 4: Print, cancelled
	CHECK(PMSetCopies(settings, 5, false) == noErr);
	accepted = true;
	announce(4);
	CHECK(PMSessionPrintDialog(session, settings, format, &accepted) == noErr);
	CHECK(!accepted);
	CHECK(PMGetCopies(settings, &u) == noErr && u == 5);

	// 5: Page Setup as a sheet
	CHECK(PMSessionUseSheets(session, (WindowRef) 0x1000, sheetDone) == noErr);
	accepted = false;
	announce(5);
	CHECK(PMSessionPageSetupDialog(session, format, &accepted) == noErr);
	CHECK(!g_sheetDone);
	for (int i = 0; i < 1200 && !g_sheetDone; i++)
		CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.1, false);
	CHECK(g_sheetDone && g_sheetAccepted && g_sheetWindow == (WindowRef) 0x1000);
	CHECK(PMGetAdjustedPageRect(format, &after) == noErr && same(before, after));
	CHECK(PMSessionUseSheets(session, NULL, NULL) == noErr);

	// the accepted settings print a page
	CHECK(PMSessionBeginDocument(session, settings, format) == noErr);
	CHECK(PMSessionBeginPage(session, format, NULL) == noErr);
	CGContextRef context = NULL;
	CHECK(PMSessionGetCGGraphicsContext(session, &context) == noErr && context);
	CHECK(PMSessionEndPage(session) == noErr);
	CHECK(PMSessionEndDocument(session) == noErr);
	CHECK(access(path, R_OK) == 0);
	unlink(path);
	rmdir(dir);

	printf(g_failures ? "FAIL carbon_print_dialogs\n" : "PASS carbon_print_dialogs\n");
	fflush(stdout);
	return g_failures != 0;
}
#endif
