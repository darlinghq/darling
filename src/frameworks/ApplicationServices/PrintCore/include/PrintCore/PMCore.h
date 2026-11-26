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

#ifndef _PrintCore_PMCore_H_
#define _PrintCore_PMCore_H_

#include <CoreGraphics/CoreGraphics.h>
#include <PrintCore/PMDefinitions.h>
#include <PrintCore/PMErrors.h>

#ifdef __cplusplus
extern "C" {
#endif

// Printing objects are reference counted
OSStatus PMRetain(PMObject object);
OSStatus PMRelease(PMObject object);

// Sessions
OSStatus PMCreateSession(PMPrintSession* printSession);
OSStatus PMSessionError(PMPrintSession printSession);

OSStatus PMSessionBeginCGDocumentNoDialog(PMPrintSession printSession, PMPrintSettings printSettings, PMPageFormat pageFormat);
OSStatus PMSessionEndDocumentNoDialog(PMPrintSession printSession);
OSStatus PMSessionBeginPageNoDialog(PMPrintSession printSession, PMPageFormat pageFormat, const PMRect* pageFrame);
OSStatus PMSessionEndPageNoDialog(PMPrintSession printSession);
OSStatus PMSessionGetCGGraphicsContext(PMPrintSession printSession, CGContextRef* context);

OSStatus PMSessionGetDestinationType(PMPrintSession printSession, PMPrintSettings printSettings, PMDestinationType* destTypeP);
OSStatus PMSessionSetDestination(PMPrintSession printSession, PMPrintSettings printSettings, PMDestinationType destType, CFStringRef destFormat, CFURLRef destLocation);
OSStatus PMSessionCopyOutputFormatList(PMPrintSession printSession, PMDestinationType destType, CFArrayRef* documentFormatP);

// The graphics context a session hands to the application, as the formats of the document it makes
OSStatus PMSessionSetDocumentFormatGeneration(PMPrintSession printSession, CFStringRef docFormat, CFArrayRef graphicsContextTypes, CFTypeRef options);
OSStatus PMSessionGetGraphicsContext(PMPrintSession printSession, CFStringRef graphicsContextType, void** graphicsContext);

// Page formats
OSStatus PMCreatePageFormat(PMPageFormat* pageFormat);
OSStatus PMSessionDefaultPageFormat(PMPrintSession printSession, PMPageFormat pageFormat);
OSStatus PMSessionValidatePageFormat(PMPrintSession printSession, PMPageFormat pageFormat, Boolean* changed);
OSStatus PMGetAdjustedPageRect(PMPageFormat pageFormat, PMRect* pageRect);
OSStatus PMGetAdjustedPaperRect(PMPageFormat pageFormat, PMRect* paperRect);
OSStatus PMGetUnadjustedPageRect(PMPageFormat pageFormat, PMRect* pageRect);
OSStatus PMGetUnadjustedPaperRect(PMPageFormat pageFormat, PMRect* paperRect);
OSStatus PMGetOrientation(PMPageFormat pageFormat, PMOrientation* orientation);
OSStatus PMSetOrientation(PMPageFormat pageFormat, PMOrientation orientation, Boolean lock);
OSStatus PMGetScale(PMPageFormat pageFormat, double* scale);
OSStatus PMSetScale(PMPageFormat pageFormat, double scale);

#ifndef __LP64__
// The Memory Manager handle of the flattened page format, which an application keeps in its documents
OSStatus PMFlattenPageFormat(PMPageFormat pageFormat, Handle* flatFormat);
OSStatus PMUnflattenPageFormat(Handle flatFormat, PMPageFormat* pageFormat);
#endif

// Print settings
OSStatus PMCreatePrintSettings(PMPrintSettings* printSettings);
OSStatus PMSessionDefaultPrintSettings(PMPrintSession printSession, PMPrintSettings printSettings);
OSStatus PMSessionValidatePrintSettings(PMPrintSession printSession, PMPrintSettings printSettings, Boolean* changed);
OSStatus PMGetCopies(PMPrintSettings printSettings, UInt32* copies);
OSStatus PMSetCopies(PMPrintSettings printSettings, UInt32 copies, Boolean lock);
OSStatus PMGetFirstPage(PMPrintSettings printSettings, UInt32* first);
OSStatus PMSetFirstPage(PMPrintSettings printSettings, UInt32 first, Boolean lock);
OSStatus PMGetLastPage(PMPrintSettings printSettings, UInt32* last);
OSStatus PMSetLastPage(PMPrintSettings printSettings, UInt32 last, Boolean lock);
OSStatus PMGetPageRange(PMPrintSettings printSettings, UInt32* minPage, UInt32* maxPage);
OSStatus PMSetPageRange(PMPrintSettings printSettings, UInt32 minPage, UInt32 maxPage);
OSStatus PMPrintSettingsGetJobName(PMPrintSettings printSettings, CFStringRef* name);
OSStatus PMPrintSettingsSetJobName(PMPrintSettings printSettings, CFStringRef name);
OSStatus PMSetJobNameCFString(PMPrintSettings printSettings, CFStringRef name);

#ifdef __cplusplus
}
#endif

#endif
