// Copyright (C) 2026 Lubos Dolezel
//
// AEBuildDesc, AEBuildParameters and AEBuildAppleEvent: descriptors, records,
// lists and Apple Events described by a format string, with arguments
// substituted for @, and the error code and position reported for a bad
// format string. The program passes unchanged on macOS 26.
#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <string.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

#define BUILD_OK(desc, format, ...) do { AEBuildError e_; OSStatus s_ = AEBuildDesc(&(desc), &e_, format, ##__VA_ARGS__); \
	if (s_ != noErr) { printf("FAIL %d: %s -> %d (syntax %d at %u)\n", __LINE__, format, (int) s_, (int) e_.fError, (unsigned) e_.fErrorPos); g_failures++; } } while (0)

static SInt32 as_long(const AEDesc *d)
{
	SInt32 v = 0;
	AEGetDescData(d, &v, sizeof(v));
	return v;
}

static int bytes_equal(const AEDesc *d, const void *expected, Size size)
{
	char buffer[64];
	if (AEGetDescDataSize(d) != size || size > (Size) sizeof(buffer))
		return 0;
	AEGetDescData(d, buffer, size);
	return memcmp(buffer, expected, size) == 0;
}

static void expect_error(const char *format, AEBuildErrorCode code, UInt32 position)
{
	AEDesc d;
	AEBuildError e = {0, 0};
	OSStatus s = AEBuildDesc(&d, &e, format);
	if (s != errAEBuildSyntaxError || e.fError != code || (position != (UInt32) -1 && e.fErrorPos != position)) {
		printf("FAIL: \"%s\": status %d syntax error %d at %u, expected %d at %d\n", format, (int) s, (int) e.fError,
			(unsigned) e.fErrorPos, (int) code, (int) position);
		g_failures++;
	}
	if (s == noErr)
		AEDisposeDesc(&d);
}

static void test_scalars(void)
{
	AEDesc d;
	// An integer is 16 bits wide when it fits and 32 bits otherwise.
	BUILD_OK(d, "1234");
	CHECK(d.descriptorType == typeSInt16 && AEGetDescDataSize(&d) == 2);
	{
		SInt16 v = 0;
		AEGetDescData(&d, &v, sizeof(v));
		CHECK(v == 1234);
	}
	AEDisposeDesc(&d);
	BUILD_OK(d, "-5");
	CHECK(d.descriptorType == typeSInt16 && AEGetDescDataSize(&d) == 2);
	AEDisposeDesc(&d);
	BUILD_OK(d, "32767");
	CHECK(d.descriptorType == typeSInt16);
	AEDisposeDesc(&d);
	BUILD_OK(d, "32768");
	CHECK(d.descriptorType == typeSInt32 && as_long(&d) == 32768);
	AEDisposeDesc(&d);
	BUILD_OK(d, "-70000");
	CHECK(d.descriptorType == typeSInt32 && as_long(&d) == -70000);
	AEDisposeDesc(&d);
	BUILD_OK(d, "  \"hello\"  ");
	CHECK(d.descriptorType == typeChar && bytes_equal(&d, "hello", 5));
	AEDisposeDesc(&d);
	BUILD_OK(d, "\"a\\\"b\"");
	CHECK(d.descriptorType == typeChar && bytes_equal(&d, "a\"b", 3));
	AEDisposeDesc(&d);
	BUILD_OK(d, "\"\"");
	CHECK(d.descriptorType == typeChar && AEGetDescDataSize(&d) == 0);
	AEDisposeDesc(&d);

	// A bare four-character code is an enumerated descriptor, quoted or not; short codes are padded with spaces.
	BUILD_OK(d, "'abcd'");
	CHECK(d.descriptorType == typeEnumerated && as_long(&d) == 'abcd');
	AEDisposeDesc(&d);
	BUILD_OK(d, "'abc'");
	CHECK(d.descriptorType == typeEnumerated && as_long(&d) == 'abc ');
	AEDisposeDesc(&d);
	BUILD_OK(d, "abcd");
	CHECK(d.descriptorType == typeEnumerated && as_long(&d) == 'abcd');
	AEDisposeDesc(&d);

	BUILD_OK(d, "null()");
	CHECK(d.descriptorType == typeNull);
	AEDisposeDesc(&d);
}

