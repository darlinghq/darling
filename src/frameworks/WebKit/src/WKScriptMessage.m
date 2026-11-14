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

#import <WebKit/WKScriptMessage.h>
#import <WebKit/WKFrameInfo.h>
#import "WKWebViewBackend.h"
#import "WKStubForwarding.h"

// The new implementation is 64-bit only, like WebKit2 on macOS; the 32-bit slice keeps the stub.
#if __LP64__

@implementation WKScriptMessage {
    id _body;
    NSString *_name;
    __weak WKWebView *_webView;
    WKFrameInfo *_frameInfo;
}

@synthesize body = _body;
@synthesize name = _name;
@synthesize webView = _webView;
@synthesize frameInfo = _frameInfo;

- (instancetype)initWithBody:(id)body name:(NSString *)name webView:(WKWebView *)webView {
    if ((self = [super init])) {
        _body = body;
        _name = [name copy];
        _webView = webView;
    }
    return self;
}

WK_FORWARD_UNIMPLEMENTED_SELECTORS

@end

#else

// The properties and methods of the public headers have no implementation in the 32-bit stub.
#pragma clang diagnostic ignored "-Wobjc-property-implementation"
#pragma clang diagnostic ignored "-Wincomplete-implementation"

@implementation WKScriptMessage

WK_FORWARD_UNIMPLEMENTED_SELECTORS

@end

#endif
