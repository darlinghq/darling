// Integration fixture for the proposed allocation-realization change.
// Build against the runtime under test, without ARC:
//   clang -fno-objc-arc objc-raw-allocation.m -lobjc -o objc-raw-allocation
// Run each mode in a fresh process: raw, lookup, or concurrent.
// Supports ordinary arm64/x86_64 class metadata, not arm64e pointer signing.
#include <objc/runtime.h>
#include <malloc/malloc.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#ifdef NDEBUG
#error This fixture requires assertions enabled, including their checked calls
#endif

#if defined(__arm64e__) || !defined(__LP64__)
#error This metadata probe requires unsigned-pointer LP64 Objective-C 2 metadata
#endif

__attribute__((objc_root_class))
@interface RawAllocationFixture {
    Class isa;
    uint64_t payload[16];
}
@end
@implementation RawAllocationFixture
@end

// Address the compiler-emitted class structure, not an Objective-C message or
// a lookup API that may realize it before the allocation under test.
extern uintptr_t fixtureMetadata[] __asm__("_OBJC_CLASS_$_RawAllocationFixture");

static int fixtureIsRealized(void) {
    // Same metadata observation used by objc4's Swift initializer fixtures.
    uintptr_t data = fixtureMetadata[4] & ~(uintptr_t)7;
    return (*(const uint32_t *)data & (UINT32_C(1) << 31)) != 0;
}

static pthread_mutex_t gateMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t gateCondition = PTHREAD_COND_INITIALIZER;
static unsigned ready;
static int start;

static void *allocateRepeatedly(void *argument) {
    Class cls = (Class)argument;
    assert(pthread_mutex_lock(&gateMutex) == 0);
    ++ready;
    assert(pthread_cond_broadcast(&gateCondition) == 0);
    while (!start) assert(pthread_cond_wait(&gateCondition, &gateMutex) == 0);
    assert(pthread_mutex_unlock(&gateMutex) == 0);
    for (unsigned i = 0; i < 1000; ++i) {
        id object = class_createInstance(cls, 24);
        assert(object != nil && object_getClass(object) == cls);
        assert(malloc_size(object) >= class_getInstanceSize(cls) + 24);
        object_dispose(object);
    }
    return NULL;
}

static void concurrentAllocations(Class cls) {
    pthread_t threads[8];
    for (unsigned i = 0; i < 8; ++i)
        assert(pthread_create(&threads[i], NULL, allocateRepeatedly, cls) == 0);
    assert(pthread_mutex_lock(&gateMutex) == 0);
    while (ready != 8) assert(pthread_cond_wait(&gateCondition, &gateMutex) == 0);
    assert(!fixtureIsRealized());
    start = 1;
    assert(pthread_cond_broadcast(&gateCondition) == 0);
    assert(pthread_mutex_unlock(&gateMutex) == 0);
    for (unsigned i = 0; i < 8; ++i) assert(pthread_join(threads[i], NULL) == 0);
    assert(fixtureIsRealized());
    fprintf(stderr, "PHASE: eight threads completed 8000 allocations\n");
}

int main(int argc, char **argv) {
    // The staged server can launch an init path without supplying arguments.
    const char *mode = argc == 2 ? argv[1] : getenv("OBJC_RAW_ALLOCATION_MODE");
    if (argc > 2 || !mode || (strcmp(mode, "raw") && strcmp(mode, "lookup") && strcmp(mode, "concurrent"))) {
        fprintf(stderr, "usage: %s raw|lookup|concurrent\n", argv[0]);
        return 2;
    }
    Class cls = (Class)fixtureMetadata;
    fprintf(stderr, "BEGIN: %s, initially realized=%d\n", mode, fixtureIsRealized());
    if (strcmp(mode, "lookup")) {
        if (fixtureIsRealized()) {
            fprintf(stderr, "INCONCLUSIVE: class was already realized before allocation\n");
            return 77;
        }
    } else {
        cls = objc_getClass("RawAllocationFixture");
        fprintf(stderr, "PHASE: lookup returned\n");
        assert(cls == (Class)fixtureMetadata && fixtureIsRealized());
    }
    assert(class_createInstance(Nil, 0) == nil);
    if (!strcmp(mode, "concurrent")) concurrentAllocations(cls);
    fprintf(stderr, "PHASE: allocating\n");
    id object = class_createInstance(cls, 24);
    fprintf(stderr, "PHASE: allocation returned\n");
    assert(object != nil);
    assert(fixtureIsRealized());
    assert(object_getClass(object) == cls);
    assert(malloc_size(object) >= class_getInstanceSize(cls) + 24);
    object_dispose(object);
    fprintf(stderr, "PASS: %s allocation, instance identity/size, nil input and disposal\n", mode);
    return 0;
}
