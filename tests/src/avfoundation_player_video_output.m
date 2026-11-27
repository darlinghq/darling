// Copyright (C) 2026 Lubos Dolezel
//
// AVPlayer's preferredVideoDecoderGPURegistryID (default 0) and
// automaticallyWaitsToMinimizeStalling (default YES), and AVPlayerItemVideoOutput's
// initWithOutputSettings:, delegate and queue accessors, and item-time conversions.
// Defaults, the invalid time without an item, and the exception for an empty
// output-settings dictionary were observed on macOS 26.7.3 (black-box).
#import <AVFoundation/AVFoundation.h>
#import <CoreVideo/CoreVideo.h>
#include <mach/mach_time.h>
#include <stdio.h>

static int g_failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static double hostSeconds(void)
{
	mach_timebase_info_data_t tb;
	mach_timebase_info(&tb);
	return (double)mach_absolute_time() * tb.numer / tb.denom / 1e9;
}

static NSString *makeWav(NSString *dir)
{
	int rate = 8000, n = rate;
	NSMutableData *d = [NSMutableData data];
	uint32_t v; uint16_t s;
#define W32(x) v = (x); [d appendBytes:&v length:4]
#define W16(x) s = (x); [d appendBytes:&s length:2]
	[d appendBytes:"RIFF" length:4]; W32(36 + n * 2); [d appendBytes:"WAVEfmt " length:8];
	W32(16); W16(1); W16(1); W32(rate); W32(rate * 2); W16(2); W16(16);
	[d appendBytes:"data" length:4]; W32(n * 2);
	for (int i = 0; i < n; i++) { s = (uint16_t)(i * 37); [d appendBytes:&s length:2]; }
	NSString *path = [dir stringByAppendingPathComponent:@"tone.wav"];
	[d writeToFile:path atomically:NO];
	return path;
}

@interface Delegate : NSObject <AVPlayerItemOutputPullDelegate>
@end
@implementation Delegate
@end

int main(void)
{
	@autoreleasepool {
		AVPlayer *player = [[AVPlayer alloc] init];
		CHECK(player.preferredVideoDecoderGPURegistryID == 0);
		CHECK(player.automaticallyWaitsToMinimizeStalling == YES);
		player.automaticallyWaitsToMinimizeStalling = NO;
		CHECK(player.automaticallyWaitsToMinimizeStalling == NO);
		player.preferredVideoDecoderGPURegistryID = 0x123456789abcULL;
		CHECK(player.preferredVideoDecoderGPURegistryID == 0x123456789abcULL);
		AVPlayer *second = [AVPlayer playerWithPlayerItem:nil];
		CHECK(second.automaticallyWaitsToMinimizeStalling == YES);

		AVPlayerItemVideoOutput *output = [[AVPlayerItemVideoOutput alloc] initWithPixelBufferAttributes:nil];

		// Without an item the conversions have nothing to convert.
		CMTime t = [output itemTimeForHostTime:hostSeconds()];
		CHECK(!CMTIME_IS_VALID(t));
		CVTimeStamp stamp = { 0 };
		stamp.flags = kCVTimeStampHostTimeValid;
		stamp.hostTime = mach_absolute_time();
		CHECK(!CMTIME_IS_VALID([output itemTimeForCVTimeStamp:stamp]));

		// Delegate and queue are stored, the queue retained, and both can be cleared.
		CHECK(output.delegate == nil && output.delegateQueue == NULL);
		Delegate *delegate = [[Delegate alloc] init];
		dispatch_queue_t queue = dispatch_queue_create("test.output", DISPATCH_QUEUE_SERIAL);
		[output setDelegate:delegate queue:queue];
		dispatch_release(queue);
		CHECK(output.delegate == delegate);
		CHECK(output.delegateQueue != NULL);
		dispatch_sync(output.delegateQueue, ^{});
		[output setDelegate:nil queue:NULL];
		CHECK(output.delegate == nil && output.delegateQueue == NULL);
		[output requestNotificationOfMediaDataChangeWithAdvanceInterval:0.1];

		// Output settings: nil and pixel buffer keys are accepted, an empty dictionary is rejected.
		CHECK([[AVPlayerItemVideoOutput alloc] initWithOutputSettings:nil] != nil);
		CHECK([[AVPlayerItemVideoOutput alloc] initWithOutputSettings:
			@{ (id)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32BGRA) }] != nil);
		BOOL raised = NO;
		@try {
			[[AVPlayerItemVideoOutput alloc] initWithOutputSettings:@{}];
		} @catch (NSException *e) {
			raised = [[e name] isEqualToString:NSInvalidArgumentException];
		}
		CHECK(raised);

		// With an item in a paused player the item time stays where the player is.
		{
			NSString *path = makeWav(NSTemporaryDirectory());
			AVPlayerItem *item = [AVPlayerItem playerItemWithURL:[NSURL fileURLWithPath:path]];
			AVPlayer *p = [AVPlayer playerWithPlayerItem:item];
			[item addOutput:output];
			CHECK(p.rate == 0);
			CMTime at = [output itemTimeForHostTime:hostSeconds() + 5.0];
			CHECK(CMTIME_IS_VALID(at));
			CHECK(fabs(CMTimeGetSeconds(at) - CMTimeGetSeconds([p currentTime])) < 0.001);
			stamp.hostTime = mach_absolute_time();
			at = [output itemTimeForCVTimeStamp:stamp];
			CHECK(CMTIME_IS_VALID(at));
			CHECK(fabs(CMTimeGetSeconds(at) - CMTimeGetSeconds([p currentTime])) < 0.001);
			CVTimeStamp none = { 0 };
			CHECK(!CMTIME_IS_VALID([output itemTimeForCVTimeStamp:none]));
		}
	}
	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
