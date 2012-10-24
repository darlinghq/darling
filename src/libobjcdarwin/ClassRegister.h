#ifndef CLASSREGISTER_H
#define CLASSREGISTER_H
#ifndef __STDC_LIMIT_MACROS
#	define __STDC_LIMIT_MACROS
#endif
#include <stdint.h>
#include <cstddef>
#include <algorithm>
#include <objc/runtime.h>
#include "../util/log.h"

void ProcessImageLoad(const struct mach_header* mh, intptr_t slide);
void ProcessImageUnload(const struct mach_header* mh, intptr_t);

#endif

