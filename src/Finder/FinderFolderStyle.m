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

// The records used, as the public descriptions of the format give them (see tests/fixtures/dsstore.py):
// - "." 'bwsp': a property list of the window's settings (WindowBounds, ShowToolbar, ShowSidebar);
// - "." 'icvp': a property list of the icon view's (iconSize, textSize, labelOnBottom, backgroundType 0 default,
//   1 colour (backgroundColorRed/Green/Blue), 2 picture (backgroundImageAlias: alias or bookmark data));
// - "." 'vstl': the view ('Nlsv' the list); 'lsvp' the list view's settings;
// - older: "." 'icvo' ("icv4", icon size, arrangement, label position) and 'BKGD' ('DefB', or 'ClrB' and a 16-bit
//   colour);
// - an item's 'Iloc': the centre of its icon.

#import "FinderFolderStyle.h"
#import "FinderDSStore.h"
#include <math.h>
#include <stdlib.h>
#include <sys/stat.h>

static const CGFloat FinderDefaultIconSize = 64, FinderDefaultTextSize = 12;

static CGFloat FinderClamp(CGFloat value, CGFloat low, CGFloat high)
{
    return value < low ? low : value > high ? high : value;
}

static id FinderPropertyList(FinderDSStoreRecord *record)
{
    if (record.type != 'blob')
        return nil;
    id list = [NSPropertyListSerialization propertyListWithData:record.value options:NSPropertyListImmutable format:NULL error:NULL];
    return [list isKindOfClass:[NSDictionary class]] ? list : nil;
}

// A number setting; one that isn't finite (a property list can hold NaN and infinities) counts as missing
static NSNumber *FinderNumber(NSDictionary *dictionary, NSString *key)
{
    id value = dictionary[key];
    return [value isKindOfClass:[NSNumber class]] && isfinite([value doubleValue]) ? value : nil;
}

static uint16_t FinderUInt16(const uint8_t *bytes)
{
    return (uint16_t)((bytes[0] << 8) | bytes[1]);
}

static uint32_t FinderLittle32(const uint8_t *bytes)
{
    return bytes[0] | ((uint32_t)bytes[1] << 8) | ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[3] << 24);
}

// The path inside its volume that version 2 or 3 alias data names: tag 18 (the POSIX path in the volume), else tag 2
// (the Carbon path, "Volume:folder:file"). Tagged items follow the 150-byte fixed part: a 2-byte tag, a 2-byte length
// and the data, padded to an even length; tag 0xffff ends them.
static NSString *FinderAliasPath(NSData *data)
{
    const uint8_t *bytes = [data bytes];
    size_t length = [data length], position = 150;
    if (length < position)
        return nil;
    uint16_t version = FinderUInt16(bytes + 6);
    if (version != 2 && version != 3)
        return nil;
    NSString *carbon = nil;
    while (position + 4 <= length) {
        uint16_t tag = FinderUInt16(bytes + position), size = FinderUInt16(bytes + position + 2);
        position += 4;
        if (tag == 0xffff || size > length - position)
            break;
        NSString *text = [[NSString alloc] initWithBytes:bytes + position length:size encoding:NSUTF8StringEncoding];
        if (tag == 18 && text != nil)
            return text;
        if (tag == 2 && text != nil)
            carbon = text;
        position += size + (size & 1);
    }
    NSRange colon = [carbon rangeOfString:@":"];
    if (colon.location == NSNotFound)
        return nil;
    return [[carbon substringFromIndex:colon.location] stringByReplacingOccurrencesOfString:@":" withString:@"/"];
}

// An item of bookmark data, at an offset from the start of its data area: a length, a type, the bytes
static BOOL FinderBookmarkItem(const uint8_t *area, size_t areaLength, uint32_t offset, uint32_t *type,
                               const uint8_t **itemBytes, uint32_t *itemLength)
{
    if (offset > areaLength || areaLength - offset < 8)
        return NO;
    *itemLength = FinderLittle32(area + offset);
    *type = FinderLittle32(area + offset + 4);
    if (*itemLength > areaLength - offset - 8)
        return NO;
    *itemBytes = area + offset + 8;
    return YES;
}

