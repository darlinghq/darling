#ifndef OLD_CLASS_H
#define OLD_CLASS_H
#include "./AppleLayout.h"
#include <objc/runtime.h>
#include <stdint.h>
#include <string>
#include <vector>
#include "../../dyld/public.h"

Class RegisterClass(old_class* cls, bool hasExt);
std::vector<const char*> ProcessClassesOld(const struct mach_header* mh, intptr_t slide, module_info* info);
void AddClassProtocols(Class conv, old_protocol_list* list);
void UpdateClassRefs(const struct mach_header* mh);

#endif
