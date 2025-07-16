#import <UIFoundation/UINibDecoder.h>
#import <Foundation/NSMethodSignature.h>
#import <Foundation/NSInvocation.h>
#import <stdlib.h>

/* Header constants */
const CFStringRef CONSTANT_HEADER = CFSTR("NIBArchive");
const uint32_t FORMAT_VERSION     = 1;
const uint32_t CODER_VERSION      = 10;

/* Offset constants */
const NSUInteger FORMAT_VERSION_POSITION = 10;
const NSUInteger CODER_VERSION_POSITION  = 14;
const NSUInteger OBJECT_COUNT_POSITION   = 18;
const NSUInteger OBJECT_OFFSET_POSITION  = 22;
const NSUInteger KEY_COUNT_POSITION      = 26;
const NSUInteger KEY_OFFSET_POSITION     = 30;
const NSUInteger VALUE_COUNT_POSITION    = 34;
const NSUInteger VALUE_OFFSET_POSITION   = 38;
const NSUInteger CLASS_COUNT_POSITION    = 42;
const NSUInteger CLASS_OFFSET_POSITION   = 46;

@implementation UINibDecoder

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

- (instancetype)init
{
    self = [super init];

    if (!self) {
        return nil;
    }

    _bytes = nil;
    _length = 0;
    _nameClassMapper = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    _objRefMapper = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    _delegate = nil;

    return self;
}

