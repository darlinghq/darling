#ifndef MACH_TASK_H
#define MACH_TASK_H
#include <sys/types.h>
#include <mach/kern_return.h>

struct darwin_task
{
	darwin_task(pid_t p) : pid(p) {}
	pid_t pid;
};

typedef darwin_task* darwin_task_t;

typedef unsigned int     natural_t;
typedef int              integer_t;

typedef natural_t        mach_port_name_t;
typedef mach_port_name_t mach_port_t;
typedef mach_port_t      ipc_space_t;
typedef natural_t        mach_port_right_t;
typedef integer_t        mach_port_delta_t;
typedef mach_port_t      task_name_t;
typedef natural_t        task_flavor_t;
typedef integer_t       *task_info_t;
typedef natural_t        mach_msg_type_number_t;
typedef mach_port_t      vm_map_t;
typedef uint64_t         mach_vm_address_t;
typedef uint64_t         mach_vm_size_t;
typedef int             *vm_region_recurse_info_t;

#ifdef __cplusplus
extern "C"
{
#endif
	extern darwin_task_t mach_task_self_;
	inline darwin_task_t mach_task_self() { return mach_task_self_; }
	kern_return_t mach_port_deallocate(darwin_task_t task, void* port);
	kern_return_t pid_for_task(mach_port_name_t t, int *pid);
	kern_return_t mach_port_mod_refs(ipc_space_t task, mach_port_name_t name, mach_port_right_t right, mach_port_delta_t delta);
	kern_return_t task_info(task_name_t target_task, task_flavor_t flavor, task_info_t task_info_out, mach_msg_type_number_t *task_info_outCnt);

	// mach_vm.h

#ifdef __cplusplus
}
#endif

#endif
