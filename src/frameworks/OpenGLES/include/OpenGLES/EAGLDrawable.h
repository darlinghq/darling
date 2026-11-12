/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Team

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
*/

#ifndef _OPENGLES_EAGLDRAWABLE_H
#define _OPENGLES_EAGLDRAWABLE_H

#import <OpenGLES/EAGL.h>

FOUNDATION_EXPORT NSString *const kEAGLDrawablePropertyRetainedBacking;
FOUNDATION_EXPORT NSString *const kEAGLDrawablePropertyColorFormat;
FOUNDATION_EXPORT NSString *const kEAGLColorFormatRGBA8;
FOUNDATION_EXPORT NSString *const kEAGLColorFormatRGB565;
FOUNDATION_EXPORT NSString *const kEAGLColorFormatSRGBA8;

/* QuartzCore's CAEAGLLayer declares the same protocol. */
#ifndef EAGL_DRAWABLE_PROTOCOL
#define EAGL_DRAWABLE_PROTOCOL
@protocol EAGLDrawable
@property (copy) NSDictionary *drawableProperties;
@end
#endif

@interface EAGLContext (EAGLContextDrawableAdditions)
- (BOOL)renderbufferStorage:(NSUInteger)target fromDrawable:(id<EAGLDrawable>)drawable;
- (BOOL)presentRenderbuffer:(NSUInteger)target;
- (BOOL)presentRenderbuffer:(NSUInteger)target atTime:(CFTimeInterval)presentationTime;
- (BOOL)presentRenderbuffer:(NSUInteger)target afterMinimumDuration:(CFTimeInterval)duration;
@end

#endif
