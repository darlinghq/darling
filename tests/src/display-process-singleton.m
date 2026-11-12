/* Copyright (C) 2026 Lubos Dolezel
 *
 * +[NSDisplay currentDisplay] is one object per process, whichever thread
 * asks first, and -[NSScreen screens] works from a secondary thread.
 * Needs an X display (Xvfb :99). Prints PASS or FAIL lines.
 */
#import <AppKit/AppKit.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

static int failures;
#define CHECK(c, ...) do { if (!(c)) { printf("FAIL: " __VA_ARGS__); printf("\n"); failures++; } } while (0)

static id workerDisplay;
static NSUInteger workerScreenCount;
static int workerRaised;

static void *worker(void *arg) {
    @autoreleasepool {
        @try {
            workerDisplay = [NSClassFromString(@"NSDisplay") performSelector: @selector(currentDisplay)];
            workerScreenCount = [[NSScreen screens] count];
        } @catch (id e) {
            workerRaised = 1;
        }
    }
    return NULL;
}

static void watchdog(int s) {
    write(1, "FAIL: timeout\n", 14);
    _exit(1);
}

int main(void) {
    signal(SIGALRM, watchdog);
    alarm(60);
    @autoreleasepool {
        [NSApplication sharedApplication];
        Class cls = NSClassFromString(@"NSDisplay");
        id first = [cls performSelector: @selector(currentDisplay)];
        CHECK(first != nil, "currentDisplay is nil on the main thread");
        CHECK([cls performSelector: @selector(currentDisplay)] == first, "second call on the main thread differs");
        NSUInteger mainScreens = [[NSScreen screens] count];

        for (int i = 0; i < 4; i++) {
            workerDisplay = nil; workerRaised = 0; workerScreenCount = 0;
            pthread_t t;
            pthread_create(&t, NULL, worker, NULL);
            pthread_join(t, NULL);
            CHECK(!workerRaised, "worker thread %d raised", i);
            CHECK(workerDisplay == first, "worker thread %d got another display", i);
            CHECK(workerScreenCount == mainScreens, "worker thread %d sees %lu screens, main %lu", i,
                  (unsigned long) workerScreenCount, (unsigned long) mainScreens);
        }
    }
    if (!failures) puts("PASS: one NSDisplay per process across threads");
    return failures ? 1 : 0;
}
