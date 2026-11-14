// CFLAGS: -fobjc-arc -framework AppKit -framework WebKit
// Copyright (C) 2026 Lubos Dolezel
//
// WKWebView and its configuration classes, from the public WebKit API:
//   - WKWebView is an NSView with the documented initial state (empty title, no URL, not
//     loading, no history), and -configuration returns a copy that shares the process pool,
//     preferences, content controller and data store;
//   - with no rendering engine behind it, every load answers asynchronously on the main queue:
//     didStartProvisionalNavigation, then didFailProvisionalNavigation with
//     NSURLErrorDomain / NSURLErrorCannotConnectToHost, never commit or finish; the observable
//     properties (isLoading, URL, estimatedProgress, title, canGoBack) change coherently and
//     announce each change through key-value observing;
//   - evaluateJavaScript:completionHandler: completes asynchronously on the main queue with a
//     WKErrorDomain error;
//   - stopLoading, reload, goBack and goForward;
//   - the navigation delegate is not retained;
//   - WKUserScript, WKUserContentController, WKPreferences, WKWebsiteDataStore and
//     WKWebViewConfiguration behave as documented, including the exception for a second script
//     message handler with the same name;
//   - a web view drives a replacement backend through the private backend seam: commit, finish,
//     title, progress and history changes, and script messages reaching the registered handler.
//
// Creates views but shows no window; if creating an NSView needs a display in the guest, run it
// under the disposable Xvfb used for the AppKit tests. Prints PASS or FAIL lines, then ALL PASSED
// or FAILED n.
#import <Foundation/Foundation.h>
#import <WebKit/WebKit.h>
#import <AppKit/AppKit.h>
#include <pthread.h>
#include <stdio.h>

static int failures;

#define CHECK(cond, ...) do { \
    if (cond) { printf("PASS: "); } else { printf("FAIL: "); failures++; } \
    printf(__VA_ARGS__); printf("\n"); \
} while (0)

// The private backend seam. Not in the installed headers, so declared here as the framework does.
@protocol WKWebViewBackendClient <NSObject>
- (void)backendDidStartProvisionalNavigation:(WKNavigation *)navigation;
- (void)backendDidFailProvisionalNavigation:(WKNavigation *)navigation error:(NSError *)error;
- (void)backendDidCommitNavigation:(WKNavigation *)navigation URL:(NSURL *)URL;
- (void)backendDidFinishNavigation:(WKNavigation *)navigation;
- (void)backendDidFailNavigation:(WKNavigation *)navigation error:(NSError *)error;
- (void)backendDidChangeTitle:(NSString *)title;
- (void)backendDidChangeEstimatedProgress:(double)progress;
- (void)backendDidChangeCanGoBack:(BOOL)canGoBack canGoForward:(BOOL)canGoForward;
- (void)backendDidReceiveScriptMessageBody:(id)body name:(NSString *)name;
@end

@interface WKWebView (BackendSelection)
+ (void)_setBackendClass:(Class)backendClass;
@end

@interface WKNavigation (Private)
@property (nonatomic, readonly, copy) NSURLRequest *request;
@end

static void spin(BOOL (^done)(void), NSTimeInterval timeout) {
    NSDate *end = [NSDate dateWithTimeIntervalSinceNow:timeout];
    while (!(done && done()) && [end timeIntervalSinceNow] > 0) {
        [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.01]];
    }
}

static void spinFor(NSTimeInterval seconds) {
    spin(nil, seconds);
}

@interface Recorder : NSObject <WKNavigationDelegate>
@property (nonatomic, strong) NSMutableArray<NSString *> *events;
@property (nonatomic, strong) NSMutableArray<NSError *> *errors;
@property (nonatomic, strong) NSMutableArray<WKNavigation *> *navigations;
// isLoading as the web view reported it inside each callback.
@property (nonatomic, strong) NSMutableArray<NSNumber *> *loadingStates;
@property (nonatomic) BOOL allOnMain;
@end

