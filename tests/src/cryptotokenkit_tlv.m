// CryptoTokenKit TKTLVRecord and its BER, Simple and Compact subclasses. Expected octets are worked out from
// ITU-T X.690 8.1.2 / 8.1.3 / 10.1 (BER identifier and length octets, DER shortest length) and from ISO/IEC 7816-4
// 5.4.4 (Simple-TLV) and 8.3 (Compact-TLV), 2005 edition numbering; nothing here was observed on macOS, so the
// behaviours the documentation leaves open (see the pull request's open questions) are pinned only where marked.
// Prints nothing on success; the exit code is the number of failed checks.
#import <Foundation/Foundation.h>
#import <CryptoTokenKit/CryptoTokenKit.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)

#define DATA(...) ({ const unsigned char b[] = { __VA_ARGS__ }; [NSData dataWithBytes:b length:sizeof b]; })

static void test_ber(void)
{
	TKBERTLVRecord *r = [[[TKBERTLVRecord alloc] initWithTag:0x61 value:DATA(0xaa, 0xbb, 0xcc)] autorelease];
	CHECK(r.tag == 0x61);
	CHECK([r.value isEqualToData:DATA(0xaa, 0xbb, 0xcc)]);
	CHECK([r.data isEqualToData:DATA(0x61, 0x03, 0xaa, 0xbb, 0xcc)]);

	// Long form for 128 value octets: 81 80 (shortest definite length, X.690 10.1).
	NSData *big = [NSMutableData dataWithLength:128];
	TKBERTLVRecord *longer = [[[TKBERTLVRecord alloc] initWithTag:0x04 value:big] autorelease];
	CHECK(longer.data.length == 3 + 128 && [[longer.data subdataWithRange:NSMakeRange(0, 3)] isEqualToData:DATA(0x04, 0x81, 0x80)]);

	// Constructed record: the value is the concatenation of the sub-records' encodings.
	TKBERTLVRecord *a = [[[TKBERTLVRecord alloc] initWithTag:0x80 value:DATA(1)] autorelease];
	TKBERTLVRecord *b = [[[TKBERTLVRecord alloc] initWithTag:0x81 value:DATA(2, 3)] autorelease];
	TKBERTLVRecord *parent = [[[TKBERTLVRecord alloc] initWithTag:0x61 records:@[a, b]] autorelease];
	CHECK([parent.value isEqualToData:DATA(0x80, 0x01, 0x01, 0x81, 0x02, 0x02, 0x03)]);
	CHECK([parent.data isEqualToData:DATA(0x61, 0x07, 0x80, 0x01, 0x01, 0x81, 0x02, 0x02, 0x03)]);

	// Parsing the constructed record gives back its children.
	TKBERTLVRecord *parsed = [TKBERTLVRecord recordFromData:parent.data];
	NSArray *children = [TKBERTLVRecord sequenceOfRecordsFromData:parsed.value];
	CHECK(parsed.tag == 0x61 && children.count == 2);
	CHECK([children[0] tag] == 0x80 && [[children[1] value] isEqualToData:DATA(2, 3)]);
	CHECK([parsed isEqual:parent] && parsed.hash == parent.hash);
	CHECK(![parsed isEqual:a]);

	// High tag number form (X.690 8.1.2.4): 7F 21.
	CHECK([[TKBERTLVRecord dataForTag:0x7f21] isEqualToData:DATA(0x7f, 0x21)]);
	CHECK([[TKBERTLVRecord dataForTag:0x61] isEqualToData:DATA(0x61)]);
	TKBERTLVRecord *high = [TKBERTLVRecord recordFromData:DATA(0x7f, 0x21, 0x01, 0x05)];
	CHECK(high.tag == 0x7f21 && [high.value isEqualToData:DATA(5)]);

	// GUESS: tags that are not well-formed identifier octets (X.690 8.1.2) raise NSInvalidArgumentException instead of
	// encoding to bytes that read back as something else: 0x1f has no tag number octets, 0x0101 reads back as tag 01.
	BOOL raised = NO;
	@try { [[[TKBERTLVRecord alloc] initWithTag:0x1f value:DATA(1)] autorelease]; } @catch (NSException *e) { raised = [e.name isEqualToString:NSInvalidArgumentException]; }
	CHECK(raised);
	raised = NO;
	@try { [[[TKBERTLVRecord alloc] initWithTag:0x0101 value:DATA(1)] autorelease]; } @catch (NSException *e) { raised = [e.name isEqualToString:NSInvalidArgumentException]; }
	CHECK(raised);
	raised = NO;
	@try { [TKBERTLVRecord dataForTag:0x1f]; } @catch (NSException *e) { raised = [e.name isEqualToString:NSInvalidArgumentException]; }
	CHECK(raised);
	raised = NO;
	@try { [TKBERTLVRecord dataForTag:0x0101]; } @catch (NSException *e) { raised = [e.name isEqualToString:NSInvalidArgumentException]; }
	CHECK(raised);
	// Malformed identifier octets in input: 1F 80 01 and 1F 05.
	CHECK([TKBERTLVRecord recordFromData:DATA(0x1f, 0x80, 0x01, 0x00)] == nil);
	CHECK([TKBERTLVRecord recordFromData:DATA(0x1f, 0x05, 0x00)] == nil);
	// GUESS: identifier octet 00 is not a tag (X.690 8.1.5, ISO/IEC 7816-4 5.4.4).
	CHECK([TKBERTLVRecord recordFromData:DATA(0x00, 0x00)] == nil);
	raised = NO;
	@try { [TKBERTLVRecord dataForTag:0]; } @catch (NSException *e) { raised = [e.name isEqualToString:NSInvalidArgumentException]; }
	CHECK(raised);

	// BER input with a non-minimal length is accepted and keeps its original encoding.
	TKBERTLVRecord *loose = [TKBERTLVRecord recordFromData:DATA(0x04, 0x81, 0x01, 0x09)];
	CHECK(loose != nil && [loose.value isEqualToData:DATA(9)] && loose.data.length == 4);

	// Malformed input: trailing octets, truncated value, indefinite length, empty data.
	CHECK([TKBERTLVRecord recordFromData:DATA(0x04, 0x01, 0x09, 0x00)] == nil);
	CHECK([TKBERTLVRecord recordFromData:DATA(0x04, 0x02, 0x09)] == nil);
	CHECK([TKBERTLVRecord recordFromData:DATA(0x30, 0x80, 0x00, 0x00)] == nil);
	CHECK([TKBERTLVRecord recordFromData:[NSData data]] == nil);

	NSArray *two = [TKBERTLVRecord sequenceOfRecordsFromData:DATA(0x04, 0x01, 0x07, 0x05, 0x00)];
	CHECK(two.count == 2 && [two[0] tag] == 0x04 && [two[1] tag] == 0x05 && [[two[1] value] length] == 0);
	CHECK([TKBERTLVRecord sequenceOfRecordsFromData:DATA(0x04, 0x01, 0x07, 0x05)] == nil);
	// Data holding zero records is a valid (empty) sequence.
	CHECK([[TKBERTLVRecord sequenceOfRecordsFromData:[NSData data]] count] == 0);
	// GUESS: the base class has no encoding of its own.
	CHECK([TKTLVRecord recordFromData:DATA(0x04, 0x00)] == nil);
}

