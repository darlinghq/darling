// Probe 2406: what is publicly constructible or observable of ASWebAuthenticationSessionRequest and
// ASWebAuthenticationSessionWebBrowserSessionManager. A request with a URL and UUID is only made by the system
// when a client starts a session, so this probe cannot build one with values; it observes what a bare request,
// the shared manager and the delegate protocol do. No window, no browser, no network.
// Each section runs in its own child copy of this program so a crash (a nil URL, an unavailable initializer)
// is reported as a line and the other sections still run. Selectors are used by name: nothing here depends on
// the SDK declaring the classes.
#import "probe_common.h"
#import <AuthenticationServices/AuthenticationServices.h>
#import <objc/message.h>
#include <pthread.h>

static BOOL respondsToSel(id o, const char *s);
static NSString *yesno(BOOL b) { return b ? @"YES" : @"NO"; }
static id msg0(id t, const char *s) { SEL x = sel_registerName(s); return [t respondsToSelector:x] ? ((id(*)(id, SEL))objc_msgSend)(t, x) : (id)@"<selector n/a>"; }

// A delegate that records every call, the thread, and the order.
@interface Rec : NSObject
@property NSMutableArray *log;
@end
@implementation Rec
- (instancetype)init { if ((self = [super init])) _log = [NSMutableArray array]; return self; }
- (void)authenticationSessionRequest:(id)request didCompleteWithCallbackURL:(NSURL *)url {
    [_log addObject:PF(@"didComplete(%@) on %@ thread", [url absoluteString], [NSThread isMainThread] ? @"main" : @"other")];
}
- (void)authenticationSessionRequest:(id)request didCancelWithError:(NSError *)error {
    [_log addObject:PF(@"didCancel(%@/%ld) on %@ thread", [error domain], (long)[error code], [NSThread isMainThread] ? @"main" : @"other")];
}
@end

// A session handler object (the protocol's two methods) that records calls.
@interface Handler : NSObject
@property NSMutableArray *log;
@end
@implementation Handler
- (instancetype)init { if ((self = [super init])) _log = [NSMutableArray array]; return self; }
- (void)beginHandlingWebAuthenticationSessionRequest:(id)request { [_log addObject:@"begin"]; }
- (void)cancelWebAuthenticationSessionRequest:(id)request { [_log addObject:@"cancel"]; }
@end

static id makeBareRequest(void) {
    Class c = NSClassFromString(@"ASWebAuthenticationSessionRequest");
    return c ? [[c alloc] init] : nil; // -init may be marked unavailable in headers; called by selector name
}

static void sectionClasses(void) {
    for (NSString *n in @[@"ASWebAuthenticationSessionRequest", @"ASWebAuthenticationSessionWebBrowserSessionManager",
                          @"ASWebAuthenticationSession"]) {
        Class c = NSClassFromString(n);
        probe_line("2406.A.class", n, c ? PF(@"present ; superclass %@ ; supportsSecureCoding %@ ; conforms NSCopying %@",
                                             NSStringFromClass(class_getSuperclass(c)),
                                             [c respondsToSelector:@selector(supportsSecureCoding)] ? yesno([(id)c supportsSecureCoding]) : @"n/a",
                                             yesno(class_conformsToProtocol(c, @protocol(NSCopying)))) : @"missing");
    }
    for (NSString *n in @[@"ASWebAuthenticationSessionRequestDelegate", @"ASWebAuthenticationSessionWebBrowserSessionHandling"]) {
        Protocol *p = NSProtocolFromString(n);
        if (!p) { probe_line("2406.A.protocol", n, @"missing"); continue; }
        NSMutableArray *out = [NSMutableArray array];
        for (int req = 0; req < 2; req++) {
            unsigned cnt = 0;
            struct objc_method_description *m = protocol_copyMethodDescriptionList(p, req ? YES : NO, YES, &cnt);
            for (unsigned i = 0; i < cnt; i++) [out addObject:PF(@"%s %s", req ? "required" : "optional", sel_getName(m[i].name))];
            free(m);
        }
        probe_line("2406.A.protocol", n, [out componentsJoinedByString:@" ; "]);
    }
}

