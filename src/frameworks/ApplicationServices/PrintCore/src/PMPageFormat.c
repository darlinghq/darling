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

// Page formats: a paper, an orientation and a scale, and the rectangles that these make

#include "PMInternal.h"
#include <string.h>

// ---- the data that keeps a format and data of the application that is stored with it

static CFStringRef extendedKey(OSType identifier)
{
	return CFStringCreateWithFormat(NULL, NULL, CFSTR("%08x"), (unsigned) identifier);
}

OSStatus PMSetPageFormatExtendedData(PMPageFormat pageFormat, OSType dataID, UInt32 size, void* extendedData)
{
	struct __PMPageFormat* format = PMAsPageFormat(pageFormat);
	CFDataRef data;
	CFStringRef key;

	if (!format)
		return kPMInvalidPageFormat;
	if (!extendedData && size)
		return kPMInvalidParameter;
	if (!format->extended)
		format->extended = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	data = CFDataCreate(NULL, extendedData, size);
	key = extendedKey(dataID);
	CFDictionarySetValue(format->extended, key, data);
	CFRelease(key);
	CFRelease(data);
	return noErr;
}

// The size is the size of the buffer on the call and is the size of the data after it
OSStatus PMGetPageFormatExtendedData(PMPageFormat pageFormat, OSType dataID, UInt32* size, void* extendedData)
{
	struct __PMPageFormat* format = PMAsPageFormat(pageFormat);
	CFStringRef key;
	CFDataRef data;

	if (!format)
		return kPMInvalidPageFormat;
	key = extendedKey(dataID);
	data = format->extended ? CFDictionaryGetValue(format->extended, key) : NULL;
	CFRelease(key);
	if (!data)
		return kPMKeyNotFound;
	if (extendedData && size)
		memcpy(extendedData, CFDataGetBytePtr(data), *size < CFDataGetLength(data) ? *size : CFDataGetLength(data));
	if (size)
		*size = CFDataGetLength(data);
	return noErr;
}

// ---- objects

OSStatus PMCreatePageFormat(PMPageFormat* pageFormat)
{
	if (!pageFormat)
		return kPMInvalidParameter;
	*pageFormat = (PMPageFormat) PMNewObject(PMPageFormatTypeID(), sizeof(struct __PMPageFormat));
	return *pageFormat ? noErr : kPMGeneralError;
}

static void setPaper(struct __PMPageFormat* format, struct __PMPaper* paper)
{
	CFRetain(paper);
	if (format->paper)
		CFRelease(format->paper);
	format->paper = paper;
}

static void makeValid(struct __PMPageFormat* format)
{
	format->orientation = format->appliedOrientation = kPMPortrait;
	format->scale = format->appliedScale = 100;
	format->valid = true;
}

OSStatus PMSessionDefaultPageFormat(PMPrintSession printSession, PMPageFormat pageFormat)
{
	struct __PMPageFormat* format = PMAsPageFormat(pageFormat);
	struct __PMPaper* paper;

	if (!PMAsSession(printSession))
		return kPMInvalidPrintSession;
	if (!format)
		return kPMInvalidPageFormat;
	paper = PMCopyDefaultPaper();
	if (!paper)
		return kPMGeneralError;
	setPaper(format, paper);
	CFRelease(paper);
	makeValid(format);
	return noErr;
}

OSStatus PMCreatePageFormatWithPMPaper(PMPageFormat* pageFormat, PMPaper paper)
{
	struct __PMPaper* p = PMAsPaper(paper);
	struct __PMPageFormat* format;

	if (!pageFormat || !p)
		return kPMInvalidParameter;
	format = PMNewObject(PMPageFormatTypeID(), sizeof(struct __PMPageFormat));
	if (!format)
		return kPMGeneralError;
	setPaper(format, p);
	makeValid(format);
	*pageFormat = (PMPageFormat) format;
	return noErr;
}

