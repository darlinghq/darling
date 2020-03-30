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
void DisposePtr(Ptr p);
void DisposeHandle(Handle handle);
Size GetHandleSize(Handle h);
void EmptyHandle(Handle h);
OSErr PtrToHand(const void* srcPtr, Handle* dstHndl, long size);
OSErr PtrToXHand(const void* srcPtr, Handle dstHndl, long size);
Handle NewHandle(long size);
Handle NewHandleClear(long size);
Handle NewEmptyHandle(void);
long GetPtrSize(Ptr ptr);
void SetPtrSize(Ptr p, long newSize);
void SetHandleSize(Handle h, long newSize);
long GetHandleSize(Handle h);
void ReallocateHandle(Handle h, Size byteCount);
void HLock(Handle h);
void HLockHi(Handle h);
void HUnlock(Handle h);
OSErr HandToHand(Handle* h);
OSErr HandAndHand(Handle h1, Handle h2);
OSErr PtrAndHand(const void* ptr1, Handle hand2, long size);

#ifdef __cplusplus
}
#endif

#endif
