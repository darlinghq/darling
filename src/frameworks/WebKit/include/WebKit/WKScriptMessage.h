/*
 Copyright (C) 2026 Lubos Dolezel
 Darling is free software under the GNU GPL version 3 or later.
*/

#import <Foundation/Foundation.h>

@class WKFrameInfo, WKWebView;

NS_ASSUME_NONNULL_BEGIN

@interface WKScriptMessage : NSObject

@property (nonatomic, readonly, copy) id body;
@property (nullable, nonatomic, readonly, weak) WKWebView *webView;
@property (nonatomic, readonly, copy) NSString *name;
@property (nonatomic, readonly, copy) WKFrameInfo *frameInfo;

@end

NS_ASSUME_NONNULL_END
