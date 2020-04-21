
// BUILD:  $CC linked1.m -dynamiclib -o $BUILD_DIR/liblinked1.dylib -install_name $RUN_DIR/liblinked1.dylib -lobjc
// BUILD:  $CC linked2.m -dynamiclib -o $BUILD_DIR/liblinked2.dylib -install_name $RUN_DIR/liblinked2.dylib -lobjc
// BUILD:  $CC main.m -o $BUILD_DIR/_dyld_for_each_objc_class-duplicates.exe $BUILD_DIR/liblinked1.dylib $BUILD_DIR/liblinked2.dylib -lobjc -framework Foundation

// RUN:  ./_dyld_for_each_objc_class-duplicates.exe

#include <mach-o/dyld_priv.h>
#include <stdlib.h>
#include <string.h>

#import <Foundation/NSObject.h>

// All the libraries have a copy of NSString
@interface NSString : NSObject
@end

@implementation NSString
@end

extern Class OBJC_CLASS_$_NSString;

// The main executable also has versions of these Foundation classes
@interface NSDictionary : NSObject
@end

@implementation NSDictionary
@end

extern Class OBJC_CLASS_$_NSDictionary;

@interface NSError : NSObject
@end

@implementation NSError
@end

extern Class OBJC_CLASS_$_NSError;

@interface NSSet : NSObject
@end

@implementation NSSet
@end

extern Class OBJC_CLASS_$_NSSet;

@interface NSArray : NSObject
@end

@implementation NSArray
@end

extern Class OBJC_CLASS_$_NSArray;

Class getMainNSString() {
  return (Class)&OBJC_CLASS_$_NSString;
}

extern int printf(const char*, ...);

extern id objc_getClass(const char *name);

// Get the NSString from liblinked1.dylib
extern Class getLinked1NSString();

// Get the NSString from liblinked2.dylib
extern Class getLinked2NSString();

static bool gotNSStringMain = false;
static bool gotNSStringLinked = false;
static bool gotNSStringLinked2 = false;
static bool gotNSStringFoundation = false;

bool testDuplicate(const char* className, Class nonCacheClass) {
  // Walk all the implementations of the class.  There should be 2.  One in the executable and one in the shared cache
  // The shared cache one should be returned first.

  // The objc runtime should have chosen the Foundation one as the canonical definition.
  Class objcRuntimeClassImpl = (Class)objc_getClass(className);
  if (objcRuntimeClassImpl == nil) {
    printf("[FAIL] _dyld_for_each_objc_class-duplicates: class %s not found via runtime\n", className);
    return false;
  }

  if (objcRuntimeClassImpl == nonCacheClass) {
    printf("[FAIL] _dyld_for_each_objc_class-duplicates: class %s from runtime should not match main exexutable\n", className);
    return false;
  }

  __block bool foundSharedCacheImpl = false;
  __block bool foundMainExecutableImpl = false;
  __block bool foundAnyClass = false;
  __block bool foundTooManyClasses = false;
  _dyld_for_each_objc_class(className, ^(void* classPtr, bool isLoaded, bool* stop) {
    foundAnyClass = true;

    // We should walk these in the order Foundation, main exe
    if (!foundSharedCacheImpl) {
      if (classPtr != objcRuntimeClassImpl) {
        printf("[FAIL] _dyld_for_each_objc_class-duplicates: Optimized class %s should have come from Foundation\n", className);
        *stop = true;
        return;
      }
      if (!isLoaded) {
        printf("[FAIL] _dyld_for_each_objc_class-duplicates: Optimized class %s isLoaded should have been set on Foundation\n", className);
        *stop = true;
        return;
      }
      foundSharedCacheImpl = true;
      return;
    }
    if (!foundMainExecutableImpl) {
      if (classPtr != nonCacheClass) {
        printf("[FAIL] _dyld_for_each_objc_class-duplicates: Optimized class %s should have come from main executable\n", className);
        *stop = true;
        return;
      }
      if (!isLoaded) {
        printf("[FAIL] _dyld_for_each_objc_class-duplicates: Optimized class %s isLoaded should have been set on main executable\n", className);
        *stop = true;
        return;
      }
      foundMainExecutableImpl = true;
      return;
    }

    foundTooManyClasses = true;
    printf("[FAIL] _dyld_for_each_objc_class-duplicates: class %s found somewhere other than main executable and Foundation\n", className);
    *stop = true;
    return;
  });

  if (!foundAnyClass) {
    printf("[FAIL] _dyld_for_each_objc_class-duplicates: class %s not found\n", className);
    return false;
  }

  if (foundTooManyClasses) {
    printf("[FAIL] _dyld_for_each_objc_class-duplicates: class %s found too many times\n", className);
    return false;
  }

  if (!foundSharedCacheImpl || !foundMainExecutableImpl) {
    printf("[FAIL] _dyld_for_each_objc_class-duplicates: class %s not found for shared cache or main executable\n", className);
    return false;
  }

  return true;
}

