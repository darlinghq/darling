// BUILD_ONLY: MacOSX

// BUILD:  $CC main.c            -o $BUILD_DIR/NSAddImage-fail.exe -Wno-deprecated-declarations

// RUN:  ./NSAddImage-fail.exe return
// RUN:  NOCR_TEST_NAME="NSAddImage-fail expected abort" $REQUIRE_CRASH  ./NSAddImage-fail.exe abort



#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <mach-o/dyld.h>


int main(int argc, const char* argv[])
{
    const char* arg = argv[1];

    if ( strcmp(arg, "return") == 0 ) {
        printf("[BEGIN] NSAddImage-fail %s\n", arg);
        const struct mach_header* mh = NSAddImage("/xqz/42/libnotfound.xxx", NSADDIMAGE_OPTION_RETURN_ONLY_IF_LOADED);
        if ( mh == NULL )
            printf("[PASS] NSAddImage-fail %s\n", arg);
        else
            printf("[FAIL] NSAddImage-fail %s\n", arg);
    }
    else {
        // run with nocr which print BEGIN/PASS/FAIL
        NSAddImage("/xqz/42/libnotfound.xxx", 0);
    }

	return 0;
}

