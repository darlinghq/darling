// Probe 0056, two independent questions.
// PART 1 (always runs, nothing shown): the contract of -[NSPrintOperation runOperationModalForWindow:delegate:
//   didRunSelector:contextInfo:] for an operation that shows no panel and saves a PDF to a temporary file:
//   is the delegate called before or after the call returns, with what arguments, how often?
// PART 2 (runs only with the argument --ui, may open windows or another application): what the application
//   does for -[NSHelpManager findString:inBook:] and -openHelpAnchor:inBook: and -[NSApplication showHelp:]
//   when it has no help book. Windows that appear in this process are listed, then closed by the probe.
#define PROBE_APPKIT 1
#import "probe_common.h"
#include <unistd.h>

@interface Delegate : NSObject
@property int calls;
@property(copy) NSString *detail;
@property BOOL returnedYet;
@property BOOL calledBeforeReturn;
@end
@implementation Delegate
- (void)printOperationDidRun:(NSPrintOperation *)op success:(BOOL)success contextInfo:(void *)ctx {
    self.calls++;
    self.calledBeforeReturn = !self.returnedYet;
    self.detail = PF(@"operation-is-the-one-passed-in=%d success=%d contextInfo=%p", (int)(op != nil), (int)success, ctx);
}
@end

static NSString *listWindows(void) {
    NSMutableArray *a = [NSMutableArray array];
    for (NSWindow *w in [NSApp windows]) {
        NSString *cls = [w isKindOfClass:[NSPanel class]] ? @"NSPanel" : @"NSWindow";
        NSMutableArray *texts = [NSMutableArray array];
        NSMutableArray *stack = [NSMutableArray arrayWithObject:[w contentView]];
        while ([stack count]) {
            NSView *v = [stack lastObject];
            [stack removeLastObject];
            if ([v isKindOfClass:[NSTextField class]] && [[(NSTextField *)v stringValue] length])
                [texts addObject:PF(@"\"%@\"", [(NSTextField *)v stringValue])];
            if ([v isKindOfClass:[NSButton class]] && [[(NSButton *)v title] length])
                [texts addObject:PF(@"button \"%@\"", [(NSButton *)v title])];
            [stack addObjectsFromArray:[v subviews]];
        }
        [a addObject:PF(@"%@ visible=%d modal=%d title=\"%@\" texts=[%@]", cls, (int)[w isVisible], (int)([NSApp modalWindow] == w), [w title], [texts componentsJoinedByString:@" ; "])];
    }
    return [a count] ? [a componentsJoinedByString:@" || "] : @"no windows in this process";
}

int main(int argc, char **argv) {
    @autoreleasepool {
        probe_env("0056");
        probe_app();
        // PART 1
        NSView *content = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 200, 100)];
        NSString *path = [NSTemporaryDirectory() stringByAppendingPathComponent:PF(@"probe0056-%d.pdf", getpid())];
        NSPrintInfo *info = [[NSPrintInfo sharedPrintInfo] copy];
        [info setJobDisposition:NSPrintSaveJob];
        [[info dictionary] setObject:path forKey:NSPrintSavePath];
        NSPrintOperation *op = [NSPrintOperation printOperationWithView:content printInfo:info];
        [op setShowsPrintPanel:NO];
        [op setShowsProgressPanel:NO];
        Delegate *d = [[Delegate alloc] init];
        __block NSString *r = probe_try(^{
            [op runOperationModalForWindow:nil delegate:d didRunSelector:@selector(printOperationDidRun:success:contextInfo:) contextInfo:(void *)0x1234];
        });
        d.returnedYet = YES;
        probe_line("0056.1", @"runOperationModalForWindow:nil, no panels, job disposition save-to-file",
                   PF(@"%@ ; delegate called %d time(s) ; called before the method returned: %d ; %@ ; PDF file written: %d",
                      r, d.calls, (int)d.calledBeforeReturn, d.detail ?: @"(never called)", (int)[[NSFileManager defaultManager] fileExistsAtPath:path]));
        // Let any deferred callback arrive.
        probe_spin(0.5);
        probe_line("0056.1b", @"after 0.5 s of running the run loop", PF(@"delegate called %d time(s) in total", d.calls));
        [[NSFileManager defaultManager] removeItemAtPath:path error:nil];

        // PART 2
        BOOL ui = NO;
        for (int i = 1; i < argc; i++) if (!strcmp(argv[i], "--ui")) ui = YES;
        if (!ui) {
            probe_line("0056.2", @"NSHelpManager / showHelp: with no help book", @"SKIPPED (run with --ui)");
            return 0;
        }
        probe_line("0056.2", @"main bundle has a help book (CFBundleHelpBookName)", PF(@"%d", (int)([[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleHelpBookName"] != nil)));
        NSArray *steps = @[
            @{@"name": @"-[NSApplication showHelp:]", @"call": [^{ [NSApp showHelp:nil]; } copy]},
            @{@"name": @"-[NSHelpManager openHelpAnchor:inBook:] with anchor \"x\" and book nil", @"call": [^{ [[NSHelpManager sharedHelpManager] openHelpAnchor:@"x" inBook:nil]; } copy]},
            @{@"name": @"-[NSHelpManager findString:inBook:] with string \"x\" and book nil", @"call": [^{ [[NSHelpManager sharedHelpManager] findString:@"x" inBook:nil]; } copy]},
        ];
        for (NSDictionary *step in steps) {
            NSString *name = step[@"name"];
            void (^call)(void) = step[@"call"];
            CFAbsoluteTime t0 = CFAbsoluteTimeGetCurrent();
            // A modal alert would block the call; a timer inspects the windows and ends the modal session.
            NSTimer *timer = [NSTimer timerWithTimeInterval:1.0 repeats:NO block:^(NSTimer *t) {
                probe_line("0056.2.windows", PF(@"1 s into %@", name), listWindows());
                if ([NSApp modalWindow]) [NSApp abortModal];
            }];
            [[NSRunLoop currentRunLoop] addTimer:timer forMode:NSRunLoopCommonModes];
            NSString *res = probe_try(call);
            probe_line("0056.2", name, PF(@"%@ ; the call returned after %.1f s", res, CFAbsoluteTimeGetCurrent() - t0));
            probe_spin(1.5);
            for (NSWindow *w in [[NSApp windows] copy]) [w close];
        }
    }
    return 0;
}
