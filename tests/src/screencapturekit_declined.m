// Copyright (C) 2026 Lubos Dolezel
//
// ScreenCaptureKit has no capture backend: every capture request completes
// asynchronously, with no content and with SCStreamErrorUserDeclined (-3801),
// the error an app gets when the user denies screen recording. Configuration
// objects keep the values apps set on them.
#import <ScreenCaptureKit/ScreenCaptureKit.h>
#include <dispatch/dispatch.h>
#include <stdio.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL: %s (line %d)\n", #cond, __LINE__); failures++; } } while (0)

int main(void)
{
	@autoreleasepool {
		CHECK(SCStreamErrorAttemptToUpdateFilterState == -3809);
		CHECK(SCStreamErrorAttemptToConfigState == -3810);
		CHECK(SCStreamErrorInternalError == -3811);
		CHECK(SCStreamErrorInvalidParameter == -3812);
		CHECK(SCStreamErrorNoWindowList == -3813);
		CHECK(SCStreamErrorNoDisplayList == -3814);
		CHECK(SCStreamErrorNoCaptureSource == -3815);
		CHECK(SCStreamErrorRemovingStream == -3816);
		CHECK(SCStreamErrorUserStopped == -3817);
		CHECK(SCStreamErrorFailedToStartAudioCapture == -3818);
		CHECK(SCStreamErrorFailedToStopAudioCapture == -3819);
		CHECK(SCStreamErrorFailedToStartMicrophoneCapture == -3820);
		CHECK(SCStreamErrorSystemStoppedStream == -3821);
		CHECK(SCFrameStatusComplete == 0 && SCFrameStatusIdle == 1 && SCFrameStatusBlank == 2);
		CHECK(SCFrameStatusSuspended == 3 && SCFrameStatusStarted == 4 && SCFrameStatusStopped == 5);
		CHECK([SCStreamFrameInfoStatus isEqualToString:@"Status"]);
		CHECK([SCStreamFrameInfoDisplayTime isEqualToString:@"DisplayTime"]);
		CHECK([SCStreamFrameInfoScaleFactor isEqualToString:@"ScaleFactor"]);
		CHECK([SCStreamFrameInfoContentScale isEqualToString:@"ContentScale"]);
		CHECK([SCStreamFrameInfoContentRect isEqualToString:@"ContentRect"]);
		CHECK([SCStreamFrameInfoBoundingRect isEqualToString:@"BoundingRect"]);
		CHECK([SCStreamFrameInfoScreenRect isEqualToString:@"ScreenRect"]);
		CHECK([SCStreamFrameInfoDirtyRects isEqualToString:@"DirtyRects"]);
		CHECK([SCStreamFrameInfoPresenterOverlayContentRect isEqualToString:@"PresenterOverlayContentRect"]);

		dispatch_semaphore_t sem = dispatch_semaphore_create(0);
		__block SCShareableContent* content = (SCShareableContent*)1;
		__block NSError* error = nil;
		__block BOOL calledInline = YES;
		__block BOOL returned = NO;

		[SCShareableContent getShareableContentWithCompletionHandler:^(SCShareableContent* c, NSError* e) {
			calledInline = !returned;
			content = c;
			error = [e retain];
			dispatch_semaphore_signal(sem);
		}];
		returned = YES;

		long timedOut = dispatch_semaphore_wait(sem, dispatch_time(DISPATCH_TIME_NOW, 10 * NSEC_PER_SEC));
		CHECK(timedOut == 0);
		CHECK(!calledInline);
		CHECK(content == nil);
		CHECK(error != nil);
		CHECK([[error domain] isEqualToString:SCStreamErrorDomain]);
		CHECK([error code] == SCStreamErrorUserDeclined);
		CHECK([error code] == -3801);
		[error release];

		// A stream cannot be started either.
		SCStreamConfiguration* config = [[SCStreamConfiguration alloc] init];
		SCStream* stream = [[SCStream alloc] initWithFilter:nil configuration:config delegate:nil];
		__block NSError* startError = nil;
		[stream startCaptureWithCompletionHandler:^(NSError* e) {
			startError = [e retain];
			dispatch_semaphore_signal(sem);
		}];
		timedOut = dispatch_semaphore_wait(sem, dispatch_time(DISPATCH_TIME_NOW, 10 * NSEC_PER_SEC));
		CHECK(timedOut == 0);
		CHECK([startError code] == SCStreamErrorUserDeclined);
		[startError release];

		NSError* addError = nil;
		CHECK(![stream addStreamOutput:nil type:SCStreamOutputTypeScreen sampleHandlerQueue:nil error:&addError]);
		CHECK([addError code] == SCStreamErrorUserDeclined);

		// Completion handlers are optional.
		[stream stopCaptureWithCompletionHandler:nil];

		// Properties round-trip.
		config.width = 640;
		config.height = 480;
		config.showsCursor = YES;
		config.queueDepth = 5;
		CHECK(config.width == 640 && config.height == 480);
		CHECK(config.showsCursor == YES);
		CHECK(config.queueDepth == 5);

		[stream release];
		[config release];
		dispatch_release(sem);
	}

	if (failures) {
		return 1;
	}
	puts("OK");
	return 0;
}
