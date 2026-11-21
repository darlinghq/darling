// Copyright (C) 2026 Lubos Dolezel
//
// AEStream: descriptors, lists, records and Apple Events built by appending
// to a stream. The results are read back with the Descriptor Manager calls
// (types, data, item counts, keyed parameters, attributes), and the nesting
// errors are checked for the misuse that does not crash macOS.
// The program passes unchanged on macOS 26.7.3.
#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <string.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_ERR(expr, expected) do { OSStatus e_ = (expr); if (e_ != (expected)) { printf("FAIL %d: %s returned %d, expected %d\n", __LINE__, #expr, (int) e_, (int) (expected)); g_failures++; } } while (0)

static Boolean paramIs(const AEDesc* record, AEKeyword key, DescType type, const char* text)
{
	char buffer[64];
	DescType actualType = 0;
	Size actualSize = 0;
	if (AEGetParamPtr(record, key, typeWildCard, &actualType, buffer, sizeof buffer, &actualSize) != noErr)
		return false;
	return actualType == type && actualSize == (Size) strlen(text) && memcmp(buffer, text, strlen(text)) == 0;
}

static Boolean itemIs(const AEDesc* list, long index, DescType type, const void* data, Size size)
{
	char buffer[64];
	AEKeyword key = 0;
	DescType actualType = 0;
	Size actualSize = 0;
	if (AEGetNthPtr(list, index, typeWildCard, &key, &actualType, buffer, sizeof buffer, &actualSize) != noErr)
		return false;
	return actualType == type && actualSize == size && memcmp(buffer, data, size) == 0;
}

static long countOf(const AEDesc* desc)
{
	long count = -1;
	if (AECountItems(desc, &count) != noErr)
		return -1;
	return count;
}

static void test_single_descriptor(void)
{
	AEDesc desc;
	AEStreamRef stream = AEStreamOpen();
	CHECK(stream != NULL);
	CHECK_ERR(AEStreamWriteDesc(stream, typeChar, "hello", 5), noErr);
	CHECK_ERR(AEStreamClose(stream, &desc), noErr);
	CHECK(desc.descriptorType == typeChar);
	CHECK(AEGetDescDataSize(&desc) == 5);
	AEDisposeDesc(&desc);

	// The first value written at the top level is the result.
	stream = AEStreamOpen();
	CHECK_ERR(AEStreamWriteDesc(stream, typeChar, "a", 1), noErr);
	CHECK_ERR(AEStreamWriteDesc(stream, typeChar, "b", 1), noErr);
	CHECK_ERR(AEStreamClose(stream, &desc), noErr);
	char byte = 0;
	CHECK_ERR(AEGetDescData(&desc, &byte, 1), noErr);
	CHECK(byte == 'a');
	AEDisposeDesc(&desc);

	// A descriptor whose data is appended in pieces.
	stream = AEStreamOpen();
	CHECK_ERR(AEStreamOpenDesc(stream, typeChar), noErr);
	CHECK_ERR(AEStreamWriteData(stream, "ab", 2), noErr);
	CHECK_ERR(AEStreamWriteData(stream, "cde", 3), noErr);
	CHECK_ERR(AEStreamCloseDesc(stream), noErr);
	CHECK_ERR(AEStreamClose(stream, &desc), noErr);
	char text[8] = {0};
	CHECK(desc.descriptorType == typeChar && AEGetDescDataSize(&desc) == 5);
	CHECK_ERR(AEGetDescData(&desc, text, 5), noErr);
	CHECK(strcmp(text, "abcde") == 0);
	AEDisposeDesc(&desc);
}