static NSString *FinderBookmarkString(const uint8_t *area, size_t areaLength, uint32_t offset)
{
    uint32_t type, length;
    const uint8_t *bytes;
    if (!FinderBookmarkItem(area, areaLength, offset, &type, &bytes, &length) || type != 0x0101)
        return nil;
    return [[NSString alloc] initWithBytes:bytes length:length encoding:NSUTF8StringEncoding];
}

// The path inside its volume that bookmark data names: its path components (0x1004) without those of the volume's
// path (0x2002), from the first table of contents
static NSString *FinderBookmarkPath(NSData *data)
{
    const uint8_t *bytes = [data bytes];
    size_t length = [data length];
    if (length < 16 || memcmp(bytes, "book", 4) != 0)
        return nil;
    uint32_t headerLength = FinderLittle32(bytes + 12);
    if (headerLength > length || length - headerLength < 4)
        return nil;
    const uint8_t *area = bytes + headerLength;
    size_t areaLength = length - headerLength;
    uint32_t toc = FinderLittle32(area);
    if (toc > areaLength || areaLength - toc < 20 || FinderLittle32(area + toc + 4) != 0xfffffffe)
        return nil;
    uint32_t count = FinderLittle32(area + toc + 16);
    if (count > (areaLength - toc - 20) / 12)
        return nil;

    NSMutableArray<NSString *> *components = nil;
    NSString *volumePath = nil;
    for (uint32_t i = 0; i < count; i++) {
        const uint8_t *entry = area + toc + 20 + i * 12;
        uint32_t key = FinderLittle32(entry), offset = FinderLittle32(entry + 4);
        if (key == 0x1004 && components == nil) {
            uint32_t type, arrayLength;
            const uint8_t *array;
            if (!FinderBookmarkItem(area, areaLength, offset, &type, &array, &arrayLength) || type != 0x0601)
                return nil;
            components = [NSMutableArray array];
            for (uint32_t j = 0; j + 4 <= arrayLength; j += 4) {
                NSString *component = FinderBookmarkString(area, areaLength, FinderLittle32(array + j));
                if (component == nil)
                    return nil;
                [components addObject:component];
            }
        } else if (key == 0x2002 && volumePath == nil) {
            volumePath = FinderBookmarkString(area, areaLength, offset);
        }
    }
    if (components == nil || volumePath == nil)
        return nil;
    NSArray<NSString *> *volume = [[volumePath pathComponents] filteredArrayUsingPredicate:
        [NSPredicate predicateWithFormat:@"SELF != '/'"]];
    if ([volume count] > [components count] || ![[components subarrayWithRange:NSMakeRange(0, [volume count])] isEqual:volume])
        return nil;
    NSArray *inside = [components subarrayWithRange:NSMakeRange([volume count], [components count] - [volume count])];
    return [@"/" stringByAppendingString:[inside componentsJoinedByString:@"/"]];
}

