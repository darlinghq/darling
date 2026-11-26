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

#import <PDFKit/PDFAnnotation.h>
#import <darling-stub-forwarding.h>

// The documentation gives no string values for these annotation keys and subtypes. The values are
// the ISO 32000 annotation dictionary key names (Border, C, DA, P, Parent, Popup, Subtype) and
// subtype names (Highlight, StrikeOut, Underline) with a leading slash, as macOS 26.7.3 and
// 10.14.4 report for these constants (black-box observation).
// Test: tests/src/pdfkit_constants.m
PDFAnnotationKey PDFAnnotationKeyBorder = @"/Border";
PDFAnnotationKey PDFAnnotationKeyColor = @"/C";
PDFAnnotationKey PDFAnnotationKeyDefaultAppearance = @"/DA";
PDFAnnotationKey PDFAnnotationKeyPage = @"/P";
PDFAnnotationKey PDFAnnotationKeyParent = @"/Parent";
PDFAnnotationKey PDFAnnotationKeyPopup = @"/Popup";
PDFAnnotationKey PDFAnnotationKeySubtype = @"/Subtype";

PDFAnnotationSubtype PDFAnnotationSubtypeHighlight = @"/Highlight";
PDFAnnotationSubtype PDFAnnotationSubtypeStrikeOut = @"/StrikeOut";
PDFAnnotationSubtype PDFAnnotationSubtypeUnderline = @"/Underline";

@implementation PDFAnnotation

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
