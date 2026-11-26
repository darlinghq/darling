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

// The objects of the Printing Manager, shared by the files of PrintCore (not installed)
#ifndef _PrintCore_PMInternal_H_
#define _PrintCore_PMInternal_H_

#include <PrintCore/PrintCore.h>
#include <CoreFoundation/CFRuntime.h>
#include <stdbool.h>

enum {
	// The error of a call on an object that has not been made valid by its session
	kPMObjectNotValidated = -9583,
	// The error of a key that the object does not have
	kPMKeyNotFound = -9589
};

struct __PMPaper
{
	CFRuntimeBase base;
	CFStringRef id, ppdName, printerID, name;
	double width, height;	// in points, in portrait
	PMPaperMargins margins;	// the unprintable area, in portrait
	Boolean custom;
};

struct __PMPageFormat
{
	CFRuntimeBase base;
	Boolean valid;
	struct __PMPaper* paper;
	PMOrientation orientation;
	double scale;	// in percent
	// QUIRK: the orientation and the scale set on a page format show in its adjusted rectangles only after
	// the format has been validated (PMSessionValidatePageFormat); until then the rectangles are the ones of
	// the last validation. Not documented. Evidence: black-box observation on macOS 10.14.4 and 26.7.3
	// (tier 2). Test: tests/src/printcore_session.c (test_quirk_orientation_after_validate)
	PMOrientation appliedOrientation;
	double appliedScale;
	CFMutableDictionaryRef extended;	// the data of PMSetPageFormatExtendedData, by identifier
};

// The keys that print settings have (bits of the keys field)
enum {
	kPMKeyCopies = 1 << 0,
	kPMKeyFirstPage = 1 << 1,
	kPMKeyLastPage = 1 << 2,
	kPMKeyPageRange = 1 << 3,
	kPMKeyCollate = 1 << 4,
	kPMKeyDuplex = 1 << 5
};

struct __PMPrintSettings
{
	CFRuntimeBase base;
	UInt32 keys;
	UInt32 first, last, minPage, maxPage, copies;
	Boolean collate;
	UInt32 duplex;
	CFStringRef jobName;
	PMDestinationType destination;
	CFStringRef destinationFormat;
	CFURLRef destinationURL;
	CFMutableDictionaryRef values;	// PMPrintSettingsSetValue
	CFMutableDictionaryRef extended;
};

struct __PMPrinter
{
	CFRuntimeBase base;
	CFStringRef id, name;
	CFArrayRef papers;
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
	CFMutableDictionaryRef data;	// PMSessionSetDataInSession
};

CFTypeID PMPaperTypeID(void);
CFTypeID PMPageFormatTypeID(void);
CFTypeID PMPrintSettingsTypeID(void);
CFTypeID PMPrintSessionTypeID(void);
CFTypeID PMPrinterTypeID(void);
void PMRegisterClasses(void);
void* PMNewObject(CFTypeID type, size_t size);

// The paper that a page format has by default and the papers of the generic printer
struct __PMPaper* PMCopyDefaultPaper(void);
CFArrayRef PMGenericPaperList(void);
PMPrinter PMGenericPrinter(void);

// The orientation of a page format, the rotated margins and the sides in it (before the scale)
void PMFormatOrientedPaper(const struct __PMPageFormat* format, double* width, double* height);

static inline struct __PMPageFormat* PMAsPageFormat(PMPageFormat format)
{
	return format && CFGetTypeID(format) == PMPageFormatTypeID() ? (struct __PMPageFormat*) format : NULL;
}

static inline struct __PMPrintSettings* PMAsSettings(PMPrintSettings settings)
{
	return settings && CFGetTypeID(settings) == PMPrintSettingsTypeID() ? (struct __PMPrintSettings*) settings : NULL;
}

static inline struct __PMPrintSession* PMAsSession(PMPrintSession session)
{
	return session && CFGetTypeID(session) == PMPrintSessionTypeID() ? (struct __PMPrintSession*) session : NULL;
}

static inline struct __PMPaper* PMAsPaper(PMPaper paper)
{
	return paper && CFGetTypeID(paper) == PMPaperTypeID() ? (struct __PMPaper*) paper : NULL;
}

#endif
