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

#import <WebKit/WKWebViewConfiguration.h>
#import "WKStubForwarding.h"

// The new implementation is 64-bit only, like WebKit2 on macOS; the 32-bit slice keeps the stub.
#if __LP64__

@implementation WKWebViewConfiguration {
    NSMutableDictionary<NSString *, id <WKURLSchemeHandler>> *_schemeHandlers;
}

@synthesize processPool = _processPool;
@synthesize preferences = _preferences;
@synthesize userContentController = _userContentController;
@synthesize websiteDataStore = _websiteDataStore;
@synthesize suppressesIncrementalRendering = _suppressesIncrementalRendering;
@synthesize applicationNameForUserAgent = _applicationNameForUserAgent;
@synthesize allowsAirPlayForMediaPlayback = _allowsAirPlayForMediaPlayback;
@synthesize mediaTypesRequiringUserActionForPlayback = _mediaTypesRequiringUserActionForPlayback;
@synthesize limitsNavigationsToAppBoundDomains = _limitsNavigationsToAppBoundDomains;

- (instancetype)init {
    if ((self = [super init])) {
        _processPool = [WKProcessPool new];
        _preferences = [WKPreferences new];
        _userContentController = [WKUserContentController new];
        _websiteDataStore = [WKWebsiteDataStore defaultDataStore];
        _schemeHandlers = [NSMutableDictionary new];
    }
    return self;
}

// A copy refers to the same process pool, preferences, content controller and data store.
- (id)copyWithZone:(NSZone *)zone {
    WKWebViewConfiguration *copy = [[[self class] allocWithZone:zone] init];
    copy.processPool = _processPool;
    copy.preferences = _preferences;
    copy.userContentController = _userContentController;
    copy.websiteDataStore = _websiteDataStore;
    copy.suppressesIncrementalRendering = _suppressesIncrementalRendering;
    copy.applicationNameForUserAgent = _applicationNameForUserAgent;
    copy.allowsAirPlayForMediaPlayback = _allowsAirPlayForMediaPlayback;
    copy.mediaTypesRequiringUserActionForPlayback = _mediaTypesRequiringUserActionForPlayback;
    copy.limitsNavigationsToAppBoundDomains = _limitsNavigationsToAppBoundDomains;
    [copy->_schemeHandlers addEntriesFromDictionary:_schemeHandlers];
    return copy;
}

- (void)setURLSchemeHandler:(id <WKURLSchemeHandler>)urlSchemeHandler forURLScheme:(NSString *)urlScheme {
    NSString *scheme = [urlScheme lowercaseString];
    if (urlSchemeHandler) {
        _schemeHandlers[scheme] = urlSchemeHandler;
    } else {
        [_schemeHandlers removeObjectForKey:scheme];
    }
}

- (id <WKURLSchemeHandler>)urlSchemeHandlerForURLScheme:(NSString *)urlScheme {
    return _schemeHandlers[[urlScheme lowercaseString]];
}

WK_FORWARD_UNIMPLEMENTED_SELECTORS

@end

#else

// The properties and methods of the public headers have no implementation in the 32-bit stub.
#pragma clang diagnostic ignored "-Wobjc-property-implementation"
#pragma clang diagnostic ignored "-Wincomplete-implementation"

@implementation WKWebViewConfiguration

WK_FORWARD_UNIMPLEMENTED_SELECTORS

@end

#endif
