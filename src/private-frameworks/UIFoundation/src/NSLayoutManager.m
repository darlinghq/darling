/* Copyright (c) 2006-2009 Christopher J. W. Lloyd <cjwl@objc.net>

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

#import <AppKit/NSGlyphGenerator.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSLayoutManager.h>
#import <AppKit/NSTextAttachment.h>
#import <AppKit/NSTextContainer.h>
#import <AppKit/NSTextStorage.h>
#import <AppKit/NSTextView.h>
#import <AppKit/NSTypesetter.h>
#import <AppKit/NSWindow.h>

#import <AppKit/NSAttributedString.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSParagraphStyle.h>
#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/NSKeyedArchiver.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSRaiseException.h>
#import <Foundation/NSRangeEntries.h>
#import <UIFoundation/UIFoundationGraphicsContext.h>

#import "NSBidiHelper.h"
#import "NSLayoutManager-Private.h"

typedef struct {
    NSRect rect;
    NSRect usedRect;
    NSPoint location;
    NSTextContainer *container;
    BOOL leftToRight;
} NSGlyphFragment;

typedef struct {
    int _xxxNeedSomething;
} NSInvalidFragment;

// Forward declaration
@interface NSLayoutManager ()
- (NSRange) validateGlyphsAndLayoutForGlyphRange: (NSRange) glyphRange;
- (NSRange) _currentGlyphRangeForTextContainer: (NSTextContainer *) container;
@end

@interface NSTextContainer (NSTextContainer_layoutManagerDetach)
- (void) _darlingDetachFromLayoutManager: (NSLayoutManager *) layoutManager;
@end

@implementation NSLayoutManager

static inline NSGlyphFragment *fragmentForGlyphRange(NSLayoutManager *self,
                                                     NSRange range)
{
    NSGlyphFragment *result = NSRangeEntryAtRange(self->_glyphFragments, range);

    if (result == NULL) {
        // That can happens in normal cases, so we don't want to crash or log
        // that. For example when some text can't be layout (too small
        // container...)
        //	[NSException raise:NSGenericException
        // format:@"fragmentForGlyphRange fragment is NULL for range %d
        //%d",range.location,range.length];
    }
    return result;
}

static inline NSGlyphFragment *fragmentAtGlyphIndex(NSLayoutManager *self,
                                                    NSUInteger index,
                                                    NSRange *effectiveRange)
{
    NSGlyphFragment *result =
            NSRangeEntryAtIndex(self->_glyphFragments, index, effectiveRange);

    if (result == NULL) {
        // That can happens in normal cases, so we don't want to crash or log
        // that. For example when some text can't be layout (too small
        // container...)
        //  [NSException raise:NSGenericException format:@"fragmentAtGlyphIndex
        //  fragment is NULL for index %d",index];
    }
    return result;
}

- initWithCoder: (NSCoder *) coder {
    if ([coder allowsKeyedCoding]) {
        NSKeyedUnarchiver *keyed = (NSKeyedUnarchiver *) coder;

        // The text storage owns the layout manager (and text view can own the
        // storage) Retaining it here would create a retain cycle.
        _textStorage = [[[keyed decodeObjectForKey: @"NSTextStorage"] retain]
                autorelease];
        [_textStorage addLayoutManager: self];

        _rangeToTemporaryAttributes = NSCreateRangeToCopiedObjectEntries(0);
        NSRangeEntryInsert(_rangeToTemporaryAttributes,
                           NSMakeRange(0, [_textStorage length]),
                           [NSDictionary dictionary]);

        _typesetter = [NSTypesetter new];
        _glyphGenerator = [[NSGlyphGenerator sharedGlyphGenerator] retain];
        _delegate = [keyed decodeObjectForKey: @"NSDelegate"];
        _textContainers = [NSMutableArray new];
        [_textContainers
                addObjectsFromArray:
                        [keyed decodeObjectForKey: @"NSTextContainers"]];
        for (NSTextContainer *container in _textContainers) {
            if ([container layoutManager] == nil) {
                [container setLayoutManager: self];
            }
        }
        _glyphFragments = NSCreateRangeToOwnedPointerEntries(2);
        _invalidFragments = NSCreateRangeToOwnedPointerEntries(2);
        _layoutInvalid = YES;
        _defaultAttachmentScaling = NSImageScaleNone;
        _rectCacheCapacity = 16;
        _rectCacheCount = 0;
        _rectCache = NSZoneMalloc(NULL, sizeof(NSRect) * _rectCacheCapacity);
    } else {
        [NSException raise: NSInvalidArgumentException
                    format: @"-[%@ %s] is not implemented for coder %@",
                            [self class], sel_getName(_cmd), coder];
    }
    return self;
}

- init {
    _defaultAttachmentScaling = NSImageScaleNone;
    _typesetter = [NSTypesetter new];
    _glyphGenerator = [[NSGlyphGenerator sharedGlyphGenerator] retain];
    _textContainers = [NSMutableArray new];
    _glyphFragments = NSCreateRangeToOwnedPointerEntries(2);
    _invalidFragments = NSCreateRangeToOwnedPointerEntries(2);
    _layoutInvalid = YES;
    _rectCacheCapacity = 16;
    _rectCacheCount = 0;
    _rectCache = NSZoneMalloc(NULL, sizeof(NSRect) * _rectCacheCapacity);
    return self;
}

- (void) dealloc {
    for (NSTextContainer *container in _textContainers)
        [container _darlingDetachFromLayoutManager: self];
    NSFreeRangeEntries(_rangeToTemporaryAttributes);
    _textStorage = nil;
    [_typesetter release];
    [_glyphGenerator release];
    [_textContainers release];
    [_textFont release];
    NSFreeRangeEntries(_glyphFragments);
    NSFreeRangeEntries(_invalidFragments);
    NSZoneFree(NULL, _rectCache);
    [super dealloc];
}

- (NSTextStorage *) textStorage {
    return [[_textStorage retain] autorelease];
}

- (NSGlyphGenerator *) glyphGenerator {
    return [[_glyphGenerator retain] autorelease];
}

- (NSTypesetter *) typesetter {
    return [[_typesetter retain] autorelease];
}

- delegate {
    return _delegate;
}

- (NSArray *) textContainers {
    return [[_textContainers retain] autorelease];
}

- (NSImageScaling) defaultAttachmentScaling {
    return _defaultAttachmentScaling;
}

- (void) setDefaultAttachmentScaling: (NSImageScaling) scaling {
    _defaultAttachmentScaling = scaling;
}

- (BOOL) usesDefaultHyphenation {
    return _usesDefaultHyphenation;
}

- (void) setUsesDefaultHyphenation: (BOOL) flag {
    _usesDefaultHyphenation = flag;
}

- (NSTextView *) firstTextView {
    for (NSTextContainer *container in _textContainers) {
        NSTextView *textView = [container textView];
        if (textView) {
            return textView;
        }
    }
    return nil;
}

- (NSTextView *) textViewForBeginningOfSelection {
    return [self firstTextView];
}

- (BOOL) layoutManagerOwnsFirstResponderInWindow: (NSWindow *) window {
    NSResponder *first = [window firstResponder];
    NSInteger i, count = [_textContainers count];

    for (i = 0; i < count; i++)
        if ([[_textContainers objectAtIndex: i] textView] == first)
            return YES;

    return NO;
}

- (void) setTextStorage: (NSTextStorage *) textStorage {

    if (textStorage == _textStorage) {
        return;
    }

    // The text storage owns the layout manager - so we can't retain
    _textStorage = textStorage;

    NSFreeRangeEntries(_rangeToTemporaryAttributes);

    _rangeToTemporaryAttributes = NSCreateRangeToCopiedObjectEntries(0);
    NSRangeEntryInsert(_rangeToTemporaryAttributes,
                       NSMakeRange(0, [_textStorage length]),
                       [NSDictionary dictionary]);

    _layoutInvalid = YES;
}

// Moves every layout manager of the current text storage, and their text views, to textStorage.
- (void) replaceTextStorage: (NSTextStorage *) textStorage {
    if (textStorage == nil)
        [NSException raise: NSInvalidArgumentException
                    format: @"-[NSLayoutManager replaceTextStorage:] nil text storage"];
    if (textStorage == _textStorage)
        return;

    NSTextStorage *oldStorage = [_textStorage retain];
    // Nib-loaded text views add their shared layout manager once each, so the old list can repeat it.
    NSMutableArray *layoutManagers = [NSMutableArray array];
    for (NSLayoutManager *layoutManager in [oldStorage layoutManagers]) {
        if ([layoutManagers indexOfObjectIdenticalTo: layoutManager] == NSNotFound)
            [layoutManagers addObject: layoutManager];
    }
    if ([layoutManagers indexOfObjectIdenticalTo: self] == NSNotFound)
        [layoutManagers addObject: self];

    NSUInteger length = [textStorage length];
    for (NSLayoutManager *layoutManager in layoutManagers) {
        [textStorage addLayoutManager: layoutManager];
        [oldStorage removeLayoutManager: layoutManager];
        for (NSTextContainer *container in [layoutManager textContainers]) {
            NSTextView *textView = [container textView];
            [textView _setTextStorage: textStorage];
            NSRange selection = [textView selectedRange];
            if (textView && NSMaxRange(selection) > length)
                [textView setSelectedRange: NSMakeRange(MIN(selection.location, length), 0)];
        }
        [layoutManager invalidateDisplayForCharacterRange: NSMakeRange(0, length)];
    }
    [oldStorage release];
}

- (void) setGlyphGenerator: (NSGlyphGenerator *) generator {
    generator = [generator retain];
    [_glyphGenerator release];
    _glyphGenerator = generator;
}

- (void) setTypesetter: (NSTypesetter *) typesetter {
    typesetter = [typesetter retain];
    [_typesetter release];
    _typesetter = typesetter;
}

- (void) setDelegate: delegate {
    _delegate = delegate;
}

- (BOOL) usesScreenFonts {
    return !_doesNotUseScreenFonts;
}

- (void) setUsesScreenFonts: (BOOL) yorn {
    if (yorn == [self usesScreenFonts])
        return;
    _doesNotUseScreenFonts = !yorn;

    // The glyph advances, and with them the line fragments, depend on the fonts used for layout.
    NSRange all = NSMakeRange(0, [_textStorage length]);
    if (all.length > 0) {
        [self invalidateLayoutForCharacterRange: all isSoft: NO actualCharacterRange: NULL];
        [self invalidateDisplayForCharacterRange: all];
    }
}

// The font the layout, the glyph rects and the drawing use for a font of the
// text. With usesScreenFonts, that is the font's screen variant, which lays
// out with the advances macOS uses on screen (whole pixels up to 16 pt, exact
// above); otherwise the font itself, with exact advances.
- (NSFont *) _layoutFontForFont: (NSFont *) font {
    if (font == nil || _doesNotUseScreenFonts)
        return font;
    NSFont *screenFont = [font screenFontWithRenderingMode: NSFontDefaultRenderingMode];
    return screenFont != nil ? screenFont : font;
}

- (BOOL) showsControlCharacters {
    return _showsControlCharacters;
}

- (void) setShowsControlCharacters: (BOOL) flag {
    _showsControlCharacters = flag;
}

- (BOOL) showsInvisibleCharacters {
    return _showsInvisibleCharacters;
}

- (void) setShowsInvisibleCharacters: (BOOL) flag {
    _showsInvisibleCharacters = flag;
}

- (BOOL) usesFontLeading {
    return _usesFontLeading;
}

- (void) setUsesFontLeading: (BOOL) flag {
    _usesFontLeading = flag;
}

- (BOOL) backgroundLayoutEnabled {
    return _backgroundLayoutEnabled;
}

- (void) setBackgroundLayoutEnabled: (BOOL) flag {
    _backgroundLayoutEnabled = flag;
}

- (NSFont *) textFont {
    return _textFont;
}

- (void) setTextFont: (NSFont *) font {
    [_textFont autorelease];
    _textFont = [font retain];
}

- (void) addTextContainer: (NSTextContainer *) container {
    [_textContainers addObject: container];
    [container setLayoutManager: self];
}

- (void) removeTextContainerAtIndex: (NSUInteger) index {
    [[_textContainers objectAtIndex: index] _darlingDetachFromLayoutManager: self];
    [_textContainers removeObjectAtIndex: index];
}

- (void) insertTextContainer: (NSTextContainer *) container
                     atIndex: (NSUInteger) index
{
    [_textContainers insertObject: container atIndex: index];
    [container setLayoutManager: self];
}

- (void) insertGlyph: (NSGlyph) glyph
          atGlyphIndex: (NSUInteger) glyphIndex
        characterIndex: (NSUInteger) characterIndex
{
}

- (void) replaceGlyphAtIndex: (NSUInteger) glyphIndex
                   withGlyph: (NSGlyph) glyph
{
}

- (void) deleteGlyphsInRange: (NSRange) glyphRange {
}

- (void) setCharacterIndex: (NSUInteger) characterIndex
           forGlyphAtIndex: (NSUInteger) glyphIndex
{
}

- (void) setNotShownAttribute: (BOOL) notShown
              forGlyphAtIndex: (NSUInteger) glyphIndex
{
}

- (void) setAttachmentSize: (NSSize) size forGlyphRange: (NSRange) glyphRange {
}

- (void) setDrawsOutsideLineFragment: (BOOL) drawsOutside
                     forGlyphAtIndex: (NSUInteger) glyphIndex
{
}

- (NSUInteger) numberOfGlyphs {
    return [_textStorage length];
}

- (void) _rollbackLatestFragment {
    NSRangeEntriesRemoveEntryAtIndex(_glyphFragments,
                                     NSCountRangeEntries(_glyphFragments) - 1);
}

- (NSFont *) _fontForGlyphRange: (NSRange) glyphRange {
    NSRange characterRange = [self characterRangeForGlyphRange: glyphRange
                                              actualGlyphRange: NULL];
    NSDictionary *attributes =
            [_textStorage attributesAtIndex: characterRange.location
                             effectiveRange: NULL];

    return [self _layoutFontForFont: NSFontAttributeInDictionary(attributes)];
}

#define DEBUG_LM_LAYOUT 0

- (NSUInteger) getGlyphs: (NSGlyph *) glyphs range: (NSRange) glyphRange {

#if DEBUG_LM_LAYOUT
    NSLog(@"getGlyphs: %p range: %@", glyphs, NSStringFromRange(glyphRange));
#define DEBUG_getGlyphs_range 0
#endif

    NSRange characterRange = [self characterRangeForGlyphRange: glyphRange
                                              actualGlyphRange: NULL];
    NSFont *font = [self _fontForGlyphRange: glyphRange];
    unichar buffer[characterRange.length];

    [[_textStorage string] getCharacters: buffer range: characterRange];
    [font getGlyphs: glyphs
            forCharacters: buffer
                   length: characterRange.length];

#if DEBUG_getGlyphs_range
    NSLog(@"returning %u", glyphRange.length);
#endif

    return glyphRange.length;
}

- (NSUInteger) getOrderedGlyphs: (NSGlyph *) glyphs
                          range: (NSRange) glyphRange
                      baseLevel: (uint8_t) baseLevel
                          order: (NSUInteger *) order
{

#if DEBUG_LM_LAYOUT
    NSLog(@"getOrderedGlyphs: %p range: %@ baseLevel: %u order: %p", glyphs,
          NSStringFromRange(glyphRange), baseLevel, order);
#define DEBUG_getOrderedGlyphs_range_baseLevel_order 0
#endif

    uint8_t bidiLevels[glyphRange.length];
    NSUInteger result = [self getGlyphsInRange: glyphRange
                                        glyphs: NULL
                              characterIndexes: NULL
                             glyphInscriptions: NULL
                                   elasticBits: NULL
                                    bidiLevels: bidiLevels];
    BOOL needsOrdering = baseLevel & 1;

    for (NSInteger i = 0; i < glyphRange.length && !needsOrdering; i++) {
        if (bidiLevels[i] & 1) {
            needsOrdering = YES;
            break;
        }
    }
    if (order) {
        for (NSInteger i = 0; i < glyphRange.length; i++) {
            order[i] = i;
        }
    }
    if (needsOrdering) {
        // Reorder the chars & do mirroring, and get the glyphs from that result

        // Get the char for the range
        NSRange characterRange = [self characterRangeForGlyphRange: glyphRange
                                                  actualGlyphRange: NULL];
        unichar buffer[characterRange.length];
        [[_textStorage string] getCharacters: buffer range: characterRange];

        // Process them : that will reorder "order", and do mirroring if needed
        NSBidiHelperProcessLine(baseLevel, order, buffer, bidiLevels, true,
                                characterRange.length);

        NSFont *font = [self _fontForGlyphRange: glyphRange];
        result = [font getGlyphs: glyphs
                   forCharacters: buffer
                          length: characterRange.length];
        // Reorder the glyph
        NSBidiHelperProcessLine(baseLevel, glyphs, NULL, bidiLevels, false,
                                characterRange.length);
    } else {
        result = [self getGlyphs: glyphs range: glyphRange];
    }
    return result;
}

- (NSUInteger) getGlyphsInRange: (NSRange) range
                         glyphs: (NSGlyph *) glyphs
               characterIndexes: (NSUInteger *) charIndexes
              glyphInscriptions: (NSGlyphInscription *) inscriptions
                    elasticBits: (BOOL *) elasticBits
{

#if DEBUG_LM_LAYOUT
    NSLog(@"getGlyphsInRange: %@ glyphs: p characterIndexes: %p "
          @"glyphInscriptions: %p elasticBits: %p",
          NSStringFromRange(range), glyphs, charIndexes, inscriptions,
          elasticBits);
#endif

    return [self getGlyphsInRange: range
                           glyphs: glyphs
                 characterIndexes: charIndexes
                glyphInscriptions: inscriptions
                      elasticBits: elasticBits
                       bidiLevels: NULL];
}

- (NSUInteger) getGlyphsInRange: (NSRange) range
                         glyphs: (NSGlyph *) glyphs
               characterIndexes: (NSUInteger *) charIndexes
              glyphInscriptions: (NSGlyphInscription *) inscriptions
                    elasticBits: (BOOL *) elasticBits
                     bidiLevels: (unsigned char *) bidiLevels
{

#if DEBUG_LM_LAYOUT
    NSLog(@"getGlyphsInRange: %@ glyphs: p characterIndexes: %p "
          @"glyphInscriptions: %p elasticBits: %p bidiLevels: %p",
          NSStringFromRange(range), glyphs, charIndexes, inscriptions,
          elasticBits, bidiLevels);
#endif

    NSUInteger result = 0;
    if (glyphs) {
        result = [self getGlyphs: glyphs range: range];
    }
    // Actually ask to the typesetter for anything else - it's the one knowing
    // all the dirty details
    [_typesetter getGlyphsInRange: range
                           glyphs: NULL
                 characterIndexes: charIndexes
                glyphInscriptions: inscriptions
                      elasticBits: elasticBits
                       bidiLevels: bidiLevels];
    return result;
}

- (NSTextContainer *) textContainerForGlyphAtIndex: (NSUInteger) glyphIndex
                                    effectiveRange:
                                            (NSRangePointer) effectiveGlyphRange
{

#if DEBUG_LM_LAYOUT
    NSLog(@"textContainerForGlyphAtIndex: %u effectiveRange: %p", glyphIndex,
          effectiveGlyphRange);
#define DEBUG_textContainerForGlyphAtIndex_effectiveRange 0
#endif

    [self validateGlyphsAndLayoutForGlyphRange: NSMakeRange(glyphIndex, 1)];
    NSGlyphFragment *fragment =
            fragmentAtGlyphIndex(self, glyphIndex, effectiveGlyphRange);
    if (fragment == NULL) {

#if DEBUG_textContainerForGlyphAtIndex_effectiveRange
        NSLog(@"no fragment found - bailing...");
#endif

        return nil;
    }

    if (effectiveGlyphRange) {
        *effectiveGlyphRange =
                [self _currentGlyphRangeForTextContainer: fragment->container];
    }

#if DEBUG_textContainerForGlyphAtIndex_effectiveRange
    NSLog(@"found container: %@", fragment->container);
#endif

    return fragment->container;
}

- (NSRect) lineFragmentRectForGlyphAtIndex: (NSUInteger) glyphIndex
                            effectiveRange: (NSRangePointer) effectiveGlyphRange
{

#if DEBUG_LM_LAYOUT
    NSLog(@"lineFragmentRectForGlyphAtIndex: %u effectiveRange: %p", glyphIndex,
          effectiveGlyphRange);
#define DEBUG_lineFragmentRectForGlyphAtIndex_effectiveRange 0
#endif

    NSGlyphFragment *fragment =
            fragmentAtGlyphIndex(self, glyphIndex, effectiveGlyphRange);

    if (fragment == NULL) {

#if DEBUG_lineFragmentRectForGlyphAtIndex_effectiveRange
        NSLog(@"fragment not found returning zero rect");
#endif

        return NSZeroRect;
    }

#if DEBUG_lineFragmentRectForGlyphAtIndex_effectiveRange
    NSLog(@"returning rect: %@", NSStringFromRect(fragment->rect));
#endif

    return fragment->rect;
}

static CGFloat _glyphRunWidth(NSFont *font, const NSGlyph *glyphs, NSInteger length);

- (NSPoint) locationForGlyphAtIndex: (NSUInteger) glyphIndex {

#if DEBUG_LM_LAYOUT
    NSLog(@"locationForGlyphAtIndex: %u", glyphIndex);
#define DEBUG_locationForGlyphAtIndex 0
#endif

    NSRange effectiveRange;
    NSGlyphFragment *fragment = fragmentAtGlyphIndex(self, glyphIndex, &effectiveRange);

    if (fragment == NULL) {

#if DEBUG_locationForGlyphAtIndex
        NSLog(@"fragment not found - bailing...");
#endif

        return NSZeroPoint;
    }

#if DEBUG_locationForGlyphAtIndex
    NSLog(@"returning location: %@", NSStringFromPoint(fragment->location));
#endif

    NSPoint location = fragment->location;

    // The location of a glyph is the start of its nominally spaced run plus the advances of the glyphs before it.
    if (fragment->leftToRight && glyphIndex > effectiveRange.location) {
        NSRange through = NSMakeRange(effectiveRange.location, glyphIndex + 1 - effectiveRange.location);
        NSGlyph glyphs[through.length];
        NSUInteger count = [self getGlyphs: glyphs range: through];
        if (count > 0 && glyphs[count - 1] != NSControlGlyph) {
            NSFont *font = [self _fontForGlyphRange: through];
            BOOL ignore;
            // The width through the glyph itself, less the glyph's own advance: what precedes it, kerning included.
            location.x += _glyphRunWidth(font, glyphs, count) -
                          [font positionOfGlyph: glyphs[count - 1] precededByGlyph: NSNullGlyph isNominal: &ignore].x;
        }
    }

    return location;
}

- (NSRect) lineFragmentUsedRectForGlyphAtIndex: (NSUInteger) glyphIndex
                                effectiveRange:
                                        (NSRangePointer) effectiveGlyphRange
{

#if DEBUG_LM_LAYOUT
    NSLog(@"lineFragmentUsedRectForGlyphAtIndex: %u effectiveRange: %p",
          glyphIndex, effectiveGlyphRange);
#define DEBUG_lineFragmentUsedRectForGlyphAtIndex_effectiveRange 0
#endif

    NSGlyphFragment *fragment =
            fragmentAtGlyphIndex(self, glyphIndex, effectiveGlyphRange);

    if (fragment == NULL) {

#if DEBUG_lineFragmentUsedRectForGlyphAtIndex_effectiveRange
        NSLog(@"fragment not found - bailing...");
#endif

        return NSZeroRect;
    }

#if DEBUG_lineFragmentUsedRectForGlyphAtIndex_effectiveRange
    NSLog(@"returning rect: %@", NSStringFromRect(fragment->usedRect));
#endif

    return fragment->usedRect;
}

- (void) enumerateLineFragmentsForGlyphRange: (NSRange) glyphRange
                                  usingBlock: (void (^)(NSRect lineRect, NSRect usedRect,
                                                        NSTextContainer *textContainer,
                                                        NSRange lineGlyphRange,
                                                        BOOL *stop)) block
{
    [self validateGlyphsAndLayoutForGlyphRange: glyphRange];

    // Glyph fragments are runs sorted by glyph index; consecutive runs sharing a
    // line fragment rect form one line.
    NSRangeEnumerator state = NSRangeEntryEnumerator(_glyphFragments);
    NSRange range, lineRange = NSMakeRange(0, 0);
    NSGlyphFragment *fragment, *line = NULL;
    NSRect usedRect = NSZeroRect;
    BOOL more, stop = NO;

    do {
        more = NSNextRangeEnumeratorEntry(&state, &range, (void **) &fragment);
        if (line != NULL && (!more || !NSEqualRects(fragment->rect, line->rect) ||
                             fragment->container != line->container))
        {
            if (NSIntersectionRange(lineRange, glyphRange).length > 0)
                block(line->rect, usedRect, line->container, lineRange, &stop);
            if (NSMaxRange(lineRange) >= NSMaxRange(glyphRange))
                break;
            line = NULL;
        }
        if (more && line == NULL) {
            line = fragment;
            lineRange = range;
            usedRect = fragment->usedRect;
        } else if (more) {
            lineRange = NSUnionRange(lineRange, range);
            usedRect = NSUnionRect(usedRect, fragment->usedRect);
        }
    } while (more && !stop);
}

- (NSRange) validateGlyphsAndLayoutForGlyphRange: (NSRange) glyphRange {

#if DEBUG_LM_LAYOUT
    NSLog(@"validateGlyphsAndLayoutForGlyphRange: %@",
          NSStringFromRange(glyphRange));
#define DEBUG_validateGlyphsAndLayoutForGlyphRange 0
#endif

    // TODO: Validate glyphs in glyph cache for glyph range

    if (_layoutInvalid) {

#if DEBUG_validateGlyphsAndLayoutForGlyphRange
        NSLog(@"layout is invalid");
#endif

        NSResetRangeEntries(_glyphFragments);
        [_typesetter layoutGlyphsInLayoutManager: self
                            startingAtGlyphIndex: 0
                        maxNumberOfLineFragments: 0
                                  nextGlyphIndex: NULL];
        _layoutInvalid = NO;

        if ([_delegate respondsToSelector: @selector
                       (layoutManager:
                               didCompleteLayoutForTextContainer:atEnd:)]) {

            NSTextContainer *container = [_textContainers lastObject];
            NSRange containerRange =
                    [self _currentGlyphRangeForTextContainer: container];
            BOOL finished =
                    NSMaxRange(containerRange) >= NSMaxRange(glyphRange);
            [_delegate layoutManager: self
                    didCompleteLayoutForTextContainer: container
                                                atEnd: finished];

#if DEBUG_validateGlyphsAndLayoutForGlyphRange
            NSLog(@"informed delegate with layoutManager: %p "
                  @"didCompleteLayoutForTextContainer: %@ atEnd: %@",
                  self, container, finished ? @"YES" : @"NO");
#endif
        }
    } else {

#if DEBUG_validateGlyphsAndLayoutForGlyphRange
        NSLog(@"layout is still valid - glyphRange unchanged");
#endif
    }

    return glyphRange;

#if 0
   NSUInteger glyphIndex=[self firstUnlaidGlyphIndex];

   while(glyphIndex<NSMaxRange(glyphRange)){
    [_typesetter layoutGlyphsInLayoutManager:self
      startingAtGlyphIndex:glyphIndex maxNumberOfLineFragments:2
            nextGlyphIndex:&glyphIndex];
   }
#endif
}

- (void) validateGlyphsAndLayoutForContainer: (NSTextContainer *) container {
#if DEBUG_LM_LAYOUT
    NSLog(@"validateGlyphsAndLayoutForContainer: %@", container);
#endif
    // Validate everything - we should at least validate everything up to this
    // container
    [self validateGlyphsAndLayoutForGlyphRange: NSMakeRange(
                                                        0,
                                                        [self numberOfGlyphs])];
}

- (NSRect) usedRectForTextContainer: (NSTextContainer *) container {

#if DEBUG_LM_LAYOUT
    NSLog(@"usedRectForTextContainer: %@", container);
#define DEBUG_usedRectForTextContainer 0
#endif

    [self validateGlyphsAndLayoutForContainer: container]; {

#if DEBUG_usedRectForTextContainer
        NSLog(@"done validating, now calcing used rect");
#endif

        NSRect result = NSZeroRect;
        BOOL assignFirst = YES;
        NSRangeEnumerator state = NSRangeEntryEnumerator(_glyphFragments);
        NSRange range;
        NSGlyphFragment *fragment;

        while (NSNextRangeEnumeratorEntry(&state, &range,
                                          (void **) &fragment)) {
            if (fragment->container == container) {
                NSRect rect = fragment->usedRect;
#if DEBUG_usedRectForTextContainer
                NSLog(@"fragmentRange: %@ has width: %f",
                      NSStringFromRange(range), rect.size.width);
#endif
                if (assignFirst) {
                    result = rect;
                    assignFirst = NO;
                } else {
                    result = NSUnionRect(result, rect);
                }
            }
        }

        if (assignFirst) {

#if DEBUG_usedRectForTextContainer
            NSLog(@"no fragments so checking for _extraLineFragment...");
#endif

            // if empty, use the extra rect
            if (container == _extraLineFragmentTextContainer) {
                NSRect extra = _extraLineFragmentUsedRect;
                // The used rect of empty text is as wide as the padding on
                // both sides of the line, so nothing at all without padding.
                extra.origin.x = 0;
                extra.size.width = 2 * [container lineFragmentPadding];

                result = extra;

#if DEBUG_usedRectForTextContainer
                NSLog(@"result: %@", NSStringFromRect(result));
#endif

            } else {

#if DEBUG_usedRectForTextContainer
                NSLog(@"container is no the _extraLineFragmentTextContainer");
#endif
            }
        }

#if DEBUG_usedRectForTextContainer
        NSLog(@"result: %@", NSStringFromRect(result));
        NSLog(@"result.size.width: %f", result.size.width);
#endif

        return result;
    }
}

- (NSRect) extraLineFragmentRect {
    return _extraLineFragmentRect;
}

- (NSRect) extraLineFragmentUsedRect {
    return _extraLineFragmentUsedRect;
}

- (NSTextContainer *) extraLineFragmentTextContainer {
    return _extraLineFragmentTextContainer;
}

- (BOOL) allowsNonContiguousLayout {
    return _allowsNonContiguousLayout;
}

- (void) setTextContainer: (NSTextContainer *) container
            forGlyphRange: (NSRange) glyphRange
{

#if DEBUG_LM_LAYOUT
    NSLog(@"setTextContainer: %@ forGlyphRange: %@", container,
          NSStringFromRange(glyphRange));
#define DEBUG_setTextContainer_forGlyphRange 0
#endif

    NSGlyphFragment *insert = NSZoneMalloc(NULL, sizeof(NSGlyphFragment));

    insert->rect = NSZeroRect;
    insert->usedRect = NSZeroRect;
    insert->location = NSZeroPoint;
    insert->container = container;
    // Get the direction for the fragment
    uint8_t bidiLevel;
    [_typesetter getGlyphsInRange: NSMakeRange(glyphRange.location, 1)
                           glyphs: NULL
                 characterIndexes: NULL
                glyphInscriptions: NULL
                      elasticBits: NULL
                       bidiLevels: &bidiLevel];

#if DEBUG_setTextContainer_forGlyphRange
    NSLog(@"text direction: %@"(bidiLevel & 1) ? @"Right to left"
                                               : @"Left to right");
#endif

    insert->leftToRight = (bidiLevel & 1) == 0;
    NSRangeEntryInsert(_glyphFragments, glyphRange, insert);
}

- (void) setLineFragmentRect: (NSRect) rect
               forGlyphRange: (NSRange) range
                    usedRect: (NSRect) usedRect
{

#if DEBUG_LM_LAYOUT
    NSLog(@"setLineFragmentRect: %@ forGlyphRange: %@ usedRect: %@",
          NSStringFromRect(rect), NSStringFromRange(range),
          NSStringFromRect(usedRect));
#define DEBUG_setLineFragmentRect_forGlyphRange_usedRect 0
#endif

    NSGlyphFragment *fragment = fragmentForGlyphRange(self, range);

    if (fragment == NULL) {
#if DEBUG_setLineFragmentRect_forGlyphRange_usedRect
        NSLog(@"fragment not found - bailing...");
#endif
        return;
    }

    fragment->rect = rect;
    fragment->usedRect = usedRect;
}

- (void) setLocation: (NSPoint) location forStartOfGlyphRange: (NSRange) range {

#if DEBUG_LM_LAYOUT
    NSLog(@"setLocation: %@ forStartOfGlyphRange: %@",
          NSStringFromPoint(location), NSStringFromRange(range));
#define DEBUG_setLocation_forStartOfGlyphRange 0
#endif

    NSGlyphFragment *fragment = fragmentForGlyphRange(self, range);

    if (fragment == NULL) {
#if DEBUG_setLocation_forStartOfGlyphRange
        NSLog(@"fragment not found - bailing...");
#endif
        return;
    }

    fragment->location = location;
}

- (void) setExtraLineFragmentRect: (NSRect) fragmentRect
                         usedRect: (NSRect) usedRect
                    textContainer: (NSTextContainer *) container
{

#if DEBUG_LM_LAYOUT
    NSLog(@"setExtraLineFragmentRect: %@ usedRect: %@ textContainer: %@",
          NSStringFromRect(fragmentRect), NSStringFromRect(usedRect),
          container);
#endif

    _extraLineFragmentRect = fragmentRect;
    _extraLineFragmentUsedRect = usedRect;
    _extraLineFragmentTextContainer = container;
}

- (void) invalidateGlyphsForCharacterRange: (NSRange) charRange
                            changeInLength: (NSInteger) delta
                      actualCharacterRange: (NSRangePointer) actualRange
{

#if DEBUG_LM_LAYOUT
    NSLog(@"invalidateGlyphsForCharacterRange: %@ changeInLength: %d "
          @"actualCharacterRange: %p",
          NSStringFromRange(charRange), delta, actualRange);
#endif

    if (actualRange != NULL)
        *actualRange = charRange;
}

// The range whose layout goes when charRange's does: whole paragraphs, as a
// change can move every line of the paragraphs it touches. An empty range
// stays as it is.
static NSRange paragraphsForInvalidation(NSTextStorage *storage, NSRange charRange)
{
    NSUInteger length = [storage length];
    if (charRange.length == 0 || charRange.location >= length)
        return charRange;
    if (NSMaxRange(charRange) > length)
        charRange.length = length - charRange.location;
    return [[storage string] paragraphRangeForRange: charRange];
}

- (void) invalidateLayoutForCharacterRange: (NSRange) charRange
                      actualCharacterRange: (NSRangePointer) actualRange
{
    [self invalidateLayoutForCharacterRange: charRange
                                     isSoft: NO
                       actualCharacterRange: actualRange];
}

- (void) invalidateLayoutForCharacterRange: (NSRange) charRange
                                    isSoft: (BOOL) isSoft
                      actualCharacterRange: (NSRangePointer) actualRangep
{
    if (actualRangep != NULL)
        *actualRangep = paragraphsForInvalidation(_textStorage, charRange);

#if DEBUG_LM_LAYOUT
    NSLog(@"invalidateLayoutForCharacterRange: %@ isSoft: %@ "
          @"actualCharacterRange: %p",
          NSStringFromRange(charRange), isSoft ? @"YES" : @"NO", actualRangep);
#endif

#if 0
   NSUInteger location=charRange.location;
   NSUInteger limit=NSMaxRange(charRange);
   NSRange  actualRange=NSMakeRange(NSNotFound,NSNotFound);

   while(location<limit){
    NSRange            effectiveRange;
    NSGlyphFragment   *fragment=fragmentAtGlyphIndex(self,location,&effectiveRange);

    if(fragment!=NULL){
     NSInvalidFragment *invalid=NSZoneMalloc(NULL,sizeof(NSInvalidFragment));

     if(actualRange.location==NSNotFound)
      actualRange=effectiveRange;
     else
      actualRange=NSUnionRange(actualRange,effectiveRange);

     NSRangeEntryInsert(_invalidFragments,effectiveRange,invalid);
    }
   }
#endif
    // We could probably just invalidate paragraphs from the ones covering the
    // charRange to the end of the dc
    _layoutInvalid = YES;
}

- (void) invalidateDisplayForGlyphRange: (NSRange) glyphRange {

#if DEBUG_LM_LAYOUT
    NSLog(@"invalidateDisplayForGlyphRange: %@", NSStringFromRange(glyphRange));
#endif

    NSRange characterRange = [self characterRangeForGlyphRange: glyphRange
                                              actualGlyphRange: NULL];

    [self invalidateDisplayForCharacterRange: characterRange];
}

- (void) invalidateDisplayForCharacterRange: (NSRange) charRange {

#if DEBUG_LM_LAYOUT
    NSLog(@"invalidateDisplayForCharacterRange: %@",
          NSStringFromRange(charRange));
#define DEBUG_invalidateDisplayForCharacterRange 0
#endif

    NSInteger i, count = [_textContainers count];

    //   charRange=[self validateGlyphsAndLayoutForGlyphRange:charRange];

    for (i = 0; i < count; i++) {
        NSTextContainer *container = [_textContainers objectAtIndex: i];
        NSTextView *textView = [container textView];

        [textView sizeToFit];
        [textView setNeedsDisplay: YES];

#if DEBUG_invalidateDisplayForCharacterRange
        NSLog(@"told textView: %@ to redisplay", textView);
#endif
    }
    // FIX
}

// must be a more official way to do this
- (void) fixupSelectionInRange: (NSRange) range
                changeInLength: (NSInteger) changeInLength
{

#if DEBUG_LM_LAYOUT
    NSLog(@"fixupSelectionInRange: %@ changeInLength: %d",
          NSStringFromRange(range), changeInLength);
#define DEBUG_fixupSelectionInRange_changeInLength 0
#endif

    NSInteger i, count = [_textContainers count];
    NSUInteger textLength = [_textStorage length];

    for (i = 0; i < count; i++) {
        NSTextContainer *container = [_textContainers objectAtIndex: i];
        NSTextView *textView = [container textView];
        if (textView) {
            NSRange selectedRange = [textView selectedRange];
            if (selectedRange.location == NSNotFound)
                continue;
            NSRange fixedRange = selectedRange;
            if (fixedRange.location > textLength) {
                fixedRange.location = textLength;
                fixedRange.length = 0;
            } else if (fixedRange.length > textLength - fixedRange.location) {
                fixedRange.length = textLength - fixedRange.location;
            }
            if (!NSEqualRanges(selectedRange, fixedRange)) {
                [textView setSelectedRange: fixedRange];
            }
        }
    }
}

- (void) textStorage: (NSTextStorage *) storage
                  edited: (NSUInteger) editedMask
                   range: (NSRange) range
          changeInLength: (NSInteger) changeInLength
        invalidatedRange: (NSRange) invalidateRange
{

#if DEBUG_LM_LAYOUT
    NSLog(@"textStorage: %@ edited: %u range: %@ changeInLength: %d "
          @"invalidatedRange: %@",
          storage, editedMask, NSStringFromRange(range), changeInLength,
          NSStringFromRange(invalidateRange));
#define DEBUG_textStorage_edited_range_changeInLength_invalidatedRange 0
#endif

    NSRange actualRange;

    [self invalidateGlyphsForCharacterRange: invalidateRange
                             changeInLength: changeInLength
                       actualCharacterRange: &actualRange];

    [self invalidateLayoutForCharacterRange: actualRange
                                     isSoft: NO
                       actualCharacterRange: &actualRange];

    [self invalidateDisplayForCharacterRange: actualRange];

    [self fixupSelectionInRange: range changeInLength: changeInLength];

    // We also have to fix the temporary attributes on that range
    if (editedMask & NSTextStorageEditedCharacters) {
#if DEBUG_textStorage_edited_range_changeInLength_invalidatedRange
        NSLog(@"fixing temporary attributes on that range...");
#endif
        // Update the temporary attributes ranges according to the changes
        NSRange oldRange = range;
        oldRange.length -= changeInLength;
        NSRangeEntriesExpandAndWipe(_rangeToTemporaryAttributes, oldRange,
                                    changeInLength);
        // And clear the attributes for the new part
        if (range.length) {
#if DEBUG_textStorage_edited_range_changeInLength_invalidatedRange
            NSLog(@"clear the attributes for the new part...");
#endif
            [self setTemporaryAttributes: nil forCharacterRange: range];
        }
        NSRangeEntriesVerify(_rangeToTemporaryAttributes,
                             [_textStorage length]);
    }
}

- (void) textContainerChangedGeometry: (NSTextContainer *) container {

#if DEBUG_LM_LAYOUT
    NSLog(@"textContainerChangedGeometry: %@", container);
#endif

    NSRange range = NSMakeRange(0, [_textStorage length]);

    [self invalidateLayoutForCharacterRange: range
                                     isSoft: NO
                       actualCharacterRange: NULL];
}

- (void) ensureLayoutForTextContainer: (NSTextContainer *) container {

#if DEBUG_LM_LAYOUT
    NSLog(@"ensureLayoutForTextContainer: %@", container);
#endif

    [self validateGlyphsAndLayoutForContainer: container];
}

- (void) ensureLayoutForGlyphRange: (NSRange) glyphRange {
    [self validateGlyphsAndLayoutForGlyphRange: glyphRange];
}

- (void) ensureLayoutForCharacterRange: (NSRange) charRange {
    NSRange glyphRange = [self glyphRangeForCharacterRange: charRange actualCharacterRange: NULL];
    [self validateGlyphsAndLayoutForGlyphRange: glyphRange];
}

- (NSUInteger) glyphIndexForPoint: (NSPoint) point
                       inTextContainer: (NSTextContainer *) container
        fractionOfDistanceThroughGlyph: (CGFloat *) fraction
{

#if DEBUG_LM_LAYOUT
    NSLog(@"glyphIndexForPoint: %@ inTextContainer: %@ "
          @"fractionOfDistanceThroughGlyph: %p",
          NSStringFromPoint(point), container, fraction);
#define DEBUG_glyphIndexForPoint_inTextContainer_fractionOfDistanceThroughGlyph \
    0
#endif

    NSUInteger endOfFragment = 0;
    NSUInteger result =
            NSMaxRange([self glyphRangeForTextContainer: container]);
    NSRange range;
    NSGlyphFragment *fragment;
    NSRangeEnumerator state;
    // Fragments are in glyph order, but bidi lines are reordered visually:
    // the point is left of its line only if it is left of every fragment.
    BOOL leftOfLine = YES;
    CGFloat lineMinX = CGFLOAT_MAX;
    NSUInteger lineStart = NSNotFound;

    [self validateGlyphsAndLayoutForContainer: container];

    *fraction = 0;

    state = NSRangeEntryEnumerator(_glyphFragments);

    while (NSNextRangeEnumeratorEntry(&state, &range, (void **) &fragment)) {
        if (point.y < NSMinY(fragment->rect)) {
            // Otherwise a click in the line's left padding would return
            // the end of the line.
            if (leftOfLine && lineStart != NSNotFound) {
                return lineStart;
            }
            if (endOfFragment > 0) {
                // if we're at the end of a line we want to back up before the
                // newline This is a very ugly way to do it
                if ([[_textStorage string]
                            characterAtIndex: endOfFragment - 1] == '\n') {
#if DEBUG_glyphIndexForPoint_inTextContainer_fractionOfDistanceThroughGlyph
                    NSLog(@"backing up before the newline");
#endif
                    endOfFragment--;
                }
            }
#if DEBUG_glyphIndexForPoint_inTextContainer_fractionOfDistanceThroughGlyph
            NSLog(@"early returning: %u", endOfFragment);
#endif

            return endOfFragment;
        }

        if (point.y < NSMaxY(fragment->rect) &&
            point.y >= NSMinY(fragment->rect)) {
            // The fragment rect and the used rect also cover the line
            // fragment padding, the glyphs start at the fragment location
            if (point.x >= fragment->location.x &&
                point.x < NSMaxX(fragment->usedRect)) {

#if DEBUG_glyphIndexForPoint_inTextContainer_fractionOfDistanceThroughGlyph
                NSLog(@"point: %@ is inside the fragment",
                      NSStringFromPoint(point));
#endif

                NSRect glyphRect = fragment->usedRect;
                glyphRect.origin.x = fragment->location.x;
                NSGlyph glyphs[range.length];
                NSUInteger order[range.length];
                NSFont *font = [self _fontForGlyphRange: range];
                NSUInteger i, length = [self getOrderedGlyphs: glyphs
                                                        range: range
                                                    baseLevel: 0
                                                        order: order];
                BOOL ltor = fragment->leftToRight;

                glyphRect.size.width = 0;
                NSGlyph previousGlyph = NSNullGlyph;

                for (i = 0; i < length; i++) {
                    NSGlyph glyph = glyphs[i];

                    if (glyph != NSControlGlyph) {
                        // The same advance, kerning included, as the typesetter laid the glyph out with.
                        BOOL ignore;
                        NSSize advancement = NSMakeSize(
                                [font positionOfGlyph: glyph precededByGlyph: previousGlyph isNominal: &ignore].x, 0);
                        previousGlyph = glyph;

                        glyphRect.size.width = advancement.width;

                        if (point.x >= NSMinX(glyphRect) &&
                            point.x <= NSMaxX(glyphRect)) {
                            *fraction = (point.x - glyphRect.origin.x) /
                                        glyphRect.size.width;
                            if (ltor == NO) {
                                // Right-to-left is flipped horizontally
                                *fraction = 1. - *fraction;
                            }
#if DEBUG_glyphIndexForPoint_inTextContainer_fractionOfDistanceThroughGlyph
                            NSLog(@"found the glyph - returning: %u",
                                  range.location + order[i]);
#endif
                            return range.location + order[i];
                        }

                        glyphRect.origin.x += advancement.width;
                        glyphRect.size.width = 0;
                    }
                }
            }
            // We're on a line with fragments, but no inside a segment - that's
            // still a candidate

            // For the cases we hit a line at the left or right of all fragments
            if (point.x < fragment->location.x) {
                if (fragment->leftToRight) {
                    result = range.location;
                } else {
                    result = NSMaxRange(range);
                }
                if (fragment->location.x < lineMinX) {
                    lineMinX = fragment->location.x;
                    lineStart = result;
                }
            } else {
                leftOfLine = NO;
                if (point.x > NSMaxX(fragment->rect)) {
                    if (fragment->leftToRight) {
                        result = NSMaxRange(range);
                    } else {
                        result = range.location;
                    }
                }
            }
            endOfFragment = NSMaxRange(range);
        }
    }

#if DEBUG_glyphIndexForPoint_inTextContainer_fractionOfDistanceThroughGlyph
    NSLog(@"returning: %u", result);
#endif

    if (leftOfLine && lineStart != NSNotFound) {
        return lineStart;
    }
    return result;
}

/* Apple's documentation claims
 * glyphIndexForPoint:inTextContainer:fractionOfDistanceThroughGlyph: is
 * implemented using these two methods. Verify. The method was split in two for
 * the sake of Java, inefficient to keep it split */