- (instancetype)initForReadingWithData:(NSData *)data
{
    if (data == nil)
    {
        [self release];

        return nil;
    }

    self = [self init];

    if (!self) {
        return nil;
    }

    _data = (CFDataRef)CFRetain((__bridge CFTypeRef)data);
    _bytes = CFDataGetBytePtr(_data);
    _length = [data length];

    uint64_t (^readVarIntAt)(uint32_t *) = ^(uint32_t *offsetPtr)
    {
        uint64_t result = 0;
        int shift = 0;

        while (true) {
            uint8_t byte;
            CFDataGetBytes(_data, CFRangeMake(*offsetPtr, sizeof(uint8_t)), &byte)
            (*offsetPtr)++;
            result |= ((uint64_t)(byte & 0x7F)) << shift;

            if (byte & 0x80) {
                break;
            }

            shift += 7;
        }

        return result;
    };
    uint32_t (^readUInt32At)(NSUInteger) = ^(NSUInteger position)
    {
        uint32_t buffer;
        CFDataGetBytes(_data, CFRangeMake(position, sizeof(uint32_t)), &buffer);

        return buffer;
    };

    /* Parse header */
    _header.magic = (char *)malloc(11);
    CFDataGetBytes(_data, CFRangeMake(0, 10), _header.magic);
    _header.magic[10] = '\0';
    _header.formatVersion = readUInt32At(FORMAT_VERSION_POSITION);
    _header.coderVersion = readUInt32At(CODER_VERSION_POSITION);
    CFStringRef cfMagic = CFStringCreateWithCString(kCFAllocatorDefault, _header.magic, kCFStringEncodingUTF8);

    if (
        _length < 32
        || CFStringCompare(cfMagic, CONSTANT_HEADER, 0) != kCFCompareEqualTo
        || _header.formatVersion != FORMAT_VERSION
        || _header.coderVersion != CODER_VERSION
    ) {
        // TODO: Throw an exception if data is not a valid archive as NSKeyedUnarchive

        NSLog(@"Invalid NIBArchive file! header:%s, format:%u, coder:%u", _header.magic, _header.formatVersion, _header.coderVersion);

        return nil;
    }

    CFRelease(cfMagic);

    _header.keyCount = OSReadLittleInt32(_bytes, KEY_COUNT_POSITION);
    _header.keyOffset = OSReadLittleInt32(_bytes, KEY_OFFSET_POSITION);
    _keys = (UINibKey *)malloc(_header.keyCount * sizeof(struct UINibKey));
    void (^parseKeys)(void) = ^{
        uint32_t offset = _header.keyOffset;

        for (uint32_t i = 0; i < _header.keyCount; i++) {
            struct UINibKey key;
            key.length = readVarIntAt(&offset);
            key.name = (char *)[[data subdataWithRange:NSMakeRange(offset, key.length)] bytes];
            _keys[i] = key;
            offset += key.length;
        }
    };

    parseKeys();

    _header.classNameCount = readUInt32At(CLASS_COUNT_POSITION);
    _header.classNameOffset = readUInt32At(CLASS_OFFSET_POSITION);
    _classNames = (struct UINibClassName *)malloc(_header.classNameCount * sizeof(struct UINibClassName));
    _header.keyCount = readUInt32At(KEY_COUNT_POSITION);
    _header.keyOffset = readUInt32At(KEY_OFFSET_POSITION);
    _keys = (struct UINibKey *)malloc(_header.keyCount * sizeof(struct UINibKey));
    _header.valueCount = readUInt32At(VALUE_COUNT_POSITION);
    _header.valueOffset = readUInt32At(VALUE_OFFSET_POSITION);
    _values = (struct UINibValue *)malloc(_header.valueCount * sizeof(struct UINibValue));
    _header.objectCount = readUInt32At(OBJECT_COUNT_POSITION);
    _header.objectOffset = readUInt32At(OBJECT_OFFSET_POSITION);
    _objects = (struct UINibObject *)malloc(_header.objectCount * sizeof(struct UINibObject));

    void (^parseContent)(void) = ^{
        uint32_t _classNameOffset = _header.classNameOffset;

        for (uint32_t i = 0; i < _header.classNameCount; i++) {
            struct UINibClassName className;
            className.length = readVarIntAt(&_classNameOffset);
            className.fallbackClasses = readVarIntAt(&_classNameOffset); // Ignored at this moment
            className.name = (char *)malloc(className.length);
            CFDataGetBytes(_data, CFRangeMake(_classNameOffset, className.length), className.name);
            _classNames[i] = className;
            _classNameOffset += className.length;
        }

        uint32_t _keyOffset = _header.keyOffset;

        for (uint32_t i = 0; i < _header.keyCount; i++) {
            struct UINibKey key;
            key.length = readVarIntAt(&_keyOffset);
            key.name = (char *)malloc(key.length);
            CFDataGetBytes(_data, CFRangeMake(_keyOffset, key.length), key.name);
            key.name[key.length] = '\0';
            _keys[i] = key;
            _keyOffset += key.length;
        }

        uint32_t _valueOffset = _header.valueOffset;

        for (uint32_t i = 0; i < _header.valueCount; i++) {
            struct UINibValue value;
            value.keyIndex = readVarIntAt(&_valueOffset);
            CFDataGetBytes(_data, CFRangeMake(_valueOffset, sizeof(uint8_t)), &value.type);
            _valueOffset++;
            value.bytes = nil;
            value.length = 0;

            switch (value.type) {
                case NibValueInt8: {
                    uint8_t v;
                    CFDataGetBytes(CFRetain(data), CFRangeMake(_valueOffset, sizeof(uint8_t)), &v);
                    uint8_t *copy = malloc(sizeof(uint8_t));
                    memcpy(copy, &v, sizeof(uint8_t));
                    value.bytes = copy;
                    value.length = 1;
                    break;
                }
                case NibValueInt16: {
                    uint16_t v;
                    CFDataGetBytes(CFRetain(data), CFRangeMake(_valueOffset, sizeof(uint16_t)), &v);
                    uint8_t *copy = malloc(sizeof(uint16_t));
                    memcpy(copy, &v, sizeof(uint16_t));
                    value.bytes = copy;
                    value.length = 2;
                    break;
                }
                case NibValueInt32: {
                    uint32_t v;
                    CFDataGetBytes(CFRetain(data), CFRangeMake(_valueOffset, sizeof(uint32_t)), &v);
                    uint8_t *copy = malloc(sizeof(uint32_t));
                    memcpy(copy, &v, sizeof(uint32_t));
                    value.bytes = copy;
                    value.length = 4;
                    break;
                }
                case NibValueInt64: {
                    uint64_t v;
                    CFDataGetBytes(_data, CFRangeMake(_valueOffset, sizeof(uint64_t)), &v);
                    uint8_t *copy = malloc(sizeof(uint64_t));
                    memcpy(copy, &v, sizeof(uint64_t));
                    value.bytes = copy;
                    value.length = 8;
                    break;
                }
                case NibValueBoolTrue:
                case NibValueBoolFalse:
                case NibValueNil:
                    break;
                case NibValueFloat32: {
                    float f;
                    CFDataGetBytes(_data, CFRangeMake(_valueOffset, sizeof(float)), &f);
                    uint8_t *copy = malloc(sizeof(float));
                    memcpy(copy, &f, sizeof(float));
                    value.bytes = copy;
                    value.length = 4;
                    break;
                }
                case NibValueFloat64: {
                    double d;
                    CFDataGetBytes(_data, CFRangeMake(_valueOffset, sizeof(double)), &d);
                    uint8_t *copy = malloc(sizeof(double));
                    memcpy(copy, &d, sizeof(double));
                    value.bytes = copy;
                    value.length = 8;
                    break;
                }
                case NibValueData: { // Ignored by now
                    value.length = readVarIntAt(&_valueOffset);
                    // NSLog(@"Length: %llu", value.length);
                    // CFDataGetBytes(_data, CFRangeMake(_valueOffset, value.length), value.bytes);
                    // NSLog(@"Value: %u", value.bytes);
                    break;
                }
                case NibValueRef: {
                    uint32_t v = readUInt32At(_valueOffset);
                    uint8_t *copy = malloc(sizeof(uint32_t));
                    memcpy(copy, &v, sizeof(uint32_t));
                    value.bytes = copy;
                    value.length = 4;
                    break;
                }
                default:
                    NSLog(@"[UINibDecoder] Unknown type: %u", value.type);
                    break;
            }

            _values[i] = value;
            _valueOffset += value.length;
        }

        uint32_t _objectOffset = _header.objectOffset;

        for (uint32_t i = 0; i < _header.objectCount; i++) {
            struct UINibObject object;
            object.classIndex = readVarIntAt(&_objectOffset);
            struct UINibClassName className = _classNames[object.classIndex];
            object.valueStart = readVarIntAt(&_objectOffset);
            object.valueCount = readVarIntAt(&_objectOffset);
            _objects[i] = object;

            CFMutableDictionaryRef tmpObjectDict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
            CFDictionarySetValue(tmpObjectDict, CFSTR("class"), CFStringCreateWithCString(kCFAllocatorDefault, className.name, kCFStringEncodingUTF8));
            struct UINibKey key;

            for (uint64_t i = 0; i < object.valueCount; i++) {
                if (i == 0) {
                    key = _keys[value.keyIndex];
                } else {
                    continue;
                }
            }

            CFStringRef keyString = CFStringCreateWithCString(kCFAllocatorDefault, key.name, kCFStringEncodingUTF8);

            CFDictionarySetValue(_objRefMapper, keyString, tmpObjectDict);
            CFRelease(keyString);
        }
    };

    parseContent();

    return self;
}

