// Copyright (C) 2026 Lubos Dolezel
//
// Symbols that newer SDKs import from CoreAudio, AVFAudio, GameController, CoreVideo, CoreMedia
// and VideoToolbox: block based audio property listeners, CATapDescription, AVSpeech*,
// GCHaptics*, CVPixelBufferPoolFlush and constants. Expected values are the ones the SDK
// headers declare and macOS 26.7.3 returns (the program passes unchanged there, except
// the "speak" check, which only runs with the argument "speak" because macOS would talk).
//
// build: guest-cc.sh -o t newer-sdk-symbols.m -framework Foundation -framework AVFAudio
//   -framework GameController -framework CoreVideo -framework CoreMedia -framework VideoToolbox
//   -framework CoreAudio -fobjc-arc
// Prints PASS/FAIL lines; the exit status is the number of failures.

#import <Foundation/Foundation.h>
#import <AVFAudio/AVFAudio.h>
#import <GameController/GameController.h>
#import <CoreVideo/CoreVideo.h>
#import <CoreMedia/CoreMedia.h>
#import <VideoToolbox/VideoToolbox.h>
#import <CoreAudio/CoreAudio.h>
#import <CoreAudio/CATapDescription.h>

static int failures;

static void expect(BOOL ok, NSString *what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", [what UTF8String]);
	if (!ok)
		failures++;
}

static BOOL str(CFStringRef value, const char *expected)
{
	return value != NULL && [(__bridge NSString *)value isEqualToString:[NSString stringWithUTF8String:expected]];
}

@interface SpeechDelegate : NSObject <AVSpeechSynthesizerDelegate>
@property NSMutableArray *events;
@end

@implementation SpeechDelegate
- (void)speechSynthesizer:(AVSpeechSynthesizer *)s didStartSpeechUtterance:(AVSpeechUtterance *)u { [self.events addObject:[@"start " stringByAppendingString:u.speechString]]; }
- (void)speechSynthesizer:(AVSpeechSynthesizer *)s didFinishSpeechUtterance:(AVSpeechUtterance *)u { [self.events addObject:[@"finish " stringByAppendingString:u.speechString]]; }
@end

static void checkConstants(void)
{
	expect(str(kCVImageBufferAmbientViewingEnvironmentKey, "AmbientViewingEnvironment"), @"kCVImageBufferAmbientViewingEnvironmentKey");
	expect(str(kCMSampleBufferAttachmentKey_ResetDecoderBeforeDecoding, "ResetDecoderBeforeDecoding"), @"kCMSampleBufferAttachmentKey_ResetDecoderBeforeDecoding");
	expect(str(kVTCompressionPropertyKey_EncoderID, "EncoderID"), @"kVTCompressionPropertyKey_EncoderID");
	expect(str(kVTEncodeFrameOptionKey_BaseFrameQP, "BaseFrameQP"), @"kVTEncodeFrameOptionKey_BaseFrameQP");
	expect(str(kVTProfileLevel_HEVC_Main_AutoLevel, "HEVC_Main_AutoLevel"), @"kVTProfileLevel_HEVC_Main_AutoLevel");
	expect((int)kCVPixelBufferPoolFlushExcessBuffers == 1, @"kCVPixelBufferPoolFlushExcessBuffers is 1");

	expect(GCHapticDurationInfinite == 1e6f, @"GCHapticDurationInfinite is 1000000");
	expect([GCHapticsLocalityDefault isEqual:@"Default"] && [GCHapticsLocalityAll isEqual:@"All"] && [GCHapticsLocalityHandles isEqual:@"Handles"], @"default, all and handles localities");
	expect([GCHapticsLocalityLeftHandle isEqual:@"Left Handle"] && [GCHapticsLocalityRightHandle isEqual:@"Right Handle"], @"handle localities");
	expect([GCHapticsLocalityTriggers isEqual:@"Triggers"] && [GCHapticsLocalityLeftTrigger isEqual:@"Left Trigger"] && [GCHapticsLocalityRightTrigger isEqual:@"Right Trigger"], @"trigger localities");

	expect(AVSpeechUtteranceMinimumSpeechRate == 0.0f && AVSpeechUtteranceDefaultSpeechRate == 0.5f && AVSpeechUtteranceMaximumSpeechRate == 1.0f, @"speech rates");
	expect([AVSpeechSynthesisVoiceIdentifierAlex isEqual:@"com.apple.speech.synthesis.voice.Alex"], @"AVSpeechSynthesisVoiceIdentifierAlex");
	expect([AVSpeechSynthesisIPANotationAttribute isEqual:@"AVSpeechSynthesisIPANotationAttribute"], @"AVSpeechSynthesisIPANotationAttribute");
	expect([AVSpeechSynthesisAvailableVoicesDidChangeNotification isEqual:@"AVSpeechSynthesisAvailableVoicesDidChangeNotification"], @"voices changed notification name");
	expect(AVSpeechSynthesisVoiceQualityDefault == 1 && AVSpeechSynthesisVoiceQualityEnhanced == 2 && AVSpeechSynthesisVoiceQualityPremium == 3, @"voice quality values");
	expect(AVSpeechSynthesisVoiceGenderUnspecified == 0 && AVSpeechSynthesisVoiceGenderMale == 1 && AVSpeechSynthesisVoiceGenderFemale == 2, @"voice gender values");
	expect(AVSpeechBoundaryImmediate == 0 && AVSpeechBoundaryWord == 1, @"boundary values");
	expect(CATapUnmuted == 0 && CATapMuted == 1 && CATapMutedWhenTapped == 2, @"CATapMuteBehavior values");
}

