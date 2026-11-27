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
#import <SecurityFoundation/_SFAESKey.h>
#import "SFSymmetricInternal.h"
#import <CommonCrypto/CommonRandom.h>

#if __LP64__

static NSError *SFAESKeyError(NSString *description) {
    return [NSError errorWithDomain: NSOSStatusErrorDomain code: -50 /* paramErr */
                           userInfo: @{NSLocalizedDescriptionKey: description}];
}

@implementation _SFAESKey

- (instancetype)initRandomKeyWithSpecifier:(SFAESKeySpecifier *)specifier error:(NSError **)error {
    NSMutableData *key = [NSMutableData dataWithLength: [specifier keyLength]];
    if ([key length] == 0 || CCRandomGenerateBytes([key mutableBytes], [key length]) != kCCSuccess) {
        SFClearData(key);
        if (error != NULL)
            *error = SFAESKeyError(@"no random key could be generated");
        [self release];
        return nil;
    }
    self = [super initWithKeyData: key];
    // the key has its own copy now
    SFClearData(key);
    return self;
}

- (instancetype)initWithData:(NSData *)data specifier:(SFAESKeySpecifier *)specifier error:(NSError **)error {
    if ([data length] != [specifier keyLength]) {
        if (error != NULL)
            *error = SFAESKeyError(@"the key data does not have the size of the key specifier");
        [self release];
        return nil;
    }
    return [super initWithKeyData: data];
}

@end

#else // __LP64__

#import <darling-stub-forwarding.h>

@implementation _SFAESKey

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
