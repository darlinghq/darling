/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Team

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
*/

#import <UIKit/UIApplication.h>
#import <UIKit/UIApplicationDelegate.h>
#import <AppKit/NSApplication.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSObjCRuntime.h>
#import <Foundation/NSString.h>

/* -[NSApplication run] delivers the AppKit delegate callbacks; UIApplication
 * implements them and forwards to the iOS delegate, which never sees AppKit. */
@interface UIApplication () <NSApplicationDelegate>
@end

static UIApplication *sSharedApplication;
static id<UIApplicationDelegate> sDelegateStorage;

@implementation UIApplication

- (id<UIApplicationDelegate>)delegate
{
	return sDelegateStorage;
}

- (void)setDelegate:(id<UIApplicationDelegate>)delegate
{
	sDelegateStorage = delegate;
}

+ (UIApplication *)sharedApplication
{
	if (sSharedApplication == nil)
		sSharedApplication = [[UIApplication alloc] init];
	return sSharedApplication;
}

- (UIApplicationState)applicationState
{
	return UIApplicationStateActive;
}

- (void)run
{
	[NSApp run];
}

#pragma mark - NSApplicationDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
	id<UIApplicationDelegate> delegate = [self delegate];
	if ([delegate respondsToSelector:@selector(application:didFinishLaunchingWithOptions:)])
		[delegate application:self didFinishLaunchingWithOptions:nil];
}

- (void)applicationWillTerminate:(NSNotification *)notification
{
	id<UIApplicationDelegate> delegate = [self delegate];
	if ([delegate respondsToSelector:@selector(applicationWillTerminate:)])
		[delegate applicationWillTerminate:self];
}

/* Terminate when the run loop asks, instead of falling through to AppKit's
 * last-window handling. */
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
	return NSTerminateNow;
}

@end

int UIApplicationMain(int argc, char *argv[], NSString *principalClassName, NSString *delegateClassName)
{
	/* iOS ignores these; the delegate is instantiated by name instead. */
	(void)argc;
	(void)argv;
	(void)principalClassName;

	@autoreleasepool {
		[NSApplication sharedApplication];

		UIApplication *application = [UIApplication sharedApplication];
		[NSApp setDelegate:application];

		if (delegateClassName != nil) {
			Class delegateClass = NSClassFromString(delegateClassName);
			if (delegateClass != Nil) {
				id delegate = [[delegateClass alloc] init];
				sDelegateStorage = delegate;
				application.delegate = delegate;
			}
		}

		[NSApp run];
	}
	return 0;
}
