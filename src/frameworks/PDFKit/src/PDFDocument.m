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

// GUESS: the documentation gives no string values for these three attributes; the values follow
// the existing attributes above (symbol name without the PDFDocument prefix and Attribute suffix).
// The PDF Info dictionary key for the modification date in ISO 32000 is ModDate; that spelling
// is the alternative if the symbol-name form turns out wrong.
PDFDocumentAttribute const PDFDocumentCreatorAttribute = @"Creator";
PDFDocumentAttribute const PDFDocumentModificationDateAttribute = @"ModificationDate";
PDFDocumentAttribute const PDFDocumentSubjectAttribute = @"Subject";

// GUESS: the documentation gives no string values for these write options; the values are the
// symbol name without the PDFDocument prefix and Option suffix, as for the attributes above.
PDFDocumentWriteOption const PDFDocumentBurnInAnnotationsOption = @"BurnInAnnotations";
PDFDocumentWriteOption const PDFDocumentOptimizeImagesForScreenOption = @"OptimizeImagesForScreen";
PDFDocumentWriteOption const PDFDocumentSaveImagesAsJPEGOption = @"SaveImagesAsJPEG";
PDFDocumentWriteOption const PDFDocumentSaveTextFromOCROption = @"SaveTextFromOCR";

// GUESS: the documentation gives no string values for these notification names; the values
// are the symbol names, as for the other framework notifications in Darling.
NSNotificationName const PDFDocumentDidBeginFindNotification = @"PDFDocumentDidBeginFindNotification";
NSNotificationName const PDFDocumentDidBeginPageFindNotification = @"PDFDocumentDidBeginPageFindNotification";
NSNotificationName const PDFDocumentDidEndFindNotification = @"PDFDocumentDidEndFindNotification";
NSNotificationName const PDFDocumentDidEndPageFindNotification = @"PDFDocumentDidEndPageFindNotification";
NSNotificationName const PDFDocumentDidUnlockNotification = @"PDFDocumentDidUnlockNotification";

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
