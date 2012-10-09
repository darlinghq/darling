#ifndef NEW_IVAR_H
#define NEW_IVAR_H
#include "./AppleLayout.h"
#include <objc/runtime.h>
#include <stdint.h>

void ConvertIvarList(Class c, const ivar_list_t* list);

#endif
