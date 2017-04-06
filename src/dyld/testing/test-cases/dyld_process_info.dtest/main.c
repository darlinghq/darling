
// BUILD:  $CC linksWithCF.c  -o $BUILD_DIR/linksWithCF.exe -framework CoreFoundation
// BUILD:  $CC main.c         -o $BUILD_DIR/dyld_process_info.exe
// BUILD:  $TASK_FOR_PID_ENABLE  $BUILD_DIR/dyld_process_info.exe

// RUN:  $SUDO ./dyld_process_info.exe $RUN_DIR/linksWithCF.exe

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <signal.h>
#include <spawn.h>
#include <errno.h>
#include <mach/mach.h>
#include <mach/machine.h>
#include <mach-o/dyld_process_info.h>


extern char** environ;

#if __x86_64__
    cpu_type_t otherArch[] = { CPU_TYPE_I386 };
#elif __i386__
    cpu_type_t otherArch[] = { CPU_TYPE_X86_64 };
#elif __arm64__
    cpu_type_t otherArch[] = { CPU_TYPE_ARM };
#elif __arm__
    cpu_type_t otherArch[] = { CPU_TYPE_ARM64 };
#endif

static task_t launchTest(const char* testProgPath, bool launchOtherArch, bool launchSuspended)
{
    posix_spawnattr_t attr;
    if ( posix_spawnattr_init(&attr) != 0 ) {
        printf("[FAIL] dyld_process_info posix_spawnattr_init()\n");
        exit(0);
    }
    if ( launchSuspended ) {
        if ( posix_spawnattr_setflags(&attr, POSIX_SPAWN_START_SUSPENDED) != 0 ) {
            printf("[FAIL] dyld_process_info POSIX_SPAWN_START_SUSPENDED\n");
            exit(0);
        }
    }
    if ( launchOtherArch ) {
        size_t copied;
        if ( posix_spawnattr_setbinpref_np(&attr, 1, otherArch, &copied) != 0 ) {
           printf("[FAIL] dyld_process_info posix_spawnattr_setbinpref_np()\n");
            exit(0);
        }
    }

    pid_t childPid;
    const char* argv[] = { testProgPath, NULL };
    int psResult = posix_spawn(&childPid, testProgPath, NULL, &attr, (char**)argv, environ);
    if ( psResult != 0 ) {
        printf("[FAIL] dyld_process_info posix_spawn(%s) failed, err=%d\n", testProgPath, psResult);
        exit(0);
    }
    //printf("child pid=%d\n", childPid);

    task_t childTask = 0;
    if ( task_for_pid(mach_task_self(), childPid, &childTask) != KERN_SUCCESS ) {
        printf("[FAIL] dyld_process_info task_for_pid()\n");
        kill(childPid, SIGKILL);
        exit(0);
    }

    // wait until process is up and has suspended itself
    struct task_basic_info info;
    do {
        unsigned count = TASK_BASIC_INFO_COUNT;
        kern_return_t kr = task_info(childTask, TASK_BASIC_INFO, (task_info_t)&info, &count);
        sleep(1);
    } while ( info.suspend_count == 0 );

    return childTask;
}

static bool hasCF(task_t task, bool launchedSuspended)
{
    kern_return_t result;
    dyld_process_info info = _dyld_process_info_create(task, 0, &result);
    if ( info == NULL ) {
        printf("[FAIL] dyld_process_info _dyld_process_info_create(), kern_return_t=%d\n", result);
        return false;
    }

    dyld_process_state_info stateInfo;
    _dyld_process_info_get_state(info, &stateInfo);
    bool valueSaysLaunchedSuspended = (stateInfo.dyldState == dyld_process_state_not_started);
    if ( valueSaysLaunchedSuspended != launchedSuspended ) {
        printf("[FAIL] dyld_process_info suspend state mismatch\n");
        return false;
    }

    __block bool foundDyld = false;
    _dyld_process_info_for_each_image(info, ^(uint64_t machHeaderAddress, const uuid_t uuid, const char* path) {
        //fprintf(stderr, "0x%llX %s\n", machHeaderAddress, path);
        if ( strstr(path, "/usr/lib/dyld") != NULL )
            foundDyld = true;
    });

    if ( launchedSuspended ) {
        // fprintf(stderr, "launched suspended image list:\n");
        __block bool foundMain = false;
        _dyld_process_info_for_each_image(info, ^(uint64_t machHeaderAddress, const uuid_t uuid, const char* path) {
            //fprintf(stderr, "0x%llX %s\n", machHeaderAddress, path);
            if ( strstr(path, "/linksWithCF.exe") != NULL )
                foundMain = true;
       });
        return foundMain && foundDyld;
    }

    __block bool foundCF = false;
    _dyld_process_info_for_each_image(info, ^(uint64_t machHeaderAddress, const uuid_t uuid, const char* path) {
        //fprintf(stderr, "0x%llX %s\n", machHeaderAddress, path);
        if ( strstr(path, "/CoreFoundation.framework/") != NULL )
            foundCF = true;
    });

    _dyld_process_info_release(info);

    return foundCF && foundDyld;
}


int main(int argc, const char* argv[])
{
    printf("[BEGIN] dyld_process_info\n");

    if ( argc < 2 ) {
        printf("[FAIL] dyld_process_info missing argument\n");
        exit(0);
    }
    const char* testProgPath = argv[1];
    task_t childTask;

    // launch test program same arch as this program
    childTask = launchTest(testProgPath, false, false);
    if ( ! hasCF(childTask, false) ) {
        printf("[FAIL] dyld_process_info same arch does not link with CF and dyld\n");
        task_terminate(childTask);
        exit(0);
    }
    task_terminate(childTask);
    
    // launch test program suspended
    childTask = launchTest(testProgPath, false, true);
    if ( ! hasCF(childTask, true) ) {
        printf("[FAIL] dyld_process_info suspended does not link with CF and dyld\n");
        task_terminate(childTask);
        exit(0);
    }
    task_resume(childTask);
    task_terminate(childTask);

#if !TARGET_OS_WATCH && !TARGET_OS_TV && __LP64__
    // on 64/32 devices, run test program as other arch too
    childTask = launchTest(testProgPath, true, false);
    if ( ! hasCF(childTask, false) ) {
        printf("[FAIL] dyld_process_info other arch does not link with CF and dyld\n");
        task_terminate(childTask);
        exit(0);
    }
    task_terminate(childTask);
#endif

    // verify this program does not use CF
    if ( hasCF(mach_task_self(), false) ) {
        printf("[FAIL] dyld_process_info self links with CF and dyld\n");
        exit(0);
    }

    printf("[PASS] dyld_process_info\n");
	return 0;
}