- (NSUInteger) glyphIndexForPoint: (NSPoint) point
                  inTextContainer: (NSTextContainer *) container
{

#if DEBUG_LM_LAYOUT
    NSLog(@"glyphIndexForPoint: %@ inTextContainer: %@",
          NSStringFromPoint(point), container);
#endif

    CGFloat fraction;

    return [self glyphIndexForPoint: point
                           inTextContainer: container
            fractionOfDistanceThroughGlyph: &fraction];
}

- (CGFloat) fractionOfDistanceThroughGlyphForPoint: (NSPoint) point
                                   inTextContainer:
                                           (NSTextContainer *) container
{

#if DEBUG_LM_LAYOUT
    NSLog(@"fractionOfDistanceThroughGlyphForPoint: %@ inTextContainer: %@",
          NSStringFromPoint(point), container);
#define DEBUG_fractionOfDistanceThroughGlyphForPoint_inTextContainer 0
#endif

    CGFloat fraction;

    [self glyphIndexForPoint: point
                           inTextContainer: container
            fractionOfDistanceThroughGlyph: &fraction];

    return fraction;
}

// Returns the current glyph range for the given container
- (NSRange) _currentGlyphRangeForTextContainer: (NSTextContainer *) container {

#if DEBUG_LM_LAYOUT
    NSLog(@"_currentGlyphRangeForTextContainer: %@", container);
#define DEBUG__currentGlyphRangeForTextContainer 0
#endif

    NSRange result = NSMakeRange(0, 0);
    BOOL assignFirst = YES;
    NSRangeEnumerator state = NSRangeEntryEnumerator(_glyphFragments);
    NSRange range;
    NSGlyphFragment *fragment;

    while (NSNextRangeEnumeratorEntry(&state, &range, (void **) &fragment)) {
        if (fragment->container == container) {
            if (assignFirst) {
                result = range;
                assignFirst = NO;
            } else {
                result = NSUnionRange(result, range);
            }
        }
    }
#if DEBUG__currentGlyphRangeForTextContainer
    NSLog(@"returning: %@", NSStringFromRange(result));
#endif

    return result;
}

