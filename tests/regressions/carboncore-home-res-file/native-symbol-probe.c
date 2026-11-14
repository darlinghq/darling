/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdio.h>
#include <dlfcn.h>
int main(void)
{
    const char* path = "/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/CarbonCore.framework/Versions/A/CarbonCore";
    void* library = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (!library) { fprintf(stderr, "LOAD_FAILED %s\n", dlerror()); return 2; }
    void* symbol = dlsym(library, "HomeResFile");
    printf("HOME_RES_SYMBOL_PRESENT=%d\n", symbol != NULL);
    return symbol ? 0 : 77;
}
