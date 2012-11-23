#ifndef IOKIT_CFUTIL_H
#define IOKIT_CFUTIL_H
extern "C" {
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFNumber.h>
}
#include <cstdlib>

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

inline CFNumberRef intToCF(const char* val, CFAllocatorRef all = kCFAllocatorDefault)
{
	if (!val)
		return nullptr;
	return intToCF(atoi(val), all);
}

#endif
