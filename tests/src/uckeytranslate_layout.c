// Copyright (C) 2026 Lubos Dolezel
//
// UCKeyTranslate against a hand-built 'uchr' layout, from the description in the public
// UnicodeUtilities.h: keyboard-type entries, the modifier byte to table mapping, key to character
// tables, state records with terminal and range entries, state terminators and sequence data.
//
// The expected results were observed on macOS 26.7.3 and on macOS 10.14.4 (identical on both),
// where the same layout blob gives these results: dead-key states and the state value that
// UCKeyTranslate hands back, sequences, the four key actions, kUCKeyTranslateNoDeadKeysMask,
// too-small output buffers (kUCOutputBufferTooSmall with the characters that fit), the keyboard
// type selection and missing optional tables. Build for x86_64 and i386.
#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Entry formats of a state record (kUCKeyStateEntryTerminalFormat, kUCKeyStateEntryRangeFormat).
enum { FMT_TERMINAL = 0x0001, FMT_RANGE = 0x0002 };

static uint8_t g[8192], g_noterm[8192], g_nostate[8192], g_noseq[8192];
static size_t at;

static void w16(size_t o, uint16_t v) { memcpy(g + o, &v, 2); }
static void w32(size_t o, uint32_t v) { memcpy(g + o, &v, 4); }
static size_t put16(uint16_t v) { w16(at, v); at += 2; return at - 2; }
static size_t put32(uint32_t v) { w32(at, v); at += 4; return at - 4; }
static void align4(void) { while (at % 4) g[at++] = 0; }

enum { KTYPES = 3, TABLE_SIZE = 64, NTABLES = 4 };

enum {
	K_A = 0, K_S = 1, K_DEAD1 = 2, K_E = 3, K_DEAD2 = 4, K_DEAD3 = 5, K_RANGE = 6, K_CHAIN = 7,
	K_SEQ0 = 8, K_SEQ1 = 9, K_SEQBAD = 10, K_FFFE = 11, K_FFFF = 12, K_ZERO = 13,
	K_RECSEQ = 15, K_STATEBAD = 16, K_DEADFFFE = 18, K_DEADFFFF = 19, K_DEAD0 = 20,
	K_TERM = 21, K_OPTDEAD = 22, K_XKEY = 23, K_QKEY = 24
};

static size_t typeHdr[KTYPES];

