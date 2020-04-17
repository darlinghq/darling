#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <signal.h>
#include <unistd.h>
#include <mach/mach.h>



int main(int argc, const char* argv[])
{
    if ( (argc > 1) && (strcmp(argv[1], "suspend-in-main") == 0) )
        (void)kill(getpid(), SIGSTOP);

    for (int i=0; i < 3; ++i) {
        void* h = dlopen("./libfoo.dylib", 0);
        dlclose(h);
    }

    return 0;
}

