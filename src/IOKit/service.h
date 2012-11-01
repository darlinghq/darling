#ifndef IOKIT_SERVICE_H
#define IOKIT_SERVICE_H
#ifndef __STDC_LIMIT_MACROS
#       define __STDC_LIMIT_MACROS
#endif

#include "io_object.h"
#include "io_iterator.h"
#include <CoreFoundation/CFDictionary.h>

// This seems to give all children (e.g. all USB devices)
extern "C" CFMutableDictionaryRef IOServiceMatching(const char* service);
// This seems to give the root device (e.g. the USB driver)
extern "C" CFMutableDictionaryRef IOServiceNameMatching(const char* name);
extern "C" CFMutableDictionaryRef IOBSDNameMatching(void* iokitPort, unsigned int options, const char* bsdName);

extern "C" int IOServiceGetMatchingServices(void* port, CFDictionaryRef rules, io_iterator_t* iter);
extern "C" int IOIteratorIsValid(io_iterator_t iter);
extern "C" io_object_t IOIteratorNext(io_iterator_t iter);
extern "C" int IOObjectRelease(io_object_t obj);

extern "C" int IORegistryEntryCreateCFProperties(io_object_t obj, CFMutableDictionaryRef* props, CFAllocatorRef allocator, int opts);

#endif

