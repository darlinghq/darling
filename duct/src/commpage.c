#include <commpage.h>

uint8_t __commpage[_COMM_PAGE_AREA_LENGTH];

__attribute__((constructor))
void _darling_initialize_commpage(void)
{
	*(_COMM_PAGE_NCPUS) = 1;
	*(_COMM_PAGE_VERSION) = 12;
	*((uint64_t*) _COMM_PAGE_MEMORY_SIZE) = 1024LL*1024LL*1024LL;
}

