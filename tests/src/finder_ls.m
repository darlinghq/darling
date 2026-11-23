// CFLAGS: -framework Foundation -fobjc-arc -Isrc/Finder src/Finder/FinderDirectory.m
// Copyright (C) 2026 Lubos Dolezel
//
// Prints the items Finder's folder model (src/Finder/FinderDirectory) shows for a folder, one name per line, in
// order: what a Finder window on it lists. tests/finder.sh's "styled hidden item" runs it on a disk image's volume.
// Usage: finder_ls <folder>. Exits 1 if the folder can't be read.
#import <Foundation/Foundation.h>
#import "FinderDirectory.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    @autoreleasepool {
        if (argc != 2) {
            fprintf(stderr, "usage: %s <folder>\n", argv[0]);
            return 2;
        }
        FinderDirectory *folder = [[FinderDirectory alloc] initWithURL: [NSURL fileURLWithPath: [NSString stringWithUTF8String: argv[1]]]];
        NSError *error = nil;
        if (![folder reload: &error]) {
            fprintf(stderr, "%s\n", [[error description] UTF8String]);
            return 1;
        }
        for (NSURL *item in folder.items)
            printf("%s\n", [[item lastPathComponent] UTF8String]);
    }
    return 0;
}
