// CryptoTokenKit byte codecs (BER-TLV, Simple-TLV, Compact-TLV). The codecs are plain C, so
// this builds with any C compiler on any host: cc cryptotokenkit_codec.c && ./a.out
// Vectors are hand-built from the layouts of ITU-T X.690 8.1.2 / 8.1.3 / 10.1 and ISO/IEC 7816-4 5.4.4 and 8.3
// (2005 edition numbering); each one is annotated with how it was derived.
// Prints nothing on success; the exit code is the number of failed checks.
#include "../../src/frameworks/CryptoTokenKit/src/ctkcodec.c"

#include <stdio.h>
#include <stdlib.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)

static bool ber(const uint8_t *b, size_t n, ctk_tlv *t) { return ctk_ber_decode(b, n, t); }

static void test_ber(void)
{
	ctk_tlv t;
	const uint8_t zero_tag[] = { 0x00, 0x00 };
	CHECK(!ber(zero_tag, sizeof zero_tag, &t));
	// X.690 8.1.2.2: tag 61, short length 3.
	const uint8_t basic[] = { 0x61, 0x03, 0xaa, 0xbb, 0xcc };
	CHECK(ber(basic, sizeof basic, &t) && t.tag == 0x61 && t.value_offset == 2 && t.value_length == 3 && t.total_length == 5);

	// A trailing record is not consumed: total_length says where the first one ends.
	const uint8_t two[] = { 0x04, 0x01, 0x07, 0x04, 0x00 };
	CHECK(ber(two, sizeof two, &t) && t.total_length == 3);

	// X.690 8.1.2.4: high tag number form, 7F 21 read as one integer.
	const uint8_t high[] = { 0x7f, 0x21, 0x00 };
	CHECK(ber(high, sizeof high, &t) && t.tag == 0x7f21 && t.value_length == 0 && t.total_length == 3);
	// Three tag octets: 1F 81 01 (continuation bit on the middle one).
	const uint8_t high3[] = { 0x1f, 0x81, 0x01, 0x00 };
	CHECK(ber(high3, sizeof high3, &t) && t.tag == 0x1f8101);
	// Nine tag octets do not fit the 64-bit tag.
	const uint8_t huge[] = { 0x1f, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x01, 0x00 };
	CHECK(!ber(huge, sizeof huge, &t));
	// High tag number octets are read as they stand, minimal or not (observed on macOS 26.7.3): 1F 80 01, 1F 05, 1F 1E.
	const uint8_t padded[] = { 0x1f, 0x80, 0x01, 0x00 };
	CHECK(ber(padded, sizeof padded, &t) && t.tag == 0x1f8001);
	const uint8_t needless[] = { 0x1f, 0x05, 0x00 };
	CHECK(ber(needless, sizeof needless, &t) && t.tag == 0x1f05);
	const uint8_t last_low[] = { 0x1f, 0x1e, 0x00 };
	CHECK(ber(last_low, sizeof last_low, &t) && t.tag == 0x1f1e);
	// FF has its low five bits set, so it starts the high tag number form too: FF FF 04 is one tag.
	const uint8_t ff_tag[] = { 0xff, 0xff, 0x04, 0x00 };
	CHECK(ber(ff_tag, sizeof ff_tag, &t) && t.tag == 0xffff04);
	// Tag cut off after the escape octet.
	const uint8_t cut_tag[] = { 0x1f };
	CHECK(!ber(cut_tag, sizeof cut_tag, &t));

	// X.690 8.1.3.5: long form, 81 80 means 128 value octets.
	uint8_t longform[3 + 128] = { 0x04, 0x81, 0x80 };
	CHECK(ber(longform, sizeof longform, &t) && t.value_length == 128 && t.value_offset == 3);
	// Two length octets: 82 01 00 means 256, but only 128 octets follow.
	uint8_t shortbuf[4 + 128] = { 0x04, 0x82, 0x01, 0x00 };
	CHECK(!ber(shortbuf, sizeof shortbuf, &t));
	// Non-minimal long form is still decodable (BER, not DER): 81 05.
	const uint8_t nonminimal[] = { 0x04, 0x81, 0x01, 0x09 };
	CHECK(ber(nonminimal, sizeof nonminimal, &t) && t.value_length == 1);
	// X.690 8.1.3.6: the indefinite form. The content is a run of records ending with 00 00.
	const uint8_t indefinite[] = { 0x30, 0x80, 0x04, 0x01, 0x09, 0x00, 0x00 };
	CHECK(ber(indefinite, sizeof indefinite, &t) && t.indefinite && t.tag == 0x30 && t.value_offset == 2 && t.value_length == 3 && t.total_length == 7);
	const uint8_t indefinite_empty[] = { 0x30, 0x80, 0x00, 0x00 };
	CHECK(ber(indefinite_empty, sizeof indefinite_empty, &t) && t.indefinite && t.value_length == 0 && t.total_length == 4);
	const uint8_t nested[] = { 0x30, 0x80, 0x30, 0x80, 0x00, 0x00, 0x00, 0x00 };
	CHECK(ber(nested, sizeof nested, &t) && t.value_length == 4 && t.total_length == 8);
	// The content must be well-formed records and the 00 00 must be there; the record after the end marker is not consumed.
	const uint8_t no_marker[] = { 0x30, 0x80, 0x04, 0x01, 0x09 };
	const uint8_t half_marker[] = { 0x30, 0x80, 0x00 };
	const uint8_t bad_content[] = { 0x30, 0x80, 0xff };
	const uint8_t child_overrun[] = { 0x30, 0x80, 0x04, 0x05, 0x09, 0x00, 0x00 };
	CHECK(!ber(no_marker, sizeof no_marker, &t) && !ber(half_marker, sizeof half_marker, &t));
	CHECK(!ber(bad_content, sizeof bad_content, &t) && !ber(child_overrun, sizeof child_overrun, &t));
	const uint8_t indefinite_then_more[] = { 0x30, 0x80, 0x00, 0x00, 0x05, 0x00 };
	CHECK(ber(indefinite_then_more, sizeof indefinite_then_more, &t) && t.total_length == 4);
	// A definite record does not set the flag, and one inside a definite record is not looked into.
	CHECK(ber(basic, sizeof basic, &t) && !t.indefinite);
	const uint8_t definite_over_indefinite[] = { 0x30, 0x05, 0x30, 0x80, 0x00, 0x00, 0x00 };
	CHECK(ber(definite_over_indefinite, sizeof definite_over_indefinite, &t) && !t.indefinite && t.value_length == 5);
	// Nesting of indefinite records stops at depth 33 (observed): 33 deep decodes, 34 does not.
	for (size_t depth = 1; depth <= 40; depth++) {
		uint8_t deep[2 * 2 * 40];
		for (size_t i = 0; i < depth; i++) {
			deep[2 * i] = 0x30;
			deep[2 * i + 1] = 0x80;
			deep[2 * depth + 2 * i] = 0x00;
			deep[2 * depth + 2 * i + 1] = 0x00;
		}
		CHECK(ber(deep, 4 * depth, &t) == (depth <= CTK_BER_MAX_INDEFINITE_DEPTH));
	}
	// The reserved length octet FF is not a length.
	const uint8_t reserved[] = { 0x04, 0xff, 0x00 };
	CHECK(!ber(reserved, sizeof reserved, &t));
	// A length of more octets than there are is cut off, and so is a 9-octet length (wider than size_t on 64-bit
	// hosts, and than the data on every host).
	const uint8_t wide[] = { 0x04, 0x89, 0, 0, 0, 0, 0, 0, 0, 0, 1, 9 };
	CHECK(!ber(wide, sizeof wide, &t));
	const uint8_t huge_length[] = { 0x04, 0x84, 0xff, 0xff, 0xff, 0xff, 0x01 };
	CHECK(!ber(huge_length, sizeof huge_length, &t));
	// Length longer than the data, and no data at all.
	const uint8_t overrun[] = { 0x04, 0x05, 0x01 };
	CHECK(!ber(overrun, sizeof overrun, &t));
	CHECK(!ber(basic, 0, &t));
	CHECK(!ber(basic, 1, &t));

	// "tag validity": every integer but 0 is a tag (observed: TKBERTLVRecord encodes 0x1f and 0x0101 as they are).
	// Identifier octet 00 is not a tag (X.690 8.1.5 end-of-contents).
	CHECK(ctk_ber_tag_valid(0x61) && ctk_ber_tag_valid(0x7f21) && ctk_ber_tag_valid(0x1f8101));
	CHECK(!ctk_ber_tag_valid(0));
	CHECK(ctk_ber_tag_valid(0x1f) && ctk_ber_tag_valid(0x0101) && ctk_ber_tag_valid(0x1f8001) && ctk_ber_tag_valid(0xffffffffffffffffull));

	uint8_t tag[CTK_BER_MAX_TAG_OCTETS];
	CHECK(ctk_ber_encode_tag(0x61, tag) == 1 && tag[0] == 0x61);
	CHECK(ctk_ber_encode_tag(0x7f21, tag) == 2 && tag[0] == 0x7f && tag[1] == 0x21);
	CHECK(ctk_ber_encode_tag(0, tag) == 1 && tag[0] == 0);
	CHECK(ctk_ber_encode_tag(0x0102030405060708ull, tag) == 8 && tag[0] == 1 && tag[7] == 8);

	// DER (X.690 10.1): shortest definite length.
	uint8_t len[CTK_BER_MAX_LENGTH_OCTETS];
	CHECK(ctk_ber_encode_length(0, len) == 1 && len[0] == 0);
	CHECK(ctk_ber_encode_length(127, len) == 1 && len[0] == 0x7f);
	CHECK(ctk_ber_encode_length(128, len) == 2 && len[0] == 0x81 && len[1] == 0x80);
	CHECK(ctk_ber_encode_length(255, len) == 2 && len[0] == 0x81 && len[1] == 0xff);
	CHECK(ctk_ber_encode_length(256, len) == 3 && len[0] == 0x82 && len[1] == 0x01 && len[2] == 0x00);
	CHECK(ctk_ber_encode_length(65536, len) == 4 && len[0] == 0x83 && len[1] == 1 && len[2] == 0 && len[3] == 0);
}

