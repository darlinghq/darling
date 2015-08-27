#define PRIVATE
#include <mach/mach_traps.h>
#include <mach/kern_return.h>
#include <sys/mman.h>

mach_port_name_t mach_reply_port(void)
{
	return 0;
}

mach_port_name_t thread_self_trap(void)
{
	return 0;
}

mach_port_name_t host_self_trap(void)
{
	return 0;
}

mach_msg_return_t mach_msg_trap(
				mach_msg_header_t *msg,
				mach_msg_option_t option,
				mach_msg_size_t send_size,
				mach_msg_size_t rcv_size,
				mach_port_name_t rcv_name,
				mach_msg_timeout_t timeout,
				mach_port_name_t notify)
{
	return KERN_FAILURE;
}

mach_msg_return_t mach_msg_overwrite_trap(
				mach_msg_header_t *msg,
				mach_msg_option_t option,
				mach_msg_size_t send_size,
				mach_msg_size_t rcv_size,
				mach_port_name_t rcv_name,
				mach_msg_timeout_t timeout,
				mach_port_name_t notify,
				mach_msg_header_t *rcv_msg,
				mach_msg_size_t rcv_limit)
{
	return KERN_FAILURE;
}

kern_return_t semaphore_signal_trap(
				mach_port_name_t signal_name)
{
	return KERN_FAILURE;
}
					      
kern_return_t semaphore_signal_all_trap(
				mach_port_name_t signal_name)
{
	return KERN_FAILURE;
}

kern_return_t semaphore_signal_thread_trap(
				mach_port_name_t signal_name,
				mach_port_name_t thread_name)
{
	return KERN_FAILURE;
}

kern_return_t semaphore_wait_trap(
				mach_port_name_t wait_name)
{
	return KERN_FAILURE;
}

kern_return_t semaphore_wait_signal_trap(
				mach_port_name_t wait_name,
				mach_port_name_t signal_name)
{
	return KERN_FAILURE;
}

kern_return_t semaphore_timedwait_trap(
				mach_port_name_t wait_name,
				unsigned int sec,
				clock_res_t nsec)
{
	return KERN_FAILURE;
}

kern_return_t semaphore_timedwait_signal_trap(
				mach_port_name_t wait_name,
				mach_port_name_t signal_name,
				unsigned int sec,
				clock_res_t nsec)
{
	return KERN_FAILURE;
}

kern_return_t clock_sleep_trap(
				mach_port_name_t clock_name,
				sleep_type_t sleep_type,
				int sleep_sec,
				int sleep_nsec,
				mach_timespec_t	*wakeup_time)
{
	return KERN_FAILURE;
}

kern_return_t _kernelrpc_mach_vm_allocate_trap(
				mach_port_name_t target,
				mach_vm_offset_t *addr,
				mach_vm_size_t size,
				int flags)
{
	return _kernelrpc_mach_vm_map_trap(target, addr,
			size, 0, flags, VM_PROT_READ | VM_PROT_WRITE);
}

kern_return_t _kernelrpc_mach_vm_deallocate_trap(
				mach_port_name_t target,
				mach_vm_address_t address,
				mach_vm_size_t size
)
{
	int ret;

	if (target != 0 && target != mach_task_self())
		return KERN_FAILURE;

	ret = munmap(address, size);

	if (ret == -1)
		return KERN_FAILURE;

	return KERN_SUCCESS;
}

kern_return_t _kernelrpc_mach_vm_protect_trap(
				mach_port_name_t target,
				mach_vm_address_t address,
				mach_vm_size_t size,
				boolean_t set_maximum,
				vm_prot_t new_protection
)
{
	int prot = 0;
	int ret;

	if (target != 0 && target != mach_task_self())
		return KERN_FAILURE;

	if (new_protection & VM_PROT_READ)
		prot |= PROT_READ;
	if (new_protection & VM_PROT_WRITE)
		prot |= PROT_WRITE;
	if (new_protection & VM_PROT_EXECUTE)
		prot |= PROT_EXEC;

	ret = mprotect(address, size, prot);
	if (ret == -1)
		return KERN_FAILURE;

	return KERN_SUCCESS;
}

