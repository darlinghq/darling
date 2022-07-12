#include "table.h"
#include <libsimple/lock.h>
#include <stddef.h>
#include <errno.h>
#include "../unistd/close.h"

#define MAX_GUARD_COUNT 1024

#define GUARD_ENTRY_EMPTY_FD (-1)
#define GUARD_ENTRY_DELETED_FD (-2)

typedef struct guard_entry {
	int fd;
	guard_flags_t flags;
	guard_entry_options_t options;
} guard_entry_t;

extern void* memcpy(void* dest, const void* src, size_t n);
extern void* memset(void* dest, int val, size_t n);

// this is a linearly-probed, open-addressed hash table;
// the hash is the FD number itself.
//
// basically, we find the optimal index for the FD, and if it's occupied,
// we keep going until we find an unoccupied slot, wrapping around
// if necessary (until we return to the optimal index).
//
// when deleting, we mark entries as deleted rather than empty.
// yes, this incurs a performance penalty, but:
//   1) we shouldn't ever have collisions, really. most systems limit userspace to 1024 FDs.
//   2) we don't expect to be guarding too many descriptors (mainly just the darlingserver RPC descriptors).
//   3) the additional complexity of eager deletion just isn't worth it (vs lazy deletion, which is what we're doing).
static guard_entry_t guard_table[MAX_GUARD_COUNT] = {
	[0 ... MAX_GUARD_COUNT - 1] = {
		.fd = GUARD_ENTRY_EMPTY_FD,
		.flags = guard_flag_none,
	},
};
// TODO: this should be an RW lock
static libsimple_lock_t guard_table_lock = LIBSIMPLE_LOCK_INITIALIZER;

// normal search
static guard_entry_t* guard_table_find_locked(int fd) {
	size_t start_index = fd % MAX_GUARD_COUNT;

	// check the optimal index first, then sequentially until the end of the table
	for (size_t i = start_index; i < MAX_GUARD_COUNT; ++i) {
		guard_entry_t* entry = &guard_table[i];

		if (entry->fd == fd) {
			return entry;
		}

		if (entry->fd == GUARD_ENTRY_EMPTY_FD) {
			return NULL;
		}
	}

	// rinse and repeat starting from index 0
	for (size_t i = 0; i < start_index; ++i) {
		guard_entry_t* entry = &guard_table[i];

		if (entry->fd == fd) {
			return entry;
		}

		if (entry->fd == GUARD_ENTRY_EMPTY_FD) {
			return NULL;
		}
	}

	// the table was full and our FD wasn't there
	return NULL;
};

// insertion search
static guard_entry_t* guard_table_find_available_locked(int fd) {
	size_t start_index = fd % MAX_GUARD_COUNT;

	// check the optimal index first, then sequentially until the end of the table
	for (size_t i = start_index; i < MAX_GUARD_COUNT; ++i) {
		guard_entry_t* entry = &guard_table[i];

		if (entry->fd == fd || entry->fd == GUARD_ENTRY_EMPTY_FD || entry->fd == GUARD_ENTRY_DELETED_FD) {
			return entry;
		}
	}

	// rinse and repeat starting from index 0
	for (size_t i = 0; i < start_index; ++i) {
		guard_entry_t* entry = &guard_table[i];

		if (entry->fd == fd || entry->fd == GUARD_ENTRY_EMPTY_FD || entry->fd == GUARD_ENTRY_DELETED_FD) {
			return entry;
		}
	}

	// the table was full and we had no available slot for our FD
	return NULL;
};

int guard_table_add(int fd, guard_flags_t flags, guard_entry_options_t* options) {
#ifdef VARIANT_DYLD
	return -ENOSYS;
#else
	int result = 0;

	libsimple_lock_lock(&guard_table_lock);

	guard_entry_t* entry = guard_table_find_locked(fd);
	if (entry) {
		result = -EEXIST;
	} else {
		entry = guard_table_find_available_locked(fd);
		if (entry) {
			entry->fd = fd;
			entry->flags = flags;
			if (options) {
				memcpy(&entry->options, options, sizeof(entry->options));
			} else {
				memset(&entry->options, 0, sizeof(entry->options));
			}
		} else {
			result = -ENOMEM;
		}
	}

	libsimple_lock_unlock(&guard_table_lock);

	return result;
#endif
};

int guard_table_modify(int fd, guard_flags_t flags, guard_entry_options_t* options) {
#ifdef VARIANT_DYLD
	return -ENOSYS;
#else
	int result = 0;

	libsimple_lock_lock(&guard_table_lock);

	guard_entry_t* entry = guard_table_find_locked(fd);
	if (entry) {
		entry->flags = flags;
		if (options) {
			memcpy(&entry->options, options, sizeof(entry->options));
		} else {
			memset(&entry->options, 0, sizeof(entry->options));
		}
	} else {
		result = -ENOENT;
	}

	libsimple_lock_unlock(&guard_table_lock);

	return result;
#endif
};

int guard_table_remove(int fd) {
#ifdef VARIANT_DYLD
	return -ENOSYS;
#else
	int result = 0;

	libsimple_lock_lock(&guard_table_lock);

	guard_entry_t* entry = guard_table_find_locked(fd);
	if (entry) {
		entry->fd = GUARD_ENTRY_DELETED_FD;
		entry->flags = guard_flag_none;
	} else {
		result = -ENOENT;
	}

	libsimple_lock_unlock(&guard_table_lock);

	return result;
#endif
};

int guard_table_check(int fd, guard_flags_t flags) {
#ifdef VARIANT_DYLD
	return 0;
#else
	int result = 0;
	libsimple_lock_lock(&guard_table_lock);
	guard_entry_t* entry = guard_table_find_locked(fd);
	result = entry && (entry->flags & flags) != 0;
	libsimple_lock_unlock(&guard_table_lock);
	return result;
#endif
};

void guard_table_postfork_child(void) {
#ifndef VARIANT_DYLD
	// after forking, the lock is invalid in the child; let's reinitialize it
	libsimple_lock_init(&guard_table_lock);

	// now let's close any descriptors marked as close-on-fork
	for (size_t i = 0; i < MAX_GUARD_COUNT; ++i) {
		guard_entry_t* entry = &guard_table[i];
		if (entry->fd >= 0 && (entry->flags & guard_flag_close_on_fork) != 0) {
			// FIXME: if the FD is in a kqueue, this won't notify the kqueue
			if (entry->options.close) {
				entry->options.close(entry->fd);
			} else {
				close_internal(entry->fd);
			}
			entry->fd = GUARD_ENTRY_DELETED_FD;
			entry->flags = guard_flag_none;
		}
	}
#endif
};
