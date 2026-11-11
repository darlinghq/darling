// Copyright (C) 2026 Lubos Dolezel
//
// TextEncoding bit layout (base: 16 bits, variant: 10 bits from bit 16, format:
// 4 bits from bit 26), the Get* accessors, internet name lookup, script
// upgrade and a basic Text Encoding Converter round trip.
#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <string.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

static int failures;

#define CHECK(cond) do { \
	if (cond) printf("PASS: %s\n", #cond); \
	else { printf("FAIL: %s\n", #cond); failures++; } \
} while (0)

static OSStatus by_name(const char* name, TextEncoding* enc)
{
	unsigned char p[256];
	size_t n = strlen(name);
	p[0] = (unsigned char) n;
	memcpy(p + 1, name, n);
	return TECGetTextEncodingFromInternetName(enc, p);
}

static void test_layout(void)
{
	TextEncoding e = CreateTextEncoding(0x100, 0, 2);
	CHECK(e == 0x08000100);
	CHECK(GetTextEncodingBase(e) == 0x100);
	CHECK(GetTextEncodingVariant(e) == 0);
	CHECK(GetTextEncodingFormat(e) == 2);

	// the variant keeps 10 bits and the format 4 bits
	e = CreateTextEncoding(0x1234, 0x3ff, 0x3f);
	CHECK(e == 0x3fff1234);
	CHECK(GetTextEncodingBase(e) == 0x1234);
	CHECK(GetTextEncodingVariant(e) == 0x3ff);
	CHECK(GetTextEncodingFormat(e) == 0xf);
}

static void test_names(void)
{
	TextEncoding e = 0;
	CHECK(by_name("utf-8", &e) == noErr && e == 0x08000100);
	CHECK(by_name("UTF-8", &e) == noErr && e == 0x08000100);
	CHECK(by_name("utf-16", &e) == noErr && e == 0x100);
	CHECK(by_name("utf-16be", &e) == noErr && e == 0x10000100);
	CHECK(by_name("utf-16le", &e) == noErr && e == 0x14000100);
	CHECK(by_name("utf-32", &e) == noErr && e == 0x0c000100);
	CHECK(by_name("us-ascii", &e) == noErr && e == 0x600);
	CHECK(by_name("ascii", &e) == noErr && e == 0x600);
	CHECK(by_name("iso-8859-1", &e) == noErr && e == 0x500);
	CHECK(by_name("latin1", &e) == noErr && e == 0x500);
	CHECK(by_name("macintosh", &e) == noErr && e == 0x20000);
	CHECK(by_name("shift_jis", &e) == noErr && e == 0x10a01);

	// "utf8" is not a registered name
	e = 0;
	CHECK(by_name("utf8", &e) == kTextUnsupportedEncodingErr && e == 0xffff);
	e = 0;
	CHECK(by_name("bogus-name", &e) == kTextUnsupportedEncodingErr && e == 0xffff);
}

static void test_upgrade(void)
{
	static const struct { ScriptCode script; TextEncoding expected; } cases[] = {
		{ 0, 0x20000 }, { 1, 1 }, { 2, 2 }, { 3, 3 }, { 4, 4 }, { 5, 5 },
		{ 6, 0x20006 }, { 7, 0x30007 }, { 25, 0x19 }, { -2, 0x20000 }, { -1, 0x20000 },
	};
	for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++)
	{
		TextEncoding e = 0xdeadbeef;
		OSStatus st = UpgradeScriptInfoToTextEncoding(cases[i].script, kTextLanguageDontCare,
			kTextRegionDontCare, NULL, &e);
		CHECK(st == noErr && e == cases[i].expected);
	}

	TextEncoding e = 0;
	CHECK(UpgradeScriptInfoToTextEncoding(99, kTextLanguageDontCare, kTextRegionDontCare, NULL, &e) == paramErr);
}

static void test_convert(void)
{
	TextEncoding utf8, utf16, latin1;
	CHECK(by_name("utf-8", &utf8) == noErr);
	CHECK(by_name("utf-16", &utf16) == noErr);
	CHECK(by_name("latin1", &latin1) == noErr);

	TECObjectRef conv = NULL;
	CHECK(TECCreateConverter(&conv, latin1, utf8) == noErr && conv);
	if (conv)
	{
		// U+00E9 in Latin 1 is C3 A9 in UTF-8
		const unsigned char in[] = { 'h', 0xe9 };
		unsigned char out[16];
		ByteCount inUsed = 0, outUsed = 0;
		CHECK(TECConvertText(conv, in, sizeof(in), &inUsed, out, sizeof(out), &outUsed) == noErr);
		CHECK(inUsed == 2 && outUsed == 3 && memcmp(out, "h\xc3\xa9", 3) == 0);

		CHECK(TECClearConverterContextInfo(conv) == noErr);
		inUsed = outUsed = 0;
		CHECK(TECConvertText(conv, in, sizeof(in), &inUsed, out, sizeof(out), &outUsed) == noErr);
		CHECK(outUsed == 3 && memcmp(out, "h\xc3\xa9", 3) == 0);
		TECDisposeConverter(conv);
	}

	// UTF-16 here is the native-endian format; this test runs on little-endian CPUs.
	conv = NULL;
	CHECK(TECCreateConverter(&conv, utf8, utf16) == noErr && conv);
	if (conv)
	{
		const unsigned char in[] = { 'a', 'b', 'c', 'd' };
		unsigned char out[16];
		ByteCount inUsed = 0, outUsed = 0;
		CHECK(TECConvertText(conv, in, sizeof(in), &inUsed, out, sizeof(out), &outUsed) == noErr);
		CHECK(inUsed == 4 && outUsed == 8);
		CHECK(memcmp(out, "a\0b\0c\0d\0", 8) == 0);
		TECDisposeConverter(conv);
	}
}

int main(void)
{
	test_layout();
	test_names();
	test_upgrade();
	test_convert();
	return failures ? 1 : 0;
}
