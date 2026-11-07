#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <OpenGL/OpenGL.h>
#include <stdio.h>
#undef NDEBUG
#include <assert.h>

#ifndef NSWindowStyleMaskBorderless
#define NSWindowStyleMaskBorderless NSBorderlessWindowMask
#endif

int main(void) {
    @autoreleasepool {
        NSApplication *app = [NSApplication sharedApplication];
        (void)app;

        // 1. Error checks on NULL arguments
        assert(CGLClearDrawable(NULL) == kCGLBadContext);

        NSOpenGLPixelFormatAttribute attrs[] = {
            NSOpenGLPFAColorSize, 24,
            NSOpenGLPFADoubleBuffer,
            0
        };
        NSOpenGLPixelFormat *pf = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attrs] autorelease];
        assert(pf != nil);

        NSOpenGLContext *ctx = [[[NSOpenGLContext alloc] initWithFormat:pf shareContext:nil] autorelease];
        assert(ctx != nil);
        CGLContextObj cglCtx = (CGLContextObj)[ctx CGLContextObj];
        assert(cglCtx != NULL);

        NSWindow *win = [[[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 200, 200)
                                                     styleMask:NSWindowStyleMaskBorderless
                                                       backing:NSBackingStoreBuffered
                                                         defer:NO] autorelease];
        NSView *view = [[[NSView alloc] initWithFrame:NSMakeRect(0, 0, 200, 200)] autorelease];
        [[win contentView] addSubview:view];
        [win orderFront:nil];

        // Attach view to context and make current
        [ctx setView:view];
        [ctx makeCurrentContext];
        assert(CGLGetCurrentContext() == cglCtx);

        // First flush on attached window must succeed
        [ctx flushBuffer];

        // 2. Detach view - under the hood Cocoa's [ctx setView:nil] unbinds and calls CGLDestroyWindow
        [ctx setView:nil];

        // Re-bind context in detached state
        [ctx makeCurrentContext];

        // Flush buffer on detached context:
        // Must succeed with kCGLNoError (0) rather than failing with kCGLBadDrawable (10005)
        // or attempting eglSwapBuffers on a destroyed EGLSurface.
        CGLError err = CGLFlushDrawable(cglCtx);
        assert(err == kCGLNoError);

        // 3. Test CGLClearDrawable directly while attached
        [ctx setView:view];
        [ctx makeCurrentContext];
        assert(CGLGetCurrentContext() == cglCtx);
        [ctx flushBuffer];

        err = CGLClearDrawable(cglCtx);
        assert(err == kCGLNoError);

        // CGLClearDrawable keeps context current on calling thread
        assert(CGLGetCurrentContext() == cglCtx);

        // Flushes on cleared drawable should gracefully succeed in detached mode
        err = CGLFlushDrawable(cglCtx);
        assert(err == kCGLNoError);

        // 4. Test re-attaching after CGLClearDrawable and subsequent setView:nil (CGLDestroyWindow)
        [ctx setView:view];
        [ctx makeCurrentContext];
        [ctx flushBuffer];

        [ctx setView:nil];
        err = CGLFlushDrawable(cglCtx);
        assert(err == kCGLNoError);

        [NSOpenGLContext clearCurrentContext];
    }

    puts("PASS: CGLDestroyWindow and CGLClearDrawable disassociate context surface successfully");
    return 0;
}
