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

#import "FinderDirectory.h"

@implementation FinderDirectory

- (instancetype)initWithURL:(NSURL *)url
{
    self = [super init];
    if (self != nil) {
        _URL = [url copy];
        _items = @[];
    }
    return self;
}

- (BOOL)reload:(NSError **)error
{
    NSArray<NSURLResourceKey> *keys = @[ NSURLIsHiddenKey, NSURLLocalizedNameKey ];
    NSArray<NSURL *> *contents = [[NSFileManager defaultManager] contentsOfDirectoryAtURL:_URL
                                                               includingPropertiesForKeys:keys
                                                                                  options:0
                                                                                    error:error];
    if (contents == nil) {
        _items = @[];
        return NO;
    }

    NSMutableArray<NSURL *> *visible = [NSMutableArray arrayWithCapacity:[contents count]];
    NSMutableDictionary<NSURL *, NSString *> *names = [NSMutableDictionary dictionaryWithCapacity:[contents count]];
    for (NSURL *item in contents) {
        if ([[item lastPathComponent] hasPrefix:@"."])
            continue;
        NSNumber *hidden = nil;
        if ([item getResourceValue:&hidden forKey:NSURLIsHiddenKey error:NULL] && [hidden boolValue])
            continue;
        NSString *name = nil;
        if (![item getResourceValue:&name forKey:NSURLLocalizedNameKey error:NULL] || name == nil)
            name = [item lastPathComponent];
        names[item] = name;
        [visible addObject:item];
    }
    [visible sortUsingComparator:^NSComparisonResult(NSURL *a, NSURL *b) {
        return [names[a] localizedStandardCompare:names[b]];
    }];
    _items = [visible copy];
    return YES;
}

+ (NSString *)displayNameForURL:(NSURL *)url
{
    return [[NSFileManager defaultManager] displayNameAtPath:[url path]];
}

@end