OSStatus PMGetPageFormatPaper(PMPageFormat pageFormat, PMPaper* paper)
{
	struct __PMPageFormat* format = PMAsPageFormat(pageFormat);

	if (!format || !paper)
		return kPMInvalidParameter;
	*paper = (PMPaper) format->paper;
	return noErr;
}

OSStatus PMCopyPageFormat(PMPageFormat formatSrc, PMPageFormat formatDest)
{
	struct __PMPageFormat* source = PMAsPageFormat(formatSrc);
	struct __PMPageFormat* destination = PMAsPageFormat(formatDest);

	if (!source || !destination)
		return kPMInvalidPageFormat;
	if (source == destination)
		return noErr;
	if (source->paper)
		setPaper(destination, source->paper);
	else if (destination->paper)
	{
		CFRelease(destination->paper);
		destination->paper = NULL;
	}
	destination->valid = source->valid;
	destination->orientation = source->orientation;
	destination->scale = source->scale;
	destination->appliedOrientation = source->appliedOrientation;
	destination->appliedScale = source->appliedScale;
	if (destination->extended)
	{
		CFRelease(destination->extended);
		destination->extended = NULL;
	}
	if (source->extended)
		destination->extended = CFDictionaryCreateMutableCopy(NULL, 0, source->extended);
	return noErr;
}

OSStatus PMSessionValidatePageFormat(PMPrintSession printSession, PMPageFormat pageFormat, Boolean* changed)
{
	struct __PMPageFormat* format = PMAsPageFormat(pageFormat);

	if (!PMAsSession(printSession))
		return kPMInvalidPrintSession;
	if (!format)
		return kPMInvalidPageFormat;
	if (changed)
		*changed = false;
	// QUIRK: a page format that was never made valid by PMSessionDefaultPageFormat stays invalid: the validation
	// fails with -9583. Not documented. Evidence: black-box observation on macOS 10.14.4 and 26.7.3 (tier 2).
	// Test: tests/src/printcore_session.c (test_quirk_unvalidated_format)
	if (!format->valid)
		return kPMObjectNotValidated;
	format->appliedOrientation = format->orientation;
	// A scale that is not positive does not reach the rectangles
	if (format->scale > 0)
		format->appliedScale = format->scale;
	return noErr;
}

// ---- rectangles

// The unprintable margins of the paper as the page has them in an orientation: a page turned to
// landscape has the top of the portrait paper on its left, and so on around
static void orientedMargins(const struct __PMPaper* paper, PMOrientation orientation, PMPaperMargins* m)
{
	const PMPaperMargins* p = &paper->margins;

	switch (orientation)
	{
		case kPMLandscape:
			m->left = p->top; m->top = p->right; m->right = p->bottom; m->bottom = p->left;
			break;
		case kPMReversePortrait:
			m->top = p->bottom; m->left = p->right; m->bottom = p->top; m->right = p->left;
			break;
		case kPMReverseLandscape:
			m->left = p->bottom; m->top = p->left; m->right = p->top; m->bottom = p->right;
			break;
		default:
			*m = *p;
	}
}

void PMFormatOrientedPaper(const struct __PMPageFormat* format, double* width, double* height)
{
	bool landscape = format->appliedOrientation == kPMLandscape || format->appliedOrientation == kPMReverseLandscape;

	*width = landscape ? format->paper->height : format->paper->width;
	*height = landscape ? format->paper->width : format->paper->height;
}

