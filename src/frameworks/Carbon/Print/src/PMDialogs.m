/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

// The dialogs of the Printing Manager of the 32-bit API: Page Setup and Print. They are the page layout
// panel and the print panel of the application framework, which is not linked here (Carbon is below it): it
// is loaded when a dialog is first shown, as Navigation Services does for its file panels. The panels
// work on a print info, whose page format and print settings are the objects of the application (they
// are copied into it and, when the dialog is accepted, copied back).

#ifndef __LP64__

#include <Print/PMApplication.h>
#import <Foundation/Foundation.h>
#import <objc/message.h>
#include <dispatch/dispatch.h>
#include <dlfcn.h>

// What the panels and the print info are asked (their classes are not known here)
@interface NSObject (PrintPanels)
+ (id) pageLayout;
+ (id) printPanel;
- (NSMutableDictionary*) dictionary;
@end

// The key under which PMSessionUseSheets keeps the window and the procedure in the session
#define kSheetKey CFSTR("com.apple.print.sheet")

struct SheetInfo
{
	WindowRef window;
	PMSheetDoneUPP doneProc;
};

OSStatus PMSessionUseSheets(PMPrintSession printSession, WindowRef documentWindow, PMSheetDoneUPP sheetDoneProc)
{
	struct SheetInfo info = { documentWindow, sheetDoneProc };
	OSStatus status;

	if (PMSessionError(printSession) == kPMInvalidPrintSession)
		return kPMInvalidPrintSession;
	if (!documentWindow || !sheetDoneProc)
	{
		// no window ends the use of sheets
		return PMSessionSetDataInSession(printSession, kSheetKey, NULL);
	}
	CFDataRef data = CFDataCreate(NULL, (const UInt8*) &info, sizeof(info));
	status = PMSessionSetDataInSession(printSession, kSheetKey, data);
	CFRelease(data);
	return status;
}

static bool sheetInfo(PMPrintSession session, struct SheetInfo* info)
{
	CFTypeRef data = NULL;

	if (PMSessionGetDataFromSession(session, kSheetKey, &data) != noErr || !data || CFGetTypeID(data) != CFDataGetTypeID() ||
			CFDataGetLength(data) != sizeof(*info))
		return false;
	memcpy(info, CFDataGetBytePtr(data), sizeof(*info));
	return true;
}

// The application framework, once it is loaded
static bool loadAppKit(void)
{
	static bool loaded;
	static dispatch_once_t once;

	dispatch_once(&once, ^{
		loaded = dlopen("/System/Library/Frameworks/AppKit.framework/AppKit", RTLD_LAZY | RTLD_GLOBAL) != NULL;
	});
	return loaded;
}

// Runs the panel (what it is, and what runs it) on a print info that has the objects of the application
// copied in, and copies the objects back when the user accepts. The page format or the print settings may be
// NULL when the panel does not work on them; the page format is only copied back by the page setup panel (the
// print panel shows the page format of the document and does not change it).
static bool runPanel(PMPrintSession session, PMPageFormat pageFormat, PMPrintSettings printSettings, bool printPanel)
{
	bool accepted = false;

	if (!loadAppKit())
		return false;
	@autoreleasepool
	{
		[NSClassFromString(@"NSApplication") sharedApplication];
		id printInfo = [[NSClassFromString(@"NSPrintInfo") alloc] init];
		SEL formatSelector = NSSelectorFromString(@"PMPageFormat");
		SEL settingsSelector = NSSelectorFromString(@"PMPrintSettings");

		if (pageFormat)
		{
			PMCopyPageFormat(pageFormat, ((PMPageFormat (*)(id, SEL)) objc_msgSend)(printInfo, formatSelector));
			((void (*)(id, SEL)) objc_msgSend)(printInfo, NSSelectorFromString(@"updateFromPMPageFormat"));
		}
		if (printSettings)
		{
			CFStringRef name = NULL;

			PMCopyPrintSettings(printSettings, ((PMPrintSettings (*)(id, SEL)) objc_msgSend)(printInfo, settingsSelector));
			((void (*)(id, SEL)) objc_msgSend)(printInfo, NSSelectorFromString(@"updateFromPMPrintSettings"));
			if (PMPrintSettingsGetJobName(printSettings, &name) == noErr && name)
				[[printInfo dictionary] setObject: (NSString*) name forKey: @"_title"];
		}
		if (printPanel)
		{
			id panel = [NSClassFromString(@"NSPrintPanel") printPanel];

			accepted = ((int (*)(id, SEL, id)) objc_msgSend)(panel, NSSelectorFromString(@"runModalWithPrintInfo:"), printInfo) == 1;
		}
		else
		{
			id layout = [NSClassFromString(@"NSPageLayout") pageLayout];

			accepted = ((int (*)(id, SEL, id)) objc_msgSend)(layout, NSSelectorFromString(@"runModalWithPrintInfo:"), printInfo) == 1;
		}
		if (accepted)
		{
			if (pageFormat && !printPanel)
			{
				PMCopyPageFormat(((PMPageFormat (*)(id, SEL)) objc_msgSend)(printInfo, formatSelector), pageFormat);
				PMSessionValidatePageFormat(session, pageFormat, NULL);
			}
			if (printSettings)
			{
				PMCopyPrintSettings(((PMPrintSettings (*)(id, SEL)) objc_msgSend)(printInfo, settingsSelector), printSettings);
				PMSessionValidatePrintSettings(session, printSettings, NULL);
			}
		}
		[printInfo release];
	}
	return accepted;
}

// Shows the dialog now, or as a sheet of the document window when the session uses sheets: the call
// returns at once and the procedure of the application gets the result
static OSStatus showDialog(PMPrintSession session, PMPageFormat pageFormat, PMPrintSettings printSettings, bool printPanel, Boolean* accepted)
{
	struct SheetInfo sheet;

	if (sheetInfo(session, &sheet))
	{
		PMRetain(session);
		PMRetain(pageFormat);
		PMRetain(printSettings);
		CFRunLoopPerformBlock(CFRunLoopGetMain(), kCFRunLoopCommonModes, ^{
			Boolean result = runPanel(session, pageFormat, printSettings, printPanel);

			sheet.doneProc(session, sheet.window, result);
			PMRelease(session);
			PMRelease(pageFormat);
			PMRelease(printSettings);
		});
		CFRunLoopWakeUp(CFRunLoopGetMain());
		return noErr;
	}
	if (!accepted)
		return kPMInvalidParameter;
	*accepted = runPanel(session, pageFormat, printSettings, printPanel);
	return noErr;
}

OSStatus PMSessionPageSetupDialog(PMPrintSession printSession, PMPageFormat pageFormat, Boolean* accepted)
{
	double scale;

	if (PMSessionError(printSession) == kPMInvalidPrintSession)
		return kPMInvalidPrintSession;
	if (PMGetScale(pageFormat, &scale) != noErr)
		return kPMInvalidPageFormat;
	return showDialog(printSession, pageFormat, NULL, false, accepted);
}

OSStatus PMSessionPrintDialog(PMPrintSession printSession, PMPrintSettings printSettings, PMPageFormat constPageFormat, Boolean* accepted)
{
	UInt32 copies;
	double scale;

	if (PMSessionError(printSession) == kPMInvalidPrintSession)
		return kPMInvalidPrintSession;
	if (PMGetCopies(printSettings, &copies) == kPMInvalidPrintSettings)
		return kPMInvalidPrintSettings;
	if (PMGetScale(constPageFormat, &scale) != noErr)
		return kPMInvalidPageFormat;
	return showDialog(printSession, constPageFormat, printSettings, true, accepted);
}

#endif