static void checkPixelBufferPool(void)
{
	NSDictionary *attributes = @{ (id)kCVPixelBufferWidthKey: @16, (id)kCVPixelBufferHeightKey: @16, (id)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32BGRA) };
	CVPixelBufferPoolRef pool = NULL;
	expect(CVPixelBufferPoolCreate(NULL, NULL, (__bridge CFDictionaryRef)attributes, &pool) == kCVReturnSuccess && pool != NULL, @"pixel buffer pool created");
	CVPixelBufferRef buffer = NULL;
	CVPixelBufferPoolCreatePixelBuffer(NULL, pool, &buffer);
	CVPixelBufferPoolFlush(pool, 0);
	CVPixelBufferPoolFlush(pool, kCVPixelBufferPoolFlushExcessBuffers);
	expect(buffer != NULL && CVPixelBufferGetWidth(buffer) == 16, @"a buffer in use survives a flush");
	if (buffer)
		CVPixelBufferRelease(buffer);
	CVPixelBufferPoolFlush(pool, kCVPixelBufferPoolFlushExcessBuffers);
	buffer = NULL;
	expect(CVPixelBufferPoolCreatePixelBuffer(NULL, pool, &buffer) == kCVReturnSuccess && buffer != NULL, @"the pool still hands out buffers after a flush");
	if (buffer)
		CVPixelBufferRelease(buffer);
	CVPixelBufferPoolRelease(pool);
}

static void checkAudioListenerBlocks(void)
{
	AudioObjectPropertyAddress address = { kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMain };
	AudioObjectPropertyListenerBlock block = ^(UInt32 count, const AudioObjectPropertyAddress *addresses) { (void)count; (void)addresses; };
	dispatch_queue_t queue = dispatch_queue_create("listener-test", DISPATCH_QUEUE_SERIAL);

	expect(AudioObjectAddPropertyListenerBlock(kAudioObjectSystemObject, &address, queue, block) == noErr, @"AudioObjectAddPropertyListenerBlock on a queue");
	expect(AudioObjectRemovePropertyListenerBlock(kAudioObjectSystemObject, &address, queue, block) == noErr, @"AudioObjectRemovePropertyListenerBlock with the same queue and block");
	expect(AudioObjectAddPropertyListenerBlock(kAudioObjectSystemObject, &address, NULL, block) == noErr, @"AudioObjectAddPropertyListenerBlock without a queue");
	expect(AudioObjectRemovePropertyListenerBlock(kAudioObjectSystemObject, &address, NULL, block) == noErr, @"AudioObjectRemovePropertyListenerBlock without a queue");
	expect(AudioObjectRemovePropertyListenerBlock(kAudioObjectSystemObject, &address, NULL, block) == noErr, @"removing a listener that is no longer registered succeeds (quirk)");
}

