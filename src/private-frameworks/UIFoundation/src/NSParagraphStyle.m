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
#include <stdio.h>
#import <Foundation/NSKeyedArchiver.h>
#import <Foundation/NSLocale.h>

#import "NSParagraphStyle-Private.h"
#import <Foundation/NSRaise.h>
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
    UIF_EXTRA_NSParagraphStyle(self)->_writingDirection = NSWritingDirectionLeftToRight;
    UIF_EXTRA_NSParagraphStyle(self)->_paragraphSpacing = 0;
    UIF_EXTRA_NSParagraphStyle(self)->_paragraphSpacingBefore = 0;
    UIF_EXTRA_NSParagraphStyle(self)->_textBlocks = nil;
    UIF_EXTRA_NSParagraphStyle(self)->_textLists = nil;
    UIF_EXTRA_NSParagraphStyle(self)->_headerLevel = 0;
    UIF_EXTRA_NSParagraphStyle(self)->_firstLineHeadIndent = 0;
    UIF_EXTRA_NSParagraphStyle(self)->_headIndent = 0;
    UIF_EXTRA_NSParagraphStyle(self)->_tailIndent = 0;
    UIF_EXTRA_NSParagraphStyle(self)->_alignment = NSNaturalTextAlignment;
    UIF_EXTRA_NSParagraphStyle(self)->_lineBreakMode = NSLineBreakByWordWrapping;
    UIF_EXTRA_NSParagraphStyle(self)->_minimumLineHeight = 0;
    UIF_EXTRA_NSParagraphStyle(self)->_maximumLineHeight = 0;
    UIF_EXTRA_NSParagraphStyle(self)->_lineHeightMultiple = 0;
    UIF_EXTRA_NSParagraphStyle(self)->_lineSpacing = 0;
    UIF_EXTRA_NSParagraphStyle(self)->_defaultTabInterval = 0;
    UIF_EXTRA_NSParagraphStyle(self)->_tabStops = [[[self class] _defaultTabStops] mutableCopy];
    UIF_EXTRA_NSParagraphStyle(self)->_hyphenationFactor = 0;
    UIF_EXTRA_NSParagraphStyle(self)->_tighteningFactorForTruncation = 0;
    UIF_EXTRA_NSParagraphStyle(self)->_usesDefaultHyphenation = NO;
    UIF_EXTRA_NSParagraphStyle(self)->_allowsDefaultTighteningForTruncation = NO;
    UIF_EXTRA_NSParagraphStyle(self)->_lineBreakStrategy = NSLineBreakStrategyNone;
}

