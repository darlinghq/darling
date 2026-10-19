/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Team

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
*/

#ifndef UIKIT_UIVIEW_H
#define UIKIT_UIVIEW_H

#import <AppKit/NSView.h>
#import <CoreGraphics/CGGeometry.h>

@class UIColor;

/*
 * UIView is an AppKit view: Darling has no separate compositor, so the UIKit
 * tree *is* the window's content view tree.  Its coordinate system is flipped
 * to match UIKit (origin top-left).
 *
 * Deviation from iOS: NSView already declares -window (NSWindow *) and its own
 * ivars, so the UIKit hosting window is exposed internally as -uiWindow rather
 * than shadowing -window with an incompatible type.
 */
@interface UIView : NSView

- (instancetype)initWithFrame:(CGRect)frame;

@property (nonatomic, strong) UIColor *backgroundColor;

- (void)layoutSubviews;
- (void)setNeedsLayout;
- (void)layoutIfNeeded;

@end

#endif /* UIKIT_UIVIEW_H */
