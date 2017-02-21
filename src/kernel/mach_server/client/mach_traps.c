#define ioctl __real_ioctl
#include <mach/mach_traps.h>
#include <mach/vm_statistics.h>
#include <mach/kern_return.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "../../lkm/api.h"
#include "lkm.h"

#define UNIMPLEMENTED_TRAP() { char msg[] = "Called unimplemented Mach trap: "; write(2, msg, sizeof(msg)-1); write(2, __FUNCTION__, sizeof(__FUNCTION__)-1); write(2, "\n", 1); }

mach_port_name_t mach_reply_port(void)
{
	return ioctl(driver_fd, NR_mach_reply_port, 0);
}

mach_port_name_t thread_self_trap(void)
{
	return ioctl(driver_fd, NR_thread_self_trap, 0);
}

mach_port_name_t host_self_trap(void)
{
	return ioctl(driver_fd, NR_host_self_trap, 0);
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
	return mach_msg_overwrite_trap(msg,
			option, send_size, rcv_size,
			rcv_name, timeout, notify,
			msg, 0);
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
	struct mach_msg_overwrite_args args = {
		.msg = msg,
		.option = option,
		.send_size = send_size,
		.recv_size = rcv_size,
		.rcv_name = rcv_name,
		.timeout = timeout,
		.notify = notify,
		.rcv_msg = rcv_msg,
		.rcv_limit = rcv_limit
	};

	return ioctl(driver_fd, NR_mach_msg_overwrite_trap,
			&args);
}

kern_return_t semaphore_signal_trap(
				mach_port_name_t signal_name)
{
	struct semaphore_signal_args args = {
		.signal = signal_name
	};

	return ioctl(driver_fd, NR_semaphore_signal_trap, &args);
}
					      
kern_return_t semaphore_signal_all_trap(
				mach_port_name_t signal_name)
{
	struct semaphore_signal_all_args args = {
		.signal = signal_name
	};

	return ioctl(driver_fd, NR_semaphore_signal_all_trap,
			&args);
}

kern_return_t semaphore_signal_thread_trap(
				mach_port_name_t signal_name,
				mach_port_name_t thread_name)
{
	UNIMPLEMENTED_TRAP();
	return KERN_FAILURE;
}

kern_return_t semaphore_wait_trap(
				mach_port_name_t wait_name)
{
	struct semaphore_wait_args args = {
		.signal = wait_name
	};

	return ioctl(driver_fd, NR_semaphore_wait_trap, &args);
}

kern_return_t semaphore_wait_signal_trap(
				mach_port_name_t wait_name,
				mach_port_name_t signal_name)
{
	struct semaphore_wait_signal_args args = {
		.signal = signal_name,
		.wait = wait_name
	};

	return ioctl(driver_fd, NR_semaphore_wait_signal_trap,
			&args);
}

kern_return_t semaphore_timedwait_trap(
				mach_port_name_t wait_name,
				unsigned int sec,
				clock_res_t nsec)
{
	struct semaphore_timedwait_args args = {
		.wait = wait_name,
		.sec = sec,
		.nsec = nsec
	};

	return ioctl(driver_fd, NR_semaphore_timedwait_trap,
			&args);
}

kern_return_t semaphore_timedwait_signal_trap(
				mach_port_name_t wait_name,
				mach_port_name_t signal_name,
				unsigned int sec,
				clock_res_t nsec)
{
	struct semaphore_timedwait_signal_args args = {
		.wait = wait_name,
		.signal = signal_name,
		.sec = sec,
		.nsec = nsec
	};

	return ioctl(driver_fd, NR_semaphore_timedwait_signal_trap,
			&args);
}