static void sectionBareRequest(void) {
    Class c = NSClassFromString(@"ASWebAuthenticationSessionRequest");
    if (!c) { probe_line("2406.B", @"class", @"missing"); return; }
    __block id r = nil;
    NSString *exc = probe_try(^{ r = makeBareRequest(); });
    probe_line("2406.B.init", @"[[ASWebAuthenticationSessionRequest alloc] init]", r ? @"object returned" : PF(@"nil ; %@", exc));
    if (!r) return;
    probe_line("2406.B.properties", @"URL", [msg0(r, "URL") description] ?: @"nil");
    probe_line("2406.B.properties", @"UUID", msg0(r, "UUID") ? @"non-nil" : @"nil");
    probe_line("2406.B.properties", @"callbackURLScheme", [msg0(r, "callbackURLScheme") description] ?: @"nil");
    probe_line("2406.B.properties", @"shouldUseEphemeralSession", respondsToSel(r, "shouldUseEphemeralSession") ?
               yesno(((BOOL(*)(id, SEL))objc_msgSend)(r, sel_registerName("shouldUseEphemeralSession"))) : @"n/a");
    probe_line("2406.B.properties", @"additionalHeaderFields", [msg0(r, "additionalHeaderFields") description] ?: @"nil");
    probe_line("2406.B.properties", @"delegate", [msg0(r, "delegate") description] ?: @"nil");
    __block id cp = nil;
    exc = probe_try(^{ cp = [r copy]; });
    probe_line("2406.B.copy", @"-copy of the bare request", cp ? PF(@"new object: %@ ; same class: %@", yesno(cp != r), yesno([cp class] == [r class])) : PF(@"nil ; %@", exc));
    probe_line("2406.B.equal", @"isEqual of a request to its copy", cp ? yesno([r isEqual:cp]) : @"n/a");

    NSError *err = nil;
    NSData *data = nil;
    @try { data = [NSKeyedArchiver archivedDataWithRootObject:r requiringSecureCoding:YES error:&err]; }
    @catch (NSException *e) { probe_line("2406.B.archive", @"secure archive of the bare request", PF(@"RAISED %@", [e name])); data = nil; }
    probe_line("2406.B.archive", @"secure archive of the bare request", data ? @"encoded" : PF(@"failed %@/%ld", err.domain, (long)err.code));
    if (data) {
        NSError *e2 = nil;
        id back = nil;
        @try { back = [NSKeyedUnarchiver unarchivedObjectOfClass:c fromData:data error:&e2]; }
        @catch (NSException *e) { probe_line("2406.B.unarchive", @"decode it again", PF(@"RAISED %@", [e name])); }
        probe_line("2406.B.unarchive", @"decode the archive of a request that has no URL or UUID (decoding fails when they are absent?)",
                   back ? @"decoded an object" : PF(@"nil ; %@/%ld", e2.domain, (long)e2.code));
    }
    // An archive that names no URL/UUID built from plain Foundation values: an empty keyed archive of the class.
    NSMutableData *md = [NSMutableData data];
    NSKeyedArchiver *ka = [[NSKeyedArchiver alloc] initRequiringSecureCoding:NO];
    [ka encodeObject:@"unrelated" forKey:@"unrelated-key"];
    [ka finishEncoding];
    NSError *e3 = nil;
    id empty = nil;
    @try { empty = [[c alloc] initWithCoder:[[NSKeyedUnarchiver alloc] initForReadingFromData:[ka encodedData] error:&e3]]; }
    @catch (NSException *e) { probe_line("2406.B.init-coder", @"initWithCoder: on an unrelated archive", PF(@"RAISED %@", [e name])); }
    probe_line("2406.B.init-coder", @"initWithCoder: on an archive with no request keys", empty ? @"object" : @"nil");
    (void)md;
}

static BOOL respondsToSel(id o, const char *s) { return [o respondsToSelector:sel_registerName(s)]; }

