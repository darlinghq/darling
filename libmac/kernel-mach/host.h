#ifndef MACH_HOST_H
#define MACH_HOST_H
#include <mach/kern_return.h>
#include <mach/i386/vm_types.h>

extern "C"
{
  
typedef long host_t;
typedef long host_priv_t;

kern_return_t host_page_size(host_t host, vm_size_t* size);

}

#endif