kern_return_t clock_sleep_trap(
				mach_port_name_t clock_name,
				sleep_type_t sleep_type,
				int sleep_sec,
				int sleep_nsec,
				mach_timespec_t	*wakeup_time)
{
	UNIMPLEMENTED_TRAP();
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
	if ((flags >> 24) == VM_MEMORY_REALLOC)
		addr = __linux_mremap(((char*)*address) - 0x1000, 0x1000, 0x1000 + size, 0);
	else
		addr = mmap(*address, size, prot, posix_flags, -1, 0);
	
	if (addr == MAP_FAILED)
	{
		return KERN_FAILURE;
	}
	
	if (mask && ( ((uintptr_t)addr) & mask) != 0)
	{
		uintptr_t boundary, q, diff, iaddr;
		
		// Alignment was requested, but we couldn't get it the easy way
		munmap(addr, size);
		
		// This may not work for some crazy masks. Consider using __builtin_clz().
		boundary = mask + 1;
		
		iaddr = mmap(*address, size + boundary, prot, posix_flags, -1, 0);
		if (iaddr == (uintptr_t) MAP_FAILED)
			return KERN_FAILURE;
		
		q = (iaddr + (boundary-1)) / boundary * boundary;
		diff = q - iaddr;
		
		if (diff > 0)
			munmap(iaddr, diff);
		if (boundary - diff > 0)
			munmap((void*) (q + size), boundary - diff);
		
		addr = (void*) q;
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
	kern_return_t ret;

	struct mach_port_allocate_args args = {
		.task_right_name = target,
		.right_type = right,
		.out_right_name = 0
	};

	ret = ioctl(driver_fd, NR__kernelrpc_mach_port_allocate,
			&args);

	if (ret == KERN_SUCCESS)
		*name = args.out_right_name;
	else
		*name = 0;

	return ret;
}


kern_return_t _kernelrpc_mach_port_destroy_trap(
				mach_port_name_t target,
				mach_port_name_t name
)
{
	struct mach_port_destroy_args args = {
		.task_right_name = target,
		.port_right_name = name
	};

	return ioctl(driver_fd, NR__kernelrpc_mach_port_destroy,
			&args);
}

kern_return_t _kernelrpc_mach_port_deallocate_trap(
				mach_port_name_t target,
				mach_port_name_t name
)
{
	struct mach_port_deallocate_args args = {
		.task_right_name = target,
		.port_right_name = name
	};

	return ioctl(driver_fd, NR__kernelrpc_mach_port_deallocate,
			&args);
}

kern_return_t _kernelrpc_mach_port_mod_refs_trap(
				mach_port_name_t target,
				mach_port_name_t name,
				mach_port_right_t right,
				mach_port_delta_t delta
)
{
	struct mach_port_mod_refs_args args = {
		.task_right_name = target,
		.port_right_name = name,
		.right_type = right,
		.delta = delta
	};

	return ioctl(driver_fd, NR__kernelrpc_mach_port_mod_refs, &args);;
}

kern_return_t _kernelrpc_mach_port_move_member_trap(
				mach_port_name_t target,
				mach_port_name_t member,
				mach_port_name_t after
)
{
	UNIMPLEMENTED_TRAP();
	return KERN_FAILURE;
}

kern_return_t _kernelrpc_mach_port_insert_right_trap(
				mach_port_name_t target,
				mach_port_name_t name,
				mach_port_name_t poly,
				mach_msg_type_name_t polyPoly
)
{
	UNIMPLEMENTED_TRAP();
	return KERN_FAILURE;
}

kern_return_t _kernelrpc_mach_port_insert_member_trap(
				mach_port_name_t target,
				mach_port_name_t name,
				mach_port_name_t pset
)
{
	UNIMPLEMENTED_TRAP();
	return KERN_FAILURE;
}

kern_return_t _kernelrpc_mach_port_extract_member_trap(
				mach_port_name_t target,
				mach_port_name_t name,
				mach_port_name_t pset
)
{
	UNIMPLEMENTED_TRAP();
	return KERN_FAILURE;
}

kern_return_t _kernelrpc_mach_port_construct_trap(
				mach_port_name_t target,
				mach_port_options_t *options,
				uint64_t context,
				mach_port_name_t *name
)
{
	UNIMPLEMENTED_TRAP();
	return KERN_FAILURE;
}

kern_return_t _kernelrpc_mach_port_destruct_trap(
				mach_port_name_t target,
				mach_port_name_t name,
				mach_port_delta_t srdelta,
				uint64_t guard
)
{
	UNIMPLEMENTED_TRAP();
	return KERN_FAILURE;
}

kern_return_t _kernelrpc_mach_port_guard_trap(
				mach_port_name_t target,
				mach_port_name_t name,
				uint64_t guard,
				boolean_t strict
)
{
	UNIMPLEMENTED_TRAP();
	return KERN_FAILURE;
}

kern_return_t _kernelrpc_mach_port_unguard_trap(
				mach_port_name_t target,
				mach_port_name_t name,
				uint64_t guard
)
{
	UNIMPLEMENTED_TRAP();
	return KERN_FAILURE;
}

kern_return_t macx_swapon(
				uint64_t filename,
				int flags,
				int size,
				int priority)
{
	UNIMPLEMENTED_TRAP();
	return KERN_FAILURE;
}

kern_return_t macx_swapoff(
				uint64_t filename,
				int flags)
{
	UNIMPLEMENTED_TRAP();
	return KERN_FAILURE;
}

kern_return_t macx_triggers(
				int hi_water,
				int low_water,
				int flags,
				mach_port_t alert_port)
{
	UNIMPLEMENTED_TRAP();
	return KERN_FAILURE;
}

kern_return_t macx_backing_store_suspend(
				boolean_t suspend)
{
	UNIMPLEMENTED_TRAP();
	return KERN_FAILURE;
}

kern_return_t macx_backing_store_recovery(
				int pid)
{
	UNIMPLEMENTED_TRAP();
	return KERN_FAILURE;
}

extern void __linux_sched_yield();
boolean_t swtch_pri(int pri)
{
	__linux_sched_yield();
	return 0;
}

boolean_t swtch(void)
{
	__linux_sched_yield();
	return 0;
}

kern_return_t syscall_thread_switch(
				mach_port_name_t thread_name,
				int option,
				mach_msg_timeout_t option_time)
{
	UNIMPLEMENTED_TRAP();
	return KERN_FAILURE;
}

mach_port_name_t task_self_trap(void)
{
	return ioctl(driver_fd, NR_task_self_trap, 0);
}

/*
 *	Obsolete interfaces.
 */

kern_return_t task_for_pid(
				mach_port_name_t target_tport,
				int pid,
				mach_port_name_t *t)
{
	UNIMPLEMENTED_TRAP();
	return KERN_FAILURE;
}

kern_return_t task_name_for_pid(
				mach_port_name_t target_tport,
				int pid,
				mach_port_name_t *tn)
{
	UNIMPLEMENTED_TRAP();
	return KERN_FAILURE;
}

kern_return_t pid_for_task(
				mach_port_name_t t,
				int *x)
{
	UNIMPLEMENTED_TRAP();
	return KERN_FAILURE;
}

kern_return_t bsdthread_terminate_trap(
				uintptr_t stackaddr,
				unsigned long freesize,
				mach_port_name_t thread,
				mach_port_name_t sem)
{
	struct bsdthread_terminate_args args = {
		.stackaddr = stackaddr,
		.freesize = freesize,
		.thread_right_name = thread,
		.signal = sem
	};

	return ioctl(driver_fd, NR_bsdthread_terminate_trap, &args);
}

typedef struct voucher_mach_msg_state_s *voucher_mach_msg_state_t;

boolean_t voucher_mach_msg_set(mach_msg_header_t *msg)
{
	// UNIMPLEMENTED_TRAP();
	return 0;
}

void voucher_mach_msg_revert(voucher_mach_msg_state_t state)
{
	UNIMPLEMENTED_TRAP();
}

voucher_mach_msg_state_t voucher_mach_msg_adopt(mach_msg_header_t *msg)
{
	UNIMPLEMENTED_TRAP();
	return NULL;
}

kern_return_t mach_generate_activity_id(mach_port_name_t task, int i, uint64_t* id)
{
	UNIMPLEMENTED_TRAP();
	return KERN_FAILURE;
}

mach_port_name_t mk_timer_create(void)
{
	return ioctl(driver_fd, NR_mk_timer_create_trap, NULL);
}

kern_return_t mk_timer_destroy(mach_port_name_t name)
{
	struct mk_timer_destroy_args args = {
		.timer_port = name
	};

	return ioctl(driver_fd, NR_mk_timer_destroy_trap, &args);
}

kern_return_t mk_timer_arm(mach_port_name_t name, uint64_t expire_time)
{
	struct mk_timer_arm_args args = {
		.timer_port = name,
		.expire_time = expire_time,
	};

	return ioctl(driver_fd, NR_mk_timer_arm_trap, &args);
}

kern_return_t mk_timer_cancel(mach_port_name_t name, uint64_t *result_time)
{
	struct mk_timer_cancel_args args = {
		.timer_port = name,
		.result_time = result_time,
	};

	return ioctl(driver_fd, NR_mk_timer_cancel_trap, &args);
}