@implementation Recorder
- (instancetype)init {
    if ((self = [super init])) {
        _events = [NSMutableArray new];
        _errors = [NSMutableArray new];
        _navigations = [NSMutableArray new];
        _loadingStates = [NSMutableArray new];
        _allOnMain = YES;
    }
    return self;
}
- (void)note:(NSString *)event view:(WKWebView *)view navigation:(WKNavigation *)navigation error:(NSError *)error {
    [_loadingStates addObject:@(view.isLoading)];
    if (!pthread_main_np()) {
        _allOnMain = NO;
    }
    [_events addObject:event];
    if (navigation) {
        [_navigations addObject:navigation];
    }
    if (error) {
        [_errors addObject:error];
    }
}
- (void)webView:(WKWebView *)w didStartProvisionalNavigation:(WKNavigation *)n { [self note:@"start" view:w navigation:n error:nil]; }
- (void)webView:(WKWebView *)w didFailProvisionalNavigation:(WKNavigation *)n withError:(NSError *)e { [self note:@"failProvisional" view:w navigation:n error:e]; }
- (void)webView:(WKWebView *)w didCommitNavigation:(WKNavigation *)n { [self note:@"commit" view:w navigation:n error:nil]; }
- (void)webView:(WKWebView *)w didFinishNavigation:(WKNavigation *)n { [self note:@"finish" view:w navigation:n error:nil]; }
- (void)webView:(WKWebView *)w didFailNavigation:(WKNavigation *)n withError:(NSError *)e { [self note:@"fail" view:w navigation:n error:e]; }
@end

@interface KVORecorder : NSObject
@property (nonatomic, strong) NSMutableArray<NSString *> *changes;
@end

@implementation KVORecorder
- (instancetype)init {
    if ((self = [super init])) {
        _changes = [NSMutableArray new];
    }
    return self;
}
- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context {
    id value = change[NSKeyValueChangeNewKey];
    [_changes addObject:[NSString stringWithFormat:@"%@=%@", keyPath,
        value == [NSNull null] ? @"nil" : [value isKindOfClass:[NSNumber class]] && ![keyPath isEqual:@"loading"] ? [NSString stringWithFormat:@"%g", [value doubleValue]] : value]];
}
@end

static void testInitialState(void) {
    WKWebViewConfiguration *config = [WKWebViewConfiguration new];
    WKWebView *view = [[WKWebView alloc] initWithFrame:NSMakeRect(0, 0, 300, 200) configuration:config];
    CHECK(view != nil, "initWithFrame:configuration: returns a view");
    CHECK([view isKindOfClass:[NSView class]], "WKWebView is an NSView");
    CHECK(NSEqualRects(view.frame, NSMakeRect(0, 0, 300, 200)), "the frame is kept");
    CHECK(view.isFlipped, "the view is flipped");
    CHECK(view.title != nil && view.title.length == 0, "the initial title is the empty string");
    CHECK(view.URL == nil, "there is no URL");
    CHECK(!view.isLoading, "not loading");
    CHECK(view.estimatedProgress == 0, "progress is 0");
    CHECK(!view.canGoBack && !view.canGoForward, "no history");
    CHECK(!view.hasOnlySecureContent, "no secure content");
    CHECK(view.backForwardList != nil, "there is a back-forward list");
    CHECK(view.navigationDelegate == nil && view.UIDelegate == nil, "no delegates");
    CHECK([view goBack] == nil && [view goForward] == nil, "goBack and goForward return nil without history");
    CHECK([view reload] == nil && [view reloadFromOrigin] == nil, "reload returns nil before any load");

    WKWebView *plain = [[WKWebView alloc] initWithFrame:NSMakeRect(0, 0, 10, 10)];
    CHECK(plain.configuration != nil, "initWithFrame: creates a default configuration");

    WKWebViewConfiguration *copy1 = view.configuration;
    WKWebViewConfiguration *copy2 = view.configuration;
    CHECK(copy1 != config && copy1 != copy2, "-configuration returns a copy each time");
    CHECK(copy1.userContentController == config.userContentController, "a copy shares the user content controller");
    CHECK(copy1.preferences == config.preferences, "a copy shares the preferences");
    CHECK(copy1.processPool == config.processPool, "a copy shares the process pool");
    CHECK(copy1.websiteDataStore == config.websiteDataStore, "a copy shares the website data store");

    config.applicationNameForUserAgent = @"Example";
    config.suppressesIncrementalRendering = YES;
    WKWebViewConfiguration *copy3 = [config copy];
    CHECK([copy3.applicationNameForUserAgent isEqual:@"Example"] && copy3.suppressesIncrementalRendering, "a copy keeps the plain properties");
    CHECK([view.configuration applicationNameForUserAgent] == nil, "the view's copy was taken at creation");
}

