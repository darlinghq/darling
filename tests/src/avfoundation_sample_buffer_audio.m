// Copyright (C) 2026 Lubos Dolezel
//
// AVSampleBufferRenderSynchronizer (rate, time, notifications, observers, renderer list) and
// AVSampleBufferAudioRenderer (status, flush, formats it cannot render, rendering through the
// default output unit). Built with -DMAC_PROBE, the program leaves out the iOS-only parts
// (AVAudioSession, RemoteIO) so it can run on macOS for comparison.
#import <AVFoundation/AVFoundation.h>
#import <AudioToolbox/AudioToolbox.h>
#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CoreVideo.h>
#import <QuartzCore/QuartzCore.h>
#include <stdio.h>
#include <unistd.h>

static int g_failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

#define CHECK_TIME(cond, t) do { if (!(cond)) { printf("FAIL %d: %s (t = %f)\n", __LINE__, #cond, (double)(t)); g_failures++; } } while (0)

static void spin(double seconds)
{
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:seconds]];
}

static CMSampleBufferRef makePCM(const AudioStreamBasicDescription *asbd, size_t frames)
{
	CMAudioFormatDescriptionRef format = NULL;
	if (CMAudioFormatDescriptionCreate(NULL, asbd, 0, NULL, 0, NULL, NULL, &format) != noErr)
		return NULL;
	size_t length = frames * asbd->mBytesPerFrame;
	CMBlockBufferRef block = NULL;
	CMBlockBufferCreateWithMemoryBlock(NULL, NULL, length, NULL, NULL, 0, length, kCMBlockBufferAssureMemoryNowFlag, &block);
	CMBlockBufferFillDataBytes(0, block, 0, length);
	CMSampleBufferRef sb = NULL;
	OSStatus status = CMSampleBufferCreate(NULL, block, true, NULL, NULL, format, frames, 0, NULL, 0, NULL, &sb);
	CFRelease(block);
	CFRelease(format);
	return status == noErr ? sb : NULL;
}

static void testSynchronizer(void)
{
	AVSampleBufferRenderSynchronizer *sync = [[AVSampleBufferRenderSynchronizer alloc] init];
	CHECK(sync.rate == 0.0f);
	CHECK(CMTimeCompare(sync.currentTime, kCMTimeZero) == 0);
	CHECK(sync.renderers.count == 0);

	__block int rateNotes = 0;
	id token = [[NSNotificationCenter defaultCenter]
		addObserverForName:AVSampleBufferRenderSynchronizerRateDidChangeNotification object:sync queue:nil
		usingBlock:^(NSNotification *n) { rateNotes++; }];

	[sync setRate:1.0f time:CMTimeMake(5, 1)];
	CHECK(sync.rate == 1.0f);
	CHECK(CMTimeGetSeconds(sync.currentTime) >= 5.0);
	spin(0.3);
	double t = CMTimeGetSeconds(sync.currentTime);
	CHECK_TIME(t >= 5.25 && t < 5.9, t);

	[sync setRate:0.0f];
	double frozen = CMTimeGetSeconds(sync.currentTime);
	spin(0.2);
	CHECK(CMTimeGetSeconds(sync.currentTime) == frozen);

	[sync setRate:2.0f time:CMTimeMake(10, 1)];
	spin(0.3);
	t = CMTimeGetSeconds(sync.currentTime);
	CHECK_TIME(t >= 10.5 && t < 11.5, t);
	CHECK(rateNotes == 3);
	[sync setRate:2.0f];
	CHECK(rateNotes == 3); // unchanged rate posts nothing
	[[NSNotificationCenter defaultCenter] removeObserver:token];

	// A boundary observer fires once when the time passes it; a periodic one reports while running.
	__block int boundary = 0, periodic = 0;
	dispatch_queue_t q = dispatch_queue_create("sync-test", NULL);
	[sync setRate:1.0f time:CMTimeMake(20, 1)];
	id b = [sync addBoundaryTimeObserverForTimes:@[ [NSValue valueWithCMTime:CMTimeMake(2010, 100)] ] queue:q
		usingBlock:^{ boundary++; }];
	id p = [sync addPeriodicTimeObserverForInterval:CMTimeMake(1, 10) queue:q usingBlock:^(CMTime time) { periodic++; }];
	spin(1.0);
	CHECK(boundary == 1);
	CHECK(periodic >= 3);
	[sync removeTimeObserver:b];
	[sync removeTimeObserver:p];
	int seen = periodic;
	spin(0.4);
	CHECK(periodic <= seen + 1);

	// Renderers: adding one twice raises (macOS 26.7.3), removal reports whether it was present.
	AVSampleBufferAudioRenderer *r = [[AVSampleBufferAudioRenderer alloc] init];
	[sync addRenderer:r];
	CHECK(sync.renderers.count == 1);
	@try {
		[sync addRenderer:r];
		CHECK(0);
	} @catch (NSException *e) {
		CHECK([e.name isEqualToString:NSInternalInconsistencyException]);
	}
	CHECK(sync.renderers.count == 1);
	__block int removed = -1;
	[sync removeRenderer:r atTime:kCMTimeZero completionHandler:^(BOOL did) { removed = did; }];
	spin(0.3);
	CHECK(removed == 1);
	CHECK(sync.renderers.count == 0);
	removed = -1;
	[sync removeRenderer:r atTime:kCMTimeZero completionHandler:^(BOOL did) { removed = did; }];
	spin(0.3);
	CHECK(removed == 0);

	@try {
		[sync addPeriodicTimeObserverForInterval:kCMTimeZero queue:q usingBlock:^(CMTime time) {}];
		CHECK(0);
	} @catch (NSException *e) {
		CHECK([e.name isEqualToString:NSInvalidArgumentException]);
	}

	// Releasing the synchronizer while an observer is live must not leave the timer running.
	AVSampleBufferRenderSynchronizer *gone = [[AVSampleBufferRenderSynchronizer alloc] init];
	[gone setRate:1.0f];
	[gone addPeriodicTimeObserverForInterval:CMTimeMake(1, 50) queue:q usingBlock:^(CMTime time) {}];
	[gone release];
	spin(0.2);

	[r release];
	[sync release];
	dispatch_release(q);
}

