#include <commpage.h>
#include <sys/mman.h>

//uint8_t __commpage[_COMM_PAGE_AREA_LENGTH];

void _darling_initialize_commpage(void)
{
	mmap(_COMM_PAGE_BASE_ADDRESS, _COMM_PAGE_AREA_LENGTH, PROT_READ|PROT_WRITE,
		MAP_ANON | MAP_PRIVATE, -1, 0);

	*((uint8_t*)_COMM_PAGE_NCPUS) = 1;
	*((uint8_t*)_COMM_PAGE_VERSION) = 12;
	*((uint64_t*) _COMM_PAGE_MEMORY_SIZE) = 1024LL*1024LL*1024LL;
}

int _get_cpu_capabilities(void)
{
	const uint16_t* p = (uint16_t*) _COMM_PAGE_CPU_CAPABILITIES;
	return *p;
}

