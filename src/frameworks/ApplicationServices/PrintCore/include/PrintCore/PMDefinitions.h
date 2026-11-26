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

#ifndef _PrintCore_PMDefinitions_H_
#define _PrintCore_PMDefinitions_H_

#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>

#ifdef __cplusplus
extern "C" {
#endif

// The printing objects
typedef const void* PMObject;
typedef struct OpaquePMPrintSettings* PMPrintSettings;
typedef struct OpaquePMPageFormat* PMPageFormat;
typedef struct OpaquePMPrintSession* PMPrintSession;
typedef struct OpaquePMPrinter* PMPrinter;
typedef struct OpaquePMPaper* PMPaper;

enum {
	kPMCancel = 0x0080
};

#define kPMNoData NULL
#define kPMDontWantSize NULL
#define kPMDontWantData NULL
#define kPMDontWantBoolean NULL
#define kPMNoReference NULL
#define kPMNoPrintSettings ((PMPrintSettings) NULL)
#define kPMNoPageFormat ((PMPageFormat) NULL)

typedef UInt16 PMDestinationType;
enum {
	kPMDestinationInvalid = 0,
	kPMDestinationPrinter = 1,
	kPMDestinationFile = 2,
	kPMDestinationFax = 3,
	kPMDestinationPreview = 4,
	kPMDestinationProcessPDF = 5
};

typedef UInt16 PMOrientation;
enum {
	kPMPortrait = 1,
	kPMLandscape = 2,
	kPMReversePortrait = 3,
	kPMReverseLandscape = 4
};

enum {
	kPMPrintAllPages = -1
};

typedef UInt32 PMDuplexMode;
enum {
	kPMDuplexNone = 0x0001,
	kPMDuplexNoTumble = 0x0002,
	kPMDuplexTumble = 0x0003,
	kPMSimplexTumble = 0x0004
};

typedef UInt32 PMDataFormat;
enum {
	kPMDataFormatXMLDefault = 0,
	kPMDataFormatXMLMinimal = 1,
	kPMDataFormatXMLCompressed = 2
};

// The unprintable area of a paper
struct PMPaperMargins {
	double top;
	double left;
	double bottom;
	double right;
};
typedef struct PMPaperMargins PMPaperMargins;

struct PMRect {
	double top;
	double left;
	double bottom;
	double right;
};
typedef struct PMRect PMRect;

#define kPMDocumentFormatPDF CFSTR("application/pdf")
#define kPMGraphicsContextCoreGraphics CFSTR("com.apple.graphicscontext.coregraphics")

#ifdef __cplusplus
}
#endif

#endif
