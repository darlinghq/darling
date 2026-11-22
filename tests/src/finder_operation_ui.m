// CFLAGS: -framework Foundation -framework AppKit -fobjc-arc -Isrc/Finder src/Finder/FinderFileOperation.m src/Finder/FinderOperationQueue.m
// ARGS: "$(mktemp -d)/w"
// Copyright (C) 2026 Lubos Dolezel
//
// Finder's operation queue (src/Finder/FinderOperationQueue), in process: no panel for a quick copy; the progress
// panel of a long one (its text, a rising determinate indicator, and Stop, which leaves nothing behind); the conflict
// alert (its text, buttons and, for several items, "Apply to All"); a refused transfer and a failed one each giving
// an alert. Alerts are answered through the queue's alertResponder. See tests/finder.md. Needs a display.
// Usage: finder_operation_ui <new directory>. Prints PASS/FAIL/SKIP lines; exits 1 on a failure.
#import <AppKit/AppKit.h>
#import "FinderFileOperation.h"
#import "FinderOperationQueue.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

static int failures;

static void expect(BOOL ok, NSString *what)
{
    printf("%s: %s\n", ok ? "PASS" : "FAIL", [what UTF8String]);
    if (!ok)
        failures++;
}

static NSFileManager *fm;
static NSURL *root;

static NSURL *folder(NSString *relative)
{
    NSURL *url = [root URLByAppendingPathComponent: relative];
    [fm createDirectoryAtURL: url withIntermediateDirectories: YES attributes: nil error: NULL];
    return url;
}

static NSURL *file(NSString *relative, NSString *contents)
{
    NSURL *url = [root URLByAppendingPathComponent: relative];
    [fm createDirectoryAtURL: [url URLByDeletingLastPathComponent] withIntermediateDirectories: YES attributes: nil
                       error: NULL];
    [[contents dataUsingEncoding: NSUTF8StringEncoding] writeToURL: url atomically: NO];
    return url;
}

static BOOL exists(NSURL *url)
{
    struct stat st;
    return lstat([url fileSystemRepresentation], &st) == 0;
}

static void spinUntil(BOOL (^done)(void), NSTimeInterval seconds)
{
    NSDate *limit = [NSDate dateWithTimeIntervalSinceNow: seconds];
    while (!done() && [limit timeIntervalSinceNow] > 0)
        [[NSRunLoop currentRunLoop] runMode: NSDefaultRunLoopMode beforeDate: [NSDate dateWithTimeIntervalSinceNow: 0.01]];
}

static void subviews(NSView *view, NSMutableArray *into)
{
    for (NSView *sub in [view subviews]) {
        [into addObject: sub];
        subviews(sub, into);
    }
}

static id viewOfClass(NSWindow *window, Class cls, NSString *title)
{
    NSMutableArray *all = [NSMutableArray array];
    subviews([window contentView], all);
    for (NSView *view in all)
        if ([view isKindOfClass: cls] && (title == nil || [[(NSButton *) view title] isEqual: title]))
            return view;
    return nil;
}

static NSString *allText(NSWindow *window)
{
    NSMutableArray *all = [NSMutableArray array], *texts = [NSMutableArray array];
    subviews([window contentView], all);
    for (NSView *view in all)
        if ([view isKindOfClass: [NSTextField class]])
            [texts addObject: [(NSTextField *) view stringValue]];
    return [texts componentsJoinedByString: @"|"];
}

static NSArray<NSString *> *buttonTitles(NSAlert *alert)
{
    NSMutableArray *titles = [NSMutableArray array];
    for (NSButton *button in alert.buttons)
        [titles addObject: button.title];
    return titles;
}

