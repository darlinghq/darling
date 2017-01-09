// BUILD:  $CC dylib.c -dynamiclib -o $BUILD_DIR/signed.dylib
// BUILD:  $CC dylib.c -dynamiclib -o $BUILD_DIR/unsigned.dylib
// BUILD:  $CC main.c            -o $BUILD_DIR/dlopen-signed.exe
// BUILD:  $CC main.c            -o $BUILD_DIR/dlopen-unsigned.exe

// FIXME: add builds that sign the executable and the dylib in in various ways
// At this time we don't have a way to do that, so this test must be run
// manually.

#include <stdio.h>
#include <dlfcn.h>

int main() {
  printf("[BEGIN] dlopen-signing\n");
  void* handle = dlopen("signed.dylib", RTLD_LAZY);
  if ( handle == NULL ) {
     printf("dlerror(): %s\n", dlerror());
     printf("[FAIL] dlopen-signing (signed loading signed)\n");
     return 0;
  } else {
    int result = dlclose(handle);
    if ( result != 0 ) {
       printf("dlclose() returned %c\n", result);
       printf("[FAIL] dlopen-signing (signed unloading signed)\n");
       return 0;
    }
  }

  handle = dlopen("unsigned.dylib", RTLD_LAZY);
  if ( handle != NULL ) {
     printf("dlerror(): %s\n", dlerror());
     printf("[FAIL] dlopen-signing (signed loading unsigned)\n");
     return 0;
  } else {
    int result = dlclose(handle);
    if ( result != 0 ) {
       printf("dlclose() returned %c\n", result);
       printf("[FAIL] dlopen-signing (signed unloading signed)\n");
       return 0;
    }
  }

  printf("[PASS] dlopen-signing\n");

  return 0;
}


