// Probe 669: error reporting of -[NSDocumentController makeDocumentWithContentsOfURL:ofType:error:] and of every
// public open path built on it, including errors that mean "the user cancelled".
// The probe relaunches itself in a throw-away application bundle (probe_bundle.h) that declares eight types, each
// with a tiny NSDocument subclass that logs every initialiser and reader the controller calls on it:
//   ProbeGood        succeeds
//   ProbeReadErr     readFromURL:ofType:error: returns NO with error ProbeDomain/7
//   ProbeReadCancel  readFromURL:ofType:error: returns NO with NSCocoaErrorDomain/NSUserCancelledError (3072)
//   ProbeReadNoErr   readFromURL:ofType:error: returns NO and leaves the error alone (nil)
//   ProbeInitErr     initWithContentsOfURL:ofType:error: returns nil with error ProbeDomain/8
//   ProbeInitCancel  initWithContentsOfURL:ofType:error: returns nil with NSUserCancelledError
//   ProbeInitNoErr   initWithContentsOfURL:ofType:error: returns nil and leaves the error alone
//   ProbeNoClass     NSDocumentClass names a class that does not exist
// Open paths exercised for each: makeDocumentWithContentsOfURL:ofType:error: (also with error NULL),
// makeDocumentForURL:withContentsOfURL:ofType:error:, openDocumentWithContentsOfURL:display:completionHandler:,
// openDocumentWithContentsOfURL:display:error: (if present), reopenDocumentForURL:withContentsOfURL:display:completionHandler:
// (the last three: whether the error reaches the handler and the result).
// Presentation: NSApplication is subclassed to log presentError: and presentError:modalForWindow:... without showing
// anything, and a watchdog aborts any modal window the controller starts by itself (reported as MODAL-WINDOW).
// Prints the returned document, the error (domain, code, description), the initialiser/reader calls in order,
// and the presentation lines. Window server: yes (no window is made by the probe; the application is not activated).
#define PROBE_APPKIT 1
#import "probe_common.h"
#import "probe_bundle.h"
#include <unistd.h>

static NSMutableArray *events;
static void ev(NSString *s) { [events addObject:s]; }
static NSString *joined(void) { return [events count] ? [events componentsJoinedByString:@" > "] : @"(none)"; }
static NSString *err(NSError *e) { return e ? PF(@"%@/%ld \"%@\"", [e domain], (long)[e code], [e localizedDescription] ?: @"") : @"nil"; }

@interface TApp : NSApplication
@end
@implementation TApp
- (BOOL)presentError:(NSError *)e { ev(PF(@"PRESENT presentError:(%@)", err(e))); return YES; }
- (void)presentError:(NSError *)e modalForWindow:(NSWindow *)w delegate:(id)d didPresentSelector:(SEL)s contextInfo:(void *)c { ev(PF(@"PRESENT presentError:modalForWindow:(%@)", err(e))); }
@end

@interface BaseDoc : NSDocument
@end
@implementation BaseDoc
- (instancetype)init { ev(@"init"); return [super init]; }
- (instancetype)initWithType:(NSString *)t error:(NSError **)e { ev(@"initWithType:error:"); return [super initWithType:t error:e]; }
- (instancetype)initWithContentsOfURL:(NSURL *)u ofType:(NSString *)t error:(NSError **)e { ev(PF(@"%@ initWithContentsOfURL:ofType:error:", NSStringFromClass([self class]))); return [super initWithContentsOfURL:u ofType:t error:e]; }
- (instancetype)initForURL:(NSURL *)u withContentsOfURL:(NSURL *)c ofType:(NSString *)t error:(NSError **)e { ev(@"initForURL:withContentsOfURL:ofType:error:"); return [super initForURL:u withContentsOfURL:c ofType:t error:e]; }
- (BOOL)readFromURL:(NSURL *)u ofType:(NSString *)t error:(NSError **)e { ev(@"readFromURL:ofType:error:"); return [super readFromURL:u ofType:t error:e]; }
- (BOOL)readFromData:(NSData *)d ofType:(NSString *)t error:(NSError **)e { ev(@"readFromData:ofType:error:"); return YES; }
- (NSData *)dataOfType:(NSString *)t error:(NSError **)e { return [NSData data]; }
@end
@interface GoodDoc : BaseDoc @end
@implementation GoodDoc @end
@interface ReadErrDoc : BaseDoc @end
@implementation ReadErrDoc
- (BOOL)readFromURL:(NSURL *)u ofType:(NSString *)t error:(NSError **)e { ev(@"readFromURL:ofType:error:"); if (e) *e = [NSError errorWithDomain:@"ProbeDomain" code:7 userInfo:@{NSLocalizedDescriptionKey: @"probe read failure"}]; return NO; }
@end
@interface ReadCancelDoc : BaseDoc @end
@implementation ReadCancelDoc
- (BOOL)readFromURL:(NSURL *)u ofType:(NSString *)t error:(NSError **)e { ev(@"readFromURL:ofType:error:"); if (e) *e = [NSError errorWithDomain:NSCocoaErrorDomain code:NSUserCancelledError userInfo:nil]; return NO; }
@end
@interface ReadNoErrDoc : BaseDoc @end
@implementation ReadNoErrDoc
- (BOOL)readFromURL:(NSURL *)u ofType:(NSString *)t error:(NSError **)e { ev(@"readFromURL:ofType:error:"); return NO; }
@end
@interface InitErrDoc : BaseDoc @end
@implementation InitErrDoc
- (instancetype)initWithContentsOfURL:(NSURL *)u ofType:(NSString *)t error:(NSError **)e { ev(@"InitErrDoc initWithContentsOfURL:ofType:error:"); if (e) *e = [NSError errorWithDomain:@"ProbeDomain" code:8 userInfo:@{NSLocalizedDescriptionKey: @"probe init failure"}]; return nil; }
@end
@interface InitCancelDoc : BaseDoc @end
@implementation InitCancelDoc
- (instancetype)initWithContentsOfURL:(NSURL *)u ofType:(NSString *)t error:(NSError **)e { ev(@"InitCancelDoc initWithContentsOfURL:ofType:error:"); if (e) *e = [NSError errorWithDomain:NSCocoaErrorDomain code:NSUserCancelledError userInfo:nil]; return nil; }
@end
@interface InitNoErrDoc : BaseDoc @end
@implementation InitNoErrDoc
- (instancetype)initWithContentsOfURL:(NSURL *)u ofType:(NSString *)t error:(NSError **)e { ev(@"InitNoErrDoc initWithContentsOfURL:ofType:error:"); return nil; }
@end