static void testRenderer(void)
{
	AVSampleBufferAudioRenderer *r = [[AVSampleBufferAudioRenderer alloc] init];
	CHECK(r.status == AVQueuedSampleBufferRenderingStatusUnknown);
	CHECK(r.error == nil);
	CHECK(r.volume == 1.0f);
	CHECK(!r.muted);
	CHECK(r.readyForMoreMediaData);
	r.volume = 0.5f;
	r.muted = YES;
	CHECK(r.volume == 0.5f && r.muted);

	// Compressed audio cannot be rendered: the renderer fails with an error and a flush does not clear it (macOS 26.7.3).
	AudioStreamBasicDescription aac = { 44100, kAudioFormatMPEG4AAC, 0, 0, 1024, 0, 2, 0, 0 };
	CMAudioFormatDescriptionRef format = NULL;
	CHECK(CMAudioFormatDescriptionCreate(NULL, &aac, 0, NULL, 0, NULL, NULL, &format) == noErr);
	CMSampleBufferRef bad = NULL;
	CMBlockBufferRef block = NULL;
	CMBlockBufferCreateWithMemoryBlock(NULL, NULL, 16, NULL, NULL, 0, 16, kCMBlockBufferAssureMemoryNowFlag, &block);
	if (format && block)
		CMSampleBufferCreate(NULL, block, true, NULL, NULL, format, 1, 0, NULL, 0, NULL, &bad);
	if (bad) {
		[r enqueueSampleBuffer:bad];
		CHECK(r.status == AVQueuedSampleBufferRenderingStatusFailed);
		CHECK(r.error != nil);
		CHECK(!r.readyForMoreMediaData);
		[r flush];
		CHECK(r.status == AVQueuedSampleBufferRenderingStatusFailed);
		CHECK(r.error != nil);
		CFRelease(bad);
	}
	if (block) CFRelease(block);
	if (format) CFRelease(format);

	// A failed renderer stays failed, so linear PCM goes to a new one: it is ready for more data until
	// about half a second is queued.
	[r release];
	r = [[AVSampleBufferAudioRenderer alloc] init];
	AudioStreamBasicDescription pcm = { 44100, kAudioFormatLinearPCM,
		kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked, 4, 1, 4, 2, 16, 0 };
	AVSampleBufferRenderSynchronizer *sync = [[AVSampleBufferRenderSynchronizer alloc] init];
	[sync addRenderer:r];
	CMSampleBufferRef sb = makePCM(&pcm, 44100);
	CHECK(sb != NULL);
	if (sb) {
		[r enqueueSampleBuffer:sb];
		// With no audio device the renderer fails; with one it renders. Either way it is not idle.
		CHECK(r.status != AVQueuedSampleBufferRenderingStatusUnknown);
		if (r.status == AVQueuedSampleBufferRenderingStatusRendering) {
			CHECK(!r.readyForMoreMediaData); // a second of audio is queued, the target is half of that
			CHECK(r.hasSufficientMediaDataForReliablePlaybackStart);
			// Playing drains the queue and asks for more through the request block.
			__block int requests = 0;
			dispatch_queue_t q = dispatch_queue_create("renderer-test", NULL);
			[r requestMediaDataWhenReadyOnQueue:q usingBlock:^{ requests++; }];
			[sync setRate:1.0f];
			spin(1.5);
			CHECK(requests >= 1);
			CHECK(r.readyForMoreMediaData);
			[sync setRate:0.0f];
			[r stopRequestingMediaData];
			dispatch_release(q);
			[r flush];
			CHECK(r.status == AVQueuedSampleBufferRenderingStatusRendering);
		}
		CFRelease(sb);
	}
	[sync removeRenderer:r atTime:kCMTimeZero completionHandler:nil];
	[sync release];
	[r release];
}