kern_return_t _kernelrpc_mach_vm_map_trap(
				mach_port_name_t target,
				mach_vm_offset_t *address,
				mach_vm_size_t size,
				mach_vm_offset_t mask,
				int flags,
				vm_prot_t cur_protection
)
{
	// We cannot allocate memory in other processes
	if (target != 0 && target != mach_task_self())
		return KERN_FAILURE;

	void* addr;
	int prot = 0;
	int posix_flags = MAP_ANON | MAP_PRIVATE;

	if (cur_protection & VM_PROT_READ)
		prot |= PROT_READ;
	if (cur_protection & VM_PROT_WRITE)
		prot |= PROT_WRITE;
	if (cur_protection & VM_PROT_EXECUTE)
		prot |= PROT_EXEC;

	if (!(flags & VM_FLAGS_ANYWHERE))
		posix_flags |= MAP_FIXED;

	addr = mmap(*address, size, prot, posix_flags, -1, 0);
	if (addr == MAP_FAILED)
	{
		return KERN_FAILURE;
	}

	*address = addr;
	return KERN_SUCCESS;
}

kern_return_t _kernelrpc_mach_port_allocate_trap(
				mach_port_name_t target,
				mach_port_right_t right,
				mach_port_name_t *name
)
{
	return KERN_FAILURE;
}


kern_return_t _kernelrpc_mach_port_destroy_trap(
				mach_port_name_t target,
				mach_port_name_t name
)
{
	return KERN_FAILURE;
}

kern_return_t _kernelrpc_mach_port_deallocate_trap(
				mach_port_name_t target,
				mach_port_name_t name
)
{
	return KERN_FAILURE;
}

kern_return_t _kernelrpc_mach_port_mod_refs_trap(
				mach_port_name_t target,
				mach_port_name_t name,
				mach_port_right_t right,
				mach_port_delta_t delta
)
{
	return KERN_FAILURE;
}

kern_return_t _kernelrpc_mach_port_move_member_trap(
				mach_port_name_t target,
				mach_port_name_t member,
				mach_port_name_t after
)
{
	return KERN_FAILURE;
}

kern_return_t _kernelrpc_mach_port_insert_right_trap(
				mach_port_name_t target,
				mach_port_name_t name,
				mach_port_name_t poly,
				mach_msg_type_name_t polyPoly
)
{
	return KERN_FAILURE;
}

kern_return_t _kernelrpc_mach_port_insert_member_trap(
				mach_port_name_t target,
				mach_port_name_t name,
				mach_port_name_t pset
)
{
	return KERN_FAILURE;
}

kern_return_t _kernelrpc_mach_port_extract_member_trap(
				mach_port_name_t target,
				mach_port_name_t name,
				mach_port_name_t pset
)
{
	return KERN_FAILURE;
}

kern_return_t _kernelrpc_mach_port_construct_trap(
				mach_port_name_t target,
				mach_port_options_t *options,
				uint64_t context,
				mach_port_name_t *name
)
{
	return KERN_FAILURE;
}

kern_return_t _kernelrpc_mach_port_destruct_trap(
				mach_port_name_t target,
				mach_port_name_t name,
				mach_port_delta_t srdelta,
				uint64_t guard
)
{
	return KERN_FAILURE;
}

kern_return_t _kernelrpc_mach_port_guard_trap(
				mach_port_name_t target,
				mach_port_name_t name,
				uint64_t guard,
				boolean_t strict
)
{
	return KERN_FAILURE;
}

kern_return_t _kernelrpc_mach_port_unguard_trap(
				mach_port_name_t target,
				mach_port_name_t name,
				uint64_t guard
)
{
	return KERN_FAILURE;
}

kern_return_t macx_swapon(
				uint64_t filename,
				int flags,
				int size,
				int priority)
{
	return KERN_FAILURE;
}

kern_return_t macx_swapoff(
				uint64_t filename,
				int flags)
{
	return KERN_FAILURE;
}

kern_return_t macx_triggers(
				int hi_water,
				int low_water,
				int flags,
				mach_port_t alert_port)
{
	return KERN_FAILURE;
}

kern_return_t macx_backing_store_suspend(
				boolean_t suspend)
{
	return KERN_FAILURE;
}

kern_return_t macx_backing_store_recovery(
				int pid)
{
	return KERN_FAILURE;
}

boolean_t swtch_pri(int pri)
{
	return 0;
}

boolean_t swtch(void)
{
	return 0;
}

kern_return_t syscall_thread_switch(
				mach_port_name_t thread_name,
				int option,
				mach_msg_timeout_t option_time)
{
	return KERN_FAILURE;
}

mach_port_name_t task_self_trap(void)
{
	return 0;
}

/*
 *	Obsolete interfaces.
 */

kern_return_t task_for_pid(
				mach_port_name_t target_tport,
				int pid,
				mach_port_name_t *t)
{
	return KERN_FAILURE;
}

kern_return_t task_name_for_pid(
				mach_port_name_t target_tport,
				int pid,
				mach_port_name_t *tn)
{
	return KERN_FAILURE;
}

kern_return_t pid_for_task(
				mach_port_name_t t,
				int *x)
{
	return KERN_FAILURE;
}

