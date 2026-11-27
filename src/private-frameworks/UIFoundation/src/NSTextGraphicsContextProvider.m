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

#import <UIFoundation/NSTextGraphicsContextProvider.h>

static Class _textGraphicsContextProviderClass = Nil;
static Class _textGraphicsContextClass = Nil;
static NSString *const _NSTextGraphicsContextCurrentKey = @"_NSTextGraphicsContextCurrent";

@implementation NSTextGraphicsContextProvider

+ (Class)textGraphicsContextProviderClass
{
    return _textGraphicsContextProviderClass;
}

+ (void)setTextGraphicsContextProviderClass:(Class)cls
{
    _textGraphicsContextProviderClass = cls;
}

+ (BOOL)textGraphicsContextProviderClassRespondsToColorQuery
{
    return _textGraphicsContextProviderClass != Nil
        && [_textGraphicsContextProviderClass respondsToSelector:@selector(colorClassForApplicationFrameworkContext:)];
}

+ (Class)__defaultColorClass
{
    if ([self textGraphicsContextProviderClassRespondsToColorQuery]) {
        Class cls = [_textGraphicsContextProviderClass colorClassForApplicationFrameworkContext:nil];
        if (cls != Nil)
            return cls;
    }
    return NSClassFromString(@"NSColor");
}

+ (Class)textGraphicsContextClass
{
    return _textGraphicsContextClass;
}

+ (void)setTextGraphicsContextClass:(Class)cls
{
    _textGraphicsContextClass = cls;
}

+ (void)setCurrentTextGraphicsContext:(id)context duringBlock:(void (^)(void))block
{
    if (block == nil)
        return;

    NSMutableDictionary *threadDict = [[NSThread currentThread] threadDictionary];
    id previous = [[threadDict objectForKey:_NSTextGraphicsContextCurrentKey] retain];

    if (context != nil)
        [threadDict setObject:context forKey:_NSTextGraphicsContextCurrentKey];
    else
        [threadDict removeObjectForKey:_NSTextGraphicsContextCurrentKey];

    @try {
        block();
    } @finally {
        if (previous != nil) {
            [threadDict setObject:previous forKey:_NSTextGraphicsContextCurrentKey];
            [previous release];
        } else {
            [threadDict removeObjectForKey:_NSTextGraphicsContextCurrentKey];
        }
    }
}

@end
