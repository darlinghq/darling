#import "NSTypesetter_concrete.h"
#import <AppKit/NSAttributedString.h>
#import <AppKit/NSParagraphStyle.h>
#import <AppKit/NSRaise.h>
#import <AppKit/NSTextTab.h>
#import <AppKit/NSTypesetter.h>

@class NSAttributedString;

@implementation NSTypesetter

+ allocWithZone: (NSZone *) zone {
    if (self == [NSTypesetter class])
        return NSAllocateObject([NSTypesetter_concrete class], 0, zone);

    return [super allocWithZone: zone];
}

+ (NSTypesetterBehavior) defaultTypesetterBehavior {
    return NSTypesetterLatestBehavior;
}

+ (NSSize) printingAdjustmentInLayoutManager: (NSLayoutManager *) layoutManager
                forNominallySpacedGlyphRange: (NSRange) glyphRange
                                packedGlyphs:
                                        (const unsigned char *) packedGlyphs
                                       count: (NSUInteger) count
{
    return NSMakeSize(0, 0);
}

+ sharedSystemTypesetterForBehavior: (NSTypesetterBehavior) behavior {
    return [[[self alloc] init] autorelease];
}

+ sharedSystemTypesetter {
    return [self sharedSystemTypesetterForBehavior:
                         [self defaultTypesetterBehavior]];
}

- (void) dealloc {
    [_layoutManager release];
    _textContainers = nil;
    [_attributedString release];
    _string = nil;
    [super dealloc];
}

- (NSTypesetterBehavior) typesetterBehavior {
    return _behavior;
}

- (CGFloat) hyphenationFactor {
    return _hyphenationFactor;
}

- (CGFloat) lineFragmentPadding {
    return _lineFragmentPadding;
}

- (BOOL) usesFontLeading {
    return _usesFontLeading;
}

- (BOOL) bidiProcessingEnabled {
    return _bidiProcessingEnabled;
}

- (void) setTypesetterBehavior: (NSTypesetterBehavior) behavior {
    _behavior = behavior;
}

- (void) setHyphenationFactor: (CGFloat) factor {
    _hyphenationFactor = factor;
}

- (void) setLineFragmentPadding: (CGFloat) padding {
    _lineFragmentPadding = padding;
}

- (void) setUsesFontLeading: (BOOL) flag {
    _usesFontLeading = flag;
}

- (void) setBidiProcessingEnabled: (BOOL) flag {
    _bidiProcessingEnabled = flag;
}

- (NSRange) characterRangeForGlyphRange: (NSRange) glyphRange
                       actualGlyphRange: (NSRange *) actualGlyphRange
{
    return [_layoutManager characterRangeForGlyphRange: glyphRange
                                      actualGlyphRange: actualGlyphRange];
}

- (NSRange) glyphRangeForCharacterRange: (NSRange) characterRange
                   actualCharacterRange: (NSRange *) actualCharacterRange
{
    return [_layoutManager glyphRangeForCharacterRange: characterRange
                                  actualCharacterRange: actualCharacterRange];
}

// Limitation: glyphs map 1:1 to characters here, which is right for simple
// scripts; combining characters, surrogate pairs and ligatures are not mapped
// through the font.
- (NSUInteger) getGlyphsInRange: (NSRange) glyphRange
                         glyphs: (NSGlyph *) glyphs
               characterIndexes: (NSUInteger *) characterIndexes
              glyphInscriptions: (NSGlyphInscription *) glyphInscriptions
                    elasticBits: (BOOL *) elasticBits
                     bidiLevels: (unsigned char *) bidiLevels
{
    if (characterIndexes) {
        for (NSUInteger i = 0; i < glyphRange.length; ++i) {
            characterIndexes[i] = glyphRange.location + i;
        }
    }
    if (glyphInscriptions) {
        for (NSUInteger i = 0; i < glyphRange.length; ++i) {
            glyphInscriptions[i] = 0;
        }
    }
    if (elasticBits) {
        for (NSUInteger i = 0; i < glyphRange.length; ++i) {
            elasticBits[i] = NO;
        }
    }
    if (bidiLevels) {
        for (NSUInteger i = 0; i < glyphRange.length; ++i) {
            bidiLevels[i] = 0;
        }
    }
    return glyphRange.length;
}

- (void) getLineFragmentRect: (NSRect *) fragmentRect
                       usedRect: (NSRect *) usedRect
                  remainingRect: (NSRect *) remainingRect
        forStartingGlyphAtIndex: (NSUInteger) startingGlyphIndex
                   proposedRect: (NSRect) proposedRect
                    lineSpacing: (CGFloat) lineSpacing
         paragraphSpacingBefore: (CGFloat) paragraphSpacingBefore
          paragraphSpacingAfter: (CGFloat) paragraphSpacingAfter
{
    NSInvalidAbstractInvocation();
}