static void sectionDelegate(void) {
    id r = makeBareRequest();
    if (!r) { probe_line("2406.C", @"bare request", @"n/a"); return; }
    Rec *d = [[Rec alloc] init];
    NSString *exc = probe_try(^{ [r setValue:d forKey:@"delegate"]; });
    probe_line("2406.C.delegate", @"setDelegate:", PF(@"%@ ; read back same object: %@", exc, yesno(msg0(r, "delegate") == d)));
    // Is the delegate held weakly?
    __weak Rec *weakD = nil;
    @autoreleasepool {
        Rec *tmp = [[Rec alloc] init];
        weakD = tmp;
        [r setValue:tmp forKey:@"delegate"];
    }
    probe_line("2406.C.delegate-weak", @"delegate released by its only owner", PF(@"request.delegate afterwards: %@ ; object alive: %@",
               msg0(r, "delegate") ? @"non-nil" : @"nil", yesno(weakD != nil)));
    [r setValue:d forKey:@"delegate"];

    NSURL *cb = [NSURL URLWithString:@"myapp://callback?code=1"];
    NSError *cancel = [NSError errorWithDomain:@"probe" code:7 userInfo:nil];
    ((void(*)(id, SEL, id))objc_msgSend)(r, sel_registerName("completeWithCallbackURL:"), cb);
    probe_line("2406.C.complete", @"completeWithCallbackURL: once", [d.log componentsJoinedByString:@" | "]);
    ((void(*)(id, SEL, id))objc_msgSend)(r, sel_registerName("completeWithCallbackURL:"), cb);
    probe_line("2406.C.complete-twice", @"completeWithCallbackURL: a second time (once-only rule?)", [d.log componentsJoinedByString:@" | "]);
    ((void(*)(id, SEL, id))objc_msgSend)(r, sel_registerName("cancelWithError:"), cancel);
    probe_line("2406.C.cancel-after-complete", @"cancelWithError: after a completion", [d.log componentsJoinedByString:@" | "]);

    Rec *d2 = [[Rec alloc] init];
    id r2 = makeBareRequest();
    [r2 setValue:d2 forKey:@"delegate"];
    ((void(*)(id, SEL, id))objc_msgSend)(r2, sel_registerName("cancelWithError:"), cancel);
    ((void(*)(id, SEL, id))objc_msgSend)(r2, sel_registerName("cancelWithError:"), cancel);
    probe_line("2406.C.cancel-twice", @"cancelWithError: twice on a fresh request", [d2.log componentsJoinedByString:@" | "]);
    ((void(*)(id, SEL, id))objc_msgSend)(r2, sel_registerName("completeWithCallbackURL:"), cb);
    probe_line("2406.C.complete-after-cancel", @"completeWithCallbackURL: after a cancel", [d2.log componentsJoinedByString:@" | "]);

    Rec *d3 = [[Rec alloc] init];
    id r3 = makeBareRequest();
    [r3 setValue:d3 forKey:@"delegate"];
    ((void(*)(id, SEL, id))objc_msgSend)(r3, sel_registerName("cancelWithError:"), nil);
    probe_line("2406.C.cancel-nil", @"cancelWithError:nil", [d3.log componentsJoinedByString:@" | "]);
    id r4 = makeBareRequest();
    [r4 setValue:d3 forKey:@"delegate"];
    ((void(*)(id, SEL, id))objc_msgSend)(r4, sel_registerName("completeWithCallbackURL:"), nil);
    probe_line("2406.C.complete-nil", @"completeWithCallbackURL:nil", [d3.log componentsJoinedByString:@" | "]);

    // Threads: call from a background thread; is the delegate called on that thread or on the main thread?
    Rec *d5 = [[Rec alloc] init];
    id r5 = makeBareRequest();
    [r5 setValue:d5 forKey:@"delegate"];
    dispatch_semaphore_t sem = dispatch_semaphore_create(0);
    dispatch_async(dispatch_get_global_queue(0, 0), ^{
        ((void(*)(id, SEL, id))objc_msgSend)(r5, sel_registerName("completeWithCallbackURL:"), cb);
        dispatch_semaphore_signal(sem);
    });
    dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);
    probe_line("2406.C.thread", @"completeWithCallbackURL: from a background queue, immediately after it returns", [d5.log componentsJoinedByString:@" | "]);
    [[NSRunLoop mainRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.5]];
    probe_line("2406.C.thread", @"same, 0.5 s later (main queue delivery?)", [d5.log componentsJoinedByString:@" | "]);
    // Delegate on the main thread, call on the main thread: synchronous?
    Rec *d6 = [[Rec alloc] init];
    id r6 = makeBareRequest();
    [r6 setValue:d6 forKey:@"delegate"];
    ((void(*)(id, SEL, id))objc_msgSend)(r6, sel_registerName("completeWithCallbackURL:"), cb);
    probe_line("2406.C.sync", @"main-thread call: delegate already called when the call returns", PF(@"%lu calls", (unsigned long)d6.log.count));
}