static void test_typed_data(void)
{
	AEDesc d;
	BUILD_OK(d, "enum(prop)");
	CHECK(d.descriptorType == typeEnumerated && as_long(&d) == 'prop');
	AEDisposeDesc(&d);
	BUILD_OK(d, "type(comt)");
	CHECK(d.descriptorType == typeType && as_long(&d) == 'comt');
	AEDisposeDesc(&d);
	BUILD_OK(d, "long(5)");
	CHECK(d.descriptorType == typeSInt32 && AEGetDescDataSize(&d) == 4 && as_long(&d) == 5);
	AEDisposeDesc(&d);
	// An integer is coerced to the requested numeric type.
	BUILD_OK(d, "shor(-3)");
	{
		SInt16 v = 0;
		CHECK(d.descriptorType == typeSInt16 && AEGetDescDataSize(&d) == 2);
		AEGetDescData(&d, &v, sizeof(v));
		CHECK(v == -3);
	}
	AEDisposeDesc(&d);
	BUILD_OK(d, "bool(1)");
	CHECK(d.descriptorType == typeBoolean);
	AEDisposeDesc(&d);
	// Text and hex data are used as raw bytes of the type.
	BUILD_OK(d, "TEXT(\"abc\")");
	CHECK(d.descriptorType == typeChar && bytes_equal(&d, "abc", 3));
	AEDisposeDesc(&d);
	BUILD_OK(d, "utf8(\"abc\")");
	CHECK(d.descriptorType == typeUTF8Text && bytes_equal(&d, "abc", 3));
	AEDisposeDesc(&d);
	BUILD_OK(d, "'abcd'($0102AB$)");
	{
		const unsigned char raw[3] = {1, 2, 0xAB};
		CHECK(d.descriptorType == 'abcd' && bytes_equal(&d, raw, 3));
	}
	AEDisposeDesc(&d);
	// A four-character code inside the parentheses is its 32-bit value.
	BUILD_OK(d, "long(abcd)");
	CHECK(d.descriptorType == typeSInt32 && as_long(&d) == 'abcd');
	AEDisposeDesc(&d);
	// An integer that cannot be coerced to the type fails the build, not the syntax.
	{
		AEBuildError e = {1, 1};
		CHECK(AEBuildDesc(&d, &e, "ldt(1)") == errAECoercionFail);
		CHECK(e.fError == aeBuildSyntaxNoErr);
	}
}

static void test_substitution(void)
{
	AEDesc d;
	BUILD_OK(d, "long(@)", (long) -7);
	CHECK(d.descriptorType == typeSInt32 && as_long(&d) == -7);
	AEDisposeDesc(&d);
	BUILD_OK(d, "shor(@)", (int) 3);
	{
		SInt16 v = 0;
		AEGetDescData(&d, &v, sizeof(v));
		CHECK(d.descriptorType == typeSInt16 && v == 3);
	}
	AEDisposeDesc(&d);
	BUILD_OK(d, "comp(@)", (long long) 5000000000LL);
	{
		SInt64 v = 0;
		AEGetDescData(&d, &v, sizeof(v));
		CHECK(d.descriptorType == typeSInt64 && v == 5000000000LL);
	}
	AEDisposeDesc(&d);
	BUILD_OK(d, "doub(@)", 1.5);
	{
		double v = 0;
		AEGetDescData(&d, &v, sizeof(v));
		CHECK(d.descriptorType == typeIEEE64BitFloatingPoint && v == 1.5);
	}
	AEDisposeDesc(&d);
	BUILD_OK(d, "bool(@)", 1);
	CHECK(d.descriptorType == typeBoolean);
	AEDisposeDesc(&d);
	BUILD_OK(d, "TEXT(@)", "hi");
	CHECK(d.descriptorType == typeChar && bytes_equal(&d, "hi", 2));
	AEDisposeDesc(&d);
	BUILD_OK(d, "enum(@)", (unsigned int) 'abcd');
	CHECK(d.descriptorType == typeEnumerated && as_long(&d) == 'abcd');
	AEDisposeDesc(&d);
	// Any other type takes a length followed by a pointer.
	BUILD_OK(d, "'abcd'(@)", (long) 3, "xyz");
	CHECK(d.descriptorType == 'abcd' && bytes_equal(&d, "xyz", 3));
	AEDisposeDesc(&d);

	// A bare @ copies a descriptor from the arguments.
	AEDesc source;
	AECreateDesc(typeChar, "wow", 3, &source);
	BUILD_OK(d, "{ '----': @, data: @ }", &source, &source);
	{
		AEDesc item;
		CHECK(d.descriptorType == typeAERecord);
		CHECK(AEGetParamDesc(&d, '----', typeWildCard, &item) == noErr && item.descriptorType == typeChar &&
			bytes_equal(&item, "wow", 3));
		AEDisposeDesc(&item);
		CHECK(AEGetParamDesc(&d, 'data', typeWildCard, &item) == noErr && bytes_equal(&item, "wow", 3));
		AEDisposeDesc(&item);
	}
	AEDisposeDesc(&d);
	AEDisposeDesc(&source);
}

