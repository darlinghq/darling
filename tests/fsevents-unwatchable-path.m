#import <CoreServices/CoreServices.h>
#import <Foundation/Foundation.h>
#include <assert.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

static int callbackCount = 0;

static void testCallback(ConstFSEventStreamRef streamRef,
                         void *clientCallBackInfo,
                         size_t numEvents,
                         void *eventPaths,
                         const FSEventStreamEventFlags eventFlags[],
                         const FSEventStreamEventId eventIds[]) {
    (void)streamRef;
    (void)clientCallBackInfo;
    (void)numEvents;
    (void)eventPaths;
    (void)eventFlags;
    (void)eventIds;
    callbackCount++;
}

int main(int argc, const char *argv[]) {
    (void)argc;
    (void)argv;
    @autoreleasepool {
        NSString *validDir = [NSString stringWithFormat:@"/tmp/fsevents_test_%d", getpid()];
        mkdir([validDir UTF8String], 0777);
        
        NSArray *paths = @[
            @"/path/that/definitely/does/not/exist/fsevents_test_missing_12345",
            validDir
        ];
        
        FSEventStreamContext context = {0, NULL, NULL, NULL, NULL};
        FSEventStreamRef stream = FSEventStreamCreate(
            kCFAllocatorDefault,
            testCallback,
            &context,
            (CFArrayRef)paths,
            kFSEventStreamEventIdSinceNow,
            0.05,
            0
        );
        
        assert(stream != NULL);
        
        FSEventStreamScheduleWithRunLoop(stream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        
        Boolean started = FSEventStreamStart(stream);
        assert(started == true);
        
        // Touch a file inside the valid watched directory to trigger an inotify event
        NSString *testFile = [validDir stringByAppendingPathComponent:@"event.txt"];
        FILE *f = fopen([testFile UTF8String], "w");
        if (f) {
            fputs("hello fsevents", f);
            fclose(f);
        }
        
        // Pump the runloop to process inotify events
        for (int i = 0; i < 10 && callbackCount == 0; i++) {
            CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.1, false);
        }
        
        // Clean up stream
        FSEventStreamStop(stream);
        FSEventStreamInvalidate(stream);
        FSEventStreamRelease(stream);
        
        // Clean up filesystem
        unlink([testFile UTF8String]);
        rmdir([validDir UTF8String]);
        
        assert(callbackCount > 0);
        printf("PASS: FSEventStream received %d event(s) on valid path while ignoring missing path\n", callbackCount);
    }
    return 0;
}
