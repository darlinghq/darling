
// BUILD:  $CC linked1.m -dynamiclib -o $BUILD_DIR/liblinked1.dylib -install_name $RUN_DIR/liblinked1.dylib -lobjc
// BUILD:  $CC linked2.m -dynamiclib -o $BUILD_DIR/liblinked2.dylib -install_name $RUN_DIR/liblinked2.dylib -lobjc
// BUILD:  $CC main.m -o $BUILD_DIR/_dyld_for_each_objc_class.exe $BUILD_DIR/liblinked1.dylib $BUILD_DIR/liblinked2.dylib -lobjc

// RUN:  ./_dyld_for_each_objc_class.exe

// The preoptimized objc class information is available via _dyld_for_each_objc_class().
// This test ensures that we match the objc behaviour when there are duplicates.
// For objc today, it walks the images in reverse load order, so the deepest library will be
// the canonical definition of a class.

#include <mach-o/dyld_priv.h>

#import <Foundation/Foundation.h>

// All the libraries have a copy of DyldClass
@interface DyldClass : NSObject
@end

@implementation DyldClass
@end

// Only the main executable has DyldMainClass
@interface DyldMainClass : NSObject
@end

@implementation DyldMainClass
@end

extern Class OBJC_CLASS_$_DyldClass;
extern Class OBJC_CLASS_$_DyldMainClass;

Class getMainDyldClass() {
  return (Class)&OBJC_CLASS_$_DyldClass;
}

Class getMainDyldMainClass() {
  return (Class)&OBJC_CLASS_$_DyldMainClass;
}

extern int printf(const char*, ...);

extern id objc_getClass(const char *name);

// Get the DyldClass from liblinked1.dylib
extern Class getLinked1DyldClass();

// Get the DyldClass from liblinked2.dylib
extern Class getLinked2DyldClass();

// Get the DyldLinkedClass from liblinked2.dylib
extern Class getLinked2DyldLinkedClass();

static bool gotDyldClassMain = false;
static bool gotDyldClassLinked = false;
static bool gotDyldClassLinked2 = false;

int main() {
  printf("[BEGIN] _dyld_for_each_objc_class\n");

  // This API is only available with dyld3 and shared caches.  If we have dyld2 then don't do anything
  const char* testDyldMode = getenv("TEST_DYLD_MODE");
  assert(testDyldMode);

  size_t unusedCacheLen;
  bool haveSharedCache = _dyld_get_shared_cache_range(&unusedCacheLen) != 0;
  if (!strcmp(testDyldMode, "2") || !haveSharedCache) {
    __block bool sawClass = false;
    _dyld_for_each_objc_class("DyldClass", ^(void* classPtr, bool isLoaded, bool* stop) {
      sawClass = true;
    });
    if (sawClass) {
      printf("[FAIL] _dyld_for_each_objc_class: dyld2 shouldn't see any classes\n");
      return 0;
    }
    printf("[PASS] _dyld_for_each_objc_class (dyld2 or no shared cache)\n");
    return 0;
  }

  // Check that DyldClass comes from liblinked2 as it is last in load order
  id runtimeDyldClass = objc_getClass("DyldClass");
  if (runtimeDyldClass != getLinked2DyldClass()) {
    printf("[FAIL] _dyld_for_each_objc_class: DyldClass should have come from liblinked2\n");
    return 0;
  }

  // Check that DyldLinkedClass comes from liblinked2 as it is last in load order
  id runtimeDyldLinkedClass = objc_getClass("DyldLinkedClass");
  if (runtimeDyldLinkedClass != getLinked2DyldLinkedClass()) {
    printf("[FAIL] _dyld_for_each_objc_class: DyldLinkedClass should have come from liblinked2\n");
    return 0;
  }

  // Walk all the implementations of "DyldClass"
  _dyld_for_each_objc_class("DyldClass", ^(void* classPtr, bool isLoaded, bool* stop) {
    // We should walk these in the order liblinked2, liblinked, main exe
    if (!gotDyldClassLinked2) {
      if (classPtr != getLinked2DyldClass()) {
        printf("[FAIL] _dyld_for_each_objc_class: Optimized DyldClass should have come from liblinked2\n");
        *stop = true;
        return;
      }
      if (!isLoaded) {
        printf("[FAIL] _dyld_for_each_objc_class: Optimized DyldClass isLoaded should have been set on liblinked2\n");
        *stop = true;
        return;
      }
      gotDyldClassLinked2 = true;
      return;
    }
    if (!gotDyldClassLinked) {
      if (classPtr != getLinked1DyldClass()) {
        printf("[FAIL] _dyld_for_each_objc_class: Optimized DyldClass should have come from liblinked\n");
        *stop = true;
        return;
      }
      if (!isLoaded) {
        printf("[FAIL] _dyld_for_each_objc_class: Optimized DyldClass isLoaded should have been set on liblinked\n");
        *stop = true;
        return;
      }
      gotDyldClassLinked = true;
      return;
    }
    if (!gotDyldClassMain) {
      if (classPtr != getMainDyldClass()) {
        printf("[FAIL] _dyld_for_each_objc_class: Optimized DyldClass should have come from main exe\n");
        *stop = true;
        return;
      }
      if (!isLoaded) {
        printf("[FAIL] _dyld_for_each_objc_class: Optimized DyldClass isLoaded should have been set on main exe\n");
        *stop = true;
        return;
      }
      gotDyldClassMain = true;
      return;
    }
    printf("[FAIL] _dyld_for_each_objc_class: Unexpected Optimized DyldClass\n");
    return;
  });

  if ( !gotDyldClassLinked2 || !gotDyldClassLinked || !gotDyldClassMain) {
    printf("[FAIL] _dyld_for_each_objc_class: Failed to find all duplicates of 'DyldClass'\n");
    return 0;
  }

  // Visit again, and return liblinked2's DyldClass
  __block void* dyldClassImpl = nil;
  _dyld_for_each_objc_class("DyldClass", ^(void* classPtr, bool isLoaded, bool* stop) {
    dyldClassImpl = classPtr;
    *stop = true;
  });
  if (dyldClassImpl != getLinked2DyldClass()) {
    printf("[FAIL] _dyld_for_each_objc_class: _dyld_for_each_objc_class should have returned DyldClass from liblinked2\n");
    return 0;
  }

  // Visit DyldMainClass and make sure it makes the callback for just the result from main.exe
  __block void* dyldMainClassImpl = nil;
  _dyld_for_each_objc_class("DyldMainClass", ^(void* classPtr, bool isLoaded, bool* stop) {
    dyldMainClassImpl = classPtr;
    *stop = true;
  });
  if (dyldMainClassImpl != getMainDyldMainClass()) {
    printf("[FAIL] _dyld_for_each_objc_class: _dyld_for_each_objc_class should have returned DyldMainClass from main.exe\n");
    return 0;
  }

  printf("[PASS] _dyld_for_each_objc_class\n");

  return 0;
}