static void testFailingLoad(void) {
    WKWebView *view = [[WKWebView alloc] initWithFrame:NSMakeRect(0, 0, 300, 200) configuration:[WKWebViewConfiguration new]];
    Recorder *delegate = [Recorder new];
    view.navigationDelegate = delegate;
    KVORecorder *kvo = [KVORecorder new];
    for (NSString *key in @[@"loading", @"URL", @"estimatedProgress", @"title", @"canGoBack", @"canGoForward"]) {
        [view addObserver:kvo forKeyPath:key options:NSKeyValueObservingOptionNew context:NULL];
    }

    NSURL *URL = [NSURL URLWithString:@"http://example.invalid/page"];
    WKNavigation *navigation = [view loadRequest:[NSURLRequest requestWithURL:URL]];
    CHECK(navigation != nil, "loadRequest: returns a navigation");
    CHECK(view.isLoading, "loading right after loadRequest:");
    CHECK([view.URL isEqual:URL], "URL is the requested URL while loading");
    CHECK(view.estimatedProgress > 0, "progress has started");
    CHECK(delegate.events.count == 0, "no delegate callback is made synchronously");

    spin(^BOOL { return delegate.events.count >= 2; }, 5);
    CHECK([delegate.events isEqualToArray:(@[@"start", @"failProvisional"])], "delegate saw start then failProvisional, got %s", [[delegate.events description] UTF8String]);
    CHECK(delegate.allOnMain, "callbacks arrive on the main thread");
    CHECK(delegate.navigations.count == 2 && delegate.navigations[0] == navigation && delegate.navigations[1] == navigation,
          "callbacks carry the navigation loadRequest: returned");
    NSError *error = delegate.errors.firstObject;
    CHECK([error.domain isEqual:NSURLErrorDomain] && error.code == NSURLErrorCannotConnectToHost,
          "failure is NSURLErrorDomain / CannotConnectToHost, got %s %ld", [error.domain UTF8String], (long)error.code);
    CHECK([error.userInfo[NSURLErrorFailingURLErrorKey] isEqual:URL], "the error carries the failing URL");
    CHECK([error.userInfo[NSURLErrorFailingURLStringErrorKey] isEqual:URL.absoluteString], "the error carries the failing URL string");
    CHECK(error.localizedDescription.length > 0, "the error has a description");
    CHECK(!view.isLoading, "not loading after the failure");
    CHECK(view.URL == nil, "URL is back to the committed page's (none)");
    CHECK(view.estimatedProgress == 0, "progress is back to 0");
    CHECK(view.title.length == 0 && !view.canGoBack && !view.canGoForward, "title and history unchanged");
    spinFor(0.2);
    CHECK(delegate.events.count == 2, "no commit, finish or further failure");

    NSArray *expected = @[@"loading=1", @"URL=http://example.invalid/page", @"estimatedProgress=0.1", @"estimatedProgress=0",
                          @"loading=0", @"URL=nil"];
    NSMutableArray *got = [kvo.changes mutableCopy];
    CHECK([[NSSet setWithArray:got] isEqualToSet:[NSSet setWithArray:expected]] && got.count == expected.count,
          "KVO announced loading, URL and progress once each, got %s", [[got description] UTF8String]);
    NSUInteger loadingOn = [got indexOfObject:@"loading=1"], loadingOff = [got indexOfObject:@"loading=0"];
    CHECK(loadingOn != NSNotFound && loadingOff != NSNotFound && loadingOn < loadingOff, "loading turned on before it turned off");
    BOOL unchangedQuiet = YES;
    for (NSString *change in got) {
        if ([change hasPrefix:@"title"] || [change hasPrefix:@"canGo"]) {
            unchangedQuiet = NO;
        }
    }
    CHECK(unchangedQuiet, "no notification for the title or history, which did not change");

    for (NSString *key in @[@"loading", @"URL", @"estimatedProgress", @"title", @"canGoBack", @"canGoForward"]) {
        [view removeObserver:kvo forKeyPath:key];
    }

    // reload repeats the failed load
    [delegate.events removeAllObjects];
    WKNavigation *again = [view reload];
    CHECK(again != nil && again != navigation, "reload after a load returns a new navigation");
    CHECK([view.URL isEqual:URL], "reload loads the same URL");
    spin(^BOOL { return delegate.events.count >= 2; }, 5);
    CHECK([delegate.events isEqualToArray:(@[@"start", @"failProvisional"])], "reload fails the same way");
    CHECK(!view.isLoading && view.URL == nil, "state is idle again after the reload failed");
}

