#ifndef LKM_API_H
#define LKM_API_H

#define DARLING_MACH_API_VERSION	1

#define DARLING_MACH_API_BASE		0x1000

enum { NR_get_api_version = DARLING_MACH_API_BASE, NR_mach_reply_port, NR__kernelrpc_mach_port_mod_refs };

struct mach_port_mod_refs_args
{
	long task_right_name;
	long port_right_name;
	int right_type;
	int delta;
};

#endif
