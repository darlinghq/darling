
// BUILD:  cp bad.txt $BUILD_DIR/libnota.dylib
// BUILD:  $CC main.c  -o $BUILD_DIR/dlopen-bad-file.exe -DRUN_DIR="$RUN_DIR"

// RUN:  ./dlopen-bad-file.exe

#include <stdio.h>
#include <dlfcn.h>
#include <string.h>



int main()
{
    printf("[BEGIN] dlopen-bad-file\n");

    // try to dlopen() a text file
	void* handle = dlopen(RUN_DIR "/libnota.dylib", RTLD_FIRST);
	if ( handle != NULL ) {
        printf("[FAIL] dlopen-bad-file should have failed on non-mach-o file %s\n", RUN_DIR "/libnota.dylib");
		return 0;
	}
    const char* message = dlerror();
    if ( (strstr(message, "mach-o") == NULL) && (strstr(message, "too short") == NULL) ) {
        printf("dlerror: %s\n", message);
        printf("[FAIL] dlopen-bad-file dlerror() message did not contain 'mach-o'\n");
		return 0;
	}

    // try to dlopen() a directory
	handle = dlopen(RUN_DIR, RTLD_FIRST);
	if ( handle != NULL ) {
        printf("[FAIL] dlopen-bad-file should have failed on dir %s\n", RUN_DIR);
		return 0;
	}
    message = dlerror();
    if ( strstr(message, "not a file") == NULL ) {
        printf("dlerror: %s\n", message);
        printf("[FAIL] dlopen-bad-file dlerror() message did not contain 'not a file'\n");
		return 0;
	}

    printf("[PASS] dlopen-bad-file\n");

	return 0;
}

