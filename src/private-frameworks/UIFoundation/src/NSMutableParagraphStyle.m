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
#import "UIFoundation-ExtraIvars.h"
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
    UIF_EXTRA_NSParagraphStyle(self)->_writingDirection = UIF_EXTRA_NSParagraphStyle(other)->_writingDirection;
    UIF_EXTRA_NSParagraphStyle(self)->_paragraphSpacing = UIF_EXTRA_NSParagraphStyle(other)->_paragraphSpacing;
    UIF_EXTRA_NSParagraphStyle(self)->_paragraphSpacingBefore = UIF_EXTRA_NSParagraphStyle(other)->_paragraphSpacingBefore;
    [self setTextBlocks: [other textBlocks]];
    [self setTextLists: [other textLists]];
    UIF_EXTRA_NSParagraphStyle(self)->_headerLevel = UIF_EXTRA_NSParagraphStyle(other)->_headerLevel;
    UIF_EXTRA_NSParagraphStyle(self)->_firstLineHeadIndent = UIF_EXTRA_NSParagraphStyle(other)->_firstLineHeadIndent;
    UIF_EXTRA_NSParagraphStyle(self)->_headIndent = UIF_EXTRA_NSParagraphStyle(other)->_headIndent;
    UIF_EXTRA_NSParagraphStyle(self)->_tailIndent = UIF_EXTRA_NSParagraphStyle(other)->_tailIndent;
    UIF_EXTRA_NSParagraphStyle(self)->_alignment = UIF_EXTRA_NSParagraphStyle(other)->_alignment;
    UIF_EXTRA_NSParagraphStyle(self)->_lineBreakMode = UIF_EXTRA_NSParagraphStyle(other)->_lineBreakMode;
    UIF_EXTRA_NSParagraphStyle(self)->_minimumLineHeight = UIF_EXTRA_NSParagraphStyle(other)->_minimumLineHeight;
    UIF_EXTRA_NSParagraphStyle(self)->_maximumLineHeight = UIF_EXTRA_NSParagraphStyle(other)->_maximumLineHeight;
    UIF_EXTRA_NSParagraphStyle(self)->_lineHeightMultiple = UIF_EXTRA_NSParagraphStyle(other)->_lineHeightMultiple;
    UIF_EXTRA_NSParagraphStyle(self)->_lineSpacing = UIF_EXTRA_NSParagraphStyle(other)->_lineSpacing;
    UIF_EXTRA_NSParagraphStyle(self)->_defaultTabInterval = UIF_EXTRA_NSParagraphStyle(other)->_defaultTabInterval;
    [self setTabStops: [other tabStops]];
    UIF_EXTRA_NSParagraphStyle(self)->_hyphenationFactor = UIF_EXTRA_NSParagraphStyle(other)->_hyphenationFactor;
    UIF_EXTRA_NSParagraphStyle(self)->_tighteningFactorForTruncation = UIF_EXTRA_NSParagraphStyle(other)->_tighteningFactorForTruncation;
    UIF_EXTRA_NSParagraphStyle(self)->_horizontalAlignment = UIF_EXTRA_NSParagraphStyle(other)->_horizontalAlignment;
    UIF_EXTRA_NSParagraphStyle(self)->_usesDefaultHyphenation = UIF_EXTRA_NSParagraphStyle(other)->_usesDefaultHyphenation;
    UIF_EXTRA_NSParagraphStyle(self)->_allowsDefaultTighteningForTruncation = UIF_EXTRA_NSParagraphStyle(other)->_allowsDefaultTighteningForTruncation;
    UIF_EXTRA_NSParagraphStyle(self)->_lineBreakStrategy = UIF_EXTRA_NSParagraphStyle(other)->_lineBreakStrategy;
}

- (void) setBaseWritingDirection: (NSWritingDirection) direction {
    UIF_EXTRA_NSParagraphStyle(self)->_writingDirection = direction;
}

- (void) setHorizontalAlignment: (NSInteger) alignment {
    UIF_EXTRA_NSParagraphStyle(self)->_horizontalAlignment = alignment;
}

- (void) setParagraphSpacing: (CGFloat) spacing {
    UIF_EXTRA_NSParagraphStyle(self)->_paragraphSpacing = spacing;
}

- (void) setParagraphSpacingBefore: (CGFloat) spacing {
    UIF_EXTRA_NSParagraphStyle(self)->_paragraphSpacingBefore = spacing;
}

