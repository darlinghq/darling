#ifndef OLD_CLASS_H
#define OLD_CLASS_H
#include "./AppleLayout.h"
#include <objc/runtime.h>
#include <stdint.h>
#include "../../dyld/public.h"

Class RegisterClass(old_class* cls, bool hasExt);
void ProcessClassesOld(const struct mach_header* mh, intptr_t slide, module_info* info);

#endif
