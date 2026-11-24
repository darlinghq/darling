// Copyright (C) 2026 Lubos Dolezel
// The executable of a test application bundle that records the Apple Events it receives. It installs handlers for
// aevt/oapp, aevt/odoc, aevt/pdoc, aevt/rapp and GURL/GURL in applicationWillFinishLaunching: (where an application
// must install them to receive its launch event), and for each event appends a line to events.txt next to its bundle:
//   <event class> <event ID>[ aapd]<items>
// " aapd" when the event has an 'aapd' parameter that is true; then, for each item of the direct object (a list, or a
// single descriptor), " <URL>" for a file URL or a string, and " bmrk:<URL>" for bookmark data ('bmrk'), resolved to
// its URL. It quits 20 s after the last event (or after starting, if none comes). Used by
// launchservices_open_application.m and the reveal case of volumes.sh. Link with -framework AppKit; also builds on
// macOS.
#import <AppKit/AppKit.h>

#define IDLE_SECONDS 20.0

@interface Recorder : NSObject <NSApplicationDelegate>
@property (retain) NSTimer *idle;
@end

static NSString *fourcc(FourCharCode code)
{
    char text[5] = { (char)(code >> 24), (char)(code >> 16), (char)(code >> 8), (char)code, 0 };
    return [NSString stringWithUTF8String:text];
}

static NSString *describeItem(NSAppleEventDescriptor *item)
{
    if (item.descriptorType == 'bmrk')
    {
        BOOL stale = NO;
        NSURL *url = [NSURL URLByResolvingBookmarkData:item.data options:0 relativeToURL:nil bookmarkDataIsStale:&stale error:NULL];
        return [@"bmrk:" stringByAppendingString:url.absoluteString ?: @"?"];
    }
    if (item.descriptorType == typeFileURL)
    {
        NSString *string = [[[NSString alloc] initWithData:item.data encoding:NSUTF8StringEncoding] autorelease];
        return string ?: @"?";
    }
    return item.stringValue ?: [NSString stringWithFormat:@"<%@>", fourcc(item.descriptorType)];
}

@implementation Recorder

- (void)applicationWillFinishLaunching:(NSNotification *)notification
{
    NSAppleEventManager *manager = [NSAppleEventManager sharedAppleEventManager];
    for (NSNumber *eventID in @[ @(kAEOpenApplication), @(kAEOpenDocuments), @(kAEPrintDocuments), @(kAEReopenApplication) ])
        [manager setEventHandler:self andSelector:@selector(handle:reply:) forEventClass:kCoreEventClass andEventID:eventID.unsignedIntValue];
    [manager setEventHandler:self andSelector:@selector(handle:reply:) forEventClass:kInternetEventClass andEventID:kAEGetURL];
    [self restartIdle];
}

- (void)restartIdle
{
    [self.idle invalidate];
    self.idle = [NSTimer scheduledTimerWithTimeInterval:IDLE_SECONDS target:NSApp selector:@selector(terminate:) userInfo:nil repeats:NO];
}

- (void)handle:(NSAppleEventDescriptor *)event reply:(NSAppleEventDescriptor *)reply
{
    NSMutableString *line = [NSMutableString stringWithFormat:@"%@ %@", fourcc(event.eventClass), fourcc(event.eventID)];
    if ([[event paramDescriptorForKeyword:'aapd'] booleanValue])
        [line appendString:@" aapd"];
    NSAppleEventDescriptor *direct = [event paramDescriptorForKeyword:keyDirectObject];
    if (direct.descriptorType == typeAEList)
    {
        for (NSInteger i = 1; i <= direct.numberOfItems; i++)
            [line appendFormat:@" %@", describeItem([direct descriptorAtIndex:i])];
    }
    else if (direct != nil)
        [line appendFormat:@" %@", describeItem(direct)];
    [line appendString:@"\n"];

    NSString *path = [[[[NSBundle mainBundle] bundlePath] stringByDeletingLastPathComponent] stringByAppendingPathComponent:@"events.txt"];
    FILE *file = fopen([path fileSystemRepresentation], "a");
    if (file != NULL)
    {
        fputs([line UTF8String], file);
        fclose(file);
    }
    [self restartIdle];
}

@end

int main(void)
{
    @autoreleasepool
    {
        NSApplication *app = [NSApplication sharedApplication];
        Recorder *recorder = [[Recorder alloc] init];
        app.delegate = recorder;
        [app run];
    }
    return 0;
}
