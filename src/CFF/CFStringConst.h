#ifndef CFSTRING_CONST_H
#define CFSTRING_CONST_H
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFRuntime.h>
#include <stddef.h>

struct __CFConstantString
{
  CFRuntimeBase  _parent;
  void          *_contents;
  CFIndex        _count;
  CFHashCode     _hash;
  CFAllocatorRef _deallocator;
};

#define CONST_STRING_DECL(var, str) \
  static struct __CFConstantString __ ## var ## __ = \
    { {0, 0, {1, 0, 0}}, (void*)str, sizeof(str) - 1, 0, NULL }; \
  const CFStringRef var = (CFStringRef) & __ ## var ## __;

#endif

