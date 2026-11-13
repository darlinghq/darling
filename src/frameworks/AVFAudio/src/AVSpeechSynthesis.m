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

#import <AVFAudio/AVSpeechSynthesis.h>
#import <dispatch/dispatch.h>

const float AVSpeechUtteranceMinimumSpeechRate = 0.0f;
const float AVSpeechUtteranceMaximumSpeechRate = 1.0f;
const float AVSpeechUtteranceDefaultSpeechRate = 0.5f;

NSString *const AVSpeechSynthesisVoiceIdentifierAlex = @"com.apple.speech.synthesis.voice.Alex";
NSString *const AVSpeechSynthesisIPANotationAttribute = @"AVSpeechSynthesisIPANotationAttribute";
NSNotificationName AVSpeechSynthesisAvailableVoicesDidChangeNotification = @"AVSpeechSynthesisAvailableVoicesDidChangeNotification";

@implementation AVSpeechSynthesisVoice

@synthesize language = _language;
@synthesize identifier = _identifier;
@synthesize name = _name;
@synthesize quality = _quality;
@synthesize gender = _gender;

+ (NSArray<AVSpeechSynthesisVoice *> *)speechVoices
{
	return @[];
}

// The language and region of the current locale as a BCP-47 tag, for example en-US.
+ (NSString *)currentLanguageCode
{
	NSLocale *locale = [NSLocale currentLocale];
	NSString *language = [locale objectForKey:NSLocaleLanguageCode];
	NSString *region = [locale objectForKey:NSLocaleCountryCode];
	if (language == nil)
		return @"en-US";
	return region != nil ? [NSString stringWithFormat:@"%@-%@", language, region] : language;
}

+ (AVSpeechSynthesisVoice *)voiceWithLanguage:(NSString *)languageCode
{
	return nil;
}

+ (AVSpeechSynthesisVoice *)voiceWithIdentifier:(NSString *)identifier
{
	return nil;
}

+ (BOOL)supportsSecureCoding
{
	return YES;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
	[coder encodeObject:_identifier forKey:@"AVSpeechSynthesisVoiceIdentifier"];
}

// Voices are looked up by identifier, and there are none to find.
- (instancetype)initWithCoder:(NSCoder *)coder
{
	[self release];
	return nil;
}

- (void)dealloc
{
	[_language release];
	[_identifier release];
	[_name release];
	[super dealloc];
}

@end

@implementation AVSpeechUtterance {
	NSAttributedString *_attributedString;
}

@synthesize voice = _voice;
@synthesize rate = _rate;
@synthesize pitchMultiplier = _pitchMultiplier;
@synthesize volume = _volume;
@synthesize prefersAssistiveTechnologySettings = _prefersAssistiveTechnologySettings;
@synthesize preUtteranceDelay = _preUtteranceDelay;
@synthesize postUtteranceDelay = _postUtteranceDelay;

+ (instancetype)speechUtteranceWithString:(NSString *)string
{
	return [[[self alloc] initWithString:string] autorelease];
}

+ (instancetype)speechUtteranceWithAttributedString:(NSAttributedString *)string
{
	return [[[self alloc] initWithAttributedString:string] autorelease];
}

- (instancetype)initWithAttributedString:(NSAttributedString *)string
{
	self = [super init];
	if (self != nil)
	{
		_attributedString = [string copy];
		_rate = AVSpeechUtteranceDefaultSpeechRate;
		_pitchMultiplier = 1.0f;
		_volume = 1.0f;
	}
	return self;
}

- (instancetype)initWithString:(NSString *)string
{
	NSAttributedString *attributed = [[NSAttributedString alloc] initWithString:string];
	self = [self initWithAttributedString:attributed];
	[attributed release];
	return self;
}

- (NSString *)speechString
{
	return [_attributedString string];
}

- (NSAttributedString *)attributedSpeechString
{
	return _attributedString;
}

- (id)copyWithZone:(NSZone *)zone
{
	AVSpeechUtterance *copy = [[[self class] allocWithZone:zone] initWithAttributedString:_attributedString];
	copy.voice = _voice;
	copy.rate = _rate;
	copy.pitchMultiplier = _pitchMultiplier;
	copy.volume = _volume;
	copy.prefersAssistiveTechnologySettings = _prefersAssistiveTechnologySettings;
	copy.preUtteranceDelay = _preUtteranceDelay;
	copy.postUtteranceDelay = _postUtteranceDelay;
	return copy;
}

+ (BOOL)supportsSecureCoding
{
	return YES;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
	[coder encodeObject:[_attributedString string] forKey:@"AVSpeechUtteranceString"];
	[coder encodeFloat:_rate forKey:@"AVSpeechUtteranceRate"];
	[coder encodeFloat:_pitchMultiplier forKey:@"AVSpeechUtterancePitchMultiplier"];
	[coder encodeFloat:_volume forKey:@"AVSpeechUtteranceVolume"];
	[coder encodeBool:_prefersAssistiveTechnologySettings forKey:@"AVSpeechUtterancePrefersAssistiveTechnologySettings"];
	[coder encodeDouble:_preUtteranceDelay forKey:@"AVSpeechUtterancePreDelay"];
	[coder encodeDouble:_postUtteranceDelay forKey:@"AVSpeechUtterancePostDelay"];
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
	NSString *string = [coder decodeObjectOfClass:[NSString class] forKey:@"AVSpeechUtteranceString"];
	if (string == nil)
	{
		[self release];
		return nil;
	}
	self = [self initWithString:string];
	if (self != nil)
	{
		_rate = [coder decodeFloatForKey:@"AVSpeechUtteranceRate"];
		_pitchMultiplier = [coder decodeFloatForKey:@"AVSpeechUtterancePitchMultiplier"];
		_volume = [coder decodeFloatForKey:@"AVSpeechUtteranceVolume"];
		_prefersAssistiveTechnologySettings = [coder decodeBoolForKey:@"AVSpeechUtterancePrefersAssistiveTechnologySettings"];
		_preUtteranceDelay = [coder decodeDoubleForKey:@"AVSpeechUtterancePreDelay"];
		_postUtteranceDelay = [coder decodeDoubleForKey:@"AVSpeechUtterancePostDelay"];
	}
	return self;
}