static void build(void)
{
	memset(g, 0, sizeof g);
	at = 0;
	put16(kUCKeyLayoutHeaderFormat);
	put16(0x0100);
	size_t featureOffAt = put32(0);
	put32(KTYPES);
	for (int i = 0; i < KTYPES; i++) {
		typeHdr[i] = at;
		at += 28;
	}
	align4();
	w32(featureOffAt, (uint32_t) at);
	put16(kUCKeyLayoutFeatureInfoFormat);
	put16(0);
	put32(4);

	// Modifier byte -> table: 0 -> 0; Shift and Caps -> 1; Option -> 2; every other value, and
	// everything past the count: default table 3.
	size_t modOff = at;
	put16(kUCKeyModifiersToTableNumFormat);
	put16(3);
	put32(32);
	for (int m = 0; m < 32; m++) {
		int t = 3;
		if (m == 0) t = 0;
		else if (m == 2 || m == 4) t = 1;
		else if (m == 8) t = 2;
		g[at++] = (uint8_t) t;
	}
	align4();

	size_t idx0 = at;
	put16(kUCKeyToCharTableIndexFormat);
	put16(TABLE_SIZE);
	put32(NTABLES);
	size_t offAt = at;
	at += 4 * NTABLES;
	uint16_t* tab[NTABLES];
	for (int t = 0; t < NTABLES; t++) {
		w32(offAt + 4 * t, (uint32_t) at);
		tab[t] = (uint16_t*) (g + at);
		at += TABLE_SIZE * 2;
	}
	for (int t = 0; t < NTABLES; t++) {
		tab[t][K_A] = t == 0 ? 'a' : t == 1 ? 'A' : t == 2 ? 0xe5 : 'D';
		tab[t][K_S] = t == 1 ? 'S' : 's';
		tab[t][K_DEAD1] = kUCKeyOutputStateIndexMask | 0;
		tab[t][K_E] = kUCKeyOutputStateIndexMask | 1;
		tab[t][K_DEAD2] = kUCKeyOutputStateIndexMask | 2;
		tab[t][K_DEAD3] = kUCKeyOutputStateIndexMask | 3;
		tab[t][K_RANGE] = kUCKeyOutputStateIndexMask | 4;
		tab[t][K_CHAIN] = kUCKeyOutputStateIndexMask | 5;
		tab[t][K_SEQ0] = kUCKeyOutputSequenceIndexMask | 0;
		tab[t][K_SEQ1] = kUCKeyOutputSequenceIndexMask | 1;
		tab[t][K_SEQBAD] = kUCKeyOutputSequenceIndexMask | 7;
		tab[t][K_FFFE] = 0xFFFE;
		tab[t][K_FFFF] = 0xFFFF;
		tab[t][K_ZERO] = 0;
		tab[t][K_RECSEQ] = kUCKeyOutputStateIndexMask | 6;
		tab[t][K_STATEBAD] = kUCKeyOutputStateIndexMask | 20;
		tab[t][K_DEADFFFE] = kUCKeyOutputStateIndexMask | 7;
		tab[t][K_DEADFFFF] = kUCKeyOutputStateIndexMask | 8;
		tab[t][K_DEAD0] = kUCKeyOutputStateIndexMask | 9;
		tab[t][K_TERM] = kUCKeyOutputStateIndexMask | 10;
		tab[t][K_XKEY] = kUCKeyOutputStateIndexMask | 11;
		tab[t][K_QKEY] = kUCKeyOutputStateIndexMask | 12;
	}
	tab[2][K_OPTDEAD] = kUCKeyOutputStateIndexMask | 0;
	align4();

	// Entries for other keyboard types: only key 0 is set.
	size_t idx1 = at;
	put16(kUCKeyToCharTableIndexFormat);
	put16(TABLE_SIZE);
	put32(1);
	put32((uint32_t) (at + 4));
	((uint16_t*) (g + at))[K_A] = 'B';
	at += TABLE_SIZE * 2;
	size_t idx2 = at;
	put16(kUCKeyToCharTableIndexFormat);
	put16(TABLE_SIZE);
	put32(1);
	put32((uint32_t) (at + 4));
	((uint16_t*) (g + at))[K_A] = 'C';
	at += TABLE_SIZE * 2;

	size_t recIdx = at;
	put16(kUCKeyStateRecordsIndexFormat);
	put16(13);
	size_t recOffAt = at;
	at += 4 * 13;
	int rec = 0;
#define BEGIN_REC(zero, next, count, fmt) do { w32(recOffAt + 4 * rec, (uint32_t) at); rec++; \
	put16(zero); put16(next); put16(count); put16(fmt); } while (0)
	BEGIN_REC(0x00B4, 1, 0, FMT_TERMINAL);          // 0: dead key, state 1
	BEGIN_REC('e', 0, 1, FMT_TERMINAL);             // 1: 'e'; state 1 -> U+00E9
	put16(1); put16(0x00E9);
	BEGIN_REC(0x0060, 2, 0, FMT_TERMINAL);          // 2: dead key, state 2
	BEGIN_REC(0x005E, 3, 0, FMT_TERMINAL);          // 3: dead key, state 3
	BEGIN_REC('k', 0, 1, FMT_RANGE);                // 4: range: states 1..3 -> x y z
	put16(1); g[at++] = 2; g[at++] = 1; put16('x'); put16(0);
	BEGIN_REC('m', 0, 1, FMT_RANGE);                // 5: state 1 -> 'n', next state 4
	put16(1); g[at++] = 0; g[at++] = 0; put16('n'); put16(4);
	BEGIN_REC(kUCKeyOutputSequenceIndexMask | 0, 0, 1, FMT_TERMINAL); // 6: sequences
	put16(1); put16(kUCKeyOutputSequenceIndexMask | 1);
	BEGIN_REC(0xFFFE, 1, 0, FMT_TERMINAL);          // 7
	BEGIN_REC(0xFFFF, 1, 0, FMT_TERMINAL);          // 8
	BEGIN_REC(0x0000, 1, 0, FMT_TERMINAL);          // 9
	BEGIN_REC('t', 0, 2, FMT_TERMINAL);             // 10: state 1 -> FFFE, 2 -> FFFF
	put16(1); put16(0xFFFE); put16(2); put16(0xFFFF);
	BEGIN_REC('X', 5, 0, FMT_TERMINAL);                               // 11: dead key, state 5
	BEGIN_REC('q', 6, 1, FMT_TERMINAL);                               // 12: dead key, state 6; state 1 -> 'r'
	put16(1); put16('r');
	align4();
	size_t termOff = at;
	put16(kUCKeyStateTerminatorsFormat);
	put16(5);
	put16(0x00B4); put16(0x0060); put16(0x005E); put16(0x00A8); put16(0x0059);
	align4();
	size_t seqOff = at;
	put16(kUCKeySequenceDataIndexFormat);
	put16(3);
	size_t seqOffAt = at;
	at += 2 * 4;
	static const uint16_t seq0[] = { 'f', 'i' }, seq1[] = { 'x', 'y', 'z' }, seq2[] = { 0 };
	const uint16_t* seqs[] = { seq0, seq1, seq2 };
	const size_t lens[] = { 2, 3, 0 };
	for (int i = 0; i < 3; i++) {
		w16(seqOffAt + 2 * i, (uint16_t) (at - seqOff));
		memcpy(g + at, seqs[i], lens[i] * 2);
		at += lens[i] * 2;
	}
	w16(seqOffAt + 6, (uint16_t) (at - seqOff));
	align4();

	uint32_t first[KTYPES] = { 0, 40, 91 }, last[KTYPES] = { 0, 41, 91 };
	uint32_t idx[KTYPES] = { (uint32_t) idx0, (uint32_t) idx1, (uint32_t) idx2 };
	for (int i = 0; i < KTYPES; i++) {
		w32(typeHdr[i] + 0, first[i]);
		w32(typeHdr[i] + 4, last[i]);
		w32(typeHdr[i] + 8, (uint32_t) modOff);
		w32(typeHdr[i] + 12, idx[i]);
		w32(typeHdr[i] + 16, (uint32_t) recIdx);
		w32(typeHdr[i] + 20, (uint32_t) termOff);
		w32(typeHdr[i] + 24, (uint32_t) seqOff);
	}
	// Variants with one optional table missing from every type header.
	memcpy(g_noterm, g, sizeof g);
	memcpy(g_nostate, g, sizeof g);
	memcpy(g_noseq, g, sizeof g);
	for (int i = 0; i < KTYPES; i++) {
		memcpy(g_noterm + typeHdr[i] + 20, "\0\0\0\0", 4);
		memcpy(g_nostate + typeHdr[i] + 16, "\0\0\0\0", 4);
		memcpy(g_noseq + typeHdr[i] + 24, "\0\0\0\0", 4);
	}
}


typedef struct {
	int variant; // 0 full layout, 1 no terminators, 2 no state records, 3 no sequences
	const char* name;
	int key, action, mods;
	unsigned type, options;
	int max;
	unsigned state_in;
	int rc, length, check_state;
	UniChar chars[3];
	unsigned state_out;
} row_t;

