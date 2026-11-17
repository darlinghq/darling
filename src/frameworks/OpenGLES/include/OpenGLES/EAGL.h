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

NS_ASSUME_NONNULL_BEGIN

@interface EAGLSharegroup : NSObject
@property (nullable, copy, nonatomic) NSString *debugLabel;
@end

@interface EAGLContext : NSObject
- (nullable instancetype)init NS_UNAVAILABLE;
- (nullable instancetype)initWithAPI:(EAGLRenderingAPI)api;
- (nullable instancetype)initWithAPI:(EAGLRenderingAPI)api sharegroup:(nullable EAGLSharegroup *)sharegroup NS_DESIGNATED_INITIALIZER;
+ (BOOL)setCurrentContext:(nullable EAGLContext *)context;
+ (nullable EAGLContext *)currentContext;
@property (readonly) EAGLRenderingAPI API;
@property (nonnull, readonly) EAGLSharegroup *sharegroup;
@property (nullable, copy, nonatomic) NSString *debugLabel;
@property (getter=isMultiThreaded, nonatomic) BOOL multiThreaded;
@end

NS_ASSUME_NONNULL_END

#endif
