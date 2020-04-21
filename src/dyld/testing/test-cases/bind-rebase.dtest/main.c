
// BUILD:  $CC main.c  -Wl,-fixup_chains  -o $BUILD_DIR/bind-rebase.exe

// RUN:  ./bind-rebase.exe

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <dlfcn.h> 
#include <assert.h>

extern char tzname[];  // a char array in libSystem.dylib


#define VERIFY(a,b) assert(a==b)


static uint8_t a;
uint8_t* const rebasedPtrs[] = { NULL, NULL, &a, &a+1, &a+16, &a+1023, NULL, &a-1 };

void verifyRebases()
{
    VERIFY(rebasedPtrs[0], NULL);
    VERIFY(rebasedPtrs[1], NULL);
    VERIFY(rebasedPtrs[2], &a);
    VERIFY(rebasedPtrs[3], &a+1);
    VERIFY(rebasedPtrs[4], &a+16);
    VERIFY(rebasedPtrs[5], &a+1023);
    VERIFY(rebasedPtrs[6], NULL);
    VERIFY(rebasedPtrs[7], &a-1);
}


static char* const bindPtrs[] = { NULL, NULL, tzname, tzname+1, &tzname[16], &tzname[1023], NULL, &tzname[-1], (char*)&malloc, (char*)&free };

void verifyBinds()
{
    VERIFY(bindPtrs[0], NULL);
    VERIFY(bindPtrs[1], NULL);
    VERIFY(bindPtrs[2], tzname);
    VERIFY(bindPtrs[3], tzname+1);
    VERIFY(bindPtrs[4], tzname+16);
    VERIFY(bindPtrs[5], tzname+1023);
    VERIFY(bindPtrs[6], NULL);
    VERIFY(bindPtrs[7], tzname-1);
    VERIFY(bindPtrs[8], (char*)&malloc);
    VERIFY(bindPtrs[9], (char*)&free);
}

#if !__LP64__
#define JUNK ((uint8_t*)0x12345678)
uint8_t* const otherPtrs[] = {
    &a,
    // far enough apart to require co-opting a NULL
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    &a+1,
    // far enough apart to require co-opting two NULLs
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    &a,
    // far apart and intermediate values are not co-optable
    // so a new chain must be used
    JUNK, JUNK, JUNK, JUNK, JUNK, JUNK, JUNK, JUNK,
    JUNK, JUNK, JUNK, JUNK, JUNK, JUNK, JUNK, JUNK,
    JUNK, JUNK, JUNK, JUNK, JUNK, JUNK, JUNK, JUNK,
    JUNK, JUNK, JUNK, JUNK, JUNK, JUNK, JUNK, JUNK,
    JUNK, JUNK, JUNK, JUNK, JUNK, JUNK, JUNK, JUNK,
    &a,
    &a,
};

void verifyLongChains()
{
    VERIFY(otherPtrs[0],   &a);
    VERIFY(otherPtrs[41],  &a+1);
    VERIFY(otherPtrs[122], &a);
    VERIFY(otherPtrs[163], &a);
    VERIFY(otherPtrs[164], &a);
}
#endif

int main()
{
    printf("[BEGIN] bind-rebase\n");
    verifyRebases();
    verifyBinds();

#if !__LP64__
    verifyLongChains();
#endif

    printf("[PASS]  bind-rebase\n");
    return 0;
}

