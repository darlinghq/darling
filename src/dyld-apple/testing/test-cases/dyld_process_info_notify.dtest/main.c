
// BUILD:  $CC target.c      -o $BUILD_DIR/target.exe
// BUILD:  $CC foo.c         -o $BUILD_DIR/libfoo.dylib -dynamiclib
// BUILD:  $CC main.c        -o $BUILD_DIR/dyld_process_info_notify.exe
// BUILD:  $TASK_FOR_PID_ENABLE $BUILD_DIR/dyld_process_info_notify.exe

// RUN_TIMEOUT: 2400
// RUN:  $SUDO ./dyld_process_info_notify.exe  $RUN_DIR/target.exe

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
#include <dispatch/dispatch.h>


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

static task_t launchTest(const char* testProgPath, const char* arg1, bool launchOtherArch, bool launchSuspended)
{
    //fprintf(stderr, "launchTest() launchOtherArch=%d, launchSuspended=%d, arg=%s\n", launchOtherArch, launchSuspended, arg1);
    posix_spawnattr_t attr;
    if ( posix_spawnattr_init(&attr) != 0 ) {
        printf("[FAIL] dyld_process_info_notify posix_spawnattr_init()\n");
        exit(0);
    }
    if ( launchSuspended ) {
        if ( posix_spawnattr_setflags(&attr, POSIX_SPAWN_START_SUSPENDED) != 0 ) {
            printf("[FAIL] dyld_process_info_notify POSIX_SPAWN_START_SUSPENDED\n");
            exit(0);
        }
    }
    if ( launchOtherArch ) {
        size_t copied;
        if ( posix_spawnattr_setbinpref_np(&attr, 1, otherArch, &copied) != 0 ) {
           printf("[FAIL] dyld_process_info_notify posix_spawnattr_setbinpref_np()\n");
            exit(0);
        }
    }

    pid_t childPid;
    const char* argv[] = { testProgPath, arg1, NULL };
    int psResult = posix_spawn(&childPid, testProgPath, NULL, &attr, (char**)argv, environ);
    if ( psResult != 0 ) {
        printf("[FAIL] dyld_process_info_notify posix_spawn(%s) failed, err=%d\n", testProgPath, psResult);
        exit(0);
    }
    //printf("child pid=%d\n", childPid);

    task_t childTask = 0;
    if ( task_for_pid(mach_task_self(), childPid, &childTask) != KERN_SUCCESS ) {
        printf("[FAIL] dyld_process_info_notify task_for_pid()\n");
        kill(childPid, SIGKILL);
        exit(0);
    }

    return childTask;
}

static void wait_util_task_suspended(task_t task)
{
   struct task_basic_info info;
    do {
        unsigned count = TASK_BASIC_INFO_COUNT;
        kern_return_t kr = task_info(task, TASK_BASIC_INFO, (task_info_t)&info, &count);
        //fprintf(stderr, "task_info() => %d, suspendCount=%d\n", kr, info.suspend_count);
        sleep(1);
    } while ( info.suspend_count == 0 );
}


static bool monitor(task_t task, bool disconnectEarly, bool attachLate)
{
    kern_return_t kr;
    __block bool sawMainExecutable = false;
    __block bool sawlibSystem = false;
    __block bool gotTerminationNotice = false;
    __block bool gotEarlyNotice = false;
    __block int libFooLoadCount = 0;
    __block int libFooUnloadCount = 0;
    dispatch_semaphore_t taskDone = dispatch_semaphore_create(0);

    dispatch_queue_t serviceQueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0);

    unsigned count = 0;
    dyld_process_info_notify handle;
    do {
        handle = _dyld_process_info_notify(task, serviceQueue,
                                          ^(bool unload, uint64_t timestamp, uint64_t machHeader, const uuid_t uuid, const char* path) {
                                            if ( strstr(path, "/target.exe") != NULL )
                                                sawMainExecutable = true;
                                            if ( strstr(path, "/libSystem") != NULL )
                                                sawlibSystem = true;
                                            if ( strstr(path, "/libfoo.dylib") != NULL ) {
                                                if ( unload )
                                                    ++libFooUnloadCount;
                                                else
                                                    ++libFooLoadCount;
                                                if ( disconnectEarly ) {
                                                    gotEarlyNotice = true;
                                                    dispatch_semaphore_signal(taskDone);
                                                }
                                            }
                                            //fprintf(stderr, "unload=%d, 0x%012llX <%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X> %s\n",
                                            //   unload, machHeader, uuid[0],  uuid[1],  uuid[2],  uuid[3],  uuid[4],  uuid[5],  uuid[6],  uuid[7],
                                            //                        uuid[8],  uuid[9],  uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15], path);
                                          },
                                          ^{
                                            //fprintf(stderr, "target exited\n");
                                            gotTerminationNotice = true;
                                            dispatch_semaphore_signal(taskDone);
                                          },
                                          &kr);
        ++count;
        if ( handle == NULL )
            fprintf(stderr, "_dyld_process_info_notify() returned NULL, result=%d, count=%d\n", kr, count);
     } while ( (handle == NULL) && (count < 5) );

    if ( handle == NULL ) {
        return false;
    }

    // if process suspends itself, wait until it has done so
    if ( attachLate )
        wait_util_task_suspended(task);

    // resume from initial suspend
    task_resume(task);

    // block waiting for notification that target has exited
    bool gotSignal = (dispatch_semaphore_wait(taskDone, dispatch_time(DISPATCH_TIME_NOW, 10LL * NSEC_PER_SEC)) == 0);
    _dyld_process_info_notify_release(handle);


    if ( !gotSignal ) {
        fprintf(stderr, "did not get exit signal\n");
        return false;
    }

    if ( !attachLate && !sawMainExecutable ) {
        fprintf(stderr, "did not get load notification of main executable\n");
        return false;
    }

    if ( !attachLate && !sawlibSystem ) {
        fprintf(stderr, "did not get load notification of libSystem\n");
        return false;
    }

    if ( disconnectEarly ) {
        if ( libFooLoadCount != 1 ) {
            fprintf(stderr, "got %d load notifications about libFoo instead of 1\n", libFooLoadCount);
            return false;
        }
        if ( libFooUnloadCount != 0 ) {
            fprintf(stderr, "got %d unload notifications about libFoo instead of 1\n", libFooUnloadCount);
            return false;
        }
    }
    else {
        if ( libFooLoadCount != 3 ) {
            fprintf(stderr, "got %d load notifications about libFoo instead of 3\n", libFooLoadCount);
            return false;
        }
        if ( libFooUnloadCount != 3 ) {
            fprintf(stderr, "got %d unload notifications about libFoo instead of 3\n", libFooUnloadCount);
            return false;
        }
    }

    return true;
}

