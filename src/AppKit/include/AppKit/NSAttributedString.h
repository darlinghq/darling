#import <Foundation/NSAttributedString.h>
#import <AppKit/NSFontManager.h>
#import <AppKit/NSText.h>
#import <AppKit/NSPasteboard.h>
#import <AppKit/AppKitDefines.h>
@class NSFileWrapper, NSURL, NSTextBlock, NSTextTable, NSTextList;



EXTERN_C NSString *NSFontAttributeName;
EXTERN_C NSString *NSParagraphStyleAttributeName;
EXTERN_C NSString *NSForegroundColorAttributeName;
EXTERN_C NSString *NSUnderlineStyleAttributeName;
EXTERN_C NSString *NSSuperscriptAttributeName;
EXTERN_C NSString *NSBackgroundColorAttributeName;
EXTERN_C NSString *NSAttachmentAttributeName;
EXTERN_C NSString *NSLigatureAttributeName;
EXTERN_C NSString *NSBaselineOffsetAttributeName;
EXTERN_C NSString *NSKernAttributeName;
EXTERN_C NSString *NSLinkAttributeName;

EXTERN_C NSString *NSStrokeWidthAttributeName;
EXTERN_C NSString *NSStrokeColorAttributeName;
EXTERN_C NSString *NSUnderlineColorAttributeName;
EXTERN_C NSString *NSStrikethroughStyleAttributeName;
EXTERN_C NSString *NSStrikethroughColorAttributeName;
EXTERN_C NSString *NSShadowAttributeName;
EXTERN_C NSString *NSObliquenessAttributeName;
EXTERN_C NSString *NSExpansionAttributeName;
EXTERN_C NSString *NSCursorAttributeName;
EXTERN_C NSString *NSToolTipAttributeName;



EXTERN_C NSString *NSCharacterShapeAttributeName;



EXTERN_C NSString *NSGlyphInfoAttributeName;



EXTERN_C NSString *NSWritingDirectionAttributeName;



EXTERN_C NSString *NSMarkedClauseSegmentAttributeName;



EXTERN_C NSString *NSVerticalGlyphFormAttributeName;


EXTERN_C NSString *NSTextAlternativesAttributeName;




enum {
    NSUnderlineStyleNone = 0x00,
    NSUnderlineStyleSingle = 0x01,
    NSUnderlineStyleThick = 0x02,
    NSUnderlineStyleDouble = 0x09
};

enum {
    NSUnderlinePatternSolid = 0x0000,
    NSUnderlinePatternDot = 0x0100,
    NSUnderlinePatternDash = 0x0200,
    NSUnderlinePatternDashDot = 0x0300,
    NSUnderlinePatternDashDotDot = 0x0400
};

EXTERN_C NSUInteger NSUnderlineByWordMask;



EXTERN_C NSString *NSSpellingStateAttributeName;

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5


enum {
    NSSpellingStateSpellingFlag = (1 << 0),
    NSSpellingStateGrammarFlag = (1 << 1)
};
#endif




EXTERN_C NSString *NSPlainTextDocumentType;
EXTERN_C NSString *NSRTFTextDocumentType;
EXTERN_C NSString *NSRTFDTextDocumentType;
EXTERN_C NSString *NSMacSimpleTextDocumentType;
EXTERN_C NSString *NSHTMLTextDocumentType;
EXTERN_C NSString *NSDocFormatTextDocumentType;
EXTERN_C NSString *NSWordMLTextDocumentType;
EXTERN_C NSString *NSWebArchiveTextDocumentType;
EXTERN_C NSString *NSOfficeOpenXMLTextDocumentType;
EXTERN_C NSString *NSOpenDocumentTextDocumentType;




EXTERN_C NSString *NSTextLayoutSectionOrientation;
EXTERN_C NSString *NSTextLayoutSectionRange;




