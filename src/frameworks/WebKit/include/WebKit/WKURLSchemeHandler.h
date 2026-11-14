/*
 Copyright (C) 2026 Lubos Dolezel
 Darling is free software under the GNU GPL version 3 or later.
*/

#import <Foundation/Foundation.h>
#import <WebKit/WKURLSchemeTask.h>

@class WKWebView;

NS_ASSUME_NONNULL_BEGIN

@protocol WKURLSchemeHandler <NSObject>

- (void)webView:(WKWebView *)webView startURLSchemeTask:(id <WKURLSchemeTask>)urlSchemeTask;
- (void)webView:(WKWebView *)webView stopURLSchemeTask:(id <WKURLSchemeTask>)urlSchemeTask;

@end

NS_ASSUME_NONNULL_END
