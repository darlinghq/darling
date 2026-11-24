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

#import <AppKit/AppKit.h>
#import "FinderFileOperation.h"

// Runs Finder's copies and moves: a progress panel for one still running after progressDelay, with Stop; an alert
// for each name conflict (Replace, Keep Both, Stop; Apply to All while more items remain); an alert for a transfer
// that is refused or fails.
@interface FinderOperationQueue : NSObject

+ (instancetype)sharedQueue;

// Starts a transfer. One that is refused (a folder into itself) gets an alert, and the result is nil. window is the
// window the transfer was asked for in; nil for none.
- (FinderFileOperation *)transferItems:(NSArray<NSURL *> *)items
                                    to:(NSURL *)folder
                                  kind:(FinderTransferKind)kind
                                window:(NSWindow *)window;

@property (readonly) NSArray<FinderFileOperation *> *runningOperations;

// 1.0 s
@property NSTimeInterval progressDelay;

// The operation's progress panel; nil until it is shown, and after it closed.
- (NSPanel *)progressPanelForOperation:(FinderFileOperation *)operation;

// Runs an alert (or hands it to alertResponder); the index of the button pressed, 0 for the first.
- (NSInteger)runAlert:(NSAlert *)alert;

// Answers alerts instead of the user (tests): given the alert, returns the index of the button to press and whether to
// tick its suppression button. nil, the default, runs the alert.
@property (copy) NSInteger (^alertResponder)(NSAlert *alert, BOOL *suppress);

@end
