// Copyright (C) 2026 Lubos Dolezel
//
// CGLGetSurface(): the window server ids (connection, window, surface) of a
// CGL context's drawable, and the CGSGetSurfaceBounds() / CGSGetWindowBounds()
// calls made with them, as a game overlay library injected into an OpenGL
// application makes them. CGLGetSurface is private; it is declared here.
//
// Observed on macOS 26.7.3 (x86_64), the behaviour this test pins:
//  - a NULL context gives kCGLBadContext and leaves the outputs untouched;
//  - a context without a drawable gives kCGLNoError and zeros for all three
//    ids (also when it is current), and CGSGetSurfaceBounds() and
//    CGSGetWindowBounds() with them fail;
//  - a context attached to an NSOpenGLView's window gives the default
//    connection id, a window id derived from the window and a nonzero surface
//    id; they do not change when the context is made current or flushed or the
//    window is resized, and CGSGetSurfaceBounds() gives the origin 0,0 and the
//    size of the view (updated after a resize);
//  - -clearDrawable returns the context to zeros;
//  - a CGS window and surface given to CGLSetSurface() are given back by
//    CGLGetSurface() unchanged, and bounds of nonexistent windows and
//    surfaces fail with kCGErrorFailure and kCGErrorIllegalArgument.
// Where macOS and Darling differ (marked below), macOS uses an internal
// window one above the AppKit window number and reports empty window bounds
// for it; Darling uses the window number itself and reports the window frame.
// Needs a window system (X11 or Wayland) and a host EGL driver.
#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#include <dlfcn.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef unsigned ConnID, WinID, SurfID;
// The private calls are looked up by name, so that this builds against SDKs
// that declare them differently (or not at all) and a missing one is a plain
// failure.
static CGLError (*CGLGetSurface_)(CGLContextObj ctx, ConnID *cid, WinID *wid, SurfID *sid);
static CGLError (*CGLSetSurface_)(CGLContextObj ctx, ConnID cid, WinID wid, SurfID sid);
static ConnID (*defaultConnection_)(void);
static CGError (*getSurfaceBounds_)(ConnID cid, WinID wid, SurfID sid, CGRect *bounds);
static CGError (*getWindowBounds_)(ConnID cid, WinID wid, CGRect *bounds);
#define CGLGetSurface CGLGetSurface_
#define CGLSetSurface CGLSetSurface_
#define _CGSDefaultConnection defaultConnection_
#define CGSGetSurfaceBounds getSurfaceBounds_
#define CGSGetWindowBounds getWindowBounds_
#define CGSConnectionID ConnID
#define CGSWindowID WinID
#define CGSSurfaceID SurfID

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_EQ(a, b) do { long _a = (long) (a), _b = (long) (b); if (_a != _b) { printf("FAIL %d: %s = %ld, expected %ld\n", __LINE__, #a, _a, _b); g_failures++; } } while (0)

static void on_alarm(int sig)
{
    (void) sig;
    printf("FAIL: timed out\n");
    fflush(stdout);
    _exit(1);
}

static void spin(void)
{
    for (int i = 0; i < 10; i++)
        [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.05]];
}

static void expect_zero_ids(const char *what, CGLContextObj ctx)
{
    CGSConnectionID cid = 77;
    CGSWindowID wid = 78;
    CGSSurfaceID sid = 79;
    CGLError error = CGLGetSurface(ctx, &cid, &wid, &sid);
    if (error != kCGLNoError || cid != 0 || wid != 0 || sid != 0) {
        printf("FAIL: %s: error %d, ids %u %u %u (expected 0 and zeros)\n", what, error, cid, wid, sid);
        g_failures++;
    }
    CGRect bounds;
    // macOS gives kCGErrorIllegalArgument and kCGErrorFailure for the zero ids;
    // only failure is checked, because the error for an invalid connection id
    // varies between window server versions and implementations.
    CHECK(CGSGetSurfaceBounds(cid, wid, sid, &bounds) != kCGErrorSuccess);
    CHECK(CGSGetWindowBounds(cid, wid, &bounds) != kCGErrorSuccess);
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    signal(SIGALRM, on_alarm);
    alarm(90);
    CGLGetSurface = dlsym(RTLD_DEFAULT, "CGLGetSurface");
    CGLSetSurface = dlsym(RTLD_DEFAULT, "CGLSetSurface");
    _CGSDefaultConnection = dlsym(RTLD_DEFAULT, "_CGSDefaultConnection");
    CGSGetSurfaceBounds = dlsym(RTLD_DEFAULT, "CGSGetSurfaceBounds");
    CGSGetWindowBounds = dlsym(RTLD_DEFAULT, "CGSGetWindowBounds");
    if (!CGLGetSurface || !CGLSetSurface || !_CGSDefaultConnection || !CGSGetSurfaceBounds || !CGSGetWindowBounds) {
        printf("FAIL: a private call is missing (CGLGetSurface %p, CGLSetSurface %p, _CGSDefaultConnection %p, "
               "CGSGetSurfaceBounds %p, CGSGetWindowBounds %p)\n",
               CGLGetSurface, CGLSetSurface, _CGSDefaultConnection, CGSGetSurfaceBounds, CGSGetWindowBounds);
        return 1;
    }
    @autoreleasepool {
        [NSApplication sharedApplication];

        // A NULL context: kCGLBadContext, the outputs are not written.
        {
            CGSConnectionID cid = 77;
            CGSWindowID wid = 78;
            CGSSurfaceID sid = 79;
            CHECK_EQ(CGLGetSurface(NULL, &cid, &wid, &sid), kCGLBadContext);
            CHECK(cid == 77 && wid == 78 && sid == 79);
        }

        // A context with no drawable.
        CGLPixelFormatAttribute attributes[] = { kCGLPFADoubleBuffer, 0 };
        CGLPixelFormatObj format = NULL;
        GLint count = 0;
        if (CGLChoosePixelFormat(attributes, &format, &count) != kCGLNoError || format == NULL) {
            printf("SKIP: no pixel format\n");
            return 0;
        }
        CGLContextObj plain = NULL;
        CHECK_EQ(CGLCreateContext(format, NULL, &plain), kCGLNoError);
        expect_zero_ids("no drawable", plain);
        CGLSetCurrentContext(plain);
        expect_zero_ids("no drawable, current", plain);
        CGLSetCurrentContext(NULL);

        // A context attached to an NSOpenGLView's window.
        NSOpenGLPixelFormatAttribute viewAttributes[] = { NSOpenGLPFADoubleBuffer, 0 };
        NSOpenGLPixelFormat *viewFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:viewAttributes];
        NSWindow *window = [[NSWindow alloc] initWithContentRect:NSMakeRect(100, 200, 300, 200)
                                                       styleMask:NSWindowStyleMaskTitled
                                                         backing:NSBackingStoreBuffered
                                                           defer:NO];
        NSOpenGLView *view = [[NSOpenGLView alloc] initWithFrame:NSMakeRect(0, 0, 300, 200) pixelFormat:viewFormat];
        [window setContentView:view];
        [window makeKeyAndOrderFront:nil];
        spin();
        NSOpenGLContext *context = [view openGLContext];
        CGLContextObj cgl = [context CGLContextObj];
        [context makeCurrentContext];
        CHECK(CGLGetCurrentContext() == cgl);

        CGSConnectionID cid = 0;
        CGSWindowID wid = 0;
        CGSSurfaceID sid = 0;
        CHECK_EQ(CGLGetSurface(CGLGetCurrentContext(), &cid, &wid, &sid), kCGLNoError);
        CHECK_EQ(cid, _CGSDefaultConnection());
        CHECK(wid != 0 && sid != 0);
        long number = [window windowNumber];
        CHECK(wid == number || wid == number + 1);
        int darling = wid == number;

        CGRect bounds = CGRectMake(-1, -1, -1, -1);
        CHECK_EQ(CGSGetSurfaceBounds(cid, wid, sid, &bounds), kCGErrorSuccess);
        CHECK(bounds.origin.x == 0 && bounds.origin.y == 0);
        NSSize viewSize = [view bounds].size;
        CHECK(bounds.size.width == viewSize.width && bounds.size.height == viewSize.height);

        bounds = CGRectMake(-1, -1, -1, -1);
        CHECK_EQ(CGSGetWindowBounds(cid, wid, &bounds), kCGErrorSuccess);
        if (darling) {
            NSRect frame = [window frame];
            CGFloat screenHeight = CGDisplayBounds(CGMainDisplayID()).size.height;
            CHECK(bounds.size.width == frame.size.width && bounds.size.height == frame.size.height);
            CHECK(bounds.origin.x == frame.origin.x && bounds.origin.y == screenHeight - NSMaxY(frame));
        }

        // Ids stay the same over a flush; a resize changes the surface size only.
        CGLFlushDrawable(cgl);
        CGSConnectionID cid2 = 0;
        CGSWindowID wid2 = 0;
        CGSSurfaceID sid2 = 0;
        CHECK_EQ(CGLGetSurface(cgl, &cid2, &wid2, &sid2), kCGLNoError);
        CHECK(cid2 == cid && wid2 == wid && sid2 == sid);

        [window setFrame:NSMakeRect(50, 60, 500, 400) display:YES];
        [context update];
        spin();
        CHECK_EQ(CGLGetSurface(cgl, &cid2, &wid2, &sid2), kCGLNoError);
        CHECK(cid2 == cid && wid2 == wid && sid2 == sid);
        viewSize = [view bounds].size;
        bounds = CGRectMake(-1, -1, -1, -1);
        CHECK_EQ(CGSGetSurfaceBounds(cid, wid, sid, &bounds), kCGErrorSuccess);
        CHECK(bounds.size.width == viewSize.width && bounds.size.height == viewSize.height);
        CHECK(viewSize.width == 500);
        if (darling) {
            NSRect frame = [window frame];
            CHECK_EQ(CGSGetWindowBounds(cid, wid, &bounds), kCGErrorSuccess);
            CHECK(bounds.size.width == frame.size.width && bounds.size.height == frame.size.height);
        }

        // Nonexistent windows and surfaces.
        CHECK_EQ(CGSGetWindowBounds(cid, 0x7ffffff0, &bounds), kCGErrorFailure);
        CHECK_EQ(CGSGetSurfaceBounds(cid, wid, sid + 1000, &bounds), kCGErrorIllegalArgument);
        CHECK_EQ(CGSGetSurfaceBounds(cid, 0x7ffffff0, sid, &bounds), kCGErrorIllegalArgument);

        // A context of its own, with no view, then attached to the same view.
        NSOpenGLContext *other = [[NSOpenGLContext alloc] initWithFormat:viewFormat shareContext:nil];
        expect_zero_ids("second context, no view", [other CGLContextObj]);
        [other setView:view];
        CHECK_EQ(CGLGetSurface([other CGLContextObj], &cid2, &wid2, &sid2), kCGLNoError);
        CHECK(cid2 == cid && wid2 == wid && sid2 != 0);

        // Clearing the drawable.
        [context clearDrawable];
        expect_zero_ids("after clearDrawable", cgl);
        [other clearDrawable];
        [NSOpenGLContext clearCurrentContext];

        // A CGS window and surface given to CGLSetSurface come back unchanged.
        // Skipped on Darling's Wayland backend, where CGLSetSurface on a CGS window
        // aborts in libwayland ("Proxy and queue point to different wl_displays"),
        // independently of CGLGetSurface.
        const char *backend = getenv("DARLING_APPKIT_BACKEND");
        int wayland = backend ? !strcmp(backend, "wayland") : getenv("WAYLAND_DISPLAY") != NULL;
        if (wayland) {
            printf("SKIP: CGLSetSurface round trip on the Wayland backend\n");
            printf(g_failures ? "FAIL (%d)\n" : "PASS\n", g_failures);
            return g_failures ? 1 : 0;
        }
        ConnID (*mainConnection)(void) = dlsym(RTLD_DEFAULT, "CGSMainConnectionID");
        CGError (*newRegion)(const CGRect *, CFTypeRef *) = dlsym(RTLD_DEFAULT, "CGSNewRegionWithRect");
        CGError (*newWindow)(CGSConnectionID, int, float, float, CFTypeRef, CGSWindowID *) = dlsym(RTLD_DEFAULT, "CGSNewWindow");
        CGError (*addSurface)(CGSConnectionID, CGSWindowID, CGSSurfaceID *) = dlsym(RTLD_DEFAULT, "CGSAddSurface");
        CGError (*setSurfaceBounds)(CGSConnectionID, CGSWindowID, CGSSurfaceID, CGRect) = dlsym(RTLD_DEFAULT, "CGSSetSurfaceBounds");
        CHECK(mainConnection && newRegion && newWindow && addSurface && setSurfaceBounds);
        if (mainConnection && newRegion && newWindow && addSurface && setSurfaceBounds) {
            CGSConnectionID connection = mainConnection();
            CGRect frame = CGRectMake(100, 120, 200, 150);
            CFTypeRef region = NULL;
            CGSWindowID cgsWindow = 0;
            CGSSurfaceID cgsSurface = 0;
            CHECK_EQ(newRegion(&frame, &region), kCGErrorSuccess);
            CHECK_EQ(newWindow(connection, 2, 0, 0, region, &cgsWindow), kCGErrorSuccess);
            CHECK_EQ(addSurface(connection, cgsWindow, &cgsSurface), kCGErrorSuccess);
            setSurfaceBounds(connection, cgsWindow, cgsSurface, CGRectMake(0, 0, 200, 150));
            CHECK_EQ(CGLSetSurface(plain, connection, cgsWindow, cgsSurface), kCGLNoError);
            CHECK_EQ(CGLGetSurface(plain, &cid2, &wid2, &sid2), kCGLNoError);
            CHECK(cid2 == connection && wid2 == cgsWindow && sid2 == cgsSurface);
            CHECK_EQ(CGSGetSurfaceBounds(cid2, wid2, sid2, &bounds), kCGErrorSuccess);
            CHECK(bounds.size.width == 200 && bounds.size.height == 150);
            CHECK_EQ(CGSGetWindowBounds(cid2, wid2, &bounds), kCGErrorSuccess);
            CHECK(bounds.size.width == 200 && bounds.size.height == 150);
            CGLClearDrawable(plain);
            expect_zero_ids("CGLClearDrawable", plain);
        }
    }
    printf(g_failures ? "FAIL (%d)\n" : "PASS\n", g_failures);
    return g_failures ? 1 : 0;
}
