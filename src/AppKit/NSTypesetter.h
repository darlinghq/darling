#import <CoreFoundation/CFCharacterSet.h>
#import <Foundation/NSObject.h>
#import <AppKit/NSLayoutManager.h>
#import <AppKit/NSParagraphStyle.h>

enum {
    NSTypesetterZeroAdvancementAction = (1 << 0),
    NSTypesetterWhitespaceAction = (1 << 1),
    NSTypesetterHorizontalTabAction = (1 << 2),
    NSTypesetterLineBreakAction = (1 << 3),
    NSTypesetterParagraphBreakAction = (1 << 4),
    NSTypesetterContainerBreakAction = (1 << 5)
};
typedef NSUInteger NSTypesetterControlCharacterAction;

@interface NSTypesetter : NSObject {
#if __LP64__
    void *_reserved;
#else
    unsigned _reserved[2];
#endif
}





- (BOOL)usesFontLeading;
- (void)setUsesFontLeading:(BOOL)flag;



- (NSTypesetterBehavior)typesetterBehavior;
- (void)setTypesetterBehavior:(NSTypesetterBehavior)behavior;



- (float)hyphenationFactor;
- (void)setHyphenationFactor:(float)factor;




- (CGFloat)lineFragmentPadding;
- (void)setLineFragmentPadding:(CGFloat)padding;


- (NSFont *)substituteFontForFont:(NSFont *)originalFont;




- (NSTextTab *)textTabForGlyphLocation:(CGFloat)glyphLocation writingDirection:(NSWritingDirection)direction maxLocation:(CGFloat)maxLocation;




- (BOOL)bidiProcessingEnabled;
- (void)setBidiProcessingEnabled:(BOOL)flag;


- (void)setAttributedString:(NSAttributedString *)attrString;
- (NSAttributedString *)attributedString;
- (void)setParagraphGlyphRange:(NSRange)paragraphRange separatorGlyphRange:(NSRange)paragraphSeparatorRange;
- (NSRange)paragraphGlyphRange;
- (NSRange)paragraphSeparatorGlyphRange;
- (NSRange)paragraphCharacterRange;
- (NSRange)paragraphSeparatorCharacterRange;




- (NSUInteger)layoutParagraphAtPoint:(NSPointPointer)lineFragmentOrigin;


- (void)beginParagraph;
- (void)endParagraph;
- (void)beginLineWithGlyphAtIndex:(NSUInteger)glyphIndex;
- (void)endLineWithGlyphRange:(NSRange)lineGlyphRange;


- (CGFloat)lineSpacingAfterGlyphAtIndex:(NSUInteger)glyphIndex withProposedLineFragmentRect:(NSRect)rect;
- (CGFloat)paragraphSpacingBeforeGlyphAtIndex:(NSUInteger)glyphIndex withProposedLineFragmentRect:(NSRect)rect;
- (CGFloat)paragraphSpacingAfterGlyphAtIndex:(NSUInteger)glyphIndex withProposedLineFragmentRect:(NSRect)rect;




- (void)getLineFragmentRect:(NSRectPointer)lineFragmentRect usedRect:(NSRectPointer)lineFragmentUsedRect forParagraphSeparatorGlyphRange:(NSRange)paragraphSeparatorGlyphRange atProposedOrigin:(NSPoint)lineOrigin;




- (NSDictionary *)attributesForExtraLineFragment;




- (NSTypesetterControlCharacterAction)actionForControlCharacterAtIndex:(NSUInteger)charIndex;



- (NSLayoutManager *)layoutManager;
- (NSArray *)textContainers;
- (NSTextContainer *)currentTextContainer;
- (NSParagraphStyle *)currentParagraphStyle;



- (void)setHardInvalidation:(BOOL)flag forGlyphRange:(NSRange)glyphRange;



- (void)layoutGlyphsInLayoutManager:(NSLayoutManager *)layoutManager startingAtGlyphIndex:(NSUInteger)startGlyphIndex maxNumberOfLineFragments:(NSUInteger)maxNumLines nextGlyphIndex:(NSUInteger *)nextGlyph;



- (NSRange)layoutCharactersInRange:(NSRange)characterRange forLayoutManager:(NSLayoutManager *)layoutManager maximumNumberOfLineFragments:(NSUInteger)maxNumLines;



