#ifndef LKM_API_H
#define LKM_API_H

#define DARLING_MACH_API_VERSION		1
#define DARLING_MACH_API_VERSION_STR	"1"

#define DARLING_MACH_API_BASE		0x1000

enum { NR_get_api_version = DARLING_MACH_API_BASE,
	NR_mach_reply_port,
	NR__kernelrpc_mach_port_mod_refs,
	NR_task_self_trap,
	NR_host_self_trap,
	NR__kernelrpc_mach_port_allocate,
	NR_mach_msg_overwrite_trap,
	NR__kernelrpc_mach_port_deallocate,
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

struct mach_msg_overwrite_args
{
	void* msg;
	unsigned int option;
	unsigned int send_size;
	unsigned int recv_size;
	unsigned int rcv_name;
	unsigned int timeout;
	unsigned int notify;
	void* rcv_msg;
	unsigned int rcv_limit;
};

struct mach_port_deallocate_args
{
	unsigned int task_right_name;
	unsigned int port_right_name;
};

#endif
