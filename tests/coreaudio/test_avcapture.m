// tests/coreaudio/test_avcapture.m
// Checks AVCaptureDevice's authorization API inside Darling. Needs a default input device.
#import <AVFoundation/AVCaptureDevice.h>
#include <dispatch/dispatch.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define REQUIRE(cond) do { if (!(cond)) { printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); exit(1); } } while (0)

int main(void)
{
	REQUIRE([AVCaptureDevice authorizationStatusForMediaType: AVMediaTypeAudio] == AVAuthorizationStatusAuthorized);
	REQUIRE([AVCaptureDevice authorizationStatusForMediaType: AVMediaTypeVideo] == AVAuthorizationStatusRestricted);
	REQUIRE([[AVCaptureDevice devices] count] == 0);
	REQUIRE([AVCaptureDevice defaultDeviceWithMediaType: AVMediaTypeVideo] == nil);

	BOOL raised = NO;
	@try
	{
		[AVCaptureDevice authorizationStatusForMediaType: AVMediaTypeMuxed];
	}
	@catch (NSException* exception)
	{
		raised = [[exception name] isEqualToString: NSInvalidArgumentException];
	}
	REQUIRE(raised);

	// The handler must not run inside the call, so this lock must not be taken yet.
	static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
	dispatch_semaphore_t done = dispatch_semaphore_create(0);
	__block BOOL audioGranted = NO, videoGranted = YES;

	pthread_mutex_lock(&lock);
	[AVCaptureDevice requestAccessForMediaType: AVMediaTypeAudio completionHandler: ^(BOOL granted) {
		pthread_mutex_lock(&lock);
		audioGranted = granted;
		pthread_mutex_unlock(&lock);
		dispatch_semaphore_signal(done);
	}];
	pthread_mutex_unlock(&lock);
	REQUIRE(dispatch_semaphore_wait(done, dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC)) == 0);
	REQUIRE(audioGranted);

	[AVCaptureDevice requestAccessForMediaType: AVMediaTypeVideo completionHandler: ^(BOOL granted) {
		videoGranted = granted;
		dispatch_semaphore_signal(done);
	}];
	REQUIRE(dispatch_semaphore_wait(done, dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC)) == 0);
	REQUIRE(!videoGranted);

	printf("test_avcapture: OK\n");
	return 0;
}
