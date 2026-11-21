// Copyright (C) 2026 Lubos Dolezel
//
// FSGetVolumeParms: the startup volume's version 5 GetVolParmsInfoBuffer, a buffer smaller than
// the structure getting only its leading bytes, and the volume reference numbers that are refused.
// The expected results were observed on macOS 26.7.3 (x86_64) and macOS 10.14.4 (i386); the QUIRK
// comment in FileManager.cpp names this test. Build for x86_64 and i386.
#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <string.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_ERR(expr, expected) do { OSStatus e_ = (expr); if (e_ != (expected)) { printf("FAIL %d: %s returned %d, expected %d\n", __LINE__, #expr, (int) e_, (int) (expected)); g_failures++; } } while (0)

int main(void)
{
	FSVolumeRefNum boot = 0;
	CHECK_ERR(FSGetVolumeInfo(-100, 0, &boot, kFSVolInfoNone, NULL, NULL, NULL), noErr);
	CHECK(boot == -100);

	GetVolParmsInfoBuffer parms;
	memset(&parms, 0x5a, sizeof(parms));
	CHECK_ERR(FSGetVolumeParms(boot, &parms, sizeof(parms)), noErr);
	CHECK(parms.vMVersion == 5);
	// folder locking and asynchronous requests (bHasFolderLock, bSupportsAsyncRequests)
	CHECK((parms.vMAttrib & 0x408) == 0x408);
	CHECK(parms.vMExtendedAttributes == 0x10ac2432);
	CHECK(parms.vMVolumeGrade == 0 && parms.vMForeignPrivID == 0 && parms.vMServerAdr == 0);
	CHECK(parms.vMLocalHand == NULL);
	CHECK(parms.vMDeviceID != NULL);
	CHECK(parms.vMMaxNameLength == 85);

	// A smaller buffer gets the leading bytes, without an error; size 0 writes nothing.
	memset(&parms, 0x5a, sizeof(parms));
	CHECK_ERR(FSGetVolumeParms(boot, &parms, 0), noErr);
	CHECK(parms.vMVersion == 0x5a5a && parms.vMAttrib == 0x5a5a5a5a);
	CHECK_ERR(FSGetVolumeParms(boot, &parms, 2), noErr);
	CHECK(parms.vMVersion == 5 && parms.vMAttrib == 0x5a5a5a5a);
	CHECK_ERR(FSGetVolumeParms(boot, &parms, 10), noErr);
	CHECK(parms.vMVersion == 5 && (parms.vMAttrib & 0x408) == 0x408 && parms.vMExtendedAttributes == 0x5a5a5a5a);

	// Only the startup volume's reference number is accepted; the buffer is left alone.
	const FSVolumeRefNum refused[] = { 0, 1, -1, 99, -32768 };
	for (unsigned i = 0; i < sizeof(refused) / sizeof(refused[0]); i++) {
		memset(&parms, 0x5a, sizeof(parms));
		CHECK_ERR(FSGetVolumeParms(refused[i], &parms, sizeof(parms)), nsvErr);
		CHECK(parms.vMVersion == 0x5a5a);
	}

	CHECK_ERR(FSGetVolumeParms(boot, NULL, sizeof(parms)), paramErr);

	printf(g_failures == 0 ? "PASS\n" : "%d failure(s)\n", g_failures);
	return g_failures != 0;
}
