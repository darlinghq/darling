// CryptoTokenKit TKTLVRecord and its BER, Simple and Compact subclasses. Expected octets come from ITU-T X.690
// 8.1.2 / 8.1.3 / 10.1 (BER identifier and length octets, DER shortest length) and ISO/IEC 7816-4 5.4.4
// (Simple-TLV) and 8.3 (Compact-TLV), and, where the standards and the documentation are silent, from black-box
// observation on macOS 26.7.3 (x86_64): every check passes on that system unchanged. The checks that depend on
// such an observation are marked "observed".
// Prints nothing on success; the exit code is the number of failed checks.
#import <Foundation/Foundation.h>
#import <CryptoTokenKit/CryptoTokenKit.h>

static int failures;
#define CHECK(...) do { if (!(__VA_ARGS__)) { printf("FAIL line %d: %s\n", __LINE__, #__VA_ARGS__); failures++; } } while (0)

#define DATA(...) ({ const unsigned char b[] = { __VA_ARGS__ }; [NSData dataWithBytes:b length:sizeof b]; })

static BOOL raisesName(NSString *name, void (^block)(void))
{
	@try { block(); } @catch (NSException *e) { return [e.name isEqualToString:name]; }
	return NO;
}

static void test_ber(void)
{
	TKBERTLVRecord *r = [[[TKBERTLVRecord alloc] initWithTag:0x61 value:DATA(0xaa, 0xbb, 0xcc)] autorelease];
	CHECK(r.tag == 0x61);
	CHECK([r.value isEqualToData:DATA(0xaa, 0xbb, 0xcc)]);
	CHECK([r.data isEqualToData:DATA(0x61, 0x03, 0xaa, 0xbb, 0xcc)]);

	// Long form for 128 value octets: 81 80 (shortest definite length, X.690 10.1); 256 needs 82 01 00.
	NSData *big = [NSMutableData dataWithLength:128];
	TKBERTLVRecord *longer = [[[TKBERTLVRecord alloc] initWithTag:0x04 value:big] autorelease];
	CHECK(longer.data.length == 3 + 128 && [[longer.data subdataWithRange:NSMakeRange(0, 3)] isEqualToData:DATA(0x04, 0x81, 0x80)]);
	TKBERTLVRecord *longest = [[[TKBERTLVRecord alloc] initWithTag:0x04 value:[NSMutableData dataWithLength:256]] autorelease];
	CHECK([[longest.data subdataWithRange:NSMakeRange(0, 4)] isEqualToData:DATA(0x04, 0x82, 0x01, 0x00)]);
	TKBERTLVRecord *shortest = [[[TKBERTLVRecord alloc] initWithTag:0x04 value:[NSMutableData dataWithLength:127]] autorelease];
	CHECK([[shortest.data subdataWithRange:NSMakeRange(0, 2)] isEqualToData:DATA(0x04, 0x7f)]);

	// Constructed record: the value is the concatenation of the sub-records' encodings, whatever their encoding.
	TKBERTLVRecord *a = [[[TKBERTLVRecord alloc] initWithTag:0x80 value:DATA(1)] autorelease];
	TKBERTLVRecord *b = [[[TKBERTLVRecord alloc] initWithTag:0x81 value:DATA(2, 3)] autorelease];
	TKBERTLVRecord *parent = [[[TKBERTLVRecord alloc] initWithTag:0x61 records:@[a, b]] autorelease];
	CHECK([parent.value isEqualToData:DATA(0x80, 0x01, 0x01, 0x81, 0x02, 0x02, 0x03)]);
	CHECK([parent.data isEqualToData:DATA(0x61, 0x07, 0x80, 0x01, 0x01, 0x81, 0x02, 0x02, 0x03)]);
	TKSimpleTLVRecord *simple = [[[TKSimpleTLVRecord alloc] initWithTag:5 value:DATA(1)] autorelease];
	CHECK([[[[TKBERTLVRecord alloc] initWithTag:0x61 records:@[a, simple]] autorelease].data isEqualToData:DATA(0x61, 0x06, 0x80, 0x01, 0x01, 0x05, 0x01, 0x01)]);
	CHECK([[[[TKBERTLVRecord alloc] initWithTag:0x61 records:@[]] autorelease].data isEqualToData:DATA(0x61, 0x00)]);

	// Parsing the constructed record gives back its children.
	TKBERTLVRecord *parsed = [TKBERTLVRecord recordFromData:parent.data];
	NSArray *children = [TKBERTLVRecord sequenceOfRecordsFromData:parsed.value];
	CHECK(parsed.tag == 0x61 && children.count == 2);
	CHECK([children[0] tag] == 0x80 && [[children[1] value] isEqualToData:DATA(2, 3)]);
	CHECK([parsed.data isEqualToData:parent.data]);

	// Records are compared by identity (observed: parsed and built records with equal octets are not equal).
	CHECK(![parsed isEqual:parent] && [parsed isEqual:parsed]);

	// High tag number form (X.690 8.1.2.4): 7F 21, 5F 20.
	CHECK([[TKBERTLVRecord dataForTag:0x7f21] isEqualToData:DATA(0x7f, 0x21)]);
	CHECK([[TKBERTLVRecord dataForTag:0x61] isEqualToData:DATA(0x61)]);
	TKBERTLVRecord *high = [TKBERTLVRecord recordFromData:DATA(0x7f, 0x21, 0x01, 0x05)];
	CHECK(high.tag == 0x7f21 && [high.value isEqualToData:DATA(5)]);
	CHECK([TKBERTLVRecord recordFromData:DATA(0x5f, 0x20, 0x01, 0x01)].tag == 0x5f20);

	// Observed: only tag 0 is refused; any other integer encodes as its minimal big-endian octets, also when
	// those are not well-formed identifier octets (1F alone, 0101).
	CHECK(raisesName(NSInvalidArgumentException, ^{ [[[TKBERTLVRecord alloc] initWithTag:0 value:DATA(1)] autorelease]; }));
	CHECK(raisesName(NSInvalidArgumentException, ^{ [TKBERTLVRecord dataForTag:0]; }));
	CHECK([[TKBERTLVRecord dataForTag:0x1f] isEqualToData:DATA(0x1f)]);
	CHECK([[TKBERTLVRecord dataForTag:0x0101] isEqualToData:DATA(0x01, 0x01)]);
	CHECK([[[[TKBERTLVRecord alloc] initWithTag:0x1f value:DATA(1)] autorelease].data isEqualToData:DATA(0x1f, 0x01, 0x01)]);
	CHECK([[[[TKBERTLVRecord alloc] initWithTag:0xff value:DATA(1)] autorelease].data isEqualToData:DATA(0xff, 0x01, 0x01)]);
	CHECK([[[[TKBERTLVRecord alloc] initWithTag:4 value:nil] autorelease].data isEqualToData:DATA(0x04, 0x00)]);

	// Observed: high tag number octets are read as they stand, minimal or not (1F 80 01, 1F 05), up to 8 octets.
	CHECK([TKBERTLVRecord recordFromData:DATA(0x1f, 0x80, 0x01, 0x00)].tag == 0x1f8001);
	CHECK([TKBERTLVRecord recordFromData:DATA(0x1f, 0x05, 0x00)].tag == 0x1f05);
	// TKTLVTag is as wide as NSUInteger, so a tag of 8 octets decodes only where that is 64 bits (not on i386).
	CHECK(([TKBERTLVRecord recordFromData:DATA(0x1f, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x01, 0x00)] != nil) == (sizeof(TKTLVTag) == 8));
	CHECK([TKBERTLVRecord recordFromData:DATA(0x1f, 0x81, 0x81, 0x01, 0x00)] != nil);
	CHECK([TKBERTLVRecord recordFromData:DATA(0x1f, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x01, 0x00)] == nil);
	// Tag cut off, identifier octet 00, tag 00 with a length.
	CHECK([TKBERTLVRecord recordFromData:DATA(0x1f)] == nil);
	CHECK([TKBERTLVRecord recordFromData:DATA(0x1f, 0x81)] == nil);
	CHECK([TKBERTLVRecord recordFromData:DATA(0x00, 0x00)] == nil);
	CHECK([TKBERTLVRecord recordFromData:DATA(0xff, 0x00)] == nil);

	// BER input with a non-minimal length is accepted and keeps its original encoding (81 01).
	TKBERTLVRecord *loose = [TKBERTLVRecord recordFromData:DATA(0x04, 0x81, 0x01, 0x09)];
	CHECK(loose != nil && [loose.value isEqualToData:DATA(9)] && loose.data.length == 4);

	// Observed: octets after a definite-length record are ignored by +recordFromData:, and +sequence... reads on.
	TKBERTLVRecord *first = [TKBERTLVRecord recordFromData:DATA(0x04, 0x01, 0x09, 0x00)];
	CHECK(first != nil && first.data.length == 3 && [first.value isEqualToData:DATA(9)]);

	// Malformed input: truncated value or length, reserved length octet, empty data.
	CHECK([TKBERTLVRecord recordFromData:DATA(0x04, 0x02, 0x09)] == nil);
	CHECK([TKBERTLVRecord recordFromData:DATA(0x04)] == nil);
	CHECK([TKBERTLVRecord recordFromData:DATA(0x04, 0xff, 0x00)] == nil);
	CHECK([TKBERTLVRecord recordFromData:DATA(0x04, 0x84, 0xff, 0xff, 0xff, 0xff)] == nil);
	CHECK([TKBERTLVRecord recordFromData:DATA(0x61)] == nil);
	CHECK([TKBERTLVRecord recordFromData:[NSData data]] == nil);

	// Indefinite length (X.690 8.1.3.6): the content is a run of records ended by 00 00, and, observed, such a
	// record has to be the last thing in the data.
	TKBERTLVRecord *indefinite = [TKBERTLVRecord recordFromData:DATA(0x30, 0x80, 0x04, 0x01, 0x09, 0x00, 0x00)];
	CHECK(indefinite != nil && indefinite.tag == 0x30 && [indefinite.value isEqualToData:DATA(0x04, 0x01, 0x09)]);
	CHECK([indefinite.data isEqualToData:DATA(0x30, 0x80, 0x04, 0x01, 0x09, 0x00, 0x00)]);
	CHECK([[TKBERTLVRecord recordFromData:DATA(0x30, 0x80, 0x00, 0x00)].value length] == 0);
	CHECK([[TKBERTLVRecord recordFromData:DATA(0x30, 0x80, 0x30, 0x80, 0x00, 0x00, 0x00, 0x00)].value isEqualToData:DATA(0x30, 0x80, 0x00, 0x00)]);
	CHECK([TKBERTLVRecord recordFromData:DATA(0x30, 0x80, 0x04, 0x01, 0x09)] == nil);
	CHECK([TKBERTLVRecord recordFromData:DATA(0x30, 0x80, 0x00)] == nil);
	CHECK([TKBERTLVRecord recordFromData:DATA(0x30, 0x80, 0xff)] == nil);
	CHECK([TKBERTLVRecord recordFromData:DATA(0x30, 0x80, 0x04, 0x05, 0x09, 0x00, 0x00)] == nil);
	CHECK([TKBERTLVRecord recordFromData:DATA(0x30, 0x80, 0x04, 0x01, 0x09, 0x00, 0x00, 0x05, 0x00)] == nil);
	CHECK([[TKBERTLVRecord sequenceOfRecordsFromData:DATA(0x30, 0x80, 0x00, 0x00, 0x31, 0x80, 0x00, 0x00)] count] == 2);
	CHECK([TKBERTLVRecord recordFromData:DATA(0x30, 0x80, 0x00, 0x00, 0x31, 0x80, 0x00, 0x00)] == nil);
	// An indefinite record inside a definite one is not parsed: the definite length governs.
	CHECK([[TKBERTLVRecord recordFromData:DATA(0x30, 0x05, 0x30, 0x80, 0x00, 0x00, 0x00)].value length] == 5);
	// QUIRK test: nesting of indefinite records is accepted to a depth of 33 (observed), not 34.
	for (int depth = 1; depth <= 35; depth++) {
		NSMutableData *nested = [NSMutableData data];
		for (int i = 0; i < depth; i++)
			[nested appendData:DATA(0x30, 0x80)];
		for (int i = 0; i < depth; i++)
			[nested appendData:DATA(0x00, 0x00)];
		CHECK(([TKBERTLVRecord recordFromData:nested] != nil) == (depth <= 33));
	}

	NSArray *two = [TKBERTLVRecord sequenceOfRecordsFromData:DATA(0x04, 0x01, 0x07, 0x05, 0x00)];
	CHECK(two.count == 2 && [two[0] tag] == 0x04 && [two[1] tag] == 0x05 && [[two[1] value] length] == 0);
	CHECK([TKBERTLVRecord sequenceOfRecordsFromData:DATA(0x04, 0x01, 0x07, 0x05)] == nil);
	CHECK([TKBERTLVRecord sequenceOfRecordsFromData:DATA(0x04, 0x00, 0x30, 0x80)] == nil);
	// Observed: no padding is skipped between records.
	CHECK([TKBERTLVRecord sequenceOfRecordsFromData:DATA(0x00, 0x00, 0x04, 0x00)] == nil);
	// Data holding zero records is a valid (empty) sequence.
	CHECK([[TKBERTLVRecord sequenceOfRecordsFromData:[NSData data]] count] == 0);

	// QUIRK test, "abstract class": the base class cannot decode and raises, except for an empty sequence (observed).
	CHECK(raisesName(NSInternalInconsistencyException, ^{ [TKTLVRecord recordFromData:DATA(0x04, 0x00)]; }));
	CHECK(raisesName(NSInternalInconsistencyException, ^{ [TKTLVRecord sequenceOfRecordsFromData:DATA(0x04, 0x00)]; }));
	CHECK([[TKTLVRecord sequenceOfRecordsFromData:[NSData data]] count] == 0);
}

