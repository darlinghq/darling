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

#include <stdio.h>
#import <Foundation/NSKeyedArchiver.h>
#import <Foundation/NSLocale.h>

#import "NSParagraphStyle-Private.h"
#import <AppKit/NSRaise.h>
#import <AppKit/NSTextBlock.h>
#import <AppKit/NSTextList.h>
#import <AppKit/NSTextTab.h>

@implementation NSParagraphStyle

+ (BOOL) supportsSecureCoding {
    return YES;
}

+ (NSParagraphStyle *) defaultParagraphStyle {
    static NSParagraphStyle *shared = nil;

    if (shared == nil)
        shared = [self new];

    return shared;
}

// Right to left for languages written that way (Arabic, Hebrew, ...), left to
// right otherwise; nil means the user's preferred language.
+ (NSWritingDirection) defaultWritingDirectionForLanguage: (NSString *) languageName {
    NSString *language = languageName ? languageName : [[NSLocale preferredLanguages] firstObject];
    if (language == nil)
        return NSWritingDirectionLeftToRight;
    return [NSLocale characterDirectionForLanguage: language] ==
                           NSLocaleLanguageDirectionRightToLeft
                   ? NSWritingDirectionRightToLeft
                   : NSWritingDirectionLeftToRight;
}

+ (NSArray *) _defaultTabStops {
    static NSArray *shared = nil;

    if (shared == nil) {
        int i, count = 12;
        NSTextTab *tabs[count];

        for (i = 0; i < count; i++) {
            tabs[i] = [[[NSTextTab alloc] initWithType: NSLeftTabStopType
                                              location: (i + 1) * 28.0]
                    autorelease];
        }
        shared = [[NSArray alloc] initWithObjects: tabs count: count];
    }

    return shared;
}

- (void) _initWithDefaults {
    _writingDirection = NSWritingDirectionLeftToRight;
    _paragraphSpacing = 0;
    _paragraphSpacingBefore = 0;
    _textBlocks = nil;
    _textLists = nil;
    _headerLevel = 0;
    _firstLineHeadIndent = 0;
    _headIndent = 0;
    _tailIndent = 0;
    _alignment = NSNaturalTextAlignment;
    _lineBreakMode = NSLineBreakByWordWrapping;
    _minimumLineHeight = 0;
    _maximumLineHeight = 0;
    _lineHeightMultiple = 0;
    _lineSpacing = 0;
    _defaultTabInterval = 0;
    _tabStops = [[[self class] _defaultTabStops] mutableCopy];
    _hyphenationFactor = 0;
    _tighteningFactorForTruncation = 0;
    _usesDefaultHyphenation = NO;
    _allowsDefaultTighteningForTruncation = NO;
    _lineBreakStrategy = NSLineBreakStrategyNone;
}

