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

#import <AppKit/NSFont.h>
#import <AppKit/NSParagraphStyle.h>
#import <AppKit/NSTypesetter.h>

@class NSTextContainer, NSRangeArray, NSParagraphStyle;

@interface NSTypesetter_concrete : NSTypesetter {
    IMP _layoutNextFragment;

    NSUInteger _nextGlyphLocation;
    NSUInteger _numberOfGlyphs;
    NSRange _glyphCacheRange;
    NSUInteger _glyphCacheCapacity;
    NSGlyph *_glyphCache;
    unichar *_characterCache;

    NSUInteger _bidiLevelsCapacity;
    uint8_t *_bidiLevels;
    uint8_t _currentBidiLevel;
    uint8_t _currentParagraphBidiLevel;

    BOOL _paragraphBreak;

    NSTextContainer *_container;
    NSSize _containerSize;

    NSDictionary *_attributes;
    NSRange _attributesRange;
    NSRange _attributesGlyphRange;
    NSFont *_font;
    CGFloat _fontAscender;
    CGFloat _fontDefaultLineHeight;
    NSPoint (*_positionOfGlyph)(NSFont *, SEL, NSGlyph, NSGlyph, BOOL *);
    NSTextAlignment _alignment;
    NSLineBreakMode _lineBreakMode;
    CGFloat _whitespaceAdvancement;

    NSRange _lineRange;
    NSRangeArray *_glyphRangesInLine;
    NSGlyph _previousGlyph;
    NSRect _scanRect;
    NSRect _wordWrapScanRect;
    NSRect _fullLineRect;
    CGFloat _maxAscender;

    NSRange _wordWrapRange;
    CGFloat _wordWrapWidth;
    NSGlyph _wordWrapPreviousGlyph;
}

@end
