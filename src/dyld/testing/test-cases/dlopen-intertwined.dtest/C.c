#include <string.h>
#include <stdio.h>

extern void setState(const char* from);

void c(const char* from) {
    char buffer[100];
    sprintf(buffer, "c() from %s", from);
    setState(buffer);
}

void __attribute__((constructor))
initC()
{
    setState("initC");
}
