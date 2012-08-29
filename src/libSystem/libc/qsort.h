#ifndef LIBC_QSORT_H
#define LIBC_QSORT_H
#include <cstdlib>

struct __darwin_qsort_r_context
{
  int (*compar)(void*, const void*, const void*);
  void* thunk;
};


extern "C"
{

void __darwin_qsort_r(void* base, size_t nel, size_t width, void* thunk,
                      int (*compar)(void*, const void*, const void*));

}

#endif
