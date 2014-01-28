#import <Foundation/NSObject.h>
#import <AppKit/NSFont.h>
#import <AppKit/NSImageCell.h>
#import <AppKit/NSGlyphGenerator.h>

@class NSTextStorage;
@class NSTypesetter;
@class NSTextContainer;
@class NSTextView;
@class NSWindow;
@class NSColor;
@class NSRulerView;
@class NSParagraphStyle;
@class NSRulerMarker;
@class NSBox;
@class NSTextField;
@class NSMatrix;
@class NSTabWell;
@class NSStorage;
@class NSRunStorage;
@class NSSortedArray;
@class NSView;
@class NSMutableArray;
@class NSEvent;
@class NSCell;
@class NSGlyphGenerator;
@class NSTextBlock;

@protocol NSLayoutManagerDelegate;


enum {
    NSGlyphAttributeSoft = 0,
    NSGlyphAttributeElastic = 1,
    NSGlyphAttributeBidiLevel = 2,
    NSGlyphAttributeInscribe = 5
};


enum {
    NSGlyphInscribeBase = 0,
    NSGlyphInscribeBelow = 1,
    NSGlyphInscribeAbove = 2,
    NSGlyphInscribeOverstrike = 3,
    NSGlyphInscribeOverBelow = 4
};
typedef NSUInteger NSGlyphInscription;


enum {
    NSTypesetterLatestBehavior = -1,
    NSTypesetterOriginalBehavior = 0,
    NSTypesetterBehavior_10_2_WithCompatibility = 1,
    NSTypesetterBehavior_10_2 = 2,
    NSTypesetterBehavior_10_3 = 3,
    NSTypesetterBehavior_10_4 = 4
};
typedef NSInteger NSTypesetterBehavior;



enum {
    NSTextLayoutOrientationHorizontal = 0,
    NSTextLayoutOrientationVertical = 1,
};
typedef NSInteger NSTextLayoutOrientation;

@protocol NSTextLayoutOrientationProvider
- (NSTextLayoutOrientation)layoutOrientation;

@end


@interface NSLayoutManager : NSObject <NSCoding, NSGlyphStorage> {



  @private
    NSTextStorage *_textStorage;
    NSGlyphGenerator *_glyphGenerator;
    NSTypesetter *_typesetter;

    NSMutableArray *_textContainers;
    NSStorage *_containerUsedRects;

    NSStorage *_glyphs;
    NSRunStorage *_containerRuns;
    NSRunStorage *_fragmentRuns;
    NSRunStorage *_glyphLocations;
    NSRunStorage *_glyphRotationRuns;

    NSRect _extraLineFragmentRect;
    NSRect _extraLineFragmentUsedRect;
    NSTextContainer *_extraLineFragmentContainer;

    NSSortedArray *_glyphHoles;
    NSSortedArray *_layoutHoles;

    struct __lmFlags {
        unsigned int containersAreFull:1;
        unsigned int glyphsMightDrawOutsideLines:1;
        unsigned int backgroundLayoutEnabled:1;
        unsigned int resizingInProgress:1;
        unsigned int allowScreenFonts:1;
        unsigned int cachedRectArrayInUse:1;
        unsigned int displayInvalidationInProgress:1;
        unsigned int insertionPointNeedsUpdate:1;
        unsigned int layoutManagerInDirtyList:1;
        unsigned int usingGlyphCache:1;
        unsigned int showInvisibleCharacters:1;
        unsigned int showControlCharacters:1;
        unsigned int delegateRespondsToDidInvalidate:1;
        unsigned int delegateRespondsToDidComplete:1;
        unsigned int glyphFormat:2;
        unsigned int textStorageRespondsToIsEditing:1;
        unsigned int notifyEditedInProgress:1;
        unsigned int containersChanged:1;
        unsigned int isGeneratingGlyphs:1;
        unsigned int hasNonGeneratedGlyphData:1;
        unsigned int loggedBGLayoutException:1;
        unsigned int isLayoutRequestedFromSubthread:1;
        unsigned int defaultAttachmentScaling:2;
        unsigned int isInUILayoutMode:1;
        unsigned int seenRightToLeft:1;
        unsigned int ignoresViewTransformations:1;
        unsigned int needToFlushGlyph:1;
        unsigned int flipsIfNeeded:1;
        unsigned int allowNonContig:1;
        unsigned int useNonContig:1;
    } _lmFlags;

