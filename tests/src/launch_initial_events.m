// Copyright (C) 2026 Lubos Dolezel
// launch_initial_events: an AppKit application launched by LSOpenApplication with an initial Apple Event, handled by
// AppKit's own handlers:
//  - a reopen (aevt/rapp) as the initial event does not hold up the launch: applicationDidFinishLaunching: follows
//    applicationWillFinishLaunching: within 3 s;
//  - an open-documents event (aevt/odoc) whose items are bookmark data, as a reveal sends them, reaches
//    application:openFiles: as the files' paths.
// The program copies its own executable into a throw-away application bundle, which records its callbacks in
// log.txt next to the bundle. Run with no arguments. Prints "FAIL: ..." for each check that does not hold and exits
// 1 if any did. Link with -framework AppKit -framework CoreServices. Also passes on macOS 26.7.3.
#import <AppKit/AppKit.h>
#include <CoreServices/CoreServices.h>
#include <mach-o/dyld.h>
#include <stdlib.h>
#include <unistd.h>

#define BUNDLE_ID "org.darlinghq.test.initial-events"

static int failures;

static void expect(BOOL ok, NSString *what)
{
    if (!ok)
    {
        printf("FAIL: %s\n", [what UTF8String]);
        failures++;
    }
}

@interface Recorder : NSObject <NSApplicationDelegate>
@end

@implementation Recorder
- (void)log:(NSString *)line
{
    NSString *path = [[[[NSBundle mainBundle] bundlePath] stringByDeletingLastPathComponent] stringByAppendingPathComponent:@"log.txt"];
    FILE *file = fopen([path fileSystemRepresentation], "a");
    fprintf(file, "%s\n", [line UTF8String]);
    fclose(file);
}
- (void)applicationWillFinishLaunching:(NSNotification *)n
{
    [self log:[NSString stringWithFormat:@"willFinish %f", CFAbsoluteTimeGetCurrent()]];
}
- (void)applicationDidFinishLaunching:(NSNotification *)n
{
    [self log:[NSString stringWithFormat:@"didFinish %f", CFAbsoluteTimeGetCurrent()]];
    [NSApp performSelector:@selector(terminate:) withObject:nil afterDelay:1];
}
- (void)application:(NSApplication *)app openFiles:(NSArray<NSString *> *)files
{
    [self log:[@"openFiles " stringByAppendingString:[files componentsJoinedByString:@" "]]];
}
- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)app
{
    return NO;
}
@end

static NSArray *waitForLog(NSString *path, NSString *prefix)
{
    for (int i = 0; i < 150; i++)
    {
        NSString *text = [NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:NULL];
        NSArray *lines = [[text stringByTrimmingCharactersInSet:[NSCharacterSet newlineCharacterSet]] componentsSeparatedByString:@"\n"];
        for (NSString *line in lines)
            if ([line hasPrefix:prefix])
                return lines;
        usleep(100000);
    }
    return nil;
}

static double timeOf(NSArray *lines, NSString *name)
{
    for (NSString *line in lines)
        if ([line hasPrefix:[name stringByAppendingString:@" "]])
            return [[line substringFromIndex:name.length + 1] doubleValue];
    return 0;
}

static OSStatus launch(NSString *app, AEEventID eventID, NSArray<NSURL *> *items)
{
    NSAppleEventDescriptor *target = [NSAppleEventDescriptor descriptorWithDescriptorType:typeApplicationBundleID
        data:[@BUNDLE_ID dataUsingEncoding:NSUTF8StringEncoding]];
    NSAppleEventDescriptor *event = [NSAppleEventDescriptor appleEventWithEventClass:kCoreEventClass eventID:eventID
        targetDescriptor:target returnID:kAutoGenerateReturnID transactionID:kAnyTransactionID];
    if (items.count)
    {
        NSAppleEventDescriptor *list = [NSAppleEventDescriptor listDescriptor];
        NSInteger index = 1;
        for (NSURL *item in items)
            [list insertDescriptor:[NSAppleEventDescriptor descriptorWithDescriptorType:typeBookmarkData
                data:[item bookmarkDataWithOptions:0 includingResourceValuesForKeys:nil relativeToURL:nil error:NULL]] atIndex:index++];
        [event setParamDescriptor:list forKeyword:keyDirectObject];
    }
    FSRef ref;
    FSPathMakeRef((const UInt8 *)[app fileSystemRepresentation], &ref, NULL);
    LSApplicationParameters params = { 0, kLSLaunchDefaults, &ref, NULL, NULL, NULL, (AppleEvent *)[event aeDesc] };
    return LSOpenApplication(&params, NULL);
}

