/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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

#import <PDFKit/PDFKitExport.h>
#include <Foundation/Foundation.h>

typedef NSString * const PDFAnnotationKey;
typedef NSString * const PDFAnnotationSubtype;

PDFKIT_EXPORT PDFAnnotationKey PDFAnnotationKeyBorder;
PDFKIT_EXPORT PDFAnnotationKey PDFAnnotationKeyColor;
PDFKIT_EXPORT PDFAnnotationKey PDFAnnotationKeyDefaultAppearance;
PDFKIT_EXPORT PDFAnnotationKey PDFAnnotationKeyPage;
PDFKIT_EXPORT PDFAnnotationKey PDFAnnotationKeyParent;
PDFKIT_EXPORT PDFAnnotationKey PDFAnnotationKeyPopup;
PDFKIT_EXPORT PDFAnnotationKey PDFAnnotationKeySubtype;

PDFKIT_EXPORT PDFAnnotationSubtype PDFAnnotationSubtypeHighlight;
PDFKIT_EXPORT PDFAnnotationSubtype PDFAnnotationSubtypeStrikeOut;
PDFKIT_EXPORT PDFAnnotationSubtype PDFAnnotationSubtypeUnderline;

@interface PDFAnnotation : NSObject

@end
