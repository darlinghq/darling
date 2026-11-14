/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _AVSPEECHSYNTHESIS_H_
#define _AVSPEECHSYNTHESIS_H_

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSInteger, AVSpeechBoundary) {
	AVSpeechBoundaryImmediate,
	AVSpeechBoundaryWord
};

typedef NS_ENUM(NSInteger, AVSpeechSynthesisVoiceQuality) {
	AVSpeechSynthesisVoiceQualityDefault = 1,
	AVSpeechSynthesisVoiceQualityEnhanced,
	AVSpeechSynthesisVoiceQualityPremium
};

typedef NS_ENUM(NSInteger, AVSpeechSynthesisVoiceGender) {
	AVSpeechSynthesisVoiceGenderUnspecified,
	AVSpeechSynthesisVoiceGenderMale,
	AVSpeechSynthesisVoiceGenderFemale
};

typedef NS_ENUM(NSUInteger, AVSpeechSynthesisPersonalVoiceAuthorizationStatus) {
	AVSpeechSynthesisPersonalVoiceAuthorizationStatusNotDetermined,
	AVSpeechSynthesisPersonalVoiceAuthorizationStatusDenied,
	AVSpeechSynthesisPersonalVoiceAuthorizationStatusUnsupported,
	AVSpeechSynthesisPersonalVoiceAuthorizationStatusAuthorized
};

extern const float AVSpeechUtteranceMinimumSpeechRate;
extern const float AVSpeechUtteranceMaximumSpeechRate;
extern const float AVSpeechUtteranceDefaultSpeechRate;

extern NSString *const AVSpeechSynthesisVoiceIdentifierAlex;
extern NSString *const AVSpeechSynthesisIPANotationAttribute;
extern NSNotificationName AVSpeechSynthesisAvailableVoicesDidChangeNotification;

@protocol AVSpeechSynthesizerDelegate;

// A voice. Darling has no speech engine, so the list of voices is empty and no voice object is
// ever handed out.
@interface AVSpeechSynthesisVoice : NSObject <NSSecureCoding> {
@private
	NSString *_language;
	NSString *_identifier;
	NSString *_name;
	AVSpeechSynthesisVoiceQuality _quality;
	AVSpeechSynthesisVoiceGender _gender;
}
+ (NSArray<AVSpeechSynthesisVoice *> *)speechVoices;
+ (NSString *)currentLanguageCode;
+ (nullable AVSpeechSynthesisVoice *)voiceWithLanguage:(nullable NSString *)languageCode;
+ (nullable AVSpeechSynthesisVoice *)voiceWithIdentifier:(NSString *)identifier;
@property (nonatomic, readonly, copy) NSString *language;
@property (nonatomic, readonly, copy) NSString *identifier;
@property (nonatomic, readonly, copy) NSString *name;
@property (nonatomic, readonly) AVSpeechSynthesisVoiceQuality quality;
@property (nonatomic, readonly) AVSpeechSynthesisVoiceGender gender;
@end

// One piece of text to speak, with the way to speak it.
@interface AVSpeechUtterance : NSObject <NSCopying, NSSecureCoding> {
@private
	NSAttributedString *_attributedString;
	AVSpeechSynthesisVoice *_voice;
	float _rate;
	float _pitchMultiplier;
	float _volume;
	BOOL _prefersAssistiveTechnologySettings;
	NSTimeInterval _preUtteranceDelay;
	NSTimeInterval _postUtteranceDelay;
}
+ (instancetype)speechUtteranceWithString:(NSString *)string;
+ (instancetype)speechUtteranceWithAttributedString:(NSAttributedString *)string;
- (instancetype)initWithString:(NSString *)string;
- (instancetype)initWithAttributedString:(NSAttributedString *)string;
@property (nonatomic, retain, nullable) AVSpeechSynthesisVoice *voice;
@property (nonatomic, readonly, copy) NSString *speechString;
@property (nonatomic, readonly, copy) NSAttributedString *attributedSpeechString;
@property (nonatomic) float rate;
@property (nonatomic) float pitchMultiplier;
@property (nonatomic) float volume;
@property (nonatomic) BOOL prefersAssistiveTechnologySettings;
@property (nonatomic) NSTimeInterval preUtteranceDelay;
@property (nonatomic) NSTimeInterval postUtteranceDelay;
@end

// Speaks utterances one after the other. Darling has no speech engine: an utterance completes as
// soon as its turn comes, without producing sound.
@interface AVSpeechSynthesizer : NSObject {
@private
	id<AVSpeechSynthesizerDelegate> _delegate;
	NSMutableArray *_queue;  // utterances that have not finished; the first one is current
	BOOL _paused;
	BOOL _drainScheduled;
}
@property (nonatomic, assign, nullable) id<AVSpeechSynthesizerDelegate> delegate;
@property (nonatomic, readonly, getter=isSpeaking) BOOL speaking;
@property (nonatomic, readonly, getter=isPaused) BOOL paused;
- (void)speakUtterance:(AVSpeechUtterance *)utterance;
- (BOOL)stopSpeakingAtBoundary:(AVSpeechBoundary)boundary;
- (BOOL)pauseSpeakingAtBoundary:(AVSpeechBoundary)boundary;
- (BOOL)continueSpeaking;
+ (AVSpeechSynthesisPersonalVoiceAuthorizationStatus)personalVoiceAuthorizationStatus;
+ (void)requestPersonalVoiceAuthorizationWithCompletionHandler:(void (^)(AVSpeechSynthesisPersonalVoiceAuthorizationStatus status))handler;
@end

@protocol AVSpeechSynthesizerDelegate <NSObject>
@optional
- (void)speechSynthesizer:(AVSpeechSynthesizer *)synthesizer didStartSpeechUtterance:(AVSpeechUtterance *)utterance;
- (void)speechSynthesizer:(AVSpeechSynthesizer *)synthesizer didFinishSpeechUtterance:(AVSpeechUtterance *)utterance;
- (void)speechSynthesizer:(AVSpeechSynthesizer *)synthesizer didPauseSpeechUtterance:(AVSpeechUtterance *)utterance;
- (void)speechSynthesizer:(AVSpeechSynthesizer *)synthesizer didContinueSpeechUtterance:(AVSpeechUtterance *)utterance;
- (void)speechSynthesizer:(AVSpeechSynthesizer *)synthesizer didCancelSpeechUtterance:(AVSpeechUtterance *)utterance;
@end

NS_ASSUME_NONNULL_END

#endif