- initWithCoder: (NSCoder *) coder {
    if ([coder allowsKeyedCoding]) {
        UIF_EXTRA_NSParagraphStyle(self)->_writingDirection = [coder decodeIntForKey: @"BaseWritingDirection"];
        UIF_EXTRA_NSParagraphStyle(self)->_paragraphSpacing = [coder decodeFloatForKey: @"ParagraphSpacing"];
        UIF_EXTRA_NSParagraphStyle(self)->_paragraphSpacingBefore =
                [coder decodeFloatForKey: @"ParagraphSpacingBefore"];
        UIF_EXTRA_NSParagraphStyle(self)->_textBlocks = [[coder decodeObjectOfClasses: [NSSet setWithObjects: [NSArray class], [NSTextBlock class], nil]
                                             forKey: @"Blocks"] retain];
        UIF_EXTRA_NSParagraphStyle(self)->_textLists = [[coder decodeObjectOfClasses: [NSSet setWithObjects: [NSArray class], [NSTextList class], nil]
                                            forKey: @"Lists"] retain];
        UIF_EXTRA_NSParagraphStyle(self)->_headerLevel = [coder decodeIntegerForKey: @"HeaderLevel"];
        UIF_EXTRA_NSParagraphStyle(self)->_firstLineHeadIndent =
                [coder decodeFloatForKey: @"FirstLineHeadIndent"];
        UIF_EXTRA_NSParagraphStyle(self)->_headIndent = [coder decodeFloatForKey: @"HeadIndent"];
        UIF_EXTRA_NSParagraphStyle(self)->_tailIndent = [coder decodeFloatForKey: @"TailIndent"];
        UIF_EXTRA_NSParagraphStyle(self)->_alignment = [coder decodeIntForKey: @"Alignment"];
        UIF_EXTRA_NSParagraphStyle(self)->_lineBreakMode = [coder decodeIntForKey: @"LineBreakMode"];
        UIF_EXTRA_NSParagraphStyle(self)->_minimumLineHeight = [coder decodeFloatForKey: @"MinimumLineHeight"];
        UIF_EXTRA_NSParagraphStyle(self)->_maximumLineHeight = [coder decodeFloatForKey: @"MaximumLineHeight"];
        UIF_EXTRA_NSParagraphStyle(self)->_lineHeightMultiple = [coder decodeFloatForKey: @"LineHeightMultiple"];
        UIF_EXTRA_NSParagraphStyle(self)->_lineSpacing = [coder decodeFloatForKey: @"LineSpacing"];
        UIF_EXTRA_NSParagraphStyle(self)->_defaultTabInterval = [coder decodeFloatForKey: @"DefaultTabInterval"];
        NSArray *tabStops = [coder decodeObjectOfClasses: [NSSet setWithObjects: [NSArray class], [NSTextTab class], nil]
                                                  forKey: @"Tabs"];
        UIF_EXTRA_NSParagraphStyle(self)->_tabStops = tabStops ? [tabStops mutableCopy] : [[[self class] _defaultTabStops] mutableCopy];
        UIF_EXTRA_NSParagraphStyle(self)->_hyphenationFactor = [coder decodeFloatForKey: @"HyphenationFactor"];
        UIF_EXTRA_NSParagraphStyle(self)->_tighteningFactorForTruncation =
                [coder decodeFloatForKey: @"TighteningFactor"];
        UIF_EXTRA_NSParagraphStyle(self)->_usesDefaultHyphenation = [coder decodeBoolForKey: @"UsesDefaultHyphenation"];
        UIF_EXTRA_NSParagraphStyle(self)->_allowsDefaultTighteningForTruncation =
                [coder decodeBoolForKey: @"AllowsDefaultTighteningForTruncation"];
        UIF_EXTRA_NSParagraphStyle(self)->_lineBreakStrategy = [coder decodeIntegerForKey: @"LineBreakStrategy"];
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
        UIF_EXTRA_NSParagraphStyle(self)->_alignment = alignment;
        UIF_EXTRA_NSParagraphStyle(self)->_lineBreakMode = lineBreakMode;
        UIF_EXTRA_NSParagraphStyle(self)->_lineSpacing = fields[0];
        UIF_EXTRA_NSParagraphStyle(self)->_paragraphSpacing = fields[1];
        UIF_EXTRA_NSParagraphStyle(self)->_headIndent = fields[2];
        UIF_EXTRA_NSParagraphStyle(self)->_tailIndent = fields[3];
        UIF_EXTRA_NSParagraphStyle(self)->_firstLineHeadIndent = fields[4];
        UIF_EXTRA_NSParagraphStyle(self)->_minimumLineHeight = fields[5];
        UIF_EXTRA_NSParagraphStyle(self)->_maximumLineHeight = fields[6];
        UIF_EXTRA_NSParagraphStyle(self)->_lineHeightMultiple = fields[7];
        UIF_EXTRA_NSParagraphStyle(self)->_paragraphSpacingBefore = fields[8];
        UIF_EXTRA_NSParagraphStyle(self)->_defaultTabInterval = fields[9];
        UIF_EXTRA_NSParagraphStyle(self)->_hyphenationFactor = fields[10];
        if (tabStops != nil) {
            [UIF_EXTRA_NSParagraphStyle(self)->_tabStops release];
            UIF_EXTRA_NSParagraphStyle(self)->_tabStops = [tabStops mutableCopy];
        }
    }

    return self;
}

