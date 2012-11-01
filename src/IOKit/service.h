#ifndef IOKIT_SERVICE_H
#define IOKIT_SERVICE_H
#ifndef __STDC_LIMIT_MACROS
#       define __STDC_LIMIT_MACROS
#endif

#include "io_object.h"
#include "io_iterator.h"
#include <CoreFoundation/CFDictionary.h>

extern "C" const void* kIOMasterPortDefault;

// This seems to give all children (e.g. all USB devices)
extern "C" CFMutableDictionaryRef IOServiceMatching(const char* service);
// This seems to give the root device (e.g. the USB driver)
extern "C" CFMutableDictionaryRef IOServiceNameMatching(const char* name);
extern "C" CFMutableDictionaryRef IOBSDNameMatching(void* iokitPort, unsigned int options, const char* bsdName);

extern "C" int IOServiceGetMatchingServices(void* port, CFDictionaryRef rules, io_iterator_t* iter);

extern "C" int IORegistryEntryGetParentEntry(io_object_t obj, void* planeName /* TODO */, io_object_t* parent);
extern "C" CFTypeRef IORegistryEntryCreateCFProperty(io_object_t obj, CFStringRef key, CFAllocatorRef allocator, int opts);
extern "C" int IORegistryEntryCreateCFProperties(io_object_t obj, CFMutableDictionaryRef* props, CFAllocatorRef allocator, int opts);

#endif

