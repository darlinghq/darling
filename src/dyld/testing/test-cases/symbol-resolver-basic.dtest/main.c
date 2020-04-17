
// BUILD:  $CC foo.c foo2.c -dynamiclib  -install_name $RUN_DIR/libfoo.dylib -o $BUILD_DIR/libfoo.dylib
// BUILD:  $CC main.c $BUILD_DIR/libfoo.dylib -o $BUILD_DIR/symbol-resolver.exe
// BUILD:  $CC foo.c foo2.c -dynamiclib -DTEN=1 -install_name $RUN_DIR/libfoo10.dylib -o $BUILD_DIR/libfoo10.dylib 
// BUILD:  $CC main.c $BUILD_DIR/libfoo10.dylib -DTEN=1 -o $BUILD_DIR/symbol-resolver10.exe

// RUN:  ./symbol-resolver.exe
// RUN:  ./symbol-resolver10.exe


#include <stdio.h>
#include <stdlib.h>

extern int foo();
extern int fooPlusOne();


int main()
{
#if TEN
    if ( foo() != 10 )
        printf("[FAIL] symbol-resolver-basic: foo() != 10\n");
    else if ( fooPlusOne() != 11 )
        printf("[FAIL] symbol-resolver-basic: fooPlusOne() != 11\n");
    else
        printf("[PASS] symbol-resolver-basic\n");
#else
    if ( foo() != 0 )
        printf("[FAIL] symbol-resolver-basic: foo() != 0\n");
    else if ( fooPlusOne() != 1 )
        printf("[FAIL] symbol-resolver-basic: fooPlusOne() != 1\n");
    else
        printf("[PASS] symbol-resolver-basic\n");
#endif
  
	return 0;
}