    id _delegate;


    unsigned short _textViewResizeDisableStack;
    unsigned short _displayInvalidationDisableStack;
    NSRange _deferredDisplayCharRange;


    NSTextView *_firstTextView;


    __strong NSRect *_cachedRectArray;
    NSUInteger _cachedRectArrayCapacity;


    __strong char *_glyphBuffer;
    NSUInteger _glyphBufferSize;


    NSRange _cachedLocationNominalGlyphRange;
    NSUInteger _cachedLocationGlyphIndex;
    NSPoint _cachedLocation;
    NSRange _cachedFontCharRange;
    NSFont *_cachedFont;


    NSUInteger _firstUnlaidGlyphIndex;
    NSUInteger _firstUnlaidCharIndex;


    NSBox *_rulerAccView;
    id _rulerAccViewAlignmentButtons;
    id _rulerAccViewSpacing;
    NSTabWell *_rulerAccViewLeftTabWell;
    NSTabWell *_rulerAccViewRightTabWell;
    NSTabWell *_rulerAccViewCenterTabWell;
    NSTabWell *_rulerAccViewDecimalTabWell;
    id _rulerAccViewStyles;
    id _rulerAccViewLists;

    NSRange _newlyFilledGlyphRange;

    id _extraData;
}



- (id)init;




- (NSTextStorage *)textStorage;
- (void)setTextStorage:(NSTextStorage *)textStorage;


- (NSAttributedString *)attributedString;


- (void)replaceTextStorage:(NSTextStorage *)newTextStorage;


- (NSGlyphGenerator *)glyphGenerator;
- (void)setGlyphGenerator:(NSGlyphGenerator *)glyphGenerator;


- (NSTypesetter *)typesetter;
- (void)setTypesetter:(NSTypesetter *)typesetter;


- (id <NSLayoutManagerDelegate>)delegate;
- (void)setDelegate:(id <NSLayoutManagerDelegate>)delegate;




- (NSArray *)textContainers;

- (void)addTextContainer:(NSTextContainer *)container;

- (void)insertTextContainer:(NSTextContainer *)container atIndex:(NSUInteger)index;

- (void)removeTextContainerAtIndex:(NSUInteger)index;


- (void)textContainerChangedGeometry:(NSTextContainer *)container;


- (void)textContainerChangedTextView:(NSTextContainer *)container;




- (void)setBackgroundLayoutEnabled:(BOOL)flag;
- (BOOL)backgroundLayoutEnabled;


- (void)setUsesScreenFonts:(BOOL)flag;
- (BOOL)usesScreenFonts;


- (void)setShowsInvisibleCharacters:(BOOL)flag;
- (BOOL)showsInvisibleCharacters;


- (void)setShowsControlCharacters:(BOOL)flag;
- (BOOL)showsControlCharacters;


- (void)setHyphenationFactor:(float)factor;
- (float)hyphenationFactor;


- (void)setDefaultAttachmentScaling:(NSImageScaling)scaling;
- (NSImageScaling)defaultAttachmentScaling;


- (void)setTypesetterBehavior:(NSTypesetterBehavior)theBehavior;
- (NSTypesetterBehavior)typesetterBehavior;


- (NSUInteger)layoutOptions;


- (void)setAllowsNonContiguousLayout:(BOOL)flag;
- (BOOL)allowsNonContiguousLayout;

- (BOOL)hasNonContiguousLayout;




