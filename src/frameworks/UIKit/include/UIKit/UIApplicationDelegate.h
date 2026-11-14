/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Team

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
*/

#ifndef UIKIT_UIAPPLICATIONDELEGATE_H
#define UIKIT_UIAPPLICATIONDELEGATE_H

#import <Foundation/NSObject.h>

@class UIApplication;
@class NSDictionary;

@protocol UIApplicationDelegate <NSObject>

@optional
- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions;
- (void)applicationWillTerminate:(UIApplication *)application;

@end

#endif /* UIKIT_UIAPPLICATIONDELEGATE_H */