static void test_list(void)
{
	AEDesc list;
	AEStreamRef stream = AEStreamOpen();
	const SInt32 number = 1;
	CHECK_ERR(AEStreamOpenList(stream), noErr);
	CHECK_ERR(AEStreamWriteDesc(stream, typeChar, "a", 1), noErr);
	CHECK_ERR(AEStreamOpenDesc(stream, typeSInt32), noErr);
	CHECK_ERR(AEStreamWriteData(stream, &number, 2), noErr);
	CHECK_ERR(AEStreamWriteData(stream, ((const char*) &number) + 2, 2), noErr);
	CHECK_ERR(AEStreamCloseDesc(stream), noErr);
	CHECK_ERR(AEStreamCloseList(stream), noErr);
	CHECK_ERR(AEStreamClose(stream, &list), noErr);
	CHECK(list.descriptorType == typeAEList);
	CHECK(countOf(&list) == 2);
	CHECK(itemIs(&list, 1, typeChar, "a", 1));
	CHECK(itemIs(&list, 2, typeSInt32, &number, 4));
	AEDisposeDesc(&list);

	// A list still open when the stream is closed is finished by the close.
	stream = AEStreamOpen();
	CHECK_ERR(AEStreamOpenList(stream), noErr);
	CHECK_ERR(AEStreamWriteDesc(stream, typeChar, "q", 1), noErr);
	CHECK_ERR(AEStreamClose(stream, &list), noErr);
	CHECK(list.descriptorType == typeAEList && countOf(&list) == 1);
	AEDisposeDesc(&list);

	// Whole descriptors are written as they are.
	AEDesc text;
	CHECK_ERR(AECreateDesc(typeChar, "wd", 2, &text), noErr);
	stream = AEStreamOpen();
	CHECK_ERR(AEStreamOpenList(stream), noErr);
	CHECK_ERR(AEStreamWriteAEDesc(stream, &text), noErr);
	CHECK_ERR(AEStreamWriteAEDesc(stream, &text), noErr);
	CHECK_ERR(AEStreamCloseList(stream), noErr);
	CHECK_ERR(AEStreamClose(stream, &list), noErr);
	CHECK(countOf(&list) == 2 && itemIs(&list, 2, typeChar, "wd", 2));
	AEDisposeDesc(&list);
	AEDisposeDesc(&text);
}

static void test_record(void)
{
	AEDesc record;
	AEStreamRef stream = AEStreamOpen();
	CHECK_ERR(AEStreamOpenRecord(stream, typeAERecord), noErr);
	CHECK_ERR(AEStreamWriteKeyDesc(stream, 'kky1', typeChar, "v1", 2), noErr);
	CHECK_ERR(AEStreamOpenKeyDesc(stream, 'kky2', typeChar), noErr);
	CHECK_ERR(AEStreamWriteData(stream, "v2", 2), noErr);
	CHECK_ERR(AEStreamCloseDesc(stream), noErr);
	CHECK_ERR(AEStreamWriteKey(stream, 'kky3'), noErr);
	CHECK_ERR(AEStreamWriteDesc(stream, typeChar, "v3", 2), noErr);
	CHECK_ERR(AEStreamCloseRecord(stream), noErr);
	CHECK_ERR(AEStreamClose(stream, &record), noErr);
	CHECK(record.descriptorType == typeAERecord);
	CHECK(countOf(&record) == 3);
	CHECK(paramIs(&record, 'kky1', typeChar, "v1"));
	CHECK(paramIs(&record, 'kky2', typeChar, "v2"));
	CHECK(paramIs(&record, 'kky3', typeChar, "v3"));
	AEDisposeDesc(&record);

	// The type of a record can be changed; it stays a record.
	stream = AEStreamOpen();
	CHECK_ERR(AEStreamOpenRecord(stream, typeAERecord), noErr);
	CHECK_ERR(AEStreamSetRecordType(stream, 'abcd'), noErr);
	CHECK_ERR(AEStreamWriteKeyDesc(stream, 'kky1', typeChar, "v", 1), noErr);
	CHECK_ERR(AEStreamCloseRecord(stream), noErr);
	CHECK_ERR(AEStreamClose(stream, &record), noErr);
	CHECK(record.descriptorType == 'abcd' && AECheckIsRecord(&record) && countOf(&record) == 1);
	AEDisposeDesc(&record);

	// A key written twice before a value: the second one is used.
	stream = AEStreamOpen();
	CHECK_ERR(AEStreamOpenRecord(stream, typeAERecord), noErr);
	CHECK_ERR(AEStreamWriteKey(stream, 'kky1'), noErr);
	CHECK_ERR(AEStreamWriteKey(stream, 'kky2'), noErr);
	CHECK_ERR(AEStreamWriteDesc(stream, typeChar, "x", 1), noErr);
	CHECK_ERR(AEStreamCloseRecord(stream), noErr);
	CHECK_ERR(AEStreamClose(stream, &record), noErr);
	CHECK(countOf(&record) == 1 && paramIs(&record, 'kky2', typeChar, "x"));
	AEDisposeDesc(&record);
}