- (void) setLineFragmentRect: (NSRect) fragmentRect
               forGlyphRange: (NSRange) glyphRange
                    usedRect: (NSRect) usedRect
              baselineOffset: (CGFloat) baselineOffset
{
    [_layoutManager setLineFragmentRect: fragmentRect
                          forGlyphRange: glyphRange
                               usedRect: usedRect];
}

- (void) substituteGlyphsInRange: (NSRange) glyphRange
                      withGlyphs: (NSGlyph *) glyphs
{
    // do nothing
}

- (void) insertGlyph: (NSGlyph) glyph
          atGlyphIndex: (NSUInteger) glyphIndex
        characterIndex: (NSUInteger) characterIndex
{
    [_layoutManager insertGlyph: glyph
                   atGlyphIndex: glyphIndex
                 characterIndex: characterIndex];
}

- (void) deleteGlyphsInRange: (NSRange) glyphRange {
    [_layoutManager deleteGlyphsInRange: glyphRange];
}

- (void) setNotShownAttribute: (BOOL) flag forGlyphRange: (NSRange) range {
    NSInteger i, max = NSMaxRange(range);

    for (i = range.location; i < max; i++)
        [_layoutManager setNotShownAttribute: flag forGlyphAtIndex: i];
}

- (void) setDrawsOutsideLineFragment: (BOOL) flag
                       forGlyphRange: (NSRange) range
{
    int i, max = NSMaxRange(range);

    for (i = range.location; i < max; i++)
        [_layoutManager setDrawsOutsideLineFragment: flag forGlyphAtIndex: i];
}

- (void) setLocation: (NSPoint) location
            withAdvancements: (const CGFloat *) nominalAdvancements
        forStartOfGlyphRange: (NSRange) glyphRange
{
    [_layoutManager setLocation: location forStartOfGlyphRange: glyphRange];
}

- (void) setAttachmentSize: (NSSize) size forGlyphRange: (NSRange) glyphRange {
    [_layoutManager setAttachmentSize: size forGlyphRange: glyphRange];
}

- (void) setBidiLevels: (const unsigned char *) bidiLevels
         forGlyphRange: (NSRange) glyphRange
{

    // do nothing
}

- (void) willSetLineFragmentRect: (NSRect *) fragmentRect
                   forGlyphRange: (NSRange) glyphRange
                        usedRect: (NSRect *) usedRect
                  baselineOffset: (CGFloat *) baselineOffset
{
    // do nothing
}

- (BOOL) shouldBreakLineByHyphenatingBeforeCharacterAtIndex:
        (NSUInteger) characterIndex
{
    NSInvalidAbstractInvocation();
    return NO;
}

- (BOOL) shouldBreakLineByWordBeforeCharacterAtIndex:
        (NSUInteger) characterIndex
{
    NSInvalidAbstractInvocation();
    return NO;
}

- (CGFloat) hyphenationFactorForGlyphAtIndex: (NSUInteger) glyphIndex {
    return 0.0;
}

- (unichar) hyphenCharacterForGlyphAtIndex: (NSUInteger) glyphIndex {
    return '-';
}

- (NSRect) boundingBoxForControlGlyphAtIndex: (NSUInteger) glyphIndex
                            forTextContainer: (NSTextContainer *) textContainer
                        proposedLineFragment: (NSRect) proposedRect
                               glyphPosition: (NSPoint) glyphPosition
                              characterIndex: (NSUInteger) characterIndex
{
    NSInvalidAbstractInvocation();
    return NSMakeRect(0, 0, 0, 0);
}

- (NSAttributedString *) attributedString {
    return _attributedString;
}

- (NSDictionary *) attributesForExtraLineFragment {
    NSInvalidAbstractInvocation();
    return nil;
}

- (NSLayoutManager *) layoutManager {
    return _layoutManager;
}

- (NSArray *) textContainers {
    return _textContainers;
}

- (NSTextContainer *) currentTextContainer {
    return _currentTextContainer;
}

- (NSParagraphStyle *) currentParagraphStyle {
    return _currentParagraphStyle;
}

- (NSRange) paragraphCharacterRange {
    NSInvalidAbstractInvocation();
    return NSMakeRange(0, 0);
}

- (NSRange) paragraphGlyphRange {
    NSInvalidAbstractInvocation();
    return NSMakeRange(0, 0);
}

- (NSRange) paragraphSeparatorCharacterRange {
    NSInvalidAbstractInvocation();
    return NSMakeRange(0, 0);
}

- (NSRange) paragraphSeparatorGlyphRange {
    NSInvalidAbstractInvocation();
    return NSMakeRange(0, 0);
}