// The page and the paper in the units of the page: the page has its corner at the origin and the paper
// reaches beyond it by the margins. The adjusted rectangles have the orientation and the scale applied
// and the unadjusted ones do not.
static OSStatus formatRects(PMPageFormat pageFormat, PMRect* page, PMRect* paperRect, bool adjusted)
{
	struct __PMPageFormat* format = PMAsPageFormat(pageFormat);
	PMPaperMargins m;
	double width, height, factor = 1;
	PMOrientation orientation = adjusted ? format ? format->appliedOrientation : kPMPortrait : kPMPortrait;

	if (!(page ? page : paperRect))
		return kPMInvalidParameter;
	if (!format)
		return kPMInvalidPageFormat;
	if (!format->valid || !format->paper)
		return kPMObjectNotValidated;
	orientedMargins(format->paper, orientation, &m);
	if (adjusted)
	{
		PMFormatOrientedPaper(format, &width, &height);
		factor = 100 / format->appliedScale;
	}
	else
	{
		width = format->paper->width;
		height = format->paper->height;
	}
	if (page)
	{
		page->top = page->left = 0;
		page->bottom = (height - m.top - m.bottom) * factor;
		page->right = (width - m.left - m.right) * factor;
	}
	if (paperRect)
	{
		paperRect->top = -m.top * factor;
		paperRect->left = -m.left * factor;
		paperRect->bottom = (height - m.top) * factor;
		paperRect->right = (width - m.left) * factor;
	}
	return noErr;
}

OSStatus PMGetAdjustedPageRect(PMPageFormat pageFormat, PMRect* pageRect) { return formatRects(pageFormat, pageRect, NULL, true); }
OSStatus PMGetAdjustedPaperRect(PMPageFormat pageFormat, PMRect* paperRect) { return formatRects(pageFormat, NULL, paperRect, true); }
OSStatus PMGetUnadjustedPageRect(PMPageFormat pageFormat, PMRect* pageRect) { return formatRects(pageFormat, pageRect, NULL, false); }
OSStatus PMGetUnadjustedPaperRect(PMPageFormat pageFormat, PMRect* paperRect) { return formatRects(pageFormat, NULL, paperRect, false); }

OSStatus PMGetOrientation(PMPageFormat pageFormat, PMOrientation* orientation)
{
	struct __PMPageFormat* format = PMAsPageFormat(pageFormat);

	if (!orientation)
		return kPMInvalidParameter;
	if (!format)
		return kPMInvalidPageFormat;
	*orientation = format->orientation ? format->orientation : kPMPortrait;
	return noErr;
}

OSStatus PMSetOrientation(PMPageFormat pageFormat, PMOrientation orientation, Boolean lock)
{
	struct __PMPageFormat* format = PMAsPageFormat(pageFormat);

	if (!format)
		return kPMInvalidPageFormat;
	if (orientation < kPMPortrait || orientation > kPMReverseLandscape)
		return kPMInvalidParameter;
	format->orientation = orientation;
	return noErr;
}

OSStatus PMGetScale(PMPageFormat pageFormat, double* scale)
{
	struct __PMPageFormat* format = PMAsPageFormat(pageFormat);

	if (!scale)
		return kPMInvalidParameter;
	if (!format)
		return kPMInvalidPageFormat;
	*scale = format->scale ? format->scale : 100;
	return noErr;
}

OSStatus PMSetScale(PMPageFormat pageFormat, double scale)
{
	struct __PMPageFormat* format = PMAsPageFormat(pageFormat);

	if (!format)
		return kPMInvalidPageFormat;
	format->scale = scale;
	return noErr;
}

// ---- the data of a format, for documents that keep it

static CFNumberRef number(CFNumberType type, const void* value)
{
	return CFNumberCreate(NULL, type, value);
}

static void setNumber(CFMutableDictionaryRef dictionary, CFStringRef key, CFNumberType type, const void* value)
{
	CFNumberRef n = number(type, value);

	CFDictionarySetValue(dictionary, key, n);
	CFRelease(n);
}

static bool getNumber(CFDictionaryRef dictionary, CFStringRef key, CFNumberType type, void* value)
{
	CFTypeRef n = CFDictionaryGetValue(dictionary, key);

	return n && CFGetTypeID(n) == CFNumberGetTypeID() && CFNumberGetValue(n, type, value);
}

static void setString(CFMutableDictionaryRef dictionary, CFStringRef key, CFStringRef value)
{
	if (value)
		CFDictionarySetValue(dictionary, key, value);
}

