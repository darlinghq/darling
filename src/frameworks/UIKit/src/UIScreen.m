/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Team

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
*/

#import <UIKit/UIScreen.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSScreen.h>

@implementation UIScreen

+ (UIScreen *)mainScreen
{
	static UIScreen *screen;
	if (screen == nil)
		screen = [[UIScreen alloc] init];
	return screen;
}

- (CGRect)bounds
{
	NSScreen *screen = [NSScreen mainScreen];
	NSRect frame = screen != nil ? [screen frame] : NSMakeRect(0, 0, 1024, 768);
	return CGRectMake(0, 0, frame.size.width, frame.size.height);
}

- (CGFloat)scale
{
	NSScreen *screen = [NSScreen mainScreen];
	CGFloat scale = screen != nil ? [screen backingScaleFactor] : 1.0;
	return scale > 0 ? scale : 1.0;
}

- (CGRect)nativeBounds
{
	CGRect points = [self bounds];
	CGFloat scale = [self scale];
	return CGRectMake(0, 0, points.size.width * scale, points.size.height * scale);
}

@end
