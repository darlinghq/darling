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

#include "ctkatr.h"

static bool next_octet(ctk_byte_source source, void *context, ctk_atr *atr, uint8_t *octet)
{
	// Checked before the source is asked, so an endless source is not read past the bound.
	if (atr->length == CTK_ATR_MAX_LENGTH)
		return false;
	int value = source(context);
	if (value < 0)
		return false;
	*octet = (uint8_t)value;
	atr->bytes[atr->length++] = *octet;
	return true;
}

bool ctk_atr_parse(ctk_byte_source source, void *context, ctk_atr *out)
{
	*out = (ctk_atr){ .length = 0 };

	uint8_t ts, t0;
	if (!next_octet(source, context, out, &ts) || !next_octet(source, context, out, &t0))
		return false;

	size_t historical_length = t0 & 0x0f;
	unsigned present = t0 >> 4; // which of TAi, TBi, TCi, TDi follow
	bool tck_required = false;

	for (;;) {
		// Every group but the first follows a TDi, which cost an octet, and the ATR is capped, so this cannot
		// overflow groups[].
		ctk_atr_group *group = &out->groups[out->group_count++];
		uint8_t octet;
		if ((present & 0x1) && !(group->has_ta = next_octet(source, context, out, &group->ta)))
			return false;
		if ((present & 0x2) && !(group->has_tb = next_octet(source, context, out, &group->tb)))
			return false;
		if ((present & 0x4) && !(group->has_tc = next_octet(source, context, out, &group->tc)))
			return false;
		if (!(present & 0x8))
			break;
		if (!next_octet(source, context, out, &octet))
			return false;
		group->has_td = true;
		group->protocol = octet & 0x0f;
		if (group->protocol != 0)
			tck_required = true;
		// A group follows every TDi, even when the TDi announces nothing for it.
		present = octet >> 4;
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
	}
	return true;
}

size_t ctk_atr_protocols(const ctk_atr *atr, uint8_t out[16])
{
	if (atr->group_count > 1 && atr->groups[1].has_ta) {
		out[0] = atr->groups[1].ta & 0x0f;
		return 1;
	}

	size_t count = 0;
	bool seen[16] = { false };
	for (size_t i = 0; i < atr->group_count; i++) {
		if (!atr->groups[i].has_td || atr->groups[i].protocol == 15 || seen[atr->groups[i].protocol])
			continue;
		seen[atr->groups[i].protocol] = true;
		out[count++] = atr->groups[i].protocol;
	}
	if (count == 0) {
		out[0] = 0;
		return 1;
	}
	if (seen[1] && out[0] != 1) {
		size_t at = 0;
		while (out[at] != 1)
			at++;
		for (; at > 0; at--)
			out[at] = out[at - 1];
		out[0] = 1;
	}
	return count;
}
