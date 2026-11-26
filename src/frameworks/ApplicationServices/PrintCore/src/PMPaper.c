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

// Papers and the printer. There is no print queue on this system, so the one printer is the generic
// one, which has the standard papers; the application makes more papers of its own with
// PMPaperCreateCustom().

#include "PMInternal.h"
#include <dispatch/dispatch.h>
#include <string.h>
#include <stddef.h>

static CFTypeID g_typeIDs[5];
static CFRuntimeClass g_paperClass, g_pageFormatClass, g_settingsClass, g_sessionClass, g_printerClass;
enum { kPaperType, kPageFormatType, kSettingsType, kSessionType, kPrinterType };

CFTypeID PMPaperTypeID(void) { PMRegisterClasses(); return g_typeIDs[kPaperType]; }
CFTypeID PMPageFormatTypeID(void) { PMRegisterClasses(); return g_typeIDs[kPageFormatType]; }
CFTypeID PMPrintSettingsTypeID(void) { PMRegisterClasses(); return g_typeIDs[kSettingsType]; }
CFTypeID PMPrintSessionTypeID(void) { PMRegisterClasses(); return g_typeIDs[kSessionType]; }
CFTypeID PMPrinterTypeID(void) { PMRegisterClasses(); return g_typeIDs[kPrinterType]; }

static void releaseIf(CFTypeRef object)
{
	if (object)
		CFRelease(object);
}

static void paperFinalize(CFTypeRef cf)
{
	struct __PMPaper* paper = (struct __PMPaper*) cf;

	releaseIf(paper->id);
	releaseIf(paper->ppdName);
	releaseIf(paper->printerID);
	releaseIf(paper->name);
}

static void pageFormatFinalize(CFTypeRef cf)
{
	struct __PMPageFormat* format = (struct __PMPageFormat*) cf;

	releaseIf(format->paper);
	releaseIf(format->extended);
}

static void settingsFinalize(CFTypeRef cf)
{
	struct __PMPrintSettings* s = (struct __PMPrintSettings*) cf;

	releaseIf(s->jobName);
	releaseIf(s->destinationFormat);
	releaseIf(s->destinationURL);
	releaseIf(s->values);
	releaseIf(s->extended);
}

static void sessionFinalize(CFTypeRef cf)
{
	struct __PMPrintSession* s = (struct __PMPrintSession*) cf;

	if (s->context)
	{
		CGPDFContextClose(s->context);
		CGContextRelease(s->context);
	}
	releaseIf(s->documentFormat);
	releaseIf(s->settings);
	releaseIf(s->format);
	releaseIf(s->data);
}

static void printerFinalize(CFTypeRef cf)
{
	struct __PMPrinter* printer = (struct __PMPrinter*) cf;

	releaseIf(printer->id);
	releaseIf(printer->name);
	releaseIf(printer->papers);
}

void PMRegisterClasses(void)
{
	static dispatch_once_t once;

	dispatch_once(&once, ^{
		g_paperClass.className = "PMPaper";
		g_paperClass.finalize = paperFinalize;
		g_typeIDs[kPaperType] = _CFRuntimeRegisterClass(&g_paperClass);
		g_pageFormatClass.className = "PMPageFormat";
		g_pageFormatClass.finalize = pageFormatFinalize;
		g_typeIDs[kPageFormatType] = _CFRuntimeRegisterClass(&g_pageFormatClass);
		g_settingsClass.className = "PMPrintSettings";
		g_settingsClass.finalize = settingsFinalize;
		g_typeIDs[kSettingsType] = _CFRuntimeRegisterClass(&g_settingsClass);
		g_sessionClass.className = "PMPrintSession";
		g_sessionClass.finalize = sessionFinalize;
		g_typeIDs[kSessionType] = _CFRuntimeRegisterClass(&g_sessionClass);
		g_printerClass.className = "PMPrinter";
		g_printerClass.finalize = printerFinalize;
		g_typeIDs[kPrinterType] = _CFRuntimeRegisterClass(&g_printerClass);
	});
}

void* PMNewObject(CFTypeID type, size_t size)
{
	void* object = _CFRuntimeCreateInstance(NULL, type, size - sizeof(CFRuntimeBase), NULL);

	if (object)
		memset((char*) object + sizeof(CFRuntimeBase), 0, size - sizeof(CFRuntimeBase));
	return object;
}

// ---- papers

static struct __PMPaper* newPaper(CFStringRef id, CFStringRef ppdName, CFStringRef name, double width, double height,
		const PMPaperMargins* margins, Boolean custom)
{
	struct __PMPaper* paper = PMNewObject(PMPaperTypeID(), sizeof(struct __PMPaper));

	if (!paper)
		return NULL;
	paper->id = id ? CFRetain(id) : NULL;
	paper->ppdName = ppdName ? CFRetain(ppdName) : NULL;
	paper->name = name ? CFRetain(name) : NULL;
	paper->width = width;
	paper->height = height;
	if (margins)
		paper->margins = *margins;
	paper->custom = custom;
	return paper;
}