// Validate the glyphs and layout if needed and returns the glyph range for the
// given container
- (NSRange) glyphRangeForTextContainer: (NSTextContainer *) container {
#if DEBUG_LM_LAYOUT
    NSLog(@"glyphRangeForTextContainer: %@", container);
#endif
    [self validateGlyphsAndLayoutForContainer: container];
    return [self _currentGlyphRangeForTextContainer: container];
}

- (NSRange) glyphRangeForCharacterRange: (NSRange) charRange
                   actualCharacterRange: (NSRangePointer) actualCharRange
{

#if DEBUG_LM_LAYOUT
    NSLog(@"glyphRangeForCharacterRange: %@ actualCharacterRange: %p",
          NSStringFromRange(charRange), actualCharRange);
#endif

    if (actualCharRange != NULL)
        *actualCharRange = charRange;

    return charRange;
}

- (NSRange) glyphRangeForBoundingRect: (NSRect) bounds
                      inTextContainer: (NSTextContainer *) container
{

#if DEBUG_LM_LAYOUT
    NSLog(@"glyphRangeForBoundingRect: %@ inTextContainer: %@",
          NSStringFromRect(bounds), container);
#define DEBUG_glyphRangeForBoundingRect_inTextContainer 0
#endif

    [self validateGlyphsAndLayoutForContainer: container]; {
        NSRange result = NSMakeRange(NSNotFound, 0);
        NSRangeEnumerator state = NSRangeEntryEnumerator(_glyphFragments);
        NSRange range;
        NSGlyphFragment *fragment;

        while (NSNextRangeEnumeratorEntry(&state, &range,
                                          (void **) &fragment)) {
            if (fragment->container == container) {
                NSRect check = fragment->rect;

                if (NSIntersectsRect(bounds, check)) {
                    NSRange extend = range;

                    if (result.location == NSNotFound)
                        result = extend;
                    else
                        result = NSUnionRange(result, extend);
                }
            }
        }
#if DEBUG_glyphRangeForBoundingRect_inTextContainer
        NSLog(@"returning: %@", NSStringFromRange(result));
#endif
        return result;
    }
}

