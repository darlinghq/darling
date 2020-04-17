
// BUILD:  $CC main.c -o $BUILD_DIR/dyld_fork_test.exe

// RUN:  ./dyld_fork_test.exe 

#include <stdio.h>
#include <dlfcn.h>
#include <mach-o/dyld_priv.h>

bool isParent = true;

static void notifyBeforeFork(const struct mach_header* mh, intptr_t vmaddr_slide)
{
    static bool sRanNotifier = false;
    if (sRanNotifier)
        return;
    sRanNotifier = true;

    // fork and exec child
    pid_t sChildPid = fork();
    if ( sChildPid < 0 ) {
        printf("[FAIL] dyld_fork_test didn't fork\n");
        return;
    }
    if ( sChildPid == 0 ) {
        // child side
        isParent = false;
    }
}

int main(int argc, const char* argv[])
{
    printf("[BEGIN] dyld_fork_test\n");

    _dyld_register_func_for_add_image(&notifyBeforeFork);

    if (isParent) {
        printf("[PASS] dyld_fork_test\n");
    }

    return 0;
}