- (void)invalidateGlyphsForCharacterRange:(NSRange)charRange changeInLength:(NSInteger)delta actualCharacterRange:(NSRangePointer)actualCharRange;


- (void)invalidateLayoutForCharacterRange:(NSRange)charRange actualCharacterRange:(NSRangePointer)actualCharRange;
- (void)invalidateLayoutForCharacterRange:(NSRange)charRange isSoft:(BOOL)flag actualCharacterRange:(NSRangePointer)actualCharRange;


- (void)invalidateDisplayForCharacterRange:(NSRange)charRange;
- (void)invalidateDisplayForGlyphRange:(NSRange)glyphRange;


- (void)textStorage:(NSTextStorage *)str edited:(NSUInteger)editedMask range:(NSRange)newCharRange changeInLength:(NSInteger)delta invalidatedRange:(NSRange)invalidatedCharRange;




- (void)ensureGlyphsForCharacterRange:(NSRange)charRange;
- (void)ensureGlyphsForGlyphRange:(NSRange)glyphRange;
- (void)ensureLayoutForCharacterRange:(NSRange)charRange;
- (void)ensureLayoutForGlyphRange:(NSRange)glyphRange;
- (void)ensureLayoutForTextContainer:(NSTextContainer *)container;
- (void)ensureLayoutForBoundingRect:(NSRect)bounds inTextContainer:(NSTextContainer *)container;






- (void)insertGlyphs:(const NSGlyph *)glyphs length:(NSUInteger)length forStartingGlyphAtIndex:(NSUInteger)glyphIndex characterIndex:(NSUInteger)charIndex;


- (void)insertGlyph:(NSGlyph)glyph atGlyphIndex:(NSUInteger)glyphIndex characterIndex:(NSUInteger)charIndex;


- (void)replaceGlyphAtIndex:(NSUInteger)glyphIndex withGlyph:(NSGlyph)newGlyph;


- (void)deleteGlyphsInRange:(NSRange)glyphRange;


- (void)setCharacterIndex:(NSUInteger)charIndex forGlyphAtIndex:(NSUInteger)glyphIndex;


- (void)setIntAttribute:(NSInteger)attributeTag value:(NSInteger)val forGlyphAtIndex:(NSUInteger)glyphIndex;


- (void)invalidateGlyphsOnLayoutInvalidationForGlyphRange:(NSRange)glyphRange;




- (NSUInteger)numberOfGlyphs;


- (NSGlyph)glyphAtIndex:(NSUInteger)glyphIndex isValidIndex:(BOOL *)isValidIndex;
- (NSGlyph)glyphAtIndex:(NSUInteger)glyphIndex;
- (BOOL)isValidGlyphIndex:(NSUInteger)glyphIndex;


- (NSUInteger)characterIndexForGlyphAtIndex:(NSUInteger)glyphIndex;


- (NSUInteger)glyphIndexForCharacterAtIndex:(NSUInteger)charIndex;


- (NSInteger)intAttribute:(NSInteger)attributeTag forGlyphAtIndex:(NSUInteger)glyphIndex;


- (NSUInteger)getGlyphsInRange:(NSRange)glyphRange glyphs:(NSGlyph *)glyphBuffer characterIndexes:(NSUInteger *)charIndexBuffer glyphInscriptions:(NSGlyphInscription *)inscribeBuffer elasticBits:(BOOL *)elasticBuffer;
- (NSUInteger)getGlyphsInRange:(NSRange)glyphRange glyphs:(NSGlyph *)glyphBuffer characterIndexes:(NSUInteger *)charIndexBuffer glyphInscriptions:(NSGlyphInscription *)inscribeBuffer elasticBits:(BOOL *)elasticBuffer bidiLevels:(unsigned char *)bidiLevelBuffer;


- (NSUInteger)getGlyphs:(NSGlyph *)glyphArray range:(NSRange)glyphRange;






- (void)setTextContainer:(NSTextContainer *)container forGlyphRange:(NSRange)glyphRange;