- (NSRange) glyphRangeForBoundingRectWithoutAdditionalLayout: (NSRect) bounds
                                             inTextContainer:
                                                     (NSTextContainer *)
                                                             container
{
    return NSMakeRange(0, 0);
}

- (NSRange) rangeOfNominallySpacedGlyphsContainingIndex: (NSUInteger) glyphIndex
{
    return NSMakeRange(0, 0);
}

- (NSRect) boundingRectForGlyphRange: (NSRange) glyphRange
                     inTextContainer: (NSTextContainer *) container
{

#if DEBUG_LM_LAYOUT
    NSLog(@"boundingRectForGlyphRange: %@ inTextContainer: %@",
          NSStringFromRange(glyphRange), container);
#define DEBUG_boundingRectForGlyphRange_inTextContainer 0
#endif

    glyphRange = [self validateGlyphsAndLayoutForGlyphRange: glyphRange]; {
        NSRect result = NSZeroRect;
        NSUInteger i, rectCount = 0;
        NSRect *rects = [self rectArrayForGlyphRange: glyphRange
                            withinSelectedGlyphRange: NSMakeRange(NSNotFound, 0)
                                     inTextContainer: container
                                           rectCount: &rectCount];

        for (i = 0; i < rectCount; i++) {
            if (i == 0)
                result = rects[i];
            else
                result = NSUnionRect(result, rects[i]);
        }
#if DEBUG_boundingRectForGlyphRange_inTextContainer
        NSLog(@"returning: %@", NSStringFromRect(result));
#endif
        return result;
    }
}

// The width of the glyphs of a line fragment. The used rect of a fragment
// also covers the container's line fragment padding, so the extent of the
// glyphs themselves is derived from the fragment's location instead.
static CGFloat _glyphRunWidth(NSFont *font, const NSGlyph *glyphs,
                              NSInteger length) {
    CGFloat total = 0;
    NSGlyph previousGlyph = NSNullGlyph;
    BOOL ignore;

    for (NSInteger i = 0; i < length; i++) {
        NSGlyph glyph = glyphs[i];

        if (glyph == NSControlGlyph)
            glyph = NSNullGlyph;
        total += [font positionOfGlyph: glyph
                         precededByGlyph: previousGlyph
                               isNominal: &ignore]
                         .x;
        previousGlyph = glyph;
    }
    return total;
}

static inline void _appendRectToCache(NSLayoutManager *self, NSRect rect) {
    if (self->_rectCacheCount >= self->_rectCacheCapacity) {
        self->_rectCacheCapacity *= 2;
        self->_rectCache =
                NSZoneRealloc(NULL, self->_rectCache,
                              sizeof(NSRect) * self->_rectCacheCapacity);
    }

    self->_rectCache[self->_rectCacheCount++] = rect;
}