static void test_lists_and_records(void)
{
	AEDesc d, item;
	long count;
	BUILD_OK(d, "[ 1 , 2, \"x\", [ 3 ] ]");
	CHECK(d.descriptorType == typeAEList);
	CHECK(AECountItems(&d, &count) == noErr && count == 4);
	CHECK(AEGetNthDesc(&d, 3, typeWildCard, NULL, &item) == noErr && item.descriptorType == typeChar);
	AEDisposeDesc(&item);
	CHECK(AEGetNthDesc(&d, 4, typeWildCard, NULL, &item) == noErr && item.descriptorType == typeAEList);
	AEDisposeDesc(&item);
	AEDisposeDesc(&d);
	BUILD_OK(d, "[]");
	CHECK(d.descriptorType == typeAEList && AECountItems(&d, &count) == noErr && count == 0);
	AEDisposeDesc(&d);

	// Keys are four-character codes; shorter ones are padded and longer ones cut.
	BUILD_OK(d, "{abcd:1,efg:\"s\", 'a b ':3, hijkl:4}");
	CHECK(d.descriptorType == typeAERecord);
	CHECK(AEGetParamDesc(&d, 'abcd', typeWildCard, &item) == noErr && as_long(&item) == 1);
	AEDisposeDesc(&item);
	CHECK(AEGetParamDesc(&d, 'efg ', typeWildCard, &item) == noErr && item.descriptorType == typeChar);
	AEDisposeDesc(&item);
	CHECK(AEGetParamDesc(&d, 'a b ', typeWildCard, &item) == noErr);
	AEDisposeDesc(&item);
	CHECK(AEGetParamDesc(&d, 'hijk', typeWildCard, &item) == noErr);
	AEDisposeDesc(&item);
	AEDisposeDesc(&d);
	BUILD_OK(d, "{}");
	CHECK(d.descriptorType == typeAERecord && AECountItems(&d, &count) == noErr && count == 0);
	AEDisposeDesc(&d);

	// A type in front of the braces sets the descriptor type of the record.
	BUILD_OK(d, "'obj '{ form: enum(indx), want: type(file), seld: 1, from: null() }");
	CHECK(d.descriptorType == typeObjectSpecifier);
	CHECK(AEGetParamDesc(&d, 'form', typeWildCard, &item) == noErr && item.descriptorType == typeEnumerated &&
		as_long(&item) == 'indx');
	AEDisposeDesc(&item);
	CHECK(AEGetParamDesc(&d, 'from', typeWildCard, &item) == noErr && item.descriptorType == typeNull);
	AEDisposeDesc(&item);
	AEDisposeDesc(&d);
}

static void test_errors(void)
{
	expect_error("1 2", aeBuildSyntaxNoEOF, 2);
	expect_error("1.5", aeBuildSyntaxNoEOF, 1);
	expect_error("[1,", aeBuildSyntaxBadDesc, 3);
	expect_error("", aeBuildSyntaxBadDesc, 0);
	expect_error("{ x }", aeBuildSyntaxNoColon, 5);
	expect_error("{ a: 1, }", aeBuildSyntaxNoKey, (UInt32) -1);
	expect_error("{ a: 1 ", aeBuildSyntaxNoCloseBrace, (UInt32) -1);
	expect_error("[1 2]", aeBuildSyntaxNoCloseBracket, (UInt32) -1);
	expect_error("@@", aeBuildSyntaxUncoercedDoubleAt, 1);
	expect_error("$414243$", aeBuildSyntaxUncoercedHex, 8);
	expect_error("long($4$)", aeBuildSyntaxOddHex, (UInt32) -1);
	expect_error("doub(1.5)", aeBuildSyntaxNoCloseParen, (UInt32) -1);
	expect_error("'abcd'[1,2]", aeBuildSyntaxCoercedList, 7);
	expect_error("{ a: 1 ; b: 2 }", aeBuildSyntaxNoCloseBrace, (UInt32) -1);
}

