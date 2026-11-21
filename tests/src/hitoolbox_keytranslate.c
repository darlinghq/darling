// Copyright (C) 2026 Lubos Dolezel
//
// KeyTranslate with a hand-built 'KCHR' block: the modifier byte selecting a character table, the
// key-up bit, and dead keys (the pending-accent state, completer pairs, the accent followed by a
// character that does not combine, a second accent key, keys without a character, key-up events).
// The block layout and the expected results are those observed on macOS 10.14.4 (i386 and x86_64)
// for the 'KCHR' data GetScriptManagerVariable(smKCHRCache) returns. Build for x86_64 and i386.
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef uint32_t UInt32;
typedef uint16_t UInt16;

extern UInt32 KeyTranslate(const void* transData, UInt16 keycode, UInt32* state);

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_KEY(modifiers, key, state_in, expect_result, expect_state) do { \
	UInt32 st_ = (state_in); \
	UInt32 r_ = KeyTranslate(g_kchr, (UInt16) (((modifiers) << 8) | (key)), &st_); \
	if (r_ != (UInt32) (expect_result) || st_ != (UInt32) (expect_state)) { \
		printf("FAIL %d: key %02x modifiers %02x state %x -> %x state %x, expected %x state %x\n", __LINE__, (key), (modifiers), (unsigned) (state_in), (unsigned) r_, (unsigned) st_, (unsigned) (expect_result), (unsigned) (expect_state)); \
		g_failures++; \
	} \
} while (0)

static uint8_t g_kchr[4096];
static UInt32 g_state_e, g_state_u;

static size_t put16(size_t at, uint16_t v)
{
	memcpy(g_kchr + at, &v, 2);
	return at + 2;
}

static size_t put_dead_key(size_t at, uint8_t table, uint8_t key, const uint8_t pairs[][2], int pair_count, uint8_t alone, UInt32* state)
{
	*state = (UInt32) (at - 258);
	g_kchr[at++] = table;
	g_kchr[at++] = key;
	at = put16(at, (uint16_t) pair_count);
	for (int i = 0; i < pair_count; i++) {
		g_kchr[at++] = pairs[i][0];
		g_kchr[at++] = pairs[i][1];
	}
	return put16(at, alone);
}

static void build(void)
{
	memset(g_kchr, 0, sizeof(g_kchr));
	// version 0; modifier byte -> table: shift and Caps Lock use table 1, Option table 2
	g_kchr[2 + 0x02] = 1;
	g_kchr[2 + 0x04] = 1;
	g_kchr[2 + 0x08] = 2;
	put16(258, 3);
	uint8_t* t0 = g_kchr + 260;
	uint8_t* t1 = t0 + 128;
	uint8_t* t2 = t1 + 128;
	t0[0x00] = 'a'; t0[0x01] = 's'; t0[0x0e] = 'e'; t0[0x20] = 'u'; t0[0x31] = ' '; t0[0x7e] = 0x1e;
	t1[0x00] = 'A'; t1[0x01] = 'S'; t1[0x0e] = 'E'; t1[0x20] = 'U'; t1[0x31] = ' '; t1[0x7e] = 0x1e;
	t2[0x00] = 0x8c; t2[0x01] = 0xa7; t2[0x31] = 0xca; // Option: A-ring, sharp s, no-break space; E and U are dead keys
	size_t at = put16(260 + 3 * 128, 2);
	static const uint8_t acute[][2] = { { 0x20, 0xab }, { 0x41, 0xe7 }, { 0x45, 0xe9 }, { 0x61, 0x87 }, { 0x65, 0x8e } };
	static const uint8_t umlaut[][2] = { { 0x20, 0xac }, { 0x61, 0x8a }, { 0x41, 0x80 } };
	at = put_dead_key(at, 2, 0x0e, acute, 5, 0xab, &g_state_e);
	put_dead_key(at, 2, 0x20, umlaut, 3, 0xac, &g_state_u);
}

int main(void)
{
	build();
	CHECK(g_state_e != 0 && g_state_u != 0 && g_state_e != g_state_u);

	// Characters per modifier layer; Command, the right-hand modifier bits and the up flag do not
	// change the table choice here.
	CHECK_KEY(0x00, 0x00, 0, 'a', 0);
	CHECK_KEY(0x01, 0x00, 0, 'a', 0);
	CHECK_KEY(0x02, 0x00, 0, 'A', 0);
	CHECK_KEY(0x04, 0x00, 0, 'A', 0);
	CHECK_KEY(0x08, 0x00, 0, 0x8c, 0);
	CHECK_KEY(0x08, 0x01, 0, 0xa7, 0);
	CHECK_KEY(0x00, 0x80 | 0x00, 0, 'a', 0);
	CHECK_KEY(0x00, 0x7e, 0, 0x1e, 0);
	CHECK_KEY(0x00, 0x36, 0, 0, 0); // a key without a character

	// A dead key gives no character and leaves its state; the state is different per accent.
	CHECK_KEY(0x08, 0x0e, 0, 0, g_state_e);
	CHECK_KEY(0x08, 0x20, 0, 0, g_state_u);

	// The next key combines with the accent when it has a completer pair, whatever the modifiers
	// that produce the same character; the state is cleared.
	CHECK_KEY(0x00, 0x00, g_state_e, 0x87, 0);
	CHECK_KEY(0x01, 0x00, g_state_e, 0x87, 0);
	CHECK_KEY(0x02, 0x00, g_state_e, 0xe7, 0);
	CHECK_KEY(0x04, 0x00, g_state_e, 0xe7, 0);
	CHECK_KEY(0x00, 0x0e, g_state_e, 0x8e, 0);
	CHECK_KEY(0x00, 0x00, g_state_u, 0x8a, 0);
	CHECK_KEY(0x02, 0x00, g_state_u, 0x80, 0);
	// Space gives the accent alone, as one character.
	CHECK_KEY(0x00, 0x31, g_state_e, 0xab, 0);
	CHECK_KEY(0x00, 0x31, g_state_u, 0xac, 0);
	// A character that does not combine comes out after the accent: accent in bits 16-23.
	CHECK_KEY(0x00, 0x01, g_state_e, (0xabu << 16) | 's', 0);
	CHECK_KEY(0x02, 0x01, g_state_e, (0xabu << 16) | 'S', 0);
	CHECK_KEY(0x08, 0x00, g_state_e, (0xabu << 16) | 0x8c, 0);
	CHECK_KEY(0x00, 0x7e, g_state_u, (0xacu << 16) | 0x1e, 0);

	// Keys without a character leave the accent pending.
	CHECK_KEY(0x00, 0x36, g_state_e, 0, g_state_e);
	// A second accent key gives out the first accent and starts the new one (also the same one).
	CHECK_KEY(0x08, 0x0e, g_state_e, 0xab, g_state_e);
	CHECK_KEY(0x08, 0x20, g_state_e, 0xab, g_state_u);
	CHECK_KEY(0x08, 0x0e, g_state_u, 0xac, g_state_e);

	// Key-up events never touch the state; an up event of an accent key gives the accent.
	CHECK_KEY(0x08, 0x80 | 0x0e, 0, 0xab, 0);
	CHECK_KEY(0x08, 0x80 | 0x20, 0, 0xac, 0);
	CHECK_KEY(0x00, 0x80 | 0x00, g_state_e, 'a', g_state_e);

	if (g_failures == 0)
		printf("PASS\n");
	else
		printf("%d failure(s)\n", g_failures);
	return g_failures != 0;
}
