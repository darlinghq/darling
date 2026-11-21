/*
 This file is part of Darling.

 Copyright (C) 2025 Darling Developers

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

#import <UIFoundation/UINibDecoder.h>
#import <UIFoundation/UIFoundation.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

// NIBArchive layout (integers are little-endian; the format is described publicly at
// https://www.mothersruin.com/software/Archaeology/reverse/uinib.html and
// https://matrixeditor.github.io/nibarchive):
//   "NIBArchive", uint32 formatVersion, uint32 coderVersion,
//   uint32 objectCount, objectsOffset, keyCount, keysOffset, valueCount, valuesOffset,
//          classNameCount, classNamesOffset
//   object:    varint classNameIndex, varint firstValueIndex, varint valueCount
//   key:       varint length, bytes
//   value:     varint keyIndex, uint8 type, payload (see UINibValueType)
//   className: varint length, varint extraCount, int32 extras[extraCount], bytes (NUL-terminated)
// A varint holds 7 bits per byte, least significant group first; the high bit marks the LAST
// byte. Object 0 is the root: its values are the top-level keys. Collections store their
// elements as repeated "UINibEncoderEmptyKey" values (dictionaries alternate key and value).
// The file is untrusted input, so every count, offset, length and index is range-checked
// before it is used.

enum UINibValueType {
    UINibValueInt8 = 0,
    UINibValueInt16 = 1,
    UINibValueInt32 = 2,
    UINibValueInt64 = 3,
    // Read off ibtool output (macOS 26.7.3): the boolean keys that are YES by default
    // (NSAllowsKeyEquivalentLocalization, NSIsSeparator on a separator item) carry type 5,
    // those that are NO by default (NSHiddenInRepresentation) type 4.
    UINibValueFalse = 4,
    UINibValueTrue = 5,
    UINibValueFloat = 6,
    UINibValueDouble = 7,
    UINibValueData = 8,
    UINibValueNil = 9,
    UINibValueObject = 10,
};

typedef struct {
    uint32_t classIndex;
    uint32_t firstValue;
    uint32_t valueCount;
} UINibObjectRecord;

typedef struct {
    uint32_t keyIndex;
    uint8_t type;
    union {
        int64_t integer;
        double real;
        uint32_t object;
        struct {
            const uint8_t *bytes;
            uint32_t length;
        } data;
    } u;
} UINibValueRecord;

enum { UINibStateNone = 0, UINibStateDecoding = 1, UINibStateDone = 2 };

// Objects nest through references; refuse archives that nest deeper than any real nib.
static const unsigned UINibMaximumDepth = 512;

static NSString *const UINibEmptyKey = @"UINibEncoderEmptyKey";

typedef struct {
    const uint8_t *bytes;
    size_t length;
} UINibReader;

static BOOL readVarint(const UINibReader *r, size_t *offset, uint32_t *result)
{
    uint64_t value = 0;
    for (unsigned shift = 0; shift < 35; shift += 7) {
        if (*offset >= r->length)
            return NO;
        uint8_t byte = r->bytes[(*offset)++];
        value |= (uint64_t) (byte & 0x7f) << shift;
        if (byte & 0x80) {
            if (value > UINT32_MAX)
                return NO;
            *result = (uint32_t) value;
            return YES;
        }
    }
    return NO;
}

static BOOL readFixed(const UINibReader *r, size_t *offset, void *out, size_t size)
{
    if (*offset > r->length || r->length - *offset < size)
        return NO;
    memcpy(out, r->bytes + *offset, size);
    *offset += size;
    return YES;
}

// The decoder state lives in a heap structure: the i386 legacy Objective-C ABI cannot declare
// instance variables in @implementation.
struct UINibDecoderState {
    NSData *data;
    const uint8_t *bytes;
    uint32_t objectCount, keyCount, valueCount, classCount;
    UINibObjectRecord *objects;
    UINibValueRecord *values;
    NSString **keys;
    NSString **classNames;
    // The decoded object of each archived object. While an object is being decoded its slot
    // holds the allocated instance without owning it (so references back to it resolve);
    // afterwards the slot owns its object.
    id *decoded;
    uint8_t *state;
    // How many values -decodeValueOfObjCType:at: has read from each object.
    uint32_t *sequence;
    uint32_t current;
    unsigned depth;
    NSMutableDictionary *classMap;
};

@implementation UINibDecoder

@synthesize delegate = _delegate;

- (instancetype) initForReadingWithData: (NSData *) data
{
    self = [super init];
    if (self == nil)
        return nil;

    _s = calloc(1, sizeof(struct UINibDecoderState));
    if (_s == NULL) {
        [self release];
        return nil;
    }
    // The decoder reads from a private immutable copy.
    _s->data = [data copy];
    UINibReader reader = { [_s->data bytes], [_s->data length] };
    if (_s->data == nil || ![self _parseWithReader: &reader]) {
        [self release];
        return nil;
    }
    _s->bytes = reader.bytes;
    _s->classMap = [NSMutableDictionary new];
    return self;
}

- (BOOL) _parseWithReader: (const UINibReader *) r
{
    if (r->length < 50 || memcmp(r->bytes, UIArchiveHeaderIdentifier, strlen(UIArchiveHeaderIdentifier)) != 0)
        return NO;

    uint32_t header[10];
    size_t offset = 10;
    if (!readFixed(r, &offset, header, sizeof(header)))
        return NO;
    // The format version says which readers can read the file; the coder version only names
    // the encoder and does not change the layout.
    if (CFSwapInt32LittleToHost(header[0]) > UIMaximumCompatibleFormatVersion)
        return NO;
    for (int i = 0; i < 10; i++)
        header[i] = CFSwapInt32LittleToHost(header[i]);

    size_t objectsOffset = header[3], keysOffset = header[5], valuesOffset = header[7], classesOffset = header[9];
    _s->objectCount = header[2];
    _s->keyCount = header[4];
    _s->valueCount = header[6];
    _s->classCount = header[8];

    // Each record takes at least one byte, which bounds the counts by the size of the data.
    if (_s->objectCount == 0 || _s->objectCount > r->length || _s->keyCount > r->length || _s->valueCount > r->length ||
        _s->classCount > r->length)
        return NO;

    _s->objects = calloc(_s->objectCount, sizeof(UINibObjectRecord));
    _s->values = calloc(_s->valueCount ? _s->valueCount : 1, sizeof(UINibValueRecord));
    _s->keys = calloc(_s->keyCount ? _s->keyCount : 1, sizeof(NSString *));
    _s->classNames = calloc(_s->classCount ? _s->classCount : 1, sizeof(NSString *));
    _s->decoded = calloc(_s->objectCount, sizeof(id));
    _s->state = calloc(_s->objectCount, sizeof(uint8_t));
    _s->sequence = calloc(_s->objectCount, sizeof(uint32_t));
    if (!_s->objects || !_s->values || !_s->keys || !_s->classNames || !_s->decoded || !_s->state || !_s->sequence)
        return NO;

    offset = keysOffset;
    for (uint32_t i = 0; i < _s->keyCount; i++) {
        uint32_t length;
        if (!readVarint(r, &offset, &length) || length > r->length - offset)
            return NO;
        _s->keys[i] = [[NSString alloc] initWithBytes: r->bytes + offset length: length encoding: NSUTF8StringEncoding];
        if (_s->keys[i] == nil)
            return NO;
        offset += length;
    }

    offset = classesOffset;
    for (uint32_t i = 0; i < _s->classCount; i++) {
        uint32_t length, extraCount;
        if (!readVarint(r, &offset, &length) || !readVarint(r, &offset, &extraCount))
            return NO;
        if (extraCount > (r->length - offset) / 4)
            return NO;
        offset += 4 * (size_t) extraCount;
        if (length > r->length - offset)
            return NO;
        uint32_t nameLength = length;
        while (nameLength > 0 && r->bytes[offset + nameLength - 1] == 0)
            nameLength--;
        _s->classNames[i] = [[NSString alloc] initWithBytes: r->bytes + offset length: nameLength encoding: NSUTF8StringEncoding];
        if (_s->classNames[i] == nil)
            return NO;
        offset += length;
    }

    offset = valuesOffset;
    for (uint32_t i = 0; i < _s->valueCount; i++) {
        UINibValueRecord *value = &_s->values[i];
        if (!readVarint(r, &offset, &value->keyIndex) || value->keyIndex >= _s->keyCount)
            return NO;
        if (!readFixed(r, &offset, &value->type, 1))
            return NO;
        switch (value->type) {
        case UINibValueInt8: {
            int8_t v;
            if (!readFixed(r, &offset, &v, sizeof(v)))
                return NO;
            value->u.integer = v;
            break;
        }
        case UINibValueInt16: {
            int16_t v;
            if (!readFixed(r, &offset, &v, sizeof(v)))
                return NO;
            value->u.integer = (int16_t) CFSwapInt16LittleToHost((uint16_t) v);
            break;
        }
        case UINibValueInt32: {
            int32_t v;
            if (!readFixed(r, &offset, &v, sizeof(v)))
                return NO;
            value->u.integer = (int32_t) CFSwapInt32LittleToHost((uint32_t) v);
            break;
        }
        case UINibValueInt64: {
            int64_t v;
            if (!readFixed(r, &offset, &v, sizeof(v)))
                return NO;
            value->u.integer = (int64_t) CFSwapInt64LittleToHost((uint64_t) v);
            break;
        }
        case UINibValueTrue:
        case UINibValueFalse:
        case UINibValueNil:
            break;
        case UINibValueFloat: {
            uint32_t bits;
            if (!readFixed(r, &offset, &bits, sizeof(bits)))
                return NO;
            bits = CFSwapInt32LittleToHost(bits);
            float f;
            memcpy(&f, &bits, sizeof(f));
            value->u.real = f;
            break;
        }
        case UINibValueDouble: {
            uint64_t bits;
            if (!readFixed(r, &offset, &bits, sizeof(bits)))
                return NO;
            bits = CFSwapInt64LittleToHost(bits);
            memcpy(&value->u.real, &bits, sizeof(double));
            break;
        }
        case UINibValueData:
            if (!readVarint(r, &offset, &value->u.data.length) || value->u.data.length > r->length - offset)
                return NO;
            value->u.data.bytes = r->bytes + offset;
            offset += value->u.data.length;
            break;
        case UINibValueObject: {
            uint32_t object;
            if (!readFixed(r, &offset, &object, sizeof(object)))
                return NO;
            value->u.object = CFSwapInt32LittleToHost(object);
            if (value->u.object >= _s->objectCount)
                return NO;
            break;
        }
        default:
            return NO;
        }
    }

    offset = objectsOffset;
    for (uint32_t i = 0; i < _s->objectCount; i++) {
        UINibObjectRecord *object = &_s->objects[i];
        if (!readVarint(r, &offset, &object->classIndex) || !readVarint(r, &offset, &object->firstValue) ||
            !readVarint(r, &offset, &object->valueCount))
            return NO;
        if (object->classIndex >= _s->classCount || object->firstValue > _s->valueCount ||
            object->valueCount > _s->valueCount - object->firstValue)
            return NO;
    }
    return YES;
}

- (void) dealloc
{
    if (_s == NULL) {
        [super dealloc];
        return;
    }
    for (uint32_t i = 0; _s->decoded && i < _s->objectCount; i++)
        if (_s->state[i] == UINibStateDone)
            [_s->decoded[i] release];
    for (uint32_t i = 0; _s->keys && i < _s->keyCount; i++)
        [_s->keys[i] release];
    for (uint32_t i = 0; _s->classNames && i < _s->classCount; i++)
        [_s->classNames[i] release];
    free(_s->objects);
    free(_s->values);
    free(_s->keys);
    free(_s->classNames);
    free(_s->decoded);
    free(_s->state);
    free(_s->sequence);
    [_s->classMap release];
    [_s->data release];
    free(_s);
    [super dealloc];
}

- (BOOL) allowsKeyedCoding
{
    return YES;
}

- (void) setClass: (Class) cls forClassName: (NSString *) className
{
    if (className == nil)
        return;
    if (cls == Nil)
        [_s->classMap removeObjectForKey: className];
    else
        [_s->classMap setObject: cls forKey: className];
}

- (Class) classForClassName: (NSString *) className
{
    return className != nil ? [_s->classMap objectForKey: className] : Nil;
}

#pragma mark - Values

- (const UINibValueRecord *) _valueForKey: (NSString *) key
{
    if (key == nil)
        return NULL;
    const UINibObjectRecord *object = &_s->objects[_s->current];
    const UINibValueRecord *found = NULL;
    for (uint32_t i = 0; i < object->valueCount; i++) {
        const UINibValueRecord *value = &_s->values[object->firstValue + i];
        if ([_s->keys[value->keyIndex] isEqualToString: key])
            found = value;
    }
    return found;
}

- (BOOL) containsValueForKey: (NSString *) key
{
    return [self _valueForKey: key] != NULL;
}

// The value as an object (not owned by the caller); nil for a nil value.
- (id) _objectForValue: (const UINibValueRecord *) value
{
    switch (value->type) {
    case UINibValueInt8:
    case UINibValueInt16:
    case UINibValueInt32:
    case UINibValueInt64:
        return [NSNumber numberWithLongLong: value->u.integer];
    case UINibValueTrue:
        return [NSNumber numberWithBool: YES];
    case UINibValueFalse:
        return [NSNumber numberWithBool: NO];
    case UINibValueFloat:
        return [NSNumber numberWithFloat: (float) value->u.real];
    case UINibValueDouble:
        return [NSNumber numberWithDouble: value->u.real];
    case UINibValueData:
        return [NSData dataWithBytes: value->u.data.bytes length: value->u.data.length];
    case UINibValueObject:
        return [self _decodedObjectAtIndex: value->u.object];
    default:
        return nil;
    }
}

- (id) decodeObjectForKey: (NSString *) key
{
    const UINibValueRecord *value = [self _valueForKey: key];
    return value != NULL ? [self _objectForValue: value] : nil;
}

- (int64_t) _integerForKey: (NSString *) key
{
    const UINibValueRecord *value = [self _valueForKey: key];
    if (value == NULL)
        return 0;
    switch (value->type) {
    case UINibValueInt8:
    case UINibValueInt16:
    case UINibValueInt32:
    case UINibValueInt64:
        return value->u.integer;
    case UINibValueTrue:
        return 1;
    case UINibValueFloat:
    case UINibValueDouble:
        return (int64_t) value->u.real;
    case UINibValueObject: {
        id object = [self _objectForValue: value];
        return [object respondsToSelector: @selector(longLongValue)] ? [object longLongValue] : 0;
    }
    default:
        return 0;
    }
}

- (double) _doubleForKey: (NSString *) key
{
    const UINibValueRecord *value = [self _valueForKey: key];
    if (value == NULL)
        return 0;
    switch (value->type) {
    case UINibValueFloat:
    case UINibValueDouble:
        return value->u.real;
    case UINibValueObject: {
        id object = [self _objectForValue: value];
        return [object respondsToSelector: @selector(doubleValue)] ? [object doubleValue] : 0;
    }
    default:
        return (double) [self _integerForKey: key];
    }
}

- (BOOL) decodeBoolForKey: (NSString *) key
{
    return [self _integerForKey: key] != 0 || [self _doubleForKey: key] != 0;
}

- (int) decodeIntForKey: (NSString *) key
{
    return (int) [self _integerForKey: key];
}

- (int32_t) decodeInt32ForKey: (NSString *) key
{
    return (int32_t) [self _integerForKey: key];
}

- (int64_t) decodeInt64ForKey: (NSString *) key
{
    return [self _integerForKey: key];
}

- (float) decodeFloatForKey: (NSString *) key
{
    return (float) [self _doubleForKey: key];
}

- (double) decodeDoubleForKey: (NSString *) key
{
    return [self _doubleForKey: key];
}

- (const uint8_t *) decodeBytesForKey: (NSString *) key returnedLength: (NSUInteger *) len
{
    const UINibValueRecord *value = [self _valueForKey: key];
    if (value != NULL && value->type == UINibValueData) {
        if (len != NULL)
            *len = value->u.data.length;
        return value->u.data.bytes;
    }
    if (value != NULL && value->type == UINibValueObject) {
        id object = [self _objectForValue: value];
        if ([object isKindOfClass: [NSData class]]) {
            if (len != NULL)
                *len = [object length];
            return [object bytes];
        }
    }
    if (len != NULL)
        *len = 0;
    return NULL;
}

#pragma mark - Values without a key

// An object that encodes values without a key (NSMapTable and NSHashTable do) has them archived
// under the keys "$0", "$1" and so on, in the order they were encoded. They are read back in that
// order: each call takes the next one of the object being decoded.
- (void) decodeValueOfObjCType: (const char *) type at: (void *) data
{
    NSString *key = [NSString stringWithFormat: @"$%u", (unsigned) _s->sequence[_s->current]++];

    if (type == NULL || data == NULL)
        return;
    switch (*type) {
    case '@':
        // As with the other coders the caller owns what is decoded through this method.
        *(id *) data = [[self decodeObjectForKey: key] retain];
        break;
    case 'c':
    case 'C':
        *(char *) data = (char) [self _integerForKey: key];
        break;
    case 'B':
        *(BOOL *) data = [self _integerForKey: key] != 0;
        break;
    case 's':
    case 'S':
        *(short *) data = (short) [self _integerForKey: key];
        break;
    case 'i':
    case 'I':
        *(int *) data = (int) [self _integerForKey: key];
        break;
    case 'l':
    case 'L':
        *(long *) data = (long) [self _integerForKey: key];
        break;
    case 'q':
    case 'Q':
        *(long long *) data = (long long) [self _integerForKey: key];
        break;
    case 'f':
        *(float *) data = (float) [self _doubleForKey: key];
        break;
    case 'd':
        *(double *) data = [self _doubleForKey: key];
        break;
    default:
        [NSException raise: NSInvalidArchiveOperationException
                    format: @"UINibDecoder cannot decode a value of type %s without a key", type];
    }
}

- (NSData *) decodeDataObject
{
    [NSException raise: NSInvalidArchiveOperationException
                format: @"UINibDecoder supports keyed coding only (decodeDataObject)"];
    return nil;
}

- (NSInteger) versionForClassName: (NSString *) className
{
    return 0;
}

#pragma mark - Objects

- (void) replaceObject: (id) object withObject: (id) replacement
{
    if ([_delegate respondsToSelector: @selector(unarchiver:willReplaceObject:withObject:)])
        [_delegate unarchiver: self willReplaceObject: object withObject: replacement];
    if (object == replacement || object == nil)
        return;
    for (uint32_t i = 0; i < _s->objectCount; i++) {
        if (_s->state[i] == UINibStateNone || _s->decoded[i] != object)
            continue;
        _s->decoded[i] = [replacement retain];
        // A slot of an object that is still being decoded does not own its object.
        if (_s->state[i] == UINibStateDone)
            [object release];
    }
}

- (id) _decodedObjectAtIndex: (uint32_t) index
{
    if (index >= _s->objectCount)
        [NSException raise: NSInvalidArchiveOperationException format: @"object index out of range"];
    if (_s->state[index] == UINibStateDone)
        return _s->decoded[index];
    if (_s->state[index] == UINibStateDecoding) {
        if (_s->decoded[index] != nil)
            return _s->decoded[index];
        [NSException raise: NSInvalidArchiveOperationException format: @"circular reference between collections"];
    }
    if (_s->depth >= UINibMaximumDepth)
        [NSException raise: NSInvalidArchiveOperationException format: @"objects nested too deeply"];

    _s->state[index] = UINibStateDecoding;
    _s->depth++;
    BOOL finished = NO;
    @try {
        [self _decodeObjectAtIndex: index];
        finished = YES;
    } @finally {
        _s->depth--;
        if (!finished) {
            _s->decoded[index] = nil;
            _s->state[index] = UINibStateNone;
        }
    }
    return _s->decoded[index];
}

static BOOL classNameIsOneOf(NSString *className, const char *const names[])
{
    const char *name = [className UTF8String];
    for (int i = 0; names[i]; i++)
        if (strcmp(name, names[i]) == 0)
            return YES;
    return NO;
}

- (const UINibValueRecord *) _valueNamed: (NSString *) key ofObject: (uint32_t) index
{
    uint32_t saved = _s->current;
    _s->current = index;
    const UINibValueRecord *value = [self _valueForKey: key];
    _s->current = saved;
    return value;
}

// Strings, data, numbers and collections have no initWithCoder: reading the archive's own
// layout; build them from it directly. Returns an owned object, or nil if the object is
// not one of those (or lacks the expected values).
- (id) _createBuiltinForObject: (uint32_t) index className: (NSString *) className
{
    static const char *const stringClasses[] = { "NSString", "NSMutableString", "NSLocalizableString", NULL };
    static const char *const dataClasses[] = { "NSData", "NSMutableData", NULL };
    static const char *const arrayClasses[] = { "NSArray", "NSMutableArray", NULL };
    static const char *const setClasses[] = { "NSSet", "NSMutableSet", NULL };
    static const char *const orderedSetClasses[] = { "NSOrderedSet", "NSMutableOrderedSet", NULL };
    static const char *const dictionaryClasses[] = { "NSDictionary", "NSMutableDictionary", NULL };
    const UINibObjectRecord *object = &_s->objects[index];
    BOOL mutableClass = [className hasPrefix: @"NSMutable"];

    if (classNameIsOneOf(className, stringClasses)) {
        const UINibValueRecord *bytes = [self _valueNamed: @"NS.bytes" ofObject: index];
        if (bytes == NULL || bytes->type != UINibValueData)
            return nil;
        NSString *string = [[NSString alloc] initWithBytes: bytes->u.data.bytes length: bytes->u.data.length
                                                  encoding: NSUTF8StringEncoding];
        if (string == nil)
            [NSException raise: NSInvalidArchiveOperationException format: @"string is not valid UTF-8"];
        if (mutableClass) {
            NSMutableString *copy = [string mutableCopy];
            [string release];
            return copy;
        }
        return string;
    }
    if (classNameIsOneOf(className, dataClasses)) {
        const UINibValueRecord *bytes = [self _valueNamed: @"NS.bytes" ofObject: index];
        if (bytes == NULL || bytes->type != UINibValueData)
            return nil;
        return mutableClass ? [[NSMutableData alloc] initWithBytes: bytes->u.data.bytes length: bytes->u.data.length]
                            : [[NSData alloc] initWithBytes: bytes->u.data.bytes length: bytes->u.data.length];
    }
    if ([className isEqualToString: @"NSNumber"] && object->valueCount == 1) {
        const UINibValueRecord *value = &_s->values[object->firstValue];
        switch (value->type) {
        case UINibValueInt8:
        case UINibValueInt16:
        case UINibValueInt32:
        case UINibValueInt64:
        case UINibValueTrue:
        case UINibValueFalse:
        case UINibValueFloat:
        case UINibValueDouble:
            return [[self _objectForValue: value] retain];
        default:
            return nil;
        }
    }

    BOOL isArray = classNameIsOneOf(className, arrayClasses);
    BOOL isSet = classNameIsOneOf(className, setClasses);
    BOOL isOrderedSet = classNameIsOneOf(className, orderedSetClasses);
    BOOL isDictionary = classNameIsOneOf(className, dictionaryClasses);
    if (!isArray && !isSet && !isOrderedSet && !isDictionary)
        return nil;

    NSMutableArray *elements = [NSMutableArray array];
    for (uint32_t i = 0; i < object->valueCount; i++) {
        const UINibValueRecord *value = &_s->values[object->firstValue + i];
        if (![_s->keys[value->keyIndex] isEqualToString: UINibEmptyKey])
            continue;
        id element = [self _objectForValue: value];
        if (element == nil)
            [NSException raise: NSInvalidArchiveOperationException format: @"%@ contains a nil element", className];
        [elements addObject: element];
    }

    if (isDictionary) {
        // Keys and values alternate; an unpaired key cannot be a dictionary.
        if ([elements count] % 2 != 0)
            [NSException raise: NSInvalidArchiveOperationException format: @"dictionary has a key without a value"];
        NSMutableArray *keys = [NSMutableArray array], *values = [NSMutableArray array];
        for (NSUInteger i = 0; i < [elements count]; i += 2) {
            [keys addObject: [elements objectAtIndex: i]];
            [values addObject: [elements objectAtIndex: i + 1]];
        }
        return mutableClass ? [[NSMutableDictionary alloc] initWithObjects: values forKeys: keys]
                            : [[NSDictionary alloc] initWithObjects: values forKeys: keys];
    }
    if (isSet)
        return mutableClass ? [[NSMutableSet alloc] initWithArray: elements] : [[NSSet alloc] initWithArray: elements];
    if (isOrderedSet)
        return mutableClass ? [[NSMutableOrderedSet alloc] initWithArray: elements]
                            : [[NSOrderedSet alloc] initWithArray: elements];
    return mutableClass ? [elements mutableCopy] : [elements copy];
}

- (void) _decodeObjectAtIndex: (uint32_t) index
{
    NSString *className = _s->classNames[_s->objects[index].classIndex];
    Class substitute = [_s->classMap objectForKey: className];
    id instance = nil;
    BOOL viaClass = YES;

    if (substitute == Nil) {
        instance = [self _createBuiltinForObject: index className: className];
        if (instance != nil)
            viaClass = NO;
    }

    if (viaClass) {
        Class cls = substitute != Nil ? substitute : NSClassFromString(className);
        // A class that does not exist (such as the design-time classes Interface Builder
        // stores) decodes as nil, as it does from an NSKeyedUnarchiver.
        if (cls != Nil && [cls respondsToSelector: @selector(classForKeyedUnarchiver)])
            cls = [cls classForKeyedUnarchiver];
        if (cls != Nil) {
            id allocated = [cls allocWithZone: NULL];
            _s->decoded[index] = allocated;
            uint32_t saved = _s->current;
            _s->current = index;
            @try {
                instance = [allocated initWithCoder: self];
            } @finally {
                _s->current = saved;
            }
            if (instance != nil)
                instance = [instance awakeAfterUsingCoder: self];
            // The slot can have been replaced meanwhile (replaceObject:withObject:), which
            // retained the replacement for it.
            if (_s->decoded[index] != allocated && _s->decoded[index] != nil)
                [_s->decoded[index] release];
            _s->decoded[index] = nil;
        }
    }

    if (instance != nil && [_delegate respondsToSelector: @selector(unarchiver:didDecodeObject:)] &&
        ![instance isKindOfClass: [NSString class]] && ![instance isKindOfClass: [NSData class]] && ![instance isKindOfClass: [NSNumber class]]) {
        id replacement = [_delegate unarchiver: self didDecodeObject: instance];
        if (replacement != nil && replacement != instance) {
            if ([_delegate respondsToSelector: @selector(unarchiver:willReplaceObject:withObject:)])
                [_delegate unarchiver: self willReplaceObject: instance withObject: replacement];
            [replacement retain];
            [instance release];
            instance = replacement;
        }
    }
    _s->decoded[index] = instance;
    _s->state[index] = UINibStateDone;
}

@end
