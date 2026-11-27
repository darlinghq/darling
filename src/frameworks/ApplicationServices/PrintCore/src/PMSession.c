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

// The print session and the calls that make a document in it. There is no print queue: the document goes
// to a PDF file, the destination the application (or the print dialog) sets with PMSessionSetDestination().

#include "PMInternal.h"
#include <ApplicationServices/ApplicationServices.h>
#include <string.h>

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
// error of PMSessionError() stays noErr. Not documented. Evidence: black-box observation on macOS 10.14.4
// (tier 2). Test: tests/src/printcore_session.c (test_quirk_session_error_stays)
static OSStatus sessionFail(struct __PMPrintSession* session, OSStatus error)
{
	return error;
}

OSStatus PMCreateSession(PMPrintSession* printSession)
{
	if (!printSession)
		return kPMInvalidParameter;
	*printSession = (PMPrintSession) PMNewObject(PMPrintSessionTypeID(), sizeof(struct __PMPrintSession));
	return *printSession ? noErr : kPMGeneralError;
}

OSStatus PMSessionError(PMPrintSession printSession)
{
	struct __PMPrintSession* session = PMAsSession(printSession);

	return session ? session->error : kPMInvalidPrintSession;
}

// The data that the layers above keep in a session, by the key of the layer
OSStatus PMSessionSetDataInSession(PMPrintSession printSession, CFStringRef key, CFTypeRef data)
{
	struct __PMPrintSession* session = PMAsSession(printSession);

	if (!session)
		return kPMInvalidPrintSession;
	if (!key)
		return kPMInvalidParameter;
	if (!session->data)
		session->data = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	if (data)
		CFDictionarySetValue(session->data, key, data);
	else
		CFDictionaryRemoveValue(session->data, key);
	return noErr;
}

OSStatus PMSessionGetDataFromSession(PMPrintSession printSession, CFStringRef key, CFTypeRef* data)
{
	struct __PMPrintSession* session = PMAsSession(printSession);

	if (!session)
		return kPMInvalidPrintSession;
	if (!key || !data)
		return kPMInvalidParameter;
	*data = session->data ? CFDictionaryGetValue(session->data, key) : NULL;
	return *data ? noErr : kPMKeyNotFound;
}

OSStatus PMSessionCopyOutputFormatList(PMPrintSession printSession, PMDestinationType destType, CFArrayRef* documentFormatP)
{
	const void* formats[] = { kPMDocumentFormatPDF };

	if (!PMAsSession(printSession))
		return kPMInvalidPrintSession;
	if (!documentFormatP)
		return kPMInvalidParameter;
	*documentFormatP = destType == kPMDestinationFile ? CFArrayCreate(NULL, formats, 1, &kCFTypeArrayCallBacks) : NULL;
	return noErr;
}

OSStatus PMSessionSetDocumentFormatGeneration(PMPrintSession printSession, CFStringRef docFormat, CFArrayRef graphicsContextTypes, CFTypeRef options)
{
	struct __PMPrintSession* session = PMAsSession(printSession);

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
	struct __PMPrintSession* session = PMAsSession(printSession);
	struct __PMPrintSettings* settings = PMAsSettings(printSettings);
	struct __PMPageFormat* format = PMAsPageFormat(pageFormat);

	if (!session)
		return kPMInvalidPrintSession;
	if (!settings)
		return sessionFail(session, kPMInvalidPrintSettings);
	if (!format)
		return sessionFail(session, kPMInvalidPageFormat);
	if (session->inDocument)
		return sessionFail(session, kPMOutOfScope);
	if (!format->valid || !format->paper)
		return sessionFail(session, kPMObjectNotValidated);
	// A printer queue is not there to take the job
	if (settings->destination != kPMDestinationFile || !settings->destinationURL)
		return sessionFail(session, kPMNoDefaultPrinter);

	double width, height;
	PMFormatOrientedPaper(format, &width, &height);
	CGRect mediaBox = CGRectMake(0, 0, width, height);

	// The job name of the print settings is the title of the document
	CFDictionaryRef info = NULL;
	if (settings->jobName)
	{
		const void* key = kCGPDFContextTitle;
		const void* value = settings->jobName;
		info = CFDictionaryCreate(NULL, &key, &value, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	}
	session->context = CGPDFContextCreateWithURL(settings->destinationURL, &mediaBox, info);
	if (info)
		CFRelease(info);
	if (!session->context)
		return sessionFail(session, kPMGeneralError);
	CFRetain(settings);
	if (session->settings)
		CFRelease(session->settings);
	session->settings = (PMPrintSettings) settings;
	CFRetain(format);
	if (session->format)
		CFRelease(session->format);
	session->format = (PMPageFormat) format;
	session->inDocument = true;
	return noErr;
}

OSStatus PMSessionEndDocumentNoDialog(PMPrintSession printSession)
{
	struct __PMPrintSession* session = PMAsSession(printSession);

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

// A page has the paper of the page format it is begun with, in the orientation of the format and not
// scaled, or the paper of the format of the document when none is given. Evidence: black-box observation on
// macOS 26.7.3 (tier 2).
OSStatus PMSessionBeginPageNoDialog(PMPrintSession printSession, PMPageFormat pageFormat, const PMRect* pageFrame)
{
	struct __PMPrintSession* session = PMAsSession(printSession);
	struct __PMPageFormat* format;

	if (!session)
		return kPMInvalidPrintSession;
	if (!session->inDocument || session->inPage)
		return sessionFail(session, kPMOutOfScope);
	format = pageFormat ? PMAsPageFormat(pageFormat) : (struct __PMPageFormat*) session->format;
	if (!format)
		return sessionFail(session, kPMInvalidPageFormat);
	if (!format->valid || !format->paper)
		return sessionFail(session, kPMObjectNotValidated);

	double width, height;
	PMFormatOrientedPaper(format, &width, &height);
	CGRect mediaBox = CGRectMake(0, 0, width, height);
	CFDataRef box = CFDataCreate(NULL, (const UInt8*) &mediaBox, sizeof(mediaBox));
	CFDictionaryRef pageInfo = CFDictionaryCreate(NULL, (const void**) &kCGPDFContextMediaBox, (const void**) &box, 1,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

	CGPDFContextBeginPage(session->context, pageInfo);
	CFRelease(pageInfo);
	CFRelease(box);
	session->inPage = true;
	return noErr;
}

OSStatus PMSessionEndPageNoDialog(PMPrintSession printSession)
{
	struct __PMPrintSession* session = PMAsSession(printSession);

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
	struct __PMPrintSession* session = PMAsSession(printSession);

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
