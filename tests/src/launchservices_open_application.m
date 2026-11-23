// Copyright (C) 2026 Lubos Dolezel
// launchservices_open_application <dir>: LSOpenApplication with an initial Apple Event, using <dir>/Receiver.app,
// whose executable is ae-receiver.m (it writes the events it gets to <dir>/events.txt):
//  - the application is launched, and the initial event (here aevt/odoc with two file URLs) is its first event;
//  - asked again while it runs, the same process gets the event; without an initial event it gets a reopen;
//  - LSOpenFromURLSpec with a document while it runs gives it an open-documents event;
//  - every event LaunchServices delivers carries 'aapd' true (a quirk, see LaunchServices.c);
//  - a NULL application gives paramErr, and a directory without an executable kLSNoExecutableErr
//    (-10827, as on macOS 26.7.3).
// Prints "FAIL: ..." for each check that does not hold and exits 1 if any did. The receiver is stopped at the end.
#import <Foundation/Foundation.h>
#include <CoreServices/CoreServices.h>
#include <signal.h>

static int failures;

static void expect(BOOL ok, NSString *what)
{
    if (!ok)
    {
        printf("FAIL: %s\n", [what UTF8String]);
        failures++;
    }
}

static AppleEvent openEvent(NSArray *urls)
{
    AppleEvent event;
    AEDesc target;
    AEDescList list;
    const char *bundleID = "org.darlinghq.test.ae-receiver";

    AECreateDesc(typeApplicationBundleID, bundleID, strlen(bundleID), &target);
    AECreateAppleEvent(kCoreEventClass, kAEOpenDocuments, &target, kAutoGenerateReturnID, kAnyTransactionID, &event);
    AEDisposeDesc(&target);
    AECreateList(NULL, 0, false, &list);
    for (NSURL *url in urls)
    {
        const char *string = [[url absoluteString] UTF8String];
        AEPutPtr(&list, 0, typeFileURL, string, strlen(string));
    }
    AEPutParamDesc(&event, keyDirectObject, &list);
    AEDisposeDesc(&list);
    return event;
}

// the lines of events.txt, once it has at least `count` of them (waiting up to 10 s)
static NSArray *waitForLines(NSString *path, NSUInteger count)
{
    NSArray *lines = @[];
    for (int i = 0; i < 100; i++)
    {
        NSString *text = [NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:NULL];
        lines = text ? [[text stringByTrimmingCharactersInSet:[NSCharacterSet newlineCharacterSet]] componentsSeparatedByString:@"\n"] : @[];
        if (text.length > 0 && lines.count >= count)
            break;
        usleep(100000);
    }
    return lines;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "usage: launchservices_open_application <dir with Receiver.app>\n");
        return 2;
    }
    @autoreleasepool
    {
        NSString *dir = [NSString stringWithUTF8String:argv[1]];
        NSString *app = [dir stringByAppendingPathComponent:@"Receiver.app"];
        NSString *events = [dir stringByAppendingPathComponent:@"events.txt"];
        NSURL *a = [NSURL fileURLWithPath:[dir stringByAppendingPathComponent:@"a.txt"]];
        NSURL *b = [NSURL fileURLWithPath:[dir stringByAppendingPathComponent:@"b.txt"]];
        NSString *expected = [NSString stringWithFormat:@"aevt odoc aapd %@ %@", [a absoluteString], [b absoluteString]];
        FSRef ref;
        ProcessSerialNumber psn = { 0, 0 };

        expect(LSRegisterURL((CFURLRef)[NSURL fileURLWithPath:app], true) == noErr, @"the receiver cannot be registered");
        expect(FSPathMakeRef((const UInt8 *)[app fileSystemRepresentation], &ref, NULL) == noErr, @"no FSRef for the receiver");

        AppleEvent event = openEvent(@[ a, b ]);
        LSApplicationParameters params = { 0, kLSLaunchDefaults, &ref, NULL, NULL, NULL, &event };
        OSStatus status = LSOpenApplication(&params, &psn);
        pid_t pid = (pid_t)psn.lowLongOfPSN;
        expect(status == noErr && psn.highLongOfPSN == 0 && pid > 0,
            [NSString stringWithFormat:@"LSOpenApplication gives %d, pid %d", (int)status, (int)pid]);
        NSArray *lines = waitForLines(events, 1);
        expect([[lines firstObject] isEqual:expected], [NSString stringWithFormat:@"the first event is %@", [lines firstObject]]);

        ProcessSerialNumber again = { 0, 0 };
        status = LSOpenApplication(&params, &again);
        expect(status == noErr && again.lowLongOfPSN == psn.lowLongOfPSN,
            [NSString stringWithFormat:@"asked again: %d, pid %u (first %u)", (int)status, (unsigned)again.lowLongOfPSN, (unsigned)psn.lowLongOfPSN]);
        lines = waitForLines(events, 2);
        expect(lines.count == 2 && [lines[1] isEqual:expected], [NSString stringWithFormat:@"the events are %@", lines]);
        AEDisposeDesc(&event);

        LSApplicationParameters noEvent = { 0, kLSLaunchDefaults, &ref, NULL, NULL, NULL, NULL };
        expect(LSOpenApplication(&noEvent, NULL) == noErr, @"asked again without an event: not noErr");
        lines = waitForLines(events, 3);
        expect(lines.count == 3 && [lines[2] isEqual:@"aevt rapp aapd"], [NSString stringWithFormat:@"the events are %@", lines]);

        // macOS gives documents as bookmark data ("bmrk:<URL>"), resolved to /private/tmp for /tmp
        CFArrayRef items = (CFArrayRef)@[ a ];
        LSLaunchURLSpec spec = { (CFURLRef)[NSURL fileURLWithPath:app], items, NULL, kLSLaunchDefaults, NULL };
        expect(LSOpenFromURLSpec(&spec, NULL) == noErr, @"LSOpenFromURLSpec is not noErr");
        lines = waitForLines(events, 4);
        expect(lines.count == 4 && [lines[3] hasPrefix:@"aevt odoc aapd "] && [lines[3] hasSuffix:@"/a.txt"]
            && [[lines[3] componentsSeparatedByString:@" "] count] == 4, [NSString stringWithFormat:@"the events are %@", lines]);

        LSApplicationParameters noApplication = { 0, kLSLaunchDefaults, NULL, NULL, NULL, NULL, NULL };
        expect(LSOpenApplication(&noApplication, NULL) == paramErr, @"a NULL application is not paramErr");
        FSRef dirRef;
        FSPathMakeRef((const UInt8 *)[dir fileSystemRepresentation], &dirRef, NULL);
        LSApplicationParameters notApplication = { 0, kLSLaunchDefaults, &dirRef, NULL, NULL, NULL, NULL };
        status = LSOpenApplication(&notApplication, NULL);
        expect(status == kLSNoExecutableErr, [NSString stringWithFormat:@"a plain directory gives %d", (int)status]);

        if (pid > 0)
            kill(pid, SIGTERM);
    }
    return failures ? 1 : 0;
}
