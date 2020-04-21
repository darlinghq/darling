
// BUILD:  $CC target.c       -o $BUILD_DIR/target.exe
// BUILD:  $CC foo.c          -o $BUILD_DIR/libfoo.dylib -dynamiclib -install_name $RUN_DIR/libfoo.dylib
// BUILD:  $CC main.c         -o $BUILD_DIR/dyld_process_info_unload.exe
// BUILD:  $TASK_FOR_PID_ENABLE  $BUILD_DIR/dyld_process_info_unload.exe

// RUN:  $SUDO ./dyld_process_info_unload.exe $RUN_DIR/target.exe

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

struct task_and_pid {
    pid_t pid;
    task_t task;
};

static struct task_and_pid launchTest(const char* testProgPath, bool launchOtherArch, bool launchSuspended)
{
    posix_spawnattr_t attr = 0;
    if ( posix_spawnattr_init(&attr) != 0 ) {
        printf("[FAIL] dyld_process_info_unload posix_spawnattr_init()\n");
        exit(0);
    }
    if ( launchSuspended ) {
        if ( posix_spawnattr_setflags(&attr, POSIX_SPAWN_START_SUSPENDED) != 0 ) {
            printf("[FAIL] dyld_process_info_unload POSIX_SPAWN_START_SUSPENDED\n");
            exit(0);
        }
    }
    if ( launchOtherArch ) {
        size_t copied;
        if ( posix_spawnattr_setbinpref_np(&attr, 1, otherArch, &copied) != 0 ) {
           printf("[FAIL] dyld_process_info_unload posix_spawnattr_setbinpref_np()\n");
            exit(0);
        }
    }

    struct task_and_pid child;
    const char* argv[] = { testProgPath, NULL };
    int psResult = posix_spawn(&child.pid, testProgPath, NULL, &attr, (char**)argv, environ);
    if ( psResult != 0 ) {
        printf("[FAIL] dyld_process_info_unload posix_spawn(%s) failed, err=%d\n", testProgPath, psResult);
        exit(0);
    }
    if (posix_spawnattr_destroy(&attr) != 0) {
        printf("[FAIL] dyld_process_info_unload posix_spawnattr_destroy()\n");
        exit(0);
    }

    if ( task_for_pid(mach_task_self(), child.pid, &child.task) != KERN_SUCCESS ) {
        printf("[FAIL] dyld_process_info_unload task_for_pid()\n");
        kill(child.pid, SIGKILL);
        exit(0);
    }

    // wait until process is up and has suspended itself
    struct task_basic_info info;
    do {
        unsigned count = TASK_BASIC_INFO_COUNT;
        kern_return_t kr = task_info(child.task, TASK_BASIC_INFO, (task_info_t)&info, &count);
        sleep(1);
    } while ( info.suspend_count == 0 );

    return child;
}

static void killTest(struct task_and_pid tp) {
    int r = kill(tp.pid, SIGKILL);
    waitpid(tp.pid, &r, 0);
}

static bool alwaysGetImages(struct task_and_pid tp, bool launchedSuspended)
{
    int failCount = 0;
    for (int i=0; i < 100; ++i ) {
        kern_return_t result;
        dyld_process_info info = _dyld_process_info_create(tp.task, 0, &result);
        //fprintf(stderr, "info=%p, result=%08X\n", info, result);
        if ( i == 0 )
            (void)kill(tp.pid, SIGCONT);
        if ( info == NULL ) {
            failCount++;
            //fprintf(stderr, "info=%p, result=%08X\n", info, result);
        }
        else {
            usleep(100);
            _dyld_process_info_release(info);
        }
    }
    // ideally the fail count would be zero.  But the target is dlopen/dlclosing in a tight loop, so there may never be a stable set of images.
    // The real bug driving this test case was _dyld_process_info_create() crashing when the the image list changed too fast.
    // The important thing is to not crash.  Getting NULL back is ok.
    return true;
}


int main(int argc, const char* argv[])
{
    printf("[BEGIN] dyld_process_info_unload\n");

    if ( argc < 2 ) {
        printf("[FAIL] dyld_process_info_unload missing argument\n");
        exit(0);
    }
    const char* testProgPath = argv[1];
    struct task_and_pid child;

    // launch test program suspended
    child = launchTest(testProgPath, false, true);
    if ( ! alwaysGetImages(child, true) ) {
        killTest(child);
        exit(0);
    }
    killTest(child);

    printf("[PASS] dyld_process_info_unload\n");
	return 0;
}