static void sectionManager(void) {
    Class m = NSClassFromString(@"ASWebAuthenticationSessionWebBrowserSessionManager");
    if (!m) { probe_line("2406.D", @"class", @"missing"); return; }
    id a = msg0(m, "sharedManager"), b = msg0(m, "sharedManager");
    probe_line("2406.D.shared", @"+sharedManager twice", PF(@"non-nil %@ ; same object %@", yesno(a != nil), yesno(a == b)));
    probe_line("2406.D.wasLaunched", @"wasLaunchedByAuthenticationServices", respondsToSel(a, "wasLaunchedByAuthenticationServices") ?
               yesno(((BOOL(*)(id, SEL))objc_msgSend)(a, sel_registerName("wasLaunchedByAuthenticationServices"))) : @"n/a");
    probe_line("2406.D.handler", @"sessionHandler before any assignment", [msg0(a, "sessionHandler") description] ?: @"nil");
    Handler *h = [[Handler alloc] init];
    NSString *exc = probe_try(^{ [a setValue:h forKey:@"sessionHandler"]; });
    probe_line("2406.D.handler", @"set a handler object", PF(@"%@ ; read back same: %@", exc, yesno(msg0(a, "sessionHandler") == h)));
    __weak Handler *wh = nil;
    @autoreleasepool { Handler *tmp = [[Handler alloc] init]; wh = tmp; [a setValue:tmp forKey:@"sessionHandler"]; }
    probe_line("2406.D.handler-weak", @"handler released by its only owner", PF(@"sessionHandler afterwards: %@ ; object alive: %@",
               msg0(a, "sessionHandler") ? @"non-nil" : @"nil", yesno(wh != nil)));
    exc = probe_try(^{ [a setValue:nil forKey:@"sessionHandler"]; });
    probe_line("2406.D.handler-nil", @"set sessionHandler to nil", PF(@"%@ ; read back: %@", exc, [msg0(a, "sessionHandler") description] ?: @"nil"));
    // Does setting a handler make the manager call it on its own? (Nothing public starts a session.)
    [a setValue:h forKey:@"sessionHandler"];
    [[NSRunLoop mainRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.3]];
    probe_line("2406.D.handler-calls", @"handler calls recorded 0.3 s after assignment", PF(@"%lu", (unsigned long)h.log.count));
    // Handler object that does not implement the protocol methods.
    exc = probe_try(^{ [a setValue:[[NSObject alloc] init] forKey:@"sessionHandler"]; });
    probe_line("2406.D.handler-wrong", @"set an object that does not implement the protocol", exc);
    [a setValue:nil forKey:@"sessionHandler"];
    // Subclass: +sharedManager on a subclass, and +new / -init / -copy of the manager.
    Class sub = objc_allocateClassPair(m, "Probe2406Sub", 0);
    if (sub) {
        objc_registerClassPair(sub);
        id sm = msg0(sub, "sharedManager");
        probe_line("2406.D.subclass", @"+sharedManager on a subclass", PF(@"same as the base instance: %@ ; class %@", yesno(sm == a), sm ? NSStringFromClass([sm class]) : @"nil"));
    }
    __block id n = nil;
    exc = probe_try(^{ n = [[m alloc] init]; });
    probe_line("2406.D.init", @"[[Manager alloc] init]", n ? PF(@"object ; same as shared: %@", yesno(n == a)) : PF(@"nil ; %@", exc));
    exc = probe_try(^{ n = [a copy]; });
    probe_line("2406.D.copy", @"-copy of the manager", n ? PF(@"object ; same as shared: %@", yesno(n == a)) : PF(@"nil ; %@", exc));
    // The manager also on a background thread.
    __block id onBg = nil;
    dispatch_semaphore_t s = dispatch_semaphore_create(0);
    dispatch_async(dispatch_get_global_queue(0, 0), ^{ onBg = msg0(m, "sharedManager"); dispatch_semaphore_signal(s); });
    dispatch_semaphore_wait(s, DISPATCH_TIME_FOREVER);
    probe_line("2406.D.thread", @"+sharedManager from a background queue", PF(@"same as main-thread instance: %@", yesno(onBg == a)));
}

int main(int argc, char **argv) {
    @autoreleasepool {
        if (argc > 1) {
            if (!strcmp(argv[1], "classes")) sectionClasses();
            else if (!strcmp(argv[1], "bare")) sectionBareRequest();
            else if (!strcmp(argv[1], "delegate")) sectionDelegate();
            else if (!strcmp(argv[1], "manager")) sectionManager();
            return 0;
        }
        probe_env("2406");
        for (NSString *s in @[@"classes", @"bare", @"delegate", @"manager"]) {
            NSTask *t = [[NSTask alloc] init];
            t.launchPath = [[NSBundle mainBundle] executablePath] ?: @"./probe";
            t.arguments = @[s];
            NSPipe *p = [NSPipe pipe];
            t.standardOutput = p;
            t.standardError = [NSFileHandle fileHandleWithNullDevice];
            [t launch];
            NSData *d = [[p fileHandleForReading] readDataToEndOfFile];
            [t waitUntilExit];
            NSString *out = [[NSString alloc] initWithData:d encoding:NSUTF8StringEncoding];
            for (NSString *line in [out componentsSeparatedByString:@"\n"])
                if ([line hasPrefix:@"PROBE "]) printf("%s\n", [line UTF8String]);
            if (t.terminationReason != NSTaskTerminationReasonExit || t.terminationStatus != 0)
                probe_line("2406.child", s, PF(@"section ended by %s %d", t.terminationReason == NSTaskTerminationReasonExit ? "exit status" : "signal", (int)t.terminationStatus));
            fflush(stdout);
        }
    }
    return 0;
}
