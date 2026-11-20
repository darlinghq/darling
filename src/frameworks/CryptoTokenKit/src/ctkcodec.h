#ifndef CTKCODEC_H
#define CTKCODEC_H

// Byte-level codecs for the TLV encodings of ITU-T X.690 and ISO/IEC 7816-4, kept free of Objective-C so that
// they can be tested on any host. TKTLVRecord.m wraps them.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Location of one decoded record inside the buffer given to the decoder.
typedef struct {
	uint64_t tag;
	size_t value_offset;
	size_t value_length;
	size_t total_length;
} ctk_tlv;

// BER-TLV, ITU-T X.690 clauses 8.1.2 (identifier octets) and 8.1.3 (length octets).
// The tag is the identifier octets read as one big-endian integer, so at most 8 octets, and must be well
// formed (ctk_ber_tag_valid). Only definite lengths are accepted (the indefinite form 0x80 is rejected). Decoding stops after the first record; total_length says how much it used.
#define CTK_BER_MAX_TAG_OCTETS 8
#define CTK_BER_MAX_LENGTH_OCTETS (1 + sizeof(size_t))
bool ctk_ber_decode(const uint8_t *bytes, size_t length, ctk_tlv *out);
// True when the integer is the identifier octets of a well-formed tag (8.1.2): a single octet other than 00 whose
// low five bits are not all set, or the high tag number form (first octet low bits all set, continuation bit set on every octet
// after the first but the last, first subsequent octet not 0x80, tag number above 30).
bool ctk_ber_tag_valid(uint64_t tag);
// Writes the identifier octets of a tag (the minimal big-endian octets of the integer) and returns how many.
size_t ctk_ber_encode_tag(uint64_t tag, uint8_t out[CTK_BER_MAX_TAG_OCTETS]);
// Writes the shortest definite length encoding (the DER rule) and returns how many octets.
size_t ctk_ber_encode_length(size_t value_length, uint8_t out[CTK_BER_MAX_LENGTH_OCTETS]);

// Simple-TLV, ISO/IEC 7816-4 (5.4.4 in the 2005 edition): one tag octet other than 00 and FF, then either
// one length octet 00..FE or FF followed by two length octets.
#define CTK_SIMPLE_MAX_LENGTH 0xffffu
bool ctk_simple_tag_valid(uint8_t tag);
bool ctk_simple_decode(const uint8_t *bytes, size_t length, ctk_tlv *out);
size_t ctk_simple_encode_length(size_t value_length, uint8_t out[3]);

// Compact-TLV, ISO/IEC 7816-4 (8.3 in the 2005 edition): one octet whose high nibble is the tag number and
// whose low nibble is the value length, followed by that many octets.
#define CTK_COMPACT_MAX_TAG 0x0fu
#define CTK_COMPACT_MAX_LENGTH 0x0fu
bool ctk_compact_decode(const uint8_t *bytes, size_t length, ctk_tlv *out);

#endif
