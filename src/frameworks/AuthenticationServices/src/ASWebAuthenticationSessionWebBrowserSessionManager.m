/*
 This file is part of Darling.

 Copyright (C) 2022 Darling Team

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

#import <AuthenticationServices/ASWebAuthenticationSessionWebBrowserSessionManager.h>
#import <darling-stub-forwarding.h>

// This file is built with ARC (see CMakeLists.txt) so that `sessionHandler` is retained.
// The implementation is 64-bit only, like the other ARC frameworks here: the 32-bit slice uses the
// legacy runtime, which cannot synthesize this property, and keeps the stub.
#if __LP64__

static ASWebAuthenticationSessionWebBrowserSessionManager *sharedBrowserSessionManager;

@implementation ASWebAuthenticationSessionWebBrowserSessionManager {
    id<ASWebAuthenticationSessionWebBrowserSessionHandling> _sessionHandler;
}

@synthesize sessionHandler = _sessionHandler;

// The documentation calls this the shared manager and does not say what a subclass gets: on macOS
// 26.7.3 it is the same object from every thread and for a subclass, while alloc/init and copy give
// other objects (black-box observation). It is always an instance of this class.
+ (ASWebAuthenticationSessionWebBrowserSessionManager *)sharedManager
{
    static dispatch_once_t creation;
    dispatch_once(&creation, ^{
        sharedBrowserSessionManager = [[ASWebAuthenticationSessionWebBrowserSessionManager alloc] init];
    });
    return sharedBrowserSessionManager;
}

// Darling has no component that opens a browser on behalf of Authentication Services, so there is never
// a launch to report and the answer is always NO (macOS 26.7.3 answers NO outside such a launch).
- (BOOL)wasLaunchedByAuthenticationServices
{
    return NO;
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

@end

#else

// The properties and methods of the public header have no implementation in the 32-bit stub.
#pragma clang diagnostic ignored "-Wobjc-property-implementation"
#pragma clang diagnostic ignored "-Wincomplete-implementation"

@implementation ASWebAuthenticationSessionWebBrowserSessionManager

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

@end

#endif
