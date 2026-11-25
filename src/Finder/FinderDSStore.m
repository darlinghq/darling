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

// The format is not documented by Apple. This reader follows public third-party descriptions of it (Wim Lewis's
// DSStoreFormat notes in the Mac::Finder::DSStore distribution, Mark Mentovai's description, and the documentation of
// the Python ds_store package); tests/fixtures/dsstore.py describes the layout. No code was copied.

#import "FinderDSStore.h"

enum {
    FinderDSStoreMaximumSize = 16 * 1024 * 1024,
    FinderDSStoreMaximumRecords = 100000,
    FinderDSStoreMaximumDepth = 32,
    FinderDSStoreHeaderSize = 32,
};

@interface FinderDSStoreRecord ()
- (instancetype)initWithName:(NSString *)name code:(FourCharCode)code type:(FourCharCode)type value:(id)value;
@end

@implementation FinderDSStoreRecord

- (instancetype)initWithName:(NSString *)name code:(FourCharCode)code type:(FourCharCode)type value:(id)value
{
    if ((self = [super init])) {
        _name = [name copy];
        _code = code;
        _type = type;
        _value = value;
    }
    return self;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@ %@ %c%c%c%c %@>", [self class], _name, (char)(_code >> 24),
                                      (char)(_code >> 16), (char)(_code >> 8), (char)_code, _value];
}

@end

// Reads big-endian values from one block, never past its end; a read that doesn't fit fails, and every read after a
// failed one fails too.
typedef struct {
    const uint8_t *bytes;
    size_t length;
    size_t position;
    BOOL failed;
} FinderDSStoreCursor;

static FinderDSStoreCursor FinderDSStoreCursorMake(const uint8_t *bytes, size_t length)
{
    return (FinderDSStoreCursor){ bytes, length, 0, NO };
}

static const uint8_t *FinderDSStoreTake(FinderDSStoreCursor *cursor, size_t count)
{
    if (cursor->failed || count > cursor->length - cursor->position) {
        cursor->failed = YES;
        return NULL;
    }
    const uint8_t *at = cursor->bytes + cursor->position;
    cursor->position += count;
    return at;
}

static uint32_t FinderDSStoreUInt32(FinderDSStoreCursor *cursor)
{
    const uint8_t *at = FinderDSStoreTake(cursor, 4);
    return at ? ((uint32_t)at[0] << 24) | ((uint32_t)at[1] << 16) | ((uint32_t)at[2] << 8) | at[3] : 0;
}

static uint64_t FinderDSStoreUInt64(FinderDSStoreCursor *cursor)
{
    uint64_t high = FinderDSStoreUInt32(cursor);
    return (high << 32) | FinderDSStoreUInt32(cursor);
}

// A UTF-16BE string: a code unit count, then the code units
static NSString *FinderDSStoreString(FinderDSStoreCursor *cursor)
{
    uint32_t units = FinderDSStoreUInt32(cursor);
    if (cursor->failed || units > (cursor->length - cursor->position) / 2) {
        cursor->failed = YES;
        return nil;
    }
    const uint8_t *at = FinderDSStoreTake(cursor, (size_t)units * 2);
    NSString *string = [[NSString alloc] initWithBytes:at length:(NSUInteger)units * 2 encoding:NSUTF16BigEndianStringEncoding];
    if (string == nil)
        cursor->failed = YES;
    return string;
}

@implementation FinderDSStore {
    NSData *_data;
    const uint8_t *_area; // the allocator's area: file offset 4, where block offsets count from
    size_t _areaLength;
    NSArray<NSNumber *> *_addresses;
    NSMutableIndexSet *_visited;
    NSMutableArray<FinderDSStoreRecord *> *_parsed;
    NSDictionary<NSString *, FinderDSStoreRecord *> *_index;
}

+ (instancetype)storeWithData:(NSData *)data
{
    return [[self alloc] initWithData:data];
}

+ (instancetype)storeForFolder:(NSURL *)folder
{
    NSURL *url = [folder URLByAppendingPathComponent:@".DS_Store" isDirectory:NO];
    NSNumber *size = nil;
    if (![url getResourceValue:&size forKey:NSURLFileSizeKey error:NULL] || [size unsignedLongLongValue] > FinderDSStoreMaximumSize)
        return nil;
    NSData *data = [NSData dataWithContentsOfURL:url options:NSDataReadingMappedIfSafe error:NULL];
    return data ? [self storeWithData:data] : nil;
}