static void test_simple(void)
{
	TKSimpleTLVRecord *r = [[[TKSimpleTLVRecord alloc] initWithTag:5 value:DATA(0x11, 0x22)] autorelease];
	CHECK(r.tag == 5 && [r.data isEqualToData:DATA(0x05, 0x02, 0x11, 0x22)]);

	// 255 value octets need the FF escape: FF 00 FF (7816-4 5.4.4); 254 still fit one length octet.
	NSData *v255 = [NSMutableData dataWithLength:255];
	TKSimpleTLVRecord *escaped = [[[TKSimpleTLVRecord alloc] initWithTag:2 value:v255] autorelease];
	CHECK([[escaped.data subdataWithRange:NSMakeRange(0, 4)] isEqualToData:DATA(0x02, 0xff, 0x00, 0xff)]);
	TKSimpleTLVRecord *back = [TKSimpleTLVRecord recordFromData:escaped.data];
	CHECK(back.tag == 2 && back.value.length == 255);
	NSData *v254 = [NSMutableData dataWithLength:254];
	CHECK([[[[[TKSimpleTLVRecord alloc] initWithTag:2 value:v254] autorelease] data] length] == 2 + 254);
	CHECK([[[[[TKSimpleTLVRecord alloc] initWithTag:2 value:[NSMutableData dataWithLength:65535]] autorelease] data] length] == 4 + 65535);
	// The escape also decodes for a short value.
	CHECK([[TKSimpleTLVRecord recordFromData:DATA(0x05, 0xff, 0x00, 0x01, 0x09)].value isEqualToData:DATA(9)]);
	CHECK([TKSimpleTLVRecord recordFromData:DATA(0x05, 0xff, 0x00)] == nil);
	CHECK([TKSimpleTLVRecord recordFromData:DATA(0x05, 0xff, 0x00, 0xfe)] == nil);

	// Tags 00 and FF are not tags when decoding; observed: they encode all the same.
	CHECK([TKSimpleTLVRecord recordFromData:DATA(0x00, 0x00)] == nil);
	CHECK([TKSimpleTLVRecord recordFromData:DATA(0xff, 0x00)] == nil);
	CHECK([[[[TKSimpleTLVRecord alloc] initWithTag:0 value:DATA(1)] autorelease].data isEqualToData:DATA(0x00, 0x01, 0x01)]);
	CHECK([[[[TKSimpleTLVRecord alloc] initWithTag:0xff value:DATA(1)] autorelease].data isEqualToData:DATA(0xff, 0x01, 0x01)]);

	// Truncated input, and octets after the record (ignored, observed).
	CHECK([TKSimpleTLVRecord recordFromData:DATA(0x01, 0x05, 0x01)] == nil);
	CHECK([TKSimpleTLVRecord recordFromData:DATA(0x01)] == nil);
	CHECK([TKSimpleTLVRecord recordFromData:[NSData data]] == nil);
	CHECK([[TKSimpleTLVRecord recordFromData:DATA(0x01, 0x02, 0xaa, 0xbb, 0x00)].data length] == 4);
	NSArray *seq = [TKSimpleTLVRecord sequenceOfRecordsFromData:DATA(0x01, 0x00, 0x02, 0x01, 0x09)];
	CHECK(seq.count == 2 && [[seq[1] value] isEqualToData:DATA(9)]);
	CHECK([TKSimpleTLVRecord sequenceOfRecordsFromData:DATA(0x00, 0x00, 0x01, 0x00)] == nil);

	// A value that does not fit the encoding raises NSInvalidArgumentException.
	CHECK(raisesName(NSInvalidArgumentException, ^{ [[[TKSimpleTLVRecord alloc] initWithTag:1 value:[NSMutableData dataWithLength:65536]] autorelease]; }));
}

