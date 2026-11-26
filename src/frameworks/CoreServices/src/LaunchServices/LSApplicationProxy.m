#import <Foundation/Foundation.h>
#import <darling-stub-forwarding.h>
#import "LSBundleProxy.h"

// DUMMY beyond what LSBundleProxy provides: other messages are forwarded to stubs.

@interface LSApplicationProxy : LSBundleProxy
@end

@implementation LSApplicationProxy
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


