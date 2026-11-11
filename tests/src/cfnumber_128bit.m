// Copyright (C) 2026 Lubos Dolezel
// CFNumber values of the 128-bit storage type used for unsigned values above
// INT64_MAX report "Q" from -objCType and are read as unsigned 64-bit by
// -getValue: and -unsignedLongLongValue (no clamping to INT64_MAX).
#import <Foundation/Foundation.h>
#include <stdio.h>
#include <string.h>

#define CHECK(c) do { if (!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); return 1; } } while (0)

typedef struct { int64_t high; uint64_t low; } Int128;

int main(void)
{
    @autoreleasepool {
        Int128 v = { 0, UINT64_MAX };
        NSNumber *n = (NSNumber *)CFNumberCreate(NULL, (CFNumberType)17, &v);
        CHECK(n != nil);
        CHECK(strcmp([n objCType], "Q") == 0);
        unsigned long long out = 0;
        [n getValue:&out];
        CHECK(out == UINT64_MAX);
        CHECK([n unsignedLongLongValue] == UINT64_MAX);
        CHECK([n unsignedLongValue] == (unsigned long)UINT64_MAX);
        CHECK([n unsignedIntegerValue] == (NSUInteger)UINT64_MAX);
        CHECK([n doubleValue] > 1.8e19);

        Int128 w = { 0, (uint64_t)INT64_MAX + 5 };
        NSNumber *m = (NSNumber *)CFNumberCreate(NULL, (CFNumberType)17, &w);
        out = 0;
        [m getValue:&out];
        CHECK(out == (uint64_t)INT64_MAX + 5);
        CFRelease((CFTypeRef)n);
        CFRelease((CFTypeRef)m);
    }
    printf("PASS: 128-bit CFNumber reads as unsigned 64-bit\n");
    return 0;
}
