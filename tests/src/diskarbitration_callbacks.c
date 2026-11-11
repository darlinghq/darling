// CFLAGS: -framework DiskArbitration -framework CoreFoundation
// Copyright (C) 2026 Lubos Dolezel

#include <DiskArbitration/DiskArbitration.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>

static void appeared(DADiskRef disk, void *context) {}
static void changed(DADiskRef disk, CFArrayRef keys, void *context) {}
static void disappeared(DADiskRef disk, void *context) {}
static DADissenterRef approve(DADiskRef disk, void *context) { return NULL; }

int main(void)
{
	// Registering and unregistering callbacks is accepted without a disk
	// arbitration daemon; the functions must exist and must not crash.
	DASessionRef session = DASessionCreate(kCFAllocatorDefault);
	if (session == NULL) {
		printf("FAIL: DASessionCreate\n");
		return 1;
	}
	DARegisterDiskAppearedCallback(session, NULL, appeared, NULL);
	DARegisterDiskDescriptionChangedCallback(session, NULL, NULL, changed, NULL);
	DARegisterDiskDisappearedCallback(session, NULL, disappeared, NULL);
	DARegisterDiskMountApprovalCallback(session, NULL, approve, NULL);
	DARegisterDiskUnmountApprovalCallback(session, NULL, approve, NULL);
	DARegisterDiskEjectApprovalCallback(session, NULL, approve, NULL);
	DAUnregisterCallback(session, appeared, NULL);
	DAUnregisterCallback(session, changed, NULL);
	DAUnregisterApprovalCallback(session, approve, NULL);
	CFRelease(session);
	printf("PASS\n");
	return 0;
}
