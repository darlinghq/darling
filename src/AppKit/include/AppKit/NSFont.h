#import <Foundation/NSObject.h>
#import <Foundation/NSString.h>
#import <AppKit/AppKitDefines.h>
#import <AppKit/NSCell.h>

@class NSFontDescriptor, NSAffineTransform, NSGraphicsContext;




typedef unsigned int NSGlyph;

enum {
    NSControlGlyph = 0x00FFFFFF,
    NSNullGlyph = 0x0
};




EXTERN_C const CGFloat *NSFontIdentityMatrix;




enum {
    NSNativeShortGlyphPacking = 5
};
typedef NSUInteger NSMultibyteGlyphPacking;


enum {
    NSFontDefaultRenderingMode = 0,
    NSFontAntialiasedRenderingMode = 1,
    NSFontIntegerAdvancementsRenderingMode = 2,
    NSFontAntialiasedIntegerAdvancementsRenderingMode = 3
};
typedef NSUInteger NSFontRenderingMode;


@interface NSFont : NSObject <NSCopying, NSCoding> {

    NSString *_name;
    CGFloat _size;
    NSInteger _retainCount;
    struct __fFlags {
        unsigned int _isScreenFont:1;
        unsigned int _systemFontType:8;
        unsigned int _reserved1:4;
        unsigned int _matrixIsIdentity:1;
        unsigned int _renderingMode:3;
        unsigned int _inInstanceCache:1;
        unsigned int _reserved2:14;
    } _fFlags;
    id _private;
}


+ (NSFont *)fontWithName:(NSString *)fontName size:(CGFloat)fontSize;
+ (NSFont *)fontWithName:(NSString *)fontName matrix:(const CGFloat *)fontMatrix;


+ (NSFont *)fontWithDescriptor:(NSFontDescriptor *)fontDescriptor size:(CGFloat)fontSize;



+ (NSFont *)fontWithDescriptor:(NSFontDescriptor *)fontDescriptor textTransform:(NSAffineTransform *)textTransform;




+ (NSFont *)userFontOfSize:(CGFloat)fontSize;
+ (NSFont *)userFixedPitchFontOfSize:(CGFloat)fontSize;
+ (void)setUserFont:(NSFont *)aFont;
+ (void)setUserFixedPitchFont:(NSFont *)aFont;



+ (NSFont *)systemFontOfSize:(CGFloat)fontSize;
+ (NSFont *)boldSystemFontOfSize:(CGFloat)fontSize;
+ (NSFont *)labelFontOfSize:(CGFloat)fontSize;

+ (NSFont *)titleBarFontOfSize:(CGFloat)fontSize;
+ (NSFont *)menuFontOfSize:(CGFloat)fontSize;
+ (NSFont *)menuBarFontOfSize:(CGFloat)fontSize;
+ (NSFont *)messageFontOfSize:(CGFloat)fontSize;
+ (NSFont *)paletteFontOfSize:(CGFloat)fontSize;
+ (NSFont *)toolTipsFontOfSize:(CGFloat)fontSize;
+ (NSFont *)controlContentFontOfSize:(CGFloat)fontSize;



+ (CGFloat)systemFontSize;
+ (CGFloat)smallSystemFontSize;
+ (CGFloat)labelFontSize;

+ (CGFloat)systemFontSizeForControlSize:(NSControlSize)controlSize;


- (NSString *)fontName;
- (CGFloat)pointSize;
- (const CGFloat *)matrix ;
- (NSString *)familyName;
- (NSString *)displayName;
- (NSFontDescriptor *)fontDescriptor;
- (NSAffineTransform *)textTransform;


- (NSUInteger)numberOfGlyphs;
- (NSStringEncoding)mostCompatibleStringEncoding;
- (NSGlyph)glyphWithName:(NSString *)aName;
- (NSCharacterSet *)coveredCharacterSet;




- (NSRect)boundingRectForFont;
- (NSSize)maximumAdvancement;

- (CGFloat)ascender;
- (CGFloat)descender;
- (CGFloat)leading;

- (CGFloat)underlinePosition;
- (CGFloat)underlineThickness;
- (CGFloat)italicAngle;
- (CGFloat)capHeight;
- (CGFloat)xHeight;
- (BOOL)isFixedPitch;


- (NSRect)boundingRectForGlyph:(NSGlyph)aGlyph;
- (NSSize)advancementForGlyph:(NSGlyph)ag;


