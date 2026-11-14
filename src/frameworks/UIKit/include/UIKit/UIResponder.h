/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Team

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
*/

#ifndef UIKIT_UIRESPONDER_H
#define UIKIT_UIRESPONDER_H

#import <Foundation/NSObject.h>

@class UIResponder;

@interface UIResponder : NSObject {
@private
	UIResponder *_nextResponder;
	BOOL _firstResponder;
}

- (UIResponder *)nextResponder;

- (BOOL)canBecomeFirstResponder;
- (BOOL)becomeFirstResponder;
- (BOOL)isFirstResponder;
- (BOOL)resignFirstResponder;

@end

#endif /* UIKIT_UIRESPONDER_H */
