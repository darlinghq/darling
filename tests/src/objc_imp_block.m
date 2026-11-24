// Copyright (C) 2026 Lubos Dolezel
#import <Foundation/Foundation.h>
#import <objc/runtime.h>
#import <objc/message.h>
#include <stdio.h>
#include <string.h>

// imp_implementationWithBlock / imp_getBlock / imp_removeBlock, as documented in
// <objc/runtime.h>: the IMP calls the block with the receiver left out of the
// arguments, for every return type (object, scalar, floating point, small and large
// structs); imp_getBlock returns the block the IMP was made from; imp_removeBlock
// invalidates the IMP and releases the block.

typedef struct { int a; int b; } Small;
typedef struct { int a, b, c; } Mid;
typedef struct { double x, y, z; int tag; } Big;

@interface Probe : NSObject
@end
@implementation Probe
@end

static int failures;
static void expectTrue(BOOL ok, const char *what) {
    printf("%s %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok)
        failures++;
}

static int deallocated;
@interface Token : NSObject
@end
@implementation Token
- (void) dealloc { deallocated++; [super dealloc]; }
@end

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    @autoreleasepool {
        Probe *probe = [Probe new];

        // object return, object and scalar arguments
        id (^objectBlock)(id, NSString *, int) = ^id(id self_, NSString *s, int n) {
            return [NSString stringWithFormat: @"%@:%@:%d", [self_ class], s, n];
        };
        IMP objectImp = imp_implementationWithBlock(objectBlock);
        expectTrue(objectImp != NULL, "object-returning block gets an IMP");
        class_addMethod([Probe class], @selector(objectMethod:count:), objectImp, "@@:@i");
        id (*callObject)(id, SEL, NSString *, int) = (void *) objc_msgSend;
        id result = callObject(probe, @selector(objectMethod:count:), @"str", 42);
        expectTrue([result isEqual: @"Probe:str:42"], "object return, receiver and arguments arrive");

        // scalar return
        IMP scalarImp = imp_implementationWithBlock(^long long(id self_, long long a, double d) { return a * 2 + (long long) d; });
        class_addMethod([Probe class], @selector(scalar:with:), scalarImp, "q@:qd");
        long long (*callScalar)(id, SEL, long long, double) = (void *) objc_msgSend;
        expectTrue(callScalar(probe, @selector(scalar:with:), 20, 2.0) == 42, "64-bit integer return");

        // floating point return
        IMP floatImp = imp_implementationWithBlock(^double(id self_, double d) { return d * 1.5; });
        class_addMethod([Probe class], @selector(floating:), floatImp, "d@:d");
        double (*callFloat)(id, SEL, double) = (void *) objc_msgSend_fpret;
        expectTrue(callFloat(probe, @selector(floating:), 4.0) == 6.0, "double return");

        // 8-byte struct return: in registers on both architectures
        IMP smallImp = imp_implementationWithBlock(^Small(id self_, int a) { Small s = {a, a + 1}; return s; });
        class_addMethod([Probe class], @selector(smallStruct:), smallImp, "{Small=ii}@:i");
        Small (*callSmall)(id, SEL, int) = (void *) objc_msgSend;
        Small small = callSmall(probe, @selector(smallStruct:), 7);
        expectTrue(small.a == 7 && small.b == 8, "small struct return");

        // 12-byte struct return: in registers on x86_64, through memory (stret) on i386
        IMP midImp = imp_implementationWithBlock(^Mid(id self_, int a) { Mid m = {a, a + 1, a + 2}; return m; });
        class_addMethod([Probe class], @selector(midStruct:), midImp, "{Mid=iii}@:i");
#ifdef __i386__
        Mid (*callMid)(id, SEL, int) = (void *) objc_msgSend_stret;
#else
        Mid (*callMid)(id, SEL, int) = (void *) objc_msgSend;
#endif
        Mid mid = callMid(probe, @selector(midStruct:), 5);
        expectTrue(mid.a == 5 && mid.b == 6 && mid.c == 7, "12-byte struct return");

        // large struct return: always through memory
        IMP bigImp = imp_implementationWithBlock(^Big(id self_, double scale, int tag) { Big b = {scale, scale * 2, scale * 3, tag}; return b; });
        class_addMethod([Probe class], @selector(bigStruct:tag:), bigImp, "{Big=dddi}@:di");
        Big (*callBig)(id, SEL, double, int) = (void *) objc_msgSend_stret;
        Big big = callBig(probe, @selector(bigStruct:tag:), 2.0, 9);
        expectTrue(big.x == 2.0 && big.y == 4.0 && big.z == 6.0 && big.tag == 9, "large struct return (stret) with arguments");

        // struct argument
        IMP structArgImp = imp_implementationWithBlock(^int(id self_, Big b, Small s) { return (int) (b.x + b.y + b.z) + s.a + s.b + b.tag; });
        class_addMethod([Probe class], @selector(takeBig:small:), structArgImp, "i@:{Big=dddi}{Small=ii}");
        int (*callStructArg)(id, SEL, Big, Small) = (void *) objc_msgSend;
        Big in = {1, 2, 3, 100};
        Small ins = {10, 20};
        expectTrue(callStructArg(probe, @selector(takeBig:small:), in, ins) == 136, "struct arguments");

        // imp_getBlock returns the block the IMP was made from
        expectTrue(imp_getBlock(objectImp) == (id) objectBlock || [imp_getBlock(objectImp) isEqual: objectBlock],
                   "imp_getBlock returns the block of an IMP");
        expectTrue(imp_getBlock((IMP) objc_msgSend) == nil, "imp_getBlock of an IMP not made from a block is nil");

        // imp_removeBlock: releases the block and invalidates the IMP
        Token *token = [Token new];
        IMP held = imp_implementationWithBlock(^id(id self_) { return token; });
        [token release];
        expectTrue(deallocated == 0, "the block keeps what it captured alive");
        expectTrue(imp_removeBlock(held) == YES, "imp_removeBlock succeeds for a block IMP");
        expectTrue(deallocated == 1, "imp_removeBlock releases the block");
        expectTrue(imp_getBlock(held) == nil, "imp_getBlock of a removed IMP is nil");
        expectTrue(imp_removeBlock((IMP) objc_msgSend) == NO, "imp_removeBlock of a plain IMP fails");

        // many IMPs, removed in another order, and reused slots
        enum { N = 600 };
        IMP imps[N];
        for (int i = 0; i < N; i++) {
            int k = i;
            imps[i] = imp_implementationWithBlock(^int(id self_) { return k; });
        }
        BOOL all = YES;
        for (int i = 0; i < N; i++) {
            int (*f)(id, SEL) = (void *) imps[i];
            all = all && f(probe, @selector(description)) == i;
        }
        expectTrue(all, "600 block IMPs each call their own block");
        for (int i = N - 1; i >= 0; i -= 2) all = all && imp_removeBlock(imps[i]);
        for (int i = 0; i < N; i += 2) all = all && imp_removeBlock(imps[i]);
        expectTrue(all, "all of them can be removed");
        IMP again = imp_implementationWithBlock(^int(id self_) { return 77; });
        int (*f)(id, SEL) = (void *) again;
        expectTrue(f(probe, @selector(description)) == 77, "an IMP made after the removals works");
        imp_removeBlock(again);

        // objc_allocateClassPair with a block method
        Class made = objc_allocateClassPair([NSObject class], "ProbeMade", 0);
        class_addMethod(made, @selector(answer), imp_implementationWithBlock(^int(id self_) { return 42; }), "i@:");
        objc_registerClassPair(made);
        int (*callAnswer)(id, SEL) = (void *) objc_msgSend;
        id made1 = [[made alloc] init];
        expectTrue(callAnswer(made1, @selector(answer)) == 42, "a block method on a runtime-created class");
        [made1 release];
    }
    printf("%s\n", failures ? "FAIL" : "PASS");
    return failures != 0;
}
