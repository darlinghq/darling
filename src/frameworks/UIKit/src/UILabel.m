/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Team

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
*/

#import <UIKit/UILabel.h>
#import <UIKit/UIColor.h>
#import <UIKit/UIFont.h>
#import <AppKit/NSAttributedString.h>
#import <AppKit/NSStringDrawing.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSString.h>

#import "UIKitPrivate.h"

@implementation UILabel

- (instancetype)initWithFrame:(CGRect)frame
{
	self = [super initWithFrame:frame];
	if (self != nil) {
		_textColor = [UIColor blackColor];
		_font = [UIFont systemFontOfSize:17];
		_textAlignment = NSTextAlignmentLeft;
	}
	return self;
}

- (NSString *)text
{
	return _text;
}

- (void)setText:(NSString *)text
{
	_text = [text copy];
	[self setNeedsDisplay:YES];
}

- (UIColor *)textColor
{
	return _textColor;
}

- (void)setTextColor:(UIColor *)color
{
	_textColor = color;
	[self setNeedsDisplay:YES];
}

- (UIFont *)font
{
	return _font;
}

- (void)setFont:(UIFont *)font
{
	_font = font;
	[self setNeedsDisplay:YES];
}

- (NSTextAlignment)textAlignment
{
	return _textAlignment;
}

- (void)setTextAlignment:(NSTextAlignment)alignment
{
	_textAlignment = alignment;
	[self setNeedsDisplay:YES];
}

- (NSDictionary *)textAttributes
{
	NSMutableDictionary *attributes = [NSMutableDictionary dictionary];
	if (_font != nil)
		[attributes setObject:[_font nsFont] forKey:NSFontAttributeName];
	if (_textColor != nil)
		[attributes setObject:[_textColor nsColor] forKey:NSForegroundColorAttributeName];
	return attributes;
}

- (void)drawRect:(NSRect)rect
{
	[super drawRect:rect];
	if ([_text length] == 0)
		return;

	[_text drawInRect:[self bounds] withAttributes:[self textAttributes]];
}

- (CGSize)sizeThatFits:(CGSize)size
{
	if ([_text length] == 0)
		return CGSizeMake(0, 0);
	NSSize measured = [_text sizeWithAttributes:[self textAttributes]];
	return CGSizeMake(measured.width, measured.height);
}

@end
