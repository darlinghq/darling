#ifndef MACMEMORY_H
#define MACMEMORY_H
#include <stddef.h>
#include "MacTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

void BlockMove(const void* src, void* dst, size_t count);
void BlockMoveData(const void* src, void* dst, size_t count);
void BlockMoveUncached(const void* src, void* dst, size_t count);
void BlockMoveDataUncached(const void* src, void* dst, size_t count);
void BlockZero(void* dst, size_t count);
void BlockZeroUncached(void* dst, size_t count);

Ptr NewPtr(long len);
Ptr NewPtrClear(long len);
void DiposePtr(Ptr p);
void DisposeHandle(Handle handle);
Size GetHandleSize(Handle h);
void EmptyHandle(Handle h);
OSErr PtrToHand(const void* srcPtr, Handle* dstHndl, long size);

// free and new alloc
void ReallocateHandle(Handle h, Size byteCount);
// and other crap with relocatable memory blocks etc.

#ifdef __cplusplus
}
#endif

#endif