- (void)getBoundingRects:(NSRectArray)bounds forGlyphs:(const NSGlyph *)glyphs count:(NSUInteger)glyphCount;
- (void)getAdvancements:(NSSizeArray)advancements forGlyphs:(const NSGlyph *)glyphs count:(NSUInteger)glyphCount;
- (void)getAdvancements:(NSSizeArray)advancements forPackedGlyphs:(const void *)packedGlyphs length:(NSUInteger)length;


- (void)set;
- (void)setInContext:(NSGraphicsContext *)graphicsContext;


- (NSFont *)printerFont;
- (NSFont *)screenFont;
- (NSFont *)screenFontWithRenderingMode:(NSFontRenderingMode)renderingMode;
- (NSFontRenderingMode)renderingMode;



- (NSFont *)verticalFont;


- (BOOL)isVertical;
@end




EXTERN_C NSInteger NSConvertGlyphsToPackedGlyphs(NSGlyph *glBuf, NSInteger count, NSMultibyteGlyphPacking packing, char *packedGlyphs);




EXTERN_C NSString *NSAntialiasThresholdChangedNotification;



EXTERN_C NSString *NSFontSetChangedNotification;




#if !__LP64__
#if MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_4


enum {
    NSOneByteGlyphPacking,
    NSJapaneseEUCGlyphPacking,
    NSAsciiWithDoubleByteEUCGlyphPacking,
    NSTwoByteGlyphPacking,
    NSFourByteGlyphPacking,
};

enum _NSGlyphRelation {
    NSGlyphBelow = 1,
    NSGlyphAbove = 2
};
#endif
typedef NSUInteger NSGlyphRelation;

@interface NSFont (NSFontDeprecated)
+ (void)useFont:(NSString *)fontName;
- (CGFloat)widthOfString:(NSString *)string;
- (BOOL)isBaseFont;
- (NSDictionary *)afmDictionary;
- (BOOL)glyphIsEncoded:(NSGlyph)aGlyph;
- (CGFloat)defaultLineHeightForFont;
+ (NSArray *)preferredFontNames;
+ (void)setPreferredFontNames:(NSArray *)fontNameArray;
- (NSString *)encodingScheme;
- (NSMultibyteGlyphPacking) glyphPacking;


- (NSPoint)positionOfGlyph:(NSGlyph)curGlyph precededByGlyph:(NSGlyph)prevGlyph isNominal:(BOOL *)nominal;
- (NSInteger)positionsForCompositeSequence:(NSGlyph *)someGlyphs numberOfGlyphs:(NSInteger)numGlyphs pointArray:(NSPointArray)points;
- (NSPoint)positionOfGlyph:(NSGlyph)curGlyph struckOverGlyph:(NSGlyph)prevGlyph metricsExist:(BOOL *)exist;
- (NSPoint)positionOfGlyph:(NSGlyph)aGlyph struckOverRect:(NSRect)aRect metricsExist:(BOOL *)exist;
- (NSPoint)positionOfGlyph:(NSGlyph)aGlyph forCharacter:(unichar)aChar struckOverRect:(NSRect)aRect;
- (NSPoint)positionOfGlyph:(NSGlyph)thisGlyph withRelation:(NSGlyphRelation)rel toBaseGlyph:(NSGlyph)baseGlyph totalAdvancement:(NSSizePointer)adv metricsExist:(BOOL *)exist;
@end

EXTERN_C NSString *NSAFMFamilyName;
EXTERN_C NSString *NSAFMFontName;
EXTERN_C NSString *NSAFMFormatVersion;
EXTERN_C NSString *NSAFMFullName;
EXTERN_C NSString *NSAFMNotice;
EXTERN_C NSString *NSAFMVersion;
EXTERN_C NSString *NSAFMWeight;
EXTERN_C NSString *NSAFMEncodingScheme;
EXTERN_C NSString *NSAFMCharacterSet;
EXTERN_C NSString *NSAFMCapHeight;
EXTERN_C NSString *NSAFMXHeight;
EXTERN_C NSString *NSAFMAscender;
EXTERN_C NSString *NSAFMDescender;
EXTERN_C NSString *NSAFMUnderlinePosition;
EXTERN_C NSString *NSAFMUnderlineThickness;
EXTERN_C NSString *NSAFMItalicAngle;
EXTERN_C NSString *NSAFMMappingScheme;
#endif