static void testLoadKinds(void) {
    WKWebView *view = [[WKWebView alloc] initWithFrame:NSMakeRect(0, 0, 300, 200) configuration:[WKWebViewConfiguration new]];
    Recorder *delegate = [Recorder new];
    view.navigationDelegate = delegate;

    CHECK([view loadHTMLString:@"<p>hi</p>" baseURL:nil] != nil, "loadHTMLString:baseURL: returns a navigation");
    CHECK([view.URL.absoluteString isEqual:@"about:blank"], "without a base URL the page URL is about:blank");
    spin(^BOOL { return delegate.events.count >= 2; }, 5);
    NSError *error = delegate.errors.lastObject;
    CHECK([error.domain isEqual:NSURLErrorDomain] && error.code == NSURLErrorCannotConnectToHost, "an HTML string load fails like any other load");
    CHECK([error.userInfo[NSURLErrorFailingURLStringErrorKey] isEqual:@"about:blank"], "the failing URL is about:blank");

    NSURL *base = [NSURL URLWithString:@"https://example.invalid/base/"];
    [delegate.events removeAllObjects];
    CHECK([view loadHTMLString:@"<p>hi</p>" baseURL:base] != nil, "loadHTMLString: with a base URL returns a navigation");
    CHECK([view.URL isEqual:base], "the page URL is the base URL");
    spin(^BOOL { return delegate.events.count >= 2; }, 5);
    CHECK(!view.isLoading, "not loading afterwards");

    [delegate.events removeAllObjects];
    NSData *data = [@"<p>x</p>" dataUsingEncoding:NSUTF8StringEncoding];
    CHECK([view loadData:data MIMEType:@"text/html" characterEncodingName:@"UTF-8" baseURL:base] != nil, "loadData:... returns a navigation");
    NSURL *file = [NSURL fileURLWithPath:@"/tmp/page.html"];
    CHECK([view loadFileURL:file allowingReadAccessToURL:[NSURL fileURLWithPath:@"/tmp"]] != nil, "loadFileURL:allowingReadAccessToURL: returns a navigation");
    CHECK([view.URL isEqual:file], "the page URL is the file URL");
    spin(^BOOL { return delegate.events.count >= 4; }, 5);
    CHECK(delegate.events.count == 4 && !view.isLoading, "both fail, in order, and loading ends after the last");
    CHECK(delegate.navigations.count == 8 && delegate.navigations[4] != delegate.navigations[6], "each load has its own navigation");
}

static void testOverlappingAndStop(void) {
    WKWebView *view = [[WKWebView alloc] initWithFrame:NSMakeRect(0, 0, 300, 200) configuration:[WKWebViewConfiguration new]];
    Recorder *delegate = [Recorder new];
    view.navigationDelegate = delegate;

    [view loadRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:@"http://example.invalid/1"]]];
    [view loadRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:@"http://example.invalid/2"]]];
    spin(^BOOL { return delegate.events.count >= 4; }, 5);
    CHECK(delegate.events.count == 4 && !view.isLoading, "both navigations failed and loading ended");
    CHECK(delegate.loadingStates.count == 4 && [delegate.loadingStates[1] boolValue] && ![delegate.loadingStates[3] boolValue],
          "loading was still YES when the first failed and NO when the second did");

    [delegate.events removeAllObjects];
    [view loadRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:@"http://example.invalid/3"]]];
    CHECK(view.isLoading, "loading before stopLoading");
    [view stopLoading];
    CHECK(!view.isLoading && view.URL == nil && view.estimatedProgress == 0, "stopLoading ends the load at once");
    spinFor(0.3);
    CHECK(delegate.events.count == 0, "a stopped navigation makes no delegate call");
    [view stopLoading];
    CHECK(!view.isLoading, "stopLoading when idle is harmless");
}

