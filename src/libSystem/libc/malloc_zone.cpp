#include "malloc_zone.h"
#include <malloc.h>

static uint32_t dummy = 0xbaadf00d;

malloc_zone_t* malloc_create_zone(vm_size_t start_size, unsigned flags)
{
	return malloc_default_zone();
}

void malloc_destroy_zone(malloc_zone_t *zone)
{
}

malloc_zone_t* malloc_default_zone()
{
	return &dummy;
}

malloc_zone_t* malloc_zone_from_ptr(const void *ptr)
{
	return malloc_default_zone();
}

void* malloc_zone_malloc(malloc_zone_t *zone, size_t size)
{
	return malloc(size);
}

void* malloc_zone_calloc(malloc_zone_t *zone, size_t num_items, size_t size)
{
	return calloc(num_items, size);
}

void* malloc_zone_valloc(malloc_zone_t *zone, size_t size)
{
	return valloc(size);
}

void* malloc_zone_realloc(malloc_zone_t *zone, void *ptr, size_t size)
{
	return realloc(ptr, size);
}

void* malloc_zone_memalign(malloc_zone_t *zone, size_t alignment, size_t size)
{
	return memalign(alignment, size);
}

void malloc_zone_free(malloc_zone_t *zone, void *ptr)
{
	return free(ptr);
}

void malloc_zone_statistics(malloc_zone_t* zone, __darwin_malloc_statistics_t* stats)
{
	struct mallinfo info = mallinfo();
	
	// No idea if this is even nearly correct
	stats->size_allocated = info.uordblks;
	stats->size_in_use = info.hblkhd;
	stats->max_size_in_use = info.usmblks;
	stats->blocks_in_use = stats->size_in_use;
}

