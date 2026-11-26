/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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

#import <SecurityFoundation/SFAuthorization.h>
#import <darling-stub-forwarding.h>

// Everything is delegated to Authorization Services (Security.framework); the object only holds the
// AuthorizationRef and turns a failing OSStatus into an NSError.
@implementation SFAuthorization

+ (id)authorization
{
    return [[[self alloc] init] autorelease];
}

+ (id)authorizationWithFlags:(AuthorizationFlags)flags rights:(const AuthorizationRights *)rights environment:(const AuthorizationEnvironment *)environment
{
    return [[[self alloc] initWithFlags:flags rights:rights environment:environment] autorelease];
}

- (id)init
{
    return [self initWithFlags:kAuthorizationFlagDefaults rights:NULL environment:kAuthorizationEmptyEnvironment];
}

- (id)initWithFlags:(AuthorizationFlags)flags rights:(const AuthorizationRights *)rights environment:(const AuthorizationEnvironment *)environment
{
    self = [super init];
    if (self != nil) {
        // GUESS: the documentation does not say what the initialisers return when Authorization Services
        // cannot create the reference; there is no usable object to return, so it is nil.
        if (AuthorizationCreate(rights, environment, flags, &_authorization) != errAuthorizationSuccess) {
            _authorization = NULL;
            [self release];
            return nil;
        }
    }
    return self;
}

- (void)dealloc
{
    if (_authorization != NULL)
        AuthorizationFree(_authorization, kAuthorizationFlagDefaults);
    [super dealloc];
}

- (AuthorizationRef)authorizationRef
{
    return _authorization;
}

- (BOOL)obtainWithRights:(const AuthorizationRights *)rights flags:(AuthorizationFlags)flags environment:(const AuthorizationEnvironment *)environment authorizedRights:(AuthorizationRights **)authorizedRights error:(NSError **)error
{
    OSStatus status = AuthorizationCopyRights(_authorization, rights, environment, flags, authorizedRights);
    // Observed on macOS 26.7.3 and 10.14.4 (probe, tests/src/securityfoundation_sfauthorization.m): the error holds the
    // result code in NSOSStatusErrorDomain for every call, also when it succeeds (code 0, noErr).
    if (error != NULL)
        *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:status userInfo:nil];
    return status == errAuthorizationSuccess;
}

- (BOOL)obtainWithRight:(AuthorizationString)rightName flags:(AuthorizationFlags)flags error:(NSError **)error
{
    AuthorizationItem item = { rightName, 0, NULL, 0 };
    AuthorizationRights rights = { 1, &item };
    return [self obtainWithRights:&rights flags:flags environment:kAuthorizationEmptyEnvironment authorizedRights:NULL error:error];
}

- (void)invalidateCredentials
{
    // Freeing with kAuthorizationFlagDestroyRights is how Authorization Services keeps the rights obtained
    // through a reference from being preserved. Observed on macOS 26.7.3 and 10.14.4: afterwards authorizationRef is
    // NULL, the obtain methods fail with errAuthorizationInvalidRef, and invalidating again is harmless.
    if (_authorization != NULL) {
        AuthorizationFree(_authorization, kAuthorizationFlagDestroyRights);
        _authorization = NULL;
    }
}

// NSCoding and NSSecureCoding (-initWithCoder:, -encodeWithCoder:, +supportsSecureCoding) are documented but not
// implemented: an AuthorizationRef must never be serialised. Selectors without an implementation keep the generic
// stub behaviour they had before; it never runs for the methods above.
- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation {
    DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation {
    DarlingStubForwardInvocation(self, anInvocation);
}

@end
