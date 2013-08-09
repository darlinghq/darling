#ifndef OLD_CATEGORY_H
#define OLD_CATEGORY_H

#include "./AppleLayout.h"
#include <objc/runtime.h>
#include <stdint.h>
#include <libdyld/dyld_public.h>

void ProcessCategoriesOld(const struct mach_header* mh, intptr_t slide, module_info* modinfo);

#endif

