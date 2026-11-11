// Copyright (C) 2026 Lubos Dolezel
//
// Apple Event Descriptor Manager: descriptor creation, lists, records, Apple
// Event attributes and parameters, built-in coercions with range checks, and
// the AEPutDesc index rules.
// The program passes unchanged on macOS 27.0.1.
#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <string.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_ERR(expr, expected) do { OSErr e_ = (expr); if (e_ != (expected)) { printf("FAIL %d: %s returned %d, expected %d\n", __LINE__, #expr, (int) e_, (int) (expected)); g_failures++; } } while (0)

static OSErr coerce(DescType from, const void* data, Size size, DescType to, void* out, Size outSize)
{
	AEDesc src, dst;
	OSErr err = AECreateDesc(from, data, size, &src);
	if (err != noErr)
		return err;
	err = AECoerceDesc(&src, to, &dst);
	if (err == noErr) {
		CHECK(dst.descriptorType == to);
		CHECK(AEGetDescDataSize(&dst) == outSize);
		AEGetDescData(&dst, out, outSize);
		AEDisposeDesc(&dst);
	}
	AEDisposeDesc(&src);
	return err;
}

static void test_plain_descriptor(void)
{
	AEDesc desc;
	const char text[] = "hello";
	char buffer[16];

	CHECK_ERR(AECreateDesc(typeUTF8Text, text, 5, &desc), noErr);
	CHECK(desc.descriptorType == typeUTF8Text);
	CHECK(AEGetDescDataSize(&desc) == 5);
	memset(buffer, 0, sizeof(buffer));
	CHECK_ERR(AEGetDescData(&desc, buffer, sizeof(buffer)), noErr);
	CHECK(memcmp(buffer, "hello", 5) == 0);

	AEDesc copy;
	CHECK_ERR(AEDuplicateDesc(&desc, &copy), noErr);
	CHECK(copy.descriptorType == typeUTF8Text);
	CHECK(AEGetDescDataSize(&copy) == 5);
	AEDisposeDesc(&desc);
	CHECK(desc.descriptorType == typeNull);
	CHECK(desc.dataHandle == NULL);

	memset(buffer, 0, sizeof(buffer));
	CHECK_ERR(AEGetDescData(&copy, buffer, sizeof(buffer)), noErr);
	CHECK(memcmp(buffer, "hello", 5) == 0);
	CHECK_ERR(AEReplaceDescData(typeSInt32, &(SInt32){7}, sizeof(SInt32), &copy), noErr);
	CHECK(copy.descriptorType == typeSInt32);
	AEDisposeDesc(&copy);

	CHECK_ERR(AECreateDesc(typeNull, NULL, 0, &desc), noErr);
	CHECK(AEGetDescDataSize(&desc) == 0);
	AEDisposeDesc(&desc);
}

