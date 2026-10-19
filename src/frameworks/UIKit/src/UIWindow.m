/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Team

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
*/

#import <UIKit/UIWindow.h>
#import <UIKit/UIViewController.h>
#import <UIKit/UIView.h>
#import <AppKit/NSWindow.h>
#import <Foundation/NSProcessInfo.h>

#import "UIKitPrivate.h"

@implementation UIWindow

- (instancetype)initWithFrame:(CGRect)frame
{
	self = [super initWithFrame:frame];
	if (self == nil)
		return nil;

	NSRect contentRect = NSMakeRect(frame.origin.x, frame.origin.y,
	                                frame.size.width, frame.size.height);
	NSWindow *host = [[NSWindow alloc] initWithContentRect:contentRect
	                                            styleMask:NSWindowStyleMaskTitled
	                                              backing:NSBackingStoreBuffered
	                                                defer:NO];
	[host setReleasedWhenClosed:NO];
	/* No iOS equivalent of an NSWindow title; use the process name so the
	 * windowing system and the launch probes can identify the surface. */
	[host setTitle:[[NSProcessInfo processInfo] processName]];
	[host setContentView:self];
	_hostWindow = host;

	[self setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
	[self setFrame:NSMakeRect(0, 0, frame.size.width, frame.size.height)];
	return self;
}

- (void)dealloc
{
	[(NSWindow *)_hostWindow setContentView:nil];
}

- (void)setRootViewController:(UIViewController *)controller
{
	if (_rootViewController == controller)
		return;
	if (_rootViewController != nil)
		[[_rootViewController view] removeFromSuperview];

	_rootViewController = controller;
	if (controller == nil)
		return;

	UIView *view = [controller view];
	[view setFrame:[self bounds]];
	[self addSubview:view];
	[view attachToWindow:self];
}

- (void)makeKeyAndVisible
{
	[self setHidden:NO];
	[(NSWindow *)_hostWindow makeKeyAndOrderFront:nil];
}

- (void)makeKeyWindow
{
	[(NSWindow *)_hostWindow makeKeyWindow];
}

@end