// A path inside the volume, as a URL: no empty or ".." components, a regular file once symbolic links are resolved,
// and still inside the volume
static NSURL *FinderFileInVolume(NSString *path, NSURL *volume)
{
    NSArray<NSString *> *components = [path componentsSeparatedByString:@"/"];
    if ([components count] < 2 || [components[0] length] != 0)
        return nil;
    components = [components subarrayWithRange:NSMakeRange(1, [components count] - 1)];
    for (NSString *component in components) {
        if ([component length] == 0 || [component isEqual:@".."] || [component isEqual:@"."])
            return nil;
    }

    char volumeReal[PATH_MAX], fileReal[PATH_MAX];
    NSString *joined = [[volume path] stringByAppendingPathComponent:[components componentsJoinedByString:@"/"]];
    if (realpath([[volume path] fileSystemRepresentation], volumeReal) == NULL
        || realpath([joined fileSystemRepresentation], fileReal) == NULL)
        return nil;
    size_t volumeLength = strlen(volumeReal);
    BOOL inside = strncmp(fileReal, volumeReal, volumeLength) == 0
        && (fileReal[volumeLength] == '/' || (volumeLength == 1 && volumeReal[0] == '/'));
    struct stat info;
    if (!inside || stat(fileReal, &info) != 0 || !S_ISREG(info.st_mode))
        return nil;
    return [NSURL fileURLWithFileSystemRepresentation:fileReal isDirectory:NO relativeToURL:nil];
}

@implementation FinderFolderStyle {
    FinderDSStore *_store;
}

+ (instancetype)styleForFolder:(NSURL *)folder
{
    NSURL *volume = nil;
    if (![folder getResourceValue:&volume forKey:NSURLVolumeURLKey error:NULL] || volume == nil)
        volume = [NSURL fileURLWithPath:@"/" isDirectory:YES];
    return [self styleForFolder:folder volume:volume];
}

+ (instancetype)styleForFolder:(NSURL *)folder volume:(NSURL *)volume
{
    FinderDSStore *store = [FinderDSStore storeForFolder:folder];
    if (store == nil)
        return nil;
    BOOL usable = NO;
    for (FinderDSStoreRecord *record in store.records) {
        FourCharCode code = record.code;
        if (code == 'Iloc' || ([record.name isEqual:@"."] && (code == 'bwsp' || code == 'icvp' || code == 'icvo'
                                                             || code == 'BKGD' || code == 'vstl' || code == 'lsvp'))) {
            usable = YES;
            break;
        }
    }
    return usable ? [[self alloc] initWithStore:store volume:volume] : nil;
}

- (instancetype)initWithStore:(FinderDSStore *)store volume:(NSURL *)volume
{
    if (!(self = [super init]))
        return nil;
    _store = store;
    _showsToolbar = YES;
    _showsSidebar = YES;
    _iconSize = FinderDefaultIconSize;
    _textSize = FinderDefaultTextSize;
    _labelOnBottom = YES;

    NSDictionary *window = FinderPropertyList([store recordForName:@"." code:'bwsp']);
    id bounds = window[@"WindowBounds"];
    if ([bounds isKindOfClass:[NSString class]]) {
        NSSize size = NSRectFromString(bounds).size;
        if (size.width > 0 && size.height > 0 && isfinite(size.width) && isfinite(size.height))
            _windowContentSize = size;
    }
    if (FinderNumber(window, @"ShowToolbar"))
        _showsToolbar = [FinderNumber(window, @"ShowToolbar") boolValue];
    if (FinderNumber(window, @"ShowSidebar"))
        _showsSidebar = [FinderNumber(window, @"ShowSidebar") boolValue];

    FinderDSStoreRecord *view = [store recordForName:@"." code:'vstl'];
    if (view != nil)
        _viewStyle = view.type == 'type' && [view.value isEqual:@"Nlsv"] ? FinderViewStyleList : FinderViewStyleIcons;
    else if ([store recordForName:@"." code:'lsvp'] != nil && [store recordForName:@"." code:'vSrn'] != nil)
        _viewStyle = FinderViewStyleList;

    FinderDSStoreRecord *icons = [store recordForName:@"." code:'icvp'];
    if (icons != nil)
        [self readIconView:FinderPropertyList(icons) volume:volume];
    else
        [self readOlderIconView:[store recordForName:@"." code:'icvo'] background:[store recordForName:@"." code:'BKGD']
                         volume:volume];
    return self;
}

