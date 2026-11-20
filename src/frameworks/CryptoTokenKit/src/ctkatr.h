#ifndef CTKATR_H
#define CTKATR_H

// Parser for the answer-to-reset of ISO/IEC 7816-3, free of Objective-C so that it can be tested on any host.
// TKSmartCardATR.m wraps it.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Answer-to-reset, ISO/IEC 7816-3 clause 8. Interface group i (1-based, as TAi/TBi/TCi/TDi are numbered)
// is stored at groups[i - 1].
#define CTK_ATR_MAX_LENGTH 33
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

// Returns the next octet, or a negative value when there are none left.
typedef int (*ctk_byte_source)(void *context);

// Reads exactly one ATR from the source: TS, T0, the interface bytes, the historical bytes and, when any TDi
// announces a protocol other than T=0, TCK (which must make the XOR of T0..TCK zero). Fails on a TS that is
// neither 3B nor 3F, on a source that runs dry, and on an ATR longer than CTK_ATR_MAX_LENGTH.
bool ctk_atr_parse(ctk_byte_source source, void *context, ctk_atr *out);

#endif
