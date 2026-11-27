// CFLAGS: -framework DeviceCheck -framework Foundation
// Copyright (C) 2026 Lubos Dolezel
//
// DCAppAttestService reports App Attest as unavailable the documented way:
// +sharedService is a singleton, -isSupported is NO, DCErrorDomain and the DCError
// codes have the system's values, and every request fails through its completion
// handler off the calling thread, with no result. Key generation fails with
// DCErrorFeatureUnsupported; attestation and assertion fail with DCErrorInvalidInput
// (QUIRK: pinned here, see DCAppAttestService.m). Values and errors observed on
// macOS 26.7.3 on a machine where App Attest is unsupported.
#import <DeviceCheck/DeviceCheck.h>
#import <Foundation/Foundation.h>
#include <stdio.h>

static int g_failures = 0;
#define CHECK(c) do { if (!(c)) { printf("FAIL %d: %s\n", __LINE__, #c); g_failures++; } } while (0)

typedef struct { id result; NSError *error; BOOL onMain; BOOL called; } Outcome;

static void waitSem(dispatch_semaphore_t sem)
{
	CHECK(dispatch_semaphore_wait(sem, dispatch_time(DISPATCH_TIME_NOW, 10 * NSEC_PER_SEC)) == 0);
}

int main(void) {
	@autoreleasepool {
		DCAppAttestService *s1 = [DCAppAttestService sharedService];
		CHECK(s1 != nil);
		CHECK(s1 == DCAppAttestService.sharedService);
		CHECK([s1 isSupported] == NO);
		CHECK(s1.supported == NO);

		CHECK([DCErrorDomain isEqualToString:@"com.apple.devicecheck.error"]);
		CHECK(DCErrorUnknownSystemFailure == 0 && DCErrorFeatureUnsupported == 1 && DCErrorInvalidInput == 2);
		CHECK(DCErrorInvalidKey == 3 && DCErrorServerUnavailable == 4);

		__block Outcome out;
		dispatch_semaphore_t sem = dispatch_semaphore_create(0);
		NSString *keyId = [[NSData dataWithBytes:"0123456789abcdef0123456789abcdef" length:32] base64EncodedStringWithOptions:0];
		NSData *hash = [NSData dataWithBytes:"0123456789abcdef0123456789abcdef" length:32];

		memset(&out, 0, sizeof(out));
		[s1 generateKeyWithCompletionHandler:^(NSString *key, NSError *error) {
			out.result = [key retain]; out.error = [error retain]; out.onMain = [NSThread isMainThread]; out.called = YES;
			dispatch_semaphore_signal(sem);
		}];
		waitSem(sem);
		CHECK(out.called && out.result == nil);
		CHECK(out.error != nil && [[out.error domain] isEqualToString:DCErrorDomain] && [out.error code] == DCErrorFeatureUnsupported);
		CHECK(!out.onMain);

		memset(&out, 0, sizeof(out));
		[s1 attestKey:keyId clientDataHash:hash completionHandler:^(NSData *data, NSError *error) {
			out.result = [data retain]; out.error = [error retain]; out.onMain = [NSThread isMainThread]; out.called = YES;
			dispatch_semaphore_signal(sem);
		}];
		waitSem(sem);
		CHECK(out.called && out.result == nil);
		CHECK(out.error != nil && [[out.error domain] isEqualToString:DCErrorDomain] && [out.error code] == DCErrorInvalidInput);
		CHECK(!out.onMain);

		memset(&out, 0, sizeof(out));
		[s1 generateAssertion:keyId clientDataHash:hash completionHandler:^(NSData *data, NSError *error) {
			out.result = [data retain]; out.error = [error retain]; out.onMain = [NSThread isMainThread]; out.called = YES;
			dispatch_semaphore_signal(sem);
		}];
		waitSem(sem);
		CHECK(out.called && out.result == nil);
		CHECK(out.error != nil && [[out.error domain] isEqualToString:DCErrorDomain] && [out.error code] == DCErrorInvalidInput);

		memset(&out, 0, sizeof(out));
		[s1 attestKey:@"" clientDataHash:[NSData data] completionHandler:^(NSData *data, NSError *error) {
			out.result = [data retain]; out.error = [error retain]; out.called = YES;
			dispatch_semaphore_signal(sem);
		}];
		waitSem(sem);
		CHECK(out.called && out.result == nil && [out.error code] == DCErrorInvalidInput);
	}
	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
