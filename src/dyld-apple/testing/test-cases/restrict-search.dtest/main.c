// BUILD_ONLY: MacOSX

// BUILD:  mkdir $BUILD_DIR/lc
// BUILD:  $CC foo.c -dynamiclib -o $BUILD_DIR/lc/libfoo.dylib -install_name /blah/libfoo.dylib
// BUILD:  $CC main.c $BUILD_DIR/lc/libfoo.dylib -o $BUILD_DIR/restrict-search-lc-find.exe    -Wl,-dyld_env,DYLD_LIBRARY_PATH=@loader_path/lc -DMODE=lc-find -DSHOULD_BE_FOUND=1
// BUILD:  $CC main.c $BUILD_DIR/lc/libfoo.dylib -o $BUILD_DIR/restrict-search-lc-no-find.exe -Wl,-dyld_env,DYLD_LIBRARY_PATH=@loader_path/lc -DMODE=lc-no-find -sectcreate __RESTRICT __restrict /dev/null
// BUILD:  mkdir $BUILD_DIR/rpath
// BUILD:  $CC foo.c -dynamiclib -o $BUILD_DIR/rpath/libfoo.dylib -install_name @rpath/libfoo.dylib
// BUILD:  $CC main.c $BUILD_DIR/rpath/libfoo.dylib -o $BUILD_DIR/restrict-search-rpath-find.exe    -Wl,-rpath,@loader_path/rpath/ -DMODE=rpath-find -DSHOULD_BE_FOUND=1
// BUILD:  $CC main.c $BUILD_DIR/rpath/libfoo.dylib -o $BUILD_DIR/restrict-search-rpath-no-find.exe -Wl,-rpath,@loader_path/rpath/ -DMODE=rpath-no-find -sectcreate __RESTRICT __restrict /dev/null


// RUN:  ./restrict-search-lc-find.exe
// RUN:  ./restrict-search-lc-no-find.exe
// RUN:  ./restrict-search-rpath-find.exe
// RUN:  ./restrict-search-rpath-no-find.exe



#include <stdio.h>
#include <dlfcn.h>

// Two ways to find libfoo.dylb: @rpath or DYLD_LIBRARY_PATH (set via LC_DYLD_ENVIRONMENT)
// These should work for non-restrictured programs.
// These should fail for restricted programs.
// By weak linking we can test if libfoo.dylib was found or not.


extern int foo() __attribute__((weak_import));


#define STRINGIFY2( x) #x
#define STRINGIFY(x) STRINGIFY2(x)


int main(int argc, const char* argv[])
{
   printf("[BEGIN] restrict-search %s\n", STRINGIFY(MODE));
#if SHOULD_BE_FOUND
    if ( &foo == NULL )
        printf("[FAIL] restrict-search %s\n", STRINGIFY(MODE));
    else
        printf("[PASS] restrict-search %s\n", STRINGIFY(MODE));
#else
    // dylib won't be found at runtime, so &foo should be NULL
    if ( &foo == NULL )
        printf("[PASS] restrict-search %s\n", STRINGIFY(MODE));
    else
        printf("[FAIL] restrict-search %s\n", STRINGIFY(MODE));
#endif

	return 0;
}

