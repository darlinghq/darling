#import <Foundation/NSObject.h>
#import <Foundation/NSRange.h>
#import <Foundation/NSGeometry.h>
#import <Foundation/NSTextCheckingResult.h>
#import <AppKit/AppKitDefines.h>

@class NSArray, NSDictionary, NSString, NSOrthography, NSPanel, NSView, NSViewController, NSMenu;
@interface NSSpellChecker : NSObject {



@private

    id _guessesBrowser;
    id _wordField;
    id _languagePopUp;
    id _reserved1;
    id _panel;
    id _reserved2;
    id _correctButton;
    id _guessButton;
    id _ignoreButton;
    id _reserved3;
    id _languagesBrowser;
    id _quotesBrowser;
    id _replacementsBrowser;
    id _defineButton;

    struct __scFlags {
        unsigned int autoShowGuesses:1;
        unsigned int needDelayedGuess:1;
        unsigned int unignoreInProgress:1;
        unsigned int wordFieldEdited:1;
        unsigned int inSpelling:1;
        unsigned int reconnectSpelling:1;
        unsigned int inGrammar:1;
        unsigned int reconnectGrammar:1;
        unsigned int languageIdentification:1;
        unsigned int languagesHidden:1;
        unsigned int quotesByLanguage:1;
        unsigned int _reserved:21;
    } _scFlags;

    id _substitutionsPanel;
    id _reserved4;
    id _learnButton;
    id _infoField;
    id _grammarControl;
}


+ (NSSpellChecker *)sharedSpellChecker;
+ (BOOL)sharedSpellCheckerExists;


+ (NSInteger)uniqueSpellDocumentTag;


- (NSRange)checkSpellingOfString:(NSString *)stringToCheck startingAt:(NSInteger)startingOffset language:(NSString *)language wrap:(BOOL)wrapFlag inSpellDocumentWithTag:(NSInteger)tag wordCount:(NSInteger *)wordCount;

- (NSRange)checkSpellingOfString:(NSString *)stringToCheck startingAt:(NSInteger)startingOffset;


- (NSInteger)countWordsInString:(NSString *)stringToCount language:(NSString *)language;


- (NSRange)checkGrammarOfString:(NSString *)stringToCheck startingAt:(NSInteger)startingOffset language:(NSString *)language wrap:(BOOL)wrapFlag inSpellDocumentWithTag:(NSInteger)tag details:(NSArray **)details;


- (NSArray *)checkString:(NSString *)stringToCheck range:(NSRange)range types:(NSTextCheckingTypes)checkingTypes options:(NSDictionary *)options inSpellDocumentWithTag:(NSInteger)tag orthography:(NSOrthography **)orthography wordCount:(NSInteger *)wordCount;

#if NS_BLOCKS_AVAILABLE

- (NSInteger)requestCheckingOfString:(NSString *)stringToCheck range:(NSRange)range types:(NSTextCheckingTypes)checkingTypes options:(NSDictionary *)options inSpellDocumentWithTag:(NSInteger)tag completionHandler:(void (^)(NSInteger sequenceNumber, NSArray *results, NSOrthography *orthography, NSInteger wordCount))completionHandler;
#endif


- (NSMenu *)menuForResult:(NSTextCheckingResult *)result string:(NSString *)checkedString options:(NSDictionary *)options atLocation:(NSPoint)location inView:(NSView *)view;


EXTERN_C NSString *NSTextCheckingOrthographyKey;
EXTERN_C NSString *NSTextCheckingQuotesKey;
EXTERN_C NSString *NSTextCheckingReplacementsKey;
EXTERN_C NSString *NSTextCheckingReferenceDateKey;
EXTERN_C NSString *NSTextCheckingReferenceTimeZoneKey;
EXTERN_C NSString *NSTextCheckingDocumentURLKey;
EXTERN_C NSString *NSTextCheckingDocumentTitleKey;
EXTERN_C NSString *NSTextCheckingDocumentAuthorKey;
EXTERN_C NSString *NSTextCheckingRegularExpressionsKey;


