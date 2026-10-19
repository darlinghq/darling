/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Team

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
*/

#ifndef UIKIT_UIVIEWCONTROLLER_H
#define UIKIT_UIVIEWCONTROLLER_H

#import <UIKit/UIResponder.h>
#import <CoreGraphics/CGGeometry.h>

@class UIView;
@class NSString;

@interface UIViewController : UIResponder {
@private
	UIView *_view;
	NSString *_title;
}

/* Created on first access through -loadView. */
@property (nonatomic, readonly) UIView *view;

@property (nonatomic, copy) NSString *title;

- (void)loadView;
- (void)viewDidLoad;
- (void)viewWillAppear;
- (void)viewDidAppear;

@end

#endif /* UIKIT_UIVIEWCONTROLLER_H */
