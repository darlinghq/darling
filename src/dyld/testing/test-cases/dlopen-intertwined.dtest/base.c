#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* expectedStrings[] = {
    "a() from main",
    "initC",
    "c() from initB",
    "c() from initD",
    "a() from initE",
    "d() from initF",
    "DONE"
};

static const char** curState = expectedStrings;

void setState(const char* from)
{
    printf("%s\n", from);
    if ( strcmp(*curState, from) != 0 ) {
        printf("[FAIL] dlopen-intertwined: expected %s\n", *curState);
        exit(0);
    }
    ++curState;
}

