#include "ctkatr.h"

static bool next_octet(ctk_byte_source source, void *context, ctk_atr *atr, uint8_t *octet)
{
	int value = source(context);
	if (value < 0 || value > 0xff || atr->length == CTK_ATR_MAX_LENGTH)
		return false;
	*octet = (uint8_t)value;
	atr->bytes[atr->length++] = *octet;
	return true;
}

bool ctk_atr_parse(ctk_byte_source source, void *context, ctk_atr *out)
{
	*out = (ctk_atr){ .length = 0 };

	uint8_t ts, t0;
	if (!next_octet(source, context, out, &ts) || (ts != 0x3b && ts != 0x3f))
		return false;
	if (!next_octet(source, context, out, &t0))
		return false;

	size_t historical_length = t0 & 0x0f;
	unsigned present = t0 >> 4; // which of TAi, TBi, TCi, TDi follow
	bool tck_required = false;

	while (present != 0) {
		// A group always costs at least one octet and the ATR is capped, so this cannot overflow groups[].
		ctk_atr_group *group = &out->groups[out->group_count++];
		uint8_t octet;
		if ((present & 0x1) && !(group->has_ta = next_octet(source, context, out, &group->ta)))
			return false;
		if ((present & 0x2) && !(group->has_tb = next_octet(source, context, out, &group->tb)))
			return false;
		if ((present & 0x4) && !(group->has_tc = next_octet(source, context, out, &group->tc)))
			return false;
		if (present & 0x8) {
			if (!next_octet(source, context, out, &octet))
				return false;
			group->has_td = true;
			group->protocol = octet & 0x0f;
			if (group->protocol != 0)
				tck_required = true;
			present = octet >> 4;
		} else {
			present = 0;
		}
	}

	out->historical_offset = out->length;
	out->historical_length = historical_length;
	for (size_t i = 0; i < historical_length; i++) {
		uint8_t octet;
		if (!next_octet(source, context, out, &octet))
			return false;
	}

	if (tck_required) {
		uint8_t tck;
		if (!next_octet(source, context, out, &tck))
			return false;
		uint8_t check = 0;
		for (size_t i = 1; i < out->length; i++)
			check ^= out->bytes[i];
		if (check != 0)
			return false;
	}
	return true;
}
