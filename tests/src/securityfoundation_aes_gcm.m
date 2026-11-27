// Copyright (C) 2026 Lubos Dolezel
//
// SecurityFoundation AES key and authenticated (AES-GCM) encryption: a random 256-bit
// key, an encryption that returns a ciphertext with a 96-bit IV and a 128-bit authentication
// code, the decryption that gets the data back, the secure-coding archive of the ciphertext
// (how securityd stores it), and the rejections: another key, a changed ciphertext, a changed
// authentication code, changed additional authenticated data, a key of the wrong size; and the
// key data a caller holds stays intact when the key that cleared its own copy is gone.
#import <Foundation/Foundation.h>
#import <SecurityFoundation/_SFAESKey.h>
#import <SecurityFoundation/_SFAuthenticatedEncryptionOperation.h>
#include <stdio.h>

static int failures;
static void expect(BOOL ok, NSString *what) {
    printf("%s %s\n", ok ? "PASS" : "FAIL", [what UTF8String]);
    if (!ok)
        failures++;
}

@interface SFAuthenticatedCiphertext (Test)
@property (readonly) NSData *ciphertext, *authenticationCode, *initializationVector;
- (instancetype)initWithCiphertext:(NSData *)c authenticationCode:(NSData *)a initializationVector:(NSData *)i;
@end

int main(void) {
    @autoreleasepool {
        SFAESKeySpecifier *spec = [[SFAESKeySpecifier alloc] initWithBitSize: SFAESKeyBitSize256];
        NSError *error = nil;
        SFAESKey *key = [[SFAESKey alloc] initRandomKeyWithSpecifier: spec error: &error];
        SFAESKey *other = [[SFAESKey alloc] initRandomKeyWithSpecifier: spec error: &error];
        expect(key != nil && [[key keyData] length] == 32, @"a random key has 256 bits");
        expect(![[key keyData] isEqual: [other keyData]], @"two random keys differ");
        expect([[SFAESKey alloc] initWithData: [NSData dataWithBytes: "short" length: 5] specifier: spec error: &error] == nil && error != nil,
               @"key data of the wrong size is refused");
        SFAESKey *copy = [[SFAESKey alloc] initWithData: [key keyData] specifier: spec error: &error];
        expect(copy != nil && [[copy keyData] isEqual: [key keyData]], @"a key is made from its data");

        SFAuthenticatedEncryptionOperation *op = [[SFAuthenticatedEncryptionOperation alloc] initWithKeySpecifier: spec];
        NSData *plain = [@"a secret that is longer than one cipher block, and some more" dataUsingEncoding: NSUTF8StringEncoding];
        SFAuthenticatedCiphertext *c = [op encrypt: plain withKey: key error: &error];
        expect(c != nil && [[c ciphertext] length] == [plain length] && ![[c ciphertext] isEqual: plain], @"encrypting changes the data and keeps its length");
        expect([[c authenticationCode] length] == 16 && [[c initializationVector] length] == 12, @"the authentication code is 128 bits, the IV 96");
        SFAuthenticatedCiphertext *c2 = [op encrypt: plain withKey: key error: &error];
        expect(![[c initializationVector] isEqual: [c2 initializationVector]], @"every encryption has its own IV");
        expect([[op decrypt: c withKey: key error: &error] isEqual: plain], @"decrypting gives the data back");

        expect([op decrypt: c withKey: other error: &error] == nil && error != nil, @"another key is refused");
        NSMutableData *flipped = [[c ciphertext] mutableCopy];
        ((uint8_t *) [flipped mutableBytes])[0] ^= 1;
        SFAuthenticatedCiphertext *bad = [[SFAuthenticatedCiphertext alloc] initWithCiphertext: flipped authenticationCode: [c authenticationCode] initializationVector: [c initializationVector]];
        expect([op decrypt: bad withKey: key error: &error] == nil, @"a changed ciphertext is refused");
        NSMutableData *tag = [[c authenticationCode] mutableCopy];
        ((uint8_t *) [tag mutableBytes])[15] ^= 0x80;
        bad = [[SFAuthenticatedCiphertext alloc] initWithCiphertext: [c ciphertext] authenticationCode: tag initializationVector: [c initializationVector]];
        expect([op decrypt: bad withKey: key error: &error] == nil, @"a changed authentication code is refused");

        NSData *aad = [@"header" dataUsingEncoding: NSUTF8StringEncoding];
        SFAuthenticatedCiphertext *withAad = [op encrypt: plain withKey: key additionalAuthenticatedData: aad error: &error];
        expect([[op decrypt: withAad withKey: key additionalAuthenticatedData: aad error: &error] isEqual: plain], @"additional authenticated data is accepted");
        expect([op decrypt: withAad withKey: key additionalAuthenticatedData: [@"other" dataUsingEncoding: NSUTF8StringEncoding] error: &error] == nil, @"other additional authenticated data is refused");
        expect([op decrypt: withAad withKey: key error: &error] == nil, @"missing additional authenticated data is refused");

        NSData *archive = [NSKeyedArchiver archivedDataWithRootObject: c requiringSecureCoding: YES error: &error];
        SFAuthenticatedCiphertext *restored = archive ? [NSKeyedUnarchiver unarchivedObjectOfClass: [SFAuthenticatedCiphertext class] fromData: archive error: &error] : nil;
        expect(restored != nil && [[op decrypt: restored withKey: key error: &error] isEqual: plain], @"the secure-coding archive of a ciphertext decrypts");
        expect([[op encrypt: [NSData data] withKey: key error: &error] ciphertext] != nil, @"empty data can be encrypted");

        // a key clears its own copy of the key material when it goes away, but not the caller's data
        uint8_t raw[32];
        for (int i = 0; i < 32; i++)
            raw[i] = (uint8_t)(0x40 + i);
        NSData *given = [NSData dataWithBytes: raw length: sizeof(raw)];
        SFAESKey *shortLived = [[SFAESKey alloc] initWithData: given specifier: spec error: &error];
        NSData *taken = [[shortLived keyData] retain];
        SFAuthenticatedCiphertext *c3 = [[op encrypt: plain withKey: shortLived error: &error] retain];
        [shortLived release];
        expect([given isEqual: [NSData dataWithBytes: raw length: sizeof(raw)]], @"the data a key was made from is kept as it was");
        expect([taken isEqual: given], @"the key data taken from a key outlives the key");
        SFAESKey *again = [[SFAESKey alloc] initWithData: taken specifier: spec error: &error];
        expect([[op decrypt: c3 withKey: again error: &error] isEqual: plain], @"and still decrypts");
    }
    printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
    return failures != 0;
}
