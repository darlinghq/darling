/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Team

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
*/

#ifndef UIKIT_UISCREEN_H
#define UIKIT_UISCREEN_H

#import <Foundation/NSObject.h>
#import <CoreGraphics/CGGeometry.h>

@interface UIScreen : NSObject

+ (UIScreen *)mainScreen;

@property (nonatomic, readonly) CGRect bounds;
@property (nonatomic, readonly) CGFloat scale;
@property (nonatomic, readonly) CGRect nativeBounds;

@end

#endif /* UIKIT_UISCREEN_H */
