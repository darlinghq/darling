
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>

static void* bazHandle = NULL;
static void* bazSymbol = NULL;
static int barInited = 0;
static int bazInited = 0;

__attribute__((constructor))
static void myinit()
{
	barInited = 1;
	bazHandle = dlopen(RUN_DIR "/libbaz.dylib", 0);
	if ( bazHandle == NULL ) {
		printf("[FAIL]  dlopen-in-init3, dlopen libbaz.dylib: %s\n", dlerror());
		return;
	}
	bazSymbol = dlsym(RTLD_DEFAULT, "bazIsInited");
    if ( bazSymbol == NULL ) {
		printf("[FAIL]  dlopen-in-init3, dlsym libbaz.dylib\n");
        return;
    }
    bazInited = ((int(*)())bazSymbol)();
}

int bar() {
	if ( barInited == 0 ) {
		printf("[FAIL]  dlopen-in-init3, didn't init bar\n");
		return 1;
	}
	if ( bazHandle == NULL ) {
		return 1;
	}
	if ( bazSymbol == NULL ) {
		return 1;
	}
	if ( bazInited == 0 ) {
		printf("[FAIL]  dlopen-in-init3, didn't init bar\n");
		return 1;
	}
	return 0;
}