- init {
    [self _initWithDefaults];
    return self;
}

- initWithParagraphStyle: (NSParagraphStyle *) other {
    UIF_EXTRA_NSParagraphStyle(self)->_writingDirection = UIF_EXTRA_NSParagraphStyle(other)->_writingDirection;
    UIF_EXTRA_NSParagraphStyle(self)->_paragraphSpacing = UIF_EXTRA_NSParagraphStyle(other)->_paragraphSpacing;
    UIF_EXTRA_NSParagraphStyle(self)->_paragraphSpacingBefore = UIF_EXTRA_NSParagraphStyle(other)->_paragraphSpacingBefore;
    UIF_EXTRA_NSParagraphStyle(self)->_textBlocks = [UIF_EXTRA_NSParagraphStyle(other)->_textBlocks copy];
    UIF_EXTRA_NSParagraphStyle(self)->_textLists = [UIF_EXTRA_NSParagraphStyle(other)->_textLists copy];
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
    UIF_EXTRA_NSParagraphStyle(self)->_tabStops = [UIF_EXTRA_NSParagraphStyle(other)->_tabStops mutableCopy];
    UIF_EXTRA_NSParagraphStyle(self)->_hyphenationFactor = UIF_EXTRA_NSParagraphStyle(other)->_hyphenationFactor;
    UIF_EXTRA_NSParagraphStyle(self)->_tighteningFactorForTruncation = UIF_EXTRA_NSParagraphStyle(other)->_tighteningFactorForTruncation;
    UIF_EXTRA_NSParagraphStyle(self)->_horizontalAlignment = UIF_EXTRA_NSParagraphStyle(other)->_horizontalAlignment;
    UIF_EXTRA_NSParagraphStyle(self)->_usesDefaultHyphenation = UIF_EXTRA_NSParagraphStyle(other)->_usesDefaultHyphenation;
    UIF_EXTRA_NSParagraphStyle(self)->_allowsDefaultTighteningForTruncation = UIF_EXTRA_NSParagraphStyle(other)->_allowsDefaultTighteningForTruncation;
    UIF_EXTRA_NSParagraphStyle(self)->_lineBreakStrategy = UIF_EXTRA_NSParagraphStyle(other)->_lineBreakStrategy;
    return self;
}

