/*
This file is part of Darling.

Copyright (C) 2020 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <CoreServices/MacMemory.h>
#include <cstring>
#include <stdint.h>
#include <cstdlib>

struct PtrInternal
{
	uintptr_t size;
	uint8_t data[];
};

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

Ptr NewPtr(long len)
{
	PtrInternal* ptr = (PtrInternal*) malloc(len + sizeof(uintptr_t));
	ptr->size = len;
	return (Ptr) ptr->data;
}

Ptr NewPtrClear(long len)
{
	PtrInternal* ptr = (PtrInternal*) calloc(1, len + sizeof(uintptr_t));
	ptr->size = len;
	return (Ptr) ptr->data;
}

void DisposePtr(Ptr p)
{
	free(((uint8_t*)p) - sizeof(uintptr_t));
}

void DisposeHandle(Handle handle)
{
	DisposePtr(*handle);
	free(handle);
}

Size GetHandleSize(Handle h)
{
	if (!*h)
		return 0;
	return GetPtrSize(*h);
}

void EmptyHandle(Handle h)
{
	if (*h)
	{
		DisposePtr(*h);
		*h = nullptr;
	}
}

OSErr PtrToHand(const void* srcPtr, Handle* dstHndl, long size)
{
	*dstHndl = NewHandle(size);
	memcpy(**dstHndl, srcPtr, size);
	return noErr;
}

OSErr PtrToXHand(const void* srcPtr, Handle dstHndl, long size)
{
	SetHandleSize(dstHndl, size);
	memcpy(*dstHndl, srcPtr, size);
	return noErr;
}

Handle NewHandle(long size)
{
	Handle h = (Handle) malloc(sizeof(Handle));
	*h = NewPtr(size);
	return h;
}

Handle NewHandleClear(long size)
{
	Handle h = (Handle) malloc(sizeof(Handle));
	*h = NewPtrClear(size);
	return h;
}

Handle NewEmptyHandle(void)
{
	Handle h = (Handle) malloc(sizeof(Handle));
	*h = nullptr;
	return h;
}

long GetPtrSize(Ptr ptr)
{
	PtrInternal* internal = (PtrInternal*)(((char*) ptr) - sizeof(uintptr_t));
	return internal->size;
}

void SetPtrSize(Ptr ptr, long newSize)
{
	PtrInternal* internal = (PtrInternal*)(((char*) ptr) - sizeof(uintptr_t));
	if (newSize < internal->size)
		internal->size = newSize;
	else
	{
		// This is not possible with a non-relocatable block
	}
}

void SetHandleSize(Handle h, long newSize)
{
	if (!*h)
		*h = NewPtr(newSize);
	else if (!newSize)
	{
		DisposePtr(*h);
		*h = nullptr;
	}
	else
	{
		PtrInternal* internal = (PtrInternal*)(((char*) *h) - sizeof(uintptr_t));

		internal = (PtrInternal*) realloc(internal, newSize + sizeof(uintptr_t));
		internal->size = newSize;

		*h = (Ptr) internal->data;
	}
}

void ReallocateHandle(Handle h, Size byteCount)
{
	SetHandleSize(h, 0);
	SetHandleSize(h, byteCount);
}

void HLock(Handle h)
{
}

void HLockHi(Handle h)
{
}

void HUnlock(Handle h)
{
}
