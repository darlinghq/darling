// Copyright (C) 2026 Lubos Dolezel
//
// UINibDecoder (UIFoundation): decodes NIBArchive data from archives this test writes itself
// from the documented layout (see the comment in UINibDecoder.m), and rejects corrupt data.
// Optional arguments are compiled nibs (ibtool output) that must be accepted, and every
// prefix and many single-byte corruptions of which must be handled without crashing.
//
// Usage: uifoundation-nibdecoder [nib files...]
#import <Foundation/Foundation.h>
#import <UIFoundation/UINibDecoder.h>
#include <stdio.h>

static int failures;
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #c); failures++; } } while (0)

enum { tInt8, tInt16, tInt32, tInt64, tFalse, tTrue, tFloat, tDouble, tData, tNil, tObject };

// ---- A small NIBArchive writer, the inverse of the documented layout ----
static void varint(NSMutableData *d, uint32_t v) {
    do { uint8_t b = v & 0x7f; v >>= 7; if (!v) b |= 0x80; [d appendBytes:&b length:1]; } while (v);
}
static void le32(NSMutableData *d, uint32_t v) { uint8_t b[4] = {v, v >> 8, v >> 16, v >> 24}; [d appendBytes:b length:4]; }

@interface NibWriter : NSObject {
    NSMutableArray *classes, *keys;
    NSMutableData *objects, *values;
    uint32_t objectCount, valueCount;
    NSMutableArray *pending;   // per object: class index, first value, count
    uint32_t formatVersion;
}
@property uint32_t formatVersion;
- (uint32_t)object:(NSString *)cls;           // starts an object; values follow until the next object
- (void)key:(NSString *)k type:(int)t;        // value without payload
- (void)key:(NSString *)k int:(int64_t)v type:(int)t;
- (void)key:(NSString *)k double:(double)v type:(int)t;
- (void)key:(NSString *)k data:(NSData *)d;
- (void)key:(NSString *)k object:(uint32_t)o;
- (void)key:(NSString *)k string:(NSString *)s;  // inline NSString object reference helper (needs the object table order)
- (NSData *)archive;
@end

@implementation NibWriter
@synthesize formatVersion;
- (instancetype)init {
    self = [super init];
    classes = [NSMutableArray new]; keys = [NSMutableArray new];
    objects = [NSMutableData new]; values = [NSMutableData new]; pending = [NSMutableArray new];
    formatVersion = 1;
    return self;
}
- (uint32_t)index:(NSString *)s in:(NSMutableArray *)a {
    NSUInteger i = [a indexOfObject:s];
    if (i == NSNotFound) { [a addObject:s]; i = [a count] - 1; }
    return (uint32_t)i;
}
- (uint32_t)object:(NSString *)cls {
    [pending addObject:[NSMutableArray arrayWithObjects:@([self index:cls in:classes]), @(valueCount), @0, nil]];
    return objectCount++;
}
- (void)begin:(NSString *)k type:(int)t {
    varint(values, [self index:k in:keys]);
    uint8_t b = t; [values appendBytes:&b length:1];
    NSMutableArray *last = [pending lastObject];
    last[2] = @([last[2] intValue] + 1);
    valueCount++;
}
- (void)key:(NSString *)k type:(int)t { [self begin:k type:t]; }
- (void)key:(NSString *)k int:(int64_t)v type:(int)t {
    [self begin:k type:t];
    int n = t == tInt8 ? 1 : t == tInt16 ? 2 : t == tInt32 ? 4 : 8;
    for (int i = 0; i < n; i++) { uint8_t b = (uint64_t)v >> (8 * i); [values appendBytes:&b length:1]; }
}
- (void)key:(NSString *)k double:(double)v type:(int)t {
    [self begin:k type:t];
    if (t == tFloat) { float f = v; [values appendBytes:&f length:4]; } else [values appendBytes:&v length:8];
}
- (void)key:(NSString *)k data:(NSData *)d {
    [self begin:k type:tData]; varint(values, (uint32_t)[d length]); [values appendData:d];
}
- (void)key:(NSString *)k object:(uint32_t)o {
    [self begin:k type:tObject]; le32(values, o);
}
- (void)key:(NSString *)k string:(NSString *)s { [self key:k data:[s dataUsingEncoding:NSUTF8StringEncoding]]; }
- (NSData *)archive {
    NSMutableData *o = [NSMutableData data], *k = [NSMutableData data], *c = [NSMutableData data];
    for (NSArray *p in pending) { varint(o, [p[0] intValue]); varint(o, [p[1] intValue]); varint(o, [p[2] intValue]); }
    for (NSString *s in keys) { NSData *b = [s dataUsingEncoding:NSUTF8StringEncoding]; varint(k, (uint32_t)[b length]); [k appendData:b]; }
    for (NSString *s in classes) {
        NSData *b = [s dataUsingEncoding:NSUTF8StringEncoding];
        varint(c, (uint32_t)[b length] + 1); varint(c, 0); [c appendData:b]; uint8_t z = 0; [c appendBytes:&z length:1];
    }
    NSMutableData *a = [NSMutableData dataWithBytes:"NIBArchive" length:10];
    uint32_t base = 50, ko = base + (uint32_t)[o length], vo = ko + (uint32_t)[k length], co = vo + (uint32_t)[values length];
    le32(a, formatVersion); le32(a, 10);
    le32(a, objectCount); le32(a, base); le32(a, (uint32_t)[keys count]); le32(a, ko);
    le32(a, valueCount); le32(a, vo); le32(a, (uint32_t)[classes count]); le32(a, co);
    [a appendData:o]; [a appendData:k]; [a appendData:values]; [a appendData:c];
    return a;
}
@end

