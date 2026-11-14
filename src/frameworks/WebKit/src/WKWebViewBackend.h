/*
 Copyright (C) 2026 Lubos Dolezel
 Darling is free software under the GNU GPL version 3 or later.
*/

// The seam between WKWebView and whatever renders and runs the page. Not installed with the public
// headers. WKWebView implements the client side and owns one backend; a renderer replaces the
// default backend, which has no engine and fails every load, by registering its class with
// +[WKWebView _setBackendClass:] before views are created.
//
// All client callbacks, and all completion handlers, must be delivered on the main queue.

#import <WebKit/WKError.h>
#import <WebKit/WKNavigation.h>
#import <WebKit/WKScriptMessage.h>
#import <WebKit/WKUserContentController.h>
#import <WebKit/WKWebView.h>
#import <WebKit/WKWebViewConfiguration.h>

NS_ASSUME_NONNULL_BEGIN

// What a navigation was asked to load. Requests, HTML strings and data all carry the URL that the
// page would have: the request URL, the base URL, or about:blank.
@interface WKNavigation ()
@property (nonatomic, readonly, copy) NSURLRequest *request;
// Non-nil for loadHTMLString:, loadData: and loadFileURL: style navigations that have no network request.
@property (nullable, nonatomic, readonly, copy) NSData *contentData;
@property (nullable, nonatomic, readonly, copy) NSString *contentMIMEType;
@property (nullable, nonatomic, readonly, copy) NSString *contentEncodingName;
- (instancetype)initWithRequest:(NSURLRequest *)request;
- (instancetype)initWithRequest:(NSURLRequest *)request contentData:(nullable NSData *)data MIMEType:(nullable NSString *)MIMEType encodingName:(nullable NSString *)encodingName;
@end

@interface WKScriptMessage ()
- (instancetype)initWithBody:(id)body name:(NSString *)name webView:(nullable WKWebView *)webView;
@end

@interface WKUserContentController ()
// Registered handlers by name; a backend delivers a page's message to the handler of that name.
@property (nonatomic, readonly) NSDictionary<NSString *, id <WKScriptMessageHandler>> *scriptMessageHandlers;
@end

@protocol WKWebViewBackendClient <NSObject>

- (void)backendDidStartProvisionalNavigation:(WKNavigation *)navigation;
- (void)backendDidFailProvisionalNavigation:(WKNavigation *)navigation error:(NSError *)error;
- (void)backendDidCommitNavigation:(WKNavigation *)navigation URL:(nullable NSURL *)URL;
- (void)backendDidFinishNavigation:(WKNavigation *)navigation;
- (void)backendDidFailNavigation:(WKNavigation *)navigation error:(NSError *)error;
- (void)backendDidChangeTitle:(nullable NSString *)title;
- (void)backendDidChangeEstimatedProgress:(double)progress;
- (void)backendDidChangeCanGoBack:(BOOL)canGoBack canGoForward:(BOOL)canGoForward;
- (void)backendDidReceiveScriptMessageBody:(id)body name:(NSString *)name;

@end

@protocol WKWebViewBackend <NSObject>

// The configuration is the web view's own copy.
- (instancetype)initWithClient:(id <WKWebViewBackendClient>)client configuration:(WKWebViewConfiguration *)configuration;

// Begins the navigation. Progress is reported through the client, asynchronously.
- (void)startNavigation:(WKNavigation *)navigation;
- (void)stopLoading;
// The navigation started, or nil when there is nothing to go back, forward or reload to.
- (nullable WKNavigation *)goBack;
- (nullable WKNavigation *)goForward;
- (nullable WKNavigation *)reload;

// The result is delivered to the handler, if any, on the main queue.
- (void)evaluateJavaScript:(NSString *)script completionHandler:(void (^ _Nullable)(id _Nullable result, NSError * _Nullable error))completionHandler;

// The web view is going away; drop anything pending and make no further client calls.
- (void)invalidate;

@end

@interface WKWebView (WKWebViewBackendSelection)
+ (void)_setBackendClass:(nullable Class <WKWebViewBackend>)backendClass;
@end

// The backend used when no renderer is registered: no page can be loaded or scripted.
@interface WKUnavailableWebViewBackend : NSObject <WKWebViewBackend>
@end

NS_ASSUME_NONNULL_END
