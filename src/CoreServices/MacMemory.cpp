#include "MacMemory.h"
#include <cstring>

void BlockMove(const void* src, void* dst, size_t count)
{
	memmove(dst, src, count);
}

void BlockMoveData(const void* src, void* dst, size_t count)
{
	memmove(dst, src, count);
}

void BlockMoveUncached(const void* src, void* dst, size_t count)
{
	memmove(dst, src, count);
}

void BlockMoveDataUncached(const void* src, void* dst, size_t count)
{
	memmove(dst, src, count);
}

void BlockZero(void* dst, size_t count)
{
	memset(dst, 0, count);
}

void BlockZeroUncached(void* dst, size_t count)
{
	memset(dst, 0, count);
}

