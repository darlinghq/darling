/*
This file is part of Darling.

Copyright (C) 2026 Lubos Dolezel

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

#ifndef _MLDR_SOCKET_BITMAP_H_
#define _MLDR_SOCKET_BITMAP_H_

#include <pthread.h>
#include <stddef.h>
#include <stdint.h>

// Hands out file descriptor numbers counting down from the top of RLIMIT_NOFILE,
// so that mldr's own sockets and pipes stay out of the way of the guest's descriptors.
typedef struct socket_bitmap {
	pthread_mutex_t mutex;
	/**
	 * This is always next lowest available index.
	 * If this is equal to #bit_length, then the bitmap is full.
	 */
	size_t next_index;
	uint8_t* bits;
	size_t bit_length;
	int highest;
} socket_bitmap_t;

int socket_bitmap_get(socket_bitmap_t* bitmap);
void socket_bitmap_put(socket_bitmap_t* bitmap, int socket);

// Duplicates `fd` onto a descriptor number from the bitmap and returns it, or -1 on
// failure. `fd` itself is left open. A number that is already in use, for example by
// a native library, is never replaced.
int socket_bitmap_place(socket_bitmap_t* bitmap, int fd);

// Called around fork() so that the child gets a consistent copy of the bitmap
// and an unlocked mutex.
void socket_bitmap_fork_prepare(socket_bitmap_t* bitmap);
void socket_bitmap_fork_parent(socket_bitmap_t* bitmap);
void socket_bitmap_fork_child(socket_bitmap_t* bitmap);

#endif // _MLDR_SOCKET_BITMAP_H_
