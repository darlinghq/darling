// Copyright (C) 2026 Lubos Dolezel
//
// For the mach_sq_* tests that need to reach a state a well-behaved task never
// produces on purpose (a queue word the task keeps busy, a damaged queue): finds
// the shared message queue region of this process (see mach_sq_common.h) in
// /proc/self/maps and the slot of a port in it. Built with darlingserver's
// include directory (darlingserver/mach-sq.h describes the region).

#ifndef MACH_SQ_REGION_H
#define MACH_SQ_REGION_H

#include "mach_sq_common.h"

#include <darlingserver/mach-sq.h>

typedef struct
{
	dsq_s_region_t* s; // read-only for the task
	dsq_q_region_t* q;
} sq_region_t;

static inline void* sq_region_mapping(const char* maps, const char* object)
{
	const char* at = strstr(maps, object);
	if (at == NULL)
		return NULL;
	while (at > maps && at[-1] != '\n')
		at--;
	return (void*) (uintptr_t) strtoull(at, NULL, 16);
}

// false if the process has no region (DARLING_MACH_SERVER=1, or before its first Mach message)
static inline bool sq_region_find(sq_region_t* region)
{
	static char maps[262144];
	int fd = open("/proc/self/maps", O_RDONLY);
	if (fd < 0)
		return false;
	ssize_t total = 0, n;
	while (total < (ssize_t) sizeof(maps) - 1 && (n = read(fd, maps + total, sizeof(maps) - 1 - total)) > 0)
		total += n;
	close(fd);
	maps[total] = '\0';

	region->s = sq_region_mapping(maps, "darling-mach-sq-s");
	region->q = sq_region_mapping(maps, "darling-mach-sq-q");
	return region->s != NULL && region->q != NULL && region->s->hdr.magic == DSQ_MAGIC &&
		region->s->hdr.version == DSQ_VERSION;
}

// the slot of the port or port set `name` while it is in the mode, or -1
static inline int sq_region_slot(const sq_region_t* region, mach_port_name_t name)
{
	for (int i = 0; i < DSQ_NAMES; i++)
	{
		const dsq_name_entry_t* e = &region->s->names[i];
		if (__atomic_load_n(&e->name, __ATOMIC_ACQUIRE) == name && e->slot < DSQ_SLOTS)
			return e->slot;
	}
	return -1;
}

// sets the closed bit of a slot's head as only the server does: what the server finds when the task keeps
// the head busy for longer than it tries, or when the task damages its queue
static inline void sq_region_close_head(const sq_region_t* region, int slot)
{
	__atomic_fetch_or(&region->q->slots[slot].head, (uint64_t) DSQ_CLOSED, __ATOMIC_SEQ_CST);
}

#endif // MACH_SQ_REGION_H