// The standard papers of the generic printer, in the order of its list, with the unprintable area of each
static const struct {
	const char* id;
	const char* ppdName;
	const char* name;
	double width, height, bottom;
} kStandardPapers[] = {
	{ "na-letter", "Letter", "US Letter", 612, 792, 40 },
	{ "na-legal", "Legal", "US Legal", 612, 1008, 40 },
	{ "iso-a4", "A4", "A4", 595, 842, 41 },
	{ "iso-a5", "A5", "A5", 420, 595, 18 },
	{ "jis-b5", "B5", "JIS B5", 516, 729, 18 },
	{ "iso-b5", "ISOB5", "B5", 499, 709, 18 },
	{ "na-number-10-envelope", "Env10", "Envelope #10", 297, 684, 40 },
	{ "iso-designated", "EnvDL", "Envelope DL", 312, 624, 18 },
	{ "tabloid", "Tabloid", "Tabloid", 792, 1224, 18 },
	{ "iso-a3", "A3", "A3", 842, 1191, 18 },
	{ "arch-b", "ARCHB", "Tabloid Oversize", 864, 1296, 18 },
	{ "ROC 16K", "roc16K", "ROC 16K", 558, 774, 18 },
	{ "Envelope Choukei 3", "EnvChou3", "Envelope Choukei 3", 340, 666, 18 },
	{ "super-b", "13x19", "Super B/A3", 936, 1368, 18 }
};

static struct __PMPaper* newStandardPaper(size_t index)
{
	PMPaperMargins margins = { 18, 18, kStandardPapers[index].bottom, 18 };
	CFStringRef id = CFStringCreateWithCString(NULL, kStandardPapers[index].id, kCFStringEncodingUTF8);
	CFStringRef ppdName = CFStringCreateWithCString(NULL, kStandardPapers[index].ppdName, kCFStringEncodingUTF8);
	CFStringRef name = CFStringCreateWithCString(NULL, kStandardPapers[index].name, kCFStringEncodingUTF8);
	struct __PMPaper* paper = newPaper(id, ppdName, name, kStandardPapers[index].width, kStandardPapers[index].height, &margins, false);

	CFRelease(id);
	CFRelease(ppdName);
	CFRelease(name);
	return paper;
}

// The default paper of a page format with no printer: Letter for the United States, Canada and Mexico,
// else A4
struct __PMPaper* PMCopyDefaultPaper(void)
{
	CFLocaleRef locale = CFLocaleCopyCurrent();
	CFStringRef country = locale ? CFLocaleGetValue(locale, kCFLocaleCountryCode) : NULL;
	bool letter = country && (CFStringCompare(country, CFSTR("US"), 0) == kCFCompareEqualTo ||
		CFStringCompare(country, CFSTR("CA"), 0) == kCFCompareEqualTo || CFStringCompare(country, CFSTR("MX"), 0) == kCFCompareEqualTo);

	if (locale)
		CFRelease(locale);
	return newStandardPaper(letter ? 0 : 2);
}

CFArrayRef PMGenericPaperList(void)
{
	return ((struct __PMPrinter*) PMGenericPrinter())->papers;
}

PMPrinter PMGenericPrinter(void)
{
	static struct __PMPrinter* printer;
	static dispatch_once_t once;

	dispatch_once(&once, ^{
		CFMutableArrayRef papers = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

		for (size_t i = 0; i < sizeof(kStandardPapers) / sizeof(kStandardPapers[0]); i++)
		{
			struct __PMPaper* paper = newStandardPaper(i);

			CFArrayAppendValue(papers, paper);
			CFRelease(paper);
		}
		printer = PMNewObject(PMPrinterTypeID(), sizeof(struct __PMPrinter));
		printer->id = CFSTR("");
		printer->name = CFSTR("");
		printer->papers = papers;
	});
	return (PMPrinter) printer;
}

OSStatus PMPaperCreateCustom(PMPrinter printer, CFStringRef customPaperID, CFStringRef customPaperName, double width, double height,
		const PMPaperMargins* margins, PMPaper* paperP)
{
	PMPaperMargins none = { 0, 0, 0, 0 };
	struct __PMPaper* paper;

	if (!paperP)
		return kPMInvalidParameter;
	// QUIRK: the sizes and the margins are not checked, and the margins are optional here (the system this follows
	// takes the call without any margins to be an error that stops the program). Not documented. Evidence:
	// black-box observation on macOS 10.14.4 and 26.7.3 (tier 2). Test: tests/src/carbon_print_objects.c
	// (test_quirk_custom_paper_unchecked)
	paper = newPaper(customPaperID, NULL, customPaperName, width, height, margins ? margins : &none, true);
	*paperP = (PMPaper) paper;
	return paper ? noErr : kPMGeneralError;
}

static OSStatus paperString(PMPaper paper, CFStringRef* string, size_t offset, OSStatus missing)
{
	struct __PMPaper* p = PMAsPaper(paper);
	CFStringRef value;

	if (!p || !string)
		return kPMInvalidParameter;
	value = *(CFStringRef*) ((char*) p + offset);
	*string = value;
	return value ? noErr : missing;
}

