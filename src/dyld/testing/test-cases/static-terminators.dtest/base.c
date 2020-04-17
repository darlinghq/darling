#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

static bool mainCalled           = false;
static bool libCalled            = false;
static bool libCalledBeforeMain  = false;

void mainTerminated()
{
    mainCalled = true;
}

void libDynamicTerminated()
{
    libCalled = true;
    if ( !mainCalled )
        libCalledBeforeMain = true;
}


static __attribute__((destructor))
void myTerm()
{
    if ( !mainCalled )
        printf("[FAIL] static-terminators, main's terminator not called\n");
    else if ( !libCalled )
        printf("[FAIL] static-terminators, libDynamic's terminator not called\n");
    else if ( !libCalledBeforeMain )
        printf("[FAIL] static-terminators, libDynamic's terminator called out of order\n");
    else
        printf("[PASS] static-terminators\n");
}