// The bytes of a block, from its address: log2 of its size in the low 5 bits, its offset in the rest. Fails for a
// block that doesn't exist or isn't inside the file.
- (BOOL)block:(uint32_t)number cursor:(FinderDSStoreCursor *)cursor
{
    if (number >= [_addresses count])
        return NO;
    uint32_t address = [_addresses[number] unsignedIntValue];
    uint32_t log = address & 0x1f;
    size_t offset = address & ~(uint32_t)0x1f;
    if (address == 0 || log < 5 || log > 30 || offset > _areaLength || ((size_t)1 << log) > _areaLength - offset)
        return NO;
    *cursor = FinderDSStoreCursorMake(_area + offset, (size_t)1 << log);
    return YES;
}

- (id)valueOfType:(FourCharCode)type cursor:(FinderDSStoreCursor *)cursor
{
    switch (type) {
    case 'bool': {
        const uint8_t *at = FinderDSStoreTake(cursor, 1);
        return at ? [NSNumber numberWithBool:*at != 0] : nil;
    }
    case 'long':
    case 'shor': {
        uint32_t value = FinderDSStoreUInt32(cursor);
        return cursor->failed ? nil : [NSNumber numberWithUnsignedInt:value];
    }
    case 'comp':
    case 'dutc': {
        uint64_t value = FinderDSStoreUInt64(cursor);
        return cursor->failed ? nil : [NSNumber numberWithUnsignedLongLong:value];
    }
    case 'type': {
        const uint8_t *at = FinderDSStoreTake(cursor, 4);
        return at ? [[NSString alloc] initWithBytes:at length:4 encoding:NSMacOSRomanStringEncoding] : nil;
    }
    case 'blob': {
        uint32_t length = FinderDSStoreUInt32(cursor);
        if (cursor->failed || length > cursor->length - cursor->position)
            return nil;
        return [NSData dataWithBytes:FinderDSStoreTake(cursor, length) length:length];
    }
    case 'ustr':
        return FinderDSStoreString(cursor);
    default:
        return nil;
    }
}

- (BOOL)readRecord:(FinderDSStoreCursor *)cursor
{
    NSString *name = FinderDSStoreString(cursor);
    FourCharCode code = FinderDSStoreUInt32(cursor);
    FourCharCode type = FinderDSStoreUInt32(cursor);
    if (cursor->failed)
        return NO;
    id value = [self valueOfType:type cursor:cursor];
    if (value == nil || [_parsed count] >= FinderDSStoreMaximumRecords)
        return NO;
    [_parsed addObject:[[FinderDSStoreRecord alloc] initWithName:name code:code type:type value:value]];
    return YES;
}

// A node: P and a count. P = 0 is a leaf of count records; otherwise count pairs of (child, record) follow, the
// record coming after the child's subtree, and P is the last child.
- (BOOL)readNode:(uint32_t)number depth:(int)depth
{
    FinderDSStoreCursor cursor;
    if (depth > FinderDSStoreMaximumDepth || [_visited containsIndex:number] || ![self block:number cursor:&cursor])
        return NO;
    [_visited addIndex:number];

    uint32_t last = FinderDSStoreUInt32(&cursor);
    uint32_t count = FinderDSStoreUInt32(&cursor);
    if (cursor.failed)
        return NO;
    for (uint32_t i = 0; i < count; i++) {
        if (last != 0) {
            uint32_t child = FinderDSStoreUInt32(&cursor);
            if (cursor.failed || ![self readNode:child depth:depth + 1])
                return NO;
        }
        if (![self readRecord:&cursor])
            return NO;
    }
    return last == 0 || [self readNode:last depth:depth + 1];
}