static NSDictionary *T(NSString *name, NSString *cls, NSString *ext) {
    return @{@"CFBundleTypeName": name, @"CFBundleTypeRole": @"Editor", @"NSDocumentClass": cls, @"CFBundleTypeExtensions": @[ext]};
}

static NSArray *docTypes(void) {
    return @[T(@"ProbeGood", @"GoodDoc", @"pgood"), T(@"ProbeReadErr", @"ReadErrDoc", @"preaderr"), T(@"ProbeReadCancel", @"ReadCancelDoc", @"preadcancel"),
             T(@"ProbeReadNoErr", @"ReadNoErrDoc", @"preadnoerr"), T(@"ProbeInitErr", @"InitErrDoc", @"piniterr"), T(@"ProbeInitCancel", @"InitCancelDoc", @"pinitcancel"),
             T(@"ProbeInitNoErr", @"InitNoErrDoc", @"pinitnoerr"), T(@"ProbeNoClass", @"ClassThatDoesNotExist", @"pnoclass")];
}

static void watchdog(void) {
    NSTimer *tm = [NSTimer timerWithTimeInterval:1.0 repeats:NO block:^(NSTimer *t) {
        NSWindow *m = [NSApp modalWindow];
        if (m) { ev(PF(@"MODAL-WINDOW(%@ \"%@\")", NSStringFromClass([m class]), [m title] ?: @"")); [NSApp abortModal]; }
    }];
    [[NSRunLoop currentRunLoop] addTimer:tm forMode:NSRunLoopCommonModes];
}

static NSString *windowsNow(void) {
    NSMutableArray *a = [NSMutableArray array];
    for (NSWindow *w in [NSApp windows]) if ([w isVisible]) [a addObject:NSStringFromClass([w class])];
    return [a count] ? [a componentsJoinedByString:@","] : @"none";
}

static void closeAll(NSDocumentController *dc) { for (NSDocument *d in [[dc documents] copy]) [d close]; }

