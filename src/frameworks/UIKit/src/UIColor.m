/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Team

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
*/

#import <UIKit/UIColor.h>
#import <AppKit/NSColor.h>

#import "UIKitPrivate.h"

@implementation UIColor

+ (UIColor *)colorWithRed:(CGFloat)red green:(CGFloat)green blue:(CGFloat)blue alpha:(CGFloat)alpha
{
	UIColor *color = [[UIColor alloc] init];
	color->_red = red;
	color->_green = green;
	color->_blue = blue;
	color->_alpha = alpha;
	return color;
}

+ (UIColor *)colorWithWhite:(CGFloat)white alpha:(CGFloat)alpha
{
	return [self colorWithRed:white green:white blue:white alpha:alpha];
}

+ (UIColor *)blackColor { return [self colorWithWhite:0 alpha:1]; }
+ (UIColor *)whiteColor { return [self colorWithWhite:1 alpha:1]; }
+ (UIColor *)clearColor { return [self colorWithWhite:0 alpha:0]; }
+ (UIColor *)redColor { return [self colorWithRed:1 green:0 blue:0 alpha:1]; }
+ (UIColor *)greenColor { return [self colorWithRed:0 green:1 blue:0 alpha:1]; }
+ (UIColor *)blueColor { return [self colorWithRed:0 green:0 blue:1 alpha:1]; }

- (UIColor *)colorWithAlphaComponent:(CGFloat)alpha
{
	return [UIColor colorWithRed:_red green:_green blue:_blue alpha:alpha];
}

- (CGFloat)redComponent { return _red; }
- (CGFloat)greenComponent { return _green; }
- (CGFloat)blueComponent { return _blue; }
- (CGFloat)alphaComponent { return _alpha; }

- (NSColor *)nsColor
{
	return [NSColor colorWithCalibratedRed:_red green:_green blue:_blue alpha:_alpha];
}

- (void)set
{
	[[self nsColor] set];
}

- (void)setFill
{
	[[self nsColor] setFill];
}

@end