- (NSRect *) rectArrayForGlyphRange: (NSRange) glyphRange
           withinSelectedGlyphRange: (NSRange) selGlyphRange
                    inTextContainer: (NSTextContainer *) container
                          rectCount: (NSUInteger *) rectCount
{

#if DEBUG_LM_LAYOUT
    NSLog(@"rectArrayForGlyphRange: %@ withinSelectedGlyphRange: %@ "
          @"inTextContainer: %@ rectCount: %p",
          NSStringFromRange(glyphRange), NSStringFromRange(selGlyphRange),
          container, rectCount);
#define DEBUG_rectArrayForGlyphRange_withinSelectedGlyphRange_inTextContainer_rectCount \
    0
#endif

    [self validateGlyphsAndLayoutForContainer: container];

    NSRange remainder =
            (selGlyphRange.location == NSNotFound) ? glyphRange : selGlyphRange;

    _rectCacheCount = 0;
    do {
        NSRange range;
        // Get the fragment to the range to process
        NSGlyphFragment *fragment =
                fragmentAtGlyphIndex(self, remainder.location, &range);

        if (fragment == NULL) {

#if DEBUG_rectArrayForGlyphRange_withinSelectedGlyphRange_inTextContainer_rectCount
            NSLog(@"fragment not found at index: %u - bailing",
                  remainder.location);
#endif

            break;
        } else if (fragment->container == container) {
#if DEBUG_rectArrayForGlyphRange_withinSelectedGlyphRange_inTextContainer_rectCount
            NSLog(@"This existing range is %@\n", NSStringFromRange(range));
#endif

            // The part of the that we are interested in - start with the full
            // rect, we'll change it if we don't want the full fragment
            NSRect fill = fragment->rect;
            if (NSLocationInRange(remainder.location, range)) {
                // Part of the line fragment to process
                NSRange intersect;
                if (remainder.length > 0)
                    intersect = NSIntersectionRange(remainder, range);
                else // NSIntersectionRange's returned location is undefined for
                     // 0-length ranges
                    intersect = NSMakeRange(remainder.location, 0);

#if DEBUG_rectArrayForGlyphRange_withinSelectedGlyphRange_inTextContainer_rectCount
                NSLog(@"found a sub range to process: %@",
                      NSStringFromRange(intersect));
#endif

                // We only want part of that fragment - so check the part we
                // want by getting the interesting glyphs locations

                // Start from the glyphs themselves - we're not interested in
                // any potential white space lead (the used rect also covers
                // the line fragment padding)
                fill = fragment->usedRect;
                fill.origin.x = fragment->location.x;

                NSGlyph glyphs[range.length], previousGlyph = NSNullGlyph;
                NSUInteger order[range.length];
                NSInteger i, length = [self getOrderedGlyphs: glyphs
                                                       range: range
                                                   baseLevel: 0
                                                       order: order];
                NSFont *font = [self _fontForGlyphRange: range];
                CGFloat advance;
                BOOL ignore;

                BOOL ltor = fragment->leftToRight;

                // Starts with a 0 width - we'll grow it with the width of the
                // glyphs from our intersect range
                fill.size.width = 0;

                CGFloat glyphsRight =
                        fragment->location.x +
                        _glyphRunWidth(font, glyphs, length);
                // The last line of the text has nothing after its glyphs to
                // select, other lines are selected up to the end of the
                // fragment
                CGFloat fragmentRight = NSMaxRange(range) >= [self numberOfGlyphs]
                                                ? glyphsRight
                                                : NSMaxX(fragment->rect);

                if (ltor) {
                    for (i = 0; i < length; i++) {
                        NSGlyph glyph = glyphs[i];

                        if (glyph == NSControlGlyph)
                            glyph = NSNullGlyph;

                        advance = [font positionOfGlyph: glyph
                                          precededByGlyph: previousGlyph
                                                isNominal: &ignore]
                                          .x;
                        if (range.location + i < intersect.location) {
                            // Not yet part of intersect - advance the fill rect
                            // origin
                            fill.origin.x += advance;
                        } else if (range.location + i <
                                   NSMaxRange(intersect)) {
                            // Part of intersect - grow the width (the glyph
                            // at the end of the range is not part of it)
                            fill.size.width += advance;
                        }
                        previousGlyph = glyph;
                    }
                    if (NSMaxRange(range) <= NSMaxRange(remainder)) {
                        // We want the full end of fragment, so grow the width
                        // to the end of the fragment rect
                        fill.size.width = fragmentRight - fill.origin.x;
                    }
                } else {
                    // Start from the right
                    fill.origin.x = glyphsRight;
                    for (i = 0; i < length; i++) {
                        NSGlyph glyph = glyphs[order[i]];

                        if (glyph == NSControlGlyph)
                            glyph = NSNullGlyph;

                        advance = [font positionOfGlyph: glyph
                                          precededByGlyph: previousGlyph
                                                isNominal: &ignore]
                                          .x;
                        if (range.location + i <= intersect.location) {
                            // Not yet part of intersect - advance the fill rect
                            // origin to the left
                            fill.origin.x -= advance;
                        } else if (range.location + i <=
                                   NSMaxRange(intersect)) {
                            // Part of intersect - grow the width to the left
                            fill.size.width += advance;
                            fill.origin.x -= advance;
                        }
                        previousGlyph = glyph;
                    }
                    if (NSMaxRange(range) <= NSMaxRange(remainder)) {
                        // We want the full start of fragment, so grow the width
                        // to the start of the fragment rect
                        CGFloat max = NSMaxX(fill);
                        fill.origin.x = fragment->location.x;
                        fill.size.width = max - fill.origin.x;
                    }
                }

                range = intersect;
            }

#if DEBUG_rectArrayForGlyphRange_withinSelectedGlyphRange_inTextContainer_rectCount
            NSLog(@"appending rect to cache: %@", NSStringFromRect(fill));
#endif

            _appendRectToCache(self, fill);
        }
        // Remove the range we just processed
        remainder.length = NSMaxRange(remainder) - NSMaxRange(range);
        remainder.location = NSMaxRange(range);

    } while (remainder.length > 0);

    *rectCount = _rectCacheCount;

#if DEBUG_rectArrayForGlyphRange_withinSelectedGlyphRange_inTextContainer_rectCount
    NSLog(@"returning %d rects", *rectCount);
#endif

    return _rectCache;
}

- (NSUInteger) characterIndexForGlyphAtIndex: (NSUInteger) glyphIndex {
    // Validate glyphs;

    return glyphIndex;
}

- (NSRange) characterRangeForGlyphRange: (NSRange) glyphRange
                       actualGlyphRange: (NSRangePointer) actualGlyphRange
{
    if (actualGlyphRange != NULL)
        *actualGlyphRange = glyphRange;

    return glyphRange;
}

- (NSRect *) rectArrayForCharacterRange: (NSRange) characterRange
           withinSelectedCharacterRange: (NSRange) selectedCharRange
                        inTextContainer: (NSTextContainer *) container
                              rectCount: (NSUInteger *) rectCount
{

#if DEBUG_LM_LAYOUT
    NSLog(@"rectArrayForCharacterRange: %@ withinSelectedCharacterRange: %@ "
          @"inTextContainer: %@ rectCount: %p",
          NSStringFromRange(characterRange),
          NSStringFromRange(selectedCharRange), container, rectCount);
#endif

    NSRange glyphRange = [self glyphRangeForCharacterRange: characterRange
                                      actualCharacterRange: NULL];
    NSRange glyphSelRange = [self glyphRangeForCharacterRange: selectedCharRange
                                         actualCharacterRange: NULL];

    return [self rectArrayForGlyphRange: glyphRange
               withinSelectedGlyphRange: glyphSelRange
                        inTextContainer: container
                              rectCount: rectCount];
}

- (NSUInteger) firstUnlaidGlyphIndex {
    return NSNotFound;
}

- (NSUInteger) firstUnlaidCharacterIndex {
    return NSNotFound;
}

- (void) getFirstUnlaidCharacterIndex: (NSUInteger *) charIndex
                           glyphIndex: (NSUInteger *) glyphIndex
{
    *charIndex = [self firstUnlaidCharacterIndex];
    *glyphIndex = [self firstUnlaidGlyphIndex];
}

#define DEBUG_LM_DRAWING 0

// UIFoundation draws into the context AppKit has made current, which it gets
// through the provider AppKit registers. Without one there is nothing to draw
// into and these do nothing.

// Fills the rectangle with the current fill colour, replacing what is there.
static void _fillRect(NSRect rect) {
    CGContextRef context = _UIFoundationCurrentCGContext();
    if (context == NULL)
        return;
    CGContextSaveGState(context);
    CGContextSetBlendMode(context, kCGBlendModeCopy);
    CGContextFillRect(context, NSRectToCGRect(rect));
    CGContextRestoreGState(context);
}

// Strokes the lines with the current stroke colour, using the given width,
// cap and dash pattern and the default join, miter limit and flatness.
static void _strokeLines(CGPathRef path, CGFloat lineWidth, CGLineCap lineCap,
                         const CGFloat *dashes, size_t dashCount)
{
    CGContextRef context = _UIFoundationCurrentCGContext();
    if (context == NULL)
        return;
    CGContextSaveGState(context);
    CGContextSetLineWidth(context, lineWidth);
    CGContextSetMiterLimit(context, 10.0);
    CGContextSetFlatness(context, 0.6);
    CGContextSetLineCap(context, lineCap);
    CGContextSetLineJoin(context, kCGLineJoinMiter);
    CGContextSetLineDash(context, 0.0, dashes, dashCount);
    CGContextBeginPath(context);
    CGContextAddPath(context, path);
    CGContextStrokePath(context);
    CGContextRestoreGState(context);
}

- (void) showPackedGlyphs: (char *) glyphs
                    length: (NSUInteger) length
                glyphRange: (NSRange) glyphRange
                   atPoint: (NSPoint) point
                      font: (NSFont *) font
                     color: (NSColor *) color
        printingAdjustment: (NSSize) printingAdjustment
{
    CGContextRef context = _UIFoundationCurrentCGContext();
    if (context == NULL)
        return;
    // Sets the font, its size and the text matrix in the current context.
    [font set];
    if (color)
        [color setFill];
    else
        [_UIFoundationTextColorForRole(_UIFoundationTextColorRoleControlText) setFill];
    CGGlyph *cgGlyphs = (CGGlyph *) glyphs;
    NSInteger cgGlyphsLength = length / 2;
    CGSize advances[cgGlyphsLength];
    NSGlyph nsglyphs[cgGlyphsLength];
    for (NSInteger i = 0; i < cgGlyphsLength; ++i) {
        nsglyphs[i] = cgGlyphs[i];
    }
    [font getAdvancements: advances forGlyphs: nsglyphs count: cgGlyphsLength];
    // The pair kerning the layout gave a glyph moves the glyphs after it.
    for (NSInteger i = 1; i < cgGlyphsLength; ++i) {
        BOOL ignore;
        CGFloat kerning = [font positionOfGlyph: nsglyphs[i] precededByGlyph: nsglyphs[i - 1] isNominal: &ignore].x -
                          advances[i].width;
        advances[i - 1].width += kerning;
    }

#if DEBUG_LM_SHOWPACKEDGLYPHS
    for (NSInteger i = 0; i < cgGlyphsLength; i++) {
        NSLog(@"glyph: %d advancement: %@", nsglyphs[i],
              NSStringFromSize(advances[i]));
    }
#endif

    CGContextSetTextPosition(context, point.x, point.y);
    CGContextShowGlyphsWithAdvances(context, cgGlyphs, advances,
                                    cgGlyphsLength);
}

- (void) drawSelectionAtPoint: (NSPoint) origin {

#if DEBUG_LM_DRAWING
    NSLog(@"drawSelectionAtPoint: %@", NSStringFromPoint(origin));
#define DEBUG_LM_DRAWSELECTIONATPOINT 0
#endif

    NSTextView *textView = [self textViewForBeginningOfSelection];
    NSColor *selectedColor = [[textView selectedTextAttributes]
            objectForKey: NSBackgroundColorAttributeName];
    NSRange range;
    NSRect *rectArray;
    NSUInteger i, rectCount = 0;

    if (textView == nil)
        return;

    range = [textView selectedRange];
    if (range.length == 0)
        return;

    rectArray = [self rectArrayForGlyphRange: range
                    withinSelectedGlyphRange: range
                             inTextContainer: [textView textContainer]
                                   rectCount: &rectCount];

#if DEBUG_LM_DRAWSELECTIONATPOINT
    NSLog(@"    rectArray: %@", rectArray);
#endif

    if (selectedColor == nil)
        selectedColor = _UIFoundationTextColorForRole(
                _UIFoundationTextColorRoleSelectedTextBackground);

    [selectedColor setFill];
    for (i = 0; i < rectCount; i++) {
        NSRect fill = rectArray[i];
        fill.origin.x += origin.x;
        fill.origin.y += origin.y;
        _fillRect(fill);
    }
}

- (void) drawBackgroundForGlyphRange: (NSRange) glyphRange
                             atPoint: (NSPoint) origin
{

#if DEBUG_LM_DRAWING
    NSLog(@"drawBackgroundForGlyphRange: %@ atPoint: %@",
          NSStringFromRange(glyphRange), NSStringFromPoint(origin));
#define DEBUG_LM_DRAWBACKGROUNDFORGLYPHRANGE 0
#endif

    glyphRange = [self validateGlyphsAndLayoutForGlyphRange: glyphRange]; {
        NSTextContainer *container =
                [self textContainerForGlyphAtIndex: glyphRange.location
                                    effectiveRange: &glyphRange];
        if (container == nil) {
            return;
        }
        NSRange characterRange = [self characterRangeForGlyphRange: glyphRange
                                                  actualGlyphRange: NULL];
        NSUInteger location = characterRange.location;
        NSUInteger limit = NSMaxRange(characterRange);
        BOOL isFlipped = _UIFoundationCurrentContextIsFlipped();
        CGFloat usedHeight =
                [self usedRectForTextContainer: container].size.height;

        while (location < limit) {
            NSRange effectiveRange;
            NSDictionary *attributes =
                    [_textStorage attributesAtIndex: location
                                     effectiveRange: &effectiveRange];
            // Make sure the effectiveRange doesn't take us backwards!
            NSInteger reduction = location - effectiveRange.location;
            effectiveRange.location = location;
            effectiveRange.length -= reduction;

            BOOL continueSearch = YES;
            // We've got to look carefully because we need to chop the
            // effectiveRange so that the temporary attributes get added at the
            // right places
            for (NSInteger tempOffset = effectiveRange.location;
                 continueSearch && tempOffset < NSMaxRange(effectiveRange);
                 tempOffset++) {
                NSRange tempEffectiveRange = NSMakeRange(0, 0);
                NSDictionary *tempAttrs =
                        [self temporaryAttributesAtCharacterIndex: tempOffset
                                                   effectiveRange:
                                                           &tempEffectiveRange];
                if (NSEqualRanges(effectiveRange, tempEffectiveRange)) {
                    // Things are lined up nicely - we can just use the
                    // effectiveRange as is
                    continueSearch = NO;
                } else {
                    if ([tempAttrs count] > 0) {
                        // We've got some real attributes to worry about
                        if (tempOffset > effectiveRange.location) {
                            // The temporary attributes start further along so
                            // cut effectiveRange down so it ends at the start
                            // of the temp range
                            effectiveRange.length =
                                    tempOffset - effectiveRange.location;
                            continueSearch = NO;
                        } else if (tempEffectiveRange.length <
                                   effectiveRange.length) {
                            // The temp range starts at the same point as the
                            // effective range - but it's shorter so cut the
                            // effectiveRange length down to match.
                            effectiveRange.length = tempEffectiveRange.length;
                            continueSearch = NO;
                        }
                    }
                }
            }

            NSRange tempEffectiveRange = NSMakeRange(0, 0);
            NSDictionary *tempAttrs = [self
                    temporaryAttributesAtCharacterIndex: location
                                         effectiveRange: &tempEffectiveRange];
            if ([tempAttrs count] > 0) {
                // Merge in the temporary attributes
                BOOL checkTemporaryAttributesUsage = [_delegate
                        respondsToSelector: @selector
                        (layoutManager:
                                shouldUseTemporaryAttributes:forDrawingToScreen
                                                            :atCharacterIndex
                                                            :effectiveRange:)];
                if (checkTemporaryAttributesUsage) {
                    tempAttrs = [_delegate layoutManager: self
                            shouldUseTemporaryAttributes: tempAttrs
                                      forDrawingToScreen:
                                              _UIFoundationCurrentContextIsDrawingToScreen()
                                        atCharacterIndex: location
                                          effectiveRange: NULL];
                }
                NSMutableDictionary *dict =
                        [[attributes mutableCopy] autorelease];
                [dict addEntriesFromDictionary: tempAttrs];
                attributes = dict;
            }

            NSColor *color =
                    [attributes objectForKey: NSBackgroundColorAttributeName];

            effectiveRange =
                    NSIntersectionRange(characterRange, effectiveRange);

            if (color != nil) {
                NSUInteger i, rectCount;
                NSRect *rects = [self
                          rectArrayForCharacterRange: effectiveRange
                        withinSelectedCharacterRange: NSMakeRange(NSNotFound, 0)
                                     inTextContainer: container
                                           rectCount: &rectCount];

                [color setFill];

                for (i = 0; i < rectCount; i++) {
                    NSRect fill = rects[i];

                    if (!isFlipped)
                        fill.origin.y =
                                usedHeight - (fill.origin.y + fill.size.height);

                    fill.origin.x += origin.x;
                    fill.origin.y += origin.y;

                    _fillRect(fill);
                }
            }

            location = NSMaxRange(effectiveRange);
        }
    }
    [self drawSelectionAtPoint: origin];
}

