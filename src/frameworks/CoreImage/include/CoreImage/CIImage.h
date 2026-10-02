/*
 This file is part of Darling.

 Copyright (C) 2017 Lubos Dolezel

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

#include <Foundation/Foundation.h>
#include <CoreGraphics/CoreGraphics.h>

typedef int CIFormat;

extern const CIFormat kCIFormatARGB8;
extern const CIFormat kCIFormatRGBA8;
extern const CIFormat kCIFormatBGRA8;
extern const CIFormat kCIFormatABGR8;
extern const CIFormat kCIFormatRGBAh;
extern const CIFormat kCIFormatRGBA16;
extern const CIFormat kCIFormatRGBAf;

@interface CIImage : NSObject {
    /* Named as <QuartzCore/CIImage.h> names it, so a subclass built against either
       header finds the ivar. See the commit body. */
    CGImageRef _cgImage;
}

+ (CIImage *) emptyImage;

- initWithCGImage: (CGImageRef) cgImage;

- (CGRect) extent;

@end
