/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _UIFoundation_NSTextGraphicsContextProvider_H_
#define _UIFoundation_NSTextGraphicsContextProvider_H_

#import <Foundation/Foundation.h>
#import <UIFoundation/NSTextGraphicsContext.h>

@protocol NSTextGraphicsContextProvider <NSObject>

@required
+ (id<NSTextGraphicsContext>)graphicsContextForApplicationFrameworkContext:(id)context;

@optional
+ (Class)colorClassForApplicationFrameworkContext:(id)context;

@end

@interface NSTextGraphicsContextProvider : NSObject

+ (Class)textGraphicsContextProviderClass;
+ (void)setTextGraphicsContextProviderClass:(Class)cls;
+ (BOOL)textGraphicsContextProviderClassRespondsToColorQuery;
+ (Class)__defaultColorClass;
+ (Class)textGraphicsContextClass;
+ (void)setTextGraphicsContextClass:(Class)cls;
+ (void)setCurrentTextGraphicsContext:(id)context duringBlock:(void (^)(void))block;

@end

#endif
