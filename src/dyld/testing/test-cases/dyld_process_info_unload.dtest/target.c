#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <mach/mach.h>



int main(int argc, const char* argv[])
{
    //fprintf(stderr, "target starting\n");
    usleep(1000);
    // load and unload in a loop
    for (int i=1; i < 10000; ++i) {
        void* h = dlopen("./libfoo.dylib", 0);
        usleep(100000/(i*100));
        dlclose(h);
    }
    //fprintf(stderr, "target done\n");

    return 0;
}

