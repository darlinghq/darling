/*
 Copyright (C) 2026 Lubos Dolezel
 Darling is free software under the GNU GPL version 3 or later.
*/

#import <Foundation/Foundation.h>
#import <WebKit/WKPreferences.h>
#import <WebKit/WKProcessPool.h>
#import <WebKit/WKURLSchemeHandler.h>
#import <WebKit/WKUserContentController.h>
#import <WebKit/WKWebsiteDataStore.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_OPTIONS(NSUInteger, WKAudiovisualMediaTypes) {
    WKAudiovisualMediaTypeNone = 0,
    WKAudiovisualMediaTypeAudio = 1 << 0,
    WKAudiovisualMediaTypeVideo = 1 << 1,
    WKAudiovisualMediaTypeAll = NSUIntegerMax
};

@interface WKWebViewConfiguration : NSObject <NSCopying>

@property (nonatomic, strong) WKProcessPool *processPool;
@property (nonatomic, strong) WKPreferences *preferences;
@property (nonatomic, strong) WKUserContentController *userContentController;
@property (nonatomic, strong) WKWebsiteDataStore *websiteDataStore;
@property (nonatomic) BOOL suppressesIncrementalRendering;
@property (nullable, nonatomic, copy) NSString *applicationNameForUserAgent;
@property (nonatomic) BOOL allowsAirPlayForMediaPlayback;
@property (nonatomic) WKAudiovisualMediaTypes mediaTypesRequiringUserActionForPlayback;
@property (nonatomic) BOOL limitsNavigationsToAppBoundDomains;

- (void)setURLSchemeHandler:(nullable id <WKURLSchemeHandler>)urlSchemeHandler forURLScheme:(NSString *)urlScheme;
- (nullable id <WKURLSchemeHandler>)urlSchemeHandlerForURLScheme:(NSString *)urlScheme;

@end

NS_ASSUME_NONNULL_END
