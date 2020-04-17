#include <string.h>
#include <stdio.h>

extern void setState(const char* from);


void a(const char* from) {
    char buffer[100];
    sprintf(buffer, "a() from %s", from);
    setState(buffer);
}