static const row_t rows[] = {
	{0, "mods 0", 0, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0061}, 0u},
	{0, "mods 1", 0, 0, 1, 0u, 0u, 8, 0u, 0, 1, 1, {0x0044}, 0u},
	{0, "mods 2", 0, 0, 2, 0u, 0u, 8, 0u, 0, 1, 1, {0x0041}, 0u},
	{0, "mods 3", 0, 0, 3, 0u, 0u, 8, 0u, 0, 1, 1, {0x0044}, 0u},
	{0, "mods 4", 0, 0, 4, 0u, 0u, 8, 0u, 0, 1, 1, {0x0041}, 0u},
	{0, "mods 6", 0, 0, 6, 0u, 0u, 8, 0u, 0, 1, 1, {0x0044}, 0u},
	{0, "mods 8", 0, 0, 8, 0u, 0u, 8, 0u, 0, 1, 1, {0x00E5}, 0u},
	{0, "mods 10", 0, 0, 10, 0u, 0u, 8, 0u, 0, 1, 1, {0x0044}, 0u},
	{0, "mods 16", 0, 0, 16, 0u, 0u, 8, 0u, 0, 1, 1, {0x0044}, 0u},
	{0, "mods 31", 0, 0, 31, 0u, 0u, 8, 0u, 0, 1, 1, {0x0044}, 0u},
	{0, "mods 32", 0, 0, 32, 0u, 0u, 8, 0u, 0, 1, 1, {0x0044}, 0u},
	{0, "mods 64", 0, 0, 64, 0u, 0u, 8, 0u, 0, 1, 1, {0x0044}, 0u},
	{0, "mods 128", 0, 0, 128, 0u, 0u, 8, 0u, 0, 1, 1, {0x0044}, 0u},
	{0, "mods 255", 0, 0, 255, 0u, 0u, 8, 0u, 0, 1, 1, {0x0044}, 0u},
	{0, "mods 256", 0, 0, 256, 0u, 0u, 8, 0u, 0, 1, 1, {0x0061}, 0u},
	{0, "mods 512", 0, 0, 512, 0u, 0u, 8, 0u, 0, 1, 1, {0x0061}, 0u},
	{0, "mods 2048", 0, 0, 2048, 0u, 0u, 8, 0u, 0, 1, 1, {0x0061}, 0u},
	{0, "mods 65535", 0, 0, 65535, 0u, 0u, 8, 0u, 0, 1, 1, {0x0044}, 0u},
	{0, "type 0", 0, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0061}, 0u},
	{0, "type 40", 0, 0, 0, 40u, 0u, 8, 0u, 0, 1, 1, {0x0061}, 0u},
	{0, "type 41", 0, 0, 0, 41u, 0u, 8, 0u, 0, 1, 1, {0x0061}, 0u},
	{0, "type 91", 0, 0, 0, 91u, 0u, 8, 0u, 0, 1, 1, {0x0061}, 0u},
	{0, "type 92", 0, 0, 0, 92u, 0u, 8, 0u, 0, 1, 1, {0x0061}, 0u},
	{0, "type 4294967295", 0, 0, 0, 4294967295u, 0u, 8, 0u, 0, 1, 1, {0x0061}, 0u},
	{0, "key 0", 0, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0061}, 0u},
	{0, "key 1", 1, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0073}, 0u},
	{0, "key 2", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "key 3", 3, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0065}, 0u},
	{0, "key 4", 4, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0060}, 2u},
	{0, "key 5", 5, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x005E}, 3u},
	{0, "key 6", 6, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x006B}, 0u},
	{0, "key 7", 7, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x006D}, 0u},
	{0, "key 8", 8, 0, 0, 0u, 0u, 8, 0u, 0, 2, 1, {0x0066,0x0069}, 0u},
	{0, "key 9", 9, 0, 0, 0u, 0u, 8, 0u, 0, 3, 1, {0x0078,0x0079,0x007A}, 0u},
	{0, "key 10", 10, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x8007}, 0u},
	{0, "key 11", 11, 0, 0, 0u, 0u, 8, 0u, 0, 0, 1, {}, 0u},
	{0, "key 12", 12, 0, 0, 0u, 0u, 8, 0u, 0, 0, 1, {}, 0u},
	{0, "key 13", 13, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0000}, 0u},
	{0, "key 14", 14, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0000}, 0u},
	{0, "key 15", 15, 0, 0, 0u, 0u, 8, 0u, 0, 2, 1, {0x0066,0x0069}, 0u},
	{0, "key 16", 16, 0, 0, 0u, 0u, 8, 0u, -50, 0, 0, {}, 0u},
	{0, "key 17", 17, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0000}, 0u},
	{0, "key 18", 18, 0, 0, 0u, 0u, 8, 0u, 0, 0, 1, {}, 1u},
	{0, "key 19", 19, 0, 0, 0u, 0u, 8, 0u, 0, 0, 1, {}, 1u},
	{0, "key 20", 20, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0000}, 1u},
	{0, "key 21", 21, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0074}, 0u},
	{0, "key 22", 22, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0000}, 0u},
	{0, "key 23", 23, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0058}, 5u},
	{0, "key 64", 64, 0, 0, 0u, 0u, 8, 0u, 0, 0, 1, {}, 0u},
	{0, "key 65", 65, 0, 0, 0u, 0u, 8, 0u, 0, 0, 1, {}, 0u},
	{0, "key 66", 66, 0, 0, 0u, 0u, 8, 0u, 0, 0, 1, {}, 0u},
	{0, "key 67", 67, 0, 0, 0u, 0u, 8, 0u, 0, 0, 1, {}, 0u},
	{0, "key 68", 68, 0, 0, 0u, 0u, 8, 0u, 0, 0, 1, {}, 0u},
	{0, "key 69", 69, 0, 0, 0u, 0u, 8, 0u, 0, 0, 1, {}, 0u},
	{0, "key 70", 70, 0, 0, 0u, 0u, 8, 0u, 0, 0, 1, {}, 0u},
	{0, "key 71", 71, 0, 0, 0u, 0u, 8, 0u, 0, 0, 1, {}, 0u},
	{0, "key 128", 128, 0, 0, 0u, 0u, 8, 0u, 0, 0, 1, {}, 0u},
	{0, "key 256", 256, 0, 0, 0u, 0u, 8, 0u, 0, 0, 1, {}, 0u},
	{0, "key 65535", 65535, 0, 0, 0u, 0u, 8, 0u, 0, 0, 1, {}, 0u},
	{0, "action key 0 act 0", 0, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0061}, 0u},
	{0, "action key 0 act 0 state 1", 0, 0, 0, 0u, 0u, 8, 1u, 0, 2, 1, {0x00B4,0x0061}, 0u},
	{0, "action key 0 act 1", 0, 1, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0061}, 0u},
	{0, "action key 0 act 1 state 1", 0, 1, 0, 0u, 0u, 8, 1u, 0, 2, 1, {0x00B4,0x0061}, 0u},
	{0, "action key 0 act 2", 0, 2, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0061}, 0u},
	{0, "action key 0 act 2 state 1", 0, 2, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x0061}, 1u},
	{0, "action key 0 act 3", 0, 3, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0061}, 0u},
	{0, "action key 0 act 3 state 1", 0, 3, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x0061}, 0u},
	{0, "action key 0 act 4", 0, 4, 0, 0u, 0u, 8, 0u, -50, 119, 0, {}, 0u},
	{0, "action key 0 act 4 state 1", 0, 4, 0, 0u, 0u, 8, 1u, -50, 119, 0, {}, 1u},
	{0, "action key 2 act 0", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "action key 2 act 0 state 1", 2, 0, 0, 0u, 0u, 8, 1u, 0, 2, 1, {0x00B4,0x00B4}, 1u},
	{0, "action key 2 act 1", 2, 1, 0, 0u, 0u, 8, 0u, 0, 2, 1, {0x00B4,0x00B4}, 0u},
	{0, "action key 2 act 1 state 1", 2, 1, 0, 0u, 0u, 8, 1u, 0, 2, 1, {0x00B4,0x00B4}, 0u},
	{0, "action key 2 act 2", 2, 2, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "action key 2 act 2 state 1", 2, 2, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x00B4}, 1u},
	{0, "action key 2 act 3", 2, 3, 0, 0u, 0u, 8, 0u, 0, 2, 1, {0x00B4,0x00B4}, 1u},
	{0, "action key 2 act 3 state 1", 2, 3, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x00B4}, 1u},
	{0, "action key 2 act 4", 2, 4, 0, 0u, 0u, 8, 0u, -50, 119, 0, {}, 0u},
	{0, "action key 2 act 4 state 1", 2, 4, 0, 0u, 0u, 8, 1u, -50, 119, 0, {}, 1u},
	{0, "action key 3 act 0", 3, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0065}, 0u},
	{0, "action key 3 act 0 state 1", 3, 0, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x00E9}, 65536u},
	{0, "action key 3 act 1", 3, 1, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0065}, 0u},
	{0, "action key 3 act 1 state 1", 3, 1, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x00E9}, 0u},
	{0, "action key 3 act 2", 3, 2, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0065}, 0u},
	{0, "action key 3 act 2 state 1", 3, 2, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x0065}, 0u},
	{0, "action key 3 act 3", 3, 3, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0065}, 0u},
	{0, "action key 3 act 3 state 1", 3, 3, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x00E9}, 2147483648u},
	{0, "action key 3 act 4", 3, 4, 0, 0u, 0u, 8, 0u, -50, 119, 0, {}, 0u},
	{0, "action key 3 act 4 state 1", 3, 4, 0, 0u, 0u, 8, 1u, -50, 119, 0, {}, 1u},
	{0, "action key 8 act 0", 8, 0, 0, 0u, 0u, 8, 0u, 0, 2, 1, {0x0066,0x0069}, 0u},
	{0, "action key 8 act 0 state 1", 8, 0, 0, 0u, 0u, 8, 1u, 0, 3, 1, {0x00B4,0x0066,0x0069}, 0u},
	{0, "action key 8 act 1", 8, 1, 0, 0u, 0u, 8, 0u, 0, 2, 1, {0x0066,0x0069}, 0u},
	{0, "action key 8 act 1 state 1", 8, 1, 0, 0u, 0u, 8, 1u, 0, 3, 1, {0x00B4,0x0066,0x0069}, 0u},
	{0, "action key 8 act 2", 8, 2, 0, 0u, 0u, 8, 0u, 0, 2, 1, {0x0066,0x0069}, 0u},
	{0, "action key 8 act 2 state 1", 8, 2, 0, 0u, 0u, 8, 1u, 0, 2, 1, {0x0066,0x0069}, 1u},
	{0, "action key 8 act 3", 8, 3, 0, 0u, 0u, 8, 0u, 0, 2, 1, {0x0066,0x0069}, 0u},
	{0, "action key 8 act 3 state 1", 8, 3, 0, 0u, 0u, 8, 1u, 0, 2, 1, {0x0066,0x0069}, 0u},
	{0, "action key 8 act 4", 8, 4, 0, 0u, 0u, 8, 0u, -50, 119, 0, {}, 0u},
	{0, "action key 8 act 4 state 1", 8, 4, 0, 0u, 0u, 8, 1u, -50, 119, 0, {}, 1u},
	{0, "action key 6 act 0", 6, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x006B}, 0u},
	{0, "action key 6 act 0 state 1", 6, 0, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x0078}, 65536u},
	{0, "action key 6 act 1", 6, 1, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x006B}, 0u},
	{0, "action key 6 act 1 state 1", 6, 1, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x0078}, 0u},
	{0, "action key 6 act 2", 6, 2, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x006B}, 0u},
	{0, "action key 6 act 2 state 1", 6, 2, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x006B}, 0u},
	{0, "action key 6 act 3", 6, 3, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x006B}, 0u},
	{0, "action key 6 act 3 state 1", 6, 3, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x0078}, 2147483648u},
	{0, "action key 6 act 4", 6, 4, 0, 0u, 0u, 8, 0u, -50, 119, 0, {}, 0u},
	{0, "action key 6 act 4 state 1", 6, 4, 0, 0u, 0u, 8, 1u, -50, 119, 0, {}, 1u},
	{0, "dead1 e", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "dead1 e", 3, 0, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x00E9}, 65536u},
	{0, "dead1 a", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "dead1 a", 0, 0, 0, 0u, 0u, 8, 1u, 0, 2, 1, {0x00B4,0x0061}, 0u},
	{0, "dead1 shift-a", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "dead1 shift-a", 0, 0, 2, 0u, 0u, 8, 1u, 0, 2, 1, {0x00B4,0x0041}, 0u},
	{0, "dead1 shift-e", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "dead1 shift-e", 3, 0, 2, 0u, 0u, 8, 1u, 0, 1, 1, {0x00E9}, 65536u},
	{0, "dead1 dead1", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "dead1 dead1", 2, 0, 0, 0u, 0u, 8, 1u, 0, 2, 1, {0x00B4,0x00B4}, 1u},
	{0, "dead1 dead2 e", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "dead1 dead2 e", 4, 0, 0, 0u, 0u, 8, 1u, 0, 2, 1, {0x00B4,0x0060}, 2u},
	{0, "dead1 dead2 e", 3, 0, 0, 0u, 0u, 8, 2u, 0, 2, 1, {0x0060,0x0065}, 0u},
	{0, "dead1 deadFFFE a", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "dead1 deadFFFE a", 18, 0, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x00B4}, 1u},
	{0, "dead1 deadFFFE a", 0, 0, 0, 0u, 0u, 8, 1u, 0, 2, 1, {0x00B4,0x0061}, 0u},
	{0, "deadFFFE e", 18, 0, 0, 0u, 0u, 8, 0u, 0, 0, 1, {}, 1u},
	{0, "deadFFFE e", 3, 0, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x00E9}, 65536u},
	{0, "deadFFFF e", 19, 0, 0, 0u, 0u, 8, 0u, 0, 0, 1, {}, 1u},
	{0, "deadFFFF e", 3, 0, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x00E9}, 65536u},
	{0, "dead0 e", 20, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0000}, 1u},
	{0, "dead0 e", 3, 0, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x00E9}, 65536u},
	{0, "optdead(no option) e", 22, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0000}, 0u},
	{0, "optdead(no option) e", 3, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0065}, 0u},
	{0, "optdead e", 22, 0, 8, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "optdead e", 3, 0, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x00E9}, 65536u},
	{0, "dead1 range", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "dead1 range", 6, 0, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x0078}, 65536u},
	{0, "dead2 range", 4, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0060}, 2u},
	{0, "dead2 range", 6, 0, 0, 0u, 0u, 8, 2u, 0, 1, 1, {0x0079}, 131072u},
	{0, "dead3 range", 5, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x005E}, 3u},
	{0, "dead3 range", 6, 0, 0, 0u, 0u, 8, 3u, 0, 1, 1, {0x007A}, 196608u},
	{0, "range alone", 6, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x006B}, 0u},
	{0, "range state 4", 6, 0, 0, 0u, 0u, 8, 4u, 0, 2, 1, {0x00A8,0x006B}, 0u},
	{0, "dead1 chain e", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "dead1 chain e", 7, 0, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x006E}, 65540u},
	{0, "dead1 chain e", 3, 0, 0, 0u, 0u, 8, 65540u, 0, 2, 1, {0x00A8,0x0065}, 0u},
	{0, "dead1 chain a", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "dead1 chain a", 7, 0, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x006E}, 65540u},
	{0, "dead1 chain a", 0, 0, 0, 0u, 0u, 8, 65540u, 0, 2, 1, {0x00A8,0x0061}, 0u},
	{0, "dead1 seq0", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "dead1 seq0", 8, 0, 0, 0u, 0u, 8, 1u, 0, 3, 1, {0x00B4,0x0066,0x0069}, 0u},
	{0, "dead1 recseq", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "dead1 recseq", 15, 0, 0, 0u, 0u, 8, 1u, 0, 3, 1, {0x0078,0x0079,0x007A}, 65536u},
	{0, "recseq", 15, 0, 0, 0u, 0u, 8, 0u, 0, 2, 1, {0x0066,0x0069}, 0u},
	{0, "dead1 FFFE e", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "dead1 FFFE e", 11, 0, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x00B4}, 0u},
	{0, "dead1 FFFE e", 3, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0065}, 0u},
	{0, "dead1 FFFF e", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "dead1 FFFF e", 12, 0, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x00B4}, 0u},
	{0, "dead1 FFFF e", 3, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0065}, 0u},
	{0, "dead1 zero e", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "dead1 zero e", 13, 0, 0, 0u, 0u, 8, 1u, 0, 2, 1, {0x00B4,0x0000}, 0u},
	{0, "dead1 zero e", 3, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0065}, 0u},
	{0, "dead1 term", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "dead1 term", 21, 0, 0, 0u, 0u, 8, 1u, 0, 0, 1, {}, 65536u},
	{0, "dead2 term", 4, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0060}, 2u},
	{0, "dead2 term", 21, 0, 0, 0u, 0u, 8, 2u, 0, 0, 1, {}, 131072u},
	{0, "dead3 term", 5, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x005E}, 3u},
	{0, "dead3 term", 21, 0, 0, 0u, 0u, 8, 3u, 0, 2, 1, {0x005E,0x0074}, 0u},
	{0, "dead1 statebad", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "dead1 statebad", 16, 0, 0, 0u, 0u, 8, 1u, -50, 0, 0, {}, 65537u},
	{0, "dead1 e-up e", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "dead1 e-up e", 3, 1, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x00E9}, 0u},
	{0, "dead1 e-up e", 3, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0065}, 0u},
	{0, "dead1-up e", 2, 1, 0, 0u, 0u, 8, 0u, 0, 2, 1, {0x00B4,0x00B4}, 0u},
	{0, "dead1-up e", 3, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0065}, 0u},
	{0, "dead1-auto e", 2, 2, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "dead1-auto e", 3, 0, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x00E9}, 65536u},
	{0, "dead1-display e", 2, 3, 0, 0u, 0u, 8, 0u, 0, 2, 1, {0x00B4,0x00B4}, 1u},
	{0, "dead1-display e", 3, 0, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x00E9}, 65536u},
	{0, "dead1 e-display e", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "dead1 e-display e", 3, 3, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x00E9}, 2147483648u},
	{0, "dead1 e-display e", 3, 0, 0, 0u, 0u, 8, 2147483648u, 0, 1, 1, {0x0065}, 0u},
	{0, "dead1 e-auto", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "dead1 e-auto", 3, 2, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x0065}, 0u},
	{0, "nodead dead1 e", 2, 0, 0, 0u, 1u, 8, 0u, 0, 2, 1, {0x00B4,0x00B4}, 0u},
	{0, "nodead dead1 e", 3, 0, 0, 0u, 1u, 8, 0u, 0, 1, 1, {0x0065}, 0u},
	{0, "nodead dead1 state 1", 2, 0, 0, 0u, 1u, 8, 1u, 0, 2, 1, {0x00B4,0x00B4}, 0u},
	{0, "nodead e state 1", 3, 0, 0, 0u, 1u, 8, 1u, 0, 1, 1, {0x00E9}, 65536u},
	{0, "nodead a state 1", 0, 0, 0, 0u, 1u, 8, 1u, 0, 2, 1, {0x00B4,0x0061}, 0u},
	{0, "dead1 nodead-e", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "dead1 nodead-e", 3, 0, 0, 0u, 1u, 8, 1u, 0, 1, 1, {0x00E9}, 65536u},
	{0, "nodead dead2", 4, 0, 0, 0u, 1u, 8, 0u, 0, 2, 1, {0x0060,0x0060}, 0u},
	{0, "nodead deadFFFF", 19, 0, 0, 0u, 1u, 8, 0u, 0, 1, 1, {0x00B4}, 0u},
	{0, "nodead dead0", 20, 0, 0, 0u, 1u, 8, 0u, 0, 2, 1, {0x0000,0x00B4}, 0u},
	{0, "nodead dead1 display", 2, 3, 0, 0u, 1u, 8, 0u, 0, 2, 1, {0x00B4,0x00B4}, 0u},
	{0, "nodead dead1 up", 2, 1, 0, 0u, 1u, 8, 0u, 0, 2, 1, {0x00B4,0x00B4}, 0u},
	{0, "nodead seq0", 8, 0, 0, 0u, 1u, 8, 0u, 0, 2, 1, {0x0066,0x0069}, 0u},
	{0, "option bit 1 dead1", 2, 0, 0, 0u, 2u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "all options dead1", 2, 0, 0, 0u, 4294967295u, 8, 0u, 0, 2, 1, {0x00B4,0x00B4}, 0u},
	{0, "e state 7", 3, 0, 0, 0u, 0u, 8, 7u, 0, 1, 1, {0x0065}, 0u},
	{0, "e state 4", 3, 0, 0, 0u, 0u, 8, 4u, 0, 2, 1, {0x00A8,0x0065}, 0u},
	{0, "e state 0x10001", 3, 0, 0, 0u, 0u, 8, 65537u, 0, 1, 1, {0x00E9}, 65536u},
	{0, "a state 1", 0, 0, 0, 0u, 0u, 8, 1u, 0, 2, 1, {0x00B4,0x0061}, 0u},
	{0, "a state 4", 0, 0, 0, 0u, 0u, 8, 4u, 0, 2, 1, {0x00A8,0x0061}, 0u},
	{0, "a state 9", 0, 0, 0, 0u, 0u, 8, 9u, 0, 1, 1, {0x0061}, 0u},
	{0, "max 0 dead1 a (two chars)", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "max 0 dead1 a (two chars)", 0, 0, 0, 0u, 0u, 0, 1u, -25340, 0, 1, {}, 0u},
	{0, "max 0 dead1 a (two chars)", 0, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0061}, 0u},
	{0, "max 0 seq0 (two chars)", 8, 0, 0, 0u, 0u, 0, 0u, -25340, 0, 1, {}, 0u},
	{0, "max 0 seq0 (two chars)", 8, 0, 0, 0u, 0u, 8, 0u, 0, 2, 1, {0x0066,0x0069}, 0u},
	{0, "max 0 seq1 (three chars)", 9, 0, 0, 0u, 0u, 0, 0u, -25340, 0, 1, {}, 0u},
	{0, "max 0 a (one char)", 0, 0, 0, 0u, 0u, 0, 0u, -25340, 0, 1, {}, 0u},
	{0, "max 0 dead1 e", 2, 0, 0, 0u, 0u, 0, 0u, -25340, 0, 1, {}, 1u},
	{0, "max 0 dead1 e", 3, 0, 0, 0u, 0u, 0, 1u, -25340, 0, 1, {}, 65536u},
	{0, "max 0 dead1 e", 3, 0, 0, 0u, 0u, 8, 65536u, 0, 1, 1, {0x0065}, 0u},
	{0, "max 0 nodead dead1 (one char)", 2, 0, 0, 0u, 1u, 0, 0u, -25340, 0, 1, {}, 0u},
	{0, "max 1 dead1 a (two chars)", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "max 1 dead1 a (two chars)", 0, 0, 0, 0u, 0u, 1, 1u, -25340, 1, 1, {0x00B4}, 0u},
	{0, "max 1 dead1 a (two chars)", 0, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0061}, 0u},
	{0, "max 1 seq0 (two chars)", 8, 0, 0, 0u, 0u, 1, 0u, -25340, 1, 1, {0x0066}, 0u},
	{0, "max 1 seq0 (two chars)", 8, 0, 0, 0u, 0u, 8, 0u, 0, 2, 1, {0x0066,0x0069}, 0u},
	{0, "max 1 seq1 (three chars)", 9, 0, 0, 0u, 0u, 1, 0u, -25340, 1, 1, {0x0078}, 0u},
	{0, "max 1 a (one char)", 0, 0, 0, 0u, 0u, 1, 0u, 0, 1, 1, {0x0061}, 0u},
	{0, "max 1 dead1 e", 2, 0, 0, 0u, 0u, 1, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "max 1 dead1 e", 3, 0, 0, 0u, 0u, 1, 1u, 0, 1, 1, {0x00E9}, 65536u},
	{0, "max 1 dead1 e", 3, 0, 0, 0u, 0u, 8, 65536u, 0, 1, 1, {0x0065}, 0u},
	{0, "max 1 nodead dead1 (one char)", 2, 0, 0, 0u, 1u, 1, 0u, -25340, 1, 1, {0x00B4}, 0u},
	{0, "max 2 dead1 a (two chars)", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "max 2 dead1 a (two chars)", 0, 0, 0, 0u, 0u, 2, 1u, 0, 2, 1, {0x00B4,0x0061}, 0u},
	{0, "max 2 dead1 a (two chars)", 0, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0061}, 0u},
	{0, "max 2 seq0 (two chars)", 8, 0, 0, 0u, 0u, 2, 0u, 0, 2, 1, {0x0066,0x0069}, 0u},
	{0, "max 2 seq0 (two chars)", 8, 0, 0, 0u, 0u, 8, 0u, 0, 2, 1, {0x0066,0x0069}, 0u},
	{0, "max 2 seq1 (three chars)", 9, 0, 0, 0u, 0u, 2, 0u, -25340, 2, 1, {0x0078,0x0079}, 0u},
	{0, "max 2 a (one char)", 0, 0, 0, 0u, 0u, 2, 0u, 0, 1, 1, {0x0061}, 0u},
	{0, "max 2 dead1 e", 2, 0, 0, 0u, 0u, 2, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "max 2 dead1 e", 3, 0, 0, 0u, 0u, 2, 1u, 0, 1, 1, {0x00E9}, 65536u},
	{0, "max 2 dead1 e", 3, 0, 0, 0u, 0u, 8, 65536u, 0, 1, 1, {0x0065}, 0u},
	{0, "max 2 nodead dead1 (one char)", 2, 0, 0, 0u, 1u, 2, 0u, 0, 2, 1, {0x00B4,0x00B4}, 0u},
	{0, "max 3 dead1 a (two chars)", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "max 3 dead1 a (two chars)", 0, 0, 0, 0u, 0u, 3, 1u, 0, 2, 1, {0x00B4,0x0061}, 0u},
	{0, "max 3 dead1 a (two chars)", 0, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0061}, 0u},
	{0, "max 3 seq0 (two chars)", 8, 0, 0, 0u, 0u, 3, 0u, 0, 2, 1, {0x0066,0x0069}, 0u},
	{0, "max 3 seq0 (two chars)", 8, 0, 0, 0u, 0u, 8, 0u, 0, 2, 1, {0x0066,0x0069}, 0u},
	{0, "max 3 seq1 (three chars)", 9, 0, 0, 0u, 0u, 3, 0u, 0, 3, 1, {0x0078,0x0079,0x007A}, 0u},
	{0, "max 3 a (one char)", 0, 0, 0, 0u, 0u, 3, 0u, 0, 1, 1, {0x0061}, 0u},
	{0, "max 3 dead1 e", 2, 0, 0, 0u, 0u, 3, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "max 3 dead1 e", 3, 0, 0, 0u, 0u, 3, 1u, 0, 1, 1, {0x00E9}, 65536u},
	{0, "max 3 dead1 e", 3, 0, 0, 0u, 0u, 8, 65536u, 0, 1, 1, {0x0065}, 0u},
	{0, "max 3 nodead dead1 (one char)", 2, 0, 0, 0u, 1u, 3, 0u, 0, 2, 1, {0x00B4,0x00B4}, 0u},
	{0, "max 4 dead1 a (two chars)", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "max 4 dead1 a (two chars)", 0, 0, 0, 0u, 0u, 4, 1u, 0, 2, 1, {0x00B4,0x0061}, 0u},
	{0, "max 4 dead1 a (two chars)", 0, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0061}, 0u},
	{0, "max 4 seq0 (two chars)", 8, 0, 0, 0u, 0u, 4, 0u, 0, 2, 1, {0x0066,0x0069}, 0u},
	{0, "max 4 seq0 (two chars)", 8, 0, 0, 0u, 0u, 8, 0u, 0, 2, 1, {0x0066,0x0069}, 0u},
	{0, "max 4 seq1 (three chars)", 9, 0, 0, 0u, 0u, 4, 0u, 0, 3, 1, {0x0078,0x0079,0x007A}, 0u},
	{0, "max 4 a (one char)", 0, 0, 0, 0u, 0u, 4, 0u, 0, 1, 1, {0x0061}, 0u},
	{0, "max 4 dead1 e", 2, 0, 0, 0u, 0u, 4, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "max 4 dead1 e", 3, 0, 0, 0u, 0u, 4, 1u, 0, 1, 1, {0x00E9}, 65536u},
	{0, "max 4 dead1 e", 3, 0, 0, 0u, 0u, 8, 65536u, 0, 1, 1, {0x0065}, 0u},
	{0, "max 4 nodead dead1 (one char)", 2, 0, 0, 0u, 1u, 4, 0u, 0, 2, 1, {0x00B4,0x00B4}, 0u},
	{0, "X down", 23, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0058}, 5u},
	{0, "X down", 0, 0, 0, 0u, 0u, 8, 5u, 0, 2, 1, {0x0059,0x0061}, 0u},
	{0, "X nodead", 23, 0, 0, 0u, 1u, 8, 0u, 0, 2, 1, {0x0058,0x0059}, 0u},
	{0, "X up", 23, 1, 0, 0u, 0u, 8, 0u, 0, 2, 1, {0x0058,0x0059}, 0u},
	{0, "X display", 23, 3, 0, 0u, 0u, 8, 0u, 0, 2, 1, {0x0058,0x0059}, 5u},
	{0, "X auto", 23, 2, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0058}, 5u},
	{0, "X in state 1 down", 23, 0, 0, 0u, 0u, 8, 1u, 0, 2, 1, {0x00B4,0x0058}, 5u},
	{0, "X in state 1 nodead", 23, 0, 0, 0u, 1u, 8, 1u, 0, 2, 1, {0x00B4,0x0058}, 0u},
	{0, "X in state 1 up", 23, 1, 0, 0u, 0u, 8, 1u, 0, 2, 1, {0x00B4,0x0058}, 0u},
	{0, "X in state 1 display", 23, 3, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x0058}, 5u},
	{0, "X in state 1 auto", 23, 2, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x0058}, 5u},
	{0, "a in state 1 down/up/auto/display", 0, 0, 0, 0u, 0u, 8, 1u, 0, 2, 1, {0x00B4,0x0061}, 0u},
	{0, "a in state 1 down/up/auto/display", 0, 1, 0, 0u, 0u, 8, 1u, 0, 2, 1, {0x00B4,0x0061}, 0u},
	{0, "a in state 1 down/up/auto/display", 0, 2, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x0061}, 1u},
	{0, "a in state 1 down/up/auto/display", 0, 3, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x0061}, 0u},
	{0, "X in state 5 down", 23, 0, 0, 0u, 0u, 8, 5u, 0, 2, 1, {0x0059,0x0058}, 5u},
	{0, "X in state 5 nodead", 23, 0, 0, 0u, 1u, 8, 5u, 0, 2, 1, {0x0059,0x0058}, 0u},
	{0, "X in state 5 up", 23, 1, 0, 0u, 0u, 8, 5u, 0, 2, 1, {0x0059,0x0058}, 0u},
	{0, "X in state 5 display", 23, 3, 0, 0u, 0u, 8, 5u, 0, 1, 1, {0x0058}, 5u},
	{0, "X in state 5 auto", 23, 2, 0, 0u, 0u, 8, 5u, 0, 1, 1, {0x0058}, 5u},
	{0, "X in state 6 down", 23, 0, 0, 0u, 0u, 8, 6u, 0, 1, 1, {0x0058}, 5u},
	{0, "X in state 6 nodead", 23, 0, 0, 0u, 1u, 8, 6u, 0, 1, 1, {0x0058}, 0u},
	{0, "X in state 6 up", 23, 1, 0, 0u, 0u, 8, 6u, 0, 1, 1, {0x0058}, 0u},
	{0, "X in state 6 display", 23, 3, 0, 0u, 0u, 8, 6u, 0, 1, 1, {0x0058}, 5u},
	{0, "X in state 6 auto", 23, 2, 0, 0u, 0u, 8, 6u, 0, 1, 1, {0x0058}, 5u},
	{0, "a in state 6 down/up/auto/display", 0, 0, 0, 0u, 0u, 8, 6u, 0, 1, 1, {0x0061}, 0u},
	{0, "a in state 6 down/up/auto/display", 0, 1, 0, 0u, 0u, 8, 6u, 0, 1, 1, {0x0061}, 0u},
	{0, "a in state 6 down/up/auto/display", 0, 2, 0, 0u, 0u, 8, 6u, 0, 1, 1, {0x0061}, 6u},
	{0, "a in state 6 down/up/auto/display", 0, 3, 0, 0u, 0u, 8, 6u, 0, 1, 1, {0x0061}, 0u},
	{0, "Q dead then r-key", 24, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0071}, 6u},
	{0, "Q dead then r-key", 0, 0, 0, 0u, 0u, 8, 6u, 0, 1, 1, {0x0061}, 0u},
	{0, "dead1 Q", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{0, "dead1 Q", 24, 0, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x0072}, 65536u},
	{0, "dead1 Q", 0, 0, 0, 0u, 0u, 8, 65536u, 0, 1, 1, {0x0061}, 0u},
	{0, "Q Q", 24, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0071}, 6u},
	{0, "Q Q", 24, 0, 0, 0u, 0u, 8, 6u, 0, 1, 1, {0x0071}, 6u},
	{0, "seq in state 5", 8, 0, 0, 0u, 0u, 8, 5u, 0, 3, 1, {0x0059,0x0066,0x0069}, 0u},
	{0, "max 1 X in state 1", 23, 0, 0, 0u, 0u, 1, 1u, -25340, 1, 1, {0x00B4}, 5u},
	{0, "max 1 X in state 1", 0, 0, 0, 0u, 0u, 8, 5u, 0, 2, 1, {0x0059,0x0061}, 0u},
	{0, "max 1 nodead X", 23, 0, 0, 0u, 1u, 1, 0u, -25340, 1, 1, {0x0058}, 0u},
	{0, "max 2 nodead X", 23, 0, 0, 0u, 1u, 2, 0u, 0, 2, 1, {0x0058,0x0059}, 0u},
	{0, "max 2 X in state 1", 23, 0, 0, 0u, 0u, 2, 1u, 0, 2, 1, {0x00B4,0x0058}, 5u},
	{0, "max 1 up dead1", 2, 1, 0, 0u, 0u, 1, 0u, -25340, 1, 1, {0x00B4}, 0u},
	{0, "max 1 display dead1", 2, 3, 0, 0u, 0u, 1, 0u, -25340, 1, 1, {0x00B4}, 1u},
	{1, "noterminators dead1 e", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{1, "noterminators dead1 e", 3, 0, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x00E9}, 65536u},
	{1, "noterminators dead1 a", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{1, "noterminators dead1 a", 0, 0, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x0061}, 0u},
	{1, "noterminators seq0 seq1 seqbad", 8, 0, 0, 0u, 0u, 8, 0u, 0, 2, 1, {0x0066,0x0069}, 0u},
	{1, "noterminators seq0 seq1 seqbad", 9, 0, 0, 0u, 0u, 8, 0u, 0, 3, 1, {0x0078,0x0079,0x007A}, 0u},
	{1, "noterminators seq0 seq1 seqbad", 10, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x8007}, 0u},
	{1, "noterminators stateindex keys", 16, 0, 0, 0u, 0u, 8, 0u, -50, 0, 0, {}, 0u},
	{1, "noterminators stateindex keys", 15, 0, 0, 0u, 0u, 8, 0u, 0, 2, 1, {0x0066,0x0069}, 0u},
	{1, "noterminators stateindex keys", 6, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x006B}, 0u},
	{2, "nostaterecords dead1 e", 2, 0, 0, 0u, 0u, 8, 0u, -50, 0, 0, {}, 0u},
	{2, "nostaterecords dead1 e", 3, 0, 0, 0u, 0u, 8, 0u, -50, 0, 0, {}, 0u},
	{2, "nostaterecords dead1 a", 2, 0, 0, 0u, 0u, 8, 0u, -50, 0, 0, {}, 0u},
	{2, "nostaterecords dead1 a", 0, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x0061}, 0u},
	{2, "nostaterecords seq0 seq1 seqbad", 8, 0, 0, 0u, 0u, 8, 0u, 0, 2, 1, {0x0066,0x0069}, 0u},
	{2, "nostaterecords seq0 seq1 seqbad", 9, 0, 0, 0u, 0u, 8, 0u, 0, 3, 1, {0x0078,0x0079,0x007A}, 0u},
	{2, "nostaterecords seq0 seq1 seqbad", 10, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x8007}, 0u},
	{2, "nostaterecords stateindex keys", 16, 0, 0, 0u, 0u, 8, 0u, -50, 0, 0, {}, 0u},
	{2, "nostaterecords stateindex keys", 15, 0, 0, 0u, 0u, 8, 0u, -50, 0, 0, {}, 0u},
	{2, "nostaterecords stateindex keys", 6, 0, 0, 0u, 0u, 8, 0u, -50, 0, 0, {}, 0u},
	{3, "nosequences dead1 e", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{3, "nosequences dead1 e", 3, 0, 0, 0u, 0u, 8, 1u, 0, 1, 1, {0x00E9}, 65536u},
	{3, "nosequences dead1 a", 2, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x00B4}, 1u},
	{3, "nosequences dead1 a", 0, 0, 0, 0u, 0u, 8, 1u, 0, 2, 1, {0x00B4,0x0061}, 0u},
	{3, "nosequences seq0 seq1 seqbad", 8, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x8000}, 0u},
	{3, "nosequences seq0 seq1 seqbad", 9, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x8001}, 0u},
	{3, "nosequences seq0 seq1 seqbad", 10, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x8007}, 0u},
	{3, "nosequences stateindex keys", 16, 0, 0, 0u, 0u, 8, 0u, -50, 0, 0, {}, 0u},
	{3, "nosequences stateindex keys", 15, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x8000}, 0u},
	{3, "nosequences stateindex keys", 6, 0, 0, 0u, 0u, 8, 0u, 0, 1, 1, {0x006B}, 0u},
};

