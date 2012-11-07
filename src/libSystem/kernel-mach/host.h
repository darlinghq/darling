#ifndef MACH_HOST_H
#define MACH_HOST_H
#include <stdint.h>
#include <mach/kern_return.h>
#include <mach/i386/vm_types.h>
#include <mach/host_info.h>

struct host
{
	long dummy;
};

#ifdef __cplusplus
extern "C"
{
#endif

typedef host* host_t;
typedef long host_priv_t;


kern_return_t host_page_size(host_t host, vm_size_t* size);
kern_return_t host_info(host_t host, int flavor, host_basic_info* info, long* info_cnt);
kern_return_t host_statistics(host_t host, int flavor, void* out, long* out_cnt);
host_t mach_host_self();

#ifdef __cplusplus
}
#endif

#endif
