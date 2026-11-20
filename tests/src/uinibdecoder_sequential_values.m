// Copyright (C) 2026 Lubos Dolezel
//
// UINibDecoder reads the values an object encoded without a key. NSMapTable, for one, encodes its
// two option words, its keys and values and a closing nil that way, and a NIBArchive holds them
// under the keys "$0", "$1" and so on. The archive here is written by the test, in the NIBArchive
// layout a nib compiled by ibtool has (observed on macOS 26.7.3); the decoder is
// UIFoundation's, which macOS does not have, so this test is for Darling only.
#import <Foundation/Foundation.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(cond, ...) do { if (!(cond)) { printf("FAIL line %d: ", __LINE__); printf(__VA_ARGS__); printf("\n"); failures++; } } while (0)

static void varint(NSMutableData *data, uint32_t value)
{
    // seven bits at a time, least significant first, the high bit marking the last byte
    while (value >= 0x80) {
        uint8_t byte = value & 0x7f;
        [data appendBytes:&byte length:1];
        value >>= 7;
    }
    uint8_t last = value | 0x80;
    [data appendBytes:&last length:1];
}

static void u32(NSMutableData *data, uint32_t value)
{
    value = CFSwapInt32HostToLittle(value);
    [data appendBytes:&value length:4];
}

enum { ValueInt32 = 2, ValueDouble = 7, ValueData = 8, ValueNil = 9, ValueObject = 10 };

@interface Archive : NSObject {
@public
    NSMutableArray *keys, *classes;
    NSMutableArray *objects;   // each: @[class index, array of value descriptions]
}
@end
@implementation Archive
- (instancetype)init
{
    if ((self = [super init])) {
        keys = [[NSMutableArray alloc] init];
        classes = [[NSMutableArray alloc] init];
        objects = [[NSMutableArray alloc] init];
    }
    return self;
}
- (NSUInteger)index:(NSString *)name in:(NSMutableArray *)list
{
    NSUInteger i = [list indexOfObject:name];
    if (i == NSNotFound) {
        [list addObject:name];
        i = [list count] - 1;
    }
    return i;
}
- (NSUInteger)addObjectOfClass:(NSString *)name
{
    [objects addObject:[NSMutableArray arrayWithObjects:@([self index:name in:classes]), [NSMutableArray array], nil]];
    return [objects count] - 1;
}
- (void)addValueTo:(NSUInteger)object key:(NSString *)key type:(int)type payload:(id)payload
{
    [[[objects objectAtIndex:object] objectAtIndex:1] addObject:@[ @([self index:key in:keys]), @(type), payload ]];
}
- (NSData *)data
{
    NSMutableData *objectData = [NSMutableData data], *keyData = [NSMutableData data], *valueData = [NSMutableData data],
                  *classData = [NSMutableData data];
    uint32_t valueCount = 0;

    for (NSArray *object in objects) {
        varint(objectData, [[object objectAtIndex:0] unsignedIntValue]);
        varint(objectData, valueCount);
        varint(objectData, (uint32_t)[[object objectAtIndex:1] count]);
        for (NSArray *value in [object objectAtIndex:1]) {
            int type = [[value objectAtIndex:1] intValue];
            id payload = [value objectAtIndex:2];

            varint(valueData, [[value objectAtIndex:0] unsignedIntValue]);
            uint8_t t = type;
            [valueData appendBytes:&t length:1];
            if (type == ValueInt32 || type == ValueObject)
                u32(valueData, [payload unsignedIntValue]);
            else if (type == ValueDouble) {
                double d = [payload doubleValue];
                uint64_t bits;
                memcpy(&bits, &d, 8);
                bits = CFSwapInt64HostToLittle(bits);
                [valueData appendBytes:&bits length:8];
            } else if (type == ValueData) {
                varint(valueData, (uint32_t)[payload length]);
                [valueData appendData:payload];
            }
            valueCount++;
        }
    }
    for (NSString *key in keys) {
        varint(keyData, (uint32_t)[key length]);
        [keyData appendData:[key dataUsingEncoding:NSUTF8StringEncoding]];
    }
    for (NSString *name in classes) {
        varint(classData, (uint32_t)[name length] + 1);
        varint(classData, 0);
        [classData appendData:[name dataUsingEncoding:NSUTF8StringEncoding]];
        uint8_t nul = 0;
        [classData appendBytes:&nul length:1];
    }

    NSMutableData *file = [NSMutableData dataWithBytes:"NIBArchive" length:10];
    uint32_t objectsOffset = 50, keysOffset = objectsOffset + (uint32_t)[objectData length],
             valuesOffset = keysOffset + (uint32_t)[keyData length], classesOffset = valuesOffset + (uint32_t)[valueData length];
    u32(file, 1);
    u32(file, 9);
    u32(file, (uint32_t)[objects count]);
    u32(file, objectsOffset);
    u32(file, (uint32_t)[keys count]);
    u32(file, keysOffset);
    u32(file, valueCount);
    u32(file, valuesOffset);
    u32(file, (uint32_t)[classes count]);
    u32(file, classesOffset);
    [file appendData:objectData];
    [file appendData:keyData];
    [file appendData:valueData];
    [file appendData:classData];
    return file;
}
@end

