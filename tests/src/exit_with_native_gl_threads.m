/*
 Copyright (C) 2026 Lubos Dolezel

 Licensed under the GPL, version 3 or (at your option) any later version.
*/
// A process that returns from main while a CVDisplayLink thread, which has a
// native OpenGL context current, and the driver's own worker threads are still
// alive has to exit normally: the exit must not unmap native libraries that
// those threads' thread-exit destructors (libGLdispatch's, for one) still use.
// The program starts itself repeatedly as a child that renders from a display
// link thread and then returns from main, once right after stopping the link
// (the link thread is exiting while the process exits) and once with the link
// still running, and checks that every child exits with status 0.
// Needs a display (DISPLAY on Linux hosts).
#import <AppKit/AppKit.h>
#import <CoreVideo/CoreVideo.h>
#import <OpenGL/gl.h>
#include <OpenGL/OpenGL.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

static NSOpenGLContext *context;
static volatile int frames;

static CVReturn callback(CVDisplayLinkRef link, const CVTimeStamp *now, const CVTimeStamp *out,
                         CVOptionFlags flagsIn, CVOptionFlags *flagsOut, void *info) {
    @autoreleasepool {
        CGLContextObj cgl = [context CGLContextObj];
        CGLLockContext(cgl);
        [context makeCurrentContext];
        glClearColor(0, 1, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        [context flushBuffer];
        CGLUnlockContext(cgl);
        frames++;
    }
    return kCVReturnSuccess;
}

static int child(int stop_link) {
    @autoreleasepool {
        [NSApplication sharedApplication];
        NSOpenGLPixelFormatAttribute attributes[] = {NSOpenGLPFADoubleBuffer, NSOpenGLPFAColorSize, 24, 0};
        NSOpenGLPixelFormat *format = [[NSOpenGLPixelFormat alloc] initWithAttributes: attributes];
        NSWindow *window = [[NSWindow alloc] initWithContentRect: NSMakeRect(100, 100, 200, 200)
                                                       styleMask: NSWindowStyleMaskTitled
                                                         backing: NSBackingStoreBuffered
                                                           defer: NO];
        NSOpenGLView *view = [[NSOpenGLView alloc] initWithFrame: NSMakeRect(0, 0, 200, 200) pixelFormat: format];
        [window setContentView: view];
        [window makeKeyAndOrderFront: nil];
        context = [view openGLContext];
        for (int i = 0; i < 10; i++)
            [[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.05]];

        CVDisplayLinkRef link;
        CVDisplayLinkCreateWithActiveCGDisplays(&link);
        CVDisplayLinkSetOutputCallback(link, callback, NULL);
        CVDisplayLinkStart(link);
        for (int i = 0; i < 60 && frames < 3; i++)
            [[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.05]];
        if (frames < 3)
            return 3;
        if (stop_link)
            CVDisplayLinkStop(link);
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc == 3 && !strcmp(argv[1], "child"))
        return child(atoi(argv[2]));

    int failures = 0;
    const int rounds = 24;
    for (int i = 0; i < rounds; i++) {
        char mode[2] = {(char) ('0' + (i & 1)), 0};
        char *args[] = {argv[0], (char *) "child", mode, NULL};
        pid_t pid;
        if (posix_spawn(&pid, argv[0], NULL, NULL, args, environ) != 0) {
            printf("FAIL posix_spawn\n");
            return 1;
        }
        int status = 0;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            printf("FAIL round %d (stop link %s): %s %d\n", i, mode,
                   WIFSIGNALED(status) ? "signal" : "status",
                   WIFSIGNALED(status) ? WTERMSIG(status) : WEXITSTATUS(status));
            failures++;
        }
    }
    puts(failures ? "FAIL" : "PASS");
    return failures ? 1 : 0;
}
