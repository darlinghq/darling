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

@class FinderWindowController;

// The Finder: one window per folder; quits when its last window closes.
@interface FinderAppDelegate : NSObject <NSApplicationDelegate>

// The window showing this folder. With create, one is made (and shown) if there is none; if the folder can't be read,
// an alert says so and the result is nil.
- (FinderWindowController *)windowForFolder:(NSURL *)folder create:(BOOL)create;

// Opens what the Finder was asked to open: a folder or volume opens in its window; anything else is revealed, in its
// folder's window with it selected. Items that don't exist are skipped.
- (void)openURLs:(NSArray<NSURL *> *)urls;

// The windows, front to back.
@property (readonly) NSArray<FinderWindowController *> *windowControllers;

@end