- (void) drawUnderlineForGlyphRange: (NSRange) glyphRange
                      underlineType: (NSInteger) underlineVal
                     baselineOffset: (CGFloat) baselineOffset
                   lineFragmentRect: (NSRect) lineRect
             lineFragmentGlyphRange: (NSRange) lineGlyphRange
                    containerOrigin: (NSPoint) containerOrigin
{
#if DEBUG_LM_DRAWING
    NSLog(@"drawUnderlineForGlyphRange: %@ underlineType: %@ baselineOffset: "
          @"%f "
          @"lineFragmentRect: %@ lineFragmentGlyphRange: %@ containerOrigin: "
          @"%@",
          NSStringFromRange(glyphRange), underlineVal, baselineOffset,
          NSStringFromRect(lineRect), NSStringFromRange(lineGlyphRange),
          NSStringFromPoint(containerOrigin));
#define DEBUG_LM_DRAWUNDERLINEFORGLYPHRANGE 0
#endif

    NSUInteger i, rectCount;
    NSRange characterRange = [self characterRangeForGlyphRange: glyphRange
                                              actualGlyphRange: NULL];
    BOOL isFlipped = _UIFoundationCurrentContextIsFlipped();
    NSTextContainer *container =
            [self textContainerForGlyphAtIndex: glyphRange.location
                                effectiveRange: NULL];

    NSRect *rects = [self rectArrayForCharacterRange: characterRange
                        withinSelectedCharacterRange: NSMakeRange(NSNotFound, 0)
                                     inTextContainer: container
                                           rectCount: &rectCount];

    CGMutablePathRef path = CGPathCreateMutable();

    // Lots more stylistic options available than just this
    CGFloat lineWidth = (underlineVal & NSUnderlineStyleThick) ? 1 : .5;
    CGFloat lineDash[] = {.75, 3.25};
    size_t lineDashCount = (underlineVal & NSUnderlinePatternDash)
                                   ? sizeof(lineDash) / sizeof(lineDash[0])
                                   : 0;

    NSPoint origin = containerOrigin;

    for (i = 0; i < rectCount; i++) {
        NSRect fill = rects[i];

        if (isFlipped)
            fill.origin.y += (fill.size.height - 1);

        fill.origin.x += origin.x;
        fill.origin.y +=
                origin.y + baselineOffset -
                .5; // .5 so it's better aligned on pixels - looks sharper
        CGFloat width = fill.size.width;
        CGPathMoveToPoint(path, NULL, fill.origin.x, fill.origin.y);
        CGPathAddLineToPoint(path, NULL, fill.origin.x + width, fill.origin.y);
    }

    NSDictionary *attributes =
            [_textStorage attributesAtIndex: characterRange.location
                             effectiveRange: NULL];
    // Don't forget the temporary attributes
    NSDictionary *tmpAttrs =
            [self temporaryAttributesAtCharacterIndex: characterRange.location
                                       effectiveRange: NULL];
    if ([tmpAttrs count] > 0) {
        BOOL checkTemporaryAttributesUsage = [_delegate
                respondsToSelector: @selector
                (layoutManager:
                        shouldUseTemporaryAttributes:forDrawingToScreen
                                                    :atCharacterIndex
                                                    :effectiveRange:)];
        if (checkTemporaryAttributesUsage) {
            tmpAttrs = [_delegate layoutManager: self
                    shouldUseTemporaryAttributes: tmpAttrs
                              forDrawingToScreen: _UIFoundationCurrentContextIsDrawingToScreen()
                                atCharacterIndex: characterRange.location
                                  effectiveRange: NULL];
        }
        NSMutableDictionary *dict = [[attributes mutableCopy] autorelease];
        [dict addEntriesFromDictionary: tmpAttrs];
        attributes = dict;
    }

    NSColor *underlineColor =
            [attributes objectForKey: NSUnderlineColorAttributeName];
    if (underlineColor == nil) {
        // Default to foreground color attribute...
        underlineColor =
                [attributes objectForKey: NSForegroundColorAttributeName];
        if (underlineColor == nil) {
            // ... and to black if still no luck
            underlineColor = _UIFoundationTextColorForRole(
                    _UIFoundationTextColorRoleText);
        }
    }
    [underlineColor set];
    _strokeLines(path, lineWidth, kCGLineCapSquare, lineDash, lineDashCount);
    CGPathRelease(path);
}

- (void) underlineGlyphRange: (NSRange) glyphRange
                 underlineType: (NSInteger) underlineVal
              lineFragmentRect: (NSRect) lineRect
        lineFragmentGlyphRange: (NSRange) lineGlyphRange
               containerOrigin: (NSPoint) containerOrigin
{
    // A full implementation would honor options like breaking the underline for
    // whitespace.
    [self drawUnderlineForGlyphRange: glyphRange
                       underlineType: underlineVal
                      baselineOffset: 0
                    lineFragmentRect: lineRect
              lineFragmentGlyphRange: lineGlyphRange
                     containerOrigin: containerOrigin];
}

- (void) drawStrikethroughForGlyphRange: (NSRange) glyphRange
                      strikethroughType: (NSInteger) strikethroughVal
                         baselineOffset: (CGFloat) baselineOffset
                       lineFragmentRect: (NSRect) lineRect
                 lineFragmentGlyphRange: (NSRange) lineGlyphRange
                        containerOrigin: (NSPoint) containerOrigin
{
#if DEBUG_LM_DRAWING
    NSLog(@"drawStrikethroughForGlyphRange: %@ strikethroughType: %@ "
          @"baselineOffset: %f lineFragmentRect: %@ lineFragmentGlyphRange: %@ "
          @"containerOrigin: %@",
          NSStringFromRange(glyphRange), strikethroughVal, baselineOffset,
          NSStringFromRect(lineRect), NSStringFromRange(lineGlyphRange),
          NSStringFromPoint(containerOrigin));
#define DEBUG_LM_DRAWSTRIKETHROUGHFORGLYPHRANGE 0
#endif

    NSUInteger i, rectCount;
    NSRange characterRange = [self characterRangeForGlyphRange: glyphRange
                                              actualGlyphRange: NULL];
    BOOL isFlipped = _UIFoundationCurrentContextIsFlipped();
    NSTextContainer *container =
            [self textContainerForGlyphAtIndex: glyphRange.location
                                effectiveRange: NULL];
    NSRect *rects = [self rectArrayForCharacterRange: characterRange
                        withinSelectedCharacterRange: NSMakeRange(NSNotFound, 0)
                                     inTextContainer: container
                                           rectCount: &rectCount];

    CGMutablePathRef path = CGPathCreateMutable();

    // Lots more stylistic options available than just this
    CGFloat lineWidth = (strikethroughVal & NSUnderlineStyleThick) ? 1 : .5;
    CGFloat lineDash[] = {.75, 3.25};
    size_t lineDashCount = (strikethroughVal & NSUnderlinePatternDash)
                                   ? sizeof(lineDash) / sizeof(lineDash[0])
                                   : 0;

    NSPoint origin = containerOrigin;

    for (i = 0; i < rectCount; i++) {
        NSRect fill = rects[i];

        fill.origin.y += (fill.size.height / 2);

        fill.origin.x += origin.x;
        fill.origin.y += origin.y + baselineOffset;
        CGFloat width = fill.size.width;
        CGPathMoveToPoint(path, NULL, fill.origin.x, fill.origin.y);
        CGPathAddLineToPoint(path, NULL, fill.origin.x + width, fill.origin.y);
    }

    NSDictionary *attributes =
            [_textStorage attributesAtIndex: characterRange.location
                             effectiveRange: NULL];
    NSColor *underlineColor =
            [attributes objectForKey: NSUnderlineColorAttributeName];
    if (underlineColor == nil) {
        underlineColor =
                _UIFoundationTextColorForRole(_UIFoundationTextColorRoleText);
    }
    [underlineColor set];
    _strokeLines(path, lineWidth, kCGLineCapSquare, lineDash, lineDashCount);
    CGPathRelease(path);
}

- (void) strikethroughGlyphRange: (NSRange) glyphRange
               strikethroughType: (NSInteger) strikethroughVal
                lineFragmentRect: (NSRect) lineRect
          lineFragmentGlyphRange: (NSRange) lineGlyphRange
                 containerOrigin: (NSPoint) containerOrigin
{
    // A full implementation would honor options like breaking the strikethrough
    // for whitespace.
    [self drawStrikethroughForGlyphRange: glyphRange
                       strikethroughType: strikethroughVal
                          baselineOffset: 0
                        lineFragmentRect: lineRect
                  lineFragmentGlyphRange: lineGlyphRange
                         containerOrigin: containerOrigin];
}

- (void) drawSpellingState: (NSNumber *) spellingState
                glyphRange: (NSRange) glyphRange
                 container: (NSTextContainer *) container
                    origin: (NSPoint) origin
{
#if DEBUG_LM_DRAWING
    NSLog(@"drawSpellingState: %@ glyphRange: %@ container: %p origin: %@",
          spellingState, NSStringFromRange(glyphRange), container,
          NSStringFromPoint(origin));
#define DEBUG_LM_DRAWSPELLINGSTATE 0
#endif
    if ([container textView] == nil) {
        // Don't draw anything if we aren't editing
        return;
    }
    NSUInteger i, rectCount;
    NSRange characterRange = [self characterRangeForGlyphRange: glyphRange
                                              actualGlyphRange: NULL];
    BOOL isFlipped = _UIFoundationCurrentContextIsFlipped();
    CGFloat usedHeight = [self usedRectForTextContainer: container].size.height;
    NSRect *rects = [self rectArrayForCharacterRange: characterRange
                        withinSelectedCharacterRange: NSMakeRange(NSNotFound, 0)
                                     inTextContainer: container
                                           rectCount: &rectCount];

    CGMutablePathRef path = CGPathCreateMutable();
    CGFloat lineDash[] = {.75, 3.25};

    for (i = 0; i < rectCount; i++) {
        NSRect fill = rects[i];

        if (isFlipped)
            fill.origin.y += (fill.size.height - 1);

        fill.origin.x += origin.x + 2; // some margin because of the line cap
        fill.origin.y += origin.y;
        CGFloat width = fill.size.width;
        CGPathMoveToPoint(path, NULL, fill.origin.x, fill.origin.y);
        CGPathAddLineToPoint(path, NULL, fill.origin.x + width, fill.origin.y);
    }
    [_UIFoundationTextColorForRole(_UIFoundationTextColorRoleSpellingIndicator)
            setStroke];
    _strokeLines(path, 2., kCGLineCapRound, lineDash,
                 sizeof(lineDash) / sizeof(lineDash[0]));
    CGPathRelease(path);
}

- (NSColor *) _selectedColor {
    NSTextView *textView = [self textViewForBeginningOfSelection];
    NSColor *selectedColor = [[textView selectedTextAttributes]
            objectForKey: NSForegroundColorAttributeName];

    if (selectedColor == nil) {
        selectedColor = _UIFoundationTextColorForRole(
                _UIFoundationTextColorRoleSelectedText);
    }
    return selectedColor;
}

- (void) _drawGlyphs: (NSGlyph *) glyphs
                length: (NSUInteger) length
                 range: (NSRange) range
               atPoint: (NSPoint) point
           inContainer: (NSTextContainer *) container
        withAttributes: (NSDictionary *) attributes
                origin: (NSPoint) origin
{
#if DEBUG_LM_DRAWING
    NSLog(@"_drawGlyphs: %p length: %d range: %@ atPoint: %@ inContainer: %p "
          @"withAttributes: %@ origin: %@",
          glyphs, length, NSStringFromRange(range), NSStringFromPoint(point),
          container, attributes, NSStringFromPoint(origin));
#define DEBUG_LM_DRAWGLYPHS 0
#endif

    NSColor *color = NSForegroundColorAttributeInDictionary(attributes);
    NSFont *font = [self _layoutFontForFont: NSFontAttributeInDictionary(attributes)];

    // First draw the packed glyphs
    char packedGlyphs[length * 2]; // Specs says length*4+1 but needed buffer
                                   // size is at max length*2 on Cocotron
    NSInteger packedGlyphsLength = NSConvertGlyphsToPackedGlyphs(
            glyphs, length, NSNativeShortGlyphPacking, packedGlyphs);
    [self showPackedGlyphs: packedGlyphs
                        length: packedGlyphsLength
                    glyphRange: range
                       atPoint: point
                          font: font
                         color: color
            printingAdjustment: NSZeroSize];

    NSRange glyphRange = range;

    // Next take a look at the overprinting options
    BOOL underline = [[attributes objectForKey: NSUnderlineStyleAttributeName]
            boolValue];
    BOOL strikeThru = [[attributes
            objectForKey: NSStrikethroughStyleAttributeName] boolValue];

    if (underline || strikeThru) {
        NSRange lineGlyphRange;
        NSRect lineRect =
                [self lineFragmentRectForGlyphAtIndex: glyphRange.location
                                       effectiveRange: &lineGlyphRange];

        if (underline) {
            [self underlineGlyphRange: glyphRange
                             underlineType: NSUnderlineStyleThick
                          lineFragmentRect: lineRect
                    lineFragmentGlyphRange: lineGlyphRange
                           containerOrigin: origin];
        }

        if (strikeThru) {
            // Make sure we've got a good strikeThru color
            NSColor *strikeThruColor = [attributes
                    objectForKey: NSStrikethroughColorAttributeName];
            if (strikeThruColor == nil) {
                strikeThruColor = _UIFoundationTextColorForRole(
                        _UIFoundationTextColorRoleText);
            }
            [self strikethroughGlyphRange: glyphRange
                         strikethroughType: NSUnderlineStyleThick
                          lineFragmentRect: lineRect
                    lineFragmentGlyphRange: lineGlyphRange
                           containerOrigin: origin];
        }
    }

    NSNumber *spellingState =
            [attributes objectForKey: NSSpellingStateAttributeName];
    if (spellingState != nil) {
        [self drawSpellingState: spellingState
                     glyphRange: glyphRange
                      container: container
                         origin: origin];
    }
}

