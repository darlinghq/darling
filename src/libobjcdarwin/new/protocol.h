#ifndef NEW_PROTOCOL_H
#define NEW_PROTOCOL_H
#include "./AppleLayout.h"
#include <objc/runtime.h>
#include <stdint.h>

Protocol* RegisterProtocol(const protocol_t* prot, intptr_t slide);
void ProcessProtocolsNew(const struct mach_header* mh, intptr_t slide);
void RegisterProtocolMethods(Protocol* p, const method_list_t* list, const char** extTypes, size_t& extIndex, bool required, bool instance);
void AddClassProtocols(Class c, const protocol_list_t* list, intptr_t slide);

#endif
