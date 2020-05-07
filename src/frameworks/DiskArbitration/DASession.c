#include "DASession.h"
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