static void validateMaxNotifies(task_t task)
{
    dispatch_queue_t serviceQueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0);
    dyld_process_info_notify handles[10];
    for (int i=0; i < 10; ++i) {
        kern_return_t kr;
        handles[i] = _dyld_process_info_notify(task, serviceQueue,
                                          ^(bool unload, uint64_t timestamp, uint64_t machHeader, const uuid_t uuid, const char* path) {
                                            //fprintf(stderr, "unload=%d, 0x%012llX <%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X> %s\n",
                                            //   unload, machHeader, uuid[0],  uuid[1],  uuid[2],  uuid[3],  uuid[4],  uuid[5],  uuid[6],  uuid[7],
                                            //                        uuid[8],  uuid[9],  uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15], path);
                                          },
                                          ^{
                                            //fprintf(stderr, "target exited\n");
                                          },
                                          &kr);
        if ( handles[i] == NULL ) {
            if ( i == 8 ) {
                // expected failure, because only 8 simultaneous connections allowed
                // release one and try again
                _dyld_process_info_notify_release(handles[4]);
                handles[4] = NULL;
            }
            else {
                fprintf(stderr, "_dyld_process_info_notify() returned NULL and kern_result=%d, on count=%d\n", kr, i);
                task_terminate(task);
                exit(0);
            }
        }
    }
    // release all
    for (int i=0; i < 10; ++i) {
        if ( handles[i] != NULL ) {
            _dyld_process_info_notify_release(handles[i]);
        }
    }
    dispatch_release(serviceQueue);
}

int main(int argc, const char* argv[])
{
    printf("[BEGIN] dyld_process_info_notify\n");
    if ( argc < 2 ) {
        printf("[FAIL] dyld_process_info_notify missing argument\n");
        exit(0);
    }
    const char* testProgPath = argv[1];

    dispatch_async(dispatch_get_main_queue(), ^{
        task_t childTask;

        // test 1) launch test program suspended in same arch as this program
        childTask = launchTest(testProgPath, "", false, true);
        if ( ! monitor(childTask, false, false) ) {
            printf("[FAIL] dyld_process_info_notify launch suspended missed some notifications\n");
            task_terminate(childTask);
            exit(0);
        }
        task_terminate(childTask);

        // test 2) launch test program in same arch as this program where it sleeps itself
        childTask = launchTest(testProgPath, "suspend-in-main", false, false);
        validateMaxNotifies(childTask);
        if ( ! monitor(childTask, false, true) ) {
            printf("[FAIL] dyld_process_info_notify launch suspend-in-main missed some notifications\n");
            task_terminate(childTask);
            exit(0);
        }
        task_terminate(childTask);

#if !TARGET_OS_WATCH && !TARGET_OS_TV && __LP64__
        // test 3) launch test program suspended in opposite arch as this program
        childTask = launchTest(testProgPath, "", true, true);
        if ( ! monitor(childTask, false, false) ) {
            printf("[FAIL] dyld_process_info_notify launch suspended other arch missed some notifications\n");
            task_terminate(childTask);
            exit(0);
        }
        task_terminate(childTask);

        // test 4) launch test program in opposite arch as this program where it sleeps itself
        childTask = launchTest(testProgPath, "suspend-in-main", true, false);
        if ( ! monitor(childTask, false, true) ) {
            printf("[FAIL] dyld_process_info_notify launch other arch suspend-in-main missed some notifications\n");
            task_terminate(childTask);
            exit(0);
        }
        task_terminate(childTask);
#endif

        // test 5) launch test program where we disconnect from it after first dlopen
        childTask = launchTest(testProgPath, "", false, true);
        if ( ! monitor(childTask, true, false) ) {
            printf("[FAIL] dyld_process_info_notify connect/disconnect missed some notifications\n");
            task_terminate(childTask);
            exit(0);
        }
        task_terminate(childTask);

        printf("[PASS] dyld_process_info_notify\n");
        exit(0);
    });

    dispatch_main();
}
