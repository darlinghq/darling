#import <AppKit/AppKitDefines.h>
#import <Foundation/NSObject.h>
#import <Foundation/NSRange.h>

@class NSArray, NSDictionary, NSString, NSURL, NSError;
@protocol NSSpeechSynthesizerDelegate;

EXTERN_C NSString *const NSVoiceName;
EXTERN_C NSString *const NSVoiceIdentifier;
EXTERN_C NSString *const NSVoiceAge;
EXTERN_C NSString *const NSVoiceGender;
EXTERN_C NSString *const NSVoiceDemoText;
EXTERN_C NSString *const NSVoiceLocaleIdentifier;
EXTERN_C NSString *const NSVoiceSupportedCharacters;
EXTERN_C NSString *const NSVoiceIndividuallySpokenCharacters;


EXTERN_C NSString *const NSVoiceGenderNeuter;
EXTERN_C NSString *const NSVoiceGenderMale;
EXTERN_C NSString *const NSVoiceGenderFemale;


EXTERN_C NSString *const NSVoiceLanguage;

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5

enum {
    NSSpeechImmediateBoundary = 0,
    NSSpeechWordBoundary,
    NSSpeechSentenceBoundary
};
#endif
typedef NSUInteger NSSpeechBoundary;


@interface NSSpeechSynthesizer : NSObject {
@private
    id _privateNSSpeechSynthesizerVars;
}

- (id)initWithVoice:(NSString *)voice;

- (BOOL)startSpeakingString:(NSString *)string;
- (BOOL)startSpeakingString:(NSString *)string toURL:(NSURL *)url;

- (BOOL)isSpeaking;
- (void)stopSpeaking;
- (void)stopSpeakingAtBoundary:(NSSpeechBoundary)boundary;
- (void)pauseSpeakingAtBoundary:(NSSpeechBoundary)boundary;
- (void)continueSpeaking;

- (id <NSSpeechSynthesizerDelegate>)delegate;
- (void)setDelegate:(id <NSSpeechSynthesizerDelegate>)anObject;
- (NSString *)voice;
- (BOOL)setVoice:(NSString *)voice;
- (float)rate;
- (void)setRate:(float)rate;
- (float)volume;
- (void)setVolume:(float)volume;
- (BOOL)usesFeedbackWindow;
- (void)setUsesFeedbackWindow:(BOOL)flag;

- (void)addSpeechDictionary:(NSDictionary*)speechDictionary;
- (NSString *)phonemesFromText:(NSString *)text;

- (id)objectForProperty:(NSString *)property error:(NSError **)outError;
- (BOOL)setObject:(id)object forProperty:(NSString *)property error:(NSError **)outError;

+ (BOOL)isAnyApplicationSpeaking;
+ (NSString *)defaultVoice;
+ (NSArray *)availableVoices;
+ (NSDictionary *)attributesForVoice:(NSString*)voice;

@end

@protocol NSSpeechSynthesizerDelegate <NSObject>
@optional
- (void)speechSynthesizer:(NSSpeechSynthesizer *)sender didFinishSpeaking:(BOOL)finishedSpeaking;
- (void)speechSynthesizer:(NSSpeechSynthesizer *)sender willSpeakWord:(NSRange)characterRange ofString:(NSString *)string;
- (void)speechSynthesizer:(NSSpeechSynthesizer *)sender willSpeakPhoneme:(short)phonemeOpcode;
- (void)speechSynthesizer:(NSSpeechSynthesizer *)sender didEncounterErrorAtIndex:(NSUInteger)characterIndex ofString:(NSString *)string message:(NSString *)message;
- (void)speechSynthesizer:(NSSpeechSynthesizer *)sender didEncounterSyncMessage:(NSString *)message;
@end


EXTERN_C NSString *const NSSpeechStatusProperty;
EXTERN_C NSString *const NSSpeechErrorsProperty;
EXTERN_C NSString *const NSSpeechInputModeProperty;
EXTERN_C NSString *const NSSpeechCharacterModeProperty;
EXTERN_C NSString *const NSSpeechNumberModeProperty;
EXTERN_C NSString *const NSSpeechRateProperty;
EXTERN_C NSString *const NSSpeechPitchBaseProperty;
EXTERN_C NSString *const NSSpeechPitchModProperty;
EXTERN_C NSString *const NSSpeechVolumeProperty;
EXTERN_C NSString *const NSSpeechSynthesizerInfoProperty;
EXTERN_C NSString *const NSSpeechRecentSyncProperty;
EXTERN_C NSString *const NSSpeechPhonemeSymbolsProperty;
EXTERN_C NSString *const NSSpeechCurrentVoiceProperty;
EXTERN_C NSString *const NSSpeechCommandDelimiterProperty;
EXTERN_C NSString *const NSSpeechResetProperty;
EXTERN_C NSString *const NSSpeechOutputToFileURLProperty;


EXTERN_C NSString *const NSSpeechModeText;
EXTERN_C NSString *const NSSpeechModePhoneme;


EXTERN_C NSString *const NSSpeechModeNormal;
EXTERN_C NSString *const NSSpeechModeLiteral;


EXTERN_C NSString *const NSSpeechStatusOutputBusy;
EXTERN_C NSString *const NSSpeechStatusOutputPaused;
EXTERN_C NSString *const NSSpeechStatusNumberOfCharactersLeft;
EXTERN_C NSString *const NSSpeechStatusPhonemeCode;


EXTERN_C NSString *const NSSpeechErrorCount;
EXTERN_C NSString *const NSSpeechErrorOldestCode;
EXTERN_C NSString *const NSSpeechErrorOldestCharacterOffset;
EXTERN_C NSString *const NSSpeechErrorNewestCode;
EXTERN_C NSString *const NSSpeechErrorNewestCharacterOffset;


EXTERN_C NSString *const NSSpeechSynthesizerInfoIdentifier;
EXTERN_C NSString *const NSSpeechSynthesizerInfoVersion;


EXTERN_C NSString *const NSSpeechPhonemeInfoOpcode;
EXTERN_C NSString *const NSSpeechPhonemeInfoSymbol;
EXTERN_C NSString *const NSSpeechPhonemeInfoExample;
EXTERN_C NSString *const NSSpeechPhonemeInfoHiliteStart;
EXTERN_C NSString *const NSSpeechPhonemeInfoHiliteEnd;


EXTERN_C NSString *const NSSpeechCommandPrefix;
EXTERN_C NSString *const NSSpeechCommandSuffix;


EXTERN_C NSString *const NSSpeechDictionaryLocaleIdentifier;
EXTERN_C NSString *const NSSpeechDictionaryModificationDate;
EXTERN_C NSString *const NSSpeechDictionaryPronunciations;
EXTERN_C NSString *const NSSpeechDictionaryAbbreviations;
EXTERN_C NSString *const NSSpeechDictionaryEntrySpelling;
EXTERN_C NSString *const NSSpeechDictionaryEntryPhonemes;
