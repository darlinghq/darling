#import <AppKit/NSTypesetter.h>
#import <AppKit/NSParagraphStyle.h>


@interface NSATSTypesetter : NSTypesetter {

    NSAttributedString *attributedString;

    NSRange paragraphGlyphRange;

    NSRange paragraphSeparatorGlyphRange;

    CGFloat lineFragmentPadding;


    NSLayoutManager *layoutManager;
    NSArray *textContainers;


    NSTextContainer *currentTextContainer;
    NSUInteger currentTextContainerIndex;
    NSSize currentTextContainerSize;


    NSParagraphStyle *currentParagraphStyle;

@private
    void *_atsReserved[8];
    id _private;
}


+ (id)sharedTypesetter;
@end

@interface NSATSTypesetter (NSPantherCompatibility)


- (NSRect)lineFragmentRectForProposedRect:(NSRect)proposedRect remainingRect:(NSRectPointer)remainingRect;
@end



@interface NSATSTypesetter (NSPrimitiveInterface)


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


- (NSUInteger)layoutParagraphAtPoint:(NSPoint *)lineFragmentOrigin;


- (CGFloat)lineSpacingAfterGlyphAtIndex:(NSUInteger)glyphIndex withProposedLineFragmentRect:(NSRect)rect;
- (CGFloat)paragraphSpacingBeforeGlyphAtIndex:(NSUInteger)glyphIndex withProposedLineFragmentRect:(NSRect)rect;
- (CGFloat)paragraphSpacingAfterGlyphAtIndex:(NSUInteger)glyphIndex withProposedLineFragmentRect:(NSRect)rect;




- (NSLayoutManager *)layoutManager;
- (NSTextContainer *)currentTextContainer;


- (void)setHardInvalidation:(BOOL)flag forGlyphRange:(NSRange)glyphRange;



- (void)getLineFragmentRect:(NSRect *)lineFragmentRect usedRect:(NSRect *)lineFragmentUsedRect forParagraphSeparatorGlyphRange:(NSRange)paragraphSeparatorGlyphRange atProposedOrigin:(NSPoint)lineOrigin;
@end


@interface NSATSTypesetter (NSLayoutPhaseInterface)

- (void)willSetLineFragmentRect:(NSRect *)lineRect forGlyphRange:(NSRange)glyphRange usedRect:(NSRect *)usedRect baselineOffset:(CGFloat *)baselineOffset;

- (BOOL)shouldBreakLineByWordBeforeCharacterAtIndex:(NSUInteger)charIndex;
- (BOOL)shouldBreakLineByHyphenatingBeforeCharacterAtIndex:(NSUInteger)charIndex;

- (float)hyphenationFactorForGlyphAtIndex:(NSUInteger)glyphIndex;
- (UTF32Char)hyphenCharacterForGlyphAtIndex:(NSUInteger)glyphIndex;

- (NSRect)boundingBoxForControlGlyphAtIndex:(NSUInteger)glyphIndex forTextContainer:(NSTextContainer *)textContainer proposedLineFragment:(NSRect)proposedRect glyphPosition:(NSPoint)glyphPosition characterIndex:(NSUInteger)charIndex;
@end


@interface NSATSTypesetter (NSGlyphStorageInterface)

- (NSRange)characterRangeForGlyphRange:(NSRange)glyphRange actualGlyphRange:(NSRangePointer)actualGlyphRange;
- (NSRange)glyphRangeForCharacterRange:(NSRange)charRange actualCharacterRange:(NSRangePointer)actualCharRange;


- (NSUInteger)getGlyphsInRange:(NSRange)glyphsRange glyphs:(NSGlyph *)glyphBuffer characterIndexes:(NSUInteger *)charIndexBuffer glyphInscriptions:(NSGlyphInscription *)inscribeBuffer elasticBits:(BOOL *)elasticBuffer;


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