static CMSampleBufferRef makeFrame(OSType pixelFormat)
{
	CVPixelBufferRef pixels = NULL;
	if (CVPixelBufferCreate(NULL, 8, 8, pixelFormat, NULL, &pixels) != kCVReturnSuccess)
		return NULL;
	CMVideoFormatDescriptionRef format = NULL;
	CMVideoFormatDescriptionCreateForImageBuffer(NULL, pixels, &format);
	CMSampleTimingInfo timing = { CMTimeMake(1, 30), kCMTimeZero, kCMTimeInvalid };
	CMSampleBufferRef sb = NULL;
	CMSampleBufferCreateReadyWithImageBuffer(NULL, pixels, format, &timing, &sb);
	if (format) CFRelease(format);
	CFRelease(pixels);
	return sb;
}

static void testDisplayLayer(void)
{
	AVSampleBufferDisplayLayer *layer = [[AVSampleBufferDisplayLayer alloc] init];
	CHECK(layer.status == AVQueuedSampleBufferRenderingStatusUnknown);
	CHECK(layer.error == nil);
	CHECK([layer.videoGravity isEqualToString:AVLayerVideoGravityResizeAspect]);
	CHECK(layer.readyForMoreMediaData);

	CMSampleBufferRef frame = makeFrame(kCVPixelFormatType_32BGRA);
	CHECK(frame != NULL);
	if (frame) {
		[layer enqueueSampleBuffer:frame];
		CHECK(layer.status == AVQueuedSampleBufferRenderingStatusRendering);
#ifndef MAC_PROBE
		// Darling shows the frame as the layer's contents; macOS keeps the image internally.
		spin(0.3);
		CHECK(layer.contents != nil);
		[layer flushAndRemoveImage];
		spin(0.3);
		CHECK(layer.contents == nil);
#endif
		CFRelease(frame);
	}

#ifndef MAC_PROBE
	// Limitation: a frame whose pixels Core Graphics cannot draw (biplanar YCbCr) fails the layer, with
	// a notification carrying the error; macOS displays it.
	__block NSError *notified = nil;
	id token = [[NSNotificationCenter defaultCenter]
		addObserverForName:AVSampleBufferDisplayLayerFailedToDecodeNotification object:layer queue:nil
		usingBlock:^(NSNotification *n) {
			notified = [n.userInfo[AVSampleBufferDisplayLayerFailedToDecodeNotificationErrorKey] retain];
		}];
	CMSampleBufferRef bad = makeFrame(kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange);
	if (bad) {
		[layer enqueueSampleBuffer:bad];
		CHECK(layer.status == AVQueuedSampleBufferRenderingStatusFailed);
		CHECK(layer.error != nil);
		CHECK(notified != nil);
		CFRelease(bad);
		[layer flush];
		CHECK(layer.status == AVQueuedSampleBufferRenderingStatusUnknown);
	}
	[[NSNotificationCenter defaultCenter] removeObserver:token];
	[notified release];
#endif
	[layer release];
}

