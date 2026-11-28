// Copyright (C) 2026 Lubos Dolezel
//
// The system sounds of the prefix: +[NSSound soundNamed:] finds each of the 14 standard names in
// /System/Library/Sounds, as NSSound's documentation describes, and the sounds are short.
//   system-sounds         checks the lookup, the files and NSBeep
//   system-sounds play    plays each sound in turn to the audio output, one after the other
#import <AppKit/AppKit.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(cond, ...) do { if (!(cond)) { failures++; printf("FAIL line %d: %s -- ", __LINE__, #cond); printf(__VA_ARGS__); printf("\n"); } } while (0)

static NSArray *names(void) {
    return @[@"Basso", @"Blow", @"Bottle", @"Frog", @"Funk", @"Glass", @"Hero",
             @"Morse", @"Ping", @"Pop", @"Purr", @"Sosumi", @"Submarine", @"Tink"];
}

int main(int argc, char **argv) {
    @autoreleasepool {
        if (argc > 1 && strcmp(argv[1], "play") == 0) {
            for (NSString *name in names()) {
                NSSound *s = [NSSound soundNamed: name];
                CHECK(s != nil, "%s", [name UTF8String]);
                if (s == nil)
                    continue;
                CHECK([s play], "play %s", [name UTF8String]);
                NSDate *end = [NSDate dateWithTimeIntervalSinceNow: [s duration] + 3.0];
                while ([s isPlaying] && [end timeIntervalSinceNow] > 0)
                    [[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.02]];
                CHECK(![s isPlaying], "%s finished", [name UTF8String]);
                [[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.3]];
            }
            printf(failures ? "FAIL (%d)\n" : "PASS\n", failures);
            return failures != 0;
        }

        NSFileManager *fm = [NSFileManager defaultManager];
        for (NSString *name in names()) {
            const char *n = [name UTF8String];
            NSString *path = [[@"/System/Library/Sounds" stringByAppendingPathComponent: name]
                                 stringByAppendingPathExtension: @"aiff"];
            CHECK([fm fileExistsAtPath: path], "%s file", n);

            NSSound *s = [NSSound soundNamed: name];
            CHECK(s != nil, "soundNamed: %s", n);
            if (s == nil)
                continue;
            CHECK([[s name] isEqualToString: name], "name of %s", n);
            CHECK([s duration] > 0.03 && [s duration] < 2.5, "duration of %s is %f", n, [s duration]);
            CHECK([NSSound soundNamed: name] == s, "the same object for %s", n);
            CHECK([s volume] == 1.0f, "volume of %s", n);
        }
        CHECK([NSSound soundNamed: @"NoSuchSystemSound"] == nil, "unknown name");

        // The alert paths still work (no crash, return at once) with the sounds installed
        NSBeep();
        NSBeep();
        void *toolbox = dlopen("/System/Library/Frameworks/AudioToolbox.framework/AudioToolbox", RTLD_LAZY);
        void (*alert)(unsigned) = toolbox != NULL ? dlsym(toolbox, "AudioServicesPlayAlertSound") : NULL;
        CHECK(alert != NULL, "AudioServicesPlayAlertSound");
        if (alert != NULL)
            alert(0x1000);   // kSystemSoundID_UserPreferredAlert
        printf(failures ? "FAIL (%d)\n" : "PASS\n", failures);
    }
    return failures != 0;
}
