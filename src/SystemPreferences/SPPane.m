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

#import "SPPane.h"

static NSString * const kIconLabelKey = @"NSPrefPaneIconLabel";
static NSString * const kIconFileKey = @"NSPrefPaneIconFile";
static NSString * const kSearchKey = @"NSPrefPaneSearchParameters";

@implementation SPPane

@synthesize icon = _icon;

- (instancetype)initWithPath:(NSString *)path
{
    self = [super init];
    if (self == nil)
        return nil;

    _bundle = [NSBundle bundleWithPath:path];
    NSString *identifier = [_bundle bundleIdentifier];
    if (_bundle == nil || [identifier length] == 0)
        return nil;

    _path = [path copy];
    _identifier = [identifier copy];

    id label = [_bundle objectForInfoDictionaryKey:kIconLabelKey];
    if (![label isKindOfClass:[NSString class]] || [label length] == 0)
        label = [_bundle objectForInfoDictionaryKey:@"CFBundleName"];
    if (![label isKindOfClass:[NSString class]] || [label length] == 0)
        label = [[path lastPathComponent] stringByDeletingPathExtension];
    _label = [label copy];

    // the window title is the bundle's name; the label is for the grid (and may have line breaks)
    id title = [_bundle objectForInfoDictionaryKey:@"CFBundleName"];
    _title = [title isKindOfClass:[NSString class]] && [title length] != 0 ? [title copy] : _label;

    id keywords = [_bundle objectForInfoDictionaryKey:kSearchKey];
    _searchKeywords = [keywords isKindOfClass:[NSString class]] ? [keywords copy] : @"";
    return self;
}

- (NSImage *)icon
{
    if (_icon != nil)
        return _icon;

    id file = [_bundle objectForInfoDictionaryKey:kIconFileKey];
    if (![file isKindOfClass:[NSString class]])
        file = [_bundle objectForInfoDictionaryKey:@"CFBundleIconFile"];

    NSImage *image = nil;
    if ([file isKindOfClass:[NSString class]] && [file length] != 0) {
        NSString *name = file;
        NSString *extension = [name pathExtension];
        NSString *resource = [extension length] != 0 ? [name stringByDeletingPathExtension] : name;
        NSString *path = [_bundle pathForResource:resource ofType:([extension length] != 0 ? extension : @"icns")];
        if (path == nil && [extension length] == 0)
            path = [_bundle pathForResource:resource ofType:@"tiff"];
        if (path != nil)
            image = [[NSImage alloc] initWithContentsOfFile:path];
    }
    _icon = image;
    return _icon;
}

- (BOOL)matchesSearch:(NSString *)text
{
    NSString *needle = [text stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
    if ([needle length] == 0)
        return YES;
    NSStringCompareOptions options = NSCaseInsensitiveSearch | NSDiacriticInsensitiveSearch;
    return [_label rangeOfString:needle options:options].location != NSNotFound
        || [_searchKeywords rangeOfString:needle options:options].location != NSNotFound;
}

@end
