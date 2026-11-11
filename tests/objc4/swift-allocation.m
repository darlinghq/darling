// Synthetic Swift metadata integration: real objc runtime, no Swift toolchain.
// Reuses objc4's existing metadata fixture, including its class-size layout.
#include <objc/runtime.h>
#include <objc/objc-internal.h>
#include "../runtime/NSObject.h"
#include <malloc/malloc.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define EXTERN_C
#include "../test/swift-class-def.m"

#if defined(NDEBUG) || defined(__arm64e__) || !defined(__LP64__)
#error Requires assertions and unsigned-pointer LP64 metadata
#endif

extern Class _objc_realizeClassFromSwift(Class cls, void *previously);
Class initAllocation(Class cls, void *argument);
SWIFT_CLASS(AllocationSwift, NSObject, initAllocation);
__attribute__((used, section("__DATA,__objc_classlist,regular,no_dead_strip")))
static void *allocationClassList = &OBJC_CLASS_$_AllocationSwift;

__attribute__((objc_root_class))
@interface NestedAllocation { Class isa; uint64_t payload[8]; }
@end
@implementation NestedAllocation
@end
extern uintptr_t nestedMetadata[] __asm__("_OBJC_CLASS_$_NestedAllocation");

static unsigned callbacks;
static int replaceMetadata;
static int lookupControl;
static Class realizedClass;

static int isRealized(Class cls) {
    uintptr_t data = ((uintptr_t *)cls)[4] & ~(uintptr_t)7;
    return (*(uint32_t *)data & (UINT32_C(1) << 31)) != 0;
}

Class initAllocation(Class cls, void *argument) {
    assert(cls == RawAllocationSwift && argument == NULL && callbacks++ == 0);
    assert(!isRealized(cls));
    // This nested first allocation requires runtimeLock. It would deadlock if
    // the outer realization failed to release the lock around this callback.
    Class nested = (Class)nestedMetadata;
    assert(!isRealized(nested));
    if (lookupControl) assert(objc_getClass("NestedAllocation") == nested);
    id instance = class_createInstance(nested, 0);
    assert(instance && object_getClass(instance) == nested && isRealized(nested));
    object_dispose(instance);
    fprintf(stderr, "PHASE: Swift initializer completed nested realization\n");

    realizedClass = cls;
    if (replaceMetadata) {
        realizedClass = (Class)malloc(OBJC_MAX_CLASS_SIZE);
        assert(realizedClass);
        memcpy(realizedClass, cls, OBJC_MAX_CLASS_SIZE);
    }
    _objc_realizeClassFromSwift(realizedClass, cls);
    assert(isRealized(realizedClass));
    return realizedClass;
}

int main(int argc, char **argv) {
    const char *mode = argc == 2 ? argv[1] : getenv("OBJC_RAW_ALLOCATION_MODE");
    if (argc > 2 || !mode || (strcmp(mode, "swift") && strcmp(mode, "swift-replacement") && strcmp(mode, "swift-replacement-lookup"))) return 2;
    lookupControl = !strcmp(mode, "swift-replacement-lookup");
    replaceMetadata = strcmp(mode, "swift") != 0;
    fprintf(stderr, "BEGIN: %s, initially realized=%d\n", mode, isRealized(RawAllocationSwift));
    if (isRealized(RawAllocationSwift) || isRealized((Class)nestedMetadata)) return 77;
    assert(class_createInstance(Nil, 0) == nil);
    if (lookupControl) {
        Class lookedUp = objc_getClass("AllocationSwift");
        assert(lookedUp && lookedUp == realizedClass && callbacks == 1);
    }
    id object = class_createInstance(lookupControl ? realizedClass : RawAllocationSwift, 24);
    assert(object && callbacks == 1 && object_getClass(object) == realizedClass);
    assert((realizedClass != RawAllocationSwift) == replaceMetadata);
    assert(malloc_size(object) >= class_getInstanceSize(realizedClass) + 24);
    object_dispose(object);
    // Subsequent allocation must use the realized fast path without a callback.
    object = class_createInstance(realizedClass, 0);
    assert(object && object_getClass(object) == realizedClass && callbacks == 1);
    object_dispose(object);
    // Registered replacement metadata remains owned by the runtime.
    fprintf(stderr, "PASS: %s allocation, instance identity/size, nil input and disposal\n", mode);
    return 0;
}