int main(int argc, char **argv)
{
    @autoreleasepool {
        setvbuf(stdout, NULL, _IONBF, 0);
        if (argc < 2) {
            fprintf(stderr, "usage: %s <new directory>\n", argv[0]);
            return 2;
        }
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy: NSApplicationActivationPolicyRegular];
        fm = [NSFileManager defaultManager];
        root = [NSURL fileURLWithPath: [NSString stringWithUTF8String: argv[1]] isDirectory: YES];
        [fm createDirectoryAtURL: root withIntermediateDirectories: YES attributes: nil error: NULL];
        FinderOperationQueue *queue = [FinderOperationQueue sharedQueue];
        expect(queue == [FinderOperationQueue sharedQueue], @"one shared queue");
        expect(queue.progressDelay == 1.0, @"the progress panel waits 1 s");

        __block NSMutableArray<NSAlert *> *alerts = [NSMutableArray array];
        __block NSInteger answer = 0;
        __block BOOL tick = NO;
        queue.alertResponder = ^NSInteger(NSAlert *alert, BOOL *suppress) {
            [alerts addObject: alert];
            *suppress = tick;
            return answer;
        };

        // A quick copy shows no panel
        NSURL *dst = folder(@"dst");
        NSURL *one = file(@"src/one", @"1");
        FinderFileOperation *op = [queue transferItems: @[ one ] to: dst kind: FinderTransferCopy window: nil];
        expect(op != nil, @"a transfer starts");
        expect([queue.runningOperations containsObject: op], @"the queue holds it while it runs");
        __block BOOL sawPanel = NO;
        spinUntil(^BOOL {
            sawPanel = sawPanel || [queue progressPanelForOperation: op] != nil;
            return op.finished;
        }, 30);
        spinUntil(^BOOL { return NO; }, 0.2);
        expect(op.finished && exists([dst URLByAppendingPathComponent: @"one"]), @"a quick copy completes");
        expect(!sawPanel, @"a quick copy shows no panel");
        expect(![queue.runningOperations containsObject: op], @"the queue drops it once it is done");
        expect([alerts count] == 0, @"a quick copy asks nothing");

        // A long copy shows a panel, which Stop ends
        NSURL *big = [root URLByAppendingPathComponent: @"src/big"];
        // 1 GB, sparse: reading it is quick, writing its copy isn't; and the panel is asked for at once, so that the
        // copy is still running when it is shown however fast the disk is
        [[NSData data] writeToURL: big atomically: NO];
        truncate([big fileSystemRepresentation], 1LL << 30);
        queue.progressDelay = 0;
        op = [queue transferItems: @[ big ] to: dst kind: FinderTransferCopy window: nil];
        __block NSPanel *panel = nil;
        __block double highest = 0, first = -1;
        spinUntil(^BOOL {
            panel = [queue progressPanelForOperation: op];
            NSProgressIndicator *bar = panel ? viewOfClass(panel, [NSProgressIndicator class], nil) : nil;
            if (bar) {
                if (first < 0)
                    first = bar.doubleValue;
                highest = MAX(highest, bar.doubleValue);
            }
            return op.finished || (bar != nil && highest > first);
        }, 60);
        expect(panel != nil, @"a long copy shows a panel");
        expect([panel isVisible], @"the panel is on screen");
        expect([allText(panel) rangeOfString: @"Copying “big” to “dst”"].location != NSNotFound,
               [NSString stringWithFormat: @"the panel says what is copied where (%@)", allText(panel)]);
        NSProgressIndicator *bar = viewOfClass(panel, [NSProgressIndicator class], nil);
        expect(bar != nil && !bar.isIndeterminate, @"the panel has a determinate indicator");
        expect(highest > first, [NSString stringWithFormat: @"the indicator rises (%g to %g)", first, highest]);
        NSButton *stop = viewOfClass(panel, [NSButton class], @"Stop");
        expect(stop != nil, @"the panel has Stop");
        expect(!op.finished, @"the copy is still running when Stop is pressed");
        [stop performClick: nil];
        spinUntil(^BOOL { return op.finished; }, 60);
        spinUntil(^BOOL { return NO; }, 0.2);
        expect(op.finished && !exists([dst URLByAppendingPathComponent: @"big"]), @"Stop leaves no copy");
        expect(![panel isVisible], @"the panel closes");
        expect([alerts count] == 0, @"Stop isn't reported as an error");
        queue.progressDelay = 1.0;

        // The conflict alert
        file(@"dst/a.txt", @"old");
        NSURL *a = file(@"src/a.txt", @"new");
        answer = 1;
        op = [queue transferItems: @[ a ] to: dst kind: FinderTransferCopy window: nil];
        spinUntil(^BOOL { return op.finished; }, 30);
        expect([alerts count] == 1, [NSString stringWithFormat: @"a conflict asks once (%lu)", (unsigned long) [alerts count]]);
        NSAlert *alert = [alerts firstObject];
        expect([alert.messageText isEqual: @"An item named “a.txt” already exists in this location. Do you want to replace it with the one you're copying?"],
               [NSString stringWithFormat: @"the conflict alert's text (%@)", alert.messageText]);
        expect([buttonTitles(alert) isEqual: (@[ @"Replace", @"Keep Both", @"Stop" ])],
               [NSString stringWithFormat: @"the conflict alert's buttons (%@)", buttonTitles(alert)]);
        expect(!alert.showsSuppressionButton, @"no Apply to All for one item");
        expect(exists([dst URLByAppendingPathComponent: @"a 2.txt"]), @"Keep Both keeps both");

        // Apply to All for several
        [alerts removeAllObjects];
        NSMutableArray *three = [NSMutableArray array];
        NSURL *many = folder(@"many");
        for (NSString *name in @[ @"p", @"q", @"r" ]) {
            [three addObject: file([@"src/" stringByAppendingString: name], name)];
            file([@"many/" stringByAppendingString: name], @"there");
        }
        tick = YES;
        op = [queue transferItems: three to: many kind: FinderTransferMove window: nil];
        spinUntil(^BOOL { return op.finished; }, 30);
        alert = [alerts firstObject];
        expect([alerts count] == 1 && alert.showsSuppressionButton && [alert.suppressionButton.title isEqual: @"Apply to All"],
               [NSString stringWithFormat: @"several conflicts: one alert with Apply to All (%lu, %@)", (unsigned long) [alerts count],
                                          alert.suppressionButton.title]);
        expect([alert.messageText hasSuffix: @"with the one you're moving?"], @"the alert says moving for a move");
        expect(exists([many URLByAppendingPathComponent: @"p 2"]) && exists([many URLByAppendingPathComponent: @"r 2"]),
               @"Apply to All answers the rest");
        tick = NO;

        // A refused transfer
        [alerts removeAllObjects];
        NSURL *f = folder(@"f");
        NSURL *sub = folder(@"f/sub");
        answer = 0;
        op = [queue transferItems: @[ f ] to: sub kind: FinderTransferCopy window: nil];
        expect(op == nil, @"a folder into itself starts nothing");
        expect([alerts count] == 1 && [[alerts.firstObject messageText] rangeOfString: @"“f”"].location != NSNotFound,
               [NSString stringWithFormat: @"a folder into itself: an alert naming it (%@)", [alerts.firstObject messageText]]);
        expect([[fm contentsOfDirectoryAtPath: [sub path] error: NULL] count] == 0, @"a folder into itself: nothing written");

        // A failed transfer
        if (geteuid() == 0)
            printf("SKIP: a failed copy: root reads anything\n");
        else {
            [alerts removeAllObjects];
            NSURL *t = folder(@"src/t");
            NSURL *no = file(@"src/t/no", @"no");
            chmod([no fileSystemRepresentation], 0);
            op = [queue transferItems: @[ t ] to: dst kind: FinderTransferCopy window: nil];
            spinUntil(^BOOL { return op.finished && [alerts count] > 0; }, 30);
            alert = [alerts firstObject];
            NSString *text = [NSString stringWithFormat: @"%@ %@", alert.messageText, alert.informativeText];
            expect([alerts count] == 1 && [text rangeOfString: @"“t”"].location != NSNotFound,
                   [NSString stringWithFormat: @"a failed copy: one alert naming the item (%@)", text]);
            expect([buttonTitles(alert) isEqual: @[ @"OK" ]], @"a failed copy: the alert has OK");
            chmod([no fileSystemRepresentation], 0644);
        }

        printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
    }
    return failures != 0;
}