- (void)setLineFragmentRect:(NSRect)fragmentRect forGlyphRange:(NSRange)glyphRange usedRect:(NSRect)usedRect;


- (void)setExtraLineFragmentRect:(NSRect)fragmentRect usedRect:(NSRect)usedRect textContainer:(NSTextContainer *)container;


- (void)setLocation:(NSPoint)location forStartOfGlyphRange:(NSRange)glyphRange;


- (void)setLocations:(NSPointArray)locations startingGlyphIndexes:(NSUInteger *)glyphIndexes count:(NSUInteger)count forGlyphRange:(NSRange)glyphRange;


- (void)setNotShownAttribute:(BOOL)flag forGlyphAtIndex:(NSUInteger)glyphIndex;


- (void)setDrawsOutsideLineFragment:(BOOL)flag forGlyphAtIndex:(NSUInteger)glyphIndex;


- (void)setAttachmentSize:(NSSize)attachmentSize forGlyphRange:(NSRange)glyphRange;






- (void)getFirstUnlaidCharacterIndex:(NSUInteger *)charIndex glyphIndex:(NSUInteger *)glyphIndex;
- (NSUInteger)firstUnlaidCharacterIndex;
- (NSUInteger)firstUnlaidGlyphIndex;




- (NSTextContainer *)textContainerForGlyphAtIndex:(NSUInteger)glyphIndex effectiveRange:(NSRangePointer)effectiveGlyphRange;


- (NSRect)usedRectForTextContainer:(NSTextContainer *)container;


- (NSRect)lineFragmentRectForGlyphAtIndex:(NSUInteger)glyphIndex effectiveRange:(NSRangePointer)effectiveGlyphRange;


- (NSRect)lineFragmentUsedRectForGlyphAtIndex:(NSUInteger)glyphIndex effectiveRange:(NSRangePointer)effectiveGlyphRange;


- (NSRect)lineFragmentRectForGlyphAtIndex:(NSUInteger)glyphIndex effectiveRange:(NSRangePointer)effectiveGlyphRange withoutAdditionalLayout:(BOOL)flag;
- (NSRect)lineFragmentUsedRectForGlyphAtIndex:(NSUInteger)glyphIndex effectiveRange:(NSRangePointer)effectiveGlyphRange withoutAdditionalLayout:(BOOL)flag;
- (NSTextContainer *)textContainerForGlyphAtIndex:(NSUInteger)glyphIndex effectiveRange:(NSRangePointer)effectiveGlyphRange withoutAdditionalLayout:(BOOL)flag;


- (NSRect)extraLineFragmentRect;
- (NSRect)extraLineFragmentUsedRect;
- (NSTextContainer *)extraLineFragmentTextContainer;


- (NSPoint)locationForGlyphAtIndex:(NSUInteger)glyphIndex;


- (BOOL)notShownAttributeForGlyphAtIndex:(NSUInteger)glyphIndex;


- (BOOL)drawsOutsideLineFragmentForGlyphAtIndex:(NSUInteger)glyphIndex;


- (NSSize)attachmentSizeForGlyphAtIndex:(NSUInteger)glyphIndex;




- (void)setLayoutRect:(NSRect)rect forTextBlock:(NSTextBlock *)block glyphRange:(NSRange)glyphRange;
- (void)setBoundsRect:(NSRect)rect forTextBlock:(NSTextBlock *)block glyphRange:(NSRange)glyphRange;
- (NSRect)layoutRectForTextBlock:(NSTextBlock *)block glyphRange:(NSRange)glyphRange;
- (NSRect)boundsRectForTextBlock:(NSTextBlock *)block glyphRange:(NSRange)glyphRange;

- (NSRect)layoutRectForTextBlock:(NSTextBlock *)block atIndex:(NSUInteger)glyphIndex effectiveRange:(NSRangePointer)effectiveGlyphRange;
- (NSRect)boundsRectForTextBlock:(NSTextBlock *)block atIndex:(NSUInteger)glyphIndex effectiveRange:(NSRangePointer)effectiveGlyphRange;






