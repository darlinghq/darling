
/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <CarbonCore/MacMemory.h>
#include <CarbonCore/MacErrors.h>
#include <stdio.h>
#include <string.h>

static unsigned checks, failures;

#define CHECK(x) do { ++checks; if (!(x)) { \
    ++failures; printf("FAIL %d %s\n",__LINE__,#x); \
} } while (0)

int main(void)
{
    const char source[] = "Darling";
    Handle empty = NewEmptyHandle();
    if (!empty)
        return 2;

    OSErr error = PtrToXHand(source,empty,7);
    CHECK(error == nilHandleErr);
    DisposeHandle(empty);

    /* Do not run further tests through the known unsafe baseline. */
    if (error != nilHandleErr) {
        puts("QT_UP09_MEMORY_BASELINE");
        printf("QT_UP09_MEMORY checks=%u failures=%u\n",
               checks,failures);
        return 1;
    }

    Handle h = NewHandle(1);
    if (!h)
        return 2;

    CHECK(PtrToXHand(source,h,7)==noErr);
    CHECK(GetHandleSize(h)==7);
    CHECK(*h && memcmp(*h,source,7)==0);

    if (*h && GetHandleSize(h)==7) {
        CHECK(PtrToXHand(*h+2,h,4)==noErr);
        CHECK(GetHandleSize(h)==4);
        CHECK(*h && memcmp(*h,"rlin",4)==0);
    }

    CHECK(PtrToXHand(NULL,h,3)==paramErr);
    CHECK(PtrToXHand(source,h,-1)==paramErr);
    CHECK(GetHandleSize(h)==4);
    CHECK(*h && memcmp(*h,"rlin",4)==0);

    CHECK(PtrToXHand(source,NULL,1)==nilHandleErr);
    CHECK(PtrToXHand(source,h,0)==noErr);
    CHECK(GetHandleSize(h)==0);

    DisposeHandle(h);

    printf("QT_UP09_MEMORY checks=%u failures=%u\n",
           checks,failures);
    return failures!=0;
}
