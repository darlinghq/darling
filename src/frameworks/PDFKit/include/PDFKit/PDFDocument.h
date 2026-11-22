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
#import <Foundation/Foundation.h>

typedef NSString *PDFDocumentAttribute;
typedef NSString *PDFDocumentWriteOption;

PDFKIT_EXPORT PDFDocumentAttribute const PDFDocumentAuthorAttribute;
PDFKIT_EXPORT PDFDocumentAttribute const PDFDocumentCreationDateAttribute;
PDFKIT_EXPORT PDFDocumentAttribute const PDFDocumentCreatorAttribute;
PDFKIT_EXPORT PDFDocumentAttribute const PDFDocumentKeywordsAttribute;
PDFKIT_EXPORT PDFDocumentAttribute const PDFDocumentModificationDateAttribute;
PDFKIT_EXPORT PDFDocumentAttribute const PDFDocumentSubjectAttribute;
PDFKIT_EXPORT PDFDocumentAttribute const PDFDocumentTitleAttribute;

PDFKIT_EXPORT PDFDocumentWriteOption const PDFDocumentBurnInAnnotationsOption;
PDFKIT_EXPORT PDFDocumentWriteOption const PDFDocumentOptimizeImagesForScreenOption;
PDFKIT_EXPORT PDFDocumentWriteOption const PDFDocumentSaveImagesAsJPEGOption;
PDFKIT_EXPORT PDFDocumentWriteOption const PDFDocumentSaveTextFromOCROption;

PDFKIT_EXPORT NSNotificationName const PDFDocumentDidBeginFindNotification;
PDFKIT_EXPORT NSNotificationName const PDFDocumentDidBeginPageFindNotification;
PDFKIT_EXPORT NSNotificationName const PDFDocumentDidEndFindNotification;
PDFKIT_EXPORT NSNotificationName const PDFDocumentDidEndPageFindNotification;
PDFKIT_EXPORT NSNotificationName const PDFDocumentDidUnlockNotification;

@interface PDFDocument : NSObject

@end
