#ifndef _ALLOC_ONCE_IMPL_H_
#define _ALLOC_ONCE_IMPL_H_
#include <stddef.h>

#define OS_ALLOC_ONCE_KEY_MAX 32

void* os_alloc_once(int key, size_t size, void(*ctor)(void*));

#endif

