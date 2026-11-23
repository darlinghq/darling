// Copyright (C) 2026 Darling Developers
//
// QTMovie is an NSObject subclass, and on the legacy (fragile) Objective-C ABI (i386) an application's
// subclass of it is compiled against the framework's instance size: on macOS that is 84 bytes (the
// class pointer and 20 words of the class's own state), and the subclass's ivars start there. The size
// was observed on macOS 10.14.4 (i386) with class_getInstanceSize(). This checks the superclass, that
// size (legacy ABI only; ivars slide on the non-fragile ABI) and that a subclass with ivars of its own
// keeps them intact while the movie is used. Works for i386 and x86_64:
//   guest-cc.sh -a i386 -o qtkit-movie-layout tests/qtkit-movie-layout.m -framework Foundation -framework QTKit
// Exits non-zero on failure.
#import <Foundation/Foundation.h>
#import <QTKit/QTKit.h>
#include <objc/runtime.h>
#include <stdio.h>

static int failures;

static void expect(BOOL condition, const char *what) {
	if (!condition) {
		printf("FAIL: %s\n", what);
		failures++;
	}
}

#define CANARY_WORDS 24
#define CANARY(i) (0xC0DE0000u + (unsigned) (i))

@interface MyMovie : QTMovie {
  @public
	unsigned _canary[CANARY_WORDS];
}
@end

@implementation MyMovie
@end

int main(void) {
	@autoreleasepool {
		expect(class_getSuperclass([QTMovie class]) == [NSObject class], "QTMovie superclass is NSObject");
#if !__OBJC2__
		expect(class_getInstanceSize([QTMovie class]) == 84, "QTMovie instance size is 84");
		Ivar canary = class_getInstanceVariable([MyMovie class], "_canary");
		expect(canary != NULL && ivar_getOffset(canary) == 84, "subclass ivars start at 84");
#endif

		MyMovie *movie = [[MyMovie alloc] init];
		expect(movie != nil, "QTMovie subclass init");
		for (int i = 0; i < CANARY_WORDS; i++)
			movie->_canary[i] = CANARY(i);

		// Use the movie: none of this may touch the subclass's variables.
		[movie setVolume:0.5f];
		[movie setMuted:YES];
		[movie setRate:0.0f];
		[movie play];
		[movie stop];
		[movie setCurrentTime:QTMakeTime(1, 1)];
		(void)[movie movieAttributes];
		(void)[movie duration];
		expect([movie volume] == 0.5f && [movie muted], "the movie's own state works");

		BOOL intact = YES;
		for (int i = 0; i < CANARY_WORDS; i++)
			intact &= movie->_canary[i] == CANARY(i);
		expect(intact, "the subclass ivars are intact");
		[movie release];

		// A movie that was allocated and released without being initialized must not crash.
		[[QTMovie alloc] release];
	}
	if (failures) {
		printf("%d failure(s)\n", failures);
		return 1;
	}
	printf("PASS: QTMovie instance layout\n");
	return 0;
}