- (void) _drawAttachment: (NSTextAttachment *) attachment
        atCharacterIndex: (NSUInteger) index
                 atPoint: (NSPoint) point
{
#if DEBUG_LM_DRAWING
    NSLog(@"_drawAttachment: %@ atCharacterIndex: %d atPoint: %@", attachment,
          index, NSStringFromPoint(point));
#define DEBUG_LM_DRAWATTACHMENT 0
#endif
    id<NSTextAttachmentCell> cell = [attachment attachmentCell];
    NSRect frame;

    /* `point` is the baseline origin of the run, not the top-left of the cell: using it directly
     * painted every attachment one cell-height below its own glyphs (measured: text row y=103..111,
     * attachment y=112..135, i.e. offset by exactly cellSize.height). Convert from the baseline. */
    NSPoint baselineOffset = [cell cellBaselineOffset];
    frame.size = [cell cellSize];
    frame.origin.x = point.x + baselineOffset.x;

    NSTextView *textView = [self textViewForBeginningOfSelection];
    if ([textView isFlipped])
        frame.origin.y = point.y + baselineOffset.y - frame.size.height;
    else
        frame.origin.y = point.y + baselineOffset.y;

    [cell drawWithFrame: frame
                    inView: textView
            characterIndex: index
             layoutManager: self];
}

- (CGFloat) _drawGlyphsForSubGlyphRange: (NSRange) range
                            forFragment: (NSGlyphFragment *) fragment
                                atPoint: (NSPoint) origin
                        subRangeXOffset: (CGFloat) xOffset
                            inContainer: (NSTextContainer *) container
                         withAttributes: (NSDictionary *) attributes
                             usedHeight: (CGFloat) usedHeight
{
#if DEBUG_LM_DRAWING
    NSLog(@"_drawGlyphsForSubGlyphRange: %@ forFragment: atPoint: %@ "
          @"subRangeXOffset: %f inContainer: %P withAttributes: %P usedHeight: "
          @"%f",
          NSStringFromRange(range),
          //          fragment, // Trying to log fragment results in seg-fault.
          NSStringFromPoint(origin), xOffset, container, attributes,
          usedHeight);
#define DEBUG_LM_DRAWGLYPHSFORSUBGLYPHRANGE 0
#endif

    NSTextView *textView = [self textViewForBeginningOfSelection];

    NSColor *selectedColor = [self _selectedColor];

    NSRange selectedRange =
            (textView == nil) ? NSMakeRange(0, 0) : [textView selectedRange];

    NSRange characterRange = [self characterRangeForGlyphRange: range
                                              actualGlyphRange: NULL];

    NSRange intersectRange = NSIntersectionRange(selectedRange, characterRange);

    NSColor *color = NSForegroundColorAttributeInDictionary(attributes);

    NSTextAttachment *attachment =
            [attributes objectForKey: NSAttachmentAttributeName];

    // Get the location of the fragment
    NSPoint point = fragment->location;

    BOOL isFlipped = _UIFoundationCurrentContextIsFlipped();

    // Correct its y-coord depending on the flippedness of the context
    if (!isFlipped) {
        point.y = usedHeight - point.y;
    }

    // Offset the point by the origin of the text container and the subRange
    // offset - we break fragments up because of the temp attributes...
    point.x += origin.x + xOffset;
    point.y += origin.y;

    /* A line containing an attachment taller than the text gets a taller line fragment, but the
     * baseline stayed where the font put it, so the attachment -- which sits on the baseline and
     * extends upward -- had nowhere to go and was cut off at the fragment's top edge (measured:
     * fragment y=0..24 with the baseline at y=11, so a 24pt attachment started at y=-13). Push the
     * baseline down to the bottom of the taller fragment. For a line with no tall attachment the
     * fragment is exactly the line height of its tallest font and `extra` is zero, so ordinary text
     * is not moved at all. The amount is measured against the tallest font of the whole line
     * fragment, not the font of this run: every run of the line shares the line's baseline, and a
     * run in a font with a shorter line height than another font in the same line (a fallback font
     * for symbols, say) must not be pushed below it. */
    NSFont *runFont = NSFontAttributeInDictionary(attributes);
    if (runFont != nil) {
        NSRange fragmentGlyphs;
        NSRect lineFragment =
            [self lineFragmentRectForGlyphAtIndex: range.location effectiveRange: &fragmentGlyphs];
        CGFloat fontHeight = ceilf([runFont defaultLineHeightForFont]);
        // The typesetter creates one fragment per run of one font, so widen the range to every
        // fragment of the same line: the same container, the same top and the same height.
        NSUInteger glyphCount = [self numberOfGlyphs];
        NSTextContainer *lineContainer = [self textContainerForGlyphAtIndex: range.location
                                                             effectiveRange: NULL];
        while (fragmentGlyphs.location > 0) {
            NSRange previous;
            NSRect rect = [self lineFragmentRectForGlyphAtIndex: fragmentGlyphs.location - 1
                                                 effectiveRange: &previous];
            if (rect.origin.y != lineFragment.origin.y || rect.size.height != lineFragment.size.height ||
                [self textContainerForGlyphAtIndex: previous.location effectiveRange: NULL] != lineContainer)
                break;
            fragmentGlyphs = NSUnionRange(fragmentGlyphs, previous);
        }
        while (NSMaxRange(fragmentGlyphs) < glyphCount) {
            NSRange next;
            NSRect rect = [self lineFragmentRectForGlyphAtIndex: NSMaxRange(fragmentGlyphs)
                                                 effectiveRange: &next];
            if (rect.origin.y != lineFragment.origin.y || rect.size.height != lineFragment.size.height ||
                [self textContainerForGlyphAtIndex: next.location effectiveRange: NULL] != lineContainer)
                break;
            fragmentGlyphs = NSUnionRange(fragmentGlyphs, next);
        }
        NSRange fragmentChars = [self characterRangeForGlyphRange: fragmentGlyphs
                                                 actualGlyphRange: NULL];
        NSTextStorage *storage = [self textStorage];
        NSUInteger charIndex = fragmentChars.location;
        while (storage != nil && charIndex < NSMaxRange(fragmentChars) &&
               charIndex < [storage length]) {
            NSRange fontRange;
            NSFont *lineFont = [storage attribute: NSFontAttributeName
                                          atIndex: charIndex
                            longestEffectiveRange: &fontRange
                                          inRange: fragmentChars];
            if (lineFont != nil)
                fontHeight = MAX(fontHeight, ceilf([lineFont defaultLineHeightForFont]));
            charIndex = NSMaxRange(fontRange);
        }
        CGFloat extra = lineFragment.size.height - fontHeight;
        if (extra > 0)
            point.y += isFlipped ? extra : -extra;
    }

    if (attachment != nil) {
        // Draw the attachment at the calculated point
        [self _drawAttachment: attachment
                atCharacterIndex: characterRange.location
                         atPoint: point];

    } else {

        // Load the glyphs for this run
        NSGlyph glyphs[range.length];
        NSUInteger glyphsLength;

        glyphsLength = [self getOrderedGlyphs: glyphs
                                        range: range
                                    baseLevel: 0
                                        order: NULL];

        // Prepare the font - we're within an attribute run - so this font will
        // be good for this entire range
        NSFont *font = [self _layoutFontForFont: NSFontAttributeInDictionary(attributes)];
        // Sets the font, its size and the text matrix in the current context.
        [font set];

        // We intersect with the selection so we apparently need to tread
        // carefully...
        if (intersectRange.length > 0) {
            BOOL ltor = fragment->leftToRight;

            if (ltor == NO) {
                // Mirror the selection since the glyphs order is Right-to-Left
                NSUInteger distanceToEnd =
                        NSMaxRange(characterRange) - NSMaxRange(intersectRange);
                intersectRange.location =
                        characterRange.location + distanceToEnd;
            }

            NSGlyph previousGlyph = NSNullGlyph;
            CGFloat partWidth = 0;
            NSUInteger i = 0;
            NSUInteger location = range.location;

            // We're going to identify runs of glyphs (unselected and selected)
            for (i = 0; location <= NSMaxRange(range); i++, location++) {

                NSUInteger offset = 0;
                NSUInteger length = 0;
                BOOL showGlyphs = NO;

                NSGlyph glyph = (location < NSMaxRange(range)) ? glyphs[i]
                                                               : NSNullGlyph;

                // Glyph is invisible so just translate it to a Null glyph
                if (glyph == NSControlGlyph) {
                    glyph = NSNullGlyph;
                }

                // We've found the start of the selected range - so we need to
                // draw the glyphs we've processed so far
                if (location == intersectRange.location &&
                    location > range.location) {
                    [color setFill];

                    offset = 0;

                    // length is the difference between the two locations
                    length = intersectRange.location - range.location;

                    showGlyphs = YES;
                }

                // We're at the end of the selected range intersection so make
                // sure the glyphs are drawn against the selectedColor
                // background
                else if (location == NSMaxRange(intersectRange)) {
                    [selectedColor setFill];

                    offset = intersectRange.location - range.location;

                    // The length is simply the length of the intersecting range
                    length = intersectRange.length;

                    showGlyphs = YES;
                }

                // We're at the end of our fragment range so switch back to the
                // regular background color... and show the remaining glyphs
                else if (location == NSMaxRange(range)) {
                    [color setFill];

                    offset = NSMaxRange(intersectRange) - range.location;

                    // the length is the difference between the end of the full
                    // range and the end of the intersection range
                    length = NSMaxRange(range) - NSMaxRange(intersectRange);

                    showGlyphs = YES;
                }

                if (showGlyphs) {
                    // Show the range of glyphs specifed by offset and length
                    NSRange subRange =
                            NSMakeRange(range.location + offset, length);

                    [self _drawGlyphs: glyphs + offset
                                    length: length
                                     range: subRange
                                   atPoint: point
                               inContainer: container
                            withAttributes: attributes
                                    origin: origin];

                    // And make sure we know where we are witin the fragment.
                    point.x += partWidth;
                    partWidth = 0;
                }

                // A boundary draws the preceding run. Advance past its glyphs
                // before accounting for the glyph at this boundary.
                BOOL ignore = NO;
                partWidth += [font positionOfGlyph: glyph
                                     precededByGlyph: previousGlyph
                                           isNominal: &ignore]
                                     .x;
                previousGlyph = glyph;
            }
        } else {
            // No intersection with selection so we can just process the whole
            // thing.
            [color setFill];
            NSGlyph previousGlyph = NSNullGlyph;

            CGFloat partWidth = 0;
            BOOL ignore = NO;

            for (NSUInteger i = 0; i <= range.length; i++) {
                NSGlyph glyph = i < range.length ? glyphs[i] : NSNullGlyph;

                // Glyph is invisible so just translate it to a Null glyph
                if (glyph == NSControlGlyph) {
                    glyph = NSNullGlyph;
                }
                partWidth += [font positionOfGlyph: glyph
                                     precededByGlyph: previousGlyph
                                           isNominal: &ignore]
                                     .x;
                previousGlyph = glyph;
            }

            [self _drawGlyphs: glyphs
                            length: glyphsLength
                             range: range
                           atPoint: point
                       inContainer: container
                    withAttributes: attributes
                            origin: origin];
            point.x += partWidth;
        }
    }
    // Return the distance travelled relative to the fragment origin (we added
    // these on at the start of this method)
    CGFloat newXOffset = point.x - (origin.x + fragment->location.x);
    return newXOffset;
}

