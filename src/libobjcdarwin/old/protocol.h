#ifndef OLD_PROTOCOL_H
#define OLD_PROTOCOL_H
#include "./AppleLayout.h"
#include <objc/runtime.h>
#include <stdint.h>
#include <libdyld/dyld_public.h>

void RegisterProtocolMethods(Protocol* p, const old_method_decl_list* list, bool required, bool instance);
Protocol* RegisterProtocol(old_protocol* prot, uintptr_t extStart, unsigned long extLen);
void ProcessProtocolsOld(const struct mach_header* mh, intptr_t slide);

#endif