- (instancetype)initWithData:(NSData *)data
{
    if (!(self = [super init]))
        return nil;
    if ([data length] < 4 + FinderDSStoreHeaderSize || [data length] > FinderDSStoreMaximumSize)
        return nil;
    _data = data;
    const uint8_t *bytes = [data bytes];
    if (bytes[0] != 0 || bytes[1] != 0 || bytes[2] != 0 || bytes[3] != 1)
        return nil;
    _area = bytes + 4;
    _areaLength = [data length] - 4;

    // The header: "Bud1", then the allocator's bookkeeping block (offset, size, offset again)
    FinderDSStoreCursor header = FinderDSStoreCursorMake(_area, FinderDSStoreHeaderSize);
    uint32_t magic = FinderDSStoreUInt32(&header);
    uint32_t infoOffset = FinderDSStoreUInt32(&header);
    uint32_t infoSize = FinderDSStoreUInt32(&header);
    if (magic != 'Bud1' || infoOffset != FinderDSStoreUInt32(&header) || infoOffset > _areaLength
        || infoSize > _areaLength - infoOffset)
        return nil;

    // The bookkeeping block: the block addresses (the table padded to a multiple of 256), then the named blocks
    FinderDSStoreCursor info = FinderDSStoreCursorMake(_area + infoOffset, infoSize);
    uint32_t count = FinderDSStoreUInt32(&info);
    FinderDSStoreUInt32(&info);
    if (info.failed || count > (infoSize - info.position) / 4)
        return nil;
    NSMutableArray<NSNumber *> *addresses = [NSMutableArray arrayWithCapacity:count];
    for (uint32_t i = 0; i < count; i++)
        [addresses addObject:[NSNumber numberWithUnsignedInt:FinderDSStoreUInt32(&info)]];
    _addresses = addresses;
    uint64_t padding = ((uint64_t)count + 255) / 256 * 256 - count;
    if (padding * 4 > info.length - info.position)
        return nil;
    FinderDSStoreTake(&info, (size_t)padding * 4);

    // Every block the table lists must be inside the file
    FinderDSStoreCursor block;
    for (uint32_t i = 0; i < count; i++) {
        if ([_addresses[i] unsignedIntValue] != 0 && ![self block:i cursor:&block])
            return nil;
    }

    uint32_t named = FinderDSStoreUInt32(&info);
    uint32_t dsdb = UINT32_MAX;
    for (uint32_t i = 0; i < named && !info.failed; i++) {
        const uint8_t *length = FinderDSStoreTake(&info, 1);
        const uint8_t *name = length ? FinderDSStoreTake(&info, *length) : NULL;
        uint32_t number = FinderDSStoreUInt32(&info);
        if (!info.failed && *length == 4 && memcmp(name, "DSDB", 4) == 0)
            dsdb = number;
    }
    if (info.failed || dsdb == UINT32_MAX)
        return nil;

    // The DSDB block: the root node, the tree's levels, the record count, the node count, the page size
    FinderDSStoreCursor tree;
    if (![self block:dsdb cursor:&tree])
        return nil;
    uint32_t root = FinderDSStoreUInt32(&tree);
    FinderDSStoreUInt32(&tree);
    uint32_t records = FinderDSStoreUInt32(&tree);
    if (tree.failed || records > FinderDSStoreMaximumRecords)
        return nil;

    _visited = [NSMutableIndexSet indexSet];
    _parsed = [NSMutableArray arrayWithCapacity:records];
    if (![self readNode:root depth:0] || [_parsed count] != records)
        return nil;
    _records = [_parsed copy];
    _parsed = nil;
    _visited = nil;
    _addresses = nil;
    _data = nil;
    _area = NULL;
    return self;
}

static NSString *FinderDSStoreKey(NSString *name, FourCharCode code)
{
    return [NSString stringWithFormat:@"%08x/%@", (unsigned)code, [name precomposedStringWithCanonicalMapping]];
}

- (FinderDSStoreRecord *)recordForName:(NSString *)name code:(FourCharCode)code
{
    if (_index == nil) {
        NSMutableDictionary<NSString *, FinderDSStoreRecord *> *index = [NSMutableDictionary dictionaryWithCapacity:[_records count]];
        for (FinderDSStoreRecord *record in _records) {
            NSString *key = FinderDSStoreKey(record.name, record.code);
            if (index[key] == nil)
                index[key] = record;
        }
        _index = index;
    }
    return _index[FinderDSStoreKey(name, code)];
}

@end
