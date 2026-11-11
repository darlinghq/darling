#include "DASession.h"
#include <DiskArbitration/DiskArbitration.h>
#include <stddef.h>
#include <string.h>
#include <CoreFoundation/CFRuntime.h>
#include <CoreFoundation/CFBase.h>

struct _DASession
{
  CFRuntimeBase _parent;
  const CFArrayCallBacks *_callBacks;
};

static CFTypeID _kDASessionTypeID = 0;

// DADisk.c
extern void _DADiskInitializeMatchObjects(void);
static void DASessionFinalize(CFTypeRef cf);
static Boolean DASessionEqual(CFTypeRef cf1, CFTypeRef cf2);
static CFHashCode DASessionHash(CFTypeRef cf);
static DASessionRef DASessionCreateCopy (CFAllocatorRef allocator, DASessionRef disk);

static CFRuntimeClass DASessionClass = {
  0,
  "DASession",
  NULL,
  (CFTypeRef (*)(CFAllocatorRef, CFTypeRef)) DASessionCreateCopy,
  DASessionFinalize,
  DASessionEqual,
  DASessionHash,
  NULL,
  NULL
};

__attribute__((constructor))
void
DASessionInitialize (void)
{
	_kDASessionTypeID = _CFRuntimeRegisterClass (&DASessionClass);
}

#define DASESSION_SIZE sizeof(struct _DASession) - sizeof(CFRuntimeBase)

static void
DASessionFinalize (CFTypeRef cf)
{
}

static Boolean
DASessionEqual (CFTypeRef cf1, CFTypeRef cf2)
{
	return 0;
}

static CFHashCode
DASessionHash (CFTypeRef cf)
{
	return 0;
}

static DASessionRef
DASessionCreateCopy (CFAllocatorRef allocator, DASessionRef disk)
{
	return (DASessionRef) CFRetain(disk);
}

CFTypeID DASessionGetTypeID(void)
{
	return _kDASessionTypeID;
}

DASessionRef DASessionCreate(CFAllocatorRef allocator)
{
	DASessionRef new;

	_DADiskInitializeMatchObjects();
	
	new = (DASessionRef) _CFRuntimeCreateInstance (allocator, _kDASessionTypeID,
												DASESSION_SIZE, 0);
	
	return new;
}

void DASessionScheduleWithRunLoop(DASessionRef session,
		CFRunLoopRef runLoop, CFStringRef runLoopMode)
{
	
}

void DASessionUnscheduleFromRunLoop(DASessionRef session,
		CFRunLoopRef runLoop, CFStringRef runLoopMode)
{
	
}

void DASessionSetDispatchQueue(DASessionRef session, dispatch_queue_t queue)
{
	
}

struct _DAApprovalSession
{
  CFRuntimeBase _parent;
  const CFArrayCallBacks *_callBacks;
};

static CFTypeID _kDAApprovalSessionTypeID = 0;
static void DAApprovalSessionFinalize(CFTypeRef cf);
static Boolean DAApprovalSessionEqual(CFTypeRef cf1, CFTypeRef cf2);
static CFHashCode DAApprovalSessionHash(CFTypeRef cf);
static DAApprovalSessionRef DAApprovalSessionCreateCopy (CFAllocatorRef allocator, DAApprovalSessionRef disk);

static CFRuntimeClass DAApprovalSessionClass = {
  0,
  "DAApprovalSession",
  NULL,
  (CFTypeRef (*)(CFAllocatorRef, CFTypeRef)) DAApprovalSessionCreateCopy,
  DAApprovalSessionFinalize,
  DAApprovalSessionEqual,
  DAApprovalSessionHash,
  NULL,
  NULL
};

__attribute__((constructor))
void
DAApprovalSessionInitialize (void)
{
	_kDAApprovalSessionTypeID = _CFRuntimeRegisterClass (&DAApprovalSessionClass);
}

#define DAAPPROVALSESSION_SIZE sizeof(struct _DAApprovalSession) - sizeof(CFRuntimeBase)

static void
DAApprovalSessionFinalize (CFTypeRef cf)
{
}

static Boolean
DAApprovalSessionEqual (CFTypeRef cf1, CFTypeRef cf2)
{
	return 0;
}

static CFHashCode
DAApprovalSessionHash (CFTypeRef cf)
{
	return 0;
}

static DAApprovalSessionRef
DAApprovalSessionCreateCopy (CFAllocatorRef allocator, DAApprovalSessionRef disk)
{
	return (DAApprovalSessionRef) CFRetain(disk);
}

CFTypeID DAApprovalSessionGetTypeID(void)
{
	return _kDAApprovalSessionTypeID;
}

DAApprovalSessionRef DAApprovalSessionCreate(CFAllocatorRef allocator)
{
	DAApprovalSessionRef new;

	_DADiskInitializeMatchObjects();
	
	new = (DAApprovalSessionRef) _CFRuntimeCreateInstance (allocator,
												_kDAApprovalSessionTypeID,
												DAAPPROVALSESSION_SIZE, 0);
	
	return new;
}

void DAApprovalSessionScheduleWithRunLoop(DAApprovalSessionRef session,
		CFRunLoopRef runLoop, CFStringRef runLoopMode)
{
	
}

void DAApprovalSessionUnscheduleFromRunLoop(DAApprovalSessionRef session,
		CFRunLoopRef runLoop, CFStringRef runLoopMode)
{
	
}

/* Callback registration is accepted and ignored: no disk events are delivered. */
void DARegisterDiskAppearedCallback(DASessionRef session, CFDictionaryRef match, DADiskAppearedCallback callback, void *context)
{
}

void DARegisterDiskDescriptionChangedCallback(DASessionRef session, CFDictionaryRef match, CFArrayRef watch, DADiskDescriptionChangedCallback callback, void *context)
{
}

void DARegisterDiskDisappearedCallback(DASessionRef session, CFDictionaryRef match, DADiskDisappearedCallback callback, void *context)
{
}

void DARegisterDiskMountApprovalCallback(DASessionRef session, CFDictionaryRef match, DADiskMountApprovalCallback callback, void *context)
{
}

void DARegisterDiskUnmountApprovalCallback(DASessionRef session, CFDictionaryRef match, DADiskUnmountApprovalCallback callback, void *context)
{
}

void DARegisterDiskEjectApprovalCallback(DASessionRef session, CFDictionaryRef match, DADiskEjectApprovalCallback callback, void *context)
{
}

void DARegisterDiskPeekCallback(DASessionRef session, CFDictionaryRef match, CFIndex order, DADiskPeekCallback callback, void *context)
{
}

void DAUnregisterCallback(DASessionRef session, void *callback, void *context)
{
}

void DAUnregisterApprovalCallback(DASessionRef session, void *callback, void *context)
{
}
