/*
 Copyright (C) 2026 Lubos Dolezel
 Darling is free software under the GNU GPL version 3 or later.
*/

// The classes in this framework that are only partly implemented keep forwarding the selectors
// that are not implemented yet to the stub handler, as the generated stubs do.

#import <darling-stub-forwarding.h>

#define WK_FORWARD_UNIMPLEMENTED_SELECTORS \
- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector { \
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector); \
} \
- (void)forwardInvocation:(NSInvocation *)anInvocation { \
    DarlingStubForwardInvocation(self, anInvocation); \
} \
+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector { \
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector); \
} \
+ (void)forwardInvocation:(NSInvocation *)anInvocation { \
    DarlingStubForwardInvocation(self, anInvocation); \
}