- (void) encodeWithCoder: (NSCoder *) coder {
    if ([coder isKindOfClass: [NSKeyedArchiver class]]) {
        [coder encodeInt: UIF_EXTRA_NSParagraphStyle(self)->_writingDirection forKey: @"BaseWritingDirection"];
        [coder encodeFloat: UIF_EXTRA_NSParagraphStyle(self)->_paragraphSpacing forKey: @"ParagraphSpacing"];
        [coder encodeFloat: UIF_EXTRA_NSParagraphStyle(self)->_paragraphSpacingBefore
                    forKey: @"ParagraphSpacingBefore"];
        [coder encodeObject: UIF_EXTRA_NSParagraphStyle(self)->_textBlocks forKey: @"Blocks"];
        [coder encodeObject: UIF_EXTRA_NSParagraphStyle(self)->_textLists forKey: @"Lists"];
        [coder encodeInteger: UIF_EXTRA_NSParagraphStyle(self)->_headerLevel forKey: @"HeaderLevel"];
        [coder encodeFloat: UIF_EXTRA_NSParagraphStyle(self)->_firstLineHeadIndent
                    forKey: @"FirstLineHeadIndent"];
        [coder encodeFloat: UIF_EXTRA_NSParagraphStyle(self)->_headIndent forKey: @"HeadIndent"];
        [coder encodeFloat: UIF_EXTRA_NSParagraphStyle(self)->_tailIndent forKey: @"TailIndent"];
        [coder encodeInt: UIF_EXTRA_NSParagraphStyle(self)->_alignment forKey: @"Alignment"];
        [coder encodeInt: UIF_EXTRA_NSParagraphStyle(self)->_lineBreakMode forKey: @"LineBreakMode"];
        [coder encodeFloat: UIF_EXTRA_NSParagraphStyle(self)->_minimumLineHeight forKey: @"MinimumLineHeight"];
        [coder encodeFloat: UIF_EXTRA_NSParagraphStyle(self)->_maximumLineHeight forKey: @"MaximumLineHeight"];
        [coder encodeFloat: UIF_EXTRA_NSParagraphStyle(self)->_lineHeightMultiple forKey: @"LineHeightMultiple"];
        [coder encodeFloat: UIF_EXTRA_NSParagraphStyle(self)->_lineSpacing forKey: @"LineSpacing"];
        [coder encodeFloat: UIF_EXTRA_NSParagraphStyle(self)->_defaultTabInterval forKey: @"DefaultTabInterval"];
        [coder encodeObject: UIF_EXTRA_NSParagraphStyle(self)->_tabStops forKey: @"Tabs"];
        [coder encodeFloat: UIF_EXTRA_NSParagraphStyle(self)->_hyphenationFactor forKey: @"HyphenationFactor"];
        [coder encodeFloat: UIF_EXTRA_NSParagraphStyle(self)->_tighteningFactorForTruncation
                    forKey: @"TighteningFactor"];
        [coder encodeBool: UIF_EXTRA_NSParagraphStyle(self)->_usesDefaultHyphenation forKey: @"UsesDefaultHyphenation"];
        [coder encodeBool: UIF_EXTRA_NSParagraphStyle(self)->_allowsDefaultTighteningForTruncation
                   forKey: @"AllowsDefaultTighteningForTruncation"];
        [coder encodeInteger: UIF_EXTRA_NSParagraphStyle(self)->_lineBreakStrategy forKey: @"LineBreakStrategy"];
    } else {
        NSUnimplementedMethod();
    }
}

- (void) dealloc {
    [UIF_EXTRA_NSParagraphStyle(self)->_textBlocks release];
    [UIF_EXTRA_NSParagraphStyle(self)->_textLists release];
    [UIF_EXTRA_NSParagraphStyle(self)->_tabStops release];
    UIF_EXTRA_FREE(NSParagraphStyle, self);
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
    return UIF_EXTRA_NSParagraphStyle(self)->_horizontalAlignment;
}

- (NSWritingDirection) baseWritingDirection {
    return UIF_EXTRA_NSParagraphStyle(self)->_writingDirection;
}

- (CGFloat) paragraphSpacing {
    return UIF_EXTRA_NSParagraphStyle(self)->_paragraphSpacing;
}

- (CGFloat) paragraphSpacingBefore {
    return UIF_EXTRA_NSParagraphStyle(self)->_paragraphSpacingBefore;
}

- (NSArray *) textBlocks {
    return UIF_EXTRA_NSParagraphStyle(self)->_textBlocks;
}

- (NSArray *) textLists {
    return UIF_EXTRA_NSParagraphStyle(self)->_textLists;
}

- (NSInteger) headerLevel {
    return UIF_EXTRA_NSParagraphStyle(self)->_headerLevel;
}

- (CGFloat) firstLineHeadIndent {
    return UIF_EXTRA_NSParagraphStyle(self)->_firstLineHeadIndent;
}

- (CGFloat) headIndent {
    return UIF_EXTRA_NSParagraphStyle(self)->_headIndent;
}

- (CGFloat) tailIndent {
    return UIF_EXTRA_NSParagraphStyle(self)->_tailIndent;
}

- (NSTextAlignment) alignment {
    return UIF_EXTRA_NSParagraphStyle(self)->_alignment;
}

- (NSLineBreakMode) lineBreakMode {
    return UIF_EXTRA_NSParagraphStyle(self)->_lineBreakMode;
}

- (CGFloat) minimumLineHeight {
    return UIF_EXTRA_NSParagraphStyle(self)->_minimumLineHeight;
}

