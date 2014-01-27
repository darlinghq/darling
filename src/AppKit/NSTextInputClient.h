#import <Foundation/NSObject.h>
#import <Foundation/NSRange.h>
#import <Foundation/NSGeometry.h>

@class NSArray;
@class NSAttributedString;

@protocol NSTextInputClient
@required


- (void)insertText:(id)aString replacementRange:(NSRange)replacementRange;



 - (void)doCommandBySelector:(SEL)aSelector;



- (void)setMarkedText:(id)aString selectedRange:(NSRange)selectedRange replacementRange:(NSRange)replacementRange;



- (void)unmarkText;



- (NSRange)selectedRange;



- (NSRange)markedRange;



- (BOOL)hasMarkedText;



- (NSAttributedString *)attributedSubstringForProposedRange:(NSRange)aRange actualRange:(NSRangePointer)actualRange;



- (NSArray*)validAttributesForMarkedText;



- (NSRect)firstRectForCharacterRange:(NSRange)aRange actualRange:(NSRangePointer)actualRange;



- (NSUInteger)characterIndexForPoint:(NSPoint)aPoint;

@optional


- (NSAttributedString *)attributedString;



- (CGFloat)fractionOfDistanceThroughGlyphForPoint:(NSPoint)aPoint;



- (CGFloat)baselineDeltaForCharacterAtIndex:(NSUInteger)anIndex;



- (NSInteger)windowLevel;



- (BOOL)drawsVerticallyForCharacterAtIndex:(NSUInteger)charIndex;
@end
