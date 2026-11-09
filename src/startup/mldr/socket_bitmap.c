/*
This file is part of Darling.

Copyright (C) 2017 Lubos Dolezel

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

#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <unistd.h>
#include "socket_bitmap.h"

int socket_bitmap_get(socket_bitmap_t* bitmap) {
	int fd = -1;
	bool updated = false;

	pthread_mutex_lock(&bitmap->mutex);

	if (bitmap->highest == -1) {
		// we need to initialize this bitmap
		struct rlimit limit;

		if (getrlimit(RLIMIT_NOFILE, &limit) < 0) {
			goto out;
		}

		if (limit.rlim_cur == RLIM_INFINITY) {
			// just default to 1024
			limit.rlim_cur = 1024;
		}

		bitmap->highest = limit.rlim_cur - 1;
	}

	if (bitmap->next_index >= bitmap->bit_length) {
		// we need to grow the bitmap

		if ((bitmap->bit_length % 8) == 0) {
			// we need to allocate an additional byte

			void* ptr = realloc(bitmap->bits, (bitmap->bit_length / 8) + 1);
			if (!ptr) {
				goto out;
			}

			bitmap->bits = ptr;

			bitmap->bits[bitmap->bit_length / 8] = 0;
		} else {
			// we just need to increment the bit length
		}

		++bitmap->bit_length;
	}

	fd = bitmap->highest - bitmap->next_index;

	bitmap->bits[bitmap->next_index / 8] |= 1 << (bitmap->next_index % 8);

	// update the next available index
	for (size_t i = bitmap->next_index + 1; i < bitmap->bit_length; ++i) {
		size_t byte = i / 8;
		uint8_t bit = i % 8;

		if (bit == 0) {
			// check the entire byte at once so we can avoid unnecessary iteration
			if (bitmap->bits[byte] == 0xff) {
				// this byte is full, skip it
				i += 7;
				continue;
			}
		}

		if ((bitmap->bits[byte] & (1 << bit)) == 0) {
			// this index is unused
			bitmap->next_index = i;
			updated = true;
			break;
		}
	}

	if (!updated) {
		// all of our entries are currently in-use
		bitmap->next_index = bitmap->bit_length;
	}

out:
	pthread_mutex_unlock(&bitmap->mutex);

	return fd;
};

void socket_bitmap_put(socket_bitmap_t* bitmap, int socket) {
	size_t index;

	pthread_mutex_lock(&bitmap->mutex);

	if (socket > bitmap->highest || bitmap->highest - socket >= bitmap->bit_length) {
		// not a descriptor we handed out
		goto out;
	}

	index = bitmap->highest - socket;

	bitmap->bits[index / 8] &= ~(1 << (index % 8));

	if (index < bitmap->next_index) {
		bitmap->next_index = index;
	}

	if (index == bitmap->bit_length - 1) {
		// we can shrink the bitmap
		size_t old_byte_size = (bitmap->bit_length + 7) / 8;
		size_t new_byte_size = old_byte_size;

		while (bitmap->bit_length > 0) {
			size_t index = bitmap->bit_length - 1;

			if ((bitmap->bit_length % 8) == 0) {
				// check the entire byte at once to avoid unnecessary iteration
				if (bitmap->bits[(bitmap->bit_length / 8) - 1] == 0) {
					// remove this entire byte
					bitmap->bit_length -= 8;
					continue;
				}
			}

			if ((bitmap->bits[index / 8] & (1 << (index % 8))) != 0) {
				// this bit is in-use, so we can't shrink any further
				break;
			}

			--bitmap->bit_length;
		}

		new_byte_size = (bitmap->bit_length + 7) / 8;

		if (new_byte_size == 0) {
			free(bitmap->bits);
			bitmap->bits = NULL;
		} else if (old_byte_size != new_byte_size) {
			// we can free one or more bytes from the bitmap
			void* ptr = realloc(bitmap->bits, new_byte_size);
			if (!ptr) {
				goto out;
			}

			bitmap->bits = ptr;
		}
	}

out:
	pthread_mutex_unlock(&bitmap->mutex);
};

int socket_bitmap_place(socket_bitmap_t* bitmap, int fd) {
	while (true) {
		int slot = socket_bitmap_get(bitmap);
		if (slot < 0) {
			return -1;
		}

		// an inherited descriptor may already sit on its slot; F_DUPFD would see the
		// slot as taken and move it off
		if (slot == fd) {
			return slot;
		}

		// unlike dup2(), F_DUPFD never closes a descriptor someone else holds;
		// it picks the lowest free number at or above the slot instead
		int placed = fcntl(fd, F_DUPFD, slot);
		if (placed == slot) {
			return slot;
		}

		if (placed < 0) {
			socket_bitmap_put(bitmap, slot);
			return -1;
		}

		// the slot is held by a descriptor we didn't create;
		// leave it marked as used so it's never handed out, and try the next one
		close(placed);
	}
};

void socket_bitmap_fork_prepare(socket_bitmap_t* bitmap) {
	// Serialize with realloc and bitmap updates before taking the fork snapshot.
	pthread_mutex_lock(&bitmap->mutex);
}

void socket_bitmap_fork_parent(socket_bitmap_t* bitmap) {
	pthread_mutex_unlock(&bitmap->mutex);
}

void socket_bitmap_fork_child(socket_bitmap_t* bitmap) {
	// The forking thread owns the inherited lock; do not reinitialize live state.
	pthread_mutex_unlock(&bitmap->mutex);
}