OSStatus PMPaperGetID(PMPaper paper, CFStringRef* paperID)
{
	return paperString(paper, paperID, offsetof(struct __PMPaper, id), noErr);
}

// A custom paper is not in the list of a printer, so it has no name there
OSStatus PMPaperGetPPDPaperName(PMPaper paper, CFStringRef* ppdPaperName)
{
	return paperString(paper, ppdPaperName, offsetof(struct __PMPaper, ppdName), kPMNoSuchEntry);
}

OSStatus PMPaperGetPrinterID(PMPaper paper, CFStringRef* printerID)
{
	return paperString(paper, printerID, offsetof(struct __PMPaper, printerID), noErr);
}

OSStatus PMPaperGetWidth(PMPaper paper, double* paperWidth)
{
	struct __PMPaper* p = PMAsPaper(paper);

	if (!p || !paperWidth)
		return kPMInvalidParameter;
	*paperWidth = p->width;
	return noErr;
}

OSStatus PMPaperGetHeight(PMPaper paper, double* paperHeight)
{
	struct __PMPaper* p = PMAsPaper(paper);

	if (!p || !paperHeight)
		return kPMInvalidParameter;
	*paperHeight = p->height;
	return noErr;
}

OSStatus PMPaperGetMargins(PMPaper paper, PMPaperMargins* paperMargins)
{
	struct __PMPaper* p = PMAsPaper(paper);

	if (!p || !paperMargins)
		return kPMInvalidParameter;
	*paperMargins = p->margins;
	return noErr;
}

Boolean PMPaperIsCustom(PMPaper paper)
{
	struct __PMPaper* p = PMAsPaper(paper);

	return p && p->custom;
}

// The name for a menu: the printer must be given (the call fails with paramErr without one)
OSStatus PMPaperCreateLocalizedName(PMPaper paper, PMPrinter printer, CFStringRef* paperName)
{
	struct __PMPaper* p = PMAsPaper(paper);

	if (!p || !paperName || !printer)
		return kPMInvalidParameter;
	*paperName = p->name ? CFRetain(p->name) : (p->id ? CFRetain(p->id) : NULL);
	return noErr;
}

// ---- printers

CFStringRef PMPrinterGetName(PMPrinter printer)
{
	return printer && CFGetTypeID(printer) == PMPrinterTypeID() ? ((struct __PMPrinter*) printer)->name : NULL;
}

CFStringRef PMPrinterGetID(PMPrinter printer)
{
	return printer && CFGetTypeID(printer) == PMPrinterTypeID() ? ((struct __PMPrinter*) printer)->id : NULL;
}

Boolean PMPrinterIsDefault(PMPrinter printer)
{
	return false;
}

OSStatus PMPrinterGetPaperList(PMPrinter printer, CFArrayRef* paperList)
{
	if (!printer || CFGetTypeID(printer) != PMPrinterTypeID() || !paperList)
		return kPMInvalidParameter;
	*paperList = ((struct __PMPrinter*) printer)->papers;
	return noErr;
}

// There is no print queue: the list is empty and no printer is the current one
OSStatus PMSessionCreatePrinterList(PMPrintSession printSession, CFArrayRef* printerList, CFIndex* currentIndex, PMPrinter* currentPrinter)
{
	if (!PMAsSession(printSession))
		return kPMInvalidPrintSession;
	if (printerList)
		*printerList = CFArrayCreate(NULL, NULL, 0, &kCFTypeArrayCallBacks);
	if (currentIndex)
		*currentIndex = -1;
	if (currentPrinter)
		*currentPrinter = NULL;
	return noErr;
}

OSStatus PMSessionGetCurrentPrinter(PMPrintSession printSession, PMPrinter* currentPrinter)
{
	if (!PMAsSession(printSession))
		return kPMInvalidPrintSession;
	if (!currentPrinter)
		return kPMInvalidParameter;
	*currentPrinter = PMGenericPrinter();
	return noErr;
}

// A page format for each paper of the printer
OSStatus PMSessionCreatePageFormatList(PMPrintSession printSession, PMPrinter printer, CFArrayRef* pageFormatList)
{
	CFArrayRef papers;
	CFMutableArrayRef formats;

	if (!PMAsSession(printSession))
		return kPMInvalidPrintSession;
	if (!pageFormatList || !printer || CFGetTypeID(printer) != PMPrinterTypeID())
		return kPMInvalidParameter;
	papers = ((struct __PMPrinter*) printer)->papers;
	formats = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	for (CFIndex i = 0; i < CFArrayGetCount(papers); i++)
	{
		PMPageFormat format;

		if (PMCreatePageFormatWithPMPaper(&format, (PMPaper) CFArrayGetValueAtIndex(papers, i)) == noErr)
		{
			CFArrayAppendValue(formats, format);
			CFRelease(format);
		}
	}
	*pageFormatList = formats;
	return noErr;
}
