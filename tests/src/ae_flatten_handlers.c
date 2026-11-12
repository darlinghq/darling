// Copyright (C) 2026 Lubos Dolezel
//
// AE: flattening an Apple event and unflattening it gives back an equal event, a too-small buffer is
// reported with the needed size, and the event handler table (install, get, replace, remove) behaves as
// documented. No other process is involved.
#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static OSErr handlerA(const AppleEvent *event, AppleEvent *reply, SRefCon refcon) { return noErr; }
static OSErr handlerB(const AppleEvent *event, AppleEvent *reply, SRefCon refcon) { return noErr; }

static void test_flatten(void)
{
	AEDesc target;
	SInt32 pid = 4242;
	CHECK(AECreateDesc(typeKernelProcessID, &pid, sizeof(pid), &target) == noErr);

	AppleEvent event;
	CHECK(AECreateAppleEvent('GURL', 'GURL', &target, kAutoGenerateReturnID, kAnyTransactionID, &event) == noErr);
	AEDisposeDesc(&target);
	const char *url = "scheme://host/path?x=1";
	CHECK(AEPutParamPtr(&event, keyDirectObject, typeUTF8Text, url, strlen(url)) == noErr);

	// A list nested in a parameter.
	AEDescList list;
	CHECK(AECreateList(NULL, 0, false, &list) == noErr);
	CHECK(AEPutPtr(&list, 0, typeUTF8Text, "one", 3) == noErr);
	CHECK(AEPutPtr(&list, 0, typeSInt32, &pid, sizeof(pid)) == noErr);
	CHECK(AEPutParamDesc(&event, 'list', &list) == noErr);
	AEDisposeDesc(&list);

	// The needed size is reported when the buffer is too small.
	Size needed = 0;
	OSStatus st = AEFlattenDesc(&event, NULL, 0, &needed);
	CHECK(st == errAEBufferTooSmall);
	CHECK(needed > 0);
	char small[4];
	Size smallNeeded = 0;
	CHECK(AEFlattenDesc(&event, small, sizeof(small), &smallNeeded) == errAEBufferTooSmall);
	CHECK(smallNeeded == needed);

	char *buffer = malloc(needed);
	Size actual = 0;
	CHECK(AEFlattenDesc(&event, buffer, needed, &actual) == noErr);
	CHECK(actual == needed);

	AppleEvent copy;
	CHECK(AEUnflattenDesc(buffer, &copy) == noErr);
	CHECK(copy.descriptorType == typeAppleEvent);

	AEEventClass cls = 0;
	AEEventID id = 0;
	DescType type;
	Size size;
	CHECK(AEGetAttributePtr(&copy, keyEventClassAttr, typeType, &type, &cls, sizeof(cls), &size) == noErr);
	CHECK(AEGetAttributePtr(&copy, keyEventIDAttr, typeType, &type, &id, sizeof(id), &size) == noErr);
	CHECK(cls == 'GURL' && id == 'GURL');

	char text[64] = "";
	CHECK(AEGetParamPtr(&copy, keyDirectObject, typeUTF8Text, &type, text, sizeof(text) - 1, &size) == noErr);
	CHECK(size == (Size) strlen(url) && memcmp(text, url, size) == 0);

	SInt32 gotPid = 0;
	AEDesc address;
	CHECK(AEGetAttributeDesc(&copy, keyAddressAttr, typeWildCard, &address) == noErr);
	CHECK(address.descriptorType == typeKernelProcessID);
	CHECK(AEGetDescData(&address, &gotPid, sizeof(gotPid)) == noErr && gotPid == 4242);
	AEDisposeDesc(&address);

	AEDescList copyList;
	long items = 0;
	CHECK(AEGetParamDesc(&copy, 'list', typeAEList, &copyList) == noErr);
	CHECK(AECountItems(&copyList, &items) == noErr && items == 2);
	CHECK(AEGetNthPtr(&copyList, 1, typeUTF8Text, NULL, &type, text, sizeof(text) - 1, &size) == noErr && size == 3 && memcmp(text, "one", 3) == 0);
	AEDisposeDesc(&copyList);

	// Unflattening a plain data descriptor and a null descriptor.
	AEDesc plain, nullDesc;
	CHECK(AECreateDesc(typeUTF8Text, "abc", 3, &plain) == noErr);
	AEInitializeDesc(&nullDesc);
	Size n1 = 0, n2 = 0;
	AEFlattenDesc(&plain, NULL, 0, &n1);
	AEFlattenDesc(&nullDesc, NULL, 0, &n2);
	char *b1 = malloc(n1), *b2 = malloc(n2);
	AEDesc plain2, null2;
	CHECK(AEFlattenDesc(&plain, b1, n1, &n1) == noErr && AEUnflattenDesc(b1, &plain2) == noErr);
	CHECK(plain2.descriptorType == typeUTF8Text && AEGetDescDataSize(&plain2) == 3);
	CHECK(AEFlattenDesc(&nullDesc, b2, n2, &n2) == noErr && AEUnflattenDesc(b2, &null2) == noErr);
	CHECK(null2.descriptorType == typeNull);
	AEDisposeDesc(&plain);
	AEDisposeDesc(&plain2);
	AEDisposeDesc(&copy);
	AEDisposeDesc(&event);
	free(buffer); free(b1); free(b2);

	// Corrupt input is rejected instead of read.
	AEDesc bad;
	CHECK(AEUnflattenDesc(NULL, &bad) != noErr);
}

static void test_handlers(void)
{
	AEEventHandlerUPP got = NULL;
	SRefCon ref = 0;
	CHECK(AEGetEventHandler('GURL', 'tst1', &got, &ref, false) == errAEHandlerNotFound);
	CHECK(AEInstallEventHandler('GURL', 'tst1', handlerA, (SRefCon) 1, false) == noErr);
	CHECK(AEGetEventHandler('GURL', 'tst1', &got, &ref, false) == noErr);
	CHECK(got == handlerA && ref == (SRefCon) 1);
	// A system handler is a separate table.
	CHECK(AEGetEventHandler('GURL', 'tst1', &got, &ref, true) == errAEHandlerNotFound);

	// Installing again for the same class and ID replaces the handler.
	CHECK(AEInstallEventHandler('GURL', 'tst1', handlerB, (SRefCon) 2, false) == noErr);
	CHECK(AEGetEventHandler('GURL', 'tst1', &got, &ref, false) == noErr);
	CHECK(got == handlerB && ref == (SRefCon) 2);

	CHECK(AERemoveEventHandler('GURL', 'tst1', handlerB, false) == noErr);
	CHECK(AEGetEventHandler('GURL', 'tst1', &got, &ref, false) == errAEHandlerNotFound);
	CHECK(AERemoveEventHandler('GURL', 'tst1', handlerB, false) == errAEHandlerNotFound);
}

int main(void)
{
	test_flatten();
	test_handlers();
	printf(g_failures ? "FAILED (%d)\n" : "PASSED\n", g_failures);
	return g_failures != 0;
}