static void test_coercions(void)
{
	SInt32 i32;
	SInt16 i16;
	UInt16 u16;
	double d;
	Boolean b;

	SInt32 big = 70000;
	CHECK_ERR(coerce(typeSInt32, &big, sizeof(big), typeSInt16, &i16, sizeof(i16)), errAECoercionFail);
	SInt32 small = -5;
	CHECK_ERR(coerce(typeSInt32, &small, sizeof(small), typeSInt16, &i16, sizeof(i16)), noErr);
	CHECK(i16 == -5);
	CHECK_ERR(coerce(typeSInt32, &small, sizeof(small), typeUInt16, &u16, sizeof(u16)), errAECoercionFail);
	SInt32 two = 2;
	CHECK_ERR(coerce(typeSInt32, &two, sizeof(two), typeBoolean, &b, sizeof(b)), errAECoercionFail);
	SInt32 one = 1;
	CHECK_ERR(coerce(typeSInt32, &one, sizeof(one), typeBoolean, &b, sizeof(b)), noErr);
	CHECK(b == 1);

	// Fractions round to nearest, ties to even; values out of the target range fail.
	double fraction = 2.5;
	CHECK_ERR(coerce(typeIEEE64BitFloatingPoint, &fraction, sizeof(fraction), typeSInt32, &i32, sizeof(i32)), noErr);
	CHECK(i32 == 2);
	fraction = 3.5;
	CHECK_ERR(coerce(typeIEEE64BitFloatingPoint, &fraction, sizeof(fraction), typeSInt32, &i32, sizeof(i32)), noErr);
	CHECK(i32 == 4);
	fraction = -2.5;
	CHECK_ERR(coerce(typeIEEE64BitFloatingPoint, &fraction, sizeof(fraction), typeSInt32, &i32, sizeof(i32)), noErr);
	CHECK(i32 == -2);
	fraction = 1e12;
	CHECK_ERR(coerce(typeIEEE64BitFloatingPoint, &fraction, sizeof(fraction), typeSInt32, &i32, sizeof(i32)), errAECoercionFail);
	fraction = 0.5;
	CHECK_ERR(coerce(typeIEEE64BitFloatingPoint, &fraction, sizeof(fraction), typeBoolean, &b, sizeof(b)), errAECoercionFail);
	double whole = 42.0;
	CHECK_ERR(coerce(typeIEEE64BitFloatingPoint, &whole, sizeof(whole), typeSInt32, &i32, sizeof(i32)), noErr);
	CHECK(i32 == 42);
	SInt32 seven = 7;
	CHECK_ERR(coerce(typeSInt32, &seven, sizeof(seven), typeIEEE64BitFloatingPoint, &d, sizeof(d)), noErr);
	CHECK(d == 7.0);

	// Coercing to the same type or to a wildcard keeps the descriptor.
	AEDesc desc, out;
	CHECK_ERR(AECreateDesc(typeSInt32, &seven, sizeof(seven), &desc), noErr);
	CHECK_ERR(AECoerceDesc(&desc, typeWildCard, &out), noErr);
	CHECK(out.descriptorType == typeSInt32);
	AEDisposeDesc(&out);
	AEDisposeDesc(&desc);

	// Text conversions between UTF-8 and UTF-16.
	UniChar utf16[2] = { 'h', 'i' };
	char utf8[4];
	CHECK_ERR(coerce(typeUnicodeText, utf16, sizeof(utf16), typeUTF8Text, utf8, 2), noErr);
	CHECK(memcmp(utf8, "hi", 2) == 0);
	UniChar back[2];
	CHECK_ERR(coerce(typeUTF8Text, "hi", 2, typeUnicodeText, back, sizeof(back)), noErr);
	CHECK(back[0] == 'h' && back[1] == 'i');

	// Unrelated families do not coerce.
	CHECK_ERR(coerce(typeUTF8Text, "hi", 2, typeSInt32, &i32, sizeof(i32)), errAECoercionFail);

	// AECoercePtr.
	AEDesc viaPtr;
	CHECK_ERR(AECoercePtr(typeSInt32, &seven, sizeof(seven), typeSInt16, &viaPtr), noErr);
	CHECK(viaPtr.descriptorType == typeSInt16);
	AEDisposeDesc(&viaPtr);
}

static void test_list_and_index_rules(void)
{
	AEDescList list;
	long count = -1;
	SInt32 value;
	DescType type;
	Size actual;

	CHECK_ERR(AECreateList(NULL, 0, false, &list), noErr);
	CHECK_ERR(AECountItems(&list, &count), noErr);
	CHECK(count == 0);

	value = 10;
	CHECK_ERR(AEPutPtr(&list, 1, typeSInt32, &value, sizeof(value)), noErr);
	value = 20;
	CHECK_ERR(AEPutPtr(&list, 2, typeSInt32, &value, sizeof(value)), noErr);
	value = 30;
	CHECK_ERR(AEPutPtr(&list, 0, typeSInt32, &value, sizeof(value)), noErr); // 0 appends
	CHECK_ERR(AECountItems(&list, &count), noErr);
	CHECK(count == 3);

	// Replacing an existing item does not change the count.
	value = 25;
	CHECK_ERR(AEPutPtr(&list, 2, typeSInt32, &value, sizeof(value)), noErr);
	CHECK_ERR(AECountItems(&list, &count), noErr);
	CHECK(count == 3);
	CHECK_ERR(AEGetNthPtr(&list, 2, typeSInt32, NULL, &type, &value, sizeof(value), &actual), noErr);
	CHECK(value == 25 && type == typeSInt32 && actual == sizeof(value));

	// count + 1 appends too.
	value = 40;
	CHECK_ERR(AEPutPtr(&list, 4, typeSInt32, &value, sizeof(value)), noErr);
	CHECK_ERR(AECountItems(&list, &count), noErr);
	CHECK(count == 4);
	CHECK_ERR(AEDeleteItem(&list, 4), noErr);

	// Reading is 1-based and checked.
	CHECK_ERR(AEGetNthPtr(&list, 0, typeSInt32, NULL, &type, &value, sizeof(value), &actual), errAEDescNotFound);
	CHECK_ERR(AEGetNthPtr(&list, 4, typeSInt32, NULL, &type, &value, sizeof(value), &actual), errAEDescNotFound);
	CHECK_ERR(AEDeleteItem(&list, 4), errAEDescNotFound);

	// Items coerce on the way out.
	SInt16 narrow;
	CHECK_ERR(AEGetNthPtr(&list, 1, typeSInt16, NULL, &type, &narrow, sizeof(narrow), &actual), noErr);
	CHECK(narrow == 10 && type == typeSInt16);

	CHECK_ERR(AEDeleteItem(&list, 1), noErr);
	CHECK_ERR(AECountItems(&list, &count), noErr);
	CHECK(count == 2);
	CHECK_ERR(AEGetNthPtr(&list, 1, typeSInt32, NULL, &type, &value, sizeof(value), &actual), noErr);
	CHECK(value == 25);

	// A list is not a record: keyed access is refused.
	AEDesc out;
	CHECK_ERR(AEGetParamDesc(&list, 'abcd', typeWildCard, &out), errAENotAEDesc);
	AEDisposeDesc(&list);

	// A plain descriptor is not a list.
	AEDesc plain;
	CHECK_ERR(AECreateDesc(typeSInt32, &value, sizeof(value), &plain), noErr);
	CHECK_ERR(AECountItems(&plain, &count), errAEWrongDataType);
	AEDisposeDesc(&plain);
}

