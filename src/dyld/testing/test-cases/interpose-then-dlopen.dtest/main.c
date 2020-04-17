// BUILD:  $CC fooimpl.c -dynamiclib -o $BUILD_DIR/libfooimpl.dylib -install_name $RUN_DIR/libfooimpl.dylib
// BUILD:  $CC foo.c -dynamiclib $BUILD_DIR/libfooimpl.dylib -o $BUILD_DIR/libfoo.dylib -Wl,-interposable_list,interposable.txt -install_name $RUN_DIR/libfoo.dylib
// BUILD:  $CC bar.c -bundle     $BUILD_DIR/libfooimpl.dylib -o $BUILD_DIR/libbar.bundle -Wl,-interposable_list,interposable.txt
// BUILD:  $CC main.c -DRUN_DIR="$RUN_DIR" -Wl,-interposable_list,interposable.txt -o $BUILD_DIR/interpose-then-dlopen.exe
// BUILD:  $DYLD_ENV_VARS_ENABLE $BUILD_DIR/interpose-then-dlopen.exe
// BUILD:  $CC interposer.c -dynamiclib $BUILD_DIR/libfooimpl.dylib -o $BUILD_DIR/libinterposer.dylib -install_name libinterposer.dylib

// RUN:    DYLD_INSERT_LIBRARIES=libinterposer.dylib   ./interpose-then-dlopen.exe



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

// Note, libinterposer.dylib interposes interposableFoo
extern int interposableFoo();

// Note, libfoo interposes interposableBar
extern int interposableBar();

extern int callFunc();


static int tryImage(const char* path, int expectedFoo, int expectedBar)
{
  void* handle = dlopen(path, RTLD_LAZY);
  if ( handle == NULL ) {
    printf("dlerror(): %s\n", dlerror());
    printf("[FAIL] interpose-then-dlopen %s\n", path);
    return 1;
  }
  
  __typeof(&callFunc) callFooSym = (__typeof(&callFunc))dlsym(handle, "callFoo");
  if ( callFooSym == NULL ) {
    printf("dlerror(): %s\n", dlerror());
    printf("[FAIL] interpose-then-dlopen %s\n", path);
    return 1;
  }
  
  int fooResult = callFooSym();
  if ( fooResult != expectedFoo ) {
    printf("[FAIL] interpose-then-dlopen callFoo() from %s not interposed as it returned %d\n", path, fooResult);
    return 1;
  }
  
  __typeof(&callFunc) callBarSym = (__typeof(&callFunc))dlsym(handle, "callBar");
  if ( callBarSym == NULL ) {
    printf("dlerror(): %s\n", dlerror());
    printf("[FAIL] interpose-then-dlopen %s\n", path);
    return 1;
  }
  
  int barResult = callBarSym();
  if ( barResult != expectedBar ) {
    printf("[FAIL] interpose-then-dlopen callBar() from %s not interposed as it returned %d\n", path, barResult);
    return 1;
  }
  
  return 0;
}

int main()
{
    printf("[BEGIN] interpose-then-dlopen\n");

    if (tryImage(RUN_DIR "/libfoo.dylib", 4, 2))
      return 0;
  
    if (tryImage(RUN_DIR "/libbar.bundle", 4, 100))
      return 0;

    //printf("%p %p %p %p\n", p1, p2, p3, p4);
    printf("[PASS] interpose-then-dlopen\n");
	return 0;
}
