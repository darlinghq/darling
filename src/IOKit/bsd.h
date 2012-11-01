#ifndef IOKIT_BSD_H
#define IOKIT_BSD_H
#include <CoreFoundation/CFDictionary.h>
#include <string>

extern "C"
CFMutableDictionaryRef IOBSDNameMatching(void* iokitPort, unsigned int options, const char* bsdName);

std::string DarlingTranslateBSDName(const char* name);
bool DarlingFindBSDName(const char* name, char* path);

#endif

