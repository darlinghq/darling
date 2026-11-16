#include "DASession.h"
#include <DiskArbitration/DiskArbitration.h>
#include <stddef.h>
#include <string.h>
#include <CoreFoundation/CFRuntime.h>
#include <CoreFoundation/CFBase.h>

#include "DAPrivate.h"
#include "DAProtocol.h"

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

#define DASESSION_SIZE sizeof(struct __DASession) - sizeof(CFRuntimeBase)

static void
DASessionFinalize (CFTypeRef cf)
{
	DASessionRef session = (DASessionRef) cf;

	if (session->connection != NULL)
	{
		xpc_connection_cancel(session->connection);
		xpc_release(session->connection);
	}
	if (session->queue != NULL)
		dispatch_release(session->queue);
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

// Connects to diskarbitrationd. Without the daemon the session still works; its requests then find no disks.
void _DASessionConnect(DASessionRef session)
{
	session->queue = dispatch_queue_create("com.apple.DiskArbitration.session", NULL);
	session->connection = xpc_connection_create_mach_service(DA_SERVICE_NAME, session->queue, 0);
	if (session->connection == NULL)
		return;
	xpc_connection_set_event_handler(session->connection, ^(xpc_object_t event) {
	});
	xpc_connection_resume(session->connection);
}

DASessionRef DASessionCreate(CFAllocatorRef allocator)
{
	DASessionRef new;

	_DADiskInitializeMatchObjects();
	
	new = (DASessionRef) _CFRuntimeCreateInstance (allocator, _kDASessionTypeID,
												DASESSION_SIZE, 0);
	if (new == NULL)
		return NULL;

	_DASessionConnect(new);
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
