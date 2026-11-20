// CryptoTokenKit answer-to-reset parser. The parser is plain C, so this builds with any C compiler on any host:
// cc cryptotokenkit_atr_codec.c && ./a.out
// ATRs are hand-built from the layout of ISO/IEC 7816-3 clause 8 (TS, T0, TAi..TDi, historical octets, TCK = XOR of
// T0 through the last octet before TCK); each one is annotated with how it was derived.
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

static void test_atr(void)
{
	ctk_atr a;
	size_t used;

	// TS and T0 only.
	const uint8_t minimal[] = { 0x3b, 0x00 };
	CHECK(atr(minimal, sizeof minimal, &a, &used) && a.length == 2 && a.group_count == 0 && a.historical_length == 0);
	// Inverse convention TS.
	const uint8_t inverse[] = { 0x3f, 0x00 };
	CHECK(atr(inverse, sizeof inverse, &a, &used));

	// T0 = 02: two historical octets, no interface bytes, no TCK.
	const uint8_t historical[] = { 0x3b, 0x02, 0xab, 0xcd };
	CHECK(atr(historical, sizeof historical, &a, &used) && a.historical_offset == 2 && a.historical_length == 2 && a.bytes[3] == 0xcd);

	// T0 = 91: TA1 and TD1 present, K = 1. TA1 = 11, TD1 = 01 (T=1, nothing follows), historical 42.
	// TCK = 91 ^ 11 ^ 01 ^ 42 = C3.
	const uint8_t t1[] = { 0x3b, 0x91, 0x11, 0x01, 0x42, 0xc3 };
	CHECK(atr(t1, sizeof t1, &a, &used) && used == sizeof t1 && a.group_count == 1);
	CHECK(a.groups[0].has_ta && a.groups[0].ta == 0x11 && !a.groups[0].has_tb && !a.groups[0].has_tc);
	CHECK(a.groups[0].has_td && a.groups[0].protocol == 1);
	CHECK(a.historical_offset == 4 && a.historical_length == 1 && a.bytes[4] == 0x42);
	// Wrong TCK.
	const uint8_t bad_tck[] = { 0x3b, 0x91, 0x11, 0x01, 0x42, 0xc4 };
	CHECK(!atr(bad_tck, sizeof bad_tck, &a, &used));
	// TCK missing.
	CHECK(!atr(t1, sizeof t1 - 1, &a, &used));

	// T0 = 80: TD1 only. TD1 = 80 (T=0, TD2 follows), TD2 = 01 (T=1). TCK = 80 ^ 80 ^ 01 = 01.
	const uint8_t chain[] = { 0x3b, 0x80, 0x80, 0x01, 0x01 };
	CHECK(atr(chain, sizeof chain, &a, &used) && a.group_count == 2);
	CHECK(a.groups[0].has_td && a.groups[0].protocol == 0 && a.groups[1].has_td && a.groups[1].protocol == 1);
	CHECK(!a.groups[0].has_ta && !a.groups[1].has_ta);

	// T0 = 60: TB1, TC1 and nothing else (no TD1): only T=0 is announced implicitly, so no TCK is needed.
	const uint8_t tbtc[] = { 0x3b, 0x60, 0x25, 0x03 };
	CHECK(atr(tbtc, sizeof tbtc, &a, &used) && a.groups[0].has_tb && a.groups[0].tb == 0x25 && a.groups[0].has_tc && a.groups[0].tc == 3 && !a.groups[0].has_td);

	// TD1 announcing T=0 only needs no TCK: T0 = 80, TD1 = 00.
	const uint8_t t0_only[] = { 0x3b, 0x80, 0x00 };
	CHECK(atr(t0_only, sizeof t0_only, &a, &used) && a.groups[0].protocol == 0);
	// T=15 counts as "other than T=0": TD1 = 0F needs TCK = 80 ^ 0F = 8F.
	const uint8_t t15[] = { 0x3b, 0x80, 0x0f, 0x8f };
	CHECK(atr(t15, sizeof t15, &a, &used) && a.groups[0].protocol == 15 && used == sizeof t15);
	// Without its TCK the T=15 ATR is cut short, so TCK is required for it too.
	CHECK(!atr(t15, sizeof t15 - 1, &a, &used));
	const uint8_t t15_bad[] = { 0x3b, 0x80, 0x0f, 0x8e };
	CHECK(!atr(t15_bad, sizeof t15_bad, &a, &used));

	// The parser stops at the end of the ATR and leaves the rest of the stream.
	const uint8_t trailing[] = { 0x3b, 0x00, 0xde, 0xad };
	CHECK(atr(trailing, sizeof trailing, &a, &used) && used == 2);

	// Failures: bad TS, source dry at every cut of a valid ATR, nothing at all.
	const uint8_t bad_ts[] = { 0x3a, 0x00 };
	CHECK(!atr(bad_ts, sizeof bad_ts, &a, &used));
	for (size_t cut = 0; cut < sizeof t1; cut++)
		CHECK(!atr(t1, cut, &a, &used));

	// 7816-3: an ATR has at most 33 octets. T0 = 8F then a chain of TDi with T=0 never ends the ATR early.
	uint8_t endless[40];
	endless[0] = 0x3b;
	endless[1] = 0x80;
	for (size_t i = 2; i < sizeof endless; i++)
		endless[i] = 0x80;
	CHECK(!atr(endless, sizeof endless, &a, &used));
	// Exactly 33 octets is fine: TS, T0 = 8F... build TS + T0 + 15 historical + ... = use TD chain of 16 T=0 groups.
	uint8_t longest[33];
	longest[0] = 0x3b;
	longest[1] = 0x8f; // TD1 follows, 15 historical octets
	for (size_t i = 2; i < 17; i++)
		longest[i] = 0x80; // TD1..TD15, each announcing T=0 and another TD
	longest[17] = 0x00; // TD16 ends the chain with T=0
	for (size_t i = 18; i < 33; i++)
		longest[i] = (uint8_t)i;
	CHECK(atr(longest, sizeof longest, &a, &used) && a.length == 33 && a.group_count == 16 && a.historical_length == 15);
	// One octet more is a well-formed 34-octet ATR (17 TDi, 15 historical octets) that is over the limit.
	uint8_t too_long[34];
	too_long[0] = 0x3b;
	too_long[1] = 0x8f;
	for (size_t i = 2; i < 18; i++)
		too_long[i] = 0x80;
	too_long[18] = 0x00;
	for (size_t i = 19; i < 34; i++)
		too_long[i] = (uint8_t)i;
	CHECK(!atr(too_long, sizeof too_long, &a, &used));
}

int main(void)
{
	test_atr();
	return failures;
}
