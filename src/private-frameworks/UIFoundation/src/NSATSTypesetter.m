/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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

#import <AppKit/NSATSTypesetter.h>
#import "NSTypesetter_concrete.h"

@implementation NSATSTypesetter

+ (NSATSTypesetter *) sharedSystemTypesetter {
    static NSATSTypesetter *shared = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        shared = [NSATSTypesetter new];
    });
    return shared;
}

- (void) layoutGlyphsInLayoutManager: (NSLayoutManager *) layoutManager
                startingAtGlyphIndex: (NSUInteger) startGlyphIndex
            maxNumberOfLineFragments: (NSUInteger) maxNumLines
                      nextGlyphIndex: (NSUInteger *) nextGlyph
{
    NSTypesetter_concrete *typesetter = [[NSTypesetter_concrete alloc] init];
    [typesetter layoutGlyphsInLayoutManager: layoutManager
                       startingAtGlyphIndex: startGlyphIndex
                   maxNumberOfLineFragments: maxNumLines
                             nextGlyphIndex: nextGlyph];
    [typesetter release];
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end
