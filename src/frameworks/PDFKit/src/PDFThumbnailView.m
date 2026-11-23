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

#import <PDFKit/PDFThumbnailView.h>
#import <Foundation/NSRaise.h>
#import <AppKit/NSColor.h>
#import <darling-stub-forwarding.h>

// The documentation gives no string value for this notification name; macOS 26.7.3 and 10.14.4 report
// the symbol name (black-box observation). Test: tests/src/pdfkit_constants.m
NSString * const PDFThumbnailViewDocumentEditedNotification = @"PDFThumbnailViewDocumentEditedNotification";

@implementation PDFThumbnailView

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

- (void)setBackgroundColor:(NSColor *)color
{
    NSUnimplementedMethod();
}

@end
