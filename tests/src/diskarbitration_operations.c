// CFLAGS: -framework DiskArbitration -framework CoreFoundation
// Copyright (C) 2026 Lubos Dolezel
//
// DADiskClaim, DADiskUnclaim, DADiskIsClaimed, DADiskEject and DADiskUnmount on a disk object
// made from a BSD name that names no disk, and DADissenter. The completion callbacks run with a
// dissenter of status kDAReturnBadArgument, before the call returns (the quirk_* checks: neither
// is documented). Observed on macOS 26.7.3; the program passes unchanged there.
#include <DiskArbitration/DiskArbitration.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>

static int failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL: %s (line %d)\n", #c, __LINE__); failures++; } } while (0)

struct result { int calls; DAReturn status; int hadDissenter; int hadString; void *context; };

static void completion(DADiskRef disk, DADissenterRef dissenter, void *context)
{
	struct result *r = context;
	r->calls++;
	r->hadDissenter = dissenter != NULL;
	if (dissenter) {
		r->status = DADissenterGetStatus(dissenter);
		r->hadString = DADissenterGetStatusString(dissenter) != NULL;
	}
	r->context = context;
}

static DADissenterRef releaseCallback(DADiskRef disk, void *context) { return NULL; }

int main(void)
{
	DASessionRef session = DASessionCreate(kCFAllocatorDefault);
	CHECK(session != NULL);
	DADiskRef disk = DADiskCreateFromBSDName(kCFAllocatorDefault, session, "disk99x");
	CHECK(disk != NULL);

	// quirk_claim_completes_before_return
	struct result claim = {0};
	DADiskClaim(disk, kDADiskClaimOptionDefault, releaseCallback, NULL, completion, &claim);
	CHECK(claim.calls == 1);
	CHECK(claim.hadDissenter);
	CHECK(claim.status == kDAReturnBadArgument);
	CHECK(claim.context == &claim);
	CHECK(!DADiskIsClaimed(disk));
	DADiskUnclaim(disk);
	CHECK(!DADiskIsClaimed(disk));

	// quirk_eject_completes_before_return
	struct result eject = {0};
	DADiskEject(disk, kDADiskEjectOptionDefault, completion, &eject);
	CHECK(eject.calls == 1 && eject.hadDissenter && eject.status == kDAReturnBadArgument);

	// quirk_unmount_completes_before_return
	struct result unmount = {0};
	DADiskUnmount(disk, kDADiskUnmountOptionDefault, completion, &unmount);
	CHECK(unmount.calls == 1 && unmount.hadDissenter && unmount.status == kDAReturnBadArgument);
	CHECK(!unmount.hadString);

	// Without a callback the operations are accepted
	DADiskClaim(disk, kDADiskClaimOptionDefault, NULL, NULL, NULL, NULL);
	DADiskEject(disk, kDADiskEjectOptionDefault, NULL, NULL);
	DADiskUnmount(disk, kDADiskUnmountOptionForce, NULL, NULL);

	// A dissenter keeps the status and the string it is made with
	DADissenterRef plain = DADissenterCreate(kCFAllocatorDefault, kDAReturnBusy, NULL);
	CHECK(DADissenterGetStatus(plain) == kDAReturnBusy);
	CHECK(DADissenterGetStatusString(plain) == NULL);
	DADissenterRef described = DADissenterCreate(kCFAllocatorDefault, kDAReturnNotPermitted, CFSTR("not allowed"));
	CHECK(DADissenterGetStatus(described) == kDAReturnNotPermitted);
	CHECK(DADissenterGetStatusString(described) != NULL &&
		CFStringCompare(DADissenterGetStatusString(described), CFSTR("not allowed"), 0) == kCFCompareEqualTo);
	CFRelease(plain);
	CFRelease(described);

	CFRelease(disk);
	CFRelease(session);
	printf(failures ? "FAILED\n" : "PASS\n");
	return failures != 0;
}
