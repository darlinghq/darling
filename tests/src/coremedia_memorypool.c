// CFLAGS: -framework CoreMedia -framework CoreFoundation
/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

// CMMemoryPool: a CFAllocator that holds on to recently deallocated memory for the next allocation of the
// same size.
// The documented contract (public CMMemoryPool.h and the pages below): the pool's allocator is the CFAllocator
// to allocate from; memory deallocated through it is held for reuse, in whole pages; CMMemoryPoolFlush frees
// what is held; after CMMemoryPoolInvalidate the allocator allocates and deallocates with no pooling; the
// allocator can outlive the pool, because every object allocated with it retains it.
// https://developer.apple.com/documentation/coremedia/cmmemorypoolcreate(options:)
// https://developer.apple.com/documentation/coremedia/cmmemorypoolgetallocator(_:)
// https://developer.apple.com/documentation/coremedia/cmmemorypoolflush(_:)
// https://developer.apple.com/documentation/coremedia/cmmemorypoolinvalidate(_:)
// That a block is reused is seen as the same address coming back for an allocation of the same size; that
// the size is counted in pages (a size within the same pages gets the same block) as observed on macOS 26.7.3 and
// 10.14.4 (the documented fact is that the pool allocates in whole pages). The value of the key
// kCMMemoryPoolOption_AgeOutPeriod is the one those systems give the symbol.
// Prints PASS when everything holds; the exit code is the number of failed checks.

#include <CoreMedia/CoreMedia.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { failures++; printf("FAIL line %d: %s\n", __LINE__, #cond); } } while (0)

static int filled(const unsigned char *bytes, size_t length, unsigned char value)
{
	for (size_t i = 0; i < length; i++)
		if (bytes[i] != value)
			return 0;
	return 1;
}

