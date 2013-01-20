#ifndef MACMEMORY_H
#define MACMEMORY_H
#include <stddef.h>

extern "C"
{

void BlockMove(const void* src, void* dst, size_t count);
void BlockMoveData(const void* src, void* dst, size_t count);
void BlockMoveUncached(const void* src, void* dst, size_t count);
void BlockMoveDataUncached(const void* src, void* dst, size_t count);
void BlockZero(void* dst, size_t count);
void BlockZeroUncached(void* dst, size_t count);

}

#endif
