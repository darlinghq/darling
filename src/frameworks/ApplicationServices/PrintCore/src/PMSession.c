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

// The objects of the Printing Manager (print session, page format, print settings) and the
// calls that make a document in a session. There is no printer: the document goes to a PDF
// file, the destination the application (or the print dialog) sets with PMSessionSetDestination().

#include <PrintCore/PrintCore.h>
#include <CoreFoundation/CFRuntime.h>
#include <ApplicationServices/ApplicationServices.h>
#include <dispatch/dispatch.h>
#include <stdbool.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

// The error of a page format that has not been made valid by PMSessionDefaultPageFormat()
enum {
	kPMObjectNotValidated = -9583
};

struct __PMPageFormat
{
	CFRuntimeBase base;
	Boolean valid;
	double paperWidth, paperHeight;	// the paper, in points, in portrait
	PMOrientation orientation;
	double scale;	// in percent
	// QUIRK: the orientation and the scale set on a page format show in its rectangles only after the format has
	// been validated (PMSessionValidatePageFormat); until then the rectangles are the ones of the last validation.
	// Not documented there. Evidence: black-box observation on macOS 10.14.4 i386 (tier 2).
	// Test: tests/src/printcore_session.c (test_quirk_orientation_after_validate)
	PMOrientation appliedOrientation;
	double appliedScale;
};

struct __PMPrintSettings
{
	CFRuntimeBase base;
	UInt32 first, last, minPage, maxPage, copies;
	CFStringRef jobName;
	PMDestinationType destination;
	CFStringRef destinationFormat;
	CFURLRef destinationURL;
};

struct __PMPrintSession
{
	CFRuntimeBase base;
	OSStatus error;
	CFStringRef documentFormat;
	CGContextRef context;
	Boolean inDocument, inPage;
	PMPrintSettings settings;
	PMPageFormat format;
};

static CFTypeID g_pageFormatID, g_settingsID, g_sessionID;
static CFRuntimeClass g_pageFormatClass, g_settingsClass, g_sessionClass;

static void settingsFinalize(CFTypeRef cf)
{
	struct __PMPrintSettings* s = (struct __PMPrintSettings*) cf;

	if (s->jobName)
		CFRelease(s->jobName);
	if (s->destinationFormat)
		CFRelease(s->destinationFormat);
	if (s->destinationURL)
		CFRelease(s->destinationURL);
}

static void sessionFinalize(CFTypeRef cf)
{
	struct __PMPrintSession* s = (struct __PMPrintSession*) cf;

	if (s->context)
	{
		CGPDFContextClose(s->context);
		CGContextRelease(s->context);
	}
	if (s->documentFormat)
		CFRelease(s->documentFormat);
	if (s->settings)
		CFRelease(s->settings);
	if (s->format)
		CFRelease(s->format);
}

static void registerClasses(void)
{
	static dispatch_once_t once;

	dispatch_once(&once, ^{
		g_pageFormatClass.version = 0;
		g_pageFormatClass.className = "PMPageFormat";
		g_pageFormatID = _CFRuntimeRegisterClass(&g_pageFormatClass);
		g_settingsClass.version = 0;
		g_settingsClass.className = "PMPrintSettings";
		g_settingsClass.finalize = settingsFinalize;
		g_settingsID = _CFRuntimeRegisterClass(&g_settingsClass);
		g_sessionClass.version = 0;
		g_sessionClass.className = "PMPrintSession";
		g_sessionClass.finalize = sessionFinalize;
		g_sessionID = _CFRuntimeRegisterClass(&g_sessionClass);
	});
}

static void* newObject(CFTypeID type, size_t size)
{
	void* object = _CFRuntimeCreateInstance(NULL, type, size - sizeof(CFRuntimeBase), NULL);

	if (object)
		memset((char*) object + sizeof(CFRuntimeBase), 0, size - sizeof(CFRuntimeBase));
	return object;
}

