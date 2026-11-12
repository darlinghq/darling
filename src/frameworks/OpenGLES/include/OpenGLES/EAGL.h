/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Team

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
*/

#ifndef _OPENGLES_EAGL_H
#define _OPENGLES_EAGL_H

#import <Foundation/Foundation.h>

#define EAGL_MAJOR_VERSION 1
#define EAGL_MINOR_VERSION 0

typedef NS_ENUM(NSUInteger, EAGLRenderingAPI) {
	kEAGLRenderingAPIOpenGLES1 = 1,
	kEAGLRenderingAPIOpenGLES2 = 2,
	kEAGLRenderingAPIOpenGLES3 = 3,
};

#ifdef __cplusplus
extern "C" {
#endif
void EAGLGetVersion(unsigned int *major, unsigned int *minor);
#ifdef __cplusplus
}
#endif

@interface EAGLSharegroup : NSObject
@property (copy) NSString *debugLabel;
@end

@interface EAGLContext : NSObject
- (instancetype)initWithAPI:(EAGLRenderingAPI)api;
- (instancetype)initWithAPI:(EAGLRenderingAPI)api sharegroup:(EAGLSharegroup *)sharegroup;
+ (BOOL)setCurrentContext:(EAGLContext *)context;
+ (EAGLContext *)currentContext;
@property (readonly) EAGLRenderingAPI API;
@property (readonly) EAGLSharegroup *sharegroup;
@property (copy) NSString *debugLabel;
@property (getter=isMultiThreaded) BOOL multiThreaded;
@end

#endif
