#include "config.h"
#include "host.h"
#include "trace.h"
#include <unistd.h>

kern_return_t host_page_size(void* host, size_t* size)
{
	TRACE2(host, size);
	*size = getpagesize();
	return KERN_SUCCESS;
}
