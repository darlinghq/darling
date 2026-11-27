/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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
#import <SecurityFoundation/_SFAuthenticatedCiphertext.h>
#import "SFSymmetricInternal.h"

#if __LP64__

@implementation _SFAuthenticatedCiphertext {
    NSData *_ciphertext;
    NSData *_authenticationCode;
    NSData *_initializationVector;
}

+ (BOOL)supportsSecureCoding {
    return YES;
}

- (instancetype)initWithCiphertext:(NSData *)ciphertext authenticationCode:(NSData *)authenticationCode initializationVector:(NSData *)initializationVector {
    self = [super init];
    if (self != nil) {
        _ciphertext = [ciphertext copy];
        _authenticationCode = [authenticationCode copy];
        _initializationVector = [initializationVector copy];
    }
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)coder {
    NSData *ciphertext = [coder decodeObjectOfClass: [NSData class] forKey: @"ciphertext"];
    NSData *code = [coder decodeObjectOfClass: [NSData class] forKey: @"authenticationCode"];
    NSData *iv = [coder decodeObjectOfClass: [NSData class] forKey: @"initializationVector"];
    if (ciphertext == nil || code == nil || iv == nil) {
        [self release];
        return nil;
    }
    return [self initWithCiphertext: ciphertext authenticationCode: code initializationVector: iv];
}

- (void)encodeWithCoder:(NSCoder *)coder {
    [coder encodeObject: _ciphertext forKey: @"ciphertext"];
    [coder encodeObject: _authenticationCode forKey: @"authenticationCode"];
    [coder encodeObject: _initializationVector forKey: @"initializationVector"];
}

- (void)dealloc {
    [_ciphertext release];
    [_authenticationCode release];
    [_initializationVector release];
    [super dealloc];
}

- (NSData *)ciphertext {
    return _ciphertext;
}

- (NSData *)authenticationCode {
    return _authenticationCode;
}

- (NSData *)initializationVector {
    return _initializationVector;
}

@end

#else // __LP64__

#import <darling-stub-forwarding.h>

@implementation _SFAuthenticatedCiphertext

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

#endif // __LP64__
