/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Team

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
*/

#ifndef UIKIT_UIAPPLICATION_H
#define UIKIT_UIAPPLICATION_H

#import <Foundation/NSObjCRuntime.h>
#import <UIKit/UIResponder.h>

@class NSString;
@protocol UIApplicationDelegate;

typedef NS_ENUM(NSInteger, UIApplicationState) {
	UIApplicationStateActive,
	UIApplicationStateInactive,
	UIApplicationStateBackground,
};

@interface UIApplication : UIResponder

+ (UIApplication *)sharedApplication;

@property (nonatomic, assign) id<UIApplicationDelegate> delegate;
@property (nonatomic, readonly) UIApplicationState applicationState;

/* Drain the main run loop, as -[NSApplication run] does. */
- (void)run;

@end

/*
 * Entry point.  principalClassName is accepted for source compatibility and
 * ignored; delegateClassName names the UIApplicationDelegate subclass to
 * instantiate, as on iOS.  Returns only after the application terminates.
 */
int UIApplicationMain(int argc, char *argv[], NSString *principalClassName, NSString *delegateClassName);

#endif /* UIKIT_UIAPPLICATION_H */