static int failures;

static int run_row(const row_t* r)
{
	const uint8_t* layouts[] = { g, g_noterm, g_nostate, g_noseq };
	UniChar buf[16];
	for (int i = 0; i < 16; i++)
		buf[i] = 0xAAAA;
	UniCharCount len = 0x77;
	UInt32 st = r->state_in;
	OSStatus rc = UCKeyTranslate((const UCKeyboardLayout*) layouts[r->variant], (UInt16) r->key, (UInt16) r->action,
			(UInt32) r->mods, r->type, r->options, &st, (UniCharCount) r->max, &len, buf);
	int bad = rc != r->rc || (int) len != r->length || (r->check_state && st != r->state_out);
	// A parameter error leaves the length and the buffer alone (the length keeps its 0x77).
	if (r->length == 0x77)
		bad = bad || buf[0] != 0xAAAA;
	for (int i = 0; r->length != 0x77 && i < r->length && i < 3 && !bad; i++)
		bad = buf[i] != r->chars[i];
	// Nothing is written past the returned length.
	for (int i = r->length < 16 ? r->length : 16; i < 16 && !bad; i++)
		bad = buf[i] != 0xAAAA;
	if (bad) {
		printf("FAIL %s: key %d action %d mods %d type %u options %u max %d state %u -> rc %d length %lu state %u,"
				" expected rc %d length %d state %u\n", r->name, r->key, r->action, r->mods, r->type, r->options, r->max,
				r->state_in, (int) rc, (unsigned long) len, (unsigned) st, r->rc, r->length, r->state_out);
		failures++;
	}
	return bad;
}

int main(void)
{
	build();
	for (unsigned i = 0; i < sizeof rows / sizeof rows[0]; i++)
		run_row(&rows[i]);

	// Missing pointers are parameter errors; the layout and output length stay untouched.
	{
		UniChar buf[4];
		UniCharCount len = 5;
		UInt32 st = 0;
		if (UCKeyTranslate(NULL, 0, 0, 0, 0, 0, &st, 4, &len, buf) != paramErr || len != 5) {
			printf("FAIL null layout\n");
			failures++;
		}
		len = 0x77;
		if (UCKeyTranslate((const UCKeyboardLayout*) g, K_A, 0, 0, 0, 0, NULL, 4, &len, buf) != paramErr || len != 0x77) {
			printf("FAIL null deadKeyState\n");
			failures++;
		}
		if (UCKeyTranslate((const UCKeyboardLayout*) g, K_A, 0, 0, 0, 0, &st, 4, NULL, buf) != paramErr) {
			printf("FAIL null length\n");
			failures++;
		}
	}

	if (failures == 0)
		printf("PASS %u translations\n", (unsigned) (sizeof rows / sizeof rows[0]));
	else
		printf("%d failure(s)\n", failures);
	return failures != 0;
}
