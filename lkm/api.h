#ifndef LKM_API_H
#define LKM_API_H

#define DARLING_MACH_API_VERSION	1

#define DARLING_MACH_API_BASE		0x1000

enum { NR_get_api_version = DARLING_MACH_API_BASE,
	NR_mach_reply_port,
	NR__kernelrpc_mach_port_mod_refs,
	NR_task_self_trap,
	NR__kernelrpc_mach_port_allocate
};

struct mach_port_mod_refs_args
{
	unsigned int task_right_name;
	unsigned int port_right_name;
	int right_type;
	int delta;
};

struct mach_port_allocate_args
{
	unsigned int task_right_name;
	int right_type;
	unsigned int out_right_name;
};

#endif
