// Copyright (C) 2026 Lubos Dolezel
// Registers application bundles with LaunchServices (LSRegisterURL), for test harnesses that put a stand-in
// application in place. Usage: ls_register <bundle>...; exits 1 if one cannot be registered.
// Link with -framework Foundation -framework CoreServices.
#import <Foundation/Foundation.h>
#import <CoreServices/CoreServices.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    int status = 0;
    @autoreleasepool {
        for (int i = 1; i < argc; i++) {
            NSURL *url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:argv[i]] isDirectory:YES];
            OSStatus err = LSRegisterURL((CFURLRef)url, true);
            if (err != noErr) {
                fprintf(stderr, "ls_register: %s: %d\n", argv[i], (int)err);
                status = 1;
            }
        }
    }
    return status;
}
