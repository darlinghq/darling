// Copyright (C) 2026 Lubos Dolezel
//
// -[NSFileManager directoryContentsAtPath:] (deprecated, still present) lists the shallow
// contents of a directory and returns nil for a missing one.
#import <Foundation/Foundation.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    @autoreleasepool {
        NSFileManager *fm = [NSFileManager defaultManager];
        NSString *dir = [NSTemporaryDirectory() stringByAppendingPathComponent:@"dircontents-test"];
        [fm removeItemAtPath:dir error:nil];
        [fm createDirectoryAtPath:[dir stringByAppendingPathComponent:@"sub"] withIntermediateDirectories:YES attributes:nil error:nil];
        [fm createFileAtPath:[dir stringByAppendingPathComponent:@"a.txt"] contents:[NSData data] attributes:nil];
        [fm createFileAtPath:[dir stringByAppendingPathComponent:@"sub/b.txt"] contents:[NSData data] attributes:nil];
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        NSArray *list = [[fm directoryContentsAtPath:dir] sortedArrayUsingSelector:@selector(compare:)];
        NSArray *missing = [fm directoryContentsAtPath:[dir stringByAppendingPathComponent:@"nope"]];
#pragma clang diagnostic pop
        int ok = [list isEqualToArray:@[@"a.txt", @"sub"]] && missing == nil;
        [fm removeItemAtPath:dir error:nil];
        printf(ok ? "PASS\n" : "FAIL %s\n", [[list description] UTF8String]);
        return ok ? 0 : 1;
    }
}