static void checkTapDescription(void)
{
	CATapDescription *t = [[CATapDescription alloc] init];
	expect(t.UUID != nil && t.processes.count == 0 && !t.mono && !t.exclusive && !t.mixdown && t.privateTap && t.muteBehavior == CATapUnmuted && t.deviceUID == nil && t.stream == nil, @"-init defaults");
	t = [[CATapDescription alloc] initStereoMixdownOfProcesses:@[@7]];
	expect([t.processes isEqual:@[@7]] && !t.mono && !t.exclusive && t.mixdown, @"stereo mixdown of processes");
	t = [[CATapDescription alloc] initMonoMixdownOfProcesses:@[@7]];
	expect(t.mono && !t.exclusive && t.mixdown, @"mono mixdown of processes");
	t = [[CATapDescription alloc] initStereoGlobalTapButExcludeProcesses:@[@7]];
	expect(!t.mono && t.exclusive && t.mixdown, @"stereo global tap excluding processes");
	t = [[CATapDescription alloc] initMonoGlobalTapButExcludeProcesses:@[@7]];
	expect(t.mono && t.exclusive && t.mixdown, @"mono global tap excluding processes");
	t = [[CATapDescription alloc] initWithProcesses:@[@7] andDeviceUID:@"dev" withStream:2];
	expect(!t.exclusive && !t.mixdown && [t.deviceUID isEqual:@"dev"] && [t.stream isEqual:@2], @"processes of a device stream");
	t = [[CATapDescription alloc] initExcludingProcesses:@[@7] andDeviceUID:@"dev" withStream:2];
	expect(t.exclusive && !t.mixdown && [t.deviceUID isEqual:@"dev"] && [t.stream isEqual:@2], @"all but processes of a device stream");
	t.name = @"tap";
	t.muteBehavior = CATapMuted;
	t.privateTap = NO;
	expect([t.name isEqual:@"tap"] && t.muteBehavior == CATapMuted && !t.privateTap, @"properties can be set");
	expect(![[[CATapDescription alloc] init].UUID isEqual:[[CATapDescription alloc] init].UUID], @"each description has its own UUID");
}

static void checkSpeech(BOOL speak)
{
	AVSpeechUtterance *u = [AVSpeechUtterance speechUtteranceWithString:@"hi"];
	expect([u.speechString isEqual:@"hi"] && u.rate == 0.5f && u.pitchMultiplier == 1.0f && u.volume == 1.0f && u.preUtteranceDelay == 0 && u.postUtteranceDelay == 0 && !u.prefersAssistiveTechnologySettings && u.voice == nil, @"utterance defaults");
	u.rate = 0.25f;
	AVSpeechUtterance *copy = [u copy];
	expect(copy != u && copy.rate == 0.25f && [copy.speechString isEqual:@"hi"], @"copying an utterance keeps its settings");

	AVSpeechSynthesizer *synth = [[AVSpeechSynthesizer alloc] init];
	expect(!synth.speaking && !synth.paused && synth.delegate == nil, @"idle synthesizer");
	expect(![synth stopSpeakingAtBoundary:AVSpeechBoundaryImmediate] && ![synth pauseSpeakingAtBoundary:AVSpeechBoundaryImmediate] && ![synth continueSpeaking], @"stop, pause and continue return NO when idle");
	expect([AVSpeechSynthesisVoice voiceWithLanguage:@"zz-ZZ-xx"] == nil, @"an invalid language has no voice");
	expect([[AVSpeechSynthesisVoice currentLanguageCode] length] >= 2, @"current language code");
	expect([AVSpeechSynthesizer personalVoiceAuthorizationStatus] == AVSpeechSynthesisPersonalVoiceAuthorizationStatusUnsupported, @"personal voice status");

	if (speak)
	{
		SpeechDelegate *delegate = [[SpeechDelegate alloc] init];
		delegate.events = [NSMutableArray array];
		synth.delegate = delegate;
		[synth speakUtterance:[AVSpeechUtterance speechUtteranceWithString:@"one"]];
		[synth speakUtterance:[AVSpeechUtterance speechUtteranceWithString:@"two"]];
		NSDate *deadline = [NSDate dateWithTimeIntervalSinceNow:5];
		while (delegate.events.count < 4 && [deadline timeIntervalSinceNow] > 0)
			[[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.05]];
		NSArray *expected = @[@"start one", @"finish one", @"start two", @"finish two"];
		expect([delegate.events isEqual:expected], @"queued utterances start and finish in order");
		expect(!synth.speaking, @"not speaking once the queue has finished");
	}
}

int main(int argc, char **argv)
{
	@autoreleasepool {
		BOOL speak = argc > 1 && strcmp(argv[1], "speak") == 0;
		checkConstants();
		checkPixelBufferPool();
		checkAudioListenerBlocks();
		checkTapDescription();
		checkSpeech(speak);
	}
	printf("%s\n", failures ? "FAILED" : "ALL PASSED");
	return failures;
}
