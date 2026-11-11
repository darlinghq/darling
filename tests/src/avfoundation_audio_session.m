// Copyright (C) 2026 Lubos Dolezel
//
// AVFoundation re-exports AVFAudio (AVAudioEngine resolves through an
// AVFoundation-only link), AVAudioSession and the settings keys are exported
// with the macOS values, selector-arity stubs accept arguments, and the
// AVFoundation notification and capture constants have the macOS values.
// Values and exports observed on macOS 27 (arm64).
#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>
#include <dlfcn.h>
#include <objc/message.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_STR(sym, expected) do { if (![(sym) isEqualToString:(expected)]) { printf("FAIL %d: %s = %s\n", __LINE__, #sym, [(sym) UTF8String]); g_failures++; } } while (0)

static NSString *lookup(const char *name)
{
	NSString *__unsafe_unretained *p = (NSString *__unsafe_unretained *) dlsym(RTLD_DEFAULT, name);
	return p ? *p : nil;
}

#define CHECK_SYM(name, expected) do { NSString *v = lookup(name); if (v == nil || ![v isEqualToString:(expected)]) { printf("FAIL %d: %s = %s\n", __LINE__, name, v ? [v UTF8String] : "(missing)"); g_failures++; } } while (0)

int main(void)
{
	@autoreleasepool {
		// Notifications
		CHECK_STR(AVPlayerItemDidPlayToEndTimeNotification, @"AVPlayerItemDidPlayToEndTimeNotification");
		CHECK_STR(AVPlayerItemFailedToPlayToEndTimeNotification, @"AVPlayerItemFailedToPlayToEndTimeNotification");
		CHECK_STR(AVPlayerItemPlaybackStalledNotification, @"AVPlayerItemPlaybackStalledNotification");

		// Capture constants
		CHECK_STR(AVCaptureSessionPresetPhoto, @"AVCaptureSessionPresetPhoto");
		CHECK_STR(AVCaptureDeviceTypeBuiltInWideAngleCamera, @"AVCaptureDeviceTypeBuiltInWideAngleCamera");
		CHECK_STR(AVCaptureDeviceTypeMicrophone, @"AVCaptureDeviceTypeMicrophone");
		CHECK_SYM("AVCaptureDeviceTypeBuiltInMicrophone", @"AVCaptureDeviceTypeMicrophone");
		CHECK_STR(AVCaptureDeviceTypeExternal, @"AVCaptureDeviceTypeExternal");
		CHECK_SYM("AVCaptureDeviceTypeExternalUnknown", @"AVCaptureDeviceTypeExternal");

		// AVAudioSettings keys
		CHECK_SYM("AVEncoderAudioQualityKey", @"AVEncoderQualityKey");
		CHECK_SYM("AVEncoderAudioQualityForVBRKey", @"AVEncoderQualityForVBRKey");
		CHECK_SYM("AVSampleRateConverterAudioQualityKey", @"AVSampleRateConverterQualityKey");
		CHECK_STR(AVFormatIDKey, @"AVFormatIDKey");
		CHECK_STR(AVAudioFileTypeKey, @"AVAudioFileTypeKey");

		// AVAudioSession constants (declared unavailable in the macOS SDK, but
		// exported, so they are looked up by name)
		CHECK_SYM("AVAudioSessionPortBuiltInMic", @"MicrophoneBuiltIn");
		CHECK_SYM("AVAudioSessionPortHDMI", @"HDMIOutput");
		CHECK_SYM("AVAudioSessionOrientationFront", @"Front");
		CHECK_SYM("AVAudioSessionPolarPatternCardioid", @"Cardioid");
		CHECK_SYM("AVAudioSessionCategoryPlayback", @"AVAudioSessionCategoryPlayback");
		CHECK_SYM("AVAudioSessionModeDefault", @"AVAudioSessionModeDefault");

		id session = [(id) NSClassFromString(@"AVAudioSession") performSelector:@selector(sharedInstance)];
		CHECK(session != nil);
		CHECK(session == [(id) NSClassFromString(@"AVAudioSession") performSelector:@selector(sharedInstance)]);
		CHECK_STR([session performSelector:@selector(category)], lookup("AVAudioSessionCategorySoloAmbient"));
		NSError *err = nil;
		BOOL (*setCategory)(id, SEL, NSString *, NSError **) = (BOOL (*)(id, SEL, NSString *, NSError **)) objc_msgSend;
		CHECK(setCategory(session, @selector(setCategory:error:), lookup("AVAudioSessionCategoryPlayback"), &err));
		CHECK_STR([session performSelector:@selector(category)], lookup("AVAudioSessionCategoryPlayback"));
		BOOL (*setActive)(id, SEL, BOOL, NSError **) = (BOOL (*)(id, SEL, BOOL, NSError **)) objc_msgSend;
		CHECK(setActive(session, @selector(setActive:error:), YES, &err));

		// Reachable through AVFoundation alone
		CHECK(NSClassFromString(@"AVAudioEngine") != Nil);
		CHECK(NSClassFromString(@"AVAudioPlayerNode") != Nil);
		CHECK(NSClassFromString(@"AVAudioFormat") != Nil);

		// A call with an argument must not raise NSForwardSignatureError
		AVAudioPlayerNode *node = [[AVAudioPlayerNode alloc] init];
		@try {
			((void (*)(id, SEL, float)) objc_msgSend)(node, @selector(setVolume:), 0.5f);
		} @catch (NSException *e) {
			printf("FAIL: setVolume: raised %s\n", [[e name] UTF8String]);
			g_failures++;
		}
	}

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