// ---- Classes decoded from the archive ----
static int awakeCount;
@interface Node : NSObject <NSCoding> {
    NSString *_name; int _count; BOOL _flag; double _ratio; Node *_child; __unsafe_unretained Node *_parent;
    NSArray *_list; NSDictionary *_dict; NSSet *_set; NSData *_blob; id _other;
}
@property (copy) NSString *name; @property int count; @property BOOL flag; @property double ratio;
@property (strong) Node *child; @property (unsafe_unretained) Node *parent;   // parent: a back reference to an object being decoded
@property (strong) NSArray *list; @property (strong) NSDictionary *dict; @property (strong) NSSet *set;
@property (strong) NSData *blob; @property (strong) id other;
@end
@implementation Node
@synthesize name = _name, count = _count, flag = _flag, ratio = _ratio, child = _child, parent = _parent;
@synthesize list = _list, dict = _dict, set = _set, blob = _blob, other = _other;
- (id)initWithCoder:(NSCoder *)c {
    CHECK([c allowsKeyedCoding]);
    self = [super init];
    _name = [[c decodeObjectForKey:@"name"] copy];
    _count = [c decodeIntForKey:@"count"];
    _flag = [c decodeBoolForKey:@"flag"];
    _ratio = [c decodeDoubleForKey:@"ratio"];
    _child = [c decodeObjectForKey:@"child"];
    _parent = [c decodeObjectForKey:@"parent"];
    _list = [c decodeObjectForKey:@"list"];
    _dict = [c decodeObjectForKey:@"dict"];
    _set = [c decodeObjectForKey:@"set"];
    _blob = [c decodeObjectForKey:@"blob"];
    _other = [c decodeObjectForKey:@"other"];
    return self;
}
- (void)encodeWithCoder:(NSCoder *)c {}
@end
@interface SubstituteNode : Node @end
@implementation SubstituteNode @end
@interface Awakened : NSObject <NSCoding> @end
@implementation Awakened
- (id)initWithCoder:(NSCoder *)c { return [super init]; }
- (void)encodeWithCoder:(NSCoder *)c {}
- (id)awakeAfterUsingCoder:(NSCoder *)c { awakeCount++; return @"replaced by awakeAfterUsingCoder"; }
@end

@interface Recorder : NSObject <UINibDecoderDelegate> { @public NSMutableArray *seen; NSMutableArray *replaced; }
@end
@implementation Recorder
- (id)init { self = [super init]; seen = [NSMutableArray new]; replaced = [NSMutableArray new]; return self; }
- (id)unarchiver:(UINibDecoder *)d didDecodeObject:(id)o {
    [seen addObject:NSStringFromClass([o class])];
    return [o isKindOfClass:[Awakened class]] ? nil : o;
}
- (void)unarchiver:(UINibDecoder *)d willReplaceObject:(id)o withObject:(id)r { [replaced addObject:r]; }
@end