- (NSRange)glyphRangeForCharacterRange:(NSRange)charRange actualCharacterRange:(NSRangePointer)actualCharRange;


- (NSRange)characterRangeForGlyphRange:(NSRange)glyphRange actualGlyphRange:(NSRangePointer)actualGlyphRange;




- (NSRange)glyphRangeForTextContainer:(NSTextContainer *)container;


- (NSRange)rangeOfNominallySpacedGlyphsContainingIndex:(NSUInteger)glyphIndex;


- (NSRectArray)rectArrayForCharacterRange:(NSRange)charRange withinSelectedCharacterRange:(NSRange)selCharRange inTextContainer:(NSTextContainer *)container rectCount:(NSUInteger *)rectCount ;
- (NSRectArray)rectArrayForGlyphRange:(NSRange)glyphRange withinSelectedGlyphRange:(NSRange)selGlyphRange inTextContainer:(NSTextContainer *)container rectCount:(NSUInteger *)rectCount ;



- (NSRect)boundingRectForGlyphRange:(NSRange)glyphRange inTextContainer:(NSTextContainer *)container;


- (NSRange)glyphRangeForBoundingRect:(NSRect)bounds inTextContainer:(NSTextContainer *)container;
- (NSRange)glyphRangeForBoundingRectWithoutAdditionalLayout:(NSRect)bounds inTextContainer:(NSTextContainer *)container;


- (NSUInteger)glyphIndexForPoint:(NSPoint)point inTextContainer:(NSTextContainer *)container fractionOfDistanceThroughGlyph:(CGFloat *)partialFraction;
- (NSUInteger)glyphIndexForPoint:(NSPoint)point inTextContainer:(NSTextContainer *)container;
- (CGFloat)fractionOfDistanceThroughGlyphForPoint:(NSPoint)point inTextContainer:(NSTextContainer *)container;


- (NSUInteger)characterIndexForPoint:(NSPoint)point inTextContainer:(NSTextContainer *)container fractionOfDistanceBetweenInsertionPoints:(CGFloat *)partialFraction;


- (NSUInteger)getLineFragmentInsertionPointsForCharacterAtIndex:(NSUInteger)charIndex alternatePositions:(BOOL)aFlag inDisplayOrder:(BOOL)dFlag positions:(CGFloat *)positions characterIndexes:(NSUInteger *)charIndexes;




- (NSDictionary *)temporaryAttributesAtCharacterIndex:(NSUInteger)charIndex effectiveRange:(NSRangePointer)effectiveCharRange;
- (void)setTemporaryAttributes:(NSDictionary *)attrs forCharacterRange:(NSRange)charRange;
- (void)addTemporaryAttributes:(NSDictionary *)attrs forCharacterRange:(NSRange)charRange;
- (void)removeTemporaryAttribute:(NSString *)attrName forCharacterRange:(NSRange)charRange;


- (id)temporaryAttribute:(NSString *)attrName atCharacterIndex:(NSUInteger)location effectiveRange:(NSRangePointer)range;
- (id)temporaryAttribute:(NSString *)attrName atCharacterIndex:(NSUInteger)location longestEffectiveRange:(NSRangePointer)range inRange:(NSRange)rangeLimit;
- (NSDictionary *)temporaryAttributesAtCharacterIndex:(NSUInteger)location longestEffectiveRange:(NSRangePointer)range inRange:(NSRange)rangeLimit;
- (void)addTemporaryAttribute:(NSString *)attrName value:(id)value forCharacterRange:(NSRange)charRange;




- (NSFont *)substituteFontForFont:(NSFont *)originalFont;


- (CGFloat)defaultLineHeightForFont:(NSFont *)theFont;

- (CGFloat)defaultBaselineOffsetForFont:(NSFont *)theFont;

