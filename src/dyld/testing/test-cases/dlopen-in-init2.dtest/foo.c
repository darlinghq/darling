
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>

extern int bar();
extern int bazInited();

static void* barHandle = NULL;
static void* barSymbol = NULL;
static int fooInited = 0;
static int barInited = 0;

__attribute__((constructor))
static void myinit()
{
	fooInited = 1;
	barHandle = dlopen(RUN_DIR "/libbar.dylib", 0);
	if ( barHandle == NULL ) {
		printf("[FAIL]  dlopen-in-init2, dlopen libbar.dylib: %s\n", dlerror());
		return;
	}
	barSymbol = dlsym(RTLD_DEFAULT, "barIsInited");
    if ( barSymbol == NULL ) {
		printf("[FAIL]  dlopen-in-init2, dlsym libbar.dylib\n");
        return;
    }
    barInited = ((int(*)())barSymbol)();
}

int foo() {
	if ( fooInited == 0 ) {
		printf("[FAIL]  dlopen-in-init2, didn't init foo\n");
		return 1;
	}
	if ( barHandle == NULL ) {
		return 1;
	}
	if ( barSymbol == NULL ) {
		return 1;
	}
	if ( barInited == 0 ) {
		printf("[FAIL]  dlopen-in-init2, didn't init bar\n");
		return 1;
	}
	if ( bazInited() == 0 ) {
		printf("[FAIL]  dlopen-in-init2, didn't init baz\n");
		return 1;
	}
	return 0;
}