static void test_compact(void)
{
	// 7816-4 8.3: octet XY, X the tag number, Y the length.
	TKCompactTLVRecord *r = [[[TKCompactTLVRecord alloc] initWithTag:3 value:DATA(0xc0)] autorelease];
	CHECK([r.data isEqualToData:DATA(0x31, 0xc0)] && [r.value isEqualToData:DATA(0xc0)]);
	// The tag property is the 4-bit tag number, not the first octet.
	CHECK(r.tag == 3);

	TKCompactTLVRecord *parsed = [TKCompactTLVRecord recordFromData:DATA(0x31, 0xc0)];
	CHECK(parsed.tag == 3 && [parsed.data isEqualToData:r.data]);
	TKCompactTLVRecord *empty = [TKCompactTLVRecord recordFromData:DATA(0x80)];
	CHECK(empty.tag == 8 && empty.value.length == 0);
	CHECK([TKCompactTLVRecord recordFromData:DATA(0x00)].tag == 0);
	CHECK([TKCompactTLVRecord recordFromData:DATA(0xf0)].tag == 15);
	CHECK([TKCompactTLVRecord recordFromData:DATA(0x32, 0xc0)] == nil);
	CHECK([TKCompactTLVRecord recordFromData:[NSData data]] == nil);
	// Octets after the record are ignored (observed).
	CHECK([[TKCompactTLVRecord recordFromData:DATA(0x31, 0xc0, 0x00)].data length] == 2);
	NSArray *seq = [TKCompactTLVRecord sequenceOfRecordsFromData:DATA(0x31, 0xc0, 0x80, 0x11, 0x01)];
	CHECK(seq.count == 3 && [seq[2] tag] == 1);
	CHECK([TKCompactTLVRecord sequenceOfRecordsFromData:DATA(0x31, 0xc0, 0x35, 0x01)] == nil);

	// A tag above 15 or a value above 15 octets raises NSInvalidArgumentException; 15 and 15 are the largest.
	CHECK(raisesName(NSInvalidArgumentException, ^{ [[[TKCompactTLVRecord alloc] initWithTag:16 value:DATA(1)] autorelease]; }));
	CHECK(raisesName(NSInvalidArgumentException, ^{ [[[TKCompactTLVRecord alloc] initWithTag:0x31 value:DATA(1)] autorelease]; }));
	CHECK(raisesName(NSInvalidArgumentException, ^{ [[[TKCompactTLVRecord alloc] initWithTag:1 value:[NSMutableData dataWithLength:16]] autorelease]; }));
	TKCompactTLVRecord *largest = [[[TKCompactTLVRecord alloc] initWithTag:15 value:[NSMutableData dataWithLength:15]] autorelease];
	CHECK(largest.data.length == 16 && ((const unsigned char *)largest.data.bytes)[0] == 0xff);
}

int main(void)
{
	@autoreleasepool {
		test_ber();
		test_simple();
		test_compact();
	}
	return failures;
}
