// Copyright (C) 2026 Lubos Dolezel
// NSNumber keeps unsigned values above INT64_MAX distinct from negative ones.
#import <Foundation/Foundation.h>
#include <stdio.h>
#include <string.h>

#define CHECK(c) do { if (!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); return 1; } } while (0)

int main(void)
{
    @autoreleasepool {
        NSNumber *big = [NSNumber numberWithUnsignedLongLong:UINT64_MAX];
        NSNumber *neg = [NSNumber numberWithLongLong:-1];
        CHECK(![big isEqualToNumber:neg]);
        CHECK([big compare:neg] == NSOrderedDescending);
        CHECK([neg compare:big] == NSOrderedAscending);
        CHECK([big unsignedLongLongValue] == UINT64_MAX);
        CHECK([big doubleValue] > 1.8e19);
        CHECK([[big description] isEqualToString:@"18446744073709551615"]);
        CHECK(strcmp([big objCType], "Q") == 0);
        unsigned long long out = 0;
        [big getValue:&out];
        CHECK(out == UINT64_MAX);

        CHECK([[NSNumber numberWithUnsignedLong:ULONG_MAX] isEqualToNumber:big]);
        CHECK([[NSNumber numberWithUnsignedInteger:NSUIntegerMax] isEqualToNumber:big]);
        CHECK([[NSNumber numberWithUnsignedLongLong:UINT64_MAX] hash] == [big hash]);

        // values up to INT64_MAX stay equal to their signed counterparts
        CHECK([[NSNumber numberWithUnsignedLongLong:INT64_MAX] isEqualToNumber:[NSNumber numberWithLongLong:INT64_MAX]]);
        CHECK([[NSNumber numberWithUnsignedLongLong:5] isEqualToNumber:@5]);

        // property list round trip keeps the value
        NSError *err = nil;
        NSData *d = [NSPropertyListSerialization dataWithPropertyList:@[big] format:NSPropertyListBinaryFormat_v1_0 options:0 error:&err];
        CHECK(d != nil);
        NSArray *back = [NSPropertyListSerialization propertyListWithData:d options:0 format:NULL error:&err];
        CHECK([back count] == 1);
        CHECK([[back objectAtIndex:0] unsignedLongLongValue] == UINT64_MAX);
    }
    printf("PASS: NSNumber UINT64_MAX stays unsigned\n");
    return 0;
}
