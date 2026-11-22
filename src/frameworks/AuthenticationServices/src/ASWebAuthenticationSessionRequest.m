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

#import <AuthenticationServices/ASWebAuthenticationSessionRequest.h>
#import <AuthenticationServices/ASWebAuthenticationSessionRequestDelegate.h>
#import <darling-stub-forwarding.h>

// This file is built with ARC (see CMakeLists.txt) so that `delegate` is a zeroing weak reference.
// The implementation is 64-bit only, like the other ARC frameworks here: the 32-bit slice uses the
// legacy runtime, which cannot synthesize these properties, and keeps the stub.
#if __LP64__

// The documentation lists init(coder:) and the NSSecureCoding conformance but not the archive keys.
// These keys are our own (they are not observable through the public API); archives are only exchanged
// between Darling processes.
static NSString *const kURLKey = @"requestURL";
static NSString *const kUUIDKey = @"requestUUID";
static NSString *const kEphemeralKey = @"ephemeralSession";
static NSString *const kHeadersKey = @"extraHeaders";
static NSString *const kSchemeKey = @"callbackScheme";

@implementation ASWebAuthenticationSessionRequest {
    NSURL *_URL;
    NSUUID *_UUID;
    BOOL _shouldUseEphemeralSession;
    NSDictionary<NSString *, NSString *> *_additionalHeaderFields;
    NSString *_callbackURLScheme;
}

@synthesize URL = _URL;
@synthesize UUID = _UUID;
@synthesize shouldUseEphemeralSession = _shouldUseEphemeralSession;
@synthesize additionalHeaderFields = _additionalHeaderFields;
@synthesize callbackURLScheme = _callbackURLScheme;

// The documentation lists no public initializer, so a request made with plain -init has no URL and no
// UUID, although both properties are declared non-null. Observed on macOS 26.7.3 (black-box
// observation): a bare alloc/init works and every property is nil or NO.

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (instancetype)initWithURL:(NSURL *)url
                       UUID:(NSUUID *)uuid
  shouldUseEphemeralSession:(BOOL)ephemeral
     additionalHeaderFields:(NSDictionary<NSString *, NSString *> *)headers
          callbackURLScheme:(NSString *)scheme
{
    if ((self = [super init])) {
        _URL = [url copy];
        _UUID = [uuid copy];
        _shouldUseEphemeralSession = ephemeral;
        _additionalHeaderFields = [headers copy];
        _callbackURLScheme = [scheme copy];
    }
    return self;
}

// A request without a URL or UUID cannot be decoded: macOS 26.7.3 fails to decode an archive without a URL
// or UUID (NSCocoaErrorDomain 4865) and initWithCoder: with no request keys returns nil (black-box
// observation). Test: tests/src/authenticationservices_web_session_request.m
- (instancetype)initWithCoder:(NSCoder *)coder
{
    NSURL *url = [coder decodeObjectOfClass:[NSURL class] forKey:kURLKey];
    NSUUID *uuid = [coder decodeObjectOfClass:[NSUUID class] forKey:kUUIDKey];
    if (url == nil || uuid == nil)
        return nil;
    NSSet *headerClasses = [NSSet setWithObjects:[NSDictionary class], [NSString class], nil];
    return [self initWithURL:url
                        UUID:uuid
   shouldUseEphemeralSession:[coder decodeBoolForKey:kEphemeralKey]
      additionalHeaderFields:[coder decodeObjectOfClasses:headerClasses forKey:kHeadersKey]
           callbackURLScheme:[coder decodeObjectOfClass:[NSString class] forKey:kSchemeKey]];
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_URL forKey:kURLKey];
    [coder encodeObject:_UUID forKey:kUUIDKey];
    [coder encodeBool:_shouldUseEphemeralSession forKey:kEphemeralKey];
    [coder encodeObject:_additionalHeaderFields forKey:kHeadersKey];
    [coder encodeObject:_callbackURLScheme forKey:kSchemeKey];
}

// A copy is a new object of the same class that describes the same request (same UUID) but does not
// inherit the delegate. The documentation lists NSCopying without saying what a copy is; on macOS
// 26.7.3 a copy is a new object of the same class (black-box observation).
- (id)copyWithZone:(NSZone *)zone
{
    return [[[self class] allocWithZone:zone] initWithURL:_URL
                                                     UUID:_UUID
                                shouldUseEphemeralSession:_shouldUseEphemeralSession
                                   additionalHeaderFields:_additionalHeaderFields
                                        callbackURLScheme:_callbackURLScheme];
}

// Each call is forwarded to the delegate as made, on the calling thread and before the method returns,
// and nothing is dropped: a second completion, a cancellation after a completion and a completion
// after a cancellation all reach the delegate, and a nil callback URL is forwarded as nil. The
// documentation does not say which thread or queue the delegate is called on; this is what macOS 26.7.3
// does, also when called from a background queue (black-box observation).
// Test: tests/src/authenticationservices_web_session_request.m
- (void)completeWithCallbackURL:(NSURL *)url
{
    id<ASWebAuthenticationSessionRequestDelegate> delegate = self.delegate;
    if ([delegate respondsToSelector:@selector(authenticationSessionRequest:didCompleteWithCallbackURL:)])
        [delegate authenticationSessionRequest:self didCompleteWithCallbackURL:url];
}

- (void)cancelWithError:(NSError *)error
{
    id<ASWebAuthenticationSessionRequestDelegate> delegate = self.delegate;
    if ([delegate respondsToSelector:@selector(authenticationSessionRequest:didCancelWithError:)])
        [delegate authenticationSessionRequest:self didCancelWithError:error];
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

// The properties, methods and protocol methods of the public header have no implementation in the 32-bit stub.
#pragma clang diagnostic ignored "-Wobjc-property-implementation"
#pragma clang diagnostic ignored "-Wincomplete-implementation"
#pragma clang diagnostic ignored "-Wprotocol"

@implementation ASWebAuthenticationSessionRequest

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
