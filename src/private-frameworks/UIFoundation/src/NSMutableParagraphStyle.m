/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

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
#import "NSParagraphStyle-Private.h"

@implementation NSMutableParagraphStyle

+ (NSParagraphStyle *) defaultParagraphStyle {
    static NSParagraphStyle *shared = nil;

    if (shared == nil)
        shared = [self new];

    return shared;
}

- copyWithZone: (NSZone *) zone {
    return [[NSParagraphStyle alloc] initWithParagraphStyle: self];
}

- (void) setParagraphStyle: (NSParagraphStyle *) other {
    _writingDirection = other->_writingDirection;
    _paragraphSpacing = other->_paragraphSpacing;
    _paragraphSpacingBefore = other->_paragraphSpacingBefore;
    [self setTextBlocks: [other textBlocks]];
    [self setTextLists: [other textLists]];
    _headerLevel = other->_headerLevel;
    _firstLineHeadIndent = other->_firstLineHeadIndent;
    _headIndent = other->_headIndent;
    _tailIndent = other->_tailIndent;
    _alignment = other->_alignment;
    _lineBreakMode = other->_lineBreakMode;
    _minimumLineHeight = other->_minimumLineHeight;
    _maximumLineHeight = other->_maximumLineHeight;
    _lineHeightMultiple = other->_lineHeightMultiple;
    _lineSpacing = other->_lineSpacing;
    _defaultTabInterval = other->_defaultTabInterval;
    [self setTabStops: [other tabStops]];
    _hyphenationFactor = other->_hyphenationFactor;
    _tighteningFactorForTruncation = other->_tighteningFactorForTruncation;
    _horizontalAlignment = other->_horizontalAlignment;
    _usesDefaultHyphenation = other->_usesDefaultHyphenation;
    _allowsDefaultTighteningForTruncation = other->_allowsDefaultTighteningForTruncation;
    _lineBreakStrategy = other->_lineBreakStrategy;
}

- (void) setBaseWritingDirection: (NSWritingDirection) direction {
    _writingDirection = direction;
}

- (void) setHorizontalAlignment: (NSInteger) alignment {
    _horizontalAlignment = alignment;
}

- (void) setParagraphSpacing: (CGFloat) spacing {
    _paragraphSpacing = spacing;
}

- (void) setParagraphSpacingBefore: (CGFloat) spacing {
    _paragraphSpacingBefore = spacing;
}

- (void) setTextBlocks: (NSArray *) blocks {
    blocks = [blocks copy];
    [_textBlocks release];
    _textBlocks = blocks;
}

- (void) setTextLists: (NSArray *) lists {
    lists = [lists copy];
    [_textLists release];
    _textLists = lists;
}

- (void) setHeaderLevel: (NSInteger) level {
    _headerLevel = level;
}

- (void) setFirstLineHeadIndent: (CGFloat) indent {
    _firstLineHeadIndent = indent;
}

- (void) setHeadIndent: (CGFloat) indent {
    _headIndent = indent;
}

- (void) setTailIndent: (CGFloat) indent {
    _tailIndent = indent;
}

- (void) setAlignment: (NSTextAlignment) alignment {
    _alignment = alignment;
}

- (void) setLineBreakMode: (NSLineBreakMode) mode {
    _lineBreakMode = mode;
}

- (void) setMinimumLineHeight: (CGFloat) height {
    _minimumLineHeight = height;
}

- (void) setMaximumLineHeight: (CGFloat) height {
    _maximumLineHeight = height;
}

- (void) setLineHeightMultiple: (CGFloat) multiple {
    _lineHeightMultiple = multiple;
}

- (void) setLineSpacing: (CGFloat) spacing {
    _lineSpacing = spacing;
}

- (void) setDefaultTabInterval: (CGFloat) interval {
    _defaultTabInterval = interval;
}

- (void) setTabStops: (NSArray *) tabStops {
    if (tabStops == nil)
        tabStops = [[self class] _defaultTabStops];
    if (tabStops != _tabStops) {
        [_tabStops removeAllObjects];
        [_tabStops addObjectsFromArray: tabStops];
        [_tabStops sortUsingSelector: @selector(compare:)];
    }
}

- (void) addTabStop: (NSTextTab *) tabStop {
    NSUInteger index = [_tabStops
            indexOfObjectPassingTest: ^BOOL(NSTextTab *other, NSUInteger id,
                                            BOOL *stop) {
              return [other compare: tabStop] == NSOrderedDescending;
            }];

    if (index == NSNotFound) {
        [_tabStops addObject: tabStop];
    } else {
        [_tabStops insertObject: tabStop atIndex: index];
    }
}

- (void) removeTabStop: (NSTextTab *) tabStop {
    [_tabStops removeObject: tabStop];
}

- (void) setHyphenationFactor: (float) factor {
    _hyphenationFactor = factor;
}

- (void) setTighteningFactorForTruncation: (float) factor {
    _tighteningFactorForTruncation = factor;
}

- (void) setUsesDefaultHyphenation: (BOOL) value {
    _usesDefaultHyphenation = value;
}

- (void) setAllowsDefaultTighteningForTruncation: (BOOL) value {
    _allowsDefaultTighteningForTruncation = value;
}

- (void) setLineBreakStrategy: (NSLineBreakStrategy) strategy {
    _lineBreakStrategy = strategy;
}

@end
