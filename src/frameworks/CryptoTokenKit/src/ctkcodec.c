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

#include "ctkcodec.h"

bool ctk_ber_tag_valid(uint64_t tag)
{
	// Identifier octet 00 is not a tag: X.690 8.1.5 uses 00 00 as the end-of-contents marker. Any other integer
	// is accepted and encodes to its minimal big-endian octets, as black-box observation of TKBERTLVRecord on
	// macOS 26.7.3 shows (tier 2): tags such as 0x1f or 0x0101, which are not well-formed identifier octets
	// under X.690 8.1.2, encode as they are, and only tag 0 is refused. Test: cryptotokenkit_codec.c, "tag validity".
	return tag != 0;
}

static bool ber_decode(const uint8_t *bytes, size_t length, unsigned depth, ctk_tlv *out)
{
	size_t pos = 0;
	if (length == 0)
		return false;

	uint64_t tag = bytes[pos++];
	if ((tag & 0x1f) == 0x1f) {
		// High tag number form: further octets follow while bit 8 is set. The octets are not required to be
		// minimal (1F 80 01, or a tag number below 31, are read as they stand), as observed on macOS 26.7.3.
		size_t tag_octets = 1;
		for (;;) {
			if (pos >= length || tag_octets == CTK_BER_MAX_TAG_OCTETS)
				return false;
			uint8_t octet = bytes[pos++];
			tag = (tag << 8) | octet;
			tag_octets++;
			if (!(octet & 0x80))
				break;
		}
	}

	if (!ctk_ber_tag_valid(tag) || pos >= length)
		return false;
	uint8_t first = bytes[pos++];
	size_t value_length;
	bool indefinite = false;
	if (first == 0x80) {
		// X.690 8.1.3.6: the indefinite form. The content is a run of records that ends with 00 00 (8.1.5).
		// QUIRK: nesting of indefinite records is accepted to a depth of 33 and not beyond. Not documented.
		// Evidence: black-box observation of TKBERTLVRecord on macOS 26.7.3 (tier 2): 33 nested records decode,
		// 34 do not. Test: cryptotokenkit_codec.c, "indefinite nesting depth".
		if (depth >= CTK_BER_MAX_INDEFINITE_DEPTH)
			return false;
		size_t content = pos;
		for (;;) {
			if (length - pos >= 2 && bytes[pos] == 0 && bytes[pos + 1] == 0)
				break;
			ctk_tlv child;
			if (!ber_decode(bytes + pos, length - pos, depth + 1, &child))
				return false;
			pos += child.total_length;
		}
		out->tag = tag;
		out->value_offset = content;
		out->value_length = pos - content;
		out->total_length = pos + 2;
		out->indefinite = true;
		return true;
	} else if (first < 0x80) {
		value_length = first;
	} else {
		// 0xFF is reserved; the other long forms give the length in the next 1 to 126 octets.
		size_t count = first & 0x7f;
		if (first == 0xff || count > sizeof(size_t) || length - pos < count)
			return false;
		value_length = 0;
		for (size_t i = 0; i < count; i++)
			value_length = (value_length << 8) | bytes[pos++];
	}
	if (value_length > length - pos)
		return false;

	out->tag = tag;
	out->value_offset = pos;
	out->value_length = value_length;
	out->total_length = pos + value_length;
	out->indefinite = indefinite;
	return true;
}

bool ctk_ber_decode(const uint8_t *bytes, size_t length, ctk_tlv *out)
{
	return ber_decode(bytes, length, 0, out);
}

size_t ctk_ber_encode_tag(uint64_t tag, uint8_t out[CTK_BER_MAX_TAG_OCTETS])
{
	size_t count = 1;
	for (uint64_t rest = tag >> 8; rest != 0; rest >>= 8)
		count++;
	for (size_t i = 0; i < count; i++)
		out[count - 1 - i] = (uint8_t)(tag >> (8 * i));
	return count;
}

size_t ctk_ber_encode_length(size_t value_length, uint8_t out[CTK_BER_MAX_LENGTH_OCTETS])
{
	if (value_length < 0x80) {
		out[0] = (uint8_t)value_length;
		return 1;
	}
	size_t count = 0;
	for (size_t rest = value_length; rest != 0; rest >>= 8)
		count++;
	out[0] = (uint8_t)(0x80 | count);
	for (size_t i = 0; i < count; i++)
		out[count - i] = (uint8_t)(value_length >> (8 * i));
	return 1 + count;
}

bool ctk_simple_tag_valid(uint8_t tag)
{
	return tag != 0x00 && tag != 0xff;
}

bool ctk_simple_decode(const uint8_t *bytes, size_t length, ctk_tlv *out)
{
	if (length < 2 || !ctk_simple_tag_valid(bytes[0]))
		return false;
	size_t pos = 1;
	size_t value_length = bytes[pos++];
	if (value_length == 0xff) {
		if (length < 4)
			return false;
		value_length = ((size_t)bytes[2] << 8) | bytes[3];
		pos = 4;
	}
	if (value_length > length - pos)
		return false;

	out->indefinite = false;
	out->tag = bytes[0];
	out->value_offset = pos;
	out->value_length = value_length;
	out->total_length = pos + value_length;
	return true;
}

size_t ctk_simple_encode_length(size_t value_length, uint8_t out[3])
{
	if (value_length < 0xff) {
		out[0] = (uint8_t)value_length;
		return 1;
	}
	out[0] = 0xff;
	out[1] = (uint8_t)(value_length >> 8);
	out[2] = (uint8_t)value_length;
	return 3;
}

bool ctk_compact_decode(const uint8_t *bytes, size_t length, ctk_tlv *out)
{
	if (length < 1)
		return false;
	size_t value_length = bytes[0] & 0x0f;
	if (value_length > length - 1)
		return false;

	out->indefinite = false;
	out->tag = bytes[0] >> 4;
	out->value_offset = 1;
	out->value_length = value_length;
	out->total_length = 1 + value_length;
	return true;
}
