#ifndef OLD_IVAR_H
#define OLD_IVAR_H
#include "./AppleLayout.h"
#include <objc/runtime.h>
#include <stdint.h>

void ConvertIvarList(Class c, const old_ivar_list* list);

#endif
