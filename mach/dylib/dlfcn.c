#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    void* h = dlopen("mach/dylib/lib.dylib", RTLD_NOW);
    if (!h) {
        fprintf(stderr, "dlopen failed\n");
        abort();
    }

    int (*hello)(const char*) = (int(*)(const char*))dlsym(h, "hello");
    if (!hello) {
        fprintf(stderr, "dlsym failed\n");
        abort();
    }

    if (hello("world") != 42) {
        fprintf(stderr, "hello failed\n");
        abort();
    }
    return 0;
}