static void waitForExit(NSString *log)
{
    // the application quits 1 s after applicationDidFinishLaunching:
    sleep(3);
    [[NSFileManager defaultManager] removeItemAtPath:log error:NULL];
}

int main(int argc, char **argv)
{
    @autoreleasepool
    {
        if ([[[NSBundle mainBundle] bundlePath] hasSuffix:@".app"])
        {
            NSApplication *app = [NSApplication sharedApplication];
            app.delegate = [[Recorder alloc] init];
            [app run];
            return 0;
        }

        char exe[PATH_MAX], real[PATH_MAX];
        uint32_t size = sizeof(exe);
        char template[] = "/tmp/launch-initial-events.XXXXXX";
        if (_NSGetExecutablePath(exe, &size) != 0 || mkdtemp(template) == NULL || realpath(template, real) == NULL)
            return 2;
        NSFileManager *fm = [NSFileManager defaultManager];
        NSString *dir = [NSString stringWithUTF8String:real];
        NSString *app = [dir stringByAppendingPathComponent:@"Viewer.app"];
        NSString *log = [dir stringByAppendingPathComponent:@"log.txt"];
        [fm createDirectoryAtPath:[app stringByAppendingPathComponent:@"Contents/MacOS"] withIntermediateDirectories:YES attributes:nil error:NULL];
        [fm copyItemAtPath:[NSString stringWithUTF8String:exe] toPath:[app stringByAppendingPathComponent:@"Contents/MacOS/Viewer"] error:NULL];
        [@{ @"CFBundleExecutable": @"Viewer", @"CFBundleIdentifier": @BUNDLE_ID, @"CFBundlePackageType": @"APPL",
            @"CFBundleName": @"Viewer" } writeToFile:[app stringByAppendingPathComponent:@"Contents/Info.plist"] atomically:NO];
        expect(LSRegisterURL((CFURLRef)[NSURL fileURLWithPath:app], true) == noErr, @"the application cannot be registered");

        expect(launch(app, kAEReopenApplication, nil) == noErr, @"the launch with a reopen failed");
        NSArray *lines = waitForLog(log, @"didFinish");
        double wait = timeOf(lines, @"didFinish") - timeOf(lines, @"willFinish");
        printf("reopen launch: applicationDidFinishLaunching: %.2f s after applicationWillFinishLaunching:\n", wait);
        expect(lines != nil && wait < 3, [NSString stringWithFormat:@"with a reopen as the initial event: %@", lines]);
        waitForExit(log);

        NSString *a = [dir stringByAppendingPathComponent:@"a.txt"];
        [@"a" writeToFile:a atomically:NO encoding:NSUTF8StringEncoding error:NULL];
        expect(launch(app, kAEOpenDocuments, @[ [NSURL fileURLWithPath:a] ]) == noErr, @"the launch with documents failed");
        lines = waitForLog(log, @"didFinish");
        expect([lines containsObject:[@"openFiles " stringByAppendingString:a]],
            [NSString stringWithFormat:@"with bookmark items: %@", lines]);
        waitForExit(log);

        [fm removeItemAtPath:dir error:NULL];
    }
    if (failures)
        return 1;
    printf("PASS\n");
    return 0;
}