static void test_apple_events(void)
{
	AEBuildError error;
	AEDesc event, item;
	AEDesc file, comment;
	AECreateDesc(typeChar, "f", 1, &file);
	AECreateDesc(typeChar, "c", 1, &comment);

	OSStatus status = AEBuildAppleEvent('core', 'setd', 'sign', "MACS", 4, kAutoGenerateReturnID, kAnyTransactionID,
		&event, &error,
		"'----': 'obj '{ form: enum(prop), seld: type(comt), want: type(prop), from: 'obj '{ form: enum(indx), "
		"want: type(file), seld: @, from: null() } }, data: @", &file, &comment);
	CHECK(status == noErr && error.fError == aeBuildSyntaxNoErr);
	CHECK(event.descriptorType == typeAppleEvent);
	{
		FourCharCode value = 0;
		CHECK(AEGetAttributePtr(&event, keyEventClassAttr, typeType, NULL, &value, sizeof(value), NULL) == noErr &&
			value == 'core');
		CHECK(AEGetAttributePtr(&event, keyEventIDAttr, typeType, NULL, &value, sizeof(value), NULL) == noErr &&
			value == 'setd');
		CHECK(AEGetAttributeDesc(&event, keyAddressAttr, typeWildCard, &item) == noErr && item.descriptorType == 'sign' &&
			bytes_equal(&item, "MACS", 4));
		AEDisposeDesc(&item);
	}
	CHECK(AEGetParamDesc(&event, 'data', typeWildCard, &item) == noErr && bytes_equal(&item, "c", 1));
	AEDisposeDesc(&item);
	CHECK(AEGetParamDesc(&event, keyDirectObject, typeWildCard, &item) == noErr && item.descriptorType == typeObjectSpecifier);
	{
		AEDesc from, seld;
		CHECK(AEGetParamDesc(&item, 'from', typeWildCard, &from) == noErr && from.descriptorType == typeObjectSpecifier);
		CHECK(AEGetParamDesc(&from, 'seld', typeWildCard, &seld) == noErr && bytes_equal(&seld, "f", 1));
		AEDisposeDesc(&seld);
		AEDisposeDesc(&from);
	}
	AEDisposeDesc(&item);

	// The parameters of an existing event can be extended.
	status = AEBuildParameters(&event, &error, "more: long(@), flag: bool(1)", (long) 9);
	CHECK(status == noErr);
	CHECK(AEGetParamDesc(&event, 'more', typeWildCard, &item) == noErr && as_long(&item) == 9);
	AEDisposeDesc(&item);
	AEDisposeDesc(&event);

	// No parameters, and simple ones.
	CHECK(AEBuildAppleEvent('aevt', 'oapp', typeNull, NULL, 0, kAutoGenerateReturnID, kAnyTransactionID, &event, &error,
		"") == noErr);
	CHECK(AEGetParamDesc(&event, keyDirectObject, typeWildCard, &item) != noErr);
	AEDisposeDesc(&event);
	CHECK(AEBuildAppleEvent('aevt', 'odoc', 'sign', "MACS", 4, kAutoGenerateReturnID, kAnyTransactionID, &event, &error,
		"'----': 5") == noErr);
	CHECK(AEGetParamDesc(&event, keyDirectObject, typeSInt32, &item) == noErr && as_long(&item) == 5);
	AEDisposeDesc(&item);
	AEDisposeDesc(&event);

	// The parameters of an event are a record's contents, not a bare descriptor, and a trailing comma is an error.
	AEInitializeDesc(&event);
	status = AEBuildAppleEvent('aevt', 'odoc', 'sign', "MACS", 4, kAutoGenerateReturnID, kAnyTransactionID, &event,
		&error, "1");
	CHECK(status == errAEBuildSyntaxError && error.fError == aeBuildSyntaxNoKey && error.fErrorPos == 1);
	status = AEBuildAppleEvent('aevt', 'odoc', 'sign', "MACS", 4, kAutoGenerateReturnID, kAnyTransactionID, &event,
		&error, "'----': 5, ");
	CHECK(status == errAEBuildSyntaxError && error.fError == aeBuildSyntaxNoKey && error.fErrorPos == 11);
	AEDisposeDesc(&file);
	AEDisposeDesc(&comment);
}

int main(void)
{
	test_scalars();
	test_typed_data();
	test_substitution();
	test_lists_and_records();
	test_errors();
	test_apple_events();
	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