#ifndef MAC_PROBE
static void testRemoteIO(void)
{
	AudioComponentDescription remote = { kAudioUnitType_Output, kAudioUnitSubType_RemoteIO, kAudioUnitManufacturer_Apple, 0, 0 };
	AudioComponentDescription hal = { kAudioUnitType_Output, kAudioUnitSubType_HALOutput, kAudioUnitManufacturer_Apple, 0, 0 };
	AudioComponentDescription def = { kAudioUnitType_Output, kAudioUnitSubType_DefaultOutput, kAudioUnitManufacturer_Apple, 0, 0 };
	AudioComponent r = AudioComponentFindNext(NULL, &remote);
	CHECK(r != NULL);
	CHECK(r == AudioComponentFindNext(NULL, &hal));
	CHECK(AudioComponentCount(&remote) == AudioComponentCount(&hal));
	AudioUnit unit = NULL;
	CHECK(r && AudioComponentInstanceNew(r, &unit) == noErr);
	if (unit)
		AudioComponentInstanceDispose(unit);
	// Additive: the default output unit that macOS callers use is unchanged.
	CHECK(AudioComponentFindNext(NULL, &def) != r);
	CHECK(kAudioUnitSubType_RemoteIO == 'rioc');
}

static void testAudioSession(void)
{
	AVAudioSession *s = [AVAudioSession sharedInstance];
	CHECK(s != nil && s == [AVAudioSession sharedInstance]);
	NSError *error = nil;
	CHECK([s setCategory:AVAudioSessionCategoryPlayback withOptions:AVAudioSessionCategoryOptionMixWithOthers error:&error]);
	CHECK(s.categoryOptions == AVAudioSessionCategoryOptionMixWithOthers);
	CHECK([s.category isEqualToString:AVAudioSessionCategoryPlayback]);
	CHECK([s setCategory:AVAudioSessionCategoryPlayAndRecord mode:AVAudioSessionModeDefault
		routeSharingPolicy:AVAudioSessionRouteSharingPolicyLongFormAudio
		options:AVAudioSessionCategoryOptionDefaultToSpeaker | AVAudioSessionCategoryOptionAllowBluetooth error:&error]);
	CHECK(s.routeSharingPolicy == AVAudioSessionRouteSharingPolicyLongFormAudio);
	CHECK(s.categoryOptions == (AVAudioSessionCategoryOptionDefaultToSpeaker | AVAudioSessionCategoryOptionAllowBluetooth));
	CHECK([s setActive:YES withOptions:AVAudioSessionSetActiveOptionNotifyOthersOnDeactivation error:&error]);
	CHECK([s setActive:NO withOptions:AVAudioSessionSetActiveOptionNotifyOthersOnDeactivation error:&error]);
	CHECK([s setSupportsMultichannelContent:YES error:&error] && s.supportsMultichannelContent);
	CHECK([s setPreferredOutputNumberOfChannels:6 error:&error]);
	CHECK(s.maximumOutputNumberOfChannels >= 2 && s.maximumInputNumberOfChannels >= 1);
	CHECK(AVAudioSessionCategoryOptionInterruptSpokenAudioAndMixWithOthers == 0x11);
	CHECK(AVAudioSessionCategoryOptionAllowBluetoothA2DP == 0x20);
	CHECK(AVAudioSessionPortOverrideSpeaker == 'spkr');
	CHECK(AVAudioSessionInterruptionTypeBegan == 1 && AVAudioSessionInterruptionTypeEnded == 0);
	CHECK(AVAudioSessionRouteChangeReasonNoSuitableRouteForCategory == 7);
	CHECK(AVAudioSessionRouteSharingPolicyLongForm == AVAudioSessionRouteSharingPolicyLongFormAudio);
}
#endif

int main(void)
{
	@autoreleasepool {
		testSynchronizer();
		testRenderer();
		testDisplayLayer();
#ifndef MAC_PROBE
		testRemoteIO();
		testAudioSession();
#endif
	}
	if (g_failures == 0)
		printf("PASS\n");
	return g_failures != 0;
}
