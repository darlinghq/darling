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

#import <WebKit/WKUserContentController.h>
#import "WKWebViewBackend.h"
#import "WKStubForwarding.h"

// The new implementation is 64-bit only, like WebKit2 on macOS; the 32-bit slice keeps the stub.
#if __LP64__

@implementation WKUserContentController {
    NSMutableArray<WKUserScript *> *_userScripts;
    NSMutableDictionary<NSString *, id <WKScriptMessageHandler>> *_handlers;
}

- (instancetype)init {
    if ((self = [super init])) {
        _userScripts = [NSMutableArray new];
        _handlers = [NSMutableDictionary new];
    }
    return self;
}

- (NSArray<WKUserScript *> *)userScripts {
    return [_userScripts copy];
}

- (void)addUserScript:(WKUserScript *)userScript {
    [_userScripts addObject:userScript];
}

- (void)removeAllUserScripts {
    [_userScripts removeAllObjects];
}

- (void)addScriptMessageHandler:(id <WKScriptMessageHandler>)scriptMessageHandler name:(NSString *)name {
    if (_handlers[name]) {
        [NSException raise:NSInvalidArgumentException format:@"Attempt to add script message handler with name '%@' when one already exists.", name];
    }
    _handlers[name] = scriptMessageHandler;
}

- (void)removeScriptMessageHandlerForName:(NSString *)name {
    [_handlers removeObjectForKey:name];
}

- (void)removeAllScriptMessageHandlers {
    [_handlers removeAllObjects];
}

- (NSDictionary<NSString *, id <WKScriptMessageHandler>> *)scriptMessageHandlers {
    return [_handlers copy];
}

WK_FORWARD_UNIMPLEMENTED_SELECTORS

@end

#else

// The properties and methods of the public headers have no implementation in the 32-bit stub.
#pragma clang diagnostic ignored "-Wobjc-property-implementation"
#pragma clang diagnostic ignored "-Wincomplete-implementation"

@implementation WKUserContentController

WK_FORWARD_UNIMPLEMENTED_SELECTORS

@end

#endif
