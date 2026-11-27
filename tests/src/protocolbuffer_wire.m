// Copyright (C) 2026 Lubos Dolezel
//
// ProtocolBuffer reads and writes the documented wire format
// (https://protobuf.dev/programming-guides/encoding/): a message class written the
// way generated code is, with a varint, a string, data, a nested message, a
// negative int32, a zigzag sint32 and a fixed32; the bytes the writer produces, the
// message the reader gets back, a field the reader does not know (every wire
// type, a group among them), and data that is cut off or malformed.
//
// Compile with the private header directory of ProtocolBuffer on the include path
// and link against the installed framework. Prints PASS or FAIL lines.
#import <Foundation/Foundation.h>
#import <ProtocolBuffer/ProtocolBuffer.h>
#import <ProtocolBuffer/PBConstants.h>
#include <stdio.h>
#include <string.h>

static int failures;
static void expect(BOOL ok, NSString *what) {
    printf("%s %s\n", ok ? "PASS" : "FAIL", [what UTF8String]);
    if (!ok)
        failures++;
}

@interface Inner : PBCodable {
@public
    uint32_t number;
}
@end

@implementation Inner
- (BOOL)readFrom:(PBDataReader *)reader {
    while (PBReaderHasMoreData(reader)) {
        uint32_t tag = 0;
        uint8_t type = 0;
        PBReaderReadTag32AndType(reader, &tag, &type);
        if (PBReaderHasError(reader))
            break;
        if (tag == 1)
            number = PBReaderReadUint32(reader);
        else if (!PBReaderSkipValueWithTag(reader, tag, type))
            return NO;
    }
    return !PBReaderHasError(reader);
}
- (void)writeTo:(PBDataWriter *)writer {
    PBDataWriterWriteUint32Field(writer, number, 1);
}
@end

@interface Message : PBCodable {
@public
    uint32_t varint;
    NSString *string;
    NSData *blob;
    Inner *inner;
    int32_t negative;
    int32_t zigzag;
    int32_t fixed;
}
@end

@implementation Message
- (BOOL)readFrom:(PBDataReader *)reader {
    while (PBReaderHasMoreData(reader)) {
        uint32_t tag = 0;
        uint8_t type = 0;
        PBReaderReadTag32AndType(reader, &tag, &type);
        if (PBReaderHasError(reader))
            break;
        if (type == TYPE_END_GROUP)
            break;
        switch (tag) {
        case 1: varint = PBReaderReadUint32(reader); break;
        case 2: string = [PBReaderReadString(reader) retain]; break;
        case 3: blob = [PBReaderReadData(reader) retain]; break;
        case 4: {
            PBDataReaderMark mark;
            inner = [[Inner alloc] init];
            if (!PBReaderPlaceMark(reader, &mark) || ![inner readFrom: reader])
                return NO;
            PBReaderRecallMark(reader, &mark);
            break;
        }
        case 5: negative = PBReaderReadInt32(reader); break;
        default:
            if (!PBReaderSkipValueWithTag(reader, tag, type))
                return NO;
        }
    }
    return !PBReaderHasError(reader);
}
- (void)writeTo:(PBDataWriter *)writer {
    PBDataWriterWriteUint32Field(writer, varint, 1);
    PBDataWriterWriteStringField(writer, string, 2);
    PBDataWriterWriteDataField(writer, blob, 3);
    PBDataWriterWriteSubmessage(writer, inner, 4);
    PBDataWriterWriteInt32Field(writer, negative, 5);
    PBDataWriterWriteSint32Field(writer, zigzag, 6);
    PBDataWriterWriteFixed32Field(writer, fixed, 7);
}
@end

static NSData *bytes(const char *b, size_t n) { return [NSData dataWithBytes: b length: n]; }

int main(void) {
    @autoreleasepool {
        // the example of the encoding guide: field 1 holding 150 is 08 96 01
        Inner *one = [[Inner alloc] init];
        one->number = 150;
        expect([[one data] isEqual: bytes("\x08\x96\x01", 3)], @"a varint field is encoded as in the encoding guide");
        Inner *back = [[Inner alloc] initWithData: bytes("\x08\x96\x01", 3)];
        expect(back != nil && back->number == 150, @"and decoded");

        Message *m = [[Message alloc] init];
        m->varint = 300;
        m->string = @"testing";
        m->blob = bytes("\x00\xff\x7f", 3);
        m->inner = one;
        m->negative = -2;
        m->zigzag = -1;
        m->fixed = 0x01020304;
        NSData *encoded = [m data];
        // 08 ac 02 | 12 07 "testing" | 1a 03 00 ff 7f | 22 03 08 96 01 | 28 fe ff ff ff ff ff ff ff ff 01 | 30 01 | 3d 04 03 02 01
        const char expected[] = "\x08\xac\x02\x12\x07testing\x1a\x03\x00\xff\x7f\x22\x03\x08\x96\x01"
                                "\x28\xfe\xff\xff\xff\xff\xff\xff\xff\xff\x01\x30\x01\x3d\x04\x03\x02\x01";
        expect([encoded isEqual: bytes(expected, sizeof(expected) - 1)], @"all field types are encoded as documented");

        Message *r = [[Message alloc] initWithData: encoded];
        expect(r != nil && r->varint == 300 && [r->string isEqual: @"testing"] && [r->blob isEqual: m->blob] &&
                   r->inner != nil && r->inner->number == 150 && r->negative == -2,
               @"the message is read back");

        // unknown fields of every wire type are skipped: a varint, 64-bit, length-delimited, group and 32-bit value
        NSMutableData *extra = [NSMutableData dataWithData: encoded];
        [extra appendBytes: "\x40\x05\x49\x01\x02\x03\x04\x05\x06\x07\x08\x52\x02\xaa\xbb\x5b\x08\x01\x5c\x65\x01\x02\x03\x04" length: 24];
        // tag 8 varint, tag 9 fixed64, tag 10 bytes, tag 11 group (08 01) ended by 5c, tag 12 fixed32
        Message *s = [[Message alloc] initWithData: extra];
        expect(s != nil && s->varint == 300 && s->negative == -2, @"unknown fields of every wire type are skipped");

        expect([[Message alloc] initWithData: bytes("\x12\x07tes", 5)] == nil, @"a string cut off is an error");
        expect([[Message alloc] initWithData: bytes("\x08\xff", 2)] == nil, @"a varint cut off is an error");
        expect([[Message alloc] initWithData: bytes("\x47\x01", 2)] == nil, @"an invalid wire type is an error");
        expect([[Message alloc] initWithData: [NSData data]] != nil, @"empty data is an empty message");
    }
    printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
    return failures != 0;
}
