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
#include <CoreServices/MacErrors.h>
#include <cstring>
#include <stdint.h>
#include <cstdlib>
#include <cstdio>
#include <malloc/malloc.h>

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
	return (Ptr) malloc(len);
}

Ptr NewPtrClear(long len)
{
	return (Ptr) calloc(1, len);
}

void DisposePtr(Ptr p)
{
	free(p);
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
	Handle h = (Handle) malloc(sizeof(*h));
	*h = NewPtr(size);
	return h;
}

Handle NewHandleClear(long size)
{
	Handle h = (Handle) malloc(sizeof(*h));
	*h = NewPtrClear(size);
	return h;
}

Handle NewEmptyHandle(void)
{
	Handle h = (Handle) malloc(sizeof(*h));
	*h = nullptr;
	return h;
}

long GetPtrSize(Ptr ptr)
{
	return malloc_size(ptr);
}

void SetPtrSize(Ptr ptr, long newSize)
{
	if (newSize < malloc_size(ptr))
		realloc(ptr, newSize); // Let's hope this never changes the ptr, but the whole SetPtrSize() call is idiotic
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
		*h = (Ptr) realloc(*h, newSize);
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

OSErr HandToHand(Handle* h)
{
	if (!h)
		return paramErr;
	if (!*h)
	{
		*h = NewEmptyHandle();
		return noErr;
	}

	Handle src = *h;
	long size = GetHandleSize(src);
	Handle newHandle = NewHandle(size);

	memcpy(*newHandle, *src, size);
	*h = newHandle;

	return noErr;
}

// Append h1 to h2
OSErr HandAndHand(Handle h1, Handle h2)
{
	if (!h1 || !h2)
		return paramErr;
	if (!*h1)
		return noErr;
	long origSize = GetHandleSize(h2);
	long addSize = GetHandleSize(h1);

	SetHandleSize(h2, origSize + addSize);
	memcpy(*h2 + origSize, *h1, addSize);

	return noErr;
}

OSErr PtrAndHand(const void* ptr1, Handle hand2, long size)
{
	if (!size)
		return noErr;
	if (!ptr1 || !hand2)
		return paramErr;
	
	long origSize = GetHandleSize(hand2);
	SetHandleSize(hand2, origSize + size);

	memcpy(*hand2 + origSize, ptr1, size);
	return noErr;
}
