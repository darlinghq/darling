
// BUILD:  $CC foo.c -dynamiclib -o $BUILD_DIR/libFoo.dylib -install_name $RUN_DIR/libFoo.dylib
// BUILD:  $CC main.c -o $BUILD_DIR/chained-fixups-many-binds.exe $BUILD_DIR/libFoo.dylib -Wl,-fixup_chains

// RUN:  ./chained-fixups-many-binds.exe

// Here's how to generate this monster
// ( for i in `seq 1 65000`; do echo "void foo$i() { }"; done ) > foo.c
// ( for i in `seq 1 65000`; do echo "extern void foo$i();"; done ) > foo.h
// ( for i in `seq 1 65000`; do echo "__attribute__((used)) void* use$i = (void*)&foo$i;"; done ) > uses.h

#include <stdio.h>

#include "foo.h"
#include "uses.h"

int main() {
	printf("[BEGIN] chained-fixups-many-binds\n");
	printf("[PASS] chained-fixups-many-binds\n");
	return 0;
}