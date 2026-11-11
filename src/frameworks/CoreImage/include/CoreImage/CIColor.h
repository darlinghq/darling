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

@interface CIColor : NSObject {
    /* Named as <QuartzCore/CIColor.h> names it, so a subclass built against either
       header finds the ivar. See the commit body. */
    CGColorRef _cgColor;
}

+ (CIColor *) colorWithCGColor: (CGColorRef) cgColor;

+ (CIColor *) colorWithRed: (CGFloat) red
                     green: (CGFloat) green
                      blue: (CGFloat) blue;

+ (CIColor *) colorWithRed: (CGFloat) red
                     green: (CGFloat) green
                      blue: (CGFloat) blue
                     alpha: (CGFloat) alpha;

- initWithCGColor: (CGColorRef) cgColor;

- (size_t) numberOfComponents;
- (CGColorSpaceRef) colorSpace;
- (const CGFloat *) components;

- (CGFloat) red;
- (CGFloat) green;
- (CGFloat) blue;
- (CGFloat) alpha;

@end
