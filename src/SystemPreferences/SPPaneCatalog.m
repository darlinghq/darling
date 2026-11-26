/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#import "SPPaneCatalog.h"

@implementation SPPaneCatalog {
    NSArray<NSString *> *_directories;
    NSArray<NSDictionary *> *_groupDefinitions;
    NSString *_catchAll;
    NSMutableArray<SPPane *> *_panes;
    NSMutableDictionary<NSString *, SPPane *> *_byIdentifier;
    NSMutableArray<SPPane *> *_added;
}

+ (NSArray<NSString *> *)defaultSearchDirectories
{
    return @[
        @"/System/Library/PreferencePanes",
        @"/Library/PreferencePanes",
        [NSHomeDirectory() stringByAppendingPathComponent:@"Library/PreferencePanes"],
    ];
}

- (instancetype)initWithSearchDirectories:(NSArray<NSString *> *)directories groupsPath:(NSString *)groupsPath
{
    self = [super init];
    if (self == nil)
        return nil;
    _directories = [directories copy];
    NSDictionary *groups = groupsPath != nil ? [NSDictionary dictionaryWithContentsOfFile:groupsPath] : nil;
    _groupDefinitions = [groups[@"groups"] isKindOfClass:[NSArray class]] ? groups[@"groups"] : @[];
    _catchAll = [groups[@"catchAllGroupID"] isKindOfClass:[NSString class]] ? [groups[@"catchAllGroupID"] copy] : @"addons";
    _added = [NSMutableArray array];
    [self reload];
    return self;
}

- (NSString *)catchAllGroupIdentifier
{
    return _catchAll;
}

- (void)reload
{
    NSMutableArray *panes = [NSMutableArray array];
    NSMutableDictionary *byIdentifier = [NSMutableDictionary dictionary];
    NSFileManager *manager = [NSFileManager defaultManager];

    for (NSString *directory in _directories) {
        NSArray *names = [[manager contentsOfDirectoryAtPath:directory error:NULL] sortedArrayUsingSelector:@selector(compare:)];
        for (NSString *name in names) {
            if (![[name pathExtension] isEqualToString:@"prefPane"])
                continue;
            SPPane *pane = [[SPPane alloc] initWithPath:[directory stringByAppendingPathComponent:name]];
            // an earlier directory wins over a later one with the same identifier
            if (pane == nil || byIdentifier[pane.identifier] != nil)
                continue;
            byIdentifier[pane.identifier] = pane;
            [panes addObject:pane];
        }
    }
    for (SPPane *pane in _added) {
        if (byIdentifier[pane.identifier] == nil) {
            byIdentifier[pane.identifier] = pane;
            [panes addObject:pane];
        }
    }

    // group membership: the listed groups first, the rest in the catch-all group
    NSMutableSet *listed = [NSMutableSet set];
    for (NSDictionary *group in _groupDefinitions) {
        for (NSString *identifier in group[@"panes"]) {
            SPPane *pane = byIdentifier[identifier];
            if (pane != nil) {
                pane.groupIdentifier = group[@"identifier"];
                [listed addObject:identifier];
            }
        }
    }
    for (SPPane *pane in panes) {
        if (![listed containsObject:pane.identifier])
            pane.groupIdentifier = _catchAll;
    }
    _panes = panes;
    _byIdentifier = byIdentifier;
}

- (NSArray<NSString *> *)groupIdentifiers
{
    NSMutableArray *result = [NSMutableArray array];
    for (NSDictionary *group in _groupDefinitions) {
        NSString *identifier = group[@"identifier"];
        if ([[self panesInGroup:identifier] count] != 0 && ![result containsObject:identifier])
            [result addObject:identifier];
    }
    if ([[self panesInGroup:_catchAll] count] != 0 && ![result containsObject:_catchAll])
        [result addObject:_catchAll];
    return result;
}

- (NSArray<SPPane *> *)panesInGroup:(NSString *)groupIdentifier
{
    NSMutableArray *result = [NSMutableArray array];
    for (NSDictionary *group in _groupDefinitions) {
        if (![group[@"identifier"] isEqualToString:groupIdentifier])
            continue;
        // the group's own order
        for (NSString *identifier in group[@"panes"]) {
            SPPane *pane = _byIdentifier[identifier];
            if (pane != nil && [pane.groupIdentifier isEqualToString:groupIdentifier])
                [result addObject:pane];
        }
    }
    if ([groupIdentifier isEqualToString:_catchAll]) {
        NSMutableArray *others = [NSMutableArray array];
        for (SPPane *pane in _panes) {
            if ([pane.groupIdentifier isEqualToString:_catchAll] && ![result containsObject:pane])
                [others addObject:pane];
        }
        [others sortUsingComparator:^NSComparisonResult(SPPane *a, SPPane *b) {
            return [a.label localizedCaseInsensitiveCompare:b.label];
        }];
        [result addObjectsFromArray:others];
    }
    return result;
}

- (NSArray<SPPane *> *)allPanes
{
    return _panes;
}

- (SPPane *)paneWithIdentifier:(NSString *)identifier
{
    return identifier != nil ? _byIdentifier[identifier] : nil;
}

- (SPPane *)paneByAddingBundleAtPath:(NSString *)path
{
    SPPane *pane = [[SPPane alloc] initWithPath:path];
    if (pane == nil)
        return nil;
    SPPane *known = _byIdentifier[pane.identifier];
    if (known != nil)
        return known;
    [_added addObject:pane];
    [self reload];
    return [self paneWithIdentifier:pane.identifier];
}

@end
