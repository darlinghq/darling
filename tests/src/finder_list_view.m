// CFLAGS: -framework Foundation -framework AppKit -fobjc-arc -Isrc/Finder src/Finder/FinderDirectory.m src/Finder/FinderDSStore.m src/Finder/FinderFolderStyle.m src/Finder/FinderFreeformLayout.m src/Finder/FinderListViewController.m src/Finder/FinderDrop.m src/Finder/FinderFileOperation.m
// ARGS: "$(mktemp -d)/l"
// Copyright (C) 2026 Lubos Dolezel
//
// A Finder window's list view (src/Finder/FinderListViewController), in process: its columns, the rows and what their
// cells show, sorting by a column, and expanding a folder in place. See tests/finder.md. Needs a display.
// Usage: finder_list_view <new directory>. Prints PASS/FAIL lines; exits 1 on a failure.
#import <AppKit/AppKit.h>
#import "FinderDirectory.h"
#import "FinderListViewController.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

static int failures;

static void expect(BOOL ok, NSString *what)
{
    printf("%s: %s\n", ok ? "PASS" : "FAIL", [what UTF8String]);
    if (!ok)
        failures++;
}

static NSOutlineView *listOutline;

static NSString *rows(void)
{
    NSMutableArray *names = [NSMutableArray array];
    for (NSInteger row = 0; row < [listOutline numberOfRows]; row++)
        [names addObject: [[listOutline itemAtRow: row] lastPathComponent]];
    return [names componentsJoinedByString: @" "];
}

static NSString *cell(NSString *column, NSString *name)
{
    NSInteger col = [listOutline columnWithIdentifier: column];
    for (NSInteger row = 0; row < [listOutline numberOfRows]; row++) {
        if ([[[listOutline itemAtRow: row] lastPathComponent] isEqual: name])
            return [[listOutline preparedCellAtColumn: col row: row] stringValue];
    }
    return nil;
}

static void setTime(NSString *path, NSDate *date)
{
    struct timeval times[2];
    times[0].tv_sec = times[1].tv_sec = (time_t) [date timeIntervalSince1970];
    times[0].tv_usec = times[1].tv_usec = 0;
    utimes([path fileSystemRepresentation], times);
}

static NSDate *localDate(NSInteger year, NSInteger month, NSInteger day, NSInteger hour, NSInteger minute)
{
    NSDateComponents *c = [[NSDateComponents alloc] init];
    c.year = year;
    c.month = month;
    c.day = day;
    c.hour = hour;
    c.minute = minute;
    return [[NSCalendar currentCalendar] dateFromComponents: c];
}

