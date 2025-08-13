#import <CoreFoundation/CFArray.h>
#import <CoreFoundation/CFData.h>
#import <CoreFoundation/CFDictionary.h>
#import <CoreFoundation/CFNumber.h>
#import <Foundation/NSObject.h>
#import <Foundation/NSCoder.h>
#import <Foundation/NSData.h>

@class UINibDecoder;

struct UINibHeader
{
    char *magic;              /* "NIBArchive" */
    uint32_t formatVersion;   /* Always 0x1 */
    uint32_t coderVersion;    /* I found it as 0xa using macOS 10.13 but don't know if it is always that value */
    uint32_t objectCount;     /* Count of UINibObject objects */
    uint32_t objectOffset;    /* Where to found the object data */
    uint32_t keyCount;        /* Count of UINibKey objects */
    uint32_t keyOffset;       /* Where to found the key data */
    uint32_t valueCount;      /* Count of UINibValue objects */
    uint32_t valueOffset;     /* Where to found the value data */
    uint32_t classNameCount;  /* Count of UINibClassName objects */
    uint32_t classNameOffset; /* Where to found the class name data */
}__attribute__((__packed__));

typedef uint64_t vint32_t;

struct UINibObject
{
    vint32_t classIndex; /* Index of class name of the object in _classNames */
    vint32_t valueStart; /* Index of first UINibValue contained by this object */
    vint32_t valueCount; /* Count of UINibValue */
};

struct UINibKey
{
    vint32_t length; /* Length of characters of the key */
    char *name;      /* Key name (NOT NULL-terminated) */
};

struct UINibValue
{
    vint32_t keyIndex; /* Index of the key for this value in _keys */
    uint8_t type;      /* e.g. UINibValueType */
    vint32_t length;   /* Only used for variable type (NibValueData) */
    uint8_t *bytes;    /* Length can be from 1 to 8 bytes of data per type */
};

struct UINibClassName
{
    vint32_t length;              /* Length of class name */
    vint32_t fallbackClasses;     /* Count of fallback classes */
    uint32_t *fallbackClassIndex; /* Index of fallback class in _classNames */
    char *name;                   /* Name of the class */
};

typedef NS_ENUM(uint8_t, UINibValueType)
{
    NibValueInt8      = 0,  /* 1 byte length data */
    NibValueInt16     = 1,  /* 2 bytes length data */
    NibValueInt32     = 2,  /* 4 bytes length data */
    NibValueInt64     = 3,  /* 8 bytes length data */
    NibValueBoolTrue  = 4,  /* _bytes is ignored */
    NibValueBoolFalse = 5,  /* _bytes is ignored */
    NibValueFloat32   = 6,  /* 4 bytes length data */
    NibValueFloat64   = 7,  /* 8 bytes length data */
    NibValueData      = 8,  /* variable length data */
    NibValueNil       = 9,  /* _bytes is ignored */
    NibValueRef       = 10, /* 4 bytes length data */
};

@protocol UINibDecoderDelegate <NSObject>
@optional

@end

@interface UINibDecoder : NSCoder
{
    CFDataRef _data;
    const uint8_t *_bytes;
    NSUInteger _length;

    struct UINibHeader _header;
    struct UINibObject *_objects;
    struct UINibKey *_keys;
    struct UINibValue *_values;
    struct UINibClassName *_classNames;

    id<UINibDecoderDelegate> _delegate;
    CFMutableDictionaryRef _nameClassMapper;
    CFMutableDictionaryRef _objRefMapper;
}

- (instancetype)initForReadingWithData:(NSData *)data;
- (void)setDelegate:(id <UINibDecoderDelegate>)delegate;
- (id <UINibDecoderDelegate>)delegate;
- (void)setClass:(Class)cls forClassName:(NSString *)name;
- (id)decodeObjectForKey:(NSString *)key;

@end
