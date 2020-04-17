// BUILD_ONLY: MacOSX

// BUILD:  $CC a.c -dynamiclib -arch x86_64h -o $BUILD_DIR/libHaswellCheck.dylib -install_name $RUN_DIR/libHaswellCheck.dylib
// BUILD:  $CC main.c -o $BUILD_DIR/dlopen-haswell.exe -DRUN_DIR="$RUN_DIR"

// RUN:  ./dlopen-haswell.exe


#include <stdbool.h>
#include <stdio.h>
#include <dlfcn.h>
#include <mach/host_info.h>
#include <mach/mach.h>
#include <mach/mach_host.h>

typedef bool (*BoolFunc)(void);


bool isHaswell_dynamic()
{
    struct host_basic_info info;
    mach_msg_type_number_t count = HOST_BASIC_INFO_COUNT;
    mach_port_t hostPort = mach_host_self();
    kern_return_t result = host_info(hostPort, HOST_BASIC_INFO, (host_info_t)&info, &count);
    mach_port_deallocate(mach_task_self(), hostPort);
    if ( result == KERN_SUCCESS ) {
        return (info.cpu_subtype == CPU_SUBTYPE_X86_64_H);
    }
    return false;
}


int main(int arg, const char* argv[])
{
    printf("[BEGIN] dlopen-haswell\n");

    void* handle = dlopen(RUN_DIR "/libHaswellCheck.dylib", RTLD_LAZY);
    if ( handle == NULL ) {
        printf("dlerror(): %s\n", dlerror());
        printf("[FAIL] dlopen-haswell dlopen\n");
        return 0;
    }

    BoolFunc libFunc = (BoolFunc)dlsym(handle, "isHaswell");
    if ( libFunc == NULL ) {
        printf("dlerror(): %s\n", dlerror());
        printf("[FAIL] dlopen-haswell dlsym\n");
        return 0;
    }

    // check if haswell slice of libHaswellCheck.dylib was loaded on haswell machines
    bool dylibIsHaswellSlice = (*libFunc)();
    bool runtimeIsHaswell = isHaswell_dynamic();

	if ( dylibIsHaswellSlice != runtimeIsHaswell )
        printf("[FAIL] dlopen-haswell, dylibIsHaswellSlice=%d, runtimeIsHaswell=%d\n", dylibIsHaswellSlice, runtimeIsHaswell);
	else
        printf("[PASS] dlopen-haswell\n");

	return 0;
}



