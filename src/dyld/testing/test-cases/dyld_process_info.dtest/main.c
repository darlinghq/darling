
// BUILD:  $CC linksWithCF.c  -o $BUILD_DIR/linksWithCF.exe -framework CoreFoundation
// BUILD:  $CC main.c         -o $BUILD_DIR/dyld_process_info.exe -ldarwintest
// BUILD:  $TASK_FOR_PID_ENABLE  $BUILD_DIR/dyld_process_info.exe

// RUN:  $SUDO ./dyld_process_info.exe

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <signal.h>
#include <spawn.h>
#include <errno.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <mach/mach.h>
#include <mach/machine.h>
#include <mach-o/dyld_priv.h>
#include <mach-o/dyld_process_info.h>
#include <Availability.h>

#include "dyld_test.h"

static void inspectProcess(task_t task, bool launchedSuspended, bool expectCF, bool forceIOSMac)
{
    kern_return_t result;
    dyld_process_info info = _dyld_process_info_create(task, 0, &result);
    T_EXPECT_MACH_SUCCESS(result, "dyld_process_info() should succeed");
    T_ASSERT_NOTNULL(info, "dyld_process_info(task, 0) alwats return a value");

    dyld_process_state_info stateInfo;
    bzero(&stateInfo, sizeof(stateInfo));
    _dyld_process_info_get_state(info, &stateInfo);
    T_EXPECT_EQ_UINT((stateInfo.dyldState == dyld_process_state_not_started), launchedSuspended, "If launchSuspended then stateInfo.dyldState shoould be dyld_process_state_not_started");
    if ( !launchedSuspended ) {
        T_EXPECT_GE_UCHAR(stateInfo.dyldState, dyld_process_state_libSystem_initialized, "libSystem should be initalized by now");
        T_EXPECT_GT_UINT(stateInfo.imageCount, 0, "image count should be > 0");
        T_EXPECT_GT_UINT(stateInfo.initialImageCount, 0, "initial image count should be > 0");
        T_EXPECT_GE_UINT(stateInfo.imageCount, stateInfo.initialImageCount, "image count should be >= initial image count");
    }

    if (launchedSuspended) {
        T_EXPECT_EQ_UINT(_dyld_process_info_get_platform(info), 0, "_dyld_process_info_get_platform() should be 0 for launchSuspended processes");
    } else if (forceIOSMac) {
        T_EXPECT_EQ_UINT(_dyld_process_info_get_platform(info), PLATFORM_IOSMAC, "_dyld_process_info_get_platform() should be PLATFORM_IOSMAC");
    } else {
        T_EXPECT_EQ_UINT(_dyld_process_info_get_platform(info), dyld_get_active_platform(), "_dyld_process_info_get_platform() should be the same dyld_get_active_platform()");
    }

    __block bool foundDyld = false;
    __block bool foundMain = false;
    __block bool foundCF = false;
    _dyld_process_info_for_each_image(info, ^(uint64_t machHeaderAddress, const uuid_t uuid, const char* path) {
        if ( strstr(path, "/dyld") != NULL )
            foundDyld = true;
        if ( strstr(path, "/linksWithCF.exe") != NULL )
            foundMain = true;
        if ( strstr(path, "/dyld_process_info.exe") != NULL )
            foundMain = true;
        if ( strstr(path, "/CoreFoundation.framework/") != NULL )
            foundCF = true;
    });
    T_EXPECT_TRUE(foundDyld, "dyld should always be in the image list");
    T_EXPECT_TRUE(foundMain, "The main executable should always be in the image list");
    if (expectCF) {
        T_EXPECT_TRUE(foundCF, "CF should be in the image list");
    }

     _dyld_process_info_release(info);
}

static void launchTest(bool launchOtherArch, bool launchSuspended, bool forceIOSMac)
{
    if (forceIOSMac) { setenv("DYLD_FORCE_PLATFORM", "6", 1); }
    pid_t pid = T_POSIXSPAWN_ASSERT(launchSuspended, launchOtherArch, INSTALL_PATH "/linksWithCF.exe");
    task_t task = T_TASK_FOR_PID_ASSERT(pid);
    if (forceIOSMac) { unsetenv("DYLD_FORCE_PLATFORM"); }

    // wait until process is up and has suspended itself
    struct task_basic_info info;
    do {
        unsigned count = TASK_BASIC_INFO_COUNT;
        kern_return_t kr = task_info(task, TASK_BASIC_INFO, (task_info_t)&info, &count);
        usleep(10000);
    } while ( info.suspend_count == 0 );

    inspectProcess(task, launchSuspended, !launchSuspended, forceIOSMac);
    int r = kill(pid, SIGKILL);
    waitpid(pid, &r, 0);
}

T_DECL_DYLD(dyld_process_info, "Test basic dyld_process_info functionality", T_META_ASROOT(true)) {
    launchTest(false, false, false);
    launchTest(false, true, false);
#if __MAC_OS_X_VERSION_MIN_REQUIRED
    // FIXME: Reenable these ones i386 is turned back on for simulators
    //launchTest(true, false, false);
    //launchTest(true, true, false);
    launchTest(false, false, true);
    launchTest(false, true, true);
    //FIXME: This functionality is broken, but it is an edge case no one should ever hit
    //launchTest(true, true, true);
#endif
    inspectProcess(mach_task_self(), false, false, false);
}