static struct __PMPageFormat* asPageFormat(PMPageFormat format)
{
	return format && CFGetTypeID(format) == g_pageFormatID ? (struct __PMPageFormat*) format : NULL;
}

static struct __PMPrintSettings* asSettings(PMPrintSettings settings)
{
	return settings && CFGetTypeID(settings) == g_settingsID ? (struct __PMPrintSettings*) settings : NULL;
}

static struct __PMPrintSession* asSession(PMPrintSession session)
{
	return session && CFGetTypeID(session) == g_sessionID ? (struct __PMPrintSession*) session : NULL;
}

OSStatus PMRetain(PMObject object)
{
	if (object)
		CFRetain(object);
	return noErr;
}

OSStatus PMRelease(PMObject object)
{
	if (object)
		CFRelease(object);
	return noErr;
}

// QUIRK: a call that fails because of the state of the session or its arguments only returns the error; the
// error of PMSessionError() stays noErr. Not documented. Evidence: black-box observation on macOS 10.14.4 i386
// (tier 2). Test: tests/src/printcore_session.c (test_quirk_session_error_stays)
static OSStatus sessionFail(struct __PMPrintSession* session, OSStatus error)
{
	return error;
}

OSStatus PMCreateSession(PMPrintSession* printSession)
{
	if (!printSession)
		return kPMInvalidParameter;
	registerClasses();
	*printSession = newObject(g_sessionID, sizeof(struct __PMPrintSession));
	return *printSession ? noErr : kPMGeneralError;
}

OSStatus PMSessionError(PMPrintSession printSession)
{
	struct __PMPrintSession* session = asSession(printSession);

	return session ? session->error : kPMInvalidPrintSession;
}

// ---- page formats

OSStatus PMCreatePageFormat(PMPageFormat* pageFormat)
{
	if (!pageFormat)
		return kPMInvalidParameter;
	registerClasses();
	*pageFormat = (PMPageFormat) newObject(g_pageFormatID, sizeof(struct __PMPageFormat));
	return *pageFormat ? noErr : kPMGeneralError;
}

// The paper with no printer is the default of the region: Letter for the United States, Canada and
// Mexico, else A4.
static void defaultPaper(double* width, double* height)
{
	CFLocaleRef locale = CFLocaleCopyCurrent();
	CFStringRef country = locale ? CFLocaleGetValue(locale, kCFLocaleCountryCode) : NULL;
	bool letter = country && (CFStringCompare(country, CFSTR("US"), 0) == kCFCompareEqualTo ||
		CFStringCompare(country, CFSTR("CA"), 0) == kCFCompareEqualTo || CFStringCompare(country, CFSTR("MX"), 0) == kCFCompareEqualTo);

	if (locale)
		CFRelease(locale);
	*width = letter ? 612 : 595;
	*height = letter ? 792 : 842;
}

OSStatus PMSessionDefaultPageFormat(PMPrintSession printSession, PMPageFormat pageFormat)
{
	struct __PMPageFormat* format = asPageFormat(pageFormat);

	if (!asSession(printSession))
		return kPMInvalidPrintSession;
	if (!format)
		return kPMInvalidPageFormat;
	defaultPaper(&format->paperWidth, &format->paperHeight);
	format->orientation = format->appliedOrientation = kPMPortrait;
	format->scale = format->appliedScale = 100;
	format->valid = true;
	return noErr;
}

OSStatus PMSessionValidatePageFormat(PMPrintSession printSession, PMPageFormat pageFormat, Boolean* changed)
{
	struct __PMPageFormat* format = asPageFormat(pageFormat);

	if (!asSession(printSession))
		return kPMInvalidPrintSession;
	if (!format)
		return kPMInvalidPageFormat;
	if (changed)
		*changed = false;
	// QUIRK: a page format that was never made valid by PMSessionDefaultPageFormat stays invalid: the validation
	// fails with -9583. Not documented. Evidence: black-box observation on macOS 10.14.4 i386 (tier 2).
	// Test: tests/src/printcore_session.c (test_quirk_unvalidated_format)
	if (!format->valid)
		return kPMObjectNotValidated;
	format->appliedOrientation = format->orientation;
	// A scale that is not positive does not reach the rectangles
	if (format->scale > 0)
		format->appliedScale = format->scale;
	return noErr;
}