- (instancetype)initForReadingWithData:(NSData *)data error:(NSError **)error
{
    NSLog(@"STUB %s\n", __PRETTY_FUNCTION__);

    return [self initForReadingWithData:data];
}

- (void)dealloc
{
    NSLog(@"STUB %s\n", __PRETTY_FUNCTION__);

    if (_data) {
        CFRelease(_data);
    }

    if (_nameClassMapper) {
        CFRelease(_nameClassMapper);
    }

    [super dealloc];
}

- (unsigned)systemVersion
{
    NSLog(@"STUB %s\n", __PRETTY_FUNCTION__);

    return 0;
}

- (BOOL)allowsKeyedCoding
{
    NSLog(@"STUB %s\n", __PRETTY_FUNCTION__);

    return FALSE;
}

- (NSInteger)versionForClassName:(NSString *)className
{
    NSLog(@"STUB %s\n", __PRETTY_FUNCTION__);

    return 0;
}

- (BOOL)containsValueForKey:(NSString *)key
{
    NSLog(@"STUB %s\n", __PRETTY_FUNCTION__);

    return FALSE;
}

- (void)finishDecoding
{
    NSLog(@"STUB %s\n", __PRETTY_FUNCTION__);
}

- (int)decodeIntForKey:(NSString *)key
{
    NSLog(@"STUB %s\n", __PRETTY_FUNCTION__);

    return 0;
}

- (int32_t)decodeInt32ForKey:(NSString *)key
{
    NSLog(@"STUB %s\n", __PRETTY_FUNCTION__);

    return (int32_t)0;
}

