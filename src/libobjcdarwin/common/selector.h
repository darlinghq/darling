#ifndef COMMON_SELECTOR_H
#define COMMON_SELECTOR_H
#include <objc/runtime.h>
#include <stdint.h>

void UpdateSelectors(const struct mach_header* mh, intptr_t slide);

#endif