static void testJavaScript(void) {
    WKWebView *view = [[WKWebView alloc] initWithFrame:NSMakeRect(0, 0, 300, 200) configuration:[WKWebViewConfiguration new]];
    __block BOOL called = NO, onMain = NO;
    __block id result = @"unset";
    __block NSError *error = nil;
    [view evaluateJavaScript:@"1 + 1" completionHandler:^(id r, NSError *e) {
        called = YES;
        onMain = pthread_main_np() != 0;
        result = r;
        error = e;
    }];
    CHECK(!called, "the completion handler is not called synchronously");
    spin(^BOOL { return called; }, 5);
    CHECK(called && onMain, "the completion handler is called on the main thread");
    CHECK(result == nil, "there is no result");
    CHECK([error.domain isEqual:WKErrorDomain] && error.code == WKErrorWebContentProcessTerminated,
          "the error is WKErrorDomain / WKErrorWebContentProcessTerminated, got %s %ld", [error.domain UTF8String], (long)error.code);
    BOOL raised = NO;
    @try {
        [view evaluateJavaScript:@"1" completionHandler:nil];
        spinFor(0.05);
    } @catch (NSException *e) {
        raised = YES;
    }
    CHECK(!raised, "a nil completion handler is accepted");
    CHECK([WKErrorDomain isEqual:@"WKErrorDomain"] && WKErrorUnknown == 1 && WKErrorWebContentProcessTerminated == 2
          && WKErrorWebViewInvalidated == 3 && WKErrorJavaScriptExceptionOccurred == 4 && WKErrorJavaScriptResultTypeIsUnsupported == 5,
          "WKErrorDomain and the first error codes");
}

static void testDelegateNotRetained(void) {
    WKWebView *view = [[WKWebView alloc] initWithFrame:NSMakeRect(0, 0, 300, 200) configuration:[WKWebViewConfiguration new]];
    @autoreleasepool {
        Recorder *delegate = [Recorder new];
        view.navigationDelegate = delegate;
        CHECK(view.navigationDelegate == delegate, "the navigation delegate is stored");
    }
    CHECK(view.navigationDelegate == nil, "the navigation delegate is not retained");
    // A view released with a load pending must not call anything afterwards.
    @autoreleasepool {
        WKWebView *temp = [[WKWebView alloc] initWithFrame:NSMakeRect(0, 0, 10, 10) configuration:[WKWebViewConfiguration new]];
        Recorder *delegate = [Recorder new];
        temp.navigationDelegate = delegate;
        [temp loadRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:@"http://example.invalid/"]]];
        __block BOOL called = NO;
        [temp evaluateJavaScript:@"1" completionHandler:^(id r, NSError *e) { called = YES; }];
        temp = nil;
        spin(^BOOL { return called; }, 5);
        CHECK(called, "a pending completion handler still runs after the view is released");
        CHECK(delegate.events.count == 0, "a released view makes no delegate calls");
    }
}

