

#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <dispatch/dispatch.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>


static void* work1(void* arg)
{
    void* h = dlopen("System/Library/Frameworks/Foundation.framework/Foundation", 0);

    return NULL;
}


__attribute__((constructor))
void myinit()
{
    pthread_t workerThread;

    if ( pthread_create(&workerThread, NULL, work1, NULL) != 0 ) {
        printf("[FAIL]  dlopen-in-init, pthread_create\n");
        return;
    }

    void* dummy;
    pthread_join(workerThread, &dummy);
}

