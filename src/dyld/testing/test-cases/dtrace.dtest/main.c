// BUILD_ONLY: MacOSX

// if we ever re-enable this on iOS we will need to add // BOOT_ARGS: dtrace_dof_mode=1

// BUILD:  /usr/sbin/dtrace -h -s main.d -o $TEMP_DIR/probes.h
// BUILD:  $CC main.c -I$TEMP_DIR -o $BUILD_DIR/dtrace.exe
// BUILD:  $DYLD_ENV_VARS_ENABLE $BUILD_DIR/dtrace.exe

// RUN:    $SUDO dtrace -l -n 'dyld_testing*:dtrace.exe:main:callback' -c ./dtrace.exe



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sdt.h>

#include "probes.h"

int main()
{
    printf("[BEGIN] dtrace\n");

    DYLD_TESTING_CALLBACK();

    if (!DYLD_TESTING_CALLBACK_ENABLED())
        printf("[FAIL] dtrace: DYLD_TESTING_CALLBACK_ENABLED() returned false\n");
    else
        printf("[PASS] dtrace\n");

	return 0;
}