static void testUserScriptsAndController(void) {
    WKUserScript *start = [[WKUserScript alloc] initWithSource:@"a()" injectionTime:WKUserScriptInjectionTimeAtDocumentStart forMainFrameOnly:YES];
    WKUserScript *end = [[WKUserScript alloc] initWithSource:@"b()" injectionTime:WKUserScriptInjectionTimeAtDocumentEnd forMainFrameOnly:NO];
    CHECK([start.source isEqual:@"a()"] && start.injectionTime == WKUserScriptInjectionTimeAtDocumentStart && start.isForMainFrameOnly,
          "WKUserScript keeps its source, time and frame scope");
    CHECK(end.injectionTime == WKUserScriptInjectionTimeAtDocumentEnd && !end.isForMainFrameOnly, "the second script's properties");
    CHECK([start copy] == start, "copying a script gives the script");
    CHECK(WKUserScriptInjectionTimeAtDocumentStart == 0 && WKUserScriptInjectionTimeAtDocumentEnd == 1, "injection time values");

    WKUserContentController *controller = [WKUserContentController new];
    CHECK(controller.userScripts.count == 0, "a new controller has no scripts");
    [controller addUserScript:start];
    [controller addUserScript:end];
    CHECK(controller.userScripts.count == 2 && controller.userScripts[0] == start && controller.userScripts[1] == end, "scripts are kept in order");
    [controller removeAllUserScripts];
    CHECK(controller.userScripts.count == 0, "removeAllUserScripts empties the controller");

    id<WKScriptMessageHandler> handler = (id<WKScriptMessageHandler>)[NSObject new];
    [controller addScriptMessageHandler:handler name:@"one"];
    BOOL raised = NO;
    @try {
        [controller addScriptMessageHandler:handler name:@"one"];
    } @catch (NSException *e) {
        raised = [e.name isEqual:NSInvalidArgumentException];
    }
    CHECK(raised, "adding a second handler with the same name raises NSInvalidArgumentException");
    [controller addScriptMessageHandler:handler name:@"two"];
    raised = NO;
    @try {
        [controller removeScriptMessageHandlerForName:@"nonexistent"];
    } @catch (NSException *e) {
        raised = YES;
    }
    CHECK(!raised, "removing an unknown handler is harmless");
    [controller removeScriptMessageHandlerForName:@"one"];
    raised = NO;
    @try {
        [controller addScriptMessageHandler:handler name:@"one"];
    } @catch (NSException *e) {
        raised = YES;
    }
    CHECK(!raised, "a name can be added again after its handler was removed");
    [controller removeAllScriptMessageHandlers];
    raised = NO;
    @try {
        [controller addScriptMessageHandler:handler name:@"two"];
    } @catch (NSException *e) {
        raised = YES;
    }
    CHECK(!raised, "removeAllScriptMessageHandlers frees every name");
}

static void testPreferencesAndConfiguration(void) {
    WKPreferences *prefs = [WKPreferences new];
    CHECK(prefs.minimumFontSize == 0, "minimumFontSize defaults to 0");
    CHECK(prefs.javaScriptEnabled, "JavaScript is enabled by default");
    CHECK(prefs.javaScriptCanOpenWindowsAutomatically, "JavaScript may open windows by default on macOS");
    CHECK(!prefs.tabFocusesLinks, "tabFocusesLinks defaults to NO");
    prefs.minimumFontSize = 9;
    prefs.javaScriptEnabled = NO;
    prefs.tabFocusesLinks = YES;
    CHECK(prefs.minimumFontSize == 9 && !prefs.javaScriptEnabled && prefs.tabFocusesLinks, "preferences keep what is set");

    WKWebViewConfiguration *config = [WKWebViewConfiguration new];
    CHECK(config.processPool && config.preferences && config.userContentController && config.websiteDataStore, "a configuration has all its parts");
    CHECK(config.websiteDataStore == [WKWebsiteDataStore defaultDataStore], "the default data store is used");
    CHECK(config.applicationNameForUserAgent == nil && !config.suppressesIncrementalRendering, "plain property defaults");
    CHECK(config.mediaTypesRequiringUserActionForPlayback == WKAudiovisualMediaTypeNone, "no media type requires a user action by default");
    config.preferences = prefs;
    CHECK(config.preferences == prefs, "the preferences can be replaced");

    id<WKURLSchemeHandler> handler = (id<WKURLSchemeHandler>)[NSObject new];
    CHECK([config urlSchemeHandlerForURLScheme:@"example"] == nil, "no handler is registered at first");
    [config setURLSchemeHandler:handler forURLScheme:@"example"];
    CHECK([config urlSchemeHandlerForURLScheme:@"example"] == handler, "a scheme handler is registered");
    CHECK([[config copy] urlSchemeHandlerForURLScheme:@"example"] == handler, "a copy keeps the scheme handlers");
    [config setURLSchemeHandler:nil forURLScheme:@"example"];
    CHECK([config urlSchemeHandlerForURLScheme:@"example"] == nil, "a handler is removed with nil");
}