- (void)dealloc
{
	[_attributedString release];
	[_voice release];
	[super dealloc];
}

@end

@implementation AVSpeechSynthesizer {
	NSMutableArray *_queue;  // utterances that have not finished; the first one is current
	BOOL _paused;
	BOOL _drainScheduled;
}

@synthesize delegate = _delegate;

- (instancetype)init
{
	self = [super init];
	if (self != nil)
		_queue = [[NSMutableArray alloc] init];
	return self;
}

- (void)dealloc
{
	[_queue release];
	[super dealloc];
}

- (BOOL)isSpeaking
{
	@synchronized (self)
	{
		return [_queue count] != 0;
	}
}

- (BOOL)isPaused
{
	@synchronized (self)
	{
		return _paused;
	}
}

// Utterances are processed on the main queue, where delegates are called.
- (void)_scheduleDrain
{
	@synchronized (self)
	{
		if (_drainScheduled)
			return;
		_drainScheduled = YES;
	}
	[self retain];
	dispatch_async(dispatch_get_main_queue(), ^{
		[self _drain];
		[self release];
	});
}

// There is no speech engine, so the current utterance starts and ends at once.
- (void)_drain
{
	AVSpeechUtterance *utterance = nil;
	@synchronized (self)
	{
		_drainScheduled = NO;
		if (_paused || [_queue count] == 0)
			return;
		utterance = [[_queue objectAtIndex:0] retain];
	}
	id<AVSpeechSynthesizerDelegate> delegate = _delegate;
	if ([delegate respondsToSelector:@selector(speechSynthesizer:didStartSpeechUtterance:)])
		[delegate speechSynthesizer:self didStartSpeechUtterance:utterance];
	@synchronized (self)
	{
		if ([_queue count] != 0 && [_queue objectAtIndex:0] == utterance)
			[_queue removeObjectAtIndex:0];
	}
	if ([delegate respondsToSelector:@selector(speechSynthesizer:didFinishSpeechUtterance:)])
		[delegate speechSynthesizer:self didFinishSpeechUtterance:utterance];
	[utterance release];
	@synchronized (self)
	{
		if ([_queue count] == 0)
			return;
	}
	[self _scheduleDrain];
}

- (void)speakUtterance:(AVSpeechUtterance *)utterance
{
	@synchronized (self)
	{
		if ([_queue indexOfObjectIdenticalTo:utterance] != NSNotFound)
			return;
		[_queue addObject:utterance];
	}
	[self _scheduleDrain];
}

- (BOOL)stopSpeakingAtBoundary:(AVSpeechBoundary)boundary
{
	NSArray *cancelled;
	@synchronized (self)
	{
		if ([_queue count] == 0)
			return NO;
		cancelled = [_queue copy];
		[_queue removeAllObjects];
		_paused = NO;
	}
	id<AVSpeechSynthesizerDelegate> delegate = _delegate;
	if ([delegate respondsToSelector:@selector(speechSynthesizer:didCancelSpeechUtterance:)])
	{
		for (AVSpeechUtterance *utterance in cancelled)
			[delegate speechSynthesizer:self didCancelSpeechUtterance:utterance];
	}
	[cancelled release];
	return YES;
}

- (BOOL)pauseSpeakingAtBoundary:(AVSpeechBoundary)boundary
{
	AVSpeechUtterance *current;
	@synchronized (self)
	{
		if (_paused || [_queue count] == 0)
			return NO;
		_paused = YES;
		current = [[_queue objectAtIndex:0] retain];
	}
	id<AVSpeechSynthesizerDelegate> delegate = _delegate;
	if ([delegate respondsToSelector:@selector(speechSynthesizer:didPauseSpeechUtterance:)])
		[delegate speechSynthesizer:self didPauseSpeechUtterance:current];
	[current release];
	return YES;
}

- (BOOL)continueSpeaking
{
	AVSpeechUtterance *current = nil;
	@synchronized (self)
	{
		if (!_paused)
			return NO;
		_paused = NO;
		if ([_queue count] != 0)
			current = [[_queue objectAtIndex:0] retain];
	}
	id<AVSpeechSynthesizerDelegate> delegate = _delegate;
	if (current != nil && [delegate respondsToSelector:@selector(speechSynthesizer:didContinueSpeechUtterance:)])
		[delegate speechSynthesizer:self didContinueSpeechUtterance:current];
	[current release];
	[self _scheduleDrain];
	return YES;
}

// Personal voices need an engine that has them.
+ (AVSpeechSynthesisPersonalVoiceAuthorizationStatus)personalVoiceAuthorizationStatus
{
	return AVSpeechSynthesisPersonalVoiceAuthorizationStatusUnsupported;
}

+ (void)requestPersonalVoiceAuthorizationWithCompletionHandler:(void (^)(AVSpeechSynthesisPersonalVoiceAuthorizationStatus status))handler
{
	void (^copy)(AVSpeechSynthesisPersonalVoiceAuthorizationStatus) = [handler copy];
	dispatch_async(dispatch_get_main_queue(), ^{
		copy(AVSpeechSynthesisPersonalVoiceAuthorizationStatusUnsupported);
		[copy release];
	});
}

@end
