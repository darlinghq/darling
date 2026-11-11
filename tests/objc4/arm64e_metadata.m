/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Objective-C metadata of an arm64e binary (signed superclass, method list
 * and IMP pointers) used by a plain-arm64 runtime: a subclass, a category
 * on it, and calls through objc_msgSend and through method_getImplementation.
 *
 * Build for arm64e, without ARC:
 *   clang -target arm64e-apple-macos11 -fno-objc-arc arm64e_metadata.m -lobjc -o arm64e_metadata
 * Expected: every line PASS, exit status 0. Without the signature
 * stripping the runtime faults on the first message.
 */
#include <objc/runtime.h>
#include <objc/message.h>
#include <stdio.h>
#include <string.h>

__attribute__((objc_root_class))
@interface Base {
    Class isa;
}
+ (id)new;
- (int)baseValue;
@end

@implementation Base
+ (id)new { return class_createInstance(self, 0); }
- (int)baseValue { return 1; }
@end

@interface Derived : Base
- (int)derivedValue;
@end

@implementation Derived
- (int)derivedValue { return 2; }
@end

@interface Derived (Extra)
- (int)categoryValue;
@end

@implementation Derived (Extra)
- (int)categoryValue { return 3; }
@end

static int failures;

static void check(const char *what, int cond)
{
    printf("%s: %s\n", cond ? "PASS" : "FAIL", what);
    if (!cond)
        failures++;
}

int main(void)
{
    Derived *obj = [Derived new];
    Method m;
    int (*fn)(id, SEL);

    check("superclass", class_getSuperclass([Derived class]) == [Base class]);
    check("base method", [obj baseValue] == 1);
    check("own method", [obj derivedValue] == 2);
    check("category method", [obj categoryValue] == 3);

    m = class_getInstanceMethod([Derived class], @selector(categoryValue));
    fn = (int (*)(id, SEL))method_getImplementation(m);
    check("category IMP called directly", m != NULL && fn(obj, @selector(categoryValue)) == 3);

    m = class_getInstanceMethod([Base class], @selector(baseValue));
    fn = (int (*)(id, SEL))method_getImplementation(m);
    check("base IMP called directly", m != NULL && fn(obj, @selector(baseValue)) == 1);

    check("class name", strcmp(class_getName(object_getClass(obj)), "Derived") == 0);
    object_dispose(obj);
    return failures != 0;
}