- (CGFloat) maximumLineHeight {
    return UIF_EXTRA_NSParagraphStyle(self)->_maximumLineHeight;
}

- (CGFloat) lineHeightMultiple {
    return UIF_EXTRA_NSParagraphStyle(self)->_lineHeightMultiple;
}

- (CGFloat) lineSpacing {
    return UIF_EXTRA_NSParagraphStyle(self)->_lineSpacing;
}

- (CGFloat) defaultTabInterval {
    return UIF_EXTRA_NSParagraphStyle(self)->_defaultTabInterval;
}

- (NSArray *) tabStops {
    return [[UIF_EXTRA_NSParagraphStyle(self)->_tabStops copy] autorelease];
}

- (float) hyphenationFactor {
    return UIF_EXTRA_NSParagraphStyle(self)->_hyphenationFactor;
}

- (float) tighteningFactorForTruncation {
    return UIF_EXTRA_NSParagraphStyle(self)->_tighteningFactorForTruncation;
}

- (BOOL) usesDefaultHyphenation {
    return UIF_EXTRA_NSParagraphStyle(self)->_usesDefaultHyphenation;
}

- (BOOL) allowsDefaultTighteningForTruncation {
    return UIF_EXTRA_NSParagraphStyle(self)->_allowsDefaultTighteningForTruncation;
}

- (NSLineBreakStrategy) lineBreakStrategy {
    return UIF_EXTRA_NSParagraphStyle(self)->_lineBreakStrategy;
}

- (BOOL) isEqual: (id) object {
    if (self == object) {
        return YES;
    }
    if (![object isKindOfClass: [NSParagraphStyle class]]) {
        return NO;
    }
    NSParagraphStyle *other = (NSParagraphStyle *) object;
    return (UIF_EXTRA_NSParagraphStyle(self)->_writingDirection == UIF_EXTRA_NSParagraphStyle(other)->_writingDirection &&
            UIF_EXTRA_NSParagraphStyle(self)->_paragraphSpacing == UIF_EXTRA_NSParagraphStyle(other)->_paragraphSpacing &&
            UIF_EXTRA_NSParagraphStyle(self)->_paragraphSpacingBefore == UIF_EXTRA_NSParagraphStyle(other)->_paragraphSpacingBefore &&
            UIF_EXTRA_NSParagraphStyle(self)->_headerLevel == UIF_EXTRA_NSParagraphStyle(other)->_headerLevel &&
            UIF_EXTRA_NSParagraphStyle(self)->_firstLineHeadIndent == UIF_EXTRA_NSParagraphStyle(other)->_firstLineHeadIndent &&
            UIF_EXTRA_NSParagraphStyle(self)->_headIndent == UIF_EXTRA_NSParagraphStyle(other)->_headIndent &&
            UIF_EXTRA_NSParagraphStyle(self)->_tailIndent == UIF_EXTRA_NSParagraphStyle(other)->_tailIndent &&
            UIF_EXTRA_NSParagraphStyle(self)->_alignment == UIF_EXTRA_NSParagraphStyle(other)->_alignment &&
            UIF_EXTRA_NSParagraphStyle(self)->_lineBreakMode == UIF_EXTRA_NSParagraphStyle(other)->_lineBreakMode &&
            UIF_EXTRA_NSParagraphStyle(self)->_minimumLineHeight == UIF_EXTRA_NSParagraphStyle(other)->_minimumLineHeight &&
            UIF_EXTRA_NSParagraphStyle(self)->_maximumLineHeight == UIF_EXTRA_NSParagraphStyle(other)->_maximumLineHeight &&
            UIF_EXTRA_NSParagraphStyle(self)->_lineHeightMultiple == UIF_EXTRA_NSParagraphStyle(other)->_lineHeightMultiple &&
            UIF_EXTRA_NSParagraphStyle(self)->_lineSpacing == UIF_EXTRA_NSParagraphStyle(other)->_lineSpacing &&
            UIF_EXTRA_NSParagraphStyle(self)->_defaultTabInterval == UIF_EXTRA_NSParagraphStyle(other)->_defaultTabInterval &&
            UIF_EXTRA_NSParagraphStyle(self)->_hyphenationFactor == UIF_EXTRA_NSParagraphStyle(other)->_hyphenationFactor &&
            UIF_EXTRA_NSParagraphStyle(self)->_tighteningFactorForTruncation ==
                    UIF_EXTRA_NSParagraphStyle(other)->_tighteningFactorForTruncation &&
            UIF_EXTRA_NSParagraphStyle(self)->_usesDefaultHyphenation == UIF_EXTRA_NSParagraphStyle(other)->_usesDefaultHyphenation &&
            UIF_EXTRA_NSParagraphStyle(self)->_allowsDefaultTighteningForTruncation ==
                    UIF_EXTRA_NSParagraphStyle(other)->_allowsDefaultTighteningForTruncation &&
            UIF_EXTRA_NSParagraphStyle(self)->_lineBreakStrategy == UIF_EXTRA_NSParagraphStyle(other)->_lineBreakStrategy &&
            (UIF_EXTRA_NSParagraphStyle(self)->_tabStops == UIF_EXTRA_NSParagraphStyle(other)->_tabStops ||
             [UIF_EXTRA_NSParagraphStyle(self)->_tabStops isEqual: UIF_EXTRA_NSParagraphStyle(other)->_tabStops]) &&
            (UIF_EXTRA_NSParagraphStyle(self)->_textBlocks == UIF_EXTRA_NSParagraphStyle(other)->_textBlocks ||
             [UIF_EXTRA_NSParagraphStyle(self)->_textBlocks isEqual: UIF_EXTRA_NSParagraphStyle(other)->_textBlocks]) &&
            (UIF_EXTRA_NSParagraphStyle(self)->_textLists == UIF_EXTRA_NSParagraphStyle(other)->_textLists ||
             [UIF_EXTRA_NSParagraphStyle(self)->_textLists isEqual: UIF_EXTRA_NSParagraphStyle(other)->_textLists]));
}