static void testDataStore(void) {
    WKWebsiteDataStore *def = [WKWebsiteDataStore defaultDataStore];
    WKWebsiteDataStore *other = [WKWebsiteDataStore nonPersistentDataStore];
    CHECK(def == [WKWebsiteDataStore defaultDataStore], "the default data store is a singleton");
    CHECK(def.isPersistent && !other.isPersistent, "the default store is persistent, the other is not");
    CHECK(other != def && other != [WKWebsiteDataStore nonPersistentDataStore], "each non-persistent store is new");
    NSSet *types = [WKWebsiteDataStore allWebsiteDataTypes];
    CHECK([types containsObject:WKWebsiteDataTypeCookies] && [types containsObject:WKWebsiteDataTypeLocalStorage]
          && [types containsObject:WKWebsiteDataTypeDiskCache] && [types containsObject:WKWebsiteDataTypeIndexedDBDatabases],
          "all data types include cookies, local storage, disk cache and IndexedDB");
    CHECK([WKWebsiteDataTypeCookies isEqual:@"WKWebsiteDataTypeCookies"], "type constants equal their names");

    __block BOOL fetched = NO, removed = NO, removedSince = NO, onMain = YES;
    __block NSArray *records = nil;
    [def fetchDataRecordsOfTypes:types completionHandler:^(NSArray *r) { fetched = YES; records = r; onMain = onMain && pthread_main_np(); }];
    [other removeDataOfTypes:types modifiedSince:[NSDate distantPast] completionHandler:^{ removedSince = YES; onMain = onMain && pthread_main_np(); }];
    [other removeDataOfTypes:types forDataRecords:@[] completionHandler:^{ removed = YES; onMain = onMain && pthread_main_np(); }];
    CHECK(!fetched && !removed && !removedSince, "completion handlers are not called synchronously");
    spin(^BOOL { return fetched && removed && removedSince; }, 5);
    CHECK(fetched && removed && removedSince && onMain, "completion handlers run on the main thread");
    CHECK(records != nil && records.count == 0, "a store without an engine has no records");
}

// A backend that stands in for a renderer: it commits and finishes loads, and reports a title,
// progress and history, by driving the client the way an engine would.
static __weak id gBackendClient;
static NSMutableArray<NSString *> *gBackendCalls;

@interface FakeBackend : NSObject
@property (nonatomic, weak) id<WKWebViewBackendClient> client;
@property (nonatomic, strong) NSMutableArray<WKNavigation *> *history;
@end

@implementation FakeBackend
- (instancetype)initWithClient:(id<WKWebViewBackendClient>)client configuration:(WKWebViewConfiguration *)configuration {
    if ((self = [super init])) {
        _client = client;
        _history = [NSMutableArray new];
        gBackendClient = client;
        [gBackendCalls addObject:@"init"];
    }
    return self;
}
- (void)startNavigation:(WKNavigation *)navigation {
    [gBackendCalls addObject:@"start"];
    dispatch_async(dispatch_get_main_queue(), ^{
        id<WKWebViewBackendClient> client = self.client;
        [client backendDidStartProvisionalNavigation:navigation];
        [client backendDidCommitNavigation:navigation URL:navigation.request.URL];
        [client backendDidChangeTitle:@"Example Page"];
        [client backendDidChangeEstimatedProgress:0.5];
        [self.history addObject:navigation];
        [client backendDidChangeCanGoBack:self.history.count > 1 canGoForward:NO];
        [client backendDidFinishNavigation:navigation];
    });
}
- (void)stopLoading {}
- (WKNavigation *)goBack { return nil; }
- (WKNavigation *)goForward { return nil; }
- (WKNavigation *)reload { return nil; }
- (void)evaluateJavaScript:(NSString *)script completionHandler:(void (^)(id, NSError *))completionHandler {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (completionHandler) {
            completionHandler(@42, nil);
        }
    });
}
- (void)invalidate { [gBackendCalls addObject:@"invalidate"]; }
@end

@interface MessageHandler : NSObject <WKScriptMessageHandler>
@property (nonatomic, strong) WKScriptMessage *message;
@end

@implementation MessageHandler
- (void)userContentController:(WKUserContentController *)c didReceiveScriptMessage:(WKScriptMessage *)m { self.message = m; }
@end

