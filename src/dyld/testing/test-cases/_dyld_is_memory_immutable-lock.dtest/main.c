

// BUILD:  $CC foo.c -bundle -o $BUILD_DIR/lock.bundle
// BUILD:  $CC main.c -o $BUILD_DIR/immutable-lock.exe  -DRUN_DIR="$RUN_DIR"

// RUN:  ./immutable-lock.exe

#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <pthread.h>
#include <mach-o/dyld_priv.h>


static void* work(void* mh)
{
    _dyld_is_memory_immutable(mh, 16);

    return NULL;
}

static void notify(const struct mach_header* mh, intptr_t vmaddr_slide)
{
    if ( mh->flags & 0x80000000 )
        return;

    // for each image notified that is not in the dyld cache, spin up a thread which calls _dyld_is_memory_immutable()
    pthread_t workerThread;
    if ( pthread_create(&workerThread, NULL, &work, (void*)mh) != 0 ) {
        printf("[FAIL]  _dyld_is_memory_immutable-lock, pthread_create\n");
        exit(0);
    }
    void* dummy;
    pthread_join(workerThread, &dummy);
}



int main()
{
    printf("[BEGIN] _dyld_is_memory_immutable-lock\n");

    _dyld_register_func_for_add_image(&notify);

    void* h = dlopen(RUN_DIR "/lock.bundle", 0);
    if ( h == NULL ) {
        printf("dlopen(lock.bundle) failed with error: %s\n", dlerror());
        printf("[FAIL]  _dyld_is_memory_immutable-lock, lock.bundle not loaded\n");
        return 0;
    }

    printf("[PASS]  _dyld_is_memory_immutable-lock\n");
	return 0;
}