- (void) setTextBlocks: (NSArray *) blocks {
    blocks = [blocks copy];
    [UIF_EXTRA_NSParagraphStyle(self)->_textBlocks release];
    UIF_EXTRA_NSParagraphStyle(self)->_textBlocks = blocks;
}

- (void) setTextLists: (NSArray *) lists {
    lists = [lists copy];
    [UIF_EXTRA_NSParagraphStyle(self)->_textLists release];
    UIF_EXTRA_NSParagraphStyle(self)->_textLists = lists;
}

- (void) setHeaderLevel: (NSInteger) level {
    UIF_EXTRA_NSParagraphStyle(self)->_headerLevel = level;
}

- (void) setFirstLineHeadIndent: (CGFloat) indent {
    UIF_EXTRA_NSParagraphStyle(self)->_firstLineHeadIndent = indent;
}

- (void) setHeadIndent: (CGFloat) indent {
    UIF_EXTRA_NSParagraphStyle(self)->_headIndent = indent;
}

- (void) setTailIndent: (CGFloat) indent {
    UIF_EXTRA_NSParagraphStyle(self)->_tailIndent = indent;
}

- (void) setAlignment: (NSTextAlignment) alignment {
    UIF_EXTRA_NSParagraphStyle(self)->_alignment = alignment;
}

- (void) setLineBreakMode: (NSLineBreakMode) mode {
    UIF_EXTRA_NSParagraphStyle(self)->_lineBreakMode = mode;
}

- (void) setMinimumLineHeight: (CGFloat) height {
    UIF_EXTRA_NSParagraphStyle(self)->_minimumLineHeight = height;
}

- (void) setMaximumLineHeight: (CGFloat) height {
    UIF_EXTRA_NSParagraphStyle(self)->_maximumLineHeight = height;
}

- (void) setLineHeightMultiple: (CGFloat) multiple {
    UIF_EXTRA_NSParagraphStyle(self)->_lineHeightMultiple = multiple;
}

- (void) setLineSpacing: (CGFloat) spacing {
    UIF_EXTRA_NSParagraphStyle(self)->_lineSpacing = spacing;
}

- (void) setDefaultTabInterval: (CGFloat) interval {
    UIF_EXTRA_NSParagraphStyle(self)->_defaultTabInterval = interval;
}

- (void) setTabStops: (NSArray *) tabStops {
    if (tabStops == nil)
        tabStops = [[self class] _defaultTabStops];
    if (tabStops != UIF_EXTRA_NSParagraphStyle(self)->_tabStops) {
        [UIF_EXTRA_NSParagraphStyle(self)->_tabStops removeAllObjects];
        [UIF_EXTRA_NSParagraphStyle(self)->_tabStops addObjectsFromArray: tabStops];
        [UIF_EXTRA_NSParagraphStyle(self)->_tabStops sortUsingSelector: @selector(compare:)];
    }
}

- (void) addTabStop: (NSTextTab *) tabStop {
    NSUInteger index = [UIF_EXTRA_NSParagraphStyle(self)->_tabStops
            indexOfObjectPassingTest: ^BOOL(NSTextTab *other, NSUInteger id,
                                            BOOL *stop) {
              return [other compare: tabStop] == NSOrderedDescending;
            }];

    if (index == NSNotFound) {
        [UIF_EXTRA_NSParagraphStyle(self)->_tabStops addObject: tabStop];
    } else {
        [UIF_EXTRA_NSParagraphStyle(self)->_tabStops insertObject: tabStop atIndex: index];
    }
}

- (void) removeTabStop: (NSTextTab *) tabStop {
    [UIF_EXTRA_NSParagraphStyle(self)->_tabStops removeObject: tabStop];
}

- (void) setHyphenationFactor: (float) factor {
    UIF_EXTRA_NSParagraphStyle(self)->_hyphenationFactor = factor;
}

- (void) setTighteningFactorForTruncation: (float) factor {
    UIF_EXTRA_NSParagraphStyle(self)->_tighteningFactorForTruncation = factor;
}

- (void) setUsesDefaultHyphenation: (BOOL) value {
    UIF_EXTRA_NSParagraphStyle(self)->_usesDefaultHyphenation = value;
}

- (void) setAllowsDefaultTighteningForTruncation: (BOOL) value {
    UIF_EXTRA_NSParagraphStyle(self)->_allowsDefaultTighteningForTruncation = value;
}

- (void) setLineBreakStrategy: (NSLineBreakStrategy) strategy {
    UIF_EXTRA_NSParagraphStyle(self)->_lineBreakStrategy = strategy;
}

@end