int main(int argc, char **argv)
{
    @autoreleasepool {
        if (argc != 2) {
            fprintf(stderr, "usage: %s <new directory>\n", argv[0]);
            return 2;
        }
        [NSApplication sharedApplication];
        NSFileManager *fm = [NSFileManager defaultManager];
        NSString *dir = [NSString stringWithUTF8String: argv[1]];
        [fm createDirectoryAtPath: [dir stringByAppendingPathComponent: @"sub"] withIntermediateDirectories: YES
                       attributes: nil error: NULL];
        [fm createFileAtPath: [dir stringByAppendingPathComponent: @"sub/inner.txt"] contents: [NSData data] attributes: nil];
        [fm createFileAtPath: [dir stringByAppendingPathComponent: @"small"] contents: [@"0123456789" dataUsingEncoding: NSUTF8StringEncoding]
                  attributes: nil];
        int fd = open([[dir stringByAppendingPathComponent: @"big"] fileSystemRepresentation], O_CREAT | O_WRONLY, 0644);
        ftruncate(fd, 2097152);
        close(fd);
        NSDate *first = localDate(2026, 1, 2, 3, 4), *second = localDate(2026, 2, 3, 4, 5);
        setTime([dir stringByAppendingPathComponent: @"small"], first);
        setTime([dir stringByAppendingPathComponent: @"big"], second);
        setTime([dir stringByAppendingPathComponent: @"sub"], second);

        FinderDirectory *folder = [[FinderDirectory alloc] initWithURL: [NSURL fileURLWithPath: dir]];
        [folder reload: NULL];
        FinderListViewController *list = [[FinderListViewController alloc] init];
        NSWindow *window = [[NSWindow alloc] initWithContentRect: NSMakeRect(0, 0, 800, 500)
                                                       styleMask: NSWindowStyleMaskTitled backing: NSBackingStoreBuffered defer: NO];
        list.view.frame = [window.contentView bounds];
        [window.contentView addSubview: list.view];
        list.items = folder.items;
        listOutline = list.outlineView;

        NSMutableArray *titles = [NSMutableArray array], *identifiers = [NSMutableArray array];
        for (NSTableColumn *column in [listOutline tableColumns]) {
            [titles addObject: [[column headerCell] stringValue]];
            [identifiers addObject: [column identifier]];
        }
        expect([titles isEqual: (@[ @"Name", @"Date Modified", @"Size", @"Kind" ])],
               [NSString stringWithFormat: @"the column titles (%@)", [titles componentsJoinedByString: @", "]]);
        expect([identifiers isEqual: (@[ @"name", @"modified", @"size", @"kind" ])], @"the column identifiers");
        expect([rows() isEqual: @"big small sub"], [NSString stringWithFormat: @"rows by name (%@)", rows()]);

        expect([cell(@"name", @"big") isEqual: @"big"], @"the name cell");
        NSString *mb = [NSByteCountFormatter stringFromByteCount: 2097152 countStyle: NSByteCountFormatterCountStyleFile];
        NSString *bytes = [NSByteCountFormatter stringFromByteCount: 10 countStyle: NSByteCountFormatterCountStyleFile];
        expect([cell(@"size", @"big") isEqual: mb] && [cell(@"size", @"small") isEqual: bytes] && [cell(@"size", @"sub") isEqual: @"--"],
               [NSString stringWithFormat: @"the size cells (%@, %@, %@; want %@, %@, --)", cell(@"size", @"big"),
                                          cell(@"size", @"small"), cell(@"size", @"sub"), mb, bytes]);
        NSDateFormatter *format = [[NSDateFormatter alloc] init];
        format.dateStyle = NSDateFormatterMediumStyle;
        format.timeStyle = NSDateFormatterShortStyle;
        expect([cell(@"modified", @"small") isEqual: [format stringFromDate: first]]
                   && [cell(@"modified", @"big") isEqual: [format stringFromDate: second]],
               [NSString stringWithFormat: @"the date cells (%@; want %@)", cell(@"modified", @"small"), [format stringFromDate: first]]);
        NSString *kind = nil;
        [[NSURL fileURLWithPath: [dir stringByAppendingPathComponent: @"sub"]] getResourceValue: &kind
                                                                                        forKey: NSURLLocalizedTypeDescriptionKey error: NULL];
        expect(kind != nil && [cell(@"kind", @"sub") isEqual: kind],
               [NSString stringWithFormat: @"the kind cell (%@; want %@)", cell(@"kind", @"sub"), kind]);

        listOutline.sortDescriptors = @[ [NSSortDescriptor sortDescriptorWithKey: @"size" ascending: NO] ];
        expect([rows() isEqual: @"big small sub"], [NSString stringWithFormat: @"by size, descending (%@)", rows()]);
        listOutline.sortDescriptors = @[ [NSSortDescriptor sortDescriptorWithKey: @"size" ascending: YES] ];
        expect([rows() isEqual: @"sub small big"], [NSString stringWithFormat: @"by size, ascending (%@)", rows()]);
        listOutline.sortDescriptors = @[ [NSSortDescriptor sortDescriptorWithKey: @"name" ascending: YES] ];
        expect([rows() isEqual: @"big small sub"], [NSString stringWithFormat: @"by name again (%@)", rows()]);

        NSURL *sub = [listOutline itemAtRow: 2];
        expect([listOutline isExpandable: sub] && ![listOutline isExpandable: [listOutline itemAtRow: 0]], @"folders expand, files don't");
        [listOutline expandItem: sub];
        expect([rows() isEqual: @"big small sub inner.txt"] && [listOutline levelForRow: 3] == 1,
               [NSString stringWithFormat: @"expanding sub shows inner.txt at level 1 (%@)", rows()]);

        [list selectItems: @[ [NSURL fileURLWithPath: [dir stringByAppendingPathComponent: @"small"]],
                              [NSURL fileURLWithPath: [dir stringByAppendingPathComponent: @"sub/inner.txt"]] ]];
        NSMutableArray *selected = [NSMutableArray array];
        for (NSURL *url in list.selectedItems)
            [selected addObject: [url lastPathComponent]];
        expect([selected isEqual: (@[ @"small", @"inner.txt" ])],
               [NSString stringWithFormat: @"selecting items, one of them in an expanded folder (%@)", selected]);

        printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
    }
    return failures != 0;
}
