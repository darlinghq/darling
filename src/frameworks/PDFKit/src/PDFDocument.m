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

#import <PDFKit/PDFDocument.h>
#import <darling-stub-forwarding.h>

PDFDocumentAttribute const PDFDocumentAuthorAttribute = @"Author";
PDFDocumentAttribute const PDFDocumentCreationDateAttribute = @"CreationDate";
PDFDocumentAttribute const PDFDocumentKeywordsAttribute = @"Keywords";
PDFDocumentAttribute const PDFDocumentTitleAttribute = @"Title";

// The documentation gives no string values for these three attributes. They are the PDF Info
// dictionary key names (Creator, Subject and, as in ISO 32000, ModDate), as macOS 26.7.3 and
// 10.14.4 report (black-box observation). Test: tests/src/pdfkit_constants.m
PDFDocumentAttribute const PDFDocumentCreatorAttribute = @"Creator";
PDFDocumentAttribute const PDFDocumentModificationDateAttribute = @"ModDate";
PDFDocumentAttribute const PDFDocumentSubjectAttribute = @"Subject";

// The documentation gives no string values for these write options. macOS 26.7.3 reports the symbol
// name as the value, like the other PDFDocument options (black-box observation); the constants are
// absent on 10.14.4 and are provided here regardless. Test: tests/src/pdfkit_constants.m
PDFDocumentWriteOption const PDFDocumentBurnInAnnotationsOption = @"PDFDocumentBurnInAnnotationsOption";
PDFDocumentWriteOption const PDFDocumentOptimizeImagesForScreenOption = @"PDFDocumentOptimizeImagesForScreenOption";
PDFDocumentWriteOption const PDFDocumentSaveImagesAsJPEGOption = @"PDFDocumentSaveImagesAsJPEGOption";
PDFDocumentWriteOption const PDFDocumentSaveTextFromOCROption = @"PDFDocumentSaveTextFromOCROption";

// The documentation gives no string values for these notification names. They are the strings macOS
// 26.7.3 and 10.14.4 report for these constants (black-box observation); they do not follow the
// symbol names. Test: tests/src/pdfkit_constants.m
NSNotificationName const PDFDocumentDidBeginFindNotification = @"PDFDidBeginDocumentFind";
NSNotificationName const PDFDocumentDidBeginPageFindNotification = @"PDFDidBeginPageFind";
NSNotificationName const PDFDocumentDidEndFindNotification = @"PDFDidEndDocumentFind";
NSNotificationName const PDFDocumentDidEndPageFindNotification = @"PDFDidEndPageFind";
NSNotificationName const PDFDocumentDidUnlockNotification = @"PDFDocumentDidUnlock";

@implementation PDFDocument

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

@end
