#import <Foundation/NSObject.h>
#import <AppKit/NSText.h>

enum {
    NSLeftTabStopType = 0,
    NSRightTabStopType,
    NSCenterTabStopType,
    NSDecimalTabStopType
};
typedef NSUInteger NSTextTabType;

enum {
    NSLineBreakByWordWrapping = 0,
    NSLineBreakByCharWrapping,
    NSLineBreakByClipping,
    NSLineBreakByTruncatingHead,
    NSLineBreakByTruncatingTail,
    NSLineBreakByTruncatingMiddle
};
typedef NSUInteger NSLineBreakMode;



EXTERN_C NSString *NSTabColumnTerminatorsAttributeName;


@interface NSTextTab : NSObject <NSCopying, NSCoding> {

    struct {
 NSTextAlignment alignment:4;
        unsigned int refCount:24;
        unsigned int unused:4;
#if __LP64__
        unsigned int :32;
#endif
    } _flags;
    CGFloat _location;
    id _reserved;
}
- (id)initWithTextAlignment:(NSTextAlignment)alignment location:(CGFloat)loc options:(NSDictionary *)options;

- (NSTextAlignment)alignment;
- (NSDictionary *)options;
- (id)initWithType:(NSTextTabType)type location:(CGFloat)loc;
- (CGFloat)location;
- (NSTextTabType)tabStopType;
@end


@interface NSParagraphStyle : NSObject <NSCopying, NSMutableCopying, NSCoding> {

    CGFloat _lineSpacing;
    CGFloat _paragraphSpacing;
    CGFloat _headIndent;
    CGFloat _tailIndent;
    CGFloat _firstLineHeadIndent;
    CGFloat _minimumLineHeight;
    CGFloat _maximumLineHeight;
    NSArray *_tabStops;
    struct {
 NSTextAlignment alignment:4;
        NSLineBreakMode lineBreakMode:4;
        unsigned int tabStopsIsMutable:1;
        unsigned int isNaturalDirection:1;
        unsigned int rightToLeftDirection:1;
        unsigned int fixedMultiple:2;
        unsigned int refCount:19;
#if __LP64__
        unsigned int :32;
#endif
    } _flags;
    CGFloat _defaultTabInterval;
    id _extraData;
}

+ (NSParagraphStyle *)defaultParagraphStyle;

+ (NSWritingDirection)defaultWritingDirectionForLanguage:(NSString *)languageName;

- (CGFloat)lineSpacing;
- (CGFloat)paragraphSpacing;
- (NSTextAlignment)alignment;



- (CGFloat)headIndent;
- (CGFloat)tailIndent;
- (CGFloat)firstLineHeadIndent;
- (NSArray *)tabStops;

- (CGFloat)minimumLineHeight;
- (CGFloat)maximumLineHeight;

- (NSLineBreakMode)lineBreakMode;

- (NSWritingDirection)baseWritingDirection;

- (CGFloat)lineHeightMultiple;
- (CGFloat)paragraphSpacingBefore;
- (CGFloat)defaultTabInterval;

- (NSArray *)textBlocks;
- (NSArray *)textLists;



- (float)hyphenationFactor;



- (float)tighteningFactorForTruncation;


- (NSInteger)headerLevel;

@end

@interface NSMutableParagraphStyle : NSParagraphStyle

- (void)setLineSpacing:(CGFloat)aFloat;
- (void)setParagraphSpacing:(CGFloat)aFloat;
- (void)setAlignment:(NSTextAlignment)alignment;
- (void)setFirstLineHeadIndent:(CGFloat)aFloat;
- (void)setHeadIndent:(CGFloat)aFloat;
- (void)setTailIndent:(CGFloat)aFloat;
- (void)setLineBreakMode:(NSLineBreakMode)mode;
- (void)setMinimumLineHeight:(CGFloat)aFloat;
- (void)setMaximumLineHeight:(CGFloat)aFloat;
- (void)addTabStop:(NSTextTab *)anObject;
- (void)removeTabStop:(NSTextTab *)anObject;
- (void)setTabStops:(NSArray *)array;
- (void)setParagraphStyle:(NSParagraphStyle *)obj;
- (void)setBaseWritingDirection:(NSWritingDirection)writingDirection;
- (void)setLineHeightMultiple:(CGFloat)aFloat;
- (void)setParagraphSpacingBefore:(CGFloat)aFloat;
- (void)setDefaultTabInterval:(CGFloat)aFloat;
- (void)setTextBlocks:(NSArray *)array;
- (void)setTextLists:(NSArray *)array;
- (void)setHyphenationFactor:(float)aFactor;
- (void)setTighteningFactorForTruncation:(float)aFactor;
- (void)setHeaderLevel:(NSInteger)level;
@end