static void test_simple(void)
{
	ctk_tlv t;
	const uint8_t basic[] = { 0x05, 0x02, 0x11, 0x22 };
	CHECK(ctk_simple_decode(basic, sizeof basic, &t) && t.tag == 5 && t.value_offset == 2 && t.value_length == 2);
	const uint8_t empty[] = { 0x01, 0x00 };
	CHECK(ctk_simple_decode(empty, sizeof empty, &t) && t.value_length == 0 && t.total_length == 2);
	// Tags 00 and FF are not tags (7816-4 5.4.4).
	const uint8_t zero_tag[] = { 0x00, 0x00 };
	const uint8_t ff_tag[] = { 0xff, 0x00 };
	CHECK(!ctk_simple_decode(zero_tag, sizeof zero_tag, &t));
	CHECK(!ctk_simple_decode(ff_tag, sizeof ff_tag, &t));
	CHECK(ctk_simple_tag_valid(0x01) && ctk_simple_tag_valid(0xfe) && !ctk_simple_tag_valid(0) && !ctk_simple_tag_valid(0xff));
	// FF escapes to a two-octet length: 00 FF 01 00 is 256 octets.
	static uint8_t big[4 + 256] = { 0x02, 0xff, 0x01, 0x00 };
	CHECK(ctk_simple_decode(big, sizeof big, &t) && t.value_length == 256 && t.value_offset == 4);
	CHECK(!ctk_simple_decode(big, sizeof big - 1, &t));
	const uint8_t cut_escape[] = { 0x02, 0xff, 0x01 };
	CHECK(!ctk_simple_decode(cut_escape, sizeof cut_escape, &t));
	// A one-octet length FE is the longest short form.
	CHECK(ctk_simple_decode((uint8_t[2 + 254]){ 0x02, 0xfe }, 2 + 254, &t) && t.value_length == 254);
	const uint8_t overrun[] = { 0x02, 0x03, 0x01 };
	CHECK(!ctk_simple_decode(overrun, sizeof overrun, &t));
	CHECK(!ctk_simple_decode(basic, 1, &t));

	uint8_t len[3];
	CHECK(ctk_simple_encode_length(0, len) == 1 && len[0] == 0);
	CHECK(ctk_simple_encode_length(254, len) == 1 && len[0] == 0xfe);
	CHECK(ctk_simple_encode_length(255, len) == 3 && len[0] == 0xff && len[1] == 0x00 && len[2] == 0xff);
	CHECK(ctk_simple_encode_length(0xffff, len) == 3 && len[1] == 0xff && len[2] == 0xff);
}

