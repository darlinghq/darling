/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Team

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
*/

#ifndef UIKIT_UIFONT_H
#define UIKIT_UIFONT_H

#import <Foundation/NSObject.h>
#import <CoreGraphics/CGGeometry.h>

@class NSString;

@interface UIFont : NSObject {
@private
	CGFloat _pointSize;
	NSString *_fontName;
	BOOL _bold;
}

+ (UIFont *)systemFontOfSize:(CGFloat)size;
+ (UIFont *)boldSystemFontOfSize:(CGFloat)size;
+ (UIFont *)fontWithName:(NSString *)name size:(CGFloat)size;

@property (nonatomic, readonly) CGFloat pointSize;
@property (nonatomic, readonly) NSString *fontName;
@property (nonatomic, readonly) CGFloat ascender;
@property (nonatomic, readonly) CGFloat descender;

@end

#endif /* UIKIT_UIFONT_H */