static void test_record_and_event(void)
{
	AERecord record;
	SInt32 value = 3;
	SInt32 got = 0;
	DescType type;
	Size size;

	CHECK_ERR(AECreateList(NULL, 0, true, &record), noErr);
	CHECK(record.descriptorType == typeAERecord);
	CHECK_ERR(AEPutPtr(&record, 1, typeSInt32, &value, sizeof(value)), errAEWrongDataType); // by index: lists only
	CHECK_ERR(AEPutParamPtr(&record, 'key1', typeSInt32, &value, sizeof(value)), noErr);
	CHECK_ERR(AESizeOfParam(&record, 'key1', &type, &size), noErr);
	CHECK(type == typeSInt32 && size == sizeof(SInt32));
	value = 4;
	CHECK_ERR(AEPutParamPtr(&record, 'key1', typeSInt32, &value, sizeof(value)), noErr); // replaces
	CHECK_ERR(AEGetParamPtr(&record, 'key1', typeSInt32, &type, &got, sizeof(got), &size), noErr);
	CHECK(got == 4);
	CHECK_ERR(AEGetParamPtr(&record, 'nope', typeSInt32, &type, &got, sizeof(got), &size), errAEDescNotFound);
	long count;
	CHECK_ERR(AECountItems(&record, &count), noErr);
	CHECK(count == 1);
	CHECK_ERR(AEDeleteParam(&record, 'key1'), noErr);
	CHECK_ERR(AEDeleteParam(&record, 'key1'), errAEDescNotFound);
	AEDisposeDesc(&record);

	AEDesc target;
	CHECK_ERR(AECreateDesc(typeApplicationBundleID, "com.example.app", 15, &target), noErr);
	AppleEvent event;
	CHECK_ERR(AECreateAppleEvent('test', 'evnt', &target, kAutoGenerateReturnID, kAnyTransactionID, &event), noErr);
	CHECK(event.descriptorType == typeAppleEvent);

	AEEventClass cls = 0;
	AEEventID id = 0;
	CHECK_ERR(AEGetAttributePtr(&event, keyEventClassAttr, typeType, &type, &cls, sizeof(cls), &size), noErr);
	CHECK(cls == 'test');
	CHECK_ERR(AEGetAttributePtr(&event, keyEventIDAttr, typeType, &type, &id, sizeof(id), &size), noErr);
	CHECK(id == 'evnt');

	SInt32 direct = 99;
	CHECK_ERR(AEPutParamPtr(&event, keyDirectObject, typeSInt32, &direct, sizeof(direct)), noErr);
	got = 0;
	CHECK_ERR(AEGetParamPtr(&event, keyDirectObject, typeSInt32, &type, &got, sizeof(got), &size), noErr);
	CHECK(got == 99);

	AEDesc addr;
	CHECK_ERR(AEGetAttributeDesc(&event, keyAddressAttr, typeWildCard, &addr), noErr);
	CHECK(addr.descriptorType == typeApplicationBundleID);
	AEDisposeDesc(&addr);
	CHECK_ERR(AEGetAttributeDesc(&event, 'zzzz', typeWildCard, &addr), errAEDescNotFound);

	AEDisposeDesc(&event);
	AEDisposeDesc(&target);
}

static void test_print(void)
{
	AEDesc desc;
	SInt32 value = 5;
	CHECK_ERR(AECreateDesc(typeSInt32, &value, sizeof(value), &desc), noErr);
	Handle handle = NULL;
	CHECK(AEPrintDescToHandle(&desc, &handle) == noErr);
	CHECK(handle != NULL);
	AEDisposeDesc(&desc);
}

int main(void)
{
	test_plain_descriptor();
	test_coercions();
	test_list_and_index_rules();
	test_record_and_event();
	test_print();
	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
