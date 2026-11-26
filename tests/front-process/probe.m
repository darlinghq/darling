// Copyright (C) 2026 Lubos Dolezel
// Behaviour probe for the front process (public API only).
// usage: probe NAME LOGFILE MODE SCRIPT
//   MODE   cocoa (NSApplication run loop) | carbon (RunApplicationEventLoop, no AppKit) | carbonwin (carbon,
//          with a Carbon window titled NAME, which is a window of CoreGraphics' window server)
//   SCRIPT "T:action;T:action" with T in seconds from start, actions:
//     front=NAME[,opts]  SetFrontProcess[WithOptions] to the process of the app NAME (test.vdfp.NAME)
//     frontpsn=H:L[,opts] the same with a raw serial number (kNoProcess 0:0, kSystemProcess 0:1, kCurrentProcess 0:2)
//     frontself          SetFrontProcess(current process)
//     get                log GetFrontProcess, NSWorkspace front and menu bar owner
//     activate           [NSApp activateIgnoringOtherApps:YES]
//     policy=N           [NSApp setActivationPolicy:N] / TransformProcessType for carbon
//     fg                 TransformProcessType(foreground)
//     quit
// SCRIPT may also be "@FILE": the program executes the lines that get appended to FILE (polled every 100 ms).
#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>
#include <sys/time.h>
#include <stdarg.h>
#include <dlfcn.h>

extern void RunApplicationEventLoop(void);
extern void QuitApplicationEventLoop(void);
static FILE *g_log;
static NSString *g_name;
static int g_cocoa;

static void L(const char *fmt, ...) {
    struct timeval tv; gettimeofday(&tv, NULL);
    char buf[512]; va_list ap; va_start(ap, fmt); vsnprintf(buf, sizeof buf, fmt, ap); va_end(ap);
    fprintf(g_log, "%ld.%06d %s %s\n", (long)tv.tv_sec % 100000, (int)tv.tv_usec, g_name.UTF8String, buf);
    fflush(g_log);
}

static pid_t pidOfPSN(ProcessSerialNumber p) { pid_t pid = -1; if (GetProcessPID(&p, &pid) != noErr) return -1; return pid; }

// CoreGraphics' own front process (GetFrontProcess() cannot say that no Darling process has the focus: it
// answers with the calling process then, as there is no process serial number for another client).
typedef int (*CGSGetFrontFn)(pid_t *);
typedef void (*CGSObserverFn)(pid_t previous, pid_t current, void *info);
static pid_t cgsFront(int *err) {
    static CGSGetFrontFn fn;
    pid_t p = 0;
    if (!fn) fn = (CGSGetFrontFn)dlsym(RTLD_DEFAULT, "_CGSGetFrontProcess");
    *err = fn ? fn(&p) : -1;
    return p;
}
static void frontObserved(pid_t previous, pid_t current, void *info) { L("FRONTOBS previous=%d current=%d", previous, current); }
static void watchCGSFront(void) {
    void (*add)(CGSObserverFn, void *) = (void (*)(CGSObserverFn, void *))dlsym(RTLD_DEFAULT, "_CGSAddFrontProcessObserver");
    if (add) add(frontObserved, NULL);
}

static void state(const char *tag) {
    ProcessSerialNumber f; OSErr e = GetFrontProcess(&f);
    pid_t fp = e == noErr ? pidOfPSN(f) : -2;
    NSRunningApplication *fa = nil, *mo = nil;
    if (g_cocoa) { fa = [[NSWorkspace sharedWorkspace] frontmostApplication]; mo = [[NSWorkspace sharedWorkspace] menuBarOwningApplication]; }
    int cgsErr; pid_t cgs = cgsFront(&cgsErr);
    L("STATE %s getfront=%d(err %d) ws.front=%d ws.owner=%d self=%d cgsfront=%d(err %d)", tag, fp, (int)e,
      fa ? fa.processIdentifier : -1, mo ? mo.processIdentifier : -1, getpid(), cgsErr == 0 ? cgs : 0, cgsErr);
}

