#include "DADissenter.h"
#include <CoreFoundation/CFRuntime.h>

struct __DADissenter
{
  CFRuntimeBase _parent;
  DAReturn status;
  CFStringRef string;	// may be NULL
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

#define DADISSENTER_SIZE sizeof(struct __DADissenter) - sizeof(CFRuntimeBase)

static void
DADissenterFinalize (CFTypeRef cf)
{
	DADissenterRef dissenter = (DADissenterRef) cf;
	if (dissenter->string)
		CFRelease(dissenter->string);
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
	struct __DADissenter *new;
	
	new = (struct __DADissenter *) _CFRuntimeCreateInstance (allocator,
												_kDADissenterTypeID,
												DADISSENTER_SIZE, 0);
	if (!new)
		return NULL;
	new->status = status;
	new->string = string ? CFStringCreateCopy(allocator, string) : NULL;
	
	return new;
}

DAReturn DADissenterGetStatus(DADissenterRef dissenter)
{
	return dissenter->status;
}

CFStringRef DADissenterGetStatusString(DADissenterRef dissenter)
{
	return dissenter->string;
}
