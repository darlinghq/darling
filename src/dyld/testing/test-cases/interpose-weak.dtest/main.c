// BUILD:  $CC foo.c -dynamiclib -o $BUILD_DIR/libfoo.dylib -install_name $RUN_DIR/libfoo.dylib
// BUILD:  $CC main.c $BUILD_DIR/libfoo.dylib -o $BUILD_DIR/interpose-weak-present.exe
// BUILD:  $DYLD_ENV_VARS_ENABLE $BUILD_DIR/interpose-weak-present.exe
// BUILD:  $CC interposer.c -dynamiclib $BUILD_DIR/libfoo.dylib -o $BUILD_DIR/libinterposer.dylib -install_name libinterposer.dylib

// BUILD:  $CC foo.c            -dynamiclib -o $TEMP_DIR/libfoo2.dylib  -install_name $RUN_DIR/libfoo2.dylib
// BUILD:  $CC foo.c -DNO_FOO34 -dynamiclib -o $BUILD_DIR/libfoo2.dylib -install_name $RUN_DIR/libfoo2.dylib
// BUILD:  $CC main.c -DNO_FOO34  $TEMP_DIR/libfoo2.dylib -o $BUILD_DIR/interpose-weak-missing.exe
// BUILD:  $DYLD_ENV_VARS_ENABLE $BUILD_DIR/interpose-weak-missing.exe
// BUILD:  $CC interposer.c -dynamiclib $TEMP_DIR/libfoo2.dylib -o $BUILD_DIR/libinterposer2.dylib -install_name libinterposer.dylib


// RUN:  DYLD_INSERT_LIBRARIES=libinterposer.dylib   ./interpose-weak-present.exe
// RUN:  DYLD_INSERT_LIBRARIES=libinterposer2.dylib  ./interpose-weak-missing.exe



#include <stdio.h>
#include <stdlib.h>
#include <string.h>


extern int foo1();
extern int foo2();
extern int foo3() __attribute__((weak_import));
extern int foo4() __attribute__((weak_import));

#ifndef NO_FOO34
    #define MODE "present"
#else
    #define MODE "missing"
#endif

int main()
{
    printf("[BEGIN] interpose-weak-" MODE "\n");

	if ( foo1() != 1 ) {
        printf("[FAIL] interpose-weak-" MODE ", foo1() != 1\n");
        return 0;
    }

	if ( foo2() != 12 ) {
        printf("[FAIL] interpose-weak-" MODE ", foo2() != 12\n");
        return 0;
    }

#ifndef NO_FOO34
	if ( foo3() != 3 ) {
        printf("[FAIL] interpose-weak-" MODE ", foo3() != 3\n");
        return 0;
    }

	if ( foo4() != 14 ) {
        printf("[FAIL] interpose-weak-" MODE ", foo4() != 14\n");
        return 0;
    }
#else
	if ( &foo3 != NULL ) {
        printf("[FAIL] interpose-weak-" MODE ", &foo3 != NULL\n");
        return 0;
    }

	if ( &foo4 != NULL ) {
        printf("[FAIL] interpose-weak-" MODE ", &foo4 != NULL\n");
        return 0;
    }
#endif

    printf("[PASS] interpose-weak-" MODE "\n");
	return 0;
}