// The paper in the orientation of the format, before the scale
static void orientedPaper(const struct __PMPageFormat* format, double* width, double* height)
{
	bool landscape = format->appliedOrientation == kPMLandscape;

	*width = landscape ? format->paperHeight : format->paperWidth;
	*height = landscape ? format->paperWidth : format->paperHeight;
}

static OSStatus formatRect(PMPageFormat pageFormat, PMRect* rect, bool scaled)
{
	struct __PMPageFormat* format = asPageFormat(pageFormat);
	double width, height;

	if (!rect)
		return kPMInvalidParameter;
	if (!format)
		return kPMInvalidPageFormat;
	if (!format->valid)
		return kPMObjectNotValidated;
	orientedPaper(format, &width, &height);
	if (scaled)
	{
		width = width * 100 / format->appliedScale;
		height = height * 100 / format->appliedScale;
	}
	rect->top = rect->left = 0;
	rect->bottom = height;
	rect->right = width;
	return noErr;
}

// The paper has no margins here, so the page is the paper
OSStatus PMGetAdjustedPageRect(PMPageFormat pageFormat, PMRect* pageRect) { return formatRect(pageFormat, pageRect, true); }
OSStatus PMGetAdjustedPaperRect(PMPageFormat pageFormat, PMRect* paperRect) { return formatRect(pageFormat, paperRect, true); }
OSStatus PMGetUnadjustedPageRect(PMPageFormat pageFormat, PMRect* pageRect) { return formatRect(pageFormat, pageRect, false); }
OSStatus PMGetUnadjustedPaperRect(PMPageFormat pageFormat, PMRect* paperRect) { return formatRect(pageFormat, paperRect, false); }

OSStatus PMGetOrientation(PMPageFormat pageFormat, PMOrientation* orientation)
{
	struct __PMPageFormat* format = asPageFormat(pageFormat);

	if (!orientation)
		return kPMInvalidParameter;
	if (!format)
		return kPMInvalidPageFormat;
	*orientation = format->orientation ? format->orientation : kPMPortrait;
	return noErr;
}

OSStatus PMSetOrientation(PMPageFormat pageFormat, PMOrientation orientation, Boolean lock)
{
	struct __PMPageFormat* format = asPageFormat(pageFormat);

	if (!format)
		return kPMInvalidPageFormat;
	if (orientation < kPMPortrait || orientation > kPMReversePortrait)
		return paramErr;
	format->orientation = orientation;
	return noErr;
}

OSStatus PMGetScale(PMPageFormat pageFormat, double* scale)
{
	struct __PMPageFormat* format = asPageFormat(pageFormat);

	if (!scale)
		return kPMInvalidParameter;
	if (!format)
		return kPMInvalidPageFormat;
	*scale = format->scale ? format->scale : 100;
	return noErr;
}

OSStatus PMSetScale(PMPageFormat pageFormat, double scale)
{
	struct __PMPageFormat* format = asPageFormat(pageFormat);

	if (!format)
		return kPMInvalidPageFormat;
	format->scale = scale;
	return noErr;
}

