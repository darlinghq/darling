/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Team

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
*/

#import <UIKit/UIResponder.h>

@implementation UIResponder

- (UIResponder *)nextResponder
{
	return _nextResponder;
}

/* Subclasses that own a view chain override this to return their parent. */
- (void)setNextResponder:(UIResponder *)responder
{
	_nextResponder = responder;
}

- (BOOL)canBecomeFirstResponder
{
	return NO;
}

- (BOOL)becomeFirstResponder
{
	if (![self canBecomeFirstResponder])
		return NO;
	_firstResponder = YES;
	return YES;
}

- (BOOL)isFirstResponder
{
	return _firstResponder;
}

- (BOOL)resignFirstResponder
{
	_firstResponder = NO;
	return YES;
}

@end
