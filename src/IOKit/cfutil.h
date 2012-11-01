#ifndef IOKIT_CFUTIL_H
#define IOKIT_CFUTIL_H
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFNumber.h>

#define strCFEqual(cfs, cstr) (CFStringCompare(cfs, CFSTR(cstr), CFStringCompareFlags(0)) == 0)

inline CFStringRef strToCF(const char* val, CFAllocatorRef all = kCFAllocatorDefault)
{
	if (!val)
		return nullptr;
	return CFStringCreateWithCString(kCFAllocatorDefault, val, kCFStringEncodingUTF8);
}

inline CFNumberRef intToCF(int val, CFAllocatorRef all = kCFAllocatorDefault)
{
	return CFNumberCreate(all, kCFNumberIntType, &val);
}

#endif
