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

#import "FinderOperationQueue.h"
#include <string.h>

static NSString *FinderDisplayName(NSURL *url)
{
    return [[NSFileManager defaultManager] displayNameAtPath: [url path]];
}

// A running operation's progress panel: what is copied (or moved) where, a determinate indicator, how much of how
// much, and Stop.
@interface FinderProgressPanel : NSObject
@property (readonly) NSPanel *panel;
- (instancetype)initWithOperation:(FinderFileOperation *)operation;
- (void)setDone:(int64_t)done ofTotal:(int64_t)total;
@end

@implementation FinderProgressPanel {
    __weak FinderFileOperation *_operation;
    NSProgressIndicator *_bar;
    NSTextField *_amount;
}

static NSTextField *FinderLabel(NSString *text, NSRect frame)
{
    NSTextField *label = [[NSTextField alloc] initWithFrame: frame];
    label.stringValue = text;
    label.editable = NO;
    label.selectable = NO;
    label.bordered = NO;
    label.drawsBackground = NO;
    return label;
}

- (instancetype)initWithOperation:(FinderFileOperation *)operation
{
    if ((self = [super init])) {
        _operation = operation;
        BOOL move = operation.kind == FinderTransferMove;
        _panel = [[NSPanel alloc] initWithContentRect: NSMakeRect(0, 0, 440, 100)
                                            styleMask: NSWindowStyleMaskTitled
                                              backing: NSBackingStoreBuffered
                                                defer: NO];
        _panel.title = move ? @"Move" : @"Copy";
        _panel.releasedWhenClosed = NO;
        _panel.hidesOnDeactivate = NO;

        NSString *verb = move ? @"Moving" : @"Copying";
        NSString *what = [operation.items count] == 1
            ? [NSString stringWithFormat: @"“%@”", FinderDisplayName(operation.items[0])]
            : [NSString stringWithFormat: @"%lu items", (unsigned long) [operation.items count]];
        NSString *text = [NSString stringWithFormat: @"%@ %@ to “%@”", verb, what, FinderDisplayName(operation.destination)];

        NSView *content = _panel.contentView;
        [content addSubview: FinderLabel(text, NSMakeRect(20, 68, 400, 20))];
        _bar = [[NSProgressIndicator alloc] initWithFrame: NSMakeRect(20, 46, 320, 16)];
        _bar.indeterminate = NO;
        _bar.minValue = 0;
        _bar.maxValue = 1;
        _bar.doubleValue = 0;
        [content addSubview: _bar];
        _amount = FinderLabel(@"", NSMakeRect(20, 20, 320, 18));
        [content addSubview: _amount];
        NSButton *stop = [[NSButton alloc] initWithFrame: NSMakeRect(350, 40, 70, 28)];
        stop.title = @"Stop";
        stop.bezelStyle = NSBezelStyleRounded;
        stop.target = self;
        stop.action = @selector(stop:);
        [content addSubview: stop];
        [_panel center];
    }
    return self;
}

- (void)setDone:(int64_t)done ofTotal:(int64_t)total
{
    _bar.doubleValue = total > 0 ? (double) done / (double) total : 0;
    NSString *of = [NSByteCountFormatter stringFromByteCount: done countStyle: NSByteCountFormatterCountStyleFile];
    NSString *all = [NSByteCountFormatter stringFromByteCount: total countStyle: NSByteCountFormatterCountStyleFile];
    _amount.stringValue = [NSString stringWithFormat: @"%@ of %@", of, all];
}

- (void)stop:(id)sender
{
    [_operation cancel];
}

@end

@implementation FinderOperationQueue {
    NSMutableArray<FinderFileOperation *> *_running;
    NSMapTable<FinderFileOperation *, FinderProgressPanel *> *_panels;
}

+ (instancetype)sharedQueue
{
    static FinderOperationQueue *queue;
    static dispatch_once_t once;
    dispatch_once(&once, ^{
        queue = [[FinderOperationQueue alloc] init];
    });
    return queue;
}

- (instancetype)init
{
    if ((self = [super init])) {
        _running = [NSMutableArray array];
        _panels = [NSMapTable strongToStrongObjectsMapTable];
        _progressDelay = 1.0;
    }
    return self;
}

- (NSArray<FinderFileOperation *> *)runningOperations
{
    return [_running copy];
}

- (NSPanel *)progressPanelForOperation:(FinderFileOperation *)operation
{
    return [[_panels objectForKey: operation] panel];
}

// Alerts are app-modal, also for a transfer asked for in a window: the engine waits for the answer, which a sheet
// would give only later.
- (NSInteger)runAlert:(NSAlert *)alert
{
    if (self.alertResponder) {
        BOOL suppress = NO;
        NSInteger index = self.alertResponder(alert, &suppress);
        if (alert.showsSuppressionButton)
            alert.suppressionButton.state = suppress ? NSControlStateValueOn : NSControlStateValueOff;
        return index;
    }
    return [alert runModal] - NSAlertFirstButtonReturn;
}

