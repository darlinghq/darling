// CryptoTokenKit byte codecs (BER-TLV, Simple-TLV, Compact-TLV). The codecs are plain C, so
// this builds with any C compiler on any host: cc cryptotokenkit_codec.c && ./a.out
// Vectors are hand-built from the layouts of ITU-T X.690 8.1.2 / 8.1.3 / 10.1 and ISO/IEC 7816-4 5.4.4 and 8.3
// (2005 edition numbering); each one is annotated with how it was derived.
// Prints nothing on success; the exit code is the number of failed checks.
#include "../../src/frameworks/CryptoTokenKit/src/ctkcodec.c"

#include <stdio.h>

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
	// X.690 8.1.2.4.2 (c): bits 7 to 1 of the first subsequent octet may not all be zero (1F 80 01).
	const uint8_t padded[] = { 0x1f, 0x80, 0x01, 0x00 };
	CHECK(!ber(padded, sizeof padded, &t));
	// X.690 8.1.2.2: tag numbers up to 30 use the single octet form, so 1F 05 is malformed; 1F 1F is the first high one.
	const uint8_t needless[] = { 0x1f, 0x05, 0x00 };
	CHECK(!ber(needless, sizeof needless, &t));
	const uint8_t first_high[] = { 0x1f, 0x1f, 0x00 };
	CHECK(ber(first_high, sizeof first_high, &t) && t.tag == 0x1f1f);
	const uint8_t last_low[] = { 0x1f, 0x1e, 0x00 };
	CHECK(!ber(last_low, sizeof last_low, &t));
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
	// X.690 8.1.3.6 indefinite form (80) and the reserved octet FF are rejected.
	const uint8_t indefinite[] = { 0x30, 0x80, 0x00, 0x00 };
	CHECK(!ber(indefinite, sizeof indefinite, &t));
	const uint8_t reserved[] = { 0x04, 0xff, 0x00 };
	CHECK(!ber(reserved, sizeof reserved, &t));
	// Length longer than the data, and no data at all.
	const uint8_t overrun[] = { 0x04, 0x05, 0x01 };
	CHECK(!ber(overrun, sizeof overrun, &t));
	CHECK(!ber(basic, 0, &t));
	CHECK(!ber(basic, 1, &t));

	// Tag validity on the integer form: 61 and 7F21 are tags, 1F (escape with nothing after it), 0101 (two octets
	// whose first is not an escape, which would read back as tag 01 with length 01) and 1F8001 are not.
	CHECK(ctk_ber_tag_valid(0x61) && ctk_ber_tag_valid(0x7f21) && ctk_ber_tag_valid(0x1f8101));
	// Identifier octet 00 is not a tag (X.690 8.1.5 end-of-contents; ISO/IEC 7816-4 5.4.4 padding).
	CHECK(!ctk_ber_tag_valid(0));
	CHECK(!ctk_ber_tag_valid(0x1f) && !ctk_ber_tag_valid(0x0101) && !ctk_ber_tag_valid(0x1f8001) && !ctk_ber_tag_valid(0x1f05));
	CHECK(!ctk_ber_tag_valid(0x1f81) && !ctk_ber_tag_valid(0x1f8181));

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

int main(void)
{
	test_ber();
	test_simple();
	test_compact();
	return failures;
}