static void test_simple(void)
{
	TKSimpleTLVRecord *r = [[[TKSimpleTLVRecord alloc] initWithTag:5 value:DATA(0x11, 0x22)] autorelease];
	CHECK(r.tag == 5 && [r.data isEqualToData:DATA(0x05, 0x02, 0x11, 0x22)]);

	// 255 value octets need the FF escape: FF 00 FF (7816-4 5.4.4).
	NSData *v255 = [NSMutableData dataWithLength:255];
	TKSimpleTLVRecord *escaped = [[[TKSimpleTLVRecord alloc] initWithTag:2 value:v255] autorelease];
	CHECK([[escaped.data subdataWithRange:NSMakeRange(0, 4)] isEqualToData:DATA(0x02, 0xff, 0x00, 0xff)]);
	TKSimpleTLVRecord *back = [TKSimpleTLVRecord recordFromData:escaped.data];
	CHECK(back.tag == 2 && back.value.length == 255);
	NSData *v254 = [NSMutableData dataWithLength:254];
	CHECK([[[[[TKSimpleTLVRecord alloc] initWithTag:2 value:v254] autorelease] data] length] == 2 + 254);

	CHECK([TKSimpleTLVRecord recordFromData:DATA(0x00, 0x00)] == nil);
	CHECK([TKSimpleTLVRecord recordFromData:DATA(0xff, 0x00)] == nil);
	CHECK([TKSimpleTLVRecord recordFromData:DATA(0x01, 0x02, 0x00)] == nil);
	NSArray *seq = [TKSimpleTLVRecord sequenceOfRecordsFromData:DATA(0x01, 0x00, 0x02, 0x01, 0x09)];
	CHECK(seq.count == 2 && [[seq[1] value] isEqualToData:DATA(9)]);

	// GUESS: values the encoding cannot hold raise NSInvalidArgumentException.
	BOOL raised = NO;
	@try { [[[TKSimpleTLVRecord alloc] initWithTag:0 value:DATA(1)] autorelease]; } @catch (NSException *e) { raised = [e.name isEqualToString:NSInvalidArgumentException]; }
	CHECK(raised);
	raised = NO;
	@try { [[[TKSimpleTLVRecord alloc] initWithTag:0xff value:DATA(1)] autorelease]; } @catch (NSException *e) { raised = YES; }
	CHECK(raised);
	raised = NO;
	@try { [[[TKSimpleTLVRecord alloc] initWithTag:1 value:[NSMutableData dataWithLength:65536]] autorelease]; } @catch (NSException *e) { raised = YES; }
	CHECK(raised);
}

