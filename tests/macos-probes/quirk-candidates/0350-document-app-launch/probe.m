// Probe 0350: what a document-based application does at launch (new untitled document, or the Open panel) and
// when it is asked to open files, when its delegate implements neither -application:openFile: nor
// -application:openFiles:. It is built as a tiny app bundle in two variants (see build.txt):
//   plain  : declares one document type (CFBundleDocumentTypes)
//   ubiq   : the same, plus an NSUbiquitousContainers dictionary in Info.plist
// The user default NSShowAppCentricOpenPanelInsteadOfUntitledFile and the delegate's answer to
// -applicationShouldOpenUntitledFile: are set from the command line (run.sh tries the combinations):
//   probe [-NSShowAppCentricOpenPanelInsteadOfUntitledFile YES|NO] [-probeRefuseUntitled YES|NO] [file ...]
#define PROBE_APPKIT 1
#import "probe_common.h"

@interface ProbeDoc : NSDocument
@end
@implementation ProbeDoc
+ (BOOL)autosavesInPlace { return NO; }
- (void)makeWindowControllers {
    NSWindow *w = [[NSWindow alloc] initWithContentRect:NSMakeRect(200, 200, 300, 200)
                                              styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable
                                                backing:NSBackingStoreBuffered defer:NO];
    [w setReleasedWhenClosed:NO];
    [self addWindowController:[[NSWindowController alloc] initWithWindow:w]];
}
- (BOOL)readFromData:(NSData *)data ofType:(NSString *)type error:(NSError **)e { return YES; }
- (NSData *)dataOfType:(NSString *)type error:(NSError **)e { return [NSData data]; }
@end

@interface Delegate : NSObject <NSApplicationDelegate>
@end
@implementation Delegate
- (void)applicationWillFinishLaunching:(NSNotification *)n { probe_line("0350.event", @"applicationWillFinishLaunching", @"called"); }
- (void)applicationDidFinishLaunching:(NSNotification *)n { probe_line("0350.event", @"applicationDidFinishLaunching", @"called"); }
- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)sender {
    BOOL refuse = [[NSUserDefaults standardUserDefaults] boolForKey:@"probeRefuseUntitled"];
    probe_line("0350.event", @"applicationShouldOpenUntitledFile: asked", PF(@"answering %d", (int)!refuse));
    return !refuse;
}
@end

static NSString *state(void) {
    NSMutableArray *a = [NSMutableArray array];
    for (NSWindow *w in [NSApp windows]) {
        NSString *cls = [w isKindOfClass:[NSOpenPanel class]] ? @"NSOpenPanel" : [w isKindOfClass:[NSSavePanel class]] ? @"NSSavePanel" : [w isKindOfClass:[NSPanel class]] ? @"NSPanel" : @"NSWindow";
        [a addObject:PF(@"%@ visible=%d title=\"%@\"", cls, (int)[w isVisible], [w title])];
    }
    NSDocumentController *dc = [NSDocumentController sharedDocumentController];
    NSMutableArray *docs = [NSMutableArray array];
    for (NSDocument *d in [dc documents])
        [docs addObject:PF(@"%@ untitled=%d", NSStringFromClass([d class]), (int)([d fileURL] == nil))];
    return PF(@"windows=[%@] ; documents=%lu [%@] ; modalWindow=%@", [a componentsJoinedByString:@" | "], (unsigned long)[docs count],
              [docs componentsJoinedByString:@" | "], [NSApp modalWindow] ? @"yes" : @"no");
}

int main(int argc, char **argv) {
    @autoreleasepool {
        probe_env("0350");
        NSDictionary *info = [[NSBundle mainBundle] infoDictionary];
        probe_line("0350.bundle", @"Info.plist keys present", PF(@"CFBundleDocumentTypes=%d NSUbiquitousContainers=%d", (int)([info objectForKey:@"CFBundleDocumentTypes"] != nil), (int)([info objectForKey:@"NSUbiquitousContainers"] != nil)));
        NSUserDefaults *ud = [NSUserDefaults standardUserDefaults];
        id v = [ud objectForKey:@"NSShowAppCentricOpenPanelInsteadOfUntitledFile"];
        probe_line("0350.defaults", @"NSShowAppCentricOpenPanelInsteadOfUntitledFile", v ? PF(@"%@", v) : @"not set");
        probe_line("0350.defaults", @"probeRefuseUntitled", PF(@"%d", (int)[ud boolForKey:@"probeRefuseUntitled"]));
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];
        Delegate *d = [[Delegate alloc] init];
        [NSApp setDelegate:d];
        NSTimer *t = [NSTimer timerWithTimeInterval:2.0 repeats:NO block:^(NSTimer *tm) {
            probe_line("0350.state", @"2 s after launch", state());
            if ([NSApp modalWindow]) [NSApp abortModal];
            exit(0);
        }];
        [[NSRunLoop currentRunLoop] addTimer:t forMode:NSRunLoopCommonModes];
        [NSApp run];
    }
    return 0;
}