int main(void)
{
	const size_t page = (size_t)sysconf(_SC_PAGESIZE);
	const size_t big = 64 * page - 64;

	// creation: a CF object of its own type with an allocator of its own
	CMMemoryPoolRef pool = CMMemoryPoolCreate(NULL);
	CHECK(pool != NULL && CFGetTypeID(pool) == CMMemoryPoolGetTypeID());
	CHECK(CFEqual(kCMMemoryPoolOption_AgeOutPeriod, CFSTR("FigMemoryPool_AgeOutPeriod")));
	CFAllocatorRef allocator = CMMemoryPoolGetAllocator(pool);
	CHECK(allocator != NULL && CMMemoryPoolGetAllocator(pool) == allocator);
	CHECK(allocator != kCFAllocatorDefault && allocator != kCFAllocatorSystemDefault);
	CMMemoryPoolRef second = CMMemoryPoolCreate(NULL);
	CHECK(second != NULL && CMMemoryPoolGetAllocator(second) != allocator);
	CFRelease(second);

	// the memory is usable in full, and live blocks are distinct
	unsigned char *a = CFAllocatorAllocate(allocator, (CFIndex)big, 0);
	unsigned char *b = CFAllocatorAllocate(allocator, (CFIndex)big, 0);
	CHECK(a != NULL && b != NULL && a != b);
	memset(a, 0xA5, big);
	memset(b, 0x5A, big);
	CHECK(filled(a, big, 0xA5) && filled(b, big, 0x5A));

	// a block deallocated through the pool comes back for the next allocation of the same size, and not for
	// one of a size in other pages
	CFAllocatorDeallocate(allocator, a);
	unsigned char *other = CFAllocatorAllocate(allocator, (CFIndex)(2 * big), 0);
	CHECK(other != NULL && other != a);
	unsigned char *again = CFAllocatorAllocate(allocator, (CFIndex)big, 0);
	CHECK(again == a);
	CFAllocatorDeallocate(allocator, other);

	// sizes within the same pages share a block (observed on macOS)
	CFAllocatorDeallocate(allocator, again);
	unsigned char *nearby = CFAllocatorAllocate(allocator, (CFIndex)(big - 100), 0);
	CHECK(nearby == a);
	CFAllocatorDeallocate(allocator, nearby);

	// reallocation keeps the contents, growing and shrinking
	unsigned char *grown = CFAllocatorReallocate(allocator, b, (CFIndex)(4 * big), 0);
	CHECK(grown != NULL && filled(grown, big, 0x5A));
	memset(grown + big, 0x77, 3 * big);
	unsigned char *shrunk = CFAllocatorReallocate(allocator, grown, (CFIndex)(page / 2), 0);
	CHECK(shrunk != NULL && filled(shrunk, page / 2, 0x5A));
	CFAllocatorDeallocate(allocator, shrunk);

	// a flush frees what is held and leaves the pool working
	CMMemoryPoolFlush(pool);
	unsigned char *afterFlush = CFAllocatorAllocate(allocator, (CFIndex)big, 0);
	CHECK(afterFlush != NULL);
	memset(afterFlush, 1, big);
	CFAllocatorDeallocate(allocator, afterFlush);
	CMMemoryPoolFlush(pool);
	CMMemoryPoolFlush(pool);

	// the allocator outlives the pool; a block allocated before the pool went away is freed after it
	unsigned char *survivor = CFAllocatorAllocate(allocator, (CFIndex)big, 0);
	CHECK(survivor != NULL);
	CFRetain(allocator);
	CFRelease(pool);
	memset(survivor, 2, big);
	CFAllocatorDeallocate(allocator, survivor);
	unsigned char *orphan = CFAllocatorAllocate(allocator, (CFIndex)big, 0);
	CHECK(orphan != NULL);
	memset(orphan, 3, big);
	CFAllocatorDeallocate(allocator, orphan);
	CFRelease(allocator);

	// after an invalidation the allocator still allocates, reallocates and deallocates, with no pooling; a block
	// from before the invalidation can be deallocated after it
	CMMemoryPoolRef invalidated = CMMemoryPoolCreate(NULL);
	CFAllocatorRef plain = CMMemoryPoolGetAllocator(invalidated);
	unsigned char *before = CFAllocatorAllocate(plain, (CFIndex)big, 0);
	unsigned char *heldBefore = CFAllocatorAllocate(plain, (CFIndex)big, 0);
	CFAllocatorDeallocate(plain, heldBefore);
	CMMemoryPoolInvalidate(invalidated);
	CMMemoryPoolInvalidate(invalidated);
	CHECK(CMMemoryPoolGetAllocator(invalidated) == plain);
	memset(before, 9, big);
	unsigned char *after = CFAllocatorAllocate(plain, (CFIndex)big, 0);
	CHECK(after != NULL);
	memset(after, 8, big);
	after = CFAllocatorReallocate(plain, after, (CFIndex)(2 * big), 0);
	CHECK(after != NULL && filled(after, big, 8));
	CFAllocatorDeallocate(plain, after);
	CFAllocatorDeallocate(plain, before);
	CFRelease(invalidated);

	// the age-out option: a block deallocated through a pool that keeps blocks for 30 s is still there after
	// the default period of 0.5 s has passed; an option that is not a usable number leaves the default
	double longPeriod = 30.0;
	CFNumberRef longNumber = CFNumberCreate(kCFAllocatorDefault, kCFNumberFloat64Type, &longPeriod);
	CFDictionaryRef longOptions = CFDictionaryCreate(kCFAllocatorDefault, (const void *[]){ kCMMemoryPoolOption_AgeOutPeriod },
		(const void *[]){ longNumber }, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CMMemoryPoolRef patient = CMMemoryPoolCreate(longOptions);
	CHECK(patient != NULL);
	CFAllocatorRef patientAllocator = CMMemoryPoolGetAllocator(patient);
	unsigned char *kept = CFAllocatorAllocate(patientAllocator, (CFIndex)big, 0);
	CFAllocatorDeallocate(patientAllocator, kept);
	struct timespec wait = { 0, 700 * 1000 * 1000 };
	nanosleep(&wait, NULL);
	unsigned char *keptAgain = CFAllocatorAllocate(patientAllocator, (CFIndex)big, 0);
	CHECK(keptAgain == kept);
	CFAllocatorDeallocate(patientAllocator, keptAgain);
	CFRelease(patient);
	CFRelease(longOptions);
	CFRelease(longNumber);
	CFDictionaryRef notNumberOptions = CFDictionaryCreate(kCFAllocatorDefault, (const void *[]){ kCMMemoryPoolOption_AgeOutPeriod },
		(const void *[]){ CFSTR("soon") }, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CMMemoryPoolRef fallback = CMMemoryPoolCreate(notNumberOptions);
	CHECK(fallback != NULL && CMMemoryPoolGetAllocator(fallback) != NULL);
	CFRelease(fallback);
	CFRelease(notNumberOptions);

	if (failures == 0)
		printf("PASS\n");
	return failures;
}