static NSData *sampleArchive(void) {
    NibWriter *w = [NibWriter new];
    // 0: root. Its values are the top-level keys.
    [w object:@"NSObject"];
    [w key:@"int8" int:-5 type:tInt8]; [w key:@"int16" int:-300 type:tInt16];
    [w key:@"int32" int:-70000 type:tInt32]; [w key:@"int64" int:-5000000000LL type:tInt64];
    [w key:@"yes" type:tTrue]; [w key:@"no" type:tFalse];
    [w key:@"float" double:1.5 type:tFloat]; [w key:@"double" double:-2.25 type:tDouble];
    [w key:@"bytes" data:[NSData dataWithBytes:"\x01\x02\x03" length:3]];
    [w key:@"nilvalue" type:tNil];
    [w key:@"string" object:1]; [w key:@"tree" object:2]; [w key:@"tree again" object:2];
    [w key:@"awake" object:7]; [w key:@"missing class" object:8]; [w key:@"number" object:9];
    [w key:@"cyclic" object:10];
    [w key:@"dupe" int:1 type:tInt8]; [w key:@"dupe" int:2 type:tInt8];
    // 1: string object
    [w object:@"NSString"]; [w key:@"NS.bytes" string:@"héllo"];
    // 2: Node with a child, a back reference, collections and a mutable string
    [w object:@"Node"];
    [w key:@"name" object:1]; [w key:@"count" int:42 type:tInt32]; [w key:@"flag" type:tTrue]; [w key:@"ratio" double:0.5 type:tDouble];
    [w key:@"child" object:3]; [w key:@"list" object:4]; [w key:@"dict" object:5]; [w key:@"set" object:6];
    [w key:@"blob" object:11]; [w key:@"other" object:12];
    // 3: child; "parent" refers back to object 2, which is still being decoded
    [w object:@"Node"]; [w key:@"name" object:1]; [w key:@"parent" object:2];
    // 4: array (NSInlinedValue is ignored; elements are UINibEncoderEmptyKey values in order)
    [w object:@"NSMutableArray"]; [w key:@"NSInlinedValue" type:tFalse];
    [w key:@"UINibEncoderEmptyKey" object:1]; [w key:@"UINibEncoderEmptyKey" int:7 type:tInt8]; [w key:@"UINibEncoderEmptyKey" object:3];
    // 5: dictionary: key, value, key, value
    [w object:@"NSDictionary"];
    [w key:@"UINibEncoderEmptyKey" object:1]; [w key:@"UINibEncoderEmptyKey" int:1 type:tInt8];
    [w key:@"UINibEncoderEmptyKey" object:9]; [w key:@"UINibEncoderEmptyKey" object:1];
    // 6: set
    [w object:@"NSSet"]; [w key:@"UINibEncoderEmptyKey" object:1]; [w key:@"UINibEncoderEmptyKey" object:1];
    // 7: class whose -awakeAfterUsingCoder: returns another object
    [w object:@"Awakened"];
    // 8: class that does not exist
    [w object:@"DesignTimeOnlyClass"]; [w key:@"x" int:1 type:tInt8];
    // 9: NSNumber
    [w object:@"NSNumber"]; [w key:@"NS.number" double:3.5 type:tDouble];
    // 10: array that contains itself: cannot be decoded
    [w object:@"NSArray"]; [w key:@"UINibEncoderEmptyKey" object:10];
    // 11: data object
    [w object:@"NSData"]; [w key:@"NS.bytes" data:[NSData dataWithBytes:"blob" length:4]];
    // 12: another Node, reached through an id property
    [w object:@"Node"]; [w key:@"name" object:1];
    return [w archive];
}

static BOOL decodes(NSData *data, NSString *key) {
    UINibDecoder *d = [[UINibDecoder alloc] initForReadingWithData:data];
    if (d == nil) return NO;
    @try { [d decodeObjectForKey:key]; return YES; } @catch (NSException *e) { return YES; }
}