static OSStatus carbonHandler(EventHandlerCallRef next, EventRef ev, void *ud) {
    UInt32 k = GetEventKind(ev);
    const char *n = k == kEventAppActivated ? "AppActivated" : k == kEventAppDeactivated ? "AppDeactivated" :
                    k == kEventAppFrontSwitched ? "AppFrontSwitched" : "AppOther";
    ProcessSerialNumber p = {0, 0};
    OSStatus e = GetEventParameter(ev, kEventParamProcessID, typeProcessSerialNumber, NULL, sizeof p, NULL, &p);
    L("CARBON %s class=%08x param-err=%d psn-pid=%d", n, (unsigned)GetEventClass(ev), (int)e, e == noErr ? pidOfPSN(p) : -1);
    state(n);
    return eventNotHandledErr;
}

static NSRunningApplication *appNamed(NSString *n) {
    NSArray *a = [NSRunningApplication runningApplicationsWithBundleIdentifier:[@"test.vdfp." stringByAppendingString:n]];
    return a.count ? a[0] : nil;
}

static void doFront(ProcessSerialNumber psn, int hasOpt, UInt32 opt, const char *what) {
    OSErr e = hasOpt ? SetFrontProcessWithOptions(&psn, opt) : SetFrontProcess(&psn);
    L("RESULT %s (psn %u:%u opts %d) err=%d", what, (unsigned)psn.highLongOfPSN, (unsigned)psn.lowLongOfPSN, hasOpt ? (int)opt : -1, (int)e);
    state("right-after-set");
}

static void act(NSString *a) {
    L("ACTION %s", a.UTF8String);
    NSString *arg = nil; NSRange r = [a rangeOfString:@"="];
    NSString *cmd = a;
    if (r.location != NSNotFound) { cmd = [a substringToIndex:r.location]; arg = [a substringFromIndex:r.location+1]; }
    if ([cmd isEqual:@"front"]) {
        NSArray *p = [arg componentsSeparatedByString:@","];
        NSRunningApplication *t = appNamed(p[0]);
        if (!t) { L("RESULT front: no app %s", [p[0] UTF8String]); return; }
        ProcessSerialNumber psn; OSStatus e = GetProcessForPID(t.processIdentifier, &psn);
        if (e) { L("RESULT GetProcessForPID err=%d", (int)e); return; }
        doFront(psn, p.count > 1, p.count > 1 ? (UInt32)[p[1] intValue] : 0, [p[0] UTF8String]);
    } else if ([cmd isEqual:@"frontpsn"]) {
        NSArray *p = [arg componentsSeparatedByString:@","];
        NSArray *hl = [p[0] componentsSeparatedByString:@":"];
        ProcessSerialNumber psn = { (UInt32)[hl[0] longLongValue], (UInt32)[hl[1] longLongValue] };
        doFront(psn, p.count > 1, p.count > 1 ? (UInt32)[p[1] intValue] : 0, "raw");
    } else if ([cmd isEqual:@"frontself"]) {
        ProcessSerialNumber psn; GetCurrentProcess(&psn); doFront(psn, 0, 0, "self");
    } else if ([cmd isEqual:@"get"]) {
        state("get");
    } else if ([cmd isEqual:@"activate"]) {
        if (g_cocoa) [NSApp activateIgnoringOtherApps:YES];
    } else if ([cmd isEqual:@"policy"]) {
        if (g_cocoa) L("RESULT setActivationPolicy=%d", (int)[NSApp setActivationPolicy:(NSApplicationActivationPolicy)arg.intValue]);
    } else if ([cmd isEqual:@"fg"]) {
        ProcessSerialNumber psn; GetCurrentProcess(&psn);
        L("RESULT TransformProcessType fg err=%d", (int)TransformProcessType(&psn, kProcessTransformToForegroundApplication));
    } else if ([cmd isEqual:@"quit"]) {
        state("quit"); if (g_cocoa) [NSApp terminate:nil]; else QuitApplicationEventLoop();
    }
}

@interface Obs : NSObject @end
@implementation Obs
- (void)n:(NSNotification *)n {
    NSRunningApplication *a = n.userInfo[NSWorkspaceApplicationKey];
    L("NOTE %s %s", n.name.UTF8String, a ? [NSString stringWithFormat:@"app-pid=%d", a.processIdentifier].UTF8String : "");
    state(n.name.UTF8String);
}
@end

