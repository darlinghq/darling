// CryptoTokenKit answer-to-reset parser. The parser is plain C, so this builds with any C compiler on any host:
// cc cryptotokenkit_atr_codec.c && ./a.out
// ATRs are hand-built from the layout of ISO/IEC 7816-3 clause 8 (TS, T0, TAi..TDi, historical octets, TCK = XOR of
// T0 through the last octet before TCK); each one is annotated with how it was derived. Results the standard and the
// documentation do not settle are the black-box observations of macOS 26.7.3 (marked "observed").
// Prints nothing on success; the exit code is the number of failed checks.
#include "../../src/frameworks/CryptoTokenKit/src/ctkatr.c"

#include <stdio.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)

struct feed { const uint8_t *bytes; size_t length, pos; };
static int pull(void *context)
{
	struct feed *f = context;
	return f->pos < f->length ? f->bytes[f->pos++] : -1;
}

static bool atr(const uint8_t *bytes, size_t length, ctk_atr *out, size_t *used)
{
	struct feed f = { bytes, length, 0 };
	bool ok = ctk_atr_parse(pull, &f, out);
	*used = f.pos;
	return ok;
}

static size_t protocols(const ctk_atr *a, uint8_t *out)
{
	return ctk_atr_protocols(a, out);
}

static void test_atr(void)
{
	ctk_atr a;
	size_t used;
	uint8_t p[16];

	// TS and T0 only: one (empty) group, protocol T=0. The value of TS is not checked (observed on macOS 26.7.3).
	const uint8_t minimal[] = { 0x3b, 0x00 };
	CHECK(atr(minimal, sizeof minimal, &a, &used) && a.length == 2 && a.group_count == 1 && a.historical_length == 0);
	CHECK(!a.groups[0].has_ta && !a.groups[0].has_td);
	CHECK(protocols(&a, p) == 1 && p[0] == 0);
	const uint8_t inverse[] = { 0x3f, 0x00 };
	const uint8_t odd_ts[] = { 0x3a, 0x00 };
	CHECK(atr(inverse, sizeof inverse, &a, &used) && atr(odd_ts, sizeof odd_ts, &a, &used));

	// T0 = 02: two historical octets, no interface bytes, no TCK.
	const uint8_t historical[] = { 0x3b, 0x02, 0xab, 0xcd };
	CHECK(atr(historical, sizeof historical, &a, &used) && a.historical_offset == 2 && a.historical_length == 2 && a.bytes[3] == 0xcd);

	// T0 = 91: TA1 and TD1 present, K = 1. TA1 = 11, TD1 = 01 (T=1, nothing follows), historical 42.
	// TCK = 91 ^ 11 ^ 01 ^ 42 = C3. A second, empty group follows TD1 and carries T=1.
	const uint8_t t1[] = { 0x3b, 0x91, 0x11, 0x01, 0x42, 0xc3 };
	CHECK(atr(t1, sizeof t1, &a, &used) && used == sizeof t1 && a.group_count == 2);
	CHECK(a.groups[0].has_ta && a.groups[0].ta == 0x11 && !a.groups[0].has_tb && !a.groups[0].has_tc);
	CHECK(a.groups[0].has_td && a.groups[0].protocol == 1 && !a.groups[1].has_ta && !a.groups[1].has_td);
	CHECK(a.historical_offset == 4 && a.historical_length == 1 && a.bytes[4] == 0x42);
	CHECK(protocols(&a, p) == 1 && p[0] == 1);
	// The value of TCK is not checked (observed), only that it is there.
	const uint8_t bad_tck[] = { 0x3b, 0x91, 0x11, 0x01, 0x42, 0xc4 };
	CHECK(atr(bad_tck, sizeof bad_tck, &a, &used));
	CHECK(!atr(t1, sizeof t1 - 1, &a, &used));

	// T0 = 80: TD1 only. TD1 = 80 (T=0, TD2 follows), TD2 = 01 (T=1). Three groups. T=1 comes first, then T=0.
	const uint8_t chain[] = { 0x3b, 0x80, 0x80, 0x01, 0x01 };
	CHECK(atr(chain, sizeof chain, &a, &used) && a.group_count == 3);
	CHECK(a.groups[0].has_td && a.groups[0].protocol == 0 && a.groups[1].has_td && a.groups[1].protocol == 1 && !a.groups[2].has_td);
	CHECK(!a.groups[0].has_ta && !a.groups[1].has_ta);
	CHECK(protocols(&a, p) == 2 && p[0] == 1 && p[1] == 0);

	// Protocol order: T=1 first, the others in the order of the TDi, each once, T=15 left out.
	const uint8_t order[] = { 0x3b, 0x80, 0x82, 0x80, 0x8f, 0x01, 0x00 }; // TD1 = T2 (+TD2), TD2 = T0 (+TD3), TD3 = T15 (+TD4), TD4 = T1
	CHECK(atr(order, sizeof order, &a, &used));
	CHECK(protocols(&a, p) == 3 && p[0] == 1 && p[1] == 2 && p[2] == 0);
	// Only T=15: the default T=0.
	const uint8_t only15[] = { 0x3b, 0x80, 0x0f, 0x00 };
	CHECK(atr(only15, sizeof only15, &a, &used) && protocols(&a, p) == 1 && p[0] == 0);

	// TA2 present (specific mode): its low nibble is the only protocol. T0 = 80, TD1 = 91 (T=1, TA2 and TD2 follow),
	// TA2 = 20, TD2 = 00, TCK = 31.
	const uint8_t specific[] = { 0x3b, 0x80, 0x91, 0x20, 0x00, 0x31 };
	CHECK(atr(specific, sizeof specific, &a, &used) && a.group_count == 3 && a.groups[1].has_ta && a.groups[1].ta == 0x20);
	CHECK(protocols(&a, p) == 1 && p[0] == 0);
	const uint8_t specific15[] = { 0x3b, 0x80, 0x91, 0x8f, 0x00, 0x3e };
	CHECK(atr(specific15, sizeof specific15, &a, &used) && protocols(&a, p) == 1 && p[0] == 15);

	// T0 = 60: TB1, TC1 and nothing else (no TD1): only T=0 is announced implicitly, so no TCK is needed.
	const uint8_t tbtc[] = { 0x3b, 0x60, 0x25, 0x03 };
	CHECK(atr(tbtc, sizeof tbtc, &a, &used) && a.group_count == 1 && a.groups[0].has_tb && a.groups[0].tb == 0x25 && a.groups[0].has_tc && a.groups[0].tc == 3 && !a.groups[0].has_td);

	// TD1 announcing T=0 only needs no TCK: T0 = 80, TD1 = 00.
	const uint8_t t0_only[] = { 0x3b, 0x80, 0x00 };
	CHECK(atr(t0_only, sizeof t0_only, &a, &used) && a.groups[0].protocol == 0 && a.group_count == 2);
	// T=15 counts as "other than T=0": TD1 = 0F needs a TCK octet (its value is not checked).
	const uint8_t t15[] = { 0x3b, 0x80, 0x0f, 0x8f };
	CHECK(atr(t15, sizeof t15, &a, &used) && a.groups[0].protocol == 15 && used == sizeof t15);
	CHECK(!atr(t15, sizeof t15 - 1, &a, &used));
	const uint8_t t2[] = { 0x3b, 0x80, 0x02 };
	CHECK(!atr(t2, sizeof t2, &a, &used));

	// The parser stops at the end of the ATR and leaves the rest of the stream.
	const uint8_t trailing[] = { 0x3b, 0x00, 0xde, 0xad };
	CHECK(atr(trailing, sizeof trailing, &a, &used) && used == 2);

	// A source value above 255 keeps its low eight bits; a negative one is the end of the data.
	struct feed wide = { (const uint8_t[]){ 0x3b, 0x00 }, 2, 0 };
	CHECK(ctk_atr_parse(pull, &wide, &a));
	CHECK(!atr((const uint8_t[]){ 0x3b, 0x01 }, 2, &a, &used));

	// Failures: a source that is dry at every cut of a valid ATR, nothing at all.
	for (size_t cut = 0; cut < sizeof t1; cut++)
		CHECK(!atr(t1, cut, &a, &used));

	// An ATR longer than the 33 octets of ISO/IEC 7816-3 is read as far as it is well formed (observed): TS, T0 = 8F,
	// 40 TDi each announcing T=0 and another TD, 15 historical octets.
	uint8_t longer[2 + 41 + 15];
	longer[0] = 0x3b;
	longer[1] = 0x8f;
	for (size_t i = 2; i < 2 + 40; i++)
		longer[i] = 0x80;
	longer[2 + 40] = 0x00;
	for (size_t i = 2 + 41; i < sizeof longer; i++)
		longer[i] = (uint8_t)i;
	CHECK(atr(longer, sizeof longer, &a, &used) && a.length == sizeof longer && a.group_count == 42 && a.historical_length == 15);
	CHECK(protocols(&a, p) == 1 && p[0] == 0);

	// A source that never ends (TD after TD, all T=0) stops at the bound instead of running on: the bound is
	// reached before the group array, which has as many entries as the bound has octets.
	uint8_t endless[CTK_ATR_MAX_LENGTH + 8];
	endless[0] = 0x3b;
	for (size_t i = 1; i < sizeof endless; i++)
		endless[i] = 0x80;
	CHECK(!atr(endless, sizeof endless, &a, &used) && used == CTK_ATR_MAX_LENGTH);
	// An ATR of exactly the bound is accepted: TS, T0 = 80, TD1..TD(n-1) = 80, TDn = 00.
	uint8_t at_bound[CTK_ATR_MAX_LENGTH];
	at_bound[0] = 0x3b;
	for (size_t i = 1; i < sizeof at_bound - 1; i++)
		at_bound[i] = 0x80;
	at_bound[sizeof at_bound - 1] = 0x00;
	CHECK(atr(at_bound, sizeof at_bound, &a, &used) && a.length == CTK_ATR_MAX_LENGTH && a.group_count == CTK_ATR_MAX_LENGTH - 1);
}

