#ifndef NEW_CATEGORY_H
#define NEW_CATEGORY_H
#include "./AppleLayout.h"
#include <objc/runtime.h>
#include <stdint.h>
#include <dyld/dyld_public.h>

void ProcessCategoriesNew(const struct mach_header* mh, intptr_t slide);

#endif
