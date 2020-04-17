// note: -Os is needed for armv7 to work around compiler issue where at -O0 it computes pointer to function and calls that

// BUILD:  $CC foo.c -dynamiclib -o $BUILD_DIR/libfoo.dylib         -install_name $RUN_DIR/libfoo.dylib
// BUILD:  $CC foo.c -dynamiclib -o $TEMP_DIR/libfoo-present.dylib  -install_name $RUN_DIR/libfoo.dylib -DHAS_SYMBOL=1
// BUILD:  $CC main.c            -o $BUILD_DIR/lazy-symbol-missing.exe        $TEMP_DIR/libfoo-present.dylib -Os
// BUILD:  $CC main.c            -o $BUILD_DIR/lazy-symbol-missing-flat.exe   -undefined dynamic_lookup      -Os -DFLAT=1
// BUILD:  $CC main-call.c       -o $BUILD_DIR/lazy-symbol-missing-called.exe $TEMP_DIR/libfoo-present.dylib -Os
// BUILD:  $CC runner.c          -o $BUILD_DIR/lazy-symbol-runner.exe -DRUN_DIR="$RUN_DIR"

// NO_CRASH_LOG: lazy-symbol-missing-called.exe

// RUN:  ./lazy-symbol-missing.exe
// RUN:  ./lazy-symbol-runner.exe
// RUN:  ./lazy-symbol-missing-flat.exe


#include <stdio.h>
#include <stdbool.h>
#include <mach-o/getsect.h>

#if __LP64__
extern struct mach_header_64 __dso_handle;
#else
extern struct mach_header __dso_handle;
#endif

extern int slipperySymbol();

#ifdef FLAT
  #define TESTNAME "lazy-symbol-missing-flat"
#else
  #define TESTNAME "lazy-symbol-missing"
#endif

int main(int argc, const char* argv[])
{
    printf("[BEGIN] " TESTNAME "\n");

#if __arm64e__
    // arm64e always uses chained binds which does not support lazy binding
    bool supportsLazyBinding = false;
#else
    // other architectures may or may not use lazy binding
    unsigned long sectSize = 0;
    bool supportsLazyBinding = (getsectiondata(&__dso_handle, "__DATA", "__la_symbol_ptr", &sectSize) != NULL);
  #if __ARM_ARCH_7K__
    // armv7 has two names for lazy pointers section
    if ( !supportsLazyBinding )
        supportsLazyBinding = (getsectiondata(&__dso_handle, "__DATA", "__lazy_symbol", &sectSize) != NULL);
  #endif
#endif

    if ( supportsLazyBinding ) {
        // add runtime check that results in the function never being called
        if ( argc < 0 )
            slipperySymbol();
    }

    printf("[PASS]  " TESTNAME "\n");

	return 0;
}

