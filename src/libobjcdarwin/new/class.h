#ifndef NEW_CLASS_H
#define NEW_CLASS_H
#include "./AppleLayout.h"
#include <objc/runtime.h>
#include <stdint.h>
#include "../../dyld/public.h"

Class RegisterClass(const class_t* cls, intptr_t slide);
void ProcessClassesNew(const struct mach_header* mh, intptr_t slide, const class_t** classes, unsigned long size);
void UpdateClassRefs(const struct mach_header* mh);

#endif