// Random ATRs: every octet a source hands out is recorded in the parsed ATR, and a successful parse describes
// the interface groups and historical octets that lie inside it.
static void test_random_atrs(void)
{
	uint32_t state = 0x1234567u;
	for (int round = 0; round < 20000; round++) {
		uint8_t bytes[80];
		size_t length = state % 81;
		for (size_t i = 0; i < length; i++) {
			state ^= state << 13;
			state ^= state >> 17;
			state ^= state << 5;
			// The high nibbles that announce interface bytes are common so that groups get chained.
			bytes[i] = (state & 4) ? (uint8_t)(0x80 | (state >> 8)) : (uint8_t)(state >> 8);
		}
		bytes[0] = 0x3b;
		ctk_atr a;
		size_t used;
		if (atr(bytes, length, &a, &used)) {
			uint8_t p[16];
			CHECK(a.length == used && a.length >= 2 && a.length <= length);
			CHECK(a.historical_offset + a.historical_length <= a.length && a.group_count >= 1 && a.group_count <= a.length);
			size_t count = ctk_atr_protocols(&a, p);
			CHECK(count >= 1 && count <= 16);
			for (size_t i = 0; i < count; i++)
				CHECK(p[i] < 16);
		}
	}
}

int main(void)
{
	test_random_atrs();
	test_atr();
	return failures;
}
