// Copyright (C) 2026 Lubos Dolezel
//
// The named images that menu items in nibs use for the on and mixed states exist, are
// non-empty, and the Template variants are template images.
#import <AppKit/AppKit.h>
#include <stdio.h>

int main(void)
{
    @autoreleasepool {
        int failures = 0;
        NSArray *names = @[@"NSMenuCheckmark", @"NSMenuMixedState", @"NSMenuOnState",
                           NSImageNameMenuOnStateTemplate, NSImageNameMenuMixedStateTemplate];
        for (NSString *name in names) {
            NSImage *image = [NSImage imageNamed:name];
            if (image == nil || image.size.width <= 0 || image.size.height <= 0) {
                printf("FAIL %s missing\n", name.UTF8String);
                failures++;
            }
        }
        NSImage *tmpl = [NSImage imageNamed:NSImageNameMenuOnStateTemplate];
        if (tmpl != nil && ![tmpl isTemplate]) {
            printf("FAIL template flag\n");
            failures++;
        }
        printf(failures ? "FAIL\n" : "PASS\n");
        return failures ? 1 : 0;
    }
}
