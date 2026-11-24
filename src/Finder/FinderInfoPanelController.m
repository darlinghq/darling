/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */
#import "FinderInfoPanelController.h"
#include <stdatomic.h>

// The rows, top to bottom, with their labels
static NSArray<NSString *> *FinderInfoRows(void)
{
    return @[ @"name", @"kind", @"size", @"where", @"created", @"modified" ];
}

static NSString *FinderInfoLabel(NSString *row)
{
    NSDictionary<NSString *, NSString *> *labels = @{
        @"name" : @"Name:",
        @"kind" : @"Kind:",
        @"size" : @"Size:",
        @"where" : @"Where:",
        @"created" : @"Created:",
        @"modified" : @"Modified:",
    };
    return labels[row];
}

static NSTextField *FinderInfoText(NSString *text, NSRect frame, NSTextAlignment alignment)
{
    NSTextField *field = [[NSTextField alloc] initWithFrame:frame];
    field.stringValue = text;
    field.editable = NO;
    field.selectable = NO;
    field.bordered = NO;
    field.drawsBackground = NO;
    field.alignment = alignment;
    field.font = [NSFont systemFontOfSize:[NSFont smallSystemFontSize]];
    field.lineBreakMode = NSLineBreakByTruncatingMiddle;
    return field;
}

static NSString *FinderInfoDate(NSDate *date)
{
    if (date == nil)
        return @"--";
    NSDateFormatter *formatter = [[NSDateFormatter alloc] init];
    formatter.dateStyle = NSDateFormatterMediumStyle;
    formatter.timeStyle = NSDateFormatterShortStyle;
    return [formatter stringFromDate:date];
}

static NSString *FinderInfoBytes(int64_t bytes)
{
    return [NSByteCountFormatter stringFromByteCount:bytes countStyle:NSByteCountFormatterCountStyleFile];
}

// Open panels by the standardized path of their item
static NSMutableDictionary<NSString *, FinderInfoPanelController *> *FinderInfoPanels(void)
{
    static NSMutableDictionary *panels;
    static dispatch_once_t once;
    dispatch_once(&once, ^{
        panels = [NSMutableDictionary dictionary];
    });
    return panels;
}

// Set when the panel closes, so a count still running stops
@interface FinderInfoCancellation : NSObject {
@public
    atomic_bool cancelled;
}
@end

@implementation FinderInfoCancellation
@end

@interface FinderInfoPanelController () <NSWindowDelegate>
@end

@implementation FinderInfoPanelController {
    NSMutableDictionary<NSString *, NSTextField *> *_values;
    FinderInfoCancellation *_cancellation;
    NSString *_key;
}

+ (FinderInfoPanelController *)showInfoForItem:(NSURL *)item
{
    NSString *key = [[[item URLByStandardizingPath] path] stringByStandardizingPath];
    FinderInfoPanelController *controller = FinderInfoPanels()[key];
    if (controller == nil) {
        controller = [[FinderInfoPanelController alloc] initWithItem:item key:key];
        FinderInfoPanels()[key] = controller;
    }
    [controller showWindow:nil];
    return controller;
}

- (instancetype)initWithItem:(NSURL *)item key:(NSString *)key
{
    const CGFloat width = 300, rowHeight = 20, margin = 16, labelWidth = 72;
    NSArray<NSString *> *rows = FinderInfoRows();
    CGFloat height = margin * 2 + rowHeight * [rows count];
    NSPanel *panel = [[NSPanel alloc] initWithContentRect:NSMakeRect(0, 0, width, height)
                                                styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable
                                                  backing:NSBackingStoreBuffered
                                                    defer:NO];
    if ((self = [super initWithWindow:panel])) {
        _item = item;
        _key = key;
        _values = [NSMutableDictionary dictionary];
        panel.releasedWhenClosed = NO;
        panel.hidesOnDeactivate = NO;
        panel.delegate = self;
        NSString *name = [[NSFileManager defaultManager] displayNameAtPath:[item path]];
        panel.title = [NSString stringWithFormat:@"%@ Info", name];

        NSView *content = panel.contentView;
        CGFloat y = height - margin - rowHeight;
        for (NSString *row in rows) {
            [content addSubview:FinderInfoText(FinderInfoLabel(row), NSMakeRect(margin, y, labelWidth, rowHeight - 4), NSTextAlignmentRight)];
            NSTextField *value = FinderInfoText(@"", NSMakeRect(margin + labelWidth + 6, y, width - labelWidth - margin * 2 - 6, rowHeight - 4),
                                                NSTextAlignmentLeft);
            [content addSubview:value];
            _values[row] = value;
            y -= rowHeight;
        }
        [self fillValues];
        [panel center];
    }
    return self;
}

