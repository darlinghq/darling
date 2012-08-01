
#include "host.h"
#include <unistd.h>

kern_return_t host_page_size(void* host, size_t* size)
{
  *size = getpagesize();
  return KERN_SUCCESS;
}