- (void)readIconView:(NSDictionary *)icons volume:(NSURL *)volume
{
    if (FinderNumber(icons, @"iconSize"))
        _iconSize = FinderClamp([FinderNumber(icons, @"iconSize") doubleValue], 16, 512);
    if (FinderNumber(icons, @"textSize"))
        _textSize = FinderClamp([FinderNumber(icons, @"textSize") doubleValue], 10, 16);
    if (FinderNumber(icons, @"labelOnBottom"))
        _labelOnBottom = [FinderNumber(icons, @"labelOnBottom") boolValue];

    NSInteger background = [FinderNumber(icons, @"backgroundType") integerValue];
    NSNumber *red = FinderNumber(icons, @"backgroundColorRed"), *green = FinderNumber(icons, @"backgroundColorGreen"),
             *blue = FinderNumber(icons, @"backgroundColorBlue");
    if ((background == 1 || background == 2) && red && green && blue)
        _backgroundColor = [NSColor colorWithCalibratedRed:FinderClamp([red doubleValue], 0, 1)
                                                     green:FinderClamp([green doubleValue], 0, 1)
                                                      blue:FinderClamp([blue doubleValue], 0, 1)
                                                     alpha:1];
    id picture = icons[@"backgroundImageAlias"];
    if (background == 2 && [picture isKindOfClass:[NSData class]]) {
        NSString *path = FinderBookmarkPath(picture) ?: FinderAliasPath(picture);
        _backgroundImageURL = path ? FinderFileInVolume(path, volume) : nil;
    }
}

// 'icvo' "icv4": the icon size (2 bytes), the arrangement (4), the label position (4: "botm" or "rght"). 'BKGD':
// "ClrB" and the colour as 16-bit red, green and blue; "PctB" with the alias in the "." 'pict' record.
- (void)readOlderIconView:(FinderDSStoreRecord *)icons background:(FinderDSStoreRecord *)background volume:(NSURL *)volume
{
    if (icons.type == 'blob' && [icons.value length] >= 14 && memcmp([icons.value bytes], "icv4", 4) == 0) {
        const uint8_t *bytes = [icons.value bytes];
        _iconSize = FinderClamp(FinderUInt16(bytes + 4), 16, 512);
        _labelOnBottom = memcmp(bytes + 10, "rght", 4) != 0;
    }
    if (background.type == 'blob' && [background.value length] >= 12) {
        const uint8_t *bytes = [background.value bytes];
        if (memcmp(bytes, "ClrB", 4) == 0) {
            _backgroundColor = [NSColor colorWithCalibratedRed:FinderUInt16(bytes + 4) / 65535.0
                                                         green:FinderUInt16(bytes + 6) / 65535.0
                                                          blue:FinderUInt16(bytes + 8) / 65535.0
                                                         alpha:1];
        } else if (memcmp(bytes, "PctB", 4) == 0) {
            FinderDSStoreRecord *alias = [_store recordForName:@"." code:'pict'];
            NSString *path = alias.type == 'blob' ? FinderAliasPath(alias.value) : nil;
            _backgroundImageURL = path ? FinderFileInVolume(path, volume) : nil;
        }
    }
}

- (BOOL)getCenter:(NSPoint *)center forItemNamed:(NSString *)name
{
    FinderDSStoreRecord *record = [_store recordForName:name code:'Iloc'];
    if (record.type != 'blob' || [record.value length] != 16)
        return NO;
    const uint8_t *bytes = [record.value bytes];
    uint32_t x = ((uint32_t)bytes[0] << 24) | ((uint32_t)bytes[1] << 16) | ((uint32_t)bytes[2] << 8) | bytes[3];
    uint32_t y = ((uint32_t)bytes[4] << 24) | ((uint32_t)bytes[5] << 16) | ((uint32_t)bytes[6] << 8) | bytes[7];
    // A coordinate past 2^31 is not a position (no screen is that large)
    if (x > INT32_MAX || y > INT32_MAX)
        return NO;
    *center = NSMakePoint(x, y);
    return YES;
}

@end