- (void)fillValues
{
    NSDictionary<NSURLResourceKey, id> *values = [_item resourceValuesForKeys:@[
        NSURLLocalizedTypeDescriptionKey, NSURLIsDirectoryKey, NSURLIsPackageKey, NSURLFileSizeKey,
        NSURLCreationDateKey, NSURLContentModificationDateKey
    ] error:NULL];
    [self setValue:[_item lastPathComponent] forRow:@"name"];
    [self setValue:values[NSURLLocalizedTypeDescriptionKey] ?: @"--" forRow:@"kind"];
    [self setValue:[[_item URLByDeletingLastPathComponent] path] forRow:@"where"];
    [self setValue:FinderInfoDate(values[NSURLCreationDateKey]) forRow:@"created"];
    [self setValue:FinderInfoDate(values[NSURLContentModificationDateKey]) forRow:@"modified"];

    if ([values[NSURLIsDirectoryKey] boolValue]) {
        [self setValue:@"Calculating…" forRow:@"size"];
        [self countFolder];
    } else {
        int64_t bytes = [values[NSURLFileSizeKey] longLongValue];
        NSNumberFormatter *grouping = [[NSNumberFormatter alloc] init];
        grouping.numberStyle = NSNumberFormatterDecimalStyle;
        [self setValue:[NSString stringWithFormat:@"%@ (%@ bytes)", FinderInfoBytes(bytes), [grouping stringFromNumber:@(bytes)]]
                forRow:@"size"];
    }
}

// Every file and folder inside counts as an item; the size is the files' sizes added up
- (void)countFolder
{
    _counting = YES;
    FinderInfoCancellation *cancellation = [[FinderInfoCancellation alloc] init];
    _cancellation = cancellation;
    NSURL *folder = _item;
    __weak FinderInfoPanelController *weakSelf = self;
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_UTILITY, 0), ^{
        int64_t bytes = 0, items = 0;
        NSDirectoryEnumerator *enumerator =
            [[NSFileManager defaultManager] enumeratorAtURL:folder
                                 includingPropertiesForKeys:@[ NSURLIsRegularFileKey, NSURLFileSizeKey ]
                                                    options:0
                                               errorHandler:^BOOL(NSURL *url, NSError *error) {
                                                   return YES;
                                               }];
        for (NSURL *url in enumerator) {
            if (atomic_load(&cancellation->cancelled))
                return;
            items++;
            NSNumber *regular = nil, *size = nil;
            [url getResourceValue:&regular forKey:NSURLIsRegularFileKey error:NULL];
            if ([regular boolValue]) {
                [url getResourceValue:&size forKey:NSURLFileSizeKey error:NULL];
                bytes += [size longLongValue];
            }
        }
        dispatch_async(dispatch_get_main_queue(), ^{
            FinderInfoPanelController *controller = weakSelf;
            if (controller == nil || atomic_load(&cancellation->cancelled))
                return;
            NSString *count = items == 1 ? @"1 item" : [NSString stringWithFormat:@"%lld items", (long long) items];
            [controller setValue:[NSString stringWithFormat:@"%@ for %@", FinderInfoBytes(bytes), count] forRow:@"size"];
            controller->_counting = NO;
        });
    });
}

- (void)setValue:(NSString *)value forRow:(NSString *)row
{
    _values[row].stringValue = value;
}

- (NSString *)valueForRow:(NSString *)row
{
    return _values[row].stringValue;
}

- (void)windowWillClose:(NSNotification *)notification
{
    if (_cancellation != nil)
        atomic_store(&_cancellation->cancelled, true);
    _counting = NO;
    // Released once nothing else holds it
    dispatch_async(dispatch_get_main_queue(), ^{
        [FinderInfoPanels() removeObjectForKey:self->_key];
    });
}

@end
