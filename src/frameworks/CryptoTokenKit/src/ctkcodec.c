#include "ctkcodec.h"

bool ctk_ber_tag_valid(uint64_t tag)
{
	// Identifier octet 00 is not a tag: X.690 8.1.5 uses 00 00 as the end-of-contents marker, and ISO/IEC 7816-4
	// (5.4.4, 2005 edition) keeps 00 and FF out of the tag values and allows them as padding between objects.
	if (tag == 0)
		return false;
	uint8_t octets[CTK_BER_MAX_TAG_OCTETS];
	size_t count = ctk_ber_encode_tag(tag, octets);
	if (count == 1)
		return (octets[0] & 0x1f) != 0x1f;
	if ((octets[0] & 0x1f) != 0x1f)
		return false;
	for (size_t i = 1; i < count; i++) {
		bool last = i == count - 1;
		if (((octets[i] & 0x80) != 0) == last)
			return false;
	}
	// 8.1.2.4.2 (c): bits 7 to 1 of the first subsequent octet are not all zero. A tag number of 30 or less
	// has to use the single-octet form (8.1.2.2).
	if ((octets[1] & 0x7f) == 0)
		return false;
	return count > 2 || octets[1] > 0x1e;
}

bool ctk_ber_decode(const uint8_t *bytes, size_t length, ctk_tlv *out)
{
	size_t pos = 0;
	if (length == 0)
		return false;

	uint64_t tag = bytes[pos++];
	if ((tag & 0x1f) == 0x1f) {
		// High tag number form: further octets follow while bit 8 is set.
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
	if (first < 0x80) {
		value_length = first;
	} else {
		// 0x80 is the indefinite form and 0xFF is reserved; neither is a definite length.
		size_t count = first & 0x7f;
		if (count == 0 || count > sizeof(size_t) || length - pos < count)
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
	return true;
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

	out->tag = bytes[0] >> 4;
	out->value_offset = 1;
	out->value_length = value_length;
	out->total_length = 1 + value_length;
	return true;
}