- (void)showError:(NSError *)error kind:(FinderTransferKind)kind
{
    NSAlert *alert = [[NSAlert alloc] init];
    alert.alertStyle = NSAlertStyleWarning;
    NSString *path = error.userInfo[NSFilePathErrorKey];
    NSString *description = error.userInfo[NSLocalizedDescriptionKey];
    if (description) {
        // Our own refusals say all there is to say
        alert.messageText = description;
    } else {
        NSString *name = path ? [[NSFileManager defaultManager] displayNameAtPath: path] : @"";
        alert.messageText = [NSString stringWithFormat: kind == FinderTransferMove ? @"“%@” couldn't be moved."
                                                                                   : @"“%@” couldn't be copied.", name];
        NSError *underlying = error.userInfo[NSUnderlyingErrorKey];
        if ([underlying.domain isEqual: NSPOSIXErrorDomain])
            alert.informativeText = [NSString stringWithUTF8String: strerror((int) underlying.code)];
        else
            alert.informativeText = (underlying ?: error).localizedDescription;
    }
    [alert addButtonWithTitle: @"OK"];
    [self runAlert: alert];
}

- (FinderConflictChoice)askAboutConflict:(NSURL *)source
                                existing:(NSURL *)existing
                               operation:(FinderFileOperation *)operation
                              applyToAll:(BOOL *)applyToAll
{
    NSAlert *alert = [[NSAlert alloc] init];
    alert.alertStyle = NSAlertStyleWarning;
    alert.messageText = [NSString stringWithFormat: @"An item named “%@” already exists in this location. Do you want to replace it with the one you're %@?",
                                                    FinderDisplayName(existing),
                                                    operation.kind == FinderTransferMove ? @"moving" : @"copying"];
    [alert addButtonWithTitle: @"Replace"];
    [alert addButtonWithTitle: @"Keep Both"];
    NSButton *stop = [alert addButtonWithTitle: @"Stop"];
    stop.keyEquivalent = @"\033";
    NSUInteger index = [operation.items indexOfObject: source];
    if (index != NSNotFound && index + 1 < [operation.items count]) {
        alert.showsSuppressionButton = YES;
        alert.suppressionButton.title = @"Apply to All";
    }
    NSInteger pressed = [self runAlert: alert];
    *applyToAll = alert.showsSuppressionButton && alert.suppressionButton.state == NSControlStateValueOn;
    switch (pressed) {
    case 0:
        return FinderConflictReplace;
    case 1:
        return FinderConflictKeepBoth;
    default:
        return FinderConflictStop;
    }
}

- (FinderFileOperation *)transferItems:(NSArray<NSURL *> *)items
                                    to:(NSURL *)folder
                                  kind:(FinderTransferKind)kind
                                window:(NSWindow *)window
{
    NSError *refusal = [FinderFileOperation refusalForItems: items destination: folder kind: kind];
    if (refusal) {
        [self showError: refusal kind: kind];
        return nil;
    }

    FinderFileOperation *operation = [[FinderFileOperation alloc] initWithKind: kind items: items destination: folder];
    __weak FinderFileOperation *weakOperation = operation;
    __weak FinderOperationQueue *weakSelf = self;
    operation.resolveConflict = ^FinderConflictChoice(NSURL *source, NSURL *existing, BOOL *applyToAll) {
        return [weakSelf askAboutConflict: source existing: existing operation: weakOperation applyToAll: applyToAll];
    };
    operation.progress = ^(int64_t done, int64_t total, NSURL *current) {
        FinderFileOperation *strong = weakOperation;
        if (strong)
            [[weakSelf valueForPanel: strong] setDone: done ofTotal: total];
    };
    operation.completion = ^(NSArray<NSURL *> *results, NSError *error) {
        FinderOperationQueue *queue = weakSelf;
        FinderFileOperation *strong = weakOperation;
        if (queue == nil || strong == nil)
            return;
        [[[queue valueForPanel: strong] panel] close];
        [queue->_panels removeObjectForKey: strong];
        [queue->_running removeObject: strong];
        if (error && !([error.domain isEqual: NSCocoaErrorDomain] && error.code == NSUserCancelledError))
            [queue showError: error kind: strong.kind];
    };
    [_running addObject: operation];
    [operation start];

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t) (self.progressDelay * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        FinderOperationQueue *queue = weakSelf;
        FinderFileOperation *strong = weakOperation;
        if (queue == nil || strong == nil || strong.finished || ![queue->_running containsObject: strong])
            return;
        FinderProgressPanel *panel = [[FinderProgressPanel alloc] initWithOperation: strong];
        [queue->_panels setObject: panel forKey: strong];
        [panel.panel orderFront: nil];
    });
    return operation;
}

- (FinderProgressPanel *)valueForPanel:(FinderFileOperation *)operation
{
    return [_panels objectForKey: operation];
}

@end
