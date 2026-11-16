/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import "UIFoundation-ExtraIvars.h"
#import <AppKit/NSGlyphInfo.h>
#import <Foundation/NSRaise.h>

@implementation NSGlyphInfo

- initWithCharacterIdentifier: (NSUInteger) identifier
                   collection: (NSCharacterCollection) collection
                    glyphName: (NSString *) glyphName
{
    UIF_EXTRA_NSGlyphInfo(self)->_identifier = identifier;
    UIF_EXTRA_NSGlyphInfo(self)->_collection = collection;
    UIF_EXTRA_NSGlyphInfo(self)->_glyphName = [glyphName copy];
    return self;
}

- (void) dealloc {
    [UIF_EXTRA_NSGlyphInfo(self)->_glyphName release];
    UIF_EXTRA_FREE(NSGlyphInfo, self);
    [super dealloc];
}

+ (NSGlyphInfo *)
        glyphInfoWithCharacterIdentifier: (NSUInteger) identifier
                              collection: (NSCharacterCollection) collection
                              baseString: (NSString *) baseString
{
    NSUnimplementedMethod();
    return nil;
}

+ (NSGlyphInfo *) glyphInfoWithGlyph: (NSGlyph) glyph
                             forFont: (NSFont *) font
                          baseString: (NSString *) baseString
{
    NSUnimplementedMethod();
    return nil;
}

+ (NSGlyphInfo *) glyphInfoWithGlyphName: (NSString *) glyphName
                                 forFont: (NSFont *) font
                              baseString: (NSString *) baseString
{
    NSUnimplementedMethod();
    return nil;
}

- (NSUInteger) characterIdentifier {
    return UIF_EXTRA_NSGlyphInfo(self)->_identifier;
}

- (NSCharacterCollection) characterCollection {
    return UIF_EXTRA_NSGlyphInfo(self)->_collection;
}

- (NSString *) glyphName {
    return UIF_EXTRA_NSGlyphInfo(self)->_glyphName;
}

@end