- initWithCoder: (NSCoder *) coder {
    if ([coder allowsKeyedCoding]) {
        _writingDirection = [coder decodeIntForKey: @"BaseWritingDirection"];
        _paragraphSpacing = [coder decodeFloatForKey: @"ParagraphSpacing"];
        _paragraphSpacingBefore =
                [coder decodeFloatForKey: @"ParagraphSpacingBefore"];
        _textBlocks = [[coder decodeObjectOfClasses: [NSSet setWithObjects: [NSArray class], [NSTextBlock class], nil]
                                             forKey: @"Blocks"] retain];
        _textLists = [[coder decodeObjectOfClasses: [NSSet setWithObjects: [NSArray class], [NSTextList class], nil]
                                            forKey: @"Lists"] retain];
        _headerLevel = [coder decodeIntegerForKey: @"HeaderLevel"];
        _firstLineHeadIndent =
                [coder decodeFloatForKey: @"FirstLineHeadIndent"];
        _headIndent = [coder decodeFloatForKey: @"HeadIndent"];
        _tailIndent = [coder decodeFloatForKey: @"TailIndent"];
        _alignment = [coder decodeIntForKey: @"Alignment"];
        _lineBreakMode = [coder decodeIntForKey: @"LineBreakMode"];
        _minimumLineHeight = [coder decodeFloatForKey: @"MinimumLineHeight"];
        _maximumLineHeight = [coder decodeFloatForKey: @"MaximumLineHeight"];
        _lineHeightMultiple = [coder decodeFloatForKey: @"LineHeightMultiple"];
        _lineSpacing = [coder decodeFloatForKey: @"LineSpacing"];
        _defaultTabInterval = [coder decodeFloatForKey: @"DefaultTabInterval"];
        NSArray *tabStops = [coder decodeObjectOfClasses: [NSSet setWithObjects: [NSArray class], [NSTextTab class], nil]
                                                  forKey: @"Tabs"];
        _tabStops = tabStops ? [tabStops mutableCopy] : [[[self class] _defaultTabStops] mutableCopy];
        _hyphenationFactor = [coder decodeFloatForKey: @"HyphenationFactor"];
        _tighteningFactorForTruncation =
                [coder decodeFloatForKey: @"TighteningFactor"];
        _usesDefaultHyphenation = [coder decodeBoolForKey: @"UsesDefaultHyphenation"];
        _allowsDefaultTighteningForTruncation =
                [coder decodeBoolForKey: @"AllowsDefaultTighteningForTruncation"];
        _lineBreakStrategy = [coder decodeIntegerForKey: @"LineBreakStrategy"];
    } else {
        // QUIRK: NSArchiver (typedstream) layout of a paragraph style: the
        // type string "CC@S" (alignment in the legacy NSTextAlignment
        // numbering, the line break mode, the tab stops or nil, a short of
        // flags), then, when any flag is set, an array "[Nf]" with one float per
        // set flag in ascending bit order: 0x01 line spacing, 0x02 paragraph
        // spacing, 0x04 head indent, 0x08 tail indent, 0x10 first-line head
        // indent, 0x20 minimum line height, 0x40 maximum line height, 0x80 line
        // height multiple, 0x100 paragraph spacing before, 0x200 default tab
        // interval, 0x400 hyphenation factor. The base writing direction is not
        // archived. Flags outside that set are refused rather than guessed. Not
        // documented. Evidence: archives written by NSArchiver on macOS 26.7.3
        // (tier 2, MC26.12, MAC-EVIDENCE.md and mac-probes/v246-paragraphstyle-archive.m),
        // earlier an archive written on macOS 27 (MC13.7, fixture
        // paragraphstyle.typedstream). Test: test_quirk_typedstream_paragraph_style
        // and test_quirk_typedstream_paragraph_style_flags in
        // tests/appkit-typedstream-text.m
        unsigned char alignment, lineBreakMode;
        NSArray *tabStops;
        unsigned short flags;
        [coder decodeValuesOfObjCTypes: "CC@S", &alignment, &lineBreakMode, &tabStops, &flags];
        [tabStops autorelease];
        if (alignment > NSNaturalTextAlignment || lineBreakMode > NSLineBreakByTruncatingMiddle ||
            (flags & ~0x7ffu) != 0)
            [NSException raise: NSInvalidArgumentException
                        format: @"-[%@ %s]: unsupported archived values %u %u %u", [self class], sel_getName(_cmd),
                                alignment, lineBreakMode, flags];
        float values[11];
        unsigned valueCount = 0;
        for (unsigned bit = 0; bit < 11; bit++)
            if (flags & (1u << bit))
                valueCount++;
        if (valueCount > 0) {
            char type[16];
            snprintf(type, sizeof(type), "[%uf]", valueCount);
            [coder decodeValueOfObjCType: type at: values];
        }
        float fields[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        for (unsigned bit = 0, next = 0; bit < 11; bit++)
            if (flags & (1u << bit))
                fields[bit] = values[next++];
        [self _initWithDefaults];
        _alignment = alignment;
        _lineBreakMode = lineBreakMode;
        _lineSpacing = fields[0];
        _paragraphSpacing = fields[1];
        _headIndent = fields[2];
        _tailIndent = fields[3];
        _firstLineHeadIndent = fields[4];
        _minimumLineHeight = fields[5];
        _maximumLineHeight = fields[6];
        _lineHeightMultiple = fields[7];
        _paragraphSpacingBefore = fields[8];
        _defaultTabInterval = fields[9];
        _hyphenationFactor = fields[10];
        if (tabStops != nil) {
            [_tabStops release];
            _tabStops = [tabStops mutableCopy];
        }
    }

    return self;
}

- init {
    [self _initWithDefaults];
    return self;
}

- initWithParagraphStyle: (NSParagraphStyle *) other {
    _writingDirection = other->_writingDirection;
    _paragraphSpacing = other->_paragraphSpacing;
    _paragraphSpacingBefore = other->_paragraphSpacingBefore;
    _textBlocks = [other->_textBlocks copy];
    _textLists = [other->_textLists copy];
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
    _tabStops = [other->_tabStops mutableCopy];
    _hyphenationFactor = other->_hyphenationFactor;
    _tighteningFactorForTruncation = other->_tighteningFactorForTruncation;
    _horizontalAlignment = other->_horizontalAlignment;
    _usesDefaultHyphenation = other->_usesDefaultHyphenation;
    _allowsDefaultTighteningForTruncation = other->_allowsDefaultTighteningForTruncation;
    _lineBreakStrategy = other->_lineBreakStrategy;
    return self;
}

- (void) encodeWithCoder: (NSCoder *) coder {
    if ([coder isKindOfClass: [NSKeyedArchiver class]]) {
        [coder encodeInt: _writingDirection forKey: @"BaseWritingDirection"];
        [coder encodeFloat: _paragraphSpacing forKey: @"ParagraphSpacing"];
        [coder encodeFloat: _paragraphSpacingBefore
                    forKey: @"ParagraphSpacingBefore"];
        [coder encodeObject: _textBlocks forKey: @"Blocks"];
        [coder encodeObject: _textLists forKey: @"Lists"];
        [coder encodeInteger: _headerLevel forKey: @"HeaderLevel"];
        [coder encodeFloat: _firstLineHeadIndent
                    forKey: @"FirstLineHeadIndent"];
        [coder encodeFloat: _headIndent forKey: @"HeadIndent"];
        [coder encodeFloat: _tailIndent forKey: @"TailIndent"];
        [coder encodeInt: _alignment forKey: @"Alignment"];
        [coder encodeInt: _lineBreakMode forKey: @"LineBreakMode"];
        [coder encodeFloat: _minimumLineHeight forKey: @"MinimumLineHeight"];
        [coder encodeFloat: _maximumLineHeight forKey: @"MaximumLineHeight"];
        [coder encodeFloat: _lineHeightMultiple forKey: @"LineHeightMultiple"];
        [coder encodeFloat: _lineSpacing forKey: @"LineSpacing"];
        [coder encodeFloat: _defaultTabInterval forKey: @"DefaultTabInterval"];
        [coder encodeObject: _tabStops forKey: @"Tabs"];
        [coder encodeFloat: _hyphenationFactor forKey: @"HyphenationFactor"];
        [coder encodeFloat: _tighteningFactorForTruncation
                    forKey: @"TighteningFactor"];
        [coder encodeBool: _usesDefaultHyphenation forKey: @"UsesDefaultHyphenation"];
        [coder encodeBool: _allowsDefaultTighteningForTruncation
                   forKey: @"AllowsDefaultTighteningForTruncation"];
        [coder encodeInteger: _lineBreakStrategy forKey: @"LineBreakStrategy"];
    } else {
        NSUnimplementedMethod();
    }
}

- (void) dealloc {
    [_textBlocks release];
    [_textLists release];
    [_tabStops release];
    [super dealloc];
}

- copyWithZone: (NSZone *) zone {
    return [self retain];
}

static inline id mutableCopyWithZone(NSParagraphStyle *self, NSZone *zone) {
    return [[NSMutableParagraphStyle allocWithZone: zone]
            initWithParagraphStyle: self];
}

- mutableCopy {
    return mutableCopyWithZone(self, NULL);
}

- mutableCopyWithZone: (NSZone *) zone {
    return mutableCopyWithZone(self, zone);
}

- (NSInteger) horizontalAlignment {
    return _horizontalAlignment;
}

- (NSWritingDirection) baseWritingDirection {
    return _writingDirection;
}

- (CGFloat) paragraphSpacing {
    return _paragraphSpacing;
}

- (CGFloat) paragraphSpacingBefore {
    return _paragraphSpacingBefore;
}

- (NSArray *) textBlocks {
    return _textBlocks;
}

- (NSArray *) textLists {
    return _textLists;
}

- (NSInteger) headerLevel {
    return _headerLevel;
}

- (CGFloat) firstLineHeadIndent {
    return _firstLineHeadIndent;
}

- (CGFloat) headIndent {
    return _headIndent;
}

- (CGFloat) tailIndent {
    return _tailIndent;
}

- (NSTextAlignment) alignment {
    return _alignment;
}

- (NSLineBreakMode) lineBreakMode {
    return _lineBreakMode;
}

- (CGFloat) minimumLineHeight {
    return _minimumLineHeight;
}

- (CGFloat) maximumLineHeight {
    return _maximumLineHeight;
}

- (CGFloat) lineHeightMultiple {
    return _lineHeightMultiple;
}

- (CGFloat) lineSpacing {
    return _lineSpacing;
}

- (CGFloat) defaultTabInterval {
    return _defaultTabInterval;
}

- (NSArray *) tabStops {
    return [[_tabStops copy] autorelease];
}

- (float) hyphenationFactor {
    return _hyphenationFactor;
}

- (float) tighteningFactorForTruncation {
    return _tighteningFactorForTruncation;
}

- (BOOL) usesDefaultHyphenation {
    return _usesDefaultHyphenation;
}

- (BOOL) allowsDefaultTighteningForTruncation {
    return _allowsDefaultTighteningForTruncation;
}

- (NSLineBreakStrategy) lineBreakStrategy {
    return _lineBreakStrategy;
}

- (BOOL) isEqual: (id) object {
    if (self == object) {
        return YES;
    }
    if (![object isKindOfClass: [NSParagraphStyle class]]) {
        return NO;
    }
    NSParagraphStyle *other = (NSParagraphStyle *) object;
    return (_writingDirection == other->_writingDirection &&
            _paragraphSpacing == other->_paragraphSpacing &&
            _paragraphSpacingBefore == other->_paragraphSpacingBefore &&
            _headerLevel == other->_headerLevel &&
            _firstLineHeadIndent == other->_firstLineHeadIndent &&
            _headIndent == other->_headIndent &&
            _tailIndent == other->_tailIndent &&
            _alignment == other->_alignment &&
            _lineBreakMode == other->_lineBreakMode &&
            _minimumLineHeight == other->_minimumLineHeight &&
            _maximumLineHeight == other->_maximumLineHeight &&
            _lineHeightMultiple == other->_lineHeightMultiple &&
            _lineSpacing == other->_lineSpacing &&
            _defaultTabInterval == other->_defaultTabInterval &&
            _hyphenationFactor == other->_hyphenationFactor &&
            _tighteningFactorForTruncation ==
                    other->_tighteningFactorForTruncation &&
            _usesDefaultHyphenation == other->_usesDefaultHyphenation &&
            _allowsDefaultTighteningForTruncation ==
                    other->_allowsDefaultTighteningForTruncation &&
            _lineBreakStrategy == other->_lineBreakStrategy &&
            (_tabStops == other->_tabStops ||
             [_tabStops isEqual: other->_tabStops]) &&
            (_textBlocks == other->_textBlocks ||
             [_textBlocks isEqual: other->_textBlocks]) &&
            (_textLists == other->_textLists ||
             [_textLists isEqual: other->_textLists]));
}

- (NSString *) description {
    NSMutableString *tabsString = [NSMutableString stringWithString: @" (\n"];

    for (NSTextTab *tab in _tabStops) {
        switch ([tab tabStopType]) {
        case NSLeftTabStopType:
            [tabsString
                    appendString: [NSString stringWithFormat: @"%fL",
                                                              [tab location]]];
            break;
        case NSRightTabStopType:
            [tabsString
                    appendString: [NSString stringWithFormat: @"%fR",
                                                              [tab location]]];
            break;
        case NSCenterTabStopType:
            [tabsString
                    appendString: [NSString stringWithFormat: @"%fC",
                                                              [tab location]]];
            break;
        case NSDecimalTabStopType:
            [tabsString
                    appendString: [NSString stringWithFormat: @"%fD",
                                                              [tab location]]];
            break;
        }
    }
    [tabsString appendString: @")"];

    return [NSString
            stringWithFormat:
                    @"Alignment %ld, LineSpacing %f, ParagraphSpacing %f, "
                    @"ParagraphSpacingBefore %f, HeadIndent %f, TailIndent %f, "
                    @"FirstLineHeadIndent %f, "
                    @"LineHeight %f/%f, LineHeightMultiple %f, LineBreakMode "
                    @"%lu, Tabs "
                    @"%@, "
                    @"DefaultTabInterval %f, Blocks %@, Lists %@, "
                    @"BaseWritingDirection "
                    @"%ld, "
                    @"HyphenationFactor %f, TighteningFactor %f, HeaderLevel "
                    @"%ld",
                    (long) _alignment, _lineSpacing, _paragraphSpacing,
                    _paragraphSpacingBefore, _headIndent, _tailIndent,
                    _firstLineHeadIndent, _minimumLineHeight,
                    _maximumLineHeight, _lineHeightMultiple, (unsigned long) _lineBreakMode,
                    tabsString, _defaultTabInterval, _textBlocks, _textLists,
                    (long) _writingDirection, _hyphenationFactor,
                    _tighteningFactorForTruncation, (long) _headerLevel];
}

@end
