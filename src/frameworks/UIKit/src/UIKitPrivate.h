/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Team

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
*/

/*
 * Internal bridge declarations.  These are deliberately not in include/UIKit:
 * they expose AppKit types and must not appear in the iOS-facing API surface.
 */

#ifndef UIKIT_UIKITPRIVATE_H
#define UIKIT_UIKITPRIVATE_H

#import <UIKit/UIKit.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSFont.h>
#import <AppKit/NSWindow.h>

@interface UIColor (DarlingPrivate)
- (NSColor *)nsColor;
@end

@interface UIFont (DarlingPrivate)
- (NSFont *)nsFont;
@end

@interface UIView (DarlingPrivate)
- (void)setWindow:(UIWindow *)window;
- (void)attachToWindow:(UIWindow *)window;
@end

#endif /* UIKIT_UIKITPRIVATE_H */