- (NSArray *)userQuotesArrayForLanguage:(NSString *)language;
- (NSDictionary *)userReplacementsDictionary;


- (void)updateSpellingPanelWithMisspelledWord:(NSString *)word;


- (void)updateSpellingPanelWithGrammarString:(NSString *)string detail:(NSDictionary *)detail;


- (NSPanel *)spellingPanel;
- (NSView *)accessoryView;
- (void)setAccessoryView:(NSView *)aView;


- (NSPanel *)substitutionsPanel;
- (NSViewController *)substitutionsPanelAccessoryViewController;
- (void)setSubstitutionsPanelAccessoryViewController:(NSViewController *)accessoryController;


- (void)updatePanels;


- (void)ignoreWord:(NSString *)wordToIgnore inSpellDocumentWithTag:(NSInteger)tag;

- (NSArray *)ignoredWordsInSpellDocumentWithTag:(NSInteger)tag;
- (void)setIgnoredWords:(NSArray *)words inSpellDocumentWithTag:(NSInteger)tag;


- (NSArray *)guessesForWordRange:(NSRange)range inString:(NSString *)string language:(NSString *)language inSpellDocumentWithTag:(NSInteger)tag;


- (NSString *)correctionForWordRange:(NSRange)range inString:(NSString *)string language:(NSString *)language inSpellDocumentWithTag:(NSInteger)tag;


- (NSArray *)completionsForPartialWordRange:(NSRange)range inString:(NSString *)string language:(NSString *)language inSpellDocumentWithTag:(NSInteger)tag;


- (NSString *)languageForWordRange:(NSRange)range inString:(NSString *)string orthography:(NSOrthography *)orthography;


- (void)closeSpellDocumentWithTag:(NSInteger)tag;


enum {
    NSCorrectionResponseNone,
    NSCorrectionResponseAccepted,
    NSCorrectionResponseRejected,
    NSCorrectionResponseIgnored,
    NSCorrectionResponseEdited,
    NSCorrectionResponseReverted
};
typedef NSInteger NSCorrectionResponse;

- (void)recordResponse:(NSCorrectionResponse)response toCorrection:(NSString *)correction forWord:(NSString *)word language:(NSString *)language inSpellDocumentWithTag:(NSInteger)tag;


enum {
    NSCorrectionIndicatorTypeDefault = 0,
    NSCorrectionIndicatorTypeReversion,
    NSCorrectionIndicatorTypeGuesses
};
typedef NSInteger NSCorrectionIndicatorType;

- (void)showCorrectionIndicatorOfType:(NSCorrectionIndicatorType)type primaryString:(NSString *)primaryString alternativeStrings:(NSArray *)alternativeStrings forStringInRect:(NSRect)rectOfTypedString view:(NSView *)view completionHandler:(void (^)(NSString *acceptedString))completionBlock;
- (void)dismissCorrectionIndicatorForView:(NSView *)view;



- (NSArray *)availableLanguages;
- (NSArray *)userPreferredLanguages;
- (BOOL)automaticallyIdentifiesLanguages;
- (void)setAutomaticallyIdentifiesLanguages:(BOOL)flag;


- (void)setWordFieldStringValue:(NSString *)aString;


- (void)learnWord:(NSString *)word;
- (BOOL)hasLearnedWord:(NSString *)word;
- (void)unlearnWord:(NSString *)word;


+ (BOOL)isAutomaticTextReplacementEnabled;
+ (BOOL)isAutomaticSpellingCorrectionEnabled;



- (NSString *)language;
- (BOOL)setLanguage:(NSString *)language;

@end


EXTERN_C NSString * const NSSpellCheckerDidChangeAutomaticSpellingCorrectionNotification;
EXTERN_C NSString * const NSSpellCheckerDidChangeAutomaticTextReplacementNotification;

@interface NSSpellChecker(NSDeprecated)


- (NSArray *)guessesForWord:(NSString *)word;


- (void)forgetWord:(NSString *)word;

@end