static void child(const char *tag) {
    [TApp sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];
    NSDocumentController *dc = [NSDocumentController sharedDocumentController];
    NSFileManager *fm = [NSFileManager defaultManager];
    NSString *dir = [NSTemporaryDirectory() stringByAppendingPathComponent:PF(@"p669.%d", (int)getpid())];
    [fm createDirectoryAtPath:dir withIntermediateDirectories:YES attributes:nil error:NULL];
    probe_line("669.env", @"TApp is NSApp / defaultType", PF(@"%d / %@", (int)[NSApp isKindOfClass:[TApp class]], [dc defaultType] ?: @"nil"));
    for (NSDictionary *t in docTypes()) {
        NSString *type = t[@"CFBundleTypeName"], *ext = t[@"CFBundleTypeExtensions"][0];
        NSString *path = [dir stringByAppendingPathComponent:[@"sample." stringByAppendingString:ext]];
        [[@"probe\n" dataUsingEncoding:NSUTF8StringEncoding] writeToFile:path atomically:NO];
        NSURL *url = [NSURL fileURLWithPath:path];
        NSString *tn = [type substringFromIndex:5];
        probe_line("669.type", PF(@"%@", type), PF(@"typeForContentsOfURL=%@ ; documentClassForType=%@", [dc typeForContentsOfURL:url error:NULL] ?: @"nil", [dc documentClassForType:type] ? NSStringFromClass([dc documentClassForType:type]) : @"nil"));

        // 1. make...
        events = [NSMutableArray array]; watchdog();
        __block NSError *e = nil; __block NSDocument *d = nil;
        NSString *ex = probe_try(^{ d = [dc makeDocumentWithContentsOfURL:url ofType:type error:&e]; });
        probe_line("669.make", PF(@"%@ makeDocumentWithContentsOfURL:ofType:error:", tn), PF(@"%@ ; doc=%@ ; error=%@ ; calls=[%@]", ex, d ? NSStringFromClass([d class]) : @"nil", err(e), joined()));
        closeAll(dc);
        events = [NSMutableArray array];
        ex = probe_try(^{ d = [dc makeDocumentWithContentsOfURL:url ofType:type error:NULL]; });
        probe_line("669.make", PF(@"%@ makeDocumentWithContentsOfURL:ofType:error:NULL", tn), PF(@"%@ ; doc=%@", ex, d ? NSStringFromClass([d class]) : @"nil"));
        closeAll(dc);
        // 2. makeDocumentForURL:
        events = [NSMutableArray array]; e = nil;
        ex = probe_try(^{ d = [dc makeDocumentForURL:nil withContentsOfURL:url ofType:type error:&e]; });
        probe_line("669.make", PF(@"%@ makeDocumentForURL:nil withContentsOfURL:ofType:error:", tn), PF(@"%@ ; doc=%@ ; error=%@ ; calls=[%@]", ex, d ? NSStringFromClass([d class]) : @"nil", err(e), joined()));
        closeAll(dc);
        // 3. open ... completionHandler
        events = [NSMutableArray array]; watchdog();
        __block BOOL done = NO;
        ex = probe_try(^{ [dc openDocumentWithContentsOfURL:url display:NO completionHandler:^(NSDocument *doc, BOOL was, NSError *er) { ev(PF(@"HANDLER(doc=%@ wasAlreadyOpen=%d error=%@)", doc ? NSStringFromClass([doc class]) : @"nil", (int)was, err(er))); done = YES; }]; });
        for (int i = 0; i < 20 && !done; i++) probe_spin(0.05);
        probe_spin(0.3);
        probe_line("669.open", PF(@"%@ openDocumentWithContentsOfURL:display:completionHandler:", tn), PF(@"%@ ; handler ran=%d ; events=[%@] ; visible windows=%@", ex, (int)done, joined(), windowsNow()));
        closeAll(dc);
        // 4. open ... error:
        SEL sel = NSSelectorFromString(@"openDocumentWithContentsOfURL:display:error:");
        if ([dc respondsToSelector:sel]) {
            events = [NSMutableArray array]; watchdog(); e = nil;
            NSDocument *(*imp)(id, SEL, NSURL *, BOOL, NSError **) = (void *)[dc methodForSelector:sel];
            ex = probe_try(^{ d = imp(dc, sel, url, NO, &e); });
            probe_spin(0.3);
            probe_line("669.open", PF(@"%@ openDocumentWithContentsOfURL:display:error:", tn), PF(@"%@ ; doc=%@ ; error=%@ ; events=[%@] ; visible windows=%@", ex, d ? NSStringFromClass([d class]) : @"nil", err(e), joined(), windowsNow()));
            closeAll(dc);
        } else probe_line("669.open", PF(@"%@ openDocumentWithContentsOfURL:display:error:", tn), @"not available");
        // 5. reopen
        events = [NSMutableArray array]; watchdog(); done = NO;
        ex = probe_try(^{ [dc reopenDocumentForURL:nil withContentsOfURL:url display:NO completionHandler:^(NSDocument *doc, BOOL was, NSError *er) { ev(PF(@"HANDLER(doc=%@ wasAlreadyOpen=%d error=%@)", doc ? NSStringFromClass([doc class]) : @"nil", (int)was, err(er))); done = YES; }]; });
        for (int i = 0; i < 20 && !done; i++) probe_spin(0.05);
        probe_spin(0.3);
        probe_line("669.open", PF(@"%@ reopenDocumentForURL:nil withContentsOfURL:display:completionHandler:", tn), PF(@"%@ ; handler ran=%d ; events=[%@] ; visible windows=%@", ex, (int)done, joined(), windowsNow()));
        closeAll(dc);
    }
    [fm removeItemAtPath:dir error:NULL];
}

int main(int argc, char **argv) {
    @autoreleasepool {
        if (probe_has_arg(argc, argv, "--child")) { child(argc > 2 ? argv[2] : "?"); return 0; }
        probe_env("669");
        probe_run_in_bundle(docTypes(), @"b1", @[@"--child", @"b1"]);
    }
    return 0;
}