static void test_nesting(void)
{
	AEDesc list;
	AEStreamRef stream = AEStreamOpen();
	CHECK_ERR(AEStreamOpenList(stream), noErr);
	CHECK_ERR(AEStreamOpenList(stream), noErr);
	CHECK_ERR(AEStreamWriteDesc(stream, typeChar, "in", 2), noErr);
	CHECK_ERR(AEStreamCloseList(stream), noErr);
	CHECK_ERR(AEStreamOpenRecord(stream, typeAERecord), noErr);
	CHECK_ERR(AEStreamWriteKeyDesc(stream, 'kky1', typeChar, "r", 1), noErr);
	CHECK_ERR(AEStreamCloseRecord(stream), noErr);
	CHECK_ERR(AEStreamCloseList(stream), noErr);
	CHECK_ERR(AEStreamClose(stream, &list), noErr);
	CHECK(list.descriptorType == typeAEList && countOf(&list) == 2);

	AEDesc inner;
	AEKeyword key;
	CHECK_ERR(AEGetNthDesc(&list, 1, typeWildCard, &key, &inner), noErr);
	CHECK(inner.descriptorType == typeAEList && countOf(&inner) == 1 && itemIs(&inner, 1, typeChar, "in", 2));
	AEDisposeDesc(&inner);
	CHECK_ERR(AEGetNthDesc(&list, 2, typeWildCard, &key, &inner), noErr);
	CHECK(inner.descriptorType == typeAERecord && paramIs(&inner, 'kky1', typeChar, "r"));
	AEDisposeDesc(&inner);
	AEDisposeDesc(&list);
}

static void test_event(void)
{
	AppleEvent event;
	AEStreamRef stream = AEStreamCreateEvent('evcl', 'evid', typeApplicationBundleID, "com.example.x", 13, 7, 12345);
	CHECK(stream != NULL);
	CHECK_ERR(AEStreamWriteKeyDesc(stream, keyDirectObject, typeChar, "hi", 2), noErr);
	CHECK_ERR(AEStreamOpenKeyDesc(stream, 'aaaa', typeChar), noErr);
	CHECK_ERR(AEStreamWriteData(stream, "xyz", 3), noErr);
	CHECK_ERR(AEStreamCloseDesc(stream), noErr);
	// A record written under a key.
	CHECK_ERR(AEStreamWriteKey(stream, 'kky1'), noErr);
	CHECK_ERR(AEStreamOpenRecord(stream, typeAERecord), noErr);
	CHECK_ERR(AEStreamWriteKeyDesc(stream, 'kky2', typeChar, "in", 2), noErr);
	CHECK_ERR(AEStreamCloseRecord(stream), noErr);
	CHECK_ERR(AEStreamOptionalParam(stream, 'opt1'), noErr);
	CHECK_ERR(AEStreamOptionalParam(stream, 'opt2'), noErr);
	CHECK_ERR(AEStreamClose(stream, &event), noErr);

	CHECK(event.descriptorType == typeAppleEvent);
	CHECK(paramIs(&event, keyDirectObject, typeChar, "hi"));
	CHECK(paramIs(&event, 'aaaa', typeChar, "xyz"));
	AEDesc record;
	CHECK_ERR(AEGetParamDesc(&event, 'kky1', typeWildCard, &record), noErr);
	CHECK(record.descriptorType == typeAERecord && paramIs(&record, 'kky2', typeChar, "in"));
	AEDisposeDesc(&record);

	DescType type = 0;
	Size size = 0;
	AEEventClass clazz = 0;
	AEEventID id = 0;
	char target[32];
	SInt16 returnID = 0;
	SInt32 transaction = 0;
	CHECK_ERR(AEGetAttributePtr(&event, keyEventClassAttr, typeType, &type, &clazz, sizeof clazz, &size), noErr);
	CHECK(clazz == 'evcl');
	CHECK_ERR(AEGetAttributePtr(&event, keyEventIDAttr, typeType, &type, &id, sizeof id, &size), noErr);
	CHECK(id == 'evid');
	CHECK_ERR(AEGetAttributePtr(&event, keyAddressAttr, typeWildCard, &type, target, sizeof target, &size), noErr);
	CHECK(type == typeApplicationBundleID && size == 13 && memcmp(target, "com.example.x", 13) == 0);
	CHECK_ERR(AEGetAttributePtr(&event, keyReturnIDAttr, typeSInt16, &type, &returnID, sizeof returnID, &size), noErr);
	CHECK(returnID == 7);
	CHECK_ERR(AEGetAttributePtr(&event, keyTransactionIDAttr, typeSInt32, &type, &transaction, sizeof transaction, &size), noErr);
	CHECK(transaction == 12345);

	// The optional keywords are a list of keyword descriptors.
	AEDescList optional;
	CHECK_ERR(AEGetAttributeDesc(&event, keyOptionalKeywordAttr, typeWildCard, &optional), noErr);
	CHECK(optional.descriptorType == typeAEList && countOf(&optional) == 2);
	AEKeyword first = 'opt1', second = 'opt2';
	CHECK(itemIs(&optional, 1, typeKeyword, &first, sizeof first));
	CHECK(itemIs(&optional, 2, typeKeyword, &second, sizeof second));
	AEDisposeDesc(&optional);
	AEDisposeDesc(&event);

	// A value written without a key is stored under the wildcard key.
	stream = AEStreamCreateEvent('evcl', 'evid', typeApplicationBundleID, "com.example.x", 13, 1, 1);
	CHECK_ERR(AEStreamWriteDesc(stream, typeChar, "x", 1), noErr);
	CHECK_ERR(AEStreamClose(stream, &event), noErr);
	CHECK(paramIs(&event, typeWildCard, typeChar, "x"));
	AEDisposeDesc(&event);

	// No target, no address attribute.
	stream = AEStreamCreateEvent('evcl', 'evid', typeNull, NULL, 0, 1, 1);
	CHECK(stream != NULL);
	CHECK_ERR(AEStreamClose(stream, &event), noErr);
	CHECK(event.descriptorType == typeAppleEvent);
	CHECK_ERR(AESizeOfAttribute(&event, keyAddressAttr, &type, &size), errAEDescNotFound);
	AEDisposeDesc(&event);
}