static void testBackendSeam(void) {
    gBackendCalls = [NSMutableArray new];
    [WKWebView _setBackendClass:[FakeBackend class]];
    NSURL *URL = [NSURL URLWithString:@"https://example.invalid/ok"];
    // The view's last reference must go when the pool ends: temporaries autoreleased by the
    // checks below (weak property loads) die with this pool, not the caller's.
    @autoreleasepool {
    WKWebViewConfiguration *config = [WKWebViewConfiguration new];
    MessageHandler *handler = [MessageHandler new];
    [config.userContentController addScriptMessageHandler:handler name:@"native"];
    WKWebView *view = [[WKWebView alloc] initWithFrame:NSMakeRect(0, 0, 300, 200) configuration:config];
    CHECK([gBackendCalls containsObject:@"init"], "the registered backend class is created for a new view");

    Recorder *delegate = [Recorder new];
    view.navigationDelegate = delegate;
    WKNavigation *navigation = [view loadRequest:[NSURLRequest requestWithURL:URL]];
    CHECK([gBackendCalls containsObject:@"start"], "the navigation is handed to the backend");
    spin(^BOOL { return delegate.events.count >= 3; }, 5);
    CHECK([delegate.events isEqualToArray:(@[@"start", @"commit", @"finish"])], "delegate saw start, commit, finish, got %s", [[delegate.events description] UTF8String]);
    CHECK(delegate.navigations.firstObject == navigation, "the delegate gets the navigation the load returned");
    CHECK([view.URL isEqual:URL], "the committed URL is kept after the load");
    CHECK([view.title isEqual:@"Example Page"], "the title from the backend");
    CHECK(view.estimatedProgress == 1.0, "progress is 1 after the load finished");
    CHECK(!view.isLoading, "not loading after the load finished");
    CHECK(!view.canGoBack && !view.canGoForward, "history from the backend (first page)");

    [view loadRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:@"https://example.invalid/two"]]];
    spin(^BOOL { return delegate.events.count >= 6; }, 5);
    CHECK(view.canGoBack && !view.canGoForward, "canGoBack follows the backend");

    __block id result = nil;
    __block BOOL done = NO;
    [view evaluateJavaScript:@"6*7" completionHandler:^(id r, NSError *e) { result = r; done = YES; }];
    spin(^BOOL { return done; }, 5);
    CHECK([result isEqual:@42], "evaluateJavaScript: goes through the backend");

    [gBackendClient backendDidReceiveScriptMessageBody:@{@"k": @"v"} name:@"native"];
    CHECK(handler.message != nil && [handler.message.name isEqual:@"native"] && [handler.message.body isEqual:@{@"k": @"v"}]
          && handler.message.webView == view, "a script message reaches the handler registered under its name");
    handler.message = nil;
    [gBackendClient backendDidReceiveScriptMessageBody:@1 name:@"other"];
    CHECK(handler.message == nil, "a message for an unregistered name is dropped");

    [gBackendCalls removeAllObjects];
    }
    CHECK([gBackendCalls containsObject:@"invalidate"], "releasing the view invalidates its backend");

    [WKWebView _setBackendClass:nil];
    WKWebView *def = [[WKWebView alloc] initWithFrame:NSMakeRect(0, 0, 10, 10) configuration:[WKWebViewConfiguration new]];
    Recorder *d2 = [Recorder new];
    def.navigationDelegate = d2;
    [def loadRequest:[NSURLRequest requestWithURL:URL]];
    spin(^BOOL { return d2.events.count >= 2; }, 5);
    CHECK([d2.events.lastObject isEqual:@"failProvisional"], "after clearing the registration views use the default backend again");
}

int main(void) {
    @autoreleasepool {
        testInitialState();
        testFailingLoad();
        testLoadKinds();
        testOverlappingAndStop();
        testJavaScript();
        testDelegateNotRetained();
        testUserScriptsAndController();
        testPreferencesAndConfiguration();
        testDataStore();
        testBackendSeam();
    }
    if (failures) {
        printf("FAILED %d\n", failures);
        return 1;
    }
    printf("ALL PASSED\n");
    return 0;
}