EXTERN_C NSString *NSPaperSizeDocumentAttribute ;
EXTERN_C NSString *NSLeftMarginDocumentAttribute ;
EXTERN_C NSString *NSRightMarginDocumentAttribute ;
EXTERN_C NSString *NSTopMarginDocumentAttribute ;
EXTERN_C NSString *NSBottomMarginDocumentAttribute;
EXTERN_C NSString *NSViewSizeDocumentAttribute ;
EXTERN_C NSString *NSViewZoomDocumentAttribute ;
EXTERN_C NSString *NSViewModeDocumentAttribute ;
EXTERN_C NSString *NSDocumentTypeDocumentAttribute;
EXTERN_C NSString *NSReadOnlyDocumentAttribute ;
EXTERN_C NSString *NSConvertedDocumentAttribute ;
EXTERN_C NSString *NSCocoaVersionDocumentAttribute;
EXTERN_C NSString *NSBackgroundColorDocumentAttribute ;
EXTERN_C NSString *NSHyphenationFactorDocumentAttribute ;
EXTERN_C NSString *NSDefaultTabIntervalDocumentAttribute ;
EXTERN_C NSString *NSCharacterEncodingDocumentAttribute ;
EXTERN_C NSString *NSFileTypeDocumentAttribute;
EXTERN_C NSString *NSUsesScreenFontsDocumentAttribute;

EXTERN_C NSString *NSTitleDocumentAttribute ;
EXTERN_C NSString *NSCompanyDocumentAttribute ;
EXTERN_C NSString *NSCopyrightDocumentAttribute ;
EXTERN_C NSString *NSSubjectDocumentAttribute ;
EXTERN_C NSString *NSAuthorDocumentAttribute ;
EXTERN_C NSString *NSKeywordsDocumentAttribute ;
EXTERN_C NSString *NSCommentDocumentAttribute ;
EXTERN_C NSString *NSEditorDocumentAttribute ;
EXTERN_C NSString *NSCreationTimeDocumentAttribute;
EXTERN_C NSString *NSModificationTimeDocumentAttribute;
EXTERN_C NSString *NSManagerDocumentAttribute;
EXTERN_C NSString *NSCategoryDocumentAttribute;


EXTERN_C NSString *NSTextLayoutSectionsAttribute;



EXTERN_C NSString *NSExcludedElementsDocumentAttribute ;
EXTERN_C NSString *NSTextEncodingNameDocumentAttribute ;
EXTERN_C NSString *NSPrefixSpacesDocumentAttribute ;



EXTERN_C NSString *NSDocumentTypeDocumentOption ;
EXTERN_C NSString *NSDefaultAttributesDocumentOption ;
EXTERN_C NSString *NSCharacterEncodingDocumentOption ;
EXTERN_C NSString *NSTextEncodingNameDocumentOption ;
EXTERN_C NSString *NSBaseURLDocumentOption ;
EXTERN_C NSString *NSTimeoutDocumentOption ;
EXTERN_C NSString *NSWebPreferencesDocumentOption ;
EXTERN_C NSString *NSWebResourceLoadDelegateDocumentOption;
EXTERN_C NSString *NSTextSizeMultiplierDocumentOption ;
EXTERN_C NSString *NSFileTypeDocumentOption;




@interface NSAttributedString (NSAttributedStringKitAdditions) <NSPasteboardReading, NSPasteboardWriting>



- (NSDictionary *)fontAttributesInRange:(NSRange)range;



- (NSDictionary *)rulerAttributesInRange:(NSRange)range;

- (BOOL)containsAttachments;



- (NSUInteger)lineBreakBeforeIndex:(NSUInteger)location withinRange:(NSRange)aRange;
- (NSUInteger)lineBreakByHyphenatingBeforeIndex:(NSUInteger)location withinRange:(NSRange)aRange;

- (NSRange)doubleClickAtIndex:(NSUInteger)location;
- (NSUInteger)nextWordFromIndex:(NSUInteger)location forward:(BOOL)isForward;



- (NSURL *)URLAtIndex:(NSUInteger)location effectiveRange:(NSRangePointer)effectiveRange;



+ (NSArray *)textTypes;
+ (NSArray *)textUnfilteredTypes;