static void test_compact(void)
{
	ctk_tlv t;
	// 7816-4 8.3: byte XY is tag number X, length Y. 31 followed by one octet.
	const uint8_t basic[] = { 0x31, 0xc0, 0x77 };
	CHECK(ctk_compact_decode(basic, sizeof basic, &t) && t.tag == 3 && t.value_offset == 1 && t.value_length == 1 && t.total_length == 2);
	const uint8_t empty[] = { 0x80 };
	CHECK(ctk_compact_decode(empty, sizeof empty, &t) && t.tag == 8 && t.value_length == 0);
	const uint8_t maximal[] = { 0xff, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
	CHECK(ctk_compact_decode(maximal, sizeof maximal, &t) && t.tag == 15 && t.value_length == 15);
	CHECK(!ctk_compact_decode(maximal, sizeof maximal - 1, &t));
	CHECK(!ctk_compact_decode(basic, 0, &t));
}

// Malformed input: random buffers of every length up to 40, each in a heap block of exactly that size, so that a read
// past the end is an address sanitizer error, with the bytes biased towards the values that steer the decoders
// (0x00, 0x1F, 0x80, 0x81, 0xFF). Whatever the decoders accept has to describe a record that lies inside the buffer.
static uint32_t next_random(uint32_t *state)
{
	*state ^= *state << 13;
	*state ^= *state >> 17;
	*state ^= *state << 5;
	return *state;
}

static void test_random_input(void)
{
	static const uint8_t interesting[] = { 0x00, 0x1f, 0x80, 0x81, 0xff, 0x30, 0x04, 0x7f };
	uint32_t state = 0x2545f491u;
	for (int round = 0; round < 20000; round++) {
		size_t length = next_random(&state) % 41;
		uint8_t *bytes = malloc(length ? length : 1);
		for (size_t i = 0; i < length; i++) {
			uint32_t r = next_random(&state);
			bytes[i] = (r & 1) ? interesting[(r >> 1) % sizeof interesting] : (uint8_t)(r >> 8);
		}
		ctk_tlv t;
		bool (*decoders[3])(const uint8_t *, size_t, ctk_tlv *) = { ctk_ber_decode, ctk_simple_decode, ctk_compact_decode };
		for (int d = 0; d < 3; d++) {
			if (decoders[d](bytes, length, &t)) {
				CHECK(t.value_offset <= t.total_length && t.total_length <= length);
				CHECK(t.value_length <= t.total_length - t.value_offset);
			}
		}
		free(bytes);
	}
	// Deeply nested and endless indefinite content is refused without recursing past the depth bound.
	uint8_t nested[4000];
	for (size_t i = 0; i < sizeof nested; i += 2) {
		nested[i] = 0x30;
		nested[i + 1] = 0x80;
	}
	ctk_tlv t;
	CHECK(!ctk_ber_decode(nested, sizeof nested, &t));
}

int main(void)
{
	test_random_input();
	test_ber();
	test_simple();
	test_compact();
	return failures;
}
