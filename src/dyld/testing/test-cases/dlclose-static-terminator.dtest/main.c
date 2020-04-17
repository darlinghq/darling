
// BUILD:  $CC foo.c  -dynamiclib -install_name $RUN_DIR/libterm.dylib  -o $BUILD_DIR/libterm.dylib
// BUILD:  $CC main.c -o $BUILD_DIR/dlclose-term.exe -DRUN_DIR="$RUN_DIR"

// RUN:  ./dlclose-term.exe

#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <mach-o/dyld_priv.h>


// verify dlclose() runs static terminator

typedef void (*NotifyProc)(void);

static bool termDidRun = false;

static void termNotifyFunc()
{
    termDidRun = true;
}

int main()
{
    printf("[BEGIN] dlclose-static-terminator\n");

    // load dylib
    void* handle = dlopen(RUN_DIR "/libterm.dylib", RTLD_LAZY);
    if ( handle == NULL ) {
        printf("[FAIL]  dlclose-static-terminator: libterm.dylib could not be loaded, %s\n", dlerror());
        return 0;
    }

    // stuff pointer to my notifier
    NotifyProc* pointerAddress = (NotifyProc*)dlsym(handle, "gNotifer");
    if ( pointerAddress == NULL ) {
        printf("[FAIL]  dlclose-static-terminator: gNotifer not found in libterm.dylib\n");
        return 0;
    }
    *pointerAddress = &termNotifyFunc;

    // unload dylib
    dlclose(handle);

    if ( termDidRun )
        printf("[PASS]  dlclose-static-terminator\n");
    else
        printf("[FAIL]  dlclose-static-terminator: terminator not run\n");

	return 0;
}

