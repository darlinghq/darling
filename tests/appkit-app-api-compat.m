/*
 * Copyright (C) 2026 Darling Developers
 *
 * Runtime checks for the AppKit/Quick Look API surface.
 * Build as an Objective-C executable linked against AppKit and Quartz:
 *
 *   clang -fno-objc-arc -framework AppKit -framework Quartz \
 *       tests/appkit-app-api-compat.m -o /tmp/appkit-app-api-compat
 *
 * Run the result with `darling shell`. Exits non-zero on failure.
 */
#import <AppKit/AppKit.h>
#import <Quartz/Quartz.h>
#include <stdio.h>

static int failures;
#define CHECK(condition) do { \
    if (!(condition)) { \
        printf("FAIL line %d: %s\n", __LINE__, #condition); \
        failures++; \
    } \
} while (0)

@interface AppApiProbe : NSObject <NSSoundDelegate, NSToolbarItemValidation,
    QLPreviewPanelDataSource, QLPreviewPanelDelegate> {
@public
    NSArray *_items;
    BOOL _finished;
}
@end

@implementation AppApiProbe
- (NSInteger) numberOfPreviewItemsInPreviewPanel: (QLPreviewPanel *) panel
{
    return (NSInteger)[_items count];
}

- (id<QLPreviewItem>) previewPanel: (QLPreviewPanel *) panel
                previewItemAtIndex: (NSInteger) index
{
    if (index < 0 || index >= (NSInteger)[_items count])
        return nil;
    return [_items objectAtIndex: (NSUInteger)index];
}

- (BOOL) validateToolbarItem: (NSToolbarItem *) item
{
    return YES;
}

- (void) sound: (NSSound *) sound didFinishPlaying: (BOOL) successfully
{
    _finished = successfully;
}
@end

int main(void)
{
    @autoreleasepool {
        CHECK(![QLPreviewPanel sharedPreviewPanelExists]);

        QLPreviewPanel *panel = [QLPreviewPanel sharedPreviewPanel];
        CHECK(panel != nil);
        CHECK([QLPreviewPanel sharedPreviewPanel] == panel);
        CHECK([QLPreviewPanel sharedPreviewPanelExists]);

        AppApiProbe *probe = [AppApiProbe new];
        NSURL *first = [NSURL URLWithString: @"file:///tmp/first.txt"];
        NSURL *second = [NSURL URLWithString: @"file:///tmp/second.txt"];
        probe->_items = @[ first, second ];

        panel.dataSource = probe;
        panel.delegate = probe;
        CHECK(panel.delegate == probe);
        // The preview panel delegate is also the window delegate.
        CHECK([(NSWindow *)panel delegate] == (id)probe);
        CHECK([[panel currentPreviewItem] isEqual: first]);

        panel.currentPreviewItemIndex = 1;
        CHECK([[panel currentPreviewItem] isEqual: second]);

        // A negative index is ignored; an out-of-range index is clamped after
        // -reloadData reports fewer items.
        panel.currentPreviewItemIndex = -1;
        CHECK(panel.currentPreviewItemIndex == 1);
        probe->_items = @[ first ];
        [panel reloadData];
        CHECK(panel.currentPreviewItemIndex == 0);

        // +soundNamed: finds nothing on a system without that sound resource, so
        // the sound is created explicitly.
        NSSound *sound = [[[NSSound alloc] initWithContentsOfFile: @"/nonexistent/probe.aiff"
                                                      byReference: NO] autorelease];
        CHECK(sound != nil);
        CHECK(sound.delegate == nil);
        sound.delegate = probe;
        CHECK(sound.delegate == probe);
        sound.delegate = nil;
        CHECK(sound.delegate == nil);

        if (failures)
            printf("FAIL: %d check(s)\n", failures);
        else
            printf("PASS\n");
        return failures ? 1 : 0;
    }
}