static void test_open_event(void)
{
	AppleEvent event, target, result;
	CHECK_ERR(AECreateDesc(typeApplicationBundleID, "com.example.y", 13, &target), noErr);
	CHECK_ERR(AECreateAppleEvent('ecl2', 'eid2', &target, 1, 2, &event), noErr);
	AEDisposeDesc(&target);
	CHECK_ERR(AEPutParamPtr(&event, 'old1', typeChar, "old", 3), noErr);

	AEStreamRef stream = AEStreamOpenEvent(&event);
	CHECK(stream != NULL);
	// The stream has taken the event.
	CHECK(event.descriptorType == typeNull);
	CHECK_ERR(AEStreamWriteKeyDesc(stream, 'new1', typeChar, "new", 3), noErr);
	CHECK_ERR(AEStreamClose(stream, &result), noErr);
	CHECK(result.descriptorType == typeAppleEvent && countOf(&result) == 2);
	CHECK(paramIs(&result, 'old1', typeChar, "old"));
	CHECK(paramIs(&result, 'new1', typeChar, "new"));
	AEEventClass clazz = 0;
	DescType type = 0;
	Size size = 0;
	CHECK_ERR(AEGetAttributePtr(&result, keyEventClassAttr, typeType, &type, &clazz, sizeof clazz, &size), noErr);
	CHECK(clazz == 'ecl2');
	AEDisposeDesc(&result);
}

static void test_misuse(void)
{
	AEDesc desc;
	AEStreamRef stream;

	// Nothing written.
	stream = AEStreamOpen();
	CHECK_ERR(AEStreamClose(stream, &desc), errAEStreamBadNesting);
	CHECK(desc.descriptorType == typeNull);
	stream = AEStreamOpen();
	CHECK_ERR(AEStreamClose(stream, NULL), errAEStreamBadNesting);

	// Closing or writing what is not open.
	stream = AEStreamOpen();
	CHECK_ERR(AEStreamCloseDesc(stream), errAEStreamBadNesting);
	CHECK_ERR(AEStreamWriteData(stream, "x", 1), errAEStreamBadNesting);
	CHECK_ERR(AEStreamOpenList(stream), noErr);
	CHECK_ERR(AEStreamCloseRecord(stream), errAEStreamBadNesting);
	CHECK_ERR(AEStreamCloseDesc(stream), errAEStreamBadNesting);
	CHECK_ERR(AEStreamClose(stream, NULL), noErr);

	// A descriptor whose data is still being appended cannot be closed over.
	stream = AEStreamOpen();
	CHECK_ERR(AEStreamOpenDesc(stream, typeChar), noErr);
	CHECK_ERR(AEStreamWriteData(stream, "zz", 2), noErr);
	CHECK_ERR(AEStreamClose(stream, &desc), errAEStreamBadNesting);
	CHECK(desc.descriptorType == typeNull);

	// Whatever has been written is kept when only the stream is disposed of.
	stream = AEStreamOpen();
	CHECK_ERR(AEStreamWriteDesc(stream, typeChar, "a", 1), noErr);
	CHECK_ERR(AEStreamClose(stream, NULL), noErr);
}

int main(void)
{
	test_single_descriptor();
	test_list();
	test_record();
	test_nesting();
	test_event();
	test_open_event();
	test_misuse();
	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
