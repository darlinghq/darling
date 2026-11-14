/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Team

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
*/

#ifndef UIKIT_UILABEL_H
#define UIKIT_UILABEL_H

#import <UIKit/UIView.h>
#import <AppKit/NSText.h>

@class NSString;
@class UIColor;
@class UIFont;

@interface UILabel : UIView {
@private
	NSString *_text;
	UIColor *_textColor;
	UIFont *_font;
	NSTextAlignment _textAlignment;
}

@property (nonatomic, copy) NSString *text;
@property (nonatomic, strong) UIColor *textColor;
@property (nonatomic, strong) UIFont *font;
@property (nonatomic) NSTextAlignment textAlignment;

/* Convenience matching the size the text needs at the current font. */
- (CGSize)sizeThatFits:(CGSize)size;

@end

#endif /* UIKIT_UILABEL_H */
