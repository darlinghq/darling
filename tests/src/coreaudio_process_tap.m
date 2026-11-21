// Copyright (C) 2026 Lubos Dolezel
//
// The process tap API of CoreAudio (macOS 14.2 and later): the two calls and the tap properties of the
// system object. Darling cannot capture the output of other processes, so there creating a tap
// is refused; macOS would ask for the user's permission, so a real creation is tried only with the
// argument "create" (Darling runs).
//
// Checks (the same on macOS 26.7.3 for everything that does not create a tap):
//   - creating a tap without a description is kAudioHardwareIllegalOperationError ('nope') and leaves the ID alone;
//   - destroying an unknown object ID is noErr;
//   - the system object has the tap list property, with no entries, and translating a UID to a tap gives
//     kAudioObjectUnknown;
//   - the tap constants have the documented values;
//   - "create": a description of a mono mixdown of no process is refused as an unsupported operation
//     (Darling), with the ID set to kAudioObjectUnknown.
//
// build: guest-cc.sh -o t coreaudio_process_tap.m -framework Foundation -framework CoreAudio -fobjc-arc
// Prints PASS/FAIL lines; the exit status is the number of failures.

#import <Foundation/Foundation.h>
#import <CoreAudio/CoreAudio.h>
#import <CoreAudio/AudioHardwareTapping.h>
#import <CoreAudio/CATapDescription.h>
#include <string.h>

static int failures;

static void expect(BOOL ok, NSString *what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", [what UTF8String]);
	if (!ok)
		failures++;
}

int main(int argc, char **argv)
{
	@autoreleasepool {
		expect(kAudioHardwarePropertyTapList == 'tps#' && kAudioHardwarePropertyTranslateUIDToTap == 'uidt'
			&& kAudioTapClassID == 'tcls' && kAudioTapPropertyUID == 'tuid' && kAudioTapPropertyDescription == 'tdsc'
			&& kAudioTapPropertyFormat == 'tfmt', @"tap constants");

		AudioObjectID tap = 77;
		OSStatus status = AudioHardwareCreateProcessTap(nil, &tap);
		expect(status == kAudioHardwareIllegalOperationError, [NSString stringWithFormat:@"create without a description: %d", (int)status]);
		expect(tap == 77, @"the tap ID is left alone");

		expect(AudioHardwareDestroyProcessTap(kAudioObjectUnknown) == noErr, @"destroy kAudioObjectUnknown");
		expect(AudioHardwareDestroyProcessTap(12345) == noErr, @"destroy an unused ID");

		AudioObjectPropertyAddress list = { kAudioHardwarePropertyTapList, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMain };
		UInt32 size = 99;
		expect(AudioObjectHasProperty(kAudioObjectSystemObject, &list), @"the system object has the tap list");
		status = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &list, 0, NULL, &size);
		expect(status == noErr && size == 0, [NSString stringWithFormat:@"tap list size: %d, %u", (int)status, (unsigned)size]);

		AudioObjectPropertyAddress translate = { kAudioHardwarePropertyTranslateUIDToTap, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMain };
		CFStringRef uid = CFSTR("no-such-tap-uid");
		AudioObjectID out = 55;
		size = sizeof(out);
		status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &translate, sizeof(uid), &uid, &size, &out);
		expect(status == noErr && out == kAudioObjectUnknown && size == sizeof(AudioObjectID),
			[NSString stringWithFormat:@"translate an unknown UID: %d, %u, %u", (int)status, (unsigned)out, (unsigned)size]);

		if (argc > 1 && !strcmp(argv[1], "create")) {
			CATapDescription *description = [[CATapDescription alloc] initMonoMixdownOfProcesses:@[]];
			tap = 77;
			status = AudioHardwareCreateProcessTap(description, &tap);
			expect(status == kAudioHardwareUnsupportedOperationError && tap == kAudioObjectUnknown,
				[NSString stringWithFormat:@"create: %d, %u", (int)status, (unsigned)tap]);
		}
	}
	return failures;
}
