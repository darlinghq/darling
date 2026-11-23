// Native test: compile with the ICC layout code and run on the host.
//   cc -I src tests/ICCProfileTest.c src/ICCProfile.c -o ICCProfileTest
// Also run it with -fsanitize=address,undefined: the checks on the tag count and offsets guard reads past the end.
// The profiles are built by hand from the layout in ICC.1:2022, 7.2 and 7.3
// (https://www.color.org/specification/ICC.1-2022-05.pdf), so they contain no vendor data.
#include "ICCProfile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int g_failures;

static void expect(bool condition, const char* what)
{
	if (!condition)
	{
		printf("FAIL %s\n", what);
		g_failures++;
	}
}

static void put32(uint8_t* p, uint32_t v)
{
	p[0] = v >> 24;
	p[1] = v >> 16;
	p[2] = v >> 8;
	p[3] = v;
}

#define SIG(a, b, c, d) (((uint32_t) (a) << 24) | ((uint32_t) (b) << 16) | ((uint32_t) (c) << 8) | (uint32_t) (d))

// Two tags: 'wtpt' (4 bytes) and 'desc' (8 bytes), laid out right after the tag table.
static size_t buildProfile(uint8_t* out)
{
	const uint32_t tableEnd = ICC_HEADER_SIZE + 4 + 2 * 12;
	memset(out, 0, tableEnd + 12);
	put32(out, tableEnd + 12);
	put32(out + 36, SIG('a', 'c', 's', 'p'));
	put32(out + 128, 2);
	put32(out + 132, SIG('w', 't', 'p', 't'));
	put32(out + 136, tableEnd);
	put32(out + 140, 4);
	put32(out + 144, SIG('d', 'e', 's', 'c'));
	put32(out + 148, tableEnd + 4);
	put32(out + 152, 8);
	memcpy(out + tableEnd, "WTPT", 4);
	memcpy(out + tableEnd + 4, "DESCDESC", 8);
	return tableEnd + 12;
}