static CFStringRef getString(CFDictionaryRef dictionary, CFStringRef key)
{
	CFTypeRef s = CFDictionaryGetValue(dictionary, key);

	return s && CFGetTypeID(s) == CFStringGetTypeID() ? s : NULL;
}

OSStatus PMPageFormatCreateDataRepresentation(PMPageFormat pageFormat, CFDataRef* data, PMDataFormat format)
{
	struct __PMPageFormat* f = PMAsPageFormat(pageFormat);
	CFMutableDictionaryRef dictionary;
	SInt32 orientation;

	if (!f)
		return kPMInvalidPageFormat;
	if (!data)
		return kPMInvalidParameter;
	dictionary = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFDictionarySetValue(dictionary, CFSTR("valid"), f->valid ? kCFBooleanTrue : kCFBooleanFalse);
	if (f->paper)
	{
		CFMutableDictionaryRef paper = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

		setString(paper, CFSTR("id"), f->paper->id);
		setString(paper, CFSTR("ppd"), f->paper->ppdName);
		setString(paper, CFSTR("name"), f->paper->name);
		setNumber(paper, CFSTR("width"), kCFNumberDoubleType, &f->paper->width);
		setNumber(paper, CFSTR("height"), kCFNumberDoubleType, &f->paper->height);
		setNumber(paper, CFSTR("top"), kCFNumberDoubleType, &f->paper->margins.top);
		setNumber(paper, CFSTR("left"), kCFNumberDoubleType, &f->paper->margins.left);
		setNumber(paper, CFSTR("bottom"), kCFNumberDoubleType, &f->paper->margins.bottom);
		setNumber(paper, CFSTR("right"), kCFNumberDoubleType, &f->paper->margins.right);
		CFDictionarySetValue(paper, CFSTR("custom"), f->paper->custom ? kCFBooleanTrue : kCFBooleanFalse);
		CFDictionarySetValue(dictionary, CFSTR("paper"), paper);
		CFRelease(paper);
	}
	orientation = f->orientation;
	setNumber(dictionary, CFSTR("orientation"), kCFNumberSInt32Type, &orientation);
	orientation = f->appliedOrientation;
	setNumber(dictionary, CFSTR("appliedOrientation"), kCFNumberSInt32Type, &orientation);
	setNumber(dictionary, CFSTR("scale"), kCFNumberDoubleType, &f->scale);
	setNumber(dictionary, CFSTR("appliedScale"), kCFNumberDoubleType, &f->appliedScale);
	if (f->extended)
		CFDictionarySetValue(dictionary, CFSTR("extended"), f->extended);
	*data = CFPropertyListCreateData(NULL, dictionary, kCFPropertyListXMLFormat_v1_0, 0, NULL);
	CFRelease(dictionary);
	return *data ? noErr : kPMGeneralError;
}