int main(int argc, char **argv) {
    @autoreleasepool {
        NSData *archive = sampleArchive();
        UINibDecoder *d = [[UINibDecoder alloc] initForReadingWithData:archive];
        CHECK(d != nil);
        CHECK([d allowsKeyedCoding]);

        // scalars
        CHECK([d decodeIntForKey:@"int8"] == -5 && [d decodeInt32ForKey:@"int16"] == -300);
        CHECK([d decodeIntForKey:@"int32"] == -70000 && [d decodeInt64ForKey:@"int64"] == -5000000000LL);
        CHECK([d decodeBoolForKey:@"yes"] && ![d decodeBoolForKey:@"no"] && ![d decodeBoolForKey:@"absent"]);
        CHECK([d decodeFloatForKey:@"float"] == 1.5f && [d decodeDoubleForKey:@"double"] == -2.25);
        CHECK([d decodeDoubleForKey:@"int8"] == -5.0 && [d decodeIntForKey:@"double"] == -2);
        CHECK([d decodeIntForKey:@"absent"] == 0 && [d decodeDoubleForKey:@"absent"] == 0);
        CHECK([d decodeIntForKey:@"dupe"] == 2); // the last value of a repeated key wins
        NSUInteger len = 99;
        const uint8_t *bytes = [d decodeBytesForKey:@"bytes" returnedLength:&len];
        CHECK(bytes != NULL && len == 3 && bytes[2] == 3);
        bytes = [d decodeBytesForKey:@"absent" returnedLength:&len];
        CHECK(bytes == NULL && len == 0);
        CHECK([[d decodeObjectForKey:@"bytes"] isEqual:[NSData dataWithBytes:"\x01\x02\x03" length:3]]);
        CHECK([[d decodeObjectForKey:@"int16"] isEqual:@(-300)] && [[d decodeObjectForKey:@"yes"] isEqual:@YES]);

        // containsValueForKey:
        CHECK([d containsValueForKey:@"int8"] && [d containsValueForKey:@"nilvalue"]);
        CHECK(![d containsValueForKey:@"absent"] && ![d containsValueForKey:@"count"]); // "count" belongs to another object
        CHECK([d decodeObjectForKey:@"nilvalue"] == nil && [d decodeObjectForKey:@"absent"] == nil);

        // objects: strings, nesting, identity, back references, collections
        CHECK([[d decodeObjectForKey:@"string"] isEqualToString:@"héllo"]);
        Node *tree = [d decodeObjectForKey:@"tree"];
        CHECK([tree isKindOfClass:[Node class]]);
        CHECK(tree == [d decodeObjectForKey:@"tree again"]);
        CHECK([tree.name isEqualToString:@"héllo"] && tree.count == 42 && tree.flag && tree.ratio == 0.5);
        CHECK(tree.child != nil && tree.child.parent == tree); // the back reference is the same object
        CHECK([tree.list count] == 3 && [tree.list isKindOfClass:[NSMutableArray class]]);
        CHECK([tree.list[0] isEqual:@"héllo"] && [tree.list[1] isEqual:@7] && tree.list[2] == tree.child);
        CHECK([tree.dict count] == 2 && [tree.dict[@"héllo"] isEqual:@1] && [tree.dict[@3.5] isEqual:@"héllo"]);
        CHECK([tree.set count] == 1 && [tree.set containsObject:@"héllo"]);
        CHECK([tree.blob isEqual:[NSData dataWithBytes:"blob" length:4]]);
        CHECK([tree.other isKindOfClass:[Node class]] && tree.other != tree.child);
        CHECK([[d decodeObjectForKey:@"number"] isEqual:@3.5]);
        CHECK([d decodeObjectForKey:@"missing class"] == nil);   // a class that does not exist decodes as nil

        // an array that contains itself is invalid
        BOOL raised = NO;
        @try { [d decodeObjectForKey:@"cyclic"]; } @catch (NSException *e) { raised = [[e name] isEqualToString:NSInvalidArchiveOperationException]; }
        CHECK(raised);

        // awakeAfterUsingCoder:, the delegate and class substitution (a fresh decoder)
        d = [[UINibDecoder alloc] initForReadingWithData:archive];
        Recorder *rec = [Recorder new];
        d.delegate = rec;
        [d setClass:[SubstituteNode class] forClassName:@"Node"];
        CHECK([d classForClassName:@"Node"] == [SubstituteNode class] && [d classForClassName:@"Other"] == Nil);
        int before = awakeCount;
        id awake = [d decodeObjectForKey:@"awake"];
        CHECK(awakeCount == before + 1);
        CHECK([awake isEqual:@"replaced by awakeAfterUsingCoder"]);
        Node *sub = [d decodeObjectForKey:@"tree"];
        CHECK([sub isMemberOfClass:[SubstituteNode class]] && [sub.child isMemberOfClass:[SubstituteNode class]]);
        // the delegate sees objects made through their class and collections, not strings, data or numbers
        CHECK([rec->seen containsObject:@"SubstituteNode"]);
        for (NSString *name in rec->seen)
            CHECK(![name containsString:@"String"] && ![name containsString:@"Data"] && ![name containsString:@"Number"]);

        // replaceObject:withObject: redirects later references and tells the delegate
        d = [[UINibDecoder alloc] initForReadingWithData:archive];
        rec = [Recorder new]; d.delegate = rec;
        Node *first = [d decodeObjectForKey:@"tree"];
        Node *replacement = [Node new];
        [d replaceObject:first withObject:replacement];
        CHECK([d decodeObjectForKey:@"tree again"] == replacement && [d decodeObjectForKey:@"tree"] == replacement);
        CHECK([rec->replaced count] == 1 && rec->replaced[0] == replacement);

        // unkeyed coding is refused
        raised = NO;
        @try { int x; [d decodeValueOfObjCType:@encode(int) at:&x]; } @catch (NSException *e) { raised = [[e name] isEqualToString:NSInvalidArchiveOperationException]; }
        CHECK(raised);

        // an unpaired dictionary key is invalid
        {
            NibWriter *w = [NibWriter new];
            [w object:@"NSObject"]; [w key:@"d" object:1];
            [w object:@"NSDictionary"]; [w key:@"UINibEncoderEmptyKey" int:1 type:tInt8];
            UINibDecoder *u = [[UINibDecoder alloc] initForReadingWithData:[w archive]];
            raised = NO;
            @try { [u decodeObjectForKey:@"d"]; } @catch (NSException *e) { raised = [[e name] isEqualToString:NSInvalidArchiveOperationException]; }
            CHECK(u != nil && raised);
        }

        // ---- corrupt data ----
        CHECK([[UINibDecoder alloc] initForReadingWithData:[NSData data]] == nil);
        CHECK([[UINibDecoder alloc] initForReadingWithData:[NSData dataWithBytes:"NIBArchive" length:10]] == nil);
        NSMutableData *bad = [archive mutableCopy];
        ((uint8_t *)[bad mutableBytes])[3] ^= 0x20;
        CHECK([[UINibDecoder alloc] initForReadingWithData:bad] == nil);            // bad magic
        NibWriter *newer = [NibWriter new]; newer.formatVersion = 2;
        [newer object:@"NSObject"];
        CHECK([[UINibDecoder alloc] initForReadingWithData:[newer archive]] == nil); // a format version nothing here reads
        NibWriter *empty = [NibWriter new];
        CHECK([[UINibDecoder alloc] initForReadingWithData:[empty archive]] == nil);  // no root object
        for (NSUInteger n = 0; n < [archive length]; n++)                            // every proper prefix
            CHECK([[UINibDecoder alloc] initForReadingWithData:[archive subdataWithRange:NSMakeRange(0, n)]] == nil);
        // The counts and offsets of the header (words 2 to 9 after the magic) set out of range.
        for (int word = 2; word < 10; word++) {
            NSMutableData *m = [archive mutableCopy];
            uint8_t big[4] = {0xff, 0xff, 0xff, 0x7f};
            memcpy((uint8_t *)[m mutableBytes] + 10 + 4 * word, big, 4);
            CHECK([[UINibDecoder alloc] initForReadingWithData:m] == nil);
        }
        // single-byte damage: never crashes; decoding either works or raises
        NSArray *keys = @[@"tree", @"string", @"awake", @"number", @"cyclic", @"bytes"];
        for (NSUInteger i = 50; i < [archive length]; i++) {
            for (int v = 0; v < 3; v++) {
                NSMutableData *m = [archive mutableCopy];
                ((uint8_t *)[m mutableBytes])[i] = v == 0 ? 0xff : v == 1 ? 0x00 : ((uint8_t *)[m mutableBytes])[i] + 0x40;
                for (NSString *k in keys) decodes(m, k);
            }
        }

        // ---- compiled nibs given on the command line ----
        for (int i = 1; i < argc; i++) {
            NSData *nib = [NSData dataWithContentsOfFile:[NSString stringWithUTF8String:argv[i]]];
            UINibDecoder *u = [[UINibDecoder alloc] initForReadingWithData:nib];
            CHECK(nib != nil && u != nil);
            CHECK([u containsValueForKey:@"IB.objectdata"]);
            NSUInteger step = [nib length] > 4000 ? 7 : 1;
            for (NSUInteger n = 0; n < [nib length]; n += step)
                if ([[UINibDecoder alloc] initForReadingWithData:[nib subdataWithRange:NSMakeRange(0, n)]] != nil) {
                    fprintf(stderr, "FAIL: %s truncated to %lu was accepted\n", argv[i], (unsigned long)n);
                    failures++;
                    break;
                }
            for (NSUInteger n = 50; n < [nib length]; n += 3) {
                NSMutableData *m = [nib mutableCopy];
                ((uint8_t *)[m mutableBytes])[n] ^= 0xa5;
                decodes(m, @"IB.objectdata");
            }
        }
        if (!failures) puts("PASS");
    }
    return failures ? 1 : 0;
}
