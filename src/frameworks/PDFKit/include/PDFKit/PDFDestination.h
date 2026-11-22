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
#include <CoreGraphics/CoreGraphics.h>

PDFKIT_EXPORT const CGFloat kPDFDestinationUnspecifiedValue;

@class PDFPage;

NS_ASSUME_NONNULL_BEGIN

// A position on a PDF page, given in page space (72 points per inch, origin at the page's lower-left corner).
@interface PDFDestination : NSObject
{
@private
	PDFPage* _page; // a weak reference, see PDFDestination.m
	CGPoint _point;
}

- (instancetype)initWithPage:(PDFPage*)page atPoint:(CGPoint)point;

// The page is a weak reference in PDFKit's interface, so it is nil once the page is gone.
@property (nonatomic, readonly, nullable) PDFPage* page;
@property (nonatomic, readonly) CGPoint point;

@end

NS_ASSUME_NONNULL_END
