
// BUILD:  $CC main.c -o $BUILD_DIR/bind-addend.exe -lobjc

// RUN:  ./bind-addend.exe

// Verify that negative addends work with pointers in to the shared cache and pointers to the image itself

#include <stdio.h>

// Note this is weak so that we have a bind
__attribute__((weak))
void* p = 0;

// Choose a large enough negative offset to be before the shared cache or the image
const uintptr_t offset = 1ULL << 36;
void* pMinus = (void*)((uintptr_t)&p - offset);

// Get a pointer to something we assume is in the shared cache
// Note we don't declare a function as arm64e would want to sign this
extern int objc_msgSend;
void* msgSendMinus = (void*)((uintptr_t)&objc_msgSend - offset);

int main()
{
    printf("[BEGIN] bind-addend\n");

    if ( pMinus != (void*)((uintptr_t)&p - offset) ) {
        printf("[FAIL]  bind-addend: %p != %p\n", pMinus, (void*)((uintptr_t)&p - offset));
        return 0;
    }

    if ( msgSendMinus != (void*)((uintptr_t)&objc_msgSend - offset) ) {
        printf("[FAIL]  bind-addend: %p != %p\n", msgSendMinus, (void*)((uintptr_t)&objc_msgSend - offset));
        return 0;
    }

    printf("[PASS]  bind-addend\n");
    return 0;
}