int main(int argc, char **argv) {
    @autoreleasepool {
        g_name = @(argv[1]); g_log = fopen(argv[2], "a"); g_cocoa = strcmp(argv[3], "cocoa") == 0;
        NSString *script = argc > 4 ? @(argv[4]) : @"";
        if (g_cocoa) {
            [NSApplication sharedApplication];
            L("START cocoa policy=%d", (int)[NSApp activationPolicy]);
            static Obs *o; o = [Obs new];
            for (NSString *nm in @[NSApplicationDidBecomeActiveNotification, NSApplicationDidResignActiveNotification,
                                   NSApplicationWillBecomeActiveNotification, NSApplicationWillResignActiveNotification])
                [[NSNotificationCenter defaultCenter] addObserver:o selector:@selector(n:) name:nm object:nil];
            for (NSString *nm in @[NSWorkspaceDidActivateApplicationNotification, NSWorkspaceDidDeactivateApplicationNotification])
                [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:o selector:@selector(n:) name:nm object:nil];
            static NSWindow *w; w = [[NSWindow alloc] initWithContentRect:NSMakeRect(100,100,200,100) styleMask:NSWindowStyleMaskTitled backing:NSBackingStoreBuffered defer:NO];
            w.title = g_name; [w makeKeyAndOrderFront:nil];
        } else if (strcmp(argv[3], "carbonwin") == 0) {
            L("START carbonwin");
            Rect bounds = { 150, 400, 250, 600 };
            WindowRef win = NULL;
#ifdef __LP64__
            // Every 64-bit Carbon window is a compositing window
            WindowAttributes attrs = kWindowStandardDocumentAttributes | kWindowCompositingAttribute;
#else
            WindowAttributes attrs = kWindowStandardDocumentAttributes;
#endif
            OSStatus we = CreateNewWindow(kDocumentWindowClass, attrs, &bounds, &win);
            if (we == noErr) {
                CFStringRef title = CFStringCreateWithCString(NULL, argv[1], kCFStringEncodingUTF8);
                SetWindowTitleWithCFString(win, title);
                CFRelease(title);
                ShowWindow(win);
            }
            L("WINDOW carbon window err=%d", (int)we);
        } else L("START carbon");
        static const EventTypeSpec specs[] = { {kEventClassApplication, kEventAppActivated}, {kEventClassApplication, kEventAppDeactivated},
                                               {kEventClassApplication, kEventAppFrontSwitched} };
        OSStatus ie = InstallApplicationEventHandler(NewEventHandlerUPP(carbonHandler), 3, specs, NULL, NULL);
        L("INSTALL carbon handlers err=%d", (int)ie);
        if ([script hasPrefix:@"@"]) {
            static NSString *cmdPath; static NSUInteger consumed;
            cmdPath = [script substringFromIndex:1];
            CFRunLoopTimerRef pt = CFRunLoopTimerCreateWithHandler(NULL, CFAbsoluteTimeGetCurrent() + 0.1, 0.1, 0, 0, ^(CFRunLoopTimerRef x){
                NSString *all = [NSString stringWithContentsOfFile:cmdPath encoding:NSUTF8StringEncoding error:NULL];
                if (all.length <= consumed) return;
                NSString *fresh = [all substringFromIndex:consumed];
                NSRange nl = [fresh rangeOfString:@"\n" options:NSBackwardsSearch];
                if (nl.location == NSNotFound) return;
                consumed += nl.location + 1;
                for (NSString *line in [[fresh substringToIndex:nl.location] componentsSeparatedByString:@"\n"])
                    if (line.length) act(line);
            });
            CFRunLoopAddTimer(CFRunLoopGetMain(), pt, kCFRunLoopCommonModes);
            script = @"";
        }
        for (NSString *item in [script componentsSeparatedByString:@";"]) {
            if (!item.length) continue;
            NSRange c = [item rangeOfString:@":"];
            double t = [[item substringToIndex:c.location] doubleValue];
            NSString *a = [item substringFromIndex:c.location+1];
            CFRunLoopTimerRef tm = CFRunLoopTimerCreateWithHandler(NULL, CFAbsoluteTimeGetCurrent() + t, 0, 0, 0, ^(CFRunLoopTimerRef x){ act(a); });
            CFRunLoopAddTimer(CFRunLoopGetMain(), tm, kCFRunLoopCommonModes);
        }
        if (getenv("FRONT_PROBE_OBSERVE")) watchCGSFront();
        state("start");
        if (g_cocoa) [NSApp run]; else RunApplicationEventLoop();
        L("END");
    }
    return 0;
}
