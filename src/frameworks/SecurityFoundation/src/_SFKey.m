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
#import <SecurityFoundation/_SFKey.h>
#import "SFSymmetricInternal.h"

#if __LP64__

@implementation _SFKey {
    // a buffer of the key's own, so that it can be cleared when the key goes away
    NSMutableData *_keyData;
}

- (instancetype)initWithKeyData:(NSData *)data {
    self = [super init];
    if (self != nil)
        _keyData = [data mutableCopy];
    return self;
}

- (void)dealloc {
    SFClearData(_keyData);
    [_keyData release];
    [super dealloc];
}

// A copy: the key's own buffer is cleared when the key is deallocated, while the caller may keep the data.
- (NSData *)keyData {
    return [NSData dataWithData: _keyData];
}

- (const void *)keyBytes {
    return [_keyData bytes];
}

- (size_t)keyLength {
    return [_keyData length];
}

@end

#else // __LP64__

#import <darling-stub-forwarding.h>

@implementation _SFKey

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
