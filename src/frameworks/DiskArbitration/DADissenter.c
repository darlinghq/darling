#include "DADissenter.h"
#include <CoreFoundation/CFRuntime.h>

struct _DADissenter
{
  CFRuntimeBase _parent;
  DAReturn status;
  CFStringRef string; // may be NULL
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
	struct _DADissenter* dissenter = (struct _DADissenter*) cf;
	if (dissenter->string != NULL)
		CFRelease(dissenter->string);
}

static Boolean
DADissenterEqual (CFTypeRef cf1, CFTypeRef cf2)
{
	const struct _DADissenter* d1 = cf1;
	const struct _DADissenter* d2 = cf2;
	if (d1->status != d2->status)
		return false;
	if (d1->string == NULL || d2->string == NULL)
		return d1->string == d2->string;
	return CFEqual(d1->string, d2->string);
}

static CFHashCode
DADissenterHash (CFTypeRef cf)
{
	return ((const struct _DADissenter*) cf)->status;
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
	if (new == NULL)
		return NULL;

	struct _DADissenter* dissenter = (struct _DADissenter*) new;
	dissenter->status = status;
	dissenter->string = string != NULL ? CFStringCreateCopy(allocator, string) : NULL;
	return new;
}

DAReturn DADissenterGetStatus(DADissenterRef dissenter)
{
	return ((const struct _DADissenter*) dissenter)->status;
}

CFStringRef DADissenterGetStatusString(DADissenterRef dissenter)
{
	return ((const struct _DADissenter*) dissenter)->string;
}
