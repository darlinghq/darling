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
#import <SecurityFoundation/_SFAuthenticatedEncryptionOperation.h>
#import "SFSymmetricInternal.h"
#import <CommonCrypto/CommonCryptor.h>
#import <CommonCrypto/CommonCryptorSPI.h>
#import <CommonCrypto/CommonRandom.h>
#include <string.h>

#if __LP64__

// AES-256 in Galois/counter mode (NIST SP 800-38D), with a random 96-bit initialization vector and a 128-bit
// authentication code for every message.
#define SFGCMIVLength 12
#define SFGCMTagLength 16

static NSError *SFOperationError(NSString *description) {
    return [NSError errorWithDomain: NSOSStatusErrorDomain code: -50 /* paramErr */
                           userInfo: @{NSLocalizedDescriptionKey: description}];
}

@implementation _SFAuthenticatedEncryptionOperation {
    SFAESKeySpecifier *_keySpecifier;
}

- (instancetype)initWithKeySpecifier:(SFAESKeySpecifier *)keySpecifier {
    self = [super init];
    if (self != nil)
        _keySpecifier = [keySpecifier retain];
    return self;
}

- (void)dealloc {
    [_keySpecifier release];
    [super dealloc];
}

- (SFAuthenticatedCiphertext *)encrypt:(NSData *)data withKey:(SFAESKey *)key error:(NSError **)error {
    return [self encrypt: data withKey: key additionalAuthenticatedData: nil error: error];
}

- (SFAuthenticatedCiphertext *)encrypt:(NSData *)data withKey:(SFAESKey *)key additionalAuthenticatedData:(NSData *)aad error:(NSError **)error {
    if (data == nil || [key keyLength] != [_keySpecifier keyLength]) {
        if (error != NULL)
            *error = SFOperationError(@"the key does not fit the operation");
        return nil;
    }

    uint8_t iv[SFGCMIVLength], tag[SFGCMTagLength];
    NSMutableData *ciphertext = [NSMutableData dataWithLength: [data length]];
    if (CCRandomGenerateBytes(iv, sizeof(iv)) != kCCSuccess ||
        CCCryptorGCMOneshotEncrypt(kCCAlgorithmAES, [key keyBytes], [key keyLength], iv, sizeof(iv),
                                   [aad bytes], [aad length], [data bytes], [data length],
                                   [ciphertext mutableBytes], tag, sizeof(tag)) != kCCSuccess) {
        if (error != NULL)
            *error = SFOperationError(@"the data could not be encrypted");
        return nil;
    }

    return [[[SFAuthenticatedCiphertext alloc] initWithCiphertext: ciphertext
                                               authenticationCode: [NSData dataWithBytes: tag length: sizeof(tag)]
                                             initializationVector: [NSData dataWithBytes: iv length: sizeof(iv)]] autorelease];
}

- (NSData *)decrypt:(SFAuthenticatedCiphertext *)ciphertext withKey:(SFAESKey *)key error:(NSError **)error {
    return [self decrypt: ciphertext withKey: key additionalAuthenticatedData: nil error: error];
}

- (NSData *)decrypt:(SFAuthenticatedCiphertext *)ciphertext withKey:(SFAESKey *)key additionalAuthenticatedData:(NSData *)aad error:(NSError **)error {
    NSData *encrypted = [ciphertext ciphertext];
    NSData *tag = [ciphertext authenticationCode];
    NSData *iv = [ciphertext initializationVector];
    if (encrypted == nil || [tag length] != SFGCMTagLength || [iv length] < SFGCMIVLength ||
        [key keyLength] != [_keySpecifier keyLength]) {
        if (error != NULL)
            *error = SFOperationError(@"the ciphertext or the key does not fit the operation");
        return nil;
    }

    NSMutableData *plaintext = [NSMutableData dataWithLength: [encrypted length]];
    if (CCCryptorGCMOneshotDecrypt(kCCAlgorithmAES, [key keyBytes], [key keyLength], [iv bytes], [iv length],
                                   [aad bytes], [aad length], [encrypted bytes], [encrypted length],
                                   [plaintext mutableBytes], [tag bytes], [tag length]) != kCCSuccess) {
        // nothing of data that failed authentication is kept
        SFClearData(plaintext);
        if (error != NULL)
            *error = [NSError errorWithDomain: NSOSStatusErrorDomain code: -25293 /* errSecAuthFailed */
                                     userInfo: @{NSLocalizedDescriptionKey: @"the ciphertext failed authentication"}];
        return nil;
    }
    return plaintext;
}

@end

#else // __LP64__

#import <darling-stub-forwarding.h>

@implementation _SFAuthenticatedEncryptionOperation

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