- (int64_t)decodeInt64ForKey:(NSString *)key
{
    NSLog(@"STUB %s\n", __PRETTY_FUNCTION__);

    return (int64_t)0;
}

- (float)decodeFloatForKey:(NSString *)key
{
    NSLog(@"STUB %s\n", __PRETTY_FUNCTION__);

    return (float)0.0;
}

- (void)decodeArrayOfFloats:(float *)items count:(NSUInteger)count forKey:(NSString *)key
{
    NSLog(@"STUB %s\n", __PRETTY_FUNCTION__);
}

- (double)decodeDoubleForKey:(NSString *)key
{
    NSLog(@"STUB %s\n", __PRETTY_FUNCTION__);

    return (double)0.0;
}

- (void)decodeArrayOfDoubles:(double *)items count:(NSUInteger)count forKey:(NSString *)key
{
    NSLog(@"STUB %s\n", __PRETTY_FUNCTION__);
}

- (NSInteger)decodeIntegerForKey:(NSString *)key
{
    NSLog(@"STUB %s\n", __PRETTY_FUNCTION__);

    return 0;
}

- (BOOL)decodeBoolForKey:(NSString *)key
{
    NSLog(@"STUB %s\n", __PRETTY_FUNCTION__);

    return FALSE;
}

- (id)decodeObjectForKey:(NSString *)key
{
    CFDictionaryRef dict = CFDictionaryGetValue(_objRefMapper, key);

    if (!dict) {
		return nil;
	}

    CFStringRef cls = CFDictionaryGetValue(dict, CFSTR("class"));
    Class realClass = NSClassFromString((__bridge NSString *)cls);

    return [[realClass alloc] init];
}

- (void)decodeValuesOfObjCTypes:(const char *)types, ...
{
    NSLog(@"STUB %s\n", __PRETTY_FUNCTION__);
}

- (const uint8_t *)decodeBytesForKey:(NSString *)key returnedLength:(NSUInteger *)length
{
    NSLog(@"STUB %s\n", __PRETTY_FUNCTION__);

    return nil;
}

- (void)decodeValueOfObjCType:(const char *)type at:(void *)data
{
    NSLog(@"STUB %s\n", __PRETTY_FUNCTION__);
}

- (NSData *)decodeDataObject
{
    NSLog(@"STUB %s\n", __PRETTY_FUNCTION__);

    return nil;
}

- (void)decodeArrayOfObjCType:(const char *)itemType count:(NSUInteger)count at:(void *)array
{
    NSLog(@"STUB %s\n", __PRETTY_FUNCTION__);
}

- (void *)decodeBytesWithReturnedLength:(NSUInteger *)length
{
    NSLog(@"STUB %s\n", __PRETTY_FUNCTION__);

    return nil;
}

- (id)decodePropertyListForKey:(NSString *)key
{
    NSLog(@"STUB %s\n", __PRETTY_FUNCTION__);

    return nil;
}

- (id)decodeNSObject
{
    NSLog(@"STUB %s\n", __PRETTY_FUNCTION__);

    return nil;
}

- (id)decodeCGPointForKey:(NSString *)key
{
    NSLog(@"STUB %s\n", __PRETTY_FUNCTION__);

    return nil;
}

- (id)decodeCGSizeForKey:(NSString *)key
{
    NSLog(@"STUB %s\n", __PRETTY_FUNCTION__);

    return nil;
}

- (id)decodeCFRectForKey:(NSString *)key
{
    NSLog(@"STUB %s\n", __PRETTY_FUNCTION__);

    return nil;
}

- (id)decodeArrayOfCGFloat
{
    NSLog(@"STUB %s\n", __PRETTY_FUNCTION__);

    return nil;
}

- (id)decodeCFAffineTransformForKey:(NSString *)key
{
    NSLog(@"STUB %s\n", __PRETTY_FUNCTION__);

    return nil;
}

- (void)setDelegate:(id <UINibDecoderDelegate>)delegate
{
    if (_delegate != delegate)
    {
        _delegate = delegate;
    }
}

- (id <UINibDecoderDelegate>)delegate
{
    return _delegate;
}

- (void)setClass:(Class)cls forClassName:(NSString *)name
{
    CFDictionarySetValue(_nameClassMapper, name, cls);
}

@end
