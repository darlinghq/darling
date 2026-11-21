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

#ifndef CTKATR_H
#define CTKATR_H

// Parser for the answer-to-reset of ISO/IEC 7816-3, free of Objective-C so that it can be tested on any host.
// TKSmartCardATR.m wraps it.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Implementation bound on the octets read for one ATR. ISO/IEC 7816-3 clause 8 allows 33, but readers and
// stream sources do not always keep to it, so a longer ATR is read as far as it is well formed; this bound only
// stops a source that never ends from using unbounded memory.
#define CTK_ATR_MAX_LENGTH 512

// Interface group i (1-based, as TAi/TBi/TCi/TDi are numbered) is stored at groups[i - 1]. There is always a
// first group, and one more after every TDi, even when TDi announces no interface bytes for it.
typedef struct {
	bool has_ta, has_tb, has_tc, has_td;
	uint8_t ta, tb, tc;
	uint8_t protocol; // the T in the low nibble of TDi, valid when has_td
} ctk_atr_group;

typedef struct {
	uint8_t bytes[CTK_ATR_MAX_LENGTH];
	size_t length;
	size_t historical_offset;
	size_t historical_length;
	ctk_atr_group groups[CTK_ATR_MAX_LENGTH];
	size_t group_count;
} ctk_atr;

// Returns the next octet (0..255 is used as given, a larger value keeps its low eight bits), or a negative
// value when there are none left.
typedef int (*ctk_byte_source)(void *context);

// Reads exactly one ATR from the source: TS, T0, the interface bytes, the historical bytes and, when any TDi
// announces a protocol other than T=0, TCK. Fails on a source that runs dry, on a missing TCK and on an ATR
// longer than CTK_ATR_MAX_LENGTH. The value of TS and of TCK is not checked.
bool ctk_atr_parse(ctk_byte_source source, void *context, ctk_atr *out);

// Writes the protocol types T, most preferred first, that the ATR offers, and returns how many (at most 16).
// With TA2 present the card is in specific mode and TA2's low nibble is the only one. Otherwise they are the T
// of every TDi except 15, each once, with T=1 first and the rest in the order of the TDi; with none of those the
// protocol is T=0.
size_t ctk_atr_protocols(const ctk_atr *atr, uint8_t out[16]);

#endif