#ifndef __LP64__
// The flattened form is the XML property list of the format, in a handle
OSStatus PMFlattenPageFormat(PMPageFormat pageFormat, Handle* flatFormat)
{
	struct __PMPageFormat* format = asPageFormat(pageFormat);

	if (!flatFormat)
		return kPMInvalidParameter;
	if (!format)
		return kPMInvalidPageFormat;

	CFNumberRef width = CFNumberCreate(NULL, kCFNumberDoubleType, &format->paperWidth);
	CFNumberRef height = CFNumberCreate(NULL, kCFNumberDoubleType, &format->paperHeight);
	SInt32 orientation = format->orientation;
	CFNumberRef orientationNumber = CFNumberCreate(NULL, kCFNumberSInt32Type, &orientation);
	CFNumberRef scale = CFNumberCreate(NULL, kCFNumberDoubleType, &format->scale);
	const void* keys[] = { CFSTR("paperWidth"), CFSTR("paperHeight"), CFSTR("orientation"), CFSTR("scale"), CFSTR("valid") };
	const void* values[] = { width, height, orientationNumber, scale, format->valid ? kCFBooleanTrue : kCFBooleanFalse };
	CFDictionaryRef dictionary = CFDictionaryCreate(NULL, keys, values, 5, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFDataRef data = dictionary ? CFPropertyListCreateData(NULL, dictionary, kCFPropertyListXMLFormat_v1_0, 0, NULL) : NULL;
	OSStatus status = kPMGeneralError;

	if (data)
	{
		Handle handle = NewHandle(CFDataGetLength(data));

		if (handle)
		{
			memcpy(*handle, CFDataGetBytePtr(data), CFDataGetLength(data));
			*flatFormat = handle;
			status = noErr;
		}
		CFRelease(data);
	}
	if (dictionary)
		CFRelease(dictionary);
	CFRelease(width);
	CFRelease(height);
	CFRelease(orientationNumber);
	CFRelease(scale);
	return status;
}

OSStatus PMUnflattenPageFormat(Handle flatFormat, PMPageFormat* pageFormat)
{
	if (!pageFormat || !flatFormat)
		return kPMInvalidParameter;

	CFDataRef data = CFDataCreate(NULL, (const UInt8*) *flatFormat, GetHandleSize(flatFormat));
	CFTypeRef list = data ? CFPropertyListCreateWithData(NULL, data, kCFPropertyListImmutable, NULL, NULL) : NULL;
	OSStatus status = kPMInvalidObject;

	if (data)
		CFRelease(data);
	if (list && CFGetTypeID(list) == CFDictionaryGetTypeID())
	{
		PMPageFormat created;

		status = PMCreatePageFormat(&created);
		if (status == noErr)
		{
			struct __PMPageFormat* format = (struct __PMPageFormat*) created;
			CFDictionaryRef dictionary = list;
			SInt32 orientation = kPMPortrait;
			CFNumberRef number;

			if ((number = CFDictionaryGetValue(dictionary, CFSTR("paperWidth"))))
				CFNumberGetValue(number, kCFNumberDoubleType, &format->paperWidth);
			if ((number = CFDictionaryGetValue(dictionary, CFSTR("paperHeight"))))
				CFNumberGetValue(number, kCFNumberDoubleType, &format->paperHeight);
			if ((number = CFDictionaryGetValue(dictionary, CFSTR("orientation"))))
				CFNumberGetValue(number, kCFNumberSInt32Type, &orientation);
			if ((number = CFDictionaryGetValue(dictionary, CFSTR("scale"))))
				CFNumberGetValue(number, kCFNumberDoubleType, &format->scale);
			format->orientation = format->appliedOrientation = (PMOrientation) orientation;
			format->appliedScale = format->scale;
			format->valid = CFDictionaryGetValue(dictionary, CFSTR("valid")) == kCFBooleanTrue && format->paperWidth > 0 && format->scale > 0;
			*pageFormat = created;
		}
	}
	if (list)
		CFRelease(list);
	return status;
}
#endif

// ---- print settings

OSStatus PMCreatePrintSettings(PMPrintSettings* printSettings)
{
	if (!printSettings)
		return kPMInvalidParameter;
	registerClasses();
	*printSettings = (PMPrintSettings) newObject(g_settingsID, sizeof(struct __PMPrintSettings));
	return *printSettings ? noErr : kPMGeneralError;
}

static void resetSettings(struct __PMPrintSettings* settings)
{
	settings->first = 1;
	settings->last = INT_MAX;
	settings->minPage = 1;
	settings->maxPage = INT_MAX;
	settings->copies = 1;
	settings->destination = kPMDestinationPrinter;
}

OSStatus PMSessionDefaultPrintSettings(PMPrintSession printSession, PMPrintSettings printSettings)
{
	struct __PMPrintSettings* settings = asSettings(printSettings);

	if (!asSession(printSession))
		return kPMInvalidPrintSession;
	if (!settings)
		return kPMInvalidPrintSettings;
	resetSettings(settings);
	return noErr;
}

OSStatus PMSessionValidatePrintSettings(PMPrintSession printSession, PMPrintSettings printSettings, Boolean* changed)
{
	struct __PMPrintSettings* settings = asSettings(printSettings);

	if (!asSession(printSession))
		return kPMInvalidPrintSession;
	if (!settings)
		return kPMInvalidPrintSettings;
	if (changed)
		*changed = false;
	if (settings->copies == 0)
	{
		resetSettings(settings);
		if (changed)
			*changed = true;
	}
	return noErr;
}

OSStatus PMGetCopies(PMPrintSettings printSettings, UInt32* copies)
{
	struct __PMPrintSettings* settings = asSettings(printSettings);

	if (!copies)
		return kPMInvalidParameter;
	if (!settings)
		return kPMInvalidPrintSettings;
	*copies = settings->copies ? settings->copies : 1;
	return noErr;
}

OSStatus PMSetCopies(PMPrintSettings printSettings, UInt32 copies, Boolean lock)
{
	struct __PMPrintSettings* settings = asSettings(printSettings);

	if (!settings)
		return kPMInvalidPrintSettings;
	if (copies == 0)
		return kPMValueOutOfRange;
	settings->copies = copies;
	return noErr;
}

OSStatus PMGetFirstPage(PMPrintSettings printSettings, UInt32* first)
{
	struct __PMPrintSettings* settings = asSettings(printSettings);

	if (!first)
		return kPMInvalidParameter;
	if (!settings)
		return kPMInvalidPrintSettings;
	*first = settings->first ? settings->first : 1;
	return noErr;
}

OSStatus PMGetLastPage(PMPrintSettings printSettings, UInt32* last)
{
	struct __PMPrintSettings* settings = asSettings(printSettings);

	if (!last)
		return kPMInvalidParameter;
	if (!settings)
		return kPMInvalidPrintSettings;
	*last = settings->last ? settings->last : INT_MAX;
	return noErr;
}

// QUIRK: the first and last page must lie inside the page range of the settings, else the call fails
// with kPMValueOutOfRange; the page range stays as it was when it is set (PMSetPageRange does not move
// the first and last page). Not documented. Evidence: black-box observation on macOS 10.14.4 i386 (tier 2).
// Test: tests/src/printcore_session.c
OSStatus PMSetFirstPage(PMPrintSettings printSettings, UInt32 first, Boolean lock)
{
	struct __PMPrintSettings* settings = asSettings(printSettings);

	if (!settings)
		return kPMInvalidPrintSettings;
	if (first < settings->minPage || first > settings->maxPage)
		return kPMValueOutOfRange;
	settings->first = first;
	return noErr;
}

OSStatus PMSetLastPage(PMPrintSettings printSettings, UInt32 last, Boolean lock)
{
	struct __PMPrintSettings* settings = asSettings(printSettings);

	if (!settings)
		return kPMInvalidPrintSettings;
	if (last < settings->minPage || last > settings->maxPage)
		return kPMValueOutOfRange;
	settings->last = last;
	return noErr;
}

OSStatus PMGetPageRange(PMPrintSettings printSettings, UInt32* minPage, UInt32* maxPage)
{
	struct __PMPrintSettings* settings = asSettings(printSettings);

	if (!settings)
		return kPMInvalidPrintSettings;
	if (minPage)
		*minPage = settings->minPage ? settings->minPage : 1;
	if (maxPage)
		*maxPage = settings->maxPage ? settings->maxPage : INT_MAX;
	return noErr;
}

OSStatus PMSetPageRange(PMPrintSettings printSettings, UInt32 minPage, UInt32 maxPage)
{
	struct __PMPrintSettings* settings = asSettings(printSettings);

	if (!settings)
		return kPMInvalidPrintSettings;
	if (minPage == 0 || minPage > maxPage)
		return kPMValueOutOfRange;
	settings->minPage = minPage;
	settings->maxPage = maxPage;
	return noErr;
}

OSStatus PMPrintSettingsGetJobName(PMPrintSettings printSettings, CFStringRef* name)
{
	struct __PMPrintSettings* settings = asSettings(printSettings);

	if (!name)
		return kPMInvalidParameter;
	if (!settings)
		return kPMInvalidPrintSettings;
	*name = settings->jobName;
	return noErr;
}

OSStatus PMPrintSettingsSetJobName(PMPrintSettings printSettings, CFStringRef name)
{
	struct __PMPrintSettings* settings = asSettings(printSettings);

	if (!settings)
		return kPMInvalidPrintSettings;
	if (!name)
		return kPMInvalidParameter;
	CFRetain(name);
	if (settings->jobName)
		CFRelease(settings->jobName);
	settings->jobName = name;
	return noErr;
}

OSStatus PMSetJobNameCFString(PMPrintSettings printSettings, CFStringRef name)
{
	return PMPrintSettingsSetJobName(printSettings, name);
}

// ---- destination

OSStatus PMSessionGetDestinationType(PMPrintSession printSession, PMPrintSettings printSettings, PMDestinationType* destTypeP)
{
	struct __PMPrintSettings* settings = asSettings(printSettings);

	if (!asSession(printSession))
		return kPMInvalidPrintSession;
	if (!settings)
		return kPMInvalidPrintSettings;
	if (!destTypeP)
		return kPMInvalidParameter;
	*destTypeP = settings->destination ? settings->destination : kPMDestinationPrinter;
	return noErr;
}

OSStatus PMSessionSetDestination(PMPrintSession printSession, PMPrintSettings printSettings, PMDestinationType destType, CFStringRef destFormat, CFURLRef destLocation)
{
	struct __PMPrintSettings* settings = asSettings(printSettings);

	if (!asSession(printSession))
		return kPMInvalidPrintSession;
	if (!settings)
		return kPMInvalidPrintSettings;
	if (destType < kPMDestinationPrinter || destType > kPMDestinationProcessPDF)
		return kPMValueOutOfRange;
	settings->destination = destType;
	if (destFormat)
		CFRetain(destFormat);
	if (settings->destinationFormat)
		CFRelease(settings->destinationFormat);
	settings->destinationFormat = destFormat;
	if (destLocation)
		CFRetain(destLocation);
	if (settings->destinationURL)
		CFRelease(settings->destinationURL);
	settings->destinationURL = destLocation;
	return noErr;
}

OSStatus PMSessionCopyOutputFormatList(PMPrintSession printSession, PMDestinationType destType, CFArrayRef* documentFormatP)
{
	const void* formats[] = { kPMDocumentFormatPDF };

	if (!asSession(printSession))
		return kPMInvalidPrintSession;
	if (!documentFormatP)
		return kPMInvalidParameter;
	*documentFormatP = destType == kPMDestinationFile ? CFArrayCreate(NULL, formats, 1, &kCFTypeArrayCallBacks) : NULL;
	return noErr;
}

OSStatus PMSessionSetDocumentFormatGeneration(PMPrintSession printSession, CFStringRef docFormat, CFArrayRef graphicsContextTypes, CFTypeRef options)
{
	struct __PMPrintSession* session = asSession(printSession);

	if (!session)
		return kPMInvalidPrintSession;
	// Only the PDF document is made
	if (!docFormat || CFStringCompare(docFormat, kPMDocumentFormatPDF, 0) != kCFCompareEqualTo)
		return sessionFail(session, paramErr);
	if (graphicsContextTypes && CFArrayGetCount(graphicsContextTypes) > 0 &&
			!CFArrayContainsValue(graphicsContextTypes, CFRangeMake(0, CFArrayGetCount(graphicsContextTypes)), kPMGraphicsContextCoreGraphics))
		return sessionFail(session, paramErr);
	CFRetain(docFormat);
	if (session->documentFormat)
		CFRelease(session->documentFormat);
	session->documentFormat = docFormat;
	return noErr;
}

// ---- documents and pages

OSStatus PMSessionBeginCGDocumentNoDialog(PMPrintSession printSession, PMPrintSettings printSettings, PMPageFormat pageFormat)
{
	struct __PMPrintSession* session = asSession(printSession);
	struct __PMPrintSettings* settings = asSettings(printSettings);
	struct __PMPageFormat* format = asPageFormat(pageFormat);

	if (!session)
		return kPMInvalidPrintSession;
	if (!settings)
		return sessionFail(session, kPMInvalidPrintSettings);
	if (!format)
		return sessionFail(session, kPMInvalidPageFormat);
	if (session->inDocument)
		return sessionFail(session, kPMOutOfScope);
	// A printer queue is not there to take the job
	if (settings->destination != kPMDestinationFile || !settings->destinationURL)
		return sessionFail(session, kPMNoDefaultPrinter);

	double width, height;
	orientedPaper(format, &width, &height);
	CGRect mediaBox = CGRectMake(0, 0, width, height);

	session->context = CGPDFContextCreateWithURL(settings->destinationURL, &mediaBox, NULL);
	if (!session->context)
		return sessionFail(session, kPMGeneralError);
	CFRetain(settings);
	session->settings = (PMPrintSettings) settings;
	CFRetain(format);
	session->format = (PMPageFormat) format;
	session->inDocument = true;
	return noErr;
}

OSStatus PMSessionEndDocumentNoDialog(PMPrintSession printSession)
{
	struct __PMPrintSession* session = asSession(printSession);

	if (!session)
		return kPMInvalidPrintSession;
	if (!session->inDocument || session->inPage)
		return sessionFail(session, kPMOutOfScope);
	CGPDFContextClose(session->context);
	CGContextRelease(session->context);
	session->context = NULL;
	session->inDocument = false;
	return noErr;
}

OSStatus PMSessionBeginPageNoDialog(PMPrintSession printSession, PMPageFormat pageFormat, const PMRect* pageFrame)
{
	struct __PMPrintSession* session = asSession(printSession);

	if (!session)
		return kPMInvalidPrintSession;
	if (!session->inDocument || session->inPage)
		return sessionFail(session, kPMOutOfScope);
	if (pageFormat && !asPageFormat(pageFormat))
		return sessionFail(session, kPMInvalidPageFormat);
	CGPDFContextBeginPage(session->context, NULL);
	session->inPage = true;
	return noErr;
}

OSStatus PMSessionEndPageNoDialog(PMPrintSession printSession)
{
	struct __PMPrintSession* session = asSession(printSession);

	if (!session)
		return kPMInvalidPrintSession;
	if (!session->inPage)
		return sessionFail(session, kPMOutOfScope);
	CGPDFContextEndPage(session->context);
	session->inPage = false;
	return noErr;
}

OSStatus PMSessionGetCGGraphicsContext(PMPrintSession printSession, CGContextRef* context)
{
	struct __PMPrintSession* session = asSession(printSession);

	if (!session)
		return kPMInvalidPrintSession;
	if (!context)
		return kPMInvalidParameter;
	if (!session->inPage)
		return sessionFail(session, kPMOutOfScope);
	*context = session->context;
	return noErr;
}

OSStatus PMSessionGetGraphicsContext(PMPrintSession printSession, CFStringRef graphicsContextType, void** graphicsContext)
{
	if (graphicsContextType && CFStringCompare(graphicsContextType, kPMGraphicsContextCoreGraphics, 0) != kCFCompareEqualTo)
		return kPMInvalidParameter;
	return PMSessionGetCGGraphicsContext(printSession, (CGContextRef*) graphicsContext);
}