int main(void)
{
    @autoreleasepool {
        void *handle = dlopen("/System/Library/PrivateFrameworks/UIFoundation.framework/UIFoundation", RTLD_NOW);
        Class decoderClass = NSClassFromString(@"UINibDecoder");

        CHECK(handle != NULL && decoderClass != Nil, "UIFoundation is available");
        if (decoderClass == Nil)
            return 1;

        // root: "map" -> an NSMapTable of two entries (@"a" -> 1.5, @"b" -> 2.5), the way
        // NSMapTable encodes them: options, options, key, value, key, value, nil.
        Archive *archive = [[[Archive alloc] init] autorelease];
        NSUInteger root = [archive addObjectOfClass:@"NSObject"], map = [archive addObjectOfClass:@"NSMapTable"];
        NSString *names[2] = { @"a", @"b" };
        double numbers[2] = { 1.5, 2.5 };
        NSUInteger entries[4];

        for (int i = 0; i < 2; i++) {
            entries[2 * i] = [archive addObjectOfClass:@"NSString"];
            [archive addValueTo:entries[2 * i] key:@"NS.bytes" type:ValueData payload:[names[i] dataUsingEncoding:NSUTF8StringEncoding]];
            entries[2 * i + 1] = [archive addObjectOfClass:@"NSNumber"];
            [archive addValueTo:entries[2 * i + 1] key:@"NS.dblval" type:ValueDouble payload:@(numbers[i])];
        }
        [archive addValueTo:root key:@"map" type:ValueObject payload:@(map)];
        [archive addValueTo:map key:@"$0" type:ValueInt32 payload:@517];
        [archive addValueTo:map key:@"$1" type:ValueInt32 payload:@0];
        for (int i = 0; i < 4; i++)
            [archive addValueTo:map key:[NSString stringWithFormat:@"$%d", i + 2] type:ValueObject payload:@(entries[i])];
        [archive addValueTo:map key:@"$6" type:ValueNil payload:nil];
        [archive addValueTo:map key:@"NS.count" type:ValueInt32 payload:@2];

        id decoder = [[decoderClass alloc] performSelector:@selector(initForReadingWithData:) withObject:[archive data]];
        CHECK(decoder != nil, "the archive is read");
        NSMapTable *table = [decoder decodeObjectForKey:@"map"];
        CHECK([table isKindOfClass:[NSMapTable class]], "an NSMapTable is decoded, not %s", [[table description] UTF8String]);
        CHECK([table count] == 2, "the table has %lu entries", (unsigned long)[table count]);
        CHECK([[table objectForKey:@"a"] doubleValue] == 1.5 && [[table objectForKey:@"b"] doubleValue] == 2.5,
              "the entries are a -> 1.5 and b -> 2.5");

        // an object with no sequential values still decodes
        CHECK([decoder decodeObjectForKey:@"absent"] == nil, "a missing key gives nil");
    }
    if (failures) {
        printf("FAIL (%d)\n", failures);
        return 1;
    }
    printf("PASS\n");
    return 0;
}