static void test_compact(void)
{
	// 7816-4 8.3: octet XY, X the tag number, Y the length.
	TKCompactTLVRecord *r = [[[TKCompactTLVRecord alloc] initWithTag:3 value:DATA(0xc0)] autorelease];
	CHECK([r.data isEqualToData:DATA(0x31, 0xc0)] && [r.value isEqualToData:DATA(0xc0)]);
	// GUESS: the tag property is the 4-bit tag number, not the first octet.
	CHECK(r.tag == 3);

	TKCompactTLVRecord *parsed = [TKCompactTLVRecord recordFromData:DATA(0x31, 0xc0)];
	CHECK(parsed.tag == 3 && [parsed isEqual:r]);
	TKCompactTLVRecord *empty = [TKCompactTLVRecord recordFromData:DATA(0x80)];
	CHECK(empty.tag == 8 && empty.value.length == 0);
	CHECK([TKCompactTLVRecord recordFromData:DATA(0x32, 0xc0)] == nil);
	NSArray *seq = [TKCompactTLVRecord sequenceOfRecordsFromData:DATA(0x31, 0xc0, 0x80, 0x11, 0x01)];
	CHECK(seq.count == 3 && [seq[2] tag] == 1);
	CHECK(![[TKCompactTLVRecord recordFromData:DATA(0x80)] isEqual:[TKSimpleTLVRecord recordFromData:DATA(0x01, 0x00)]]);

	// GUESS: a tag above 15 or a value above 15 octets raises NSInvalidArgumentException.
	BOOL raised = NO;
	@try { [[[TKCompactTLVRecord alloc] initWithTag:16 value:DATA(1)] autorelease]; } @catch (NSException *e) { raised = [e.name isEqualToString:NSInvalidArgumentException]; }
	CHECK(raised);
	raised = NO;
	@try { [[[TKCompactTLVRecord alloc] initWithTag:1 value:[NSMutableData dataWithLength:16]] autorelease]; } @catch (NSException *e) { raised = YES; }
	CHECK(raised);
	CHECK([[[[TKCompactTLVRecord alloc] initWithTag:15 value:[NSMutableData dataWithLength:15]] autorelease] data].length == 16);
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
