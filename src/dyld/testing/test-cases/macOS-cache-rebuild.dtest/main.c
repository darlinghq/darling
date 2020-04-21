// BUILD_ONLY: MacOSX

// BUILD:  $CC main.c            -o $BUILD_DIR/rebuild-dyld-cache.exe

// RUN:  ./rebuild-dyld-cache.exe


#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>
#include <signal.h>
#include <spawn.h>
#include <errno.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <sys/types.h>

extern char** environ;

int main()
{
    printf("[BEGIN] macOS-cache-rebuild\n");

    const char* argv[] = { "/usr/bin/update_dyld_shared_cache", "-cache_dir", "/tmp/", NULL };
    pid_t child;
    int psResult = posix_spawn(&child, "/usr/bin/update_dyld_shared_cache", NULL, NULL, (char**)argv, environ);
    if ( psResult != 0 ) {
        printf("[FAIL]  macOS-cache-rebuild: posix_spawn failed, err=%d\n", psResult);
        return 0;
    }

    int childStatus;
    (void)wait4(child, &childStatus, 0, NULL);
    if (WIFEXITED(childStatus) == 0)
        printf("[FAIL]  macOS-cache-rebuild: update_dyld_shared_cache did not exit\n");
    else if (WEXITSTATUS(childStatus) != 0)
        printf("[FAIL]  macOS-cache-rebuild: update_dyld_shared_cache failed\n");
    else
        printf("[PASS]  macOS-cache-rebuild\n");

	return 0;
}