- (NSRange)rangeOfTextBlock:(NSTextBlock *)block atIndex:(NSUInteger)location;
- (NSRange)rangeOfTextTable:(NSTextTable *)table atIndex:(NSUInteger)location;
- (NSRange)rangeOfTextList:(NSTextList *)list atIndex:(NSUInteger)location;
- (NSInteger)itemNumberInTextList:(NSTextList *)list atIndex:(NSUInteger)location;





- (id)initWithURL:(NSURL *)url options:(NSDictionary *)options documentAttributes:(NSDictionary **)dict error:(NSError **)error;
- (id)initWithData:(NSData *)data options:(NSDictionary *)options documentAttributes:(NSDictionary **)dict error:(NSError **)error;



- (id)initWithPath:(NSString *)path documentAttributes:(NSDictionary **)dict;
- (id)initWithURL:(NSURL *)url documentAttributes:(NSDictionary **)dict;



- (id)initWithRTF:(NSData *)data documentAttributes:(NSDictionary **)dict;
- (id)initWithRTFD:(NSData *)data documentAttributes:(NSDictionary **)dict;
- (id)initWithHTML:(NSData *)data documentAttributes:(NSDictionary **)dict;
- (id)initWithHTML:(NSData *)data baseURL:(NSURL *)base documentAttributes:(NSDictionary **)dict;
- (id)initWithDocFormat:(NSData *)data documentAttributes:(NSDictionary **)dict;
- (id)initWithHTML:(NSData *)data options:(NSDictionary *)options documentAttributes:(NSDictionary **)dict;



- (id)initWithRTFDFileWrapper:(NSFileWrapper *)wrapper documentAttributes:(NSDictionary **)dict;






- (NSData *)dataFromRange:(NSRange)range documentAttributes:(NSDictionary *)dict error:(NSError **)error;
- (NSFileWrapper *)fileWrapperFromRange:(NSRange)range documentAttributes:(NSDictionary *)dict error:(NSError **)error;



- (NSData *)RTFFromRange:(NSRange)range documentAttributes:(NSDictionary *)dict;
- (NSData *)RTFDFromRange:(NSRange)range documentAttributes:(NSDictionary *)dict;
- (NSFileWrapper *)RTFDFileWrapperFromRange:(NSRange)range documentAttributes:(NSDictionary *)dict;
- (NSData *)docFormatFromRange:(NSRange)range documentAttributes:(NSDictionary *)dict;

@end

@interface NSAttributedString(NSDeprecatedKitAdditions)



+ (NSArray *)textFileTypes;
+ (NSArray *)textPasteboardTypes;
+ (NSArray *)textUnfilteredFileTypes;
+ (NSArray *)textUnfilteredPasteboardTypes;

@end

@interface NSMutableAttributedString (NSMutableAttributedStringKitAdditions)





- (BOOL)readFromURL:(NSURL *)url options:(NSDictionary *)opts documentAttributes:(NSDictionary **)dict error:(NSError **)error;
- (BOOL)readFromData:(NSData *)data options:(NSDictionary *)opts documentAttributes:(NSDictionary **)dict error:(NSError **)error;



- (BOOL)readFromURL:(NSURL *)url options:(NSDictionary *)options documentAttributes:(NSDictionary **)dict;
- (BOOL)readFromData:(NSData *)data options:(NSDictionary *)options documentAttributes:(NSDictionary **)dict;



- (void)superscriptRange:(NSRange)range;
- (void)subscriptRange:(NSRange)range;
- (void)unscriptRange:(NSRange)range;
- (void)applyFontTraits:(NSFontTraitMask)traitMask range:(NSRange)range;
- (void)setAlignment:(NSTextAlignment)alignment range:(NSRange)range;
- (void)setBaseWritingDirection:(NSWritingDirection)writingDirection range:(NSRange)range;



- (void)fixAttributesInRange:(NSRange)range;
- (void)fixFontAttributeInRange:(NSRange)range;
- (void)fixParagraphStyleAttributeInRange:(NSRange)range;
- (void)fixAttachmentAttributeInRange:(NSRange)range;

@end



enum {
    NSNoUnderlineStyle = 0,
    NSSingleUnderlineStyle
};
EXTERN_C NSUInteger NSUnderlineStrikethroughMask;
