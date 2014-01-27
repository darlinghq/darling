#import <Foundation/NSAttributedString.h>
#import <AppKit/NSFont.h>


enum {
    NSShowControlGlyphs = (1 << 0),
    NSShowInvisibleGlyphs = (1 << 1),
    NSWantsBidiLevels = (1 << 2)
};

@protocol NSGlyphStorage
- (void)insertGlyphs:(const NSGlyph *)glyphs length:(NSUInteger)length forStartingGlyphAtIndex:(NSUInteger)glyphIndex characterIndex:(NSUInteger)charIndex;


- (void)setIntAttribute:(NSInteger)attributeTag value:(NSInteger)val forGlyphAtIndex:(NSUInteger)glyphIndex;

- (NSAttributedString *)attributedString;
- (NSUInteger)layoutOptions;
@end

@interface NSGlyphGenerator : NSObject
- (void)generateGlyphsForGlyphStorage:(id <NSGlyphStorage>)glyphStorage desiredNumberOfCharacters:(NSUInteger)nChars glyphIndex:(NSUInteger *)glyphIndex characterIndex:(NSUInteger *)charIndex;

+ (id) sharedGlyphGenerator;
@end
