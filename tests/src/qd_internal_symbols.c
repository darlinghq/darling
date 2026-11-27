// Copyright (C) 2026 Lubos Dolezel
//
// The QuickDraw framework exports the QuickDraw API and nothing else of its own: the helpers its sources share
// (port state, picture recording, the diagnostic count of skipped text) are not visible to a program, which cannot
// find them with dlsym(). The API itself is found.
#include <dlfcn.h>
#include <stdio.h>

static int g_failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); g_failures++; } } while (0)

int main(void)
{
	void *qd = dlopen("/System/Library/Frameworks/ApplicationServices.framework/Versions/A/Frameworks/QD.framework/Versions/A/QD", RTLD_NOW);
	CHECK(qd != NULL);
	if (!qd) return 1;
#ifndef __LP64__
	CHECK(dlsym(qd, "DrawPicture") != NULL);
	CHECK(dlsym(qd, "QDError") != NULL);
	static const char *const internal[] = {"QDLastPictureSkippedText", "QDPortInitState", "QDPortDisposeState",
		"QDPortFromPixMap", "QDPortIsLive", "QDRecordColor", "QDRecordRect", "QDRecordLine", "QDRecordCopyBits",
		"QDRecordPenPat", "QDRecordOpen", "QDRecordClose", "QDPictureUnrecordedOps"};
	for (unsigned i = 0; i < sizeof(internal) / sizeof(internal[0]); i++) {
		if (dlsym(qd, internal[i]) != NULL) { printf("FAIL: %s is exported\n", internal[i]); g_failures++; }
	}
#endif
	printf(g_failures ? "FAILED: %d\n" : "PASSED\n", g_failures);
	return g_failures != 0;
}