int main(void)
{
	uint8_t profile[256];
	const size_t length = buildProfile(profile);

	expect(ICCProfileValidate(profile, length), "well-formed profile is valid");
	expect(ICCProfileTagCount(profile) == 2, "tag count");

	ICCTag tag;
	expect(ICCProfileTagAt(profile, 0).signature == SIG('w', 't', 'p', 't'), "first tag signature");
	expect(ICCProfileFindTag(profile, SIG('d', 'e', 's', 'c'), &tag), "find desc");
	expect(tag.offset == 160 && tag.size == 8, "desc offset and size");
	expect(memcmp(profile + tag.offset, "DESCDESC", 8) == 0, "desc bytes");
	expect(!ICCProfileFindTag(profile, SIG('c', 'p', 'r', 't'), &tag), "missing tag");

	// A profile with no tags is valid.
	uint8_t empty[132] = { 0 };
	put32(empty, 132);
	put32(empty + 36, SIG('a', 'c', 's', 'p'));
	expect(ICCProfileValidate(empty, sizeof empty), "empty tag table");
	expect(ICCProfileTagCount(empty) == 0, "empty count");

	// Duplicate signatures are accepted; the last entry wins (QUIRK, see ICCProfileFindTag) and each signature is
	// listed once, in table order.
	uint8_t dup[256];
	memcpy(dup, profile, length);
	put32(dup + 144, SIG('w', 't', 'p', 't'));
	expect(ICCProfileValidate(dup, length), "duplicate signatures validate");
	expect(ICCProfileFindTag(dup, SIG('w', 't', 'p', 't'), &tag) && tag.offset == 160, "last duplicate wins");
	uint32_t indices[2], indexCount;
	expect(ICCProfileFirstTagIndices(dup, indices, &indexCount) && indexCount == 1 && indices[0] == 0, "duplicate listed once");
	expect(ICCProfileFirstTagIndices(profile, indices, &indexCount) && indexCount == 2 && indices[0] == 0 && indices[1] == 1, "distinct signatures all listed");

	// A hostile profile: the most zero-size tags that fit, all with one signature, at offset == length.
	// Validation and listing must stay close to linear (a quadratic scan would take tens of seconds here).
	const size_t hostileLength = 4u << 20;
	const uint32_t hostileCount = (uint32_t) ((hostileLength - ICC_HEADER_SIZE - 4) / 12);
	uint8_t* hostile = calloc(hostileLength, 1);
	put32(hostile, (uint32_t) hostileLength);
	put32(hostile + 36, SIG('a', 'c', 's', 'p'));
	put32(hostile + 128, hostileCount);
	for (uint32_t i = 0; i < hostileCount; i++)
	{
		uint8_t* entry = hostile + 132 + (size_t) i * 12;
		put32(entry, SIG('d', 'u', 'p', ' '));
		put32(entry + 4, (uint32_t) hostileLength);
	}
	uint32_t* hostileIndices = malloc((size_t) hostileCount * sizeof(uint32_t));
	const clock_t start = clock();
	expect(ICCProfileValidate(hostile, hostileLength), "hostile profile validates");
	expect(ICCProfileFirstTagIndices(hostile, hostileIndices, &indexCount) && indexCount == 1 && hostileIndices[0] == 0, "hostile profile lists one signature");
	expect(clock() - start < 2 * CLOCKS_PER_SEC, "hostile profile is handled in linear time");
	free(hostileIndices);
	free(hostile);

	// Invalid inputs.
	expect(!ICCProfileValidate(NULL, 0), "null data");
	expect(!ICCProfileValidate(profile, 131), "shorter than header and tag count");

	// Cut-off tag tables, in buffers of exactly the stated size so a read past the end is a sanitizer error.
	uint8_t* cut = malloc(132);
	memcpy(cut, profile, 132);
	put32(cut, 132);
	put32(cut + 128, 1);
	expect(!ICCProfileValidate(cut, 132), "tag count 1 with no room for the entry");
	free(cut);

	uint8_t bad[256];
	memcpy(bad, profile, length);
	bad[36] = 'x';
	expect(!ICCProfileValidate(bad, length), "wrong file signature");

	// A buffer of exactly the profile's size, so a read past the tag table is an address sanitizer error.
	uint8_t* exact = malloc(length);
	memcpy(exact, profile, length);
	put32(exact + 128, 0x20000000u);
	expect(!ICCProfileValidate(exact, length), "huge tag count");
	put32(exact + 128, 3);
	expect(!ICCProfileValidate(exact, length), "tag count one larger than the data holds");
	free(exact);

	memcpy(bad, profile, length);
	put32(bad + 152, 9);
	expect(!ICCProfileValidate(bad, length), "tag runs past the end");

	// One tag at the first legal offset (144) with a size whose sum with the offset wraps to 128 in 32 bits.
	uint8_t* wrap = malloc(148);
	memset(wrap, 0, 148);
	put32(wrap, 148);
	put32(wrap + 36, SIG('a', 'c', 's', 'p'));
	put32(wrap + 128, 1);
	put32(wrap + 132, SIG('w', 't', 'p', 't'));
	put32(wrap + 136, 144);
	put32(wrap + 140, 0xFFFFFFF0u);
	expect(!ICCProfileValidate(wrap, 148), "offset plus size wraps around 32 bits");
	free(wrap);

	memcpy(bad, profile, length);
	put32(bad + 148, 0xFFFFFFFCu);
	put32(bad + 152, 8);
	expect(!ICCProfileValidate(bad, length), "offset beyond the data");

	memcpy(bad, profile, length);
	put32(bad + 136, 130);
	expect(!ICCProfileValidate(bad, length), "tag data inside the tag table");

	memcpy(bad, profile, length);
	put32(bad, (uint32_t) length + 1);
	expect(ICCProfileValidate(bad, length), "header size larger than the data is accepted");
	put32(bad, 10);
	expect(ICCProfileValidate(bad, length), "header size smaller than the data is accepted");

	if (g_failures)
	{
		printf("ICCProfileTest: %d failures\n", g_failures);
		return 1;
	}
	printf("ICCProfileTest: OK\n");
	return 0;
}
