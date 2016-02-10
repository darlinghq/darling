#include "DADissenter.h"
#include <CoreFoundation/CFRuntime.h>

struct _DADissenter
{
  CFRuntimeBase _parent;
  const CFArrayCallBacks *_callBacks;
};

static CFTypeID _kDADissenterTypeID = 0;
static void DADissenterFinalize(CFTypeRef cf);
static Boolean DADissenterEqual(CFTypeRef cf1, CFTypeRef cf2);
static CFHashCode DADissenterHash(CFTypeRef cf);
static DADissenterRef DADissenterCreateCopy (CFAllocatorRef allocator, DADissenterRef disk);

static CFRuntimeClass DADissenterClass = {
  0,
  "DADissenter",
  NULL,
  (CFTypeRef (*)(CFAllocatorRef, CFTypeRef)) DADissenterCreateCopy,
  DADissenterFinalize,
  DADissenterEqual,
  DADissenterHash,
  NULL,
  NULL
};

__attribute__((constructor))
void
DADissenterInitialize (void)
{
	_kDADissenterTypeID = _CFRuntimeRegisterClass (&DADissenterClass);
}

#define DADISSENTER_SIZE sizeof(struct _DADissenter) - sizeof(CFRuntimeBase)

static void
DADissenterFinalize (CFTypeRef cf)
{
}

static Boolean
DADissenterEqual (CFTypeRef cf1, CFTypeRef cf2)
{
	return 0;
}

static CFHashCode
DADissenterHash (CFTypeRef cf)
{
	return 0;
}

static DADissenterRef
DADissenterCreateCopy (CFAllocatorRef allocator, DADissenterRef disk)
{
	return (DADissenterRef) CFRetain(disk);
}

CFTypeID DADissenterGetTypeID(void)
{
	return _kDADissenterTypeID;
}

DADissenterRef DADissenterCreate(CFAllocatorRef allocator, DAReturn status, CFStringRef string)
{
	DADissenterRef new;
	
	new = (DADissenterRef) _CFRuntimeCreateInstance (allocator,
												_kDADissenterTypeID,
												DADISSENTER_SIZE, 0);
	
	return new;
}

DAReturn DADissenterGetStatus(DADissenterRef dissenter)
{
	return 0;
}

CFStringRef DADissenterGetStatusString(DADissenterRef dissenter)
{
	return CFSTR("dummy");
}
