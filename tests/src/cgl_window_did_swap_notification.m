// Copyright (C) 2026 Lubos Dolezel
//
// CGLFlushDrawable() on a context whose drawable is a window surface
// (CGLGetWindowForContext) posts kCGLWindowDidSwapNotification to
// CoreFoundation's local notification center, on the swapping thread, with the
// native window as the object; a swap of a context without a window surface
// posts nothing, and neither does a swap after CGLDestroyWindow.
#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/CGLInternal.h>
#import <OpenGL/gl.h>
#include <stdio.h>

@interface NSWindow (SubWindowFixture)
- (CGSubWindow *) _createSubWindowWithFrame: (CGRect) frame;
@end

static int g_posts;
static void *g_object;

static void observe(CFNotificationCenterRef center, void *observer, CFNotificationName name,
                    const void *object, CFDictionaryRef info)
{
	g_posts++;
	g_object = (void *) object;
}

int main(void)
{
	@autoreleasepool {
		[NSApplication sharedApplication];
		NSWindow *w = [[NSWindow alloc] initWithContentRect:NSMakeRect(100, 100, 100, 80)
		                                          styleMask:NSWindowStyleMaskTitled
		                                            backing:NSBackingStoreBuffered defer:NO];
		[w makeKeyAndOrderFront:nil];
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:1.0]];
		CGSubWindow *child = [w _createSubWindowWithFrame:CGRectMake(0, 0, 100, 80)];
		CGLContextObj ctx = NULL;
		if (child == nil || CGLCreateContext(NULL, NULL, &ctx) != kCGLNoError) { printf("SKIP\n"); return 0; }
		void *native = [child nativeWindow];
		CGLWindowRef window = CGLGetWindowForContext(ctx, native);
		int failures = 0;
		if (window == NULL || CGLContextMakeCurrentAndAttachToWindow(ctx, window) != kCGLNoError) { printf("SKIP\n"); return 0; }
		CFNotificationCenterAddObserver(CFNotificationCenterGetLocalCenter(), &g_posts, observe,
			kCGLWindowDidSwapNotification, NULL, CFNotificationSuspensionBehaviorDeliverImmediately);

		glClearColor(0, 1, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		if (CGLFlushDrawable(ctx) != kCGLNoError || g_posts != 1 || g_object != native) {
			printf("FAIL: window swap posted %d (object %p, expected %p)\n", g_posts, g_object, native);
			failures++;
		}
		CGLClearDrawable(ctx);
		g_posts = 0;
		CGLFlushDrawable(ctx);
		if (g_posts != 0) { printf("FAIL: swap with no window surface posted\n"); failures++; }
		CGLContextMakeCurrentAndAttachToWindow(ctx, window);
		CGLDestroyWindow(window);
		CGLFlushDrawable(ctx);
		if (g_posts != 0) { printf("FAIL: swap after CGLDestroyWindow posted\n"); failures++; }
		printf(failures ? "FAIL\n" : "PASS\n");
		return failures;
	}
}
