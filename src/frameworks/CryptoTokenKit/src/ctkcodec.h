/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

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
	bool indefinite; // BER only: the length was the indefinite form, and total_length includes the 00 00 that ends it
} ctk_tlv;

// BER-TLV, ITU-T X.690 clauses 8.1.2 (identifier octets) and 8.1.3 (length octets).
// The tag is the identifier octets read as one big-endian integer, so at most 8 octets (ctk_ber_tag_valid
// refuses only 0). Both the definite lengths and the indefinite form (0x80, content ending with 00 00) are
// decoded; the content of an indefinite record has to be a run of well-formed records. Decoding stops after the
// first record; total_length says how much it used.
#define CTK_BER_MAX_TAG_OCTETS 8
#define CTK_BER_MAX_INDEFINITE_DEPTH 33
#define CTK_BER_MAX_LENGTH_OCTETS (1 + sizeof(size_t))
bool ctk_ber_decode(const uint8_t *bytes, size_t length, ctk_tlv *out);
// True for every integer but 0, which is not a tag (it is the end-of-contents octet).
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