OSStatus PMPageFormatCreateWithDataRepresentation(CFDataRef data, PMPageFormat* pageFormat)
{
	CFTypeRef list;
	OSStatus status = kPMInvalidObject;

	if (!data || !pageFormat)
		return kPMInvalidParameter;
	list = CFPropertyListCreateWithData(NULL, data, kCFPropertyListImmutable, NULL, NULL);
	if (list && CFGetTypeID(list) == CFDictionaryGetTypeID())
	{
		CFDictionaryRef dictionary = list;
		CFDictionaryRef paperDictionary = CFDictionaryGetValue(dictionary, CFSTR("paper"));
		struct __PMPageFormat* format;

		status = PMCreatePageFormat((PMPageFormat*) &format);
		if (status == noErr)
		{
			SInt32 orientation = kPMPortrait;
			CFDictionaryRef extended = CFDictionaryGetValue(dictionary, CFSTR("extended"));

			if (paperDictionary && CFGetTypeID(paperDictionary) == CFDictionaryGetTypeID())
			{
				PMPaperMargins m = { 0, 0, 0, 0 };
				double width = 0, height = 0;
				struct __PMPaper* paper;
				CFTypeRef custom = CFDictionaryGetValue(paperDictionary, CFSTR("custom"));

				getNumber(paperDictionary, CFSTR("width"), kCFNumberDoubleType, &width);
				getNumber(paperDictionary, CFSTR("height"), kCFNumberDoubleType, &height);
				getNumber(paperDictionary, CFSTR("top"), kCFNumberDoubleType, &m.top);
				getNumber(paperDictionary, CFSTR("left"), kCFNumberDoubleType, &m.left);
				getNumber(paperDictionary, CFSTR("bottom"), kCFNumberDoubleType, &m.bottom);
				getNumber(paperDictionary, CFSTR("right"), kCFNumberDoubleType, &m.right);
				paper = PMNewObject(PMPaperTypeID(), sizeof(struct __PMPaper));
				if (paper)
				{
					CFStringRef s;

					if ((s = getString(paperDictionary, CFSTR("id"))))
						paper->id = CFRetain(s);
					if ((s = getString(paperDictionary, CFSTR("ppd"))))
						paper->ppdName = CFRetain(s);
					if ((s = getString(paperDictionary, CFSTR("name"))))
						paper->name = CFRetain(s);
					paper->width = width;
					paper->height = height;
					paper->margins = m;
					paper->custom = custom == kCFBooleanTrue;
					format->paper = paper;
				}
			}
			getNumber(dictionary, CFSTR("orientation"), kCFNumberSInt32Type, &orientation);
			format->orientation = orientation >= kPMPortrait && orientation <= kPMReverseLandscape ? orientation : kPMPortrait;
			format->appliedOrientation = format->orientation;
			getNumber(dictionary, CFSTR("appliedOrientation"), kCFNumberSInt32Type, &orientation);
			if (orientation >= kPMPortrait && orientation <= kPMReverseLandscape)
				format->appliedOrientation = orientation;
			format->scale = 100;
			getNumber(dictionary, CFSTR("scale"), kCFNumberDoubleType, &format->scale);
			format->appliedScale = format->scale;
			getNumber(dictionary, CFSTR("appliedScale"), kCFNumberDoubleType, &format->appliedScale);
			format->valid = CFDictionaryGetValue(dictionary, CFSTR("valid")) == kCFBooleanTrue && format->paper && format->paper->width > 0 && format->appliedScale > 0;
			if (extended && CFGetTypeID(extended) == CFDictionaryGetTypeID())
				format->extended = CFDictionaryCreateMutableCopy(NULL, 0, extended);
			*pageFormat = (PMPageFormat) format;
		}
	}
	if (list)
		CFRelease(list);
	return status;
}

#ifndef __LP64__
// The flattened form is the data of the format, in a handle
OSStatus PMFlattenPageFormat(PMPageFormat pageFormat, Handle* flatFormat)
{
	CFDataRef data;
	OSStatus status;

	if (!flatFormat)
		return kPMInvalidParameter;
	status = PMPageFormatCreateDataRepresentation(pageFormat, &data, kPMDataFormatXMLDefault);
	if (status == noErr)
	{
		Handle handle = NewHandle(CFDataGetLength(data));

		if (handle)
		{
			memcpy(*handle, CFDataGetBytePtr(data), CFDataGetLength(data));
			*flatFormat = handle;
		}
		else
			status = kPMGeneralError;
		CFRelease(data);
	}
	return status;
}

OSStatus PMUnflattenPageFormat(Handle flatFormat, PMPageFormat* pageFormat)
{
	CFDataRef data;
	OSStatus status;

	if (!pageFormat || !flatFormat)
		return kPMInvalidParameter;
	data = CFDataCreate(NULL, (const UInt8*) *flatFormat, GetHandleSize(flatFormat));
	status = PMPageFormatCreateWithDataRepresentation(data, pageFormat);
	CFRelease(data);
	return status;
}
#endif
