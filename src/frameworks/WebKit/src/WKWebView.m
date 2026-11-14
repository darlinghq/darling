/*
 This file is part of Darling.

 Copyright (C) 2017 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#import <WebKit/WKWebView.h>
#import <WebKit/WKBackForwardList.h>
#import "WKWebViewBackend.h"
#import "WKStubForwarding.h"

// The new implementation is 64-bit only, like WebKit2 on macOS; the 32-bit slice keeps the stub.
#if __LP64__

static Class <WKWebViewBackend> gBackendClass;

@interface WKWebView () <WKWebViewBackendClient>
@property (nullable, nonatomic, readwrite, copy) NSString *title;
@property (nullable, nonatomic, readwrite, copy) NSURL *URL;
@property (nonatomic, readwrite, getter=isLoading) BOOL loading;
@property (nonatomic, readwrite) double estimatedProgress;
@property (nonatomic, readwrite) BOOL canGoBack;
@property (nonatomic, readwrite) BOOL canGoForward;
@end

@implementation WKWebView {
    WKWebViewConfiguration *_configuration;
    id <WKWebViewBackend> _backend;
    WKBackForwardList *_backForwardList;
    // The URL of the committed page, which is what URL reports once nothing is loading.
    NSURL *_committedURL;
    NSUInteger _pendingNavigationCount;
}

@synthesize navigationDelegate = _navigationDelegate;
@synthesize UIDelegate = _UIDelegate;
@synthesize title = _title;
@synthesize URL = _URL;
@synthesize loading = _loading;
@synthesize estimatedProgress = _estimatedProgress;
@synthesize canGoBack = _canGoBack;
@synthesize canGoForward = _canGoForward;
@synthesize allowsBackForwardNavigationGestures = _allowsBackForwardNavigationGestures;
@synthesize customUserAgent = _customUserAgent;
@synthesize allowsLinkPreview = _allowsLinkPreview;
@synthesize allowsMagnification = _allowsMagnification;
@synthesize pageZoom = _pageZoom;
@synthesize inspectable = _inspectable;

+ (void)_setBackendClass:(Class <WKWebViewBackend>)backendClass {
    gBackendClass = backendClass;
}

- (instancetype)initWithFrame:(NSRect)frame configuration:(WKWebViewConfiguration *)configuration {
    if ((self = [super initWithFrame:frame])) {
        [self _setUpWithConfiguration:configuration];
    }
    return self;
}

- (instancetype)initWithFrame:(NSRect)frame {
    return [self initWithFrame:frame configuration:[WKWebViewConfiguration new]];
}

- (instancetype)initWithCoder:(NSCoder *)coder {
    if ((self = [super initWithCoder:coder])) {
        [self _setUpWithConfiguration:nil];
    }
    return self;
}

- (void)_setUpWithConfiguration:(WKWebViewConfiguration *)configuration {
    _configuration = [(configuration ?: [WKWebViewConfiguration new]) copy];
    _title = @"";
    _allowsLinkPreview = YES;
    _pageZoom = 1.0;
    Class cls = gBackendClass ?: [WKUnavailableWebViewBackend class];
    _backend = [[cls alloc] initWithClient:self configuration:_configuration];
}

- (void)dealloc {
    [_backend invalidate];
}

- (BOOL)isFlipped {
    return YES;
}

- (WKWebViewConfiguration *)configuration {
    return [_configuration copy];
}

- (WKBackForwardList *)backForwardList {
    if (!_backForwardList) {
        _backForwardList = [WKBackForwardList new];
    }
    return _backForwardList;
}

- (BOOL)hasOnlySecureContent {
    return NO;
}

- (SecTrustRef)serverTrust {
    return NULL;
}

#pragma mark - State

// The observable properties announce a change only when the value actually changes.
- (void)_updateURL:(NSURL *)URL {
    if (_URL != URL && ![_URL isEqual:URL]) {
        self.URL = URL;
    }
}

- (void)_updateLoading:(BOOL)loading {
    if (_loading != loading) {
        self.loading = loading;
    }
}

- (void)_updateEstimatedProgress:(double)progress {
    if (_estimatedProgress != progress) {
        self.estimatedProgress = progress;
    }
}

#pragma mark - Loading

// Records a navigation as in progress: the page reports loading and the URL it is going to.
- (WKNavigation *)_beginNavigation:(WKNavigation *)navigation {
    _pendingNavigationCount++;
    [self _updateLoading:YES];
    [self _updateURL:navigation.request.URL];
    if (_estimatedProgress < 0.1) {
        [self _updateEstimatedProgress:0.1];
    }
    [_backend startNavigation:navigation];
    return navigation;
}

// Called when a navigation has ended, however it ended.
- (void)_endNavigationWithProgress:(double)progress {
    if (_pendingNavigationCount > 0) {
        _pendingNavigationCount--;
    }
    if (_pendingNavigationCount == 0) {
        [self _updateEstimatedProgress:progress];
        [self _updateLoading:NO];
        [self _updateURL:_committedURL];
    }
}

- (WKNavigation *)loadRequest:(NSURLRequest *)request {
    return [self _beginNavigation:[[WKNavigation alloc] initWithRequest:request]];
}

- (WKNavigation *)loadFileURL:(NSURL *)URL allowingReadAccessToURL:(NSURL *)readAccessURL {
    return [self _beginNavigation:[[WKNavigation alloc] initWithRequest:[NSURLRequest requestWithURL:URL]]];
}

- (WKNavigation *)loadHTMLString:(NSString *)string baseURL:(NSURL *)baseURL {
    NSData *data = [string dataUsingEncoding:NSUTF8StringEncoding];
    return [self loadData:data MIMEType:@"text/html" characterEncodingName:@"UTF-8" baseURL:baseURL];
}

- (WKNavigation *)loadData:(NSData *)data MIMEType:(NSString *)MIMEType characterEncodingName:(NSString *)characterEncodingName baseURL:(NSURL *)baseURL {
    NSURL *URL = baseURL ?: [NSURL URLWithString:@"about:blank"];
    WKNavigation *navigation = [[WKNavigation alloc] initWithRequest:[NSURLRequest requestWithURL:URL]
                                                         contentData:data
                                                            MIMEType:MIMEType
                                                        encodingName:characterEncodingName];
    return [self _beginNavigation:navigation];
}

- (WKNavigation *)goToBackForwardListItem:(WKBackForwardListItem *)item {
    return nil;
}

- (WKNavigation *)goBack {
    WKNavigation *navigation = [_backend goBack];
    return navigation ? [self _beginNavigation:navigation] : nil;
}

- (WKNavigation *)goForward {
    WKNavigation *navigation = [_backend goForward];
    return navigation ? [self _beginNavigation:navigation] : nil;
}

- (WKNavigation *)reload {
    WKNavigation *navigation = [_backend reload];
    return navigation ? [self _beginNavigation:navigation] : nil;
}

- (WKNavigation *)reloadFromOrigin {
    return [self reload];
}

- (void)stopLoading {
    [_backend stopLoading];
    _pendingNavigationCount = 0;
    [self _updateEstimatedProgress:0];
    [self _updateLoading:NO];
    [self _updateURL:_committedURL];
}

- (void)evaluateJavaScript:(NSString *)javaScriptString completionHandler:(void (^)(id, NSError *))completionHandler {
    [_backend evaluateJavaScript:javaScriptString completionHandler:completionHandler];
}

#pragma mark - Actions

- (IBAction)goBack:(id)sender {
    [self goBack];
}

- (IBAction)goForward:(id)sender {
    [self goForward];
}

- (IBAction)reload:(id)sender {
    [self reload];
}

- (IBAction)reloadFromOrigin:(id)sender {
    [self reloadFromOrigin];
}

- (IBAction)stopLoading:(id)sender {
    [self stopLoading];
}

#pragma mark - WKWebViewBackendClient

- (void)backendDidStartProvisionalNavigation:(WKNavigation *)navigation {
    id <WKNavigationDelegate> delegate = self.navigationDelegate;
    if ([delegate respondsToSelector:@selector(webView:didStartProvisionalNavigation:)]) {
        [delegate webView:self didStartProvisionalNavigation:navigation];
    }
}

- (void)backendDidFailProvisionalNavigation:(WKNavigation *)navigation error:(NSError *)error {
    [self _endNavigationWithProgress:0];
    id <WKNavigationDelegate> delegate = self.navigationDelegate;
    if ([delegate respondsToSelector:@selector(webView:didFailProvisionalNavigation:withError:)]) {
        [delegate webView:self didFailProvisionalNavigation:navigation withError:error];
    }
}

- (void)backendDidCommitNavigation:(WKNavigation *)navigation URL:(NSURL *)URL {
    _committedURL = [URL copy];
    [self _updateURL:_committedURL];
    id <WKNavigationDelegate> delegate = self.navigationDelegate;
    if ([delegate respondsToSelector:@selector(webView:didCommitNavigation:)]) {
        [delegate webView:self didCommitNavigation:navigation];
    }
}

- (void)backendDidFinishNavigation:(WKNavigation *)navigation {
    [self _endNavigationWithProgress:1.0];
    id <WKNavigationDelegate> delegate = self.navigationDelegate;
    if ([delegate respondsToSelector:@selector(webView:didFinishNavigation:)]) {
        [delegate webView:self didFinishNavigation:navigation];
    }
}

- (void)backendDidFailNavigation:(WKNavigation *)navigation error:(NSError *)error {
    [self _endNavigationWithProgress:0];
    id <WKNavigationDelegate> delegate = self.navigationDelegate;
    if ([delegate respondsToSelector:@selector(webView:didFailNavigation:withError:)]) {
        [delegate webView:self didFailNavigation:navigation withError:error];
    }
}

- (void)backendDidChangeTitle:(NSString *)title {
    NSString *newTitle = title ?: @"";
    if (![_title isEqual:newTitle]) {
        self.title = newTitle;
    }
}

- (void)backendDidChangeEstimatedProgress:(double)progress {
    [self _updateEstimatedProgress:progress];
}

- (void)backendDidChangeCanGoBack:(BOOL)canGoBack canGoForward:(BOOL)canGoForward {
    if (_canGoBack != canGoBack) {
        self.canGoBack = canGoBack;
    }
    if (_canGoForward != canGoForward) {
        self.canGoForward = canGoForward;
    }
}

- (void)backendDidReceiveScriptMessageBody:(id)body name:(NSString *)name {
    id <WKScriptMessageHandler> handler = _configuration.userContentController.scriptMessageHandlers[name];
    if (handler) {
        WKScriptMessage *message = [[WKScriptMessage alloc] initWithBody:body name:name webView:self];
        [handler userContentController:_configuration.userContentController didReceiveScriptMessage:message];
    }
}

WK_FORWARD_UNIMPLEMENTED_SELECTORS

@end

#else

// The properties and methods of the public headers have no implementation in the 32-bit stub.
#pragma clang diagnostic ignored "-Wobjc-property-implementation"
#pragma clang diagnostic ignored "-Wincomplete-implementation"

@implementation WKWebView

WK_FORWARD_UNIMPLEMENTED_SELECTORS

@end

#endif
