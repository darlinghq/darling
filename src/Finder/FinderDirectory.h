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

#import <Foundation/Foundation.h>

// The items of a folder that a Finder window shows: hidden items left out (names starting with "." and items whose
// NSURLIsHiddenKey is set), sorted by their localized names as the Finder sorts names.
@interface FinderDirectory : NSObject

- (instancetype)initWithURL:(NSURL *)url;

@property (readonly) NSURL *URL;

// The items as of the last successful -reload:; empty before it.
@property (readonly, copy) NSArray<NSURL *> *items;

// Reads the folder. Returns NO, with the error, if it can't be read; the items are then empty.
- (BOOL)reload:(NSError **)error;

// The name a Finder window shows for a folder or file.
+ (NSString *)displayNameForURL:(NSURL *)url;

@end
