// Copyright (C) 2026 Lubos Dolezel
//
// Classes of AudioVideoBridging and CoreAudioKit that clients link against: AVBConfigurationURL and
// AVBVirtualAudioConfiguration (NSObject subclasses that can be instantiated) and
// CANetworkMIDIViewController (an NSViewController subclass). Same results on macOS 26.7.3.
//
// build: guest-cc.sh -o t audio_private_classes.m -framework Foundation -framework AppKit
//   -framework AudioVideoBridging -framework CoreAudioKit -fobjc-arc
// Prints PASS/FAIL lines; the exit status is the number of failures.

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <objc/runtime.h>
#import <dlfcn.h>

static int failures;

static void expect(BOOL ok, NSString *what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", [what UTF8String]);
	if (!ok)
		failures++;
}

int main(void)
{
	@autoreleasepool {
		const char *frameworks[] = {
			"/System/Library/Frameworks/AudioVideoBridging.framework/AudioVideoBridging",
			"/System/Library/Frameworks/CoreAudioKit.framework/CoreAudioKit",
		};
		for (int i = 0; i < 2; i++)
			expect(dlopen(frameworks[i], RTLD_NOW) != NULL, [NSString stringWithFormat:@"load %s", frameworks[i]]);

		const char *objectClasses[] = { "AVBConfigurationURL", "AVBVirtualAudioConfiguration" };
		for (int i = 0; i < 2; i++) {
			Class c = objc_getClass(objectClasses[i]);
			expect(c != Nil, [NSString stringWithFormat:@"%s exists", objectClasses[i]]);
			if (c != Nil) {
				expect(class_getSuperclass(c) == [NSObject class], [NSString stringWithFormat:@"%s derives from NSObject", objectClasses[i]]);
				expect([[c alloc] init] != nil, [NSString stringWithFormat:@"%s can be instantiated", objectClasses[i]]);
			}
		}

		Class controller = objc_getClass("CANetworkMIDIViewController");
		expect(controller != Nil, @"CANetworkMIDIViewController exists");
		if (controller != Nil)
			expect(class_getSuperclass(controller) == [NSViewController class], @"CANetworkMIDIViewController derives from NSViewController");
	}
	return failures;
}
