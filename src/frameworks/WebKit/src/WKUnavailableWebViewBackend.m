/*
 Copyright (C) 2026 Lubos Dolezel
 Darling is free software under the GNU GPL version 3 or later.
*/

#import "WKWebViewBackend.h"
#import <Foundation/Foundation.h>

#if __LP64__

// The backend used when no renderer is registered. There is no engine, so no page can be loaded or
// scripted, and everything that would need one fails asynchronously on the main queue:
//   - A navigation reports the start of a provisional navigation and then fails it with
//     NSURLErrorDomain / NSURLErrorCannotConnectToHost, what a client sees when the server cannot
//     be reached. A network error fits every kind of load, and clients treat it as a failed load
//     rather than as a reason to rebuild the view, which is what
//     WKErrorWebContentProcessTerminated suggests.
//   - Script evaluation fails with WKErrorDomain / WKErrorWebContentProcessTerminated: the
//     web content that would run the script does not exist.
// The view never commits a page, has no history and never changes its title.

@implementation WKUnavailableWebViewBackend {
    __weak id <WKWebViewBackendClient> _client;
    NSMutableArray<WKNavigation *> *_pending;
    WKNavigation *_lastNavigation;
}

- (instancetype)initWithClient:(id <WKWebViewBackendClient>)client configuration:(WKWebViewConfiguration *)configuration {
    if ((self = [super init])) {
        _client = client;
        _pending = [NSMutableArray new];
    }
    return self;
}

- (NSError *)_connectionErrorForNavigation:(WKNavigation *)navigation {
    NSURL *URL = navigation.request.URL;
    NSMutableDictionary *userInfo = [NSMutableDictionary dictionaryWithObject:@"Could not connect to the server."
                                                                       forKey:NSLocalizedDescriptionKey];
    if (URL) {
        userInfo[NSURLErrorFailingURLErrorKey] = URL;
        userInfo[NSURLErrorFailingURLStringErrorKey] = URL.absoluteString;
    }
    return [NSError errorWithDomain:NSURLErrorDomain code:NSURLErrorCannotConnectToHost userInfo:userInfo];
}

- (void)startNavigation:(WKNavigation *)navigation {
    _lastNavigation = navigation;
    [_pending addObject:navigation];
    dispatch_async(dispatch_get_main_queue(), ^{
        // stopLoading and invalidate take the navigation out of the queue.
        if (![_pending containsObject:navigation]) {
            return;
        }
        [_pending removeObject:navigation];
        id <WKWebViewBackendClient> client = _client;
        [client backendDidStartProvisionalNavigation:navigation];
        [client backendDidFailProvisionalNavigation:navigation error:[self _connectionErrorForNavigation:navigation]];
    });
}

- (void)stopLoading {
    [_pending removeAllObjects];
}

- (WKNavigation *)goBack {
    return nil;
}

- (WKNavigation *)goForward {
    return nil;
}

// Reloading repeats the most recent navigation, which fails again.
- (WKNavigation *)reload {
    WKNavigation *last = _lastNavigation;
    if (!last) {
        return nil;
    }
    return [[WKNavigation alloc] initWithRequest:last.request
                                     contentData:last.contentData
                                        MIMEType:last.contentMIMEType
                                    encodingName:last.contentEncodingName];
}

- (void)evaluateJavaScript:(NSString *)script completionHandler:(void (^)(id, NSError *))completionHandler {
    if (!completionHandler) {
        return;
    }
    void (^handler)(id, NSError *) = [completionHandler copy];
    dispatch_async(dispatch_get_main_queue(), ^{
        NSError *error = [NSError errorWithDomain:WKErrorDomain
                                             code:WKErrorWebContentProcessTerminated
                                         userInfo:@{NSLocalizedDescriptionKey: @"No web content process is available to run the script."}];
        handler(nil, error);
    });
}

- (void)invalidate {
    [_pending removeAllObjects];
    _client = nil;
}

@end

#endif
