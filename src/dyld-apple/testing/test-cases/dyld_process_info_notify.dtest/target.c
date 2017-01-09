#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <mach/mach.h>



int main(int argc, const char* argv[])
{
    if ( (argc > 1) && (strcmp(argv[1], "suspend-in-main") == 0) )
        task_suspend(mach_task_self());

    for (int i=0; i < 3; ++i) {
        void* h = dlopen("./libfoo.dylib", 0);
        dlclose(h);
    }

    return 0;
}

