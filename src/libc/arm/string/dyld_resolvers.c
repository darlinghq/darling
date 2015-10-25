// This file implements resolvers to assist dyld in choosing the correct
// implementation of <string.h> routines on ARMv7 processors.  The
// micro-architectural differences between Cortex-A8 and Cortex-A9 are such
// that optimal write loops are quite different on the two processors.

#include <arm/arch.h>
#if defined _ARM_ARCH_7 && !defined VARIANT_DYLD

#include <stdlib.h>
#include <machine/cpu_capabilities.h>
#include <mach/machine.h>

// The MakeResolver(name) macro creates a function that dyld calls to
// pick an implementation of the name function.  It does a check to determine
// if it is running on a8 or a9 hardware, and returns a pointer to either
//
//    name$VARIANT$CortexA8
// or
//    name$VARIANT$CortexA9
// or
//    name$VARIANT$Swift
//
// This resolution only occurs once per process; once a symbol is bound to an
// implementation in dyld, no further calls to the resolver occur.
//
// On unknown implementations of the ARMv7 architecture, the Swift variant
// is returned by these resolvers.
#define MakeResolver(name)                                           \
    void * name ## Resolver(void) __asm__("_" #name);                \
    void * name ## Resolver(void) {                                  \
        __asm__(".symbol_resolver _" #name);                         \
        switch (*(int *)_COMM_PAGE_CPUFAMILY) {                      \
            case CPUFAMILY_ARM_13: return name ## $VARIANT$CortexA8; \
            case CPUFAMILY_ARM_14: return name ## $VARIANT$CortexA9; \
            default:               return name ## $VARIANT$Swift;    \
        }                                                            \
    }

void bzero$VARIANT$CortexA8(void *, size_t);
void bzero$VARIANT$CortexA9(void *, size_t);
void bzero$VARIANT$Swift(void *, size_t);
MakeResolver(bzero)

void __bzero$VARIANT$CortexA8(void *, size_t);
void __bzero$VARIANT$CortexA9(void *, size_t);
void __bzero$VARIANT$Swift(void *, size_t);
MakeResolver(__bzero)

void *memset$VARIANT$CortexA8(void *, int, size_t);
void *memset$VARIANT$CortexA9(void *, int, size_t);
void *memset$VARIANT$Swift(void *, int, size_t);
MakeResolver(memset)

void bcopy$VARIANT$CortexA8(const void *, void *, size_t);
void bcopy$VARIANT$CortexA9(const void *, void *, size_t);
void bcopy$VARIANT$Swift(const void *, void *, size_t);
MakeResolver(bcopy)

void *memmove$VARIANT$CortexA8(void *, const void *, size_t);
void *memmove$VARIANT$CortexA9(void *, const void *, size_t);
void *memmove$VARIANT$Swift(void *, const void *, size_t);
MakeResolver(memmove)

void *memcpy$VARIANT$CortexA8(void *, const void *, size_t);
void *memcpy$VARIANT$CortexA9(void *, const void *, size_t);
void *memcpy$VARIANT$Swift(void *, const void *, size_t);
MakeResolver(memcpy)

//  The memset_patternx resolvers are a little different, because we only have
//  a generic implementation and a Swift implementation.
#define MakeResolver_Swift(name)                                     \
    void * name ## Resolver(void) __asm__("_" #name);                \
    void * name ## Resolver(void) {                                  \
        __asm__(".symbol_resolver _" #name);                         \
        switch (*(int *)_COMM_PAGE_CPUFAMILY) {                      \
            case CPUFAMILY_ARM_13: return name ## $VARIANT$Generic;  \
            case CPUFAMILY_ARM_14: return name ## $VARIANT$Generic;  \
            default:               return name ## $VARIANT$Swift;    \
        }                                                            \
    }

void memset_pattern4$VARIANT$Generic(void *b, const void *c4, size_t len);
void memset_pattern4$VARIANT$Swift(void *b, const void *c4, size_t len);
MakeResolver_Swift(memset_pattern4);

void memset_pattern8$VARIANT$Generic(void *b, const void *c8, size_t len);
void memset_pattern8$VARIANT$Swift(void *b, const void *c8, size_t len);
MakeResolver_Swift(memset_pattern8);

void memset_pattern16$VARIANT$Generic(void *b, const void *c16, size_t len);
void memset_pattern16$VARIANT$Swift(void *b, const void *c16, size_t len);
MakeResolver_Swift(memset_pattern16);

#else // defined _ARM_ARCH_7 && !defined VARIANT_DYLD

typedef int emptyFilesArentCFiles;

#endif // defined _ARM_ARCH_7 && !defined VARIANT_DYLD