- (NSString *) description {
    NSMutableString *tabsString = [NSMutableString stringWithString: @" (\n"];

    for (NSTextTab *tab in UIF_EXTRA_NSParagraphStyle(self)->_tabStops) {
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
                    (long) UIF_EXTRA_NSParagraphStyle(self)->_alignment, UIF_EXTRA_NSParagraphStyle(self)->_lineSpacing, UIF_EXTRA_NSParagraphStyle(self)->_paragraphSpacing,
                    UIF_EXTRA_NSParagraphStyle(self)->_paragraphSpacingBefore, UIF_EXTRA_NSParagraphStyle(self)->_headIndent, UIF_EXTRA_NSParagraphStyle(self)->_tailIndent,
                    UIF_EXTRA_NSParagraphStyle(self)->_firstLineHeadIndent, UIF_EXTRA_NSParagraphStyle(self)->_minimumLineHeight,
                    UIF_EXTRA_NSParagraphStyle(self)->_maximumLineHeight, UIF_EXTRA_NSParagraphStyle(self)->_lineHeightMultiple, (unsigned long) UIF_EXTRA_NSParagraphStyle(self)->_lineBreakMode,
                    tabsString, UIF_EXTRA_NSParagraphStyle(self)->_defaultTabInterval, UIF_EXTRA_NSParagraphStyle(self)->_textBlocks, UIF_EXTRA_NSParagraphStyle(self)->_textLists,
                    (long) UIF_EXTRA_NSParagraphStyle(self)->_writingDirection, UIF_EXTRA_NSParagraphStyle(self)->_hyphenationFactor,
                    UIF_EXTRA_NSParagraphStyle(self)->_tighteningFactorForTruncation, (long) UIF_EXTRA_NSParagraphStyle(self)->_headerLevel];
}

@end