int main() {
  printf("[BEGIN] _dyld_for_each_objc_class-duplicates\n");

  // This API is only available with dyld3 and shared caches.  If we have dyld2 then don't do anything
  const char* testDyldMode = getenv("TEST_DYLD_MODE");
  assert(testDyldMode);

  size_t sharedCacheLen = 0;
  const void* sharedCacheStart = 0;
  sharedCacheStart = _dyld_get_shared_cache_range(&sharedCacheLen);
  bool haveSharedCache = sharedCacheStart != NULL;
  if (!strcmp(testDyldMode, "2") || !haveSharedCache) {
    __block bool sawClass = false;
    _dyld_for_each_objc_class("NSString", ^(void* classPtr, bool isLoaded, bool* stop) {
      sawClass = true;
    });
    if (sawClass) {
      printf("[FAIL] _dyld_for_each_objc_class-duplicates: dyld2 shouldn't see any classes\n");
      return 0;
    }
    printf("[PASS] _dyld_for_each_objc_class-duplicates (dyld2 or no shared cache)\n");
    return 0;
  }

  // Check that NSString comes from Foundation as the shared cache should win here.
  id runtimeNSString = objc_getClass("NSString");
  if ( (uint64_t)runtimeNSString < (uint64_t)sharedCacheStart ) {
    printf("[FAIL] _dyld_for_each_objc_class-duplicates: NSString should have come from Foundation but instead was %p\n", runtimeNSString);
    return 0;
  }
  if ( (uint64_t)runtimeNSString >= ((uint64_t)sharedCacheStart + sharedCacheLen) ) {
    printf("[FAIL] _dyld_for_each_objc_class-duplicates: NSString should have come from Foundation but instead was %p\n", runtimeNSString);
    return 0;
  }

  // Walk all the implementations of "NSString"
  _dyld_for_each_objc_class("NSString", ^(void* classPtr, bool isLoaded, bool* stop) {
    // We should walk these in the order Foundation, liblinked2, liblinked, main exe
    if (!gotNSStringFoundation) {
      if (classPtr != runtimeNSString) {
        printf("[FAIL] _dyld_for_each_objc_class-duplicates: Optimized NSString should have come from Foundation\n");
        *stop = true;
        return;
      }
      if (!isLoaded) {
        printf("[FAIL] _dyld_for_each_objc_class-duplicates: Optimized NSString isLoaded should have been set on Foundation\n");
        *stop = true;
        return;
      }
      gotNSStringFoundation = true;
      return;
    }
    if (!gotNSStringLinked2) {
      if (classPtr != getLinked2NSString()) {
        printf("[FAIL] _dyld_for_each_objc_class-duplicates: Optimized NSString should have come from liblinked2\n");
        *stop = true;
        return;
      }
      if (!isLoaded) {
        printf("[FAIL] _dyld_for_each_objc_class-duplicates: Optimized NSString isLoaded should have been set on liblinked2\n");
        *stop = true;
        return;
      }
      gotNSStringLinked2 = true;
      return;
    }
    if (!gotNSStringLinked) {
      if (classPtr != getLinked1NSString()) {
        printf("[FAIL] _dyld_for_each_objc_class-duplicates: Optimized NSString should have come from liblinked\n");
        *stop = true;
        return;
      }
      if (!isLoaded) {
        printf("[FAIL] _dyld_for_each_objc_class-duplicates: Optimized NSString isLoaded should have been set on liblinked\n");
        *stop = true;
        return;
      }
      gotNSStringLinked = true;
      return;
    }
    if (!gotNSStringMain) {
      if (classPtr != getMainNSString()) {
        printf("[FAIL] _dyld_for_each_objc_class-duplicates: Optimized NSString should have come from main exe\n");
        *stop = true;
        return;
      }
      if (!isLoaded) {
        printf("[FAIL] _dyld_for_each_objc_class-duplicates: Optimized NSString isLoaded should have been set on main exe\n");
        *stop = true;
        return;
      }
      gotNSStringMain = true;
      return;
    }
    printf("[FAIL] _dyld_for_each_objc_class-duplicates: Unexpected Optimized NSString\n");
    return;
  });

  if ( !gotNSStringFoundation || !gotNSStringLinked2 || !gotNSStringLinked || !gotNSStringMain) {
    printf("[FAIL] _dyld_for_each_objc_class-duplicates: Failed to find all duplicates of 'NSString'\n");
    return 0;
  }

  // Visit again, and return Foundation's NSString
  __block void* NSStringImpl = nil;
  _dyld_for_each_objc_class("NSString", ^(void* classPtr, bool isLoaded, bool* stop) {
    NSStringImpl = classPtr;
    *stop = true;
  });
  if (NSStringImpl != runtimeNSString) {
    printf("[FAIL] _dyld_for_each_objc_class-duplicates: _dyld_for_each_objc_class should have returned NSString from Foundation\n");
    return 0;
  }

  if (!testDuplicate("NSDictionary", (Class)&OBJC_CLASS_$_NSDictionary))
    return 0;

  if (!testDuplicate("NSError", (Class)&OBJC_CLASS_$_NSError))
    return 0;

  if (!testDuplicate("NSSet", (Class)&OBJC_CLASS_$_NSSet))
    return 0;

  if (!testDuplicate("NSArray", (Class)&OBJC_CLASS_$_NSArray))
    return 0;

  printf("[PASS] _dyld_for_each_objc_class-duplicates\n");

  return 0;
}