- (BOOL)usesFontLeading;
- (void)setUsesFontLeading:(BOOL)flag;


@end

@interface NSLayoutManager (NSTextViewSupport)



- (NSArray *)rulerMarkersForTextView:(NSTextView *)view paragraphStyle:(NSParagraphStyle *)style ruler:(NSRulerView *)ruler;
- (NSView *)rulerAccessoryViewForTextView:(NSTextView *)view paragraphStyle:(NSParagraphStyle *)style ruler:(NSRulerView *)ruler enabled:(BOOL)isEnabled;




- (BOOL)layoutManagerOwnsFirstResponderInWindow:(NSWindow *)window;


- (NSTextView *)firstTextView;

- (NSTextView *)textViewForBeginningOfSelection;




- (void)drawBackgroundForGlyphRange:(NSRange)glyphsToShow atPoint:(NSPoint)origin;
- (void)drawGlyphsForGlyphRange:(NSRange)glyphsToShow atPoint:(NSPoint)origin;


- (void)showCGGlyphs:(const CGGlyph *)glyphs positions:(const NSPoint *)positions count:(NSUInteger)glyphCount font:(NSFont *)font matrix:(NSAffineTransform *)textMatrix attributes:(NSDictionary *)attributes inContext:(NSGraphicsContext *)graphicsContext;


- (void)showPackedGlyphs:(char *)glyphs length:(NSUInteger)glyphLen glyphRange:(NSRange)glyphRange atPoint:(NSPoint)point font:(NSFont *)font color:(NSColor *)color printingAdjustment:(NSSize)printingAdjustment;


- (void)showAttachmentCell:(NSCell *)cell inRect:(NSRect)rect characterIndex:(NSUInteger)attachmentIndex;


- (void)fillBackgroundRectArray:(NSRectArray)rectArray count:(NSUInteger)rectCount forCharacterRange:(NSRange)charRange color:(NSColor *)color;


- (void)drawUnderlineForGlyphRange:(NSRange)glyphRange underlineType:(NSInteger)underlineVal baselineOffset:(CGFloat)baselineOffset lineFragmentRect:(NSRect)lineRect lineFragmentGlyphRange:(NSRange)lineGlyphRange containerOrigin:(NSPoint)containerOrigin;
- (void)underlineGlyphRange:(NSRange)glyphRange underlineType:(NSInteger)underlineVal lineFragmentRect:(NSRect)lineRect lineFragmentGlyphRange:(NSRange)lineGlyphRange containerOrigin:(NSPoint)containerOrigin;


- (void)drawStrikethroughForGlyphRange:(NSRange)glyphRange strikethroughType:(NSInteger)strikethroughVal baselineOffset:(CGFloat)baselineOffset lineFragmentRect:(NSRect)lineRect lineFragmentGlyphRange:(NSRange)lineGlyphRange containerOrigin:(NSPoint)containerOrigin;
- (void)strikethroughGlyphRange:(NSRange)glyphRange strikethroughType:(NSInteger)strikethroughVal lineFragmentRect:(NSRect)lineRect lineFragmentGlyphRange:(NSRange)lineGlyphRange containerOrigin:(NSPoint)containerOrigin;


@end

@protocol NSLayoutManagerDelegate <NSObject>
@optional
- (void)layoutManagerDidInvalidateLayout:(NSLayoutManager *)sender;


- (void)layoutManager:(NSLayoutManager *)layoutManager didCompleteLayoutForTextContainer:(NSTextContainer *)textContainer atEnd:(BOOL)layoutFinishedFlag;


- (NSDictionary *)layoutManager:(NSLayoutManager *)layoutManager shouldUseTemporaryAttributes:(NSDictionary *)attrs forDrawingToScreen:(BOOL)toScreen atCharacterIndex:(NSUInteger)charIndex effectiveRange:(NSRangePointer)effectiveCharRange;


@end
