
// BUILD:  $CC foo.c        -dynamiclib -o $BUILD_DIR/libfoo.dylib         -install_name $RUN_DIR/libfoo.dylib
// BUILD:  $CC interposer.c -dynamiclib -o $BUILD_DIR/libmyinterpose.dylib -install_name $RUN_DIR/libmyinterpose.dylib $BUILD_DIR/libfoo.dylib
// BUILD:  $CC main.c $BUILD_DIR/libfoo.dylib -o $BUILD_DIR/resolver-only.exe
// BUILD:  $CC main.c $BUILD_DIR/libfoo.dylib -o $BUILD_DIR/interposed-resolver.exe -DINTERPOSED $BUILD_DIR/libmyinterpose.dylib

// RUN:  ./resolver-only.exe
// RUN:  ./interposed-resolver.exe


#include <stdio.h>
#include <stdlib.h>

extern int foo();
int (*pFoo)() = &foo;

int main()
{
#if INTERPOSED
    printf("[BEGIN] interpose-resolver\n");
    if ( foo() != 11 )
        printf("[FAIL]  interpose-resolver: foo() != 11\n");
    else if ( (*pFoo)() != 11 )
        printf("[FAIL]  interpose-resolver: *pFoo() != 11\n");
    else
        printf("[PASS]  interpose-resolver\n");
#else
    printf("[BEGIN] resolver-only\n");
    if ( foo() != 10 )
        printf("[FAIL]  resolver-only: foo() != 10\n");
    else if ( (*pFoo)() != 10 )
        printf("[FAIL]  resolver-only: *pFoo() != 10\n");
    else
        printf("[PASS]  resolver-only\n");
#endif
	return 0;
}