- (void) drawGlyphsForGlyphRange: (NSRange) glyphRange
                         atPoint: (NSPoint) origin
{
    NSTextView *textView = [self textViewForBeginningOfSelection];
    NSRange selectedRange =
            (textView == nil) ? NSMakeRange(0, 0) : [textView selectedRange];

    glyphRange = [self validateGlyphsAndLayoutForGlyphRange: glyphRange];

    NSTextContainer *container =
            [self textContainerForGlyphAtIndex: glyphRange.location
                                effectiveRange: NULL];
    if (container == nil) {
        // Not sure if this is ever a good thing - but it's certainly a bad
        // thing and if there's no container within which to layout - we're
        // done.
        return;
    }
    CGFloat usedHeight = [self usedRectForTextContainer: container].size.height;

    NSRangeEnumerator state = NSRangeEntryEnumerator(_glyphFragments);
    NSRange range;
    NSGlyphFragment *fragment;

    BOOL checkTemporaryAttributesUsage =
            [_delegate respondsToSelector: @selector
                       (layoutManager:
                               shouldUseTemporaryAttributes:forDrawingToScreen
                                                           :atCharacterIndex
                                                           :effectiveRange:)];

    // Iterate over the glyph fragments (which identify runs of common
    // attributes)
    while (NSNextRangeEnumeratorEntry(&state, &range, (void **) &fragment)) {

#if DEBUG_LM_DRAWGLYPHSFORGLYPHRANGE
        NSLog(@"    looking at range: %@", NSStringFromRange(range));
#endif
        // Reset the offset for this fragment
        CGFloat glyphXOffset = 0;

        // Find out if we're within the range to be drawn
        NSRange intersect = NSIntersectionRange(range, glyphRange);

        if (intersect.length > 0) {
            // We don't care about the actual range or effective range here
            // because we know we're within a glyph fragment which means that
            // the character range and attributes are already limited correctly.
            NSRange characterRange =
                    [self characterRangeForGlyphRange: intersect
                                     actualGlyphRange: NULL];

            NSDictionary *attributes =
                    [_textStorage attributesAtIndex: characterRange.location
                                     effectiveRange: NULL];

#if DEBUG_LM_DRAWGLYPHSFORGLYPHRANGE
            NSLog(@"    characterRange: %@", NSStringFromRange(characterRange));
            NSLog(@"    attributes: %@", attributes);
#endif

            BOOL tempAttributesInRange = NO;
            NSUInteger tempAttributesIndex = NSNotFound;

            if (NSCountRangeEntries(_rangeToTemporaryAttributes) > 0) {
                // But we do have to worry about the temporary attributes so
                // let's do a quick check with the character range and find out
                // if we have any temp attributes to be concerned about
                NSUInteger index = characterRange.location;
                NSRange tempRange = NSMakeRange(0, 0);
                while (tempAttributesInRange == NO &&
                       index < NSMaxRange(characterRange)) {
                    if (NSRangeEntryAtIndex(_rangeToTemporaryAttributes, index,
                                            &tempRange) != nil) {
                        tempAttributesInRange = YES;
                        tempAttributesIndex = index;
                    } else {
                        index++;
                    }
                }
            }

#if DEBUG_LM_DRAWGLYPHSFORGLYPHRANGE
            NSLog(@"    temporaryAttributesInRange: %@",
                  tempAttributesInRange ? @"YES" : @"NO");
            if (tempAttributesInRange) {
                NSRange tempRange;
                NSDictionary *tmpAttrs = [self
                        temporaryAttributesAtCharacterIndex: tempAttributesIndex
                                             effectiveRange: &tempRange];
                NSLog(@"     temporaryAttributes: %@", tmpAttrs);
            }
#endif

            if (tempAttributesInRange) {

                // Ok - so we've got to proceed with caution - there are temp
                // attributes
                NSUInteger length = 0;
                NSUInteger offset = characterRange.location;

                // Iterate over the characters in the range
                for (NSUInteger charIndex = characterRange.location;
                     charIndex < NSMaxRange(characterRange);
                     charIndex++, length++) {

                    // We've found some temp attributes
                    NSRange tempRange = NSMakeRange(0, 0);
                    // Make sure we don't go beyond the remaining range of the
                    // current fragment
                    NSRange remainingRange = NSMakeRange(
                            charIndex, NSMaxRange(characterRange) - charIndex);
                    if (NSRangeEntryAtIndex(_rangeToTemporaryAttributes,
                                            charIndex, &tempRange) != nil &&
                        NSIntersectionRange(remainingRange, tempRange).length >
                                0) {

                        if (length > 0) {
                            // draw the glyphs that we've encountered so far (if
                            // any)
                            NSRange subRange = NSMakeRange(offset, length);
                            subRange =
                                    [self glyphRangeForCharacterRange: subRange
                                                 actualCharacterRange: NULL];
                            glyphXOffset = [self
                                    _drawGlyphsForSubGlyphRange: subRange
                                                    forFragment: fragment
                                                        atPoint: origin
                                                subRangeXOffset: glyphXOffset
                                                    inContainer: container
                                                 withAttributes: attributes
                                                     usedHeight: usedHeight];
                        }

                        NSDictionary *tempAttrs = [self
                                temporaryAttributesAtCharacterIndex: charIndex
                                                     effectiveRange: &
                                                                     tempRange];

                        if (checkTemporaryAttributesUsage) {
                            tempAttrs = [_delegate layoutManager: self
                                    shouldUseTemporaryAttributes: tempAttrs
                                              forDrawingToScreen:
                                                      _UIFoundationCurrentContextIsDrawingToScreen()
                                                atCharacterIndex: charIndex
                                                  effectiveRange: &tempRange];
                        }

                        tempRange =
                                NSIntersectionRange(remainingRange, tempRange);

                        // Merge the temp attributes with the permanent
                        // attributes
                        NSMutableDictionary *mergedAttrs =
                                [[attributes mutableCopy] autorelease];
                        [mergedAttrs addEntriesFromDictionary: tempAttrs];

                        NSRange tempGlyphRange =
                                [self glyphRangeForCharacterRange: tempRange
                                             actualCharacterRange: NULL];

                        glyphXOffset = [self
                                _drawGlyphsForSubGlyphRange: tempGlyphRange
                                                forFragment: fragment
                                                    atPoint: origin
                                            subRangeXOffset: glyphXOffset
                                                inContainer: container
                                             withAttributes: mergedAttrs
                                                 usedHeight: usedHeight];

                        // And reset ready for the next run
                        offset = NSMaxRange(tempRange);
                        charIndex =
                                offset -
                                1; // Make sure we leap over the temp range (one
                                   // off to accommodate the auto-increment)

                        length = -1; // accommodate the auto-increment
                    }
                }
                if (offset < NSMaxRange(characterRange) && length > 0) {
                    // We've got some glyphs left over - so draw them too
                    NSRange subRange = NSMakeRange(offset, length);
                    subRange = [self glyphRangeForCharacterRange: subRange
                                            actualCharacterRange: NULL];
                    [self _drawGlyphsForSubGlyphRange: subRange
                                          forFragment: fragment
                                              atPoint: origin
                                      subRangeXOffset: glyphXOffset
                                          inContainer: container
                                       withAttributes: attributes
                                           usedHeight: usedHeight];
                }

            } else {
                // We don't have any temp attributes - so draw this sub range of
                // glyphs
                [self _drawGlyphsForSubGlyphRange: intersect
                                      forFragment: fragment
                                          atPoint: origin
                                  subRangeXOffset: 0
                                      inContainer: container
                                   withAttributes: attributes
                                       usedHeight: usedHeight];
            }
        }
    }
}

// dwy
- (NSRange) _softLineRangeForCharacterAtIndex: (NSUInteger) location {
    NSRange result;
    NSInteger i, j;
    CGFloat origin;
    NSGlyphFragment *fragment;

    NSUInteger length = [[_textStorage string] length];
    if (length == 0)
        return NSMakeRange(0, 0);
    if (location >= length)
        location = length - 1;

    // The insertion point at EOF belongs to the last glyph's line.
    result = NSMakeRange(location, 0);
    result = [self glyphRangeForCharacterRange: result
                          actualCharacterRange: NULL];

    fragment =
            NSRangeEntryAtIndex(self->_glyphFragments, result.location, NULL);
    if (fragment == NULL)
        return result;

    origin = fragment->location.y;

    i = result.location;
    j = result.location;
    while ((fragment =
                    NSRangeEntryAtIndex(self->_glyphFragments, i - 1, NULL))) {
        if (fragment->location.y != origin)
            break;
        result.location = i;
        i--;
    }

    result.location = i;
    while ((fragment = NSRangeEntryAtIndex(self->_glyphFragments, j, NULL))) {
        if (fragment->location.y != origin)
            break;
        j++;
    }

    result.length = j - i;

#if 0    
// broken for empty lines
    // word-break fixup; best effort; produces some strange effects when a single word is wider than the view
    if ([[NSCharacterSet whitespaceAndNewlineCharacterSet] characterIsMember:[[_textStorage string] characterAtIndex:NSMaxRange(result)-1]])
        result.length--;
#endif

    return [self characterRangeForGlyphRange: result actualGlyphRange: NULL];
}

- (CGFloat) defaultLineHeightForFont: (NSFont *) font {
    return [font defaultLineHeightForFont];
}

- (CGFloat) defaultBaselineOffsetForFont: (NSFont *) font {
    return [font ascender];
}

- (NSDictionary *)
        temporaryAttributesAtCharacterIndex: (NSUInteger) charIndex
                             effectiveRange: (NSRangePointer) effectiveCharRange
{
    NSDictionary *result;

    if (charIndex >= [_textStorage length]) {
        NSRaiseException(NSRangeException, self, _cmd,
                         @"index %d beyond length %d", charIndex,
                         [_textStorage length]);
    }

    if ((result = NSRangeEntryAtIndex(_rangeToTemporaryAttributes, charIndex,
                                      effectiveCharRange)) == nil) {
        result = [NSDictionary dictionary];
    }

    // The string could be being mutated so these attributes could disappear on
    // an unwary caller
    return [[result retain] autorelease];
}

- (void) setTemporaryAttributes: (NSDictionary *) attrs
              forCharacterRange: (NSRange) charRange
{
    if (attrs == nil) {
        attrs = [NSDictionary dictionary];
    } else {
        attrs = [[attrs copy] autorelease];
    }

    if ([_textStorage length] == 0) {
        NSResetRangeEntries(_rangeToTemporaryAttributes);
    } else if (charRange.length > 0) {
        // Make sure we don't go beyond the actual text storage range
        NSRange intersect = NSIntersectionRange(
                NSMakeRange(0, [_textStorage length]), charRange);

        NSRangeEntriesDivideAndConquer(_rangeToTemporaryAttributes, intersect);
        NSRangeEntryInsert(_rangeToTemporaryAttributes, intersect, attrs);
    }

    NSRangeEntriesVerify(_rangeToTemporaryAttributes, [_textStorage length]);

    [self invalidateDisplayForCharacterRange: charRange];
}

- (void) addTemporaryAttributes: (NSDictionary *) attrs
              forCharacterRange: (NSRange) charRange
{
    NSUInteger location = charRange.location;
    NSUInteger limit = NSMaxRange(charRange);
    // clip the limit - Cocoa doesn't seem to complain when we exceed the text
    // storage range
    limit = MIN(limit, [_textStorage length]);

    while (location < limit) {
        NSRange effectiveRange;
        NSMutableDictionary *modify =
                [[[self temporaryAttributesAtCharacterIndex: location
                                             effectiveRange: &effectiveRange]
                        mutableCopy] autorelease];
        NSRange replace;

        [modify addEntriesFromDictionary: attrs];

        replace.location = MAX(location, effectiveRange.location);
        replace.length =
                MIN(NSMaxRange(charRange), NSMaxRange(effectiveRange)) -
                replace.location;

        [self setTemporaryAttributes: modify forCharacterRange: replace];

        location = NSMaxRange(replace);
    }
}

- (void) removeTemporaryAttribute: (NSString *) attrName
                forCharacterRange: (NSRange) charRange
{
    if ([_textStorage length] == 0) {
        // Nothing to do
        return;
    }

    NSUInteger location = charRange.location;

    if (location >= [_textStorage length]) {
        NSRaiseException(NSRangeException, self, _cmd,
                         @"index %d beyond length %d", location,
                         [_textStorage length]);
    }

    NSUInteger limit = MIN(NSMaxRange(charRange), [_textStorage length]);

    while (location < limit) {
        NSRange effectiveRange;
        NSDictionary *check =
                [self temporaryAttributesAtCharacterIndex: location
                                           effectiveRange: &effectiveRange];
        NSRange replace;

        replace.location = location;
        replace.length =
                MIN(NSMaxRange(charRange), NSMaxRange(effectiveRange)) -
                location;

        if ([check objectForKey: attrName] != nil) {
            NSMutableDictionary *modify = [[check mutableCopy] autorelease];

            [modify removeObjectForKey: attrName];

            [self setTemporaryAttributes: modify forCharacterRange: replace];
        }

        location = NSMaxRange(replace);
    }
}

- (id) temporaryAttribute: (NSString *) attrName
         atCharacterIndex: (NSUInteger) location
           effectiveRange: (NSRangePointer) range
{
    return [[self temporaryAttributesAtCharacterIndex: location
                                       effectiveRange: range]
            objectForKey: attrName];
}

- (id) temporaryAttribute: (NSString *) attrName
             atCharacterIndex: (NSUInteger) location
        longestEffectiveRange: (NSRangePointer) range
                      inRange: (NSRange) rangeLimit
{
    id result = [self temporaryAttribute: attrName
                        atCharacterIndex: location
                          effectiveRange: range];
    if (range) {
        // Check if we can expand the range

        // Check if we can expand it before the found range
        NSRange effectiveRange;
        while (range->location > rangeLimit.location) {
            id attr = [self temporaryAttribute: attrName
                              atCharacterIndex: range->location - 1
                                effectiveRange: &effectiveRange];
            if (attr == result || [attr isEqual: result]) {
                // Expand the found range location
                range->length = NSMaxRange(*range) - effectiveRange.location;
                range->location = effectiveRange.location;
            } else {
                break;
            }
        }
        // Check if we can expand it after the found range
        while (NSMaxRange(*range) < NSMaxRange(rangeLimit)) {
            id attr = [self temporaryAttribute: attrName
                              atCharacterIndex: NSMaxRange(*range)
                                effectiveRange: &effectiveRange];
            if (attr == result || [attr isEqual: result]) {
                // Expand the found range length
                range->length = NSMaxRange(effectiveRange) - range->location;
            } else {
                break;
            }
        }
        // Ensure we don't go outside of the rangeLimit
        *range = NSIntersectionRange(*range, rangeLimit);
    }
    return result;
}

- (NSDictionary *) temporaryAttributesAtCharacterIndex: (NSUInteger) location
                                 longestEffectiveRange: (NSRangePointer) range
                                               inRange: (NSRange) rangeLimit
{
    id result = [self temporaryAttributesAtCharacterIndex: location
                                           effectiveRange: range];
    if (range) {
        // Check if we can expand the range

        // Check if we can expand it before the found range
        NSRange effectiveRange;
        while (range->location > rangeLimit.location) {
            id attr = [self
                    temporaryAttributesAtCharacterIndex: range->location - 1
                                         effectiveRange: &effectiveRange];
            if (attr == result || [attr isEqual: result]) {
                // Expand the found range location
                range->length = NSMaxRange(*range) - effectiveRange.location;
                range->location = effectiveRange.location;
            } else {
                break;
            }
        }
        // Check if we can expand it after the found range
        while (NSMaxRange(*range) < NSMaxRange(rangeLimit)) {
            id attr = [self
                    temporaryAttributesAtCharacterIndex: NSMaxRange(*range)
                                         effectiveRange: &effectiveRange];
            if (attr == result || [attr isEqual: result]) {
                // Expand the found range length
                range->length = NSMaxRange(effectiveRange) - range->location;
            } else {
                break;
            }
        }
        // Ensure we don't go outside of the rangeLimit
        *range = NSIntersectionRange(*range, rangeLimit);
    }
    return result;
}

- (void) addTemporaryAttribute: (NSString *) attrName
                         value: (id) value
             forCharacterRange: (NSRange) charRange
{
    [self addTemporaryAttributes: [NSDictionary dictionaryWithObject: value
                                                              forKey: attrName]
               forCharacterRange: charRange];
}

// Stored only: layout is always contiguous.
- (void) setAllowsNonContiguousLayout: (BOOL) value {
    _allowsNonContiguousLayout = value;
}
@end
