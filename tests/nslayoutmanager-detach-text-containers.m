// Copyright (C) 2026 Lubos Dolezel
//
// A text container does not keep a stale layout manager. Observed on macOS 26.7.3 (black-box, tier 2; this file
// was run there): once the layout manager that holds a container is deallocated, or the container is removed from
// it, the container's layoutManager is nil. A container that was attached to a different layout manager in the
// meantime keeps that one. (A container attached only with -setLayoutManager: is not registered with the layout
// manager and is not covered.)
// No display needed. MRC.
#import <AppKit/AppKit.h>
#include <stdio.h>

static int failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); failures++; } } while (0)

int main(void)
{
    @autoreleasepool
    {
        NSTextContainer *tc = [[[NSTextContainer alloc] initWithContainerSize:NSMakeSize(100, 100)] autorelease];
        NSLayoutManager *lm = [[NSLayoutManager alloc] init];
        [lm addTextContainer:tc];
        CHECK([tc layoutManager] == lm);
        [lm release];
        CHECK([tc layoutManager] == nil);

        // removed from the layout manager
        NSTextContainer *tc2 = [[[NSTextContainer alloc] initWithContainerSize:NSMakeSize(100, 100)] autorelease];
        NSLayoutManager *lm2 = [[[NSLayoutManager alloc] init] autorelease];
        [lm2 addTextContainer:tc2];
        CHECK([tc2 layoutManager] == lm2);
        [lm2 removeTextContainerAtIndex:0];
        CHECK([tc2 layoutManager] == nil);
        CHECK([[lm2 textContainers] count] == 0);

        // inserted at an index, then the layout manager goes
        NSTextContainer *first = [[[NSTextContainer alloc] initWithContainerSize:NSMakeSize(100, 100)] autorelease];
        NSTextContainer *second = [[[NSTextContainer alloc] initWithContainerSize:NSMakeSize(100, 100)] autorelease];
        NSLayoutManager *lm3 = [[NSLayoutManager alloc] init];
        [lm3 addTextContainer:first];
        [lm3 insertTextContainer:second atIndex:0];
        CHECK([first layoutManager] == lm3 && [second layoutManager] == lm3);
        [lm3 release];
        CHECK([first layoutManager] == nil && [second layoutManager] == nil);

        // moved to another layout manager: the old one going away leaves the new attachment alone
        NSTextContainer *moved = [[[NSTextContainer alloc] initWithContainerSize:NSMakeSize(100, 100)] autorelease];
        NSLayoutManager *oldLm = [[NSLayoutManager alloc] init];
        NSLayoutManager *newLm = [[[NSLayoutManager alloc] init] autorelease];
        [oldLm addTextContainer:moved];
        [newLm addTextContainer:moved];
        CHECK([moved layoutManager] == newLm);
        [oldLm release];
        CHECK([moved layoutManager] == newLm);
    }
    if (failures == 0)
        printf("PASS nslayoutmanager-detach-text-containers\n");
    return failures != 0;
}
