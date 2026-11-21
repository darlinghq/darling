/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * NSData's no-copy constructors and who frees the bytes. Per the NSData
 * documentation, +dataWithBytesNoCopy:length: and
 * -initWithBytesNoCopy:length: take ownership of a malloc'ed buffer and free
 * it when the object is deallocated; the freeWhenDone: variants free it only
 * when asked to. NSMutableData inherits the same constructors.
 *
 * Each case hands an 8 MB malloc'ed buffer to the constructor and releases
 * the object, then checks through malloc_zone_statistics whether the
 * buffer's memory was given back.
 *
 * No ARC.
 */
#import <Foundation/Foundation.h>
#include <malloc/malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE (8u << 20)

static int failures;

static size_t bytesInUse(void)
{
	malloc_statistics_t stats;
	malloc_zone_statistics(NULL, &stats);
	return stats.size_in_use;
}

enum constructor {
	DATA_WITH_BYTES_NO_COPY,
	INIT_WITH_BYTES_NO_COPY,
	DATA_WITH_FREE_WHEN_DONE_YES,
	DATA_WITH_FREE_WHEN_DONE_NO,
	MUTABLE_DATA_WITH_BYTES_NO_COPY,
};

static NSData *make(enum constructor constructor, void *bytes)
{
	switch (constructor) {
	case DATA_WITH_BYTES_NO_COPY:
		return [NSData dataWithBytesNoCopy: bytes length: BUFFER_SIZE];
	case INIT_WITH_BYTES_NO_COPY:
		return [[[NSData alloc] initWithBytesNoCopy: bytes length: BUFFER_SIZE] autorelease];
	case DATA_WITH_FREE_WHEN_DONE_YES:
		return [NSData dataWithBytesNoCopy: bytes length: BUFFER_SIZE freeWhenDone: YES];
	case DATA_WITH_FREE_WHEN_DONE_NO:
		return [NSData dataWithBytesNoCopy: bytes length: BUFFER_SIZE freeWhenDone: NO];
	case MUTABLE_DATA_WITH_BYTES_NO_COPY:
		return [NSMutableData dataWithBytesNoCopy: bytes length: BUFFER_SIZE];
	}
	return nil;
}

static void checkConstructor(const char *name, enum constructor constructor, BOOL freesBytes)
{
	void *bytes = malloc(BUFFER_SIZE);
	if (bytes == NULL) {
		printf("FAIL: %s: cannot allocate the buffer\n", name);
		failures++;
		return;
	}
	memset(bytes, 0x5a, BUFFER_SIZE);
	size_t before = bytesInUse();

	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSData *data = make(constructor, bytes);
	BOOL sameBytes = data != nil && [data length] == BUFFER_SIZE &&
	                 ((const uint8_t *) [data bytes])[BUFFER_SIZE - 1] == 0x5a;
	[pool release];

	size_t after = bytesInUse();
	BOOL freed = after + BUFFER_SIZE / 2 < before;
	if (!freed)
		free(bytes);

	if (!sameBytes) {
		printf("FAIL: %s: the object does not hold the bytes\n", name);
		failures++;
	} else if (freed != freesBytes) {
		printf("FAIL: %s: the buffer was %s (in use %zu -> %zu bytes)\n", name,
		       freed ? "freed" : "not freed", before, after);
		failures++;
	} else {
		printf("PASS: %s %s the buffer\n", name, freed ? "frees" : "keeps");
	}
}

int main(void)
{
	checkConstructor("+dataWithBytesNoCopy:length:", DATA_WITH_BYTES_NO_COPY, YES);
	checkConstructor("-initWithBytesNoCopy:length:", INIT_WITH_BYTES_NO_COPY, YES);
	checkConstructor("+dataWithBytesNoCopy:length:freeWhenDone:YES", DATA_WITH_FREE_WHEN_DONE_YES, YES);
	checkConstructor("+dataWithBytesNoCopy:length:freeWhenDone:NO", DATA_WITH_FREE_WHEN_DONE_NO, NO);
	checkConstructor("+[NSMutableData dataWithBytesNoCopy:length:]", MUTABLE_DATA_WITH_BYTES_NO_COPY, YES);

	printf("%s\n", failures == 0 ? "PASS" : "FAIL");
	return failures == 0 ? 0 : 1;
}
