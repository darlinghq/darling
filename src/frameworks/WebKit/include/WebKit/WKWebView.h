/*
 Copyright (C) 2026 Lubos Dolezel
 Darling is free software under the GNU GPL version 3 or later.
*/

#import <Foundation/Foundation.h>
#import <AppKit/NSView.h>
#import <WebKit/WKError.h>
#import <WebKit/WKNavigation.h>
#import <WebKit/WKNavigationDelegate.h>
#import <WebKit/WKUIDelegate.h>
#import <WebKit/WKWebViewConfiguration.h>
#import <Security/Security.h>

@class WKBackForwardList, WKBackForwardListItem;

NS_ASSUME_NONNULL_BEGIN

@interface WKWebView : NSView

@property (nonatomic, readonly, copy) WKWebViewConfiguration *configuration;
@property (nullable, nonatomic, weak) id <WKNavigationDelegate> navigationDelegate;
@property (nullable, nonatomic, weak) id <WKUIDelegate> UIDelegate;
@property (nonatomic, readonly, strong) WKBackForwardList *backForwardList;

- (instancetype)initWithFrame:(NSRect)frame configuration:(WKWebViewConfiguration *)configuration NS_DESIGNATED_INITIALIZER;
- (nullable instancetype)initWithCoder:(NSCoder *)coder NS_DESIGNATED_INITIALIZER;

- (nullable WKNavigation *)loadRequest:(NSURLRequest *)request;
- (nullable WKNavigation *)loadFileURL:(NSURL *)URL allowingReadAccessToURL:(NSURL *)readAccessURL;
- (nullable WKNavigation *)loadHTMLString:(NSString *)string baseURL:(nullable NSURL *)baseURL;
- (nullable WKNavigation *)loadData:(NSData *)data MIMEType:(NSString *)MIMEType characterEncodingName:(NSString *)characterEncodingName baseURL:(NSURL *)baseURL;
- (nullable WKNavigation *)goToBackForwardListItem:(WKBackForwardListItem *)item;

@property (nullable, nonatomic, readonly, copy) NSString *title;
@property (nullable, nonatomic, readonly, copy) NSURL *URL;
@property (nonatomic, readonly, getter=isLoading) BOOL loading;
@property (nonatomic, readonly) double estimatedProgress;
@property (nonatomic, readonly) BOOL hasOnlySecureContent;
@property (nonatomic, readonly, nullable) SecTrustRef serverTrust;
@property (nonatomic, readonly) BOOL canGoBack;
@property (nonatomic, readonly) BOOL canGoForward;

- (nullable WKNavigation *)goBack;
- (nullable WKNavigation *)goForward;
- (nullable WKNavigation *)reload;
- (nullable WKNavigation *)reloadFromOrigin;
- (void)stopLoading;

- (void)evaluateJavaScript:(NSString *)javaScriptString completionHandler:(void (^ _Nullable)(id _Nullable, NSError * _Nullable error))completionHandler;

@property (nonatomic) BOOL allowsBackForwardNavigationGestures;
@property (nullable, nonatomic, copy) NSString *customUserAgent;
@property (nonatomic) BOOL allowsLinkPreview;
@property (nonatomic) BOOL allowsMagnification;
@property (nonatomic) CGFloat pageZoom;
@property (nonatomic, getter=isInspectable) BOOL inspectable;

@end

@interface WKWebView (WKIBActions)

- (IBAction)goBack:(nullable id)sender;
- (IBAction)goForward:(nullable id)sender;
- (IBAction)reload:(nullable id)sender;
- (IBAction)reloadFromOrigin:(nullable id)sender;
- (IBAction)stopLoading:(nullable id)sender;

@end

NS_ASSUME_NONNULL_END