- (NSTypesetterControlCharacterAction) actionForControlCharacterAtIndex:
        (NSUInteger) characterIndex
{
    unichar c = [[_attributedString string] characterAtIndex: characterIndex];
    if ([[NSCharacterSet newlineCharacterSet] characterIsMember: c]) {
        return NSTypesetterParagraphBreakAction;
    }
    switch (c) {
    case '\n':
        return NSTypesetterParagraphBreakAction;

    case '\t':
        return NSTypesetterHorizontalTabAction;

    case 0x200B:
        return NSTypesetterZeroAdvancementAction;

    default:
        return NSTypesetterWhitespaceAction;
    }
}

- (NSFont *) substituteFontForFont: (NSFont *) font {
    return font;
}

- (void) setAttributedString: (NSAttributedString *) text {
    text = [text retain];
    [_attributedString release];
    _attributedString = text;
    _string = [_attributedString string];
}

- (void) setHardInvalidation: (BOOL) invalidate
               forGlyphRange: (NSRange) glyphRange
{
    NSInvalidAbstractInvocation();
}

- (void) setParagraphGlyphRange: (NSRange) glyphRange
            separatorGlyphRange: (NSRange) separatorGlyphRange
{
    NSInvalidAbstractInvocation();
}

- (void) beginLineWithGlyphAtIndex: (NSUInteger) glyphIndex {
    NSInvalidAbstractInvocation();
}

- (void) endLineWithGlyphRange: (NSRange) glyphRange {
    NSInvalidAbstractInvocation();
}

- (void) beginParagraph {
    NSInvalidAbstractInvocation();
}

- (void) endParagraph {
    NSInvalidAbstractInvocation();
}

- (CGFloat) baselineOffsetInLayoutManager: (NSLayoutManager *) layoutManager
                               glyphIndex: (NSUInteger) glyphIndex
{
    NSInvalidAbstractInvocation();
    return 0;
}

- (NSTextTab *) textTabForGlyphLocation: (CGFloat) location
                       writingDirection: (NSWritingDirection) direction
                            maxLocation: (CGFloat) maxLocation
{
    NSArray *stops = [_currentParagraphStyle tabStops];
    NSInteger i, count = [stops count];

    for (i = 0; i < count; i++) {
        NSTextTab *tab = [stops objectAtIndex: i];
        CGFloat check = [tab location];

        if (check > maxLocation)
            break;

        if (check > location)
            return tab;
    }

    return nil;
}

- (void) getLineFragmentRect: (NSRect *) fragmentRect
                               usedRect: (NSRect *) usedRect
        forParagraphSeparatorGlyphRange: (NSRange) glyphRange
                       atProposedOrigin: (NSPoint) proposedOrigin
{
    NSInvalidAbstractInvocation();
}

- (NSParagraphStyle *) _paragraphStyleAfterGlyphIndex: (NSUInteger) glyphIndex {
    NSInvalidAbstractInvocation();
    return nil;
}

- (NSParagraphStyle *) _paragraphStyleBeforeGlyphIndex: (NSUInteger) glyphIndex
{
    NSInvalidAbstractInvocation();
    return nil;
}

- (CGFloat) lineSpacingAfterGlyphAtIndex: (NSUInteger) glyphIndex
            withProposedLineFragmentRect: (NSRect) rect
{
    return [[self _paragraphStyleAfterGlyphIndex: glyphIndex] lineSpacing];
}

- (CGFloat) paragraphSpacingAfterGlyphAtIndex: (NSUInteger) glyphIndex
                 withProposedLineFragmentRect: (NSRect) rect
{
    return [[self _paragraphStyleAfterGlyphIndex: glyphIndex] paragraphSpacing];
}

- (CGFloat) paragraphSpacingBeforeGlyphAtIndex: (NSUInteger) glyphIndex
                  withProposedLineFragmentRect: (NSRect) rect
{
    return [[self _paragraphStyleBeforeGlyphIndex: glyphIndex]
            paragraphSpacingBefore];
}

- (NSUInteger) layoutParagraphAtPoint: (NSPoint *) point {
    [self beginParagraph];

    NSInvalidAbstractInvocation();

    [self endParagraph];
    return 0;
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

#pragma mark NSGlyphStorage Protocol
- (NSUInteger) layoutOptions {
    return 0;
}

- (void) insertGlyphs: (const NSGlyph *) glyphs
                         length: (NSUInteger) length
        forStartingGlyphAtIndex: (NSUInteger) glyphIndex
                 characterIndex: (NSUInteger) characterIndex
{
    NSInvalidAbstractInvocation();
}

- (void) setIntAttribute: (NSInteger) intAttribute
                   value: (NSInteger) value
         forGlyphAtIndex: (NSUInteger) glyphIndex;
{
    NSInvalidAbstractInvocation();
}
@end