+ (NSSize)printingAdjustmentInLayoutManager:(NSLayoutManager *)layoutMgr forNominallySpacedGlyphRange:(NSRange)nominallySpacedGlyphsRange packedGlyphs:(const unsigned char *)packedGlyphs count:(NSUInteger)packedGlyphsCount;

- (CGFloat)baselineOffsetInLayoutManager:(NSLayoutManager *)layoutMgr glyphIndex:(NSUInteger)glyphIndex;



+ (id)sharedSystemTypesetter;
+ (id)sharedSystemTypesetterForBehavior:(NSTypesetterBehavior)theBehavior;
+ (NSTypesetterBehavior)defaultTypesetterBehavior;
@end


@interface NSTypesetter (NSLayoutPhaseInterface)

- (void)willSetLineFragmentRect:(NSRectPointer)lineRect forGlyphRange:(NSRange)glyphRange usedRect:(NSRectPointer)usedRect baselineOffset:(CGFloat *)baselineOffset;

- (BOOL)shouldBreakLineByWordBeforeCharacterAtIndex:(NSUInteger)charIndex;
- (BOOL)shouldBreakLineByHyphenatingBeforeCharacterAtIndex:(NSUInteger)charIndex;

- (float)hyphenationFactorForGlyphAtIndex:(NSUInteger)glyphIndex;
- (UTF32Char)hyphenCharacterForGlyphAtIndex:(NSUInteger)glyphIndex;

- (NSRect)boundingBoxForControlGlyphAtIndex:(NSUInteger)glyphIndex forTextContainer:(NSTextContainer *)textContainer proposedLineFragment:(NSRect)proposedRect glyphPosition:(NSPoint)glyphPosition characterIndex:(NSUInteger)charIndex;
@end


@interface NSTypesetter (NSGlyphStorageInterface)

- (NSRange)characterRangeForGlyphRange:(NSRange)glyphRange actualGlyphRange:(NSRangePointer)actualGlyphRange;
- (NSRange)glyphRangeForCharacterRange:(NSRange)charRange actualCharacterRange:(NSRangePointer)actualCharRange;


- (NSUInteger)getGlyphsInRange:(NSRange)glyphsRange glyphs:(NSGlyph *)glyphBuffer characterIndexes:(NSUInteger *)charIndexBuffer glyphInscriptions:(NSGlyphInscription *)inscribeBuffer elasticBits:(BOOL *)elasticBuffer bidiLevels:(unsigned char *)bidiLevelBuffer;




- (void)getLineFragmentRect:(NSRectPointer)lineFragmentRect usedRect:(NSRectPointer)lineFragmentUsedRect remainingRect:(NSRectPointer)remainingRect forStartingGlyphAtIndex:(NSUInteger)startingGlyphIndex proposedRect:(NSRect)proposedRect lineSpacing:(CGFloat)lineSpacing paragraphSpacingBefore:(CGFloat)paragraphSpacingBefore paragraphSpacingAfter:(CGFloat)paragraphSpacingAfter;


- (void)setLineFragmentRect:(NSRect)fragmentRect forGlyphRange:(NSRange)glyphRange usedRect:(NSRect)usedRect baselineOffset:(CGFloat)baselineOffset;
- (void)substituteGlyphsInRange:(NSRange)glyphRange withGlyphs:(NSGlyph *)glyphs;
- (void)insertGlyph:(NSGlyph)glyph atGlyphIndex:(NSUInteger)glyphIndex characterIndex:(NSUInteger)characterIndex;
- (void)deleteGlyphsInRange:(NSRange)glyphRange;
- (void)setNotShownAttribute:(BOOL)flag forGlyphRange:(NSRange)glyphRange;
- (void)setDrawsOutsideLineFragment:(BOOL)flag forGlyphRange:(NSRange)glyphRange;
- (void)setLocation:(NSPoint)location withAdvancements:(const CGFloat *)advancements forStartOfGlyphRange:(NSRange)glyphRange;
- (void)setAttachmentSize:(NSSize)attachmentSize forGlyphRange:(NSRange)glyphRange;
- (void)setBidiLevels:(const uint8_t *)levels forGlyphRange:(NSRange)glyphRange;
@end

#if !__LP64__ && MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_4
#import <AppKit/NSSimpleHorizontalTypesetter.h>
#endif
