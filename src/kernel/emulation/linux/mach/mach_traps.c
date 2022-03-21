#define ioctl __real_ioctl
#include <mach/mach_traps.h>
#include <mach/vm_statistics.h>
#include <mach/kern_return.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "mach_traps.h"
#include <mach/mach_init.h>
#include "../ext/mremap.h"
#include <darlingserver/rpc.h>
#include "../simple.h"
#include "../duct_errno.h"

#define UNIMPLEMENTED_TRAP() { char msg[] = "Called unimplemented Mach trap: "; write(2, msg, sizeof(msg)-1); write(2, __FUNCTION__, sizeof(__FUNCTION__)-1); write(2, "\n", 1); }

mach_port_name_t mach_reply_port_impl(void)
{
	unsigned int port_name;
	if (dserver_rpc_mach_reply_port(&port_name) != 0) {
		port_name = MACH_PORT_NULL;
	}
	return port_name;
}

mach_port_name_t thread_self_trap_impl(void)
{
	unsigned int port_name;
	if (dserver_rpc_thread_self_trap(&port_name) != 0) {
		port_name = MACH_PORT_NULL;
	}
	return port_name;
}

mach_port_name_t host_self_trap_impl(void)
{
	unsigned int port_name;
	if (dserver_rpc_host_self_trap(&port_name) != 0) {
		port_name = MACH_PORT_NULL;
	}
	return port_name;
}

mach_msg_return_t mach_msg_trap_impl(
				mach_msg_header_t *msg,
				mach_msg_option_t option,
				mach_msg_size_t send_size,
				mach_msg_size_t rcv_size,
				mach_port_name_t rcv_name,
				mach_msg_timeout_t timeout,
				mach_port_name_t notify)
{
	return mach_msg_overwrite_trap_impl(msg,
			option, send_size, rcv_size,
			rcv_name, timeout, notify,
			msg, 0);
}

mach_msg_return_t mach_msg_overwrite_trap_impl(
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
	int code;

retry:
	code = dserver_rpc_mach_msg_overwrite(msg, option, send_size, rcv_size, rcv_name, timeout, notify, rcv_msg);

	if (code < 0) {
		if (code == -LINUX_EINTR) {
			// when the RPC call returns EINTR, it means we didn't manage to send the RPC message to the server;
			// when the RPC receive operation receives EINTR, it retries the call, meaning we should never see EINTR from an RPC receive.
			// therefore, if we wanted to both send and receive a message, this means the send (which is performed first) was interrupted.
			//
			// we also need to check if the caller wants to know about interrupts. if they want send interrupts, we tell them.
			// if they want receive interrupts, we tell them. otherwise, we retry the call.
			if ((option & MACH_SEND_MSG) != 0 && (option & MACH_SEND_INTERRUPT) != 0) {
				return MACH_SEND_INTERRUPTED;
			} else if ((option & MACH_RCV_MSG) != 0 && (option & MACH_RCV_INTERRUPT) != 0) {
				return MACH_RCV_INTERRUPTED;
			} else {
				goto retry;
			}
		}
		__simple_printf("mach_msg_overwrite failed (internally): %d\n", code);
		__simple_abort();
	}

	return code;
}

kern_return_t semaphore_signal_trap_impl(
				mach_port_name_t signal_name)
{
	int code = dserver_rpc_semaphore_signal(signal_name);

	if (code < 0) {
		__simple_printf("semaphore_signal failed (internally): %d\n", code);
		__simple_abort();
	}

	return code;
}
					      
kern_return_t semaphore_signal_all_trap_impl(
				mach_port_name_t signal_name)
{
	int code = dserver_rpc_semaphore_signal_all(signal_name);

	if (code < 0) {
		__simple_printf("semaphore_signal_all failed (internally): %d\n", code);
		__simple_abort();
	}

	return code;
}

kern_return_t semaphore_signal_thread_trap_impl(
				mach_port_name_t signal_name,
				mach_port_name_t thread_name)
{
	UNIMPLEMENTED_TRAP();
	return KERN_FAILURE;
}

kern_return_t semaphore_wait_trap_impl(
				mach_port_name_t wait_name)
{
	int code = dserver_rpc_semaphore_wait(wait_name);

	if (code < 0) {
		if (code == -LINUX_EINTR) {
			return KERN_ABORTED;
		}
		__simple_printf("semaphore_wait failed (internally): %d\n", code);
		__simple_abort();
	}

	return code;
}

kern_return_t semaphore_wait_signal_trap_impl(
				mach_port_name_t wait_name,
				mach_port_name_t signal_name)
{
	int code = dserver_rpc_semaphore_wait_signal(wait_name, signal_name);

	if (code < 0) {
		if (code == -LINUX_EINTR) {
			return KERN_ABORTED;
		}
		__simple_printf("semaphore_wait_signal failed (internally): %d\n", code);
		__simple_abort();
	}

	return code;
}

kern_return_t semaphore_timedwait_trap_impl(
				mach_port_name_t wait_name,
				unsigned int sec,
				clock_res_t nsec)
{
	int code = dserver_rpc_semaphore_timedwait(wait_name, sec, nsec);

	if (code < 0) {
		if (code == -LINUX_EINTR) {
			return KERN_ABORTED;
		}
		__simple_printf("semaphore_timedwait failed (internally): %d\n", code);
		__simple_abort();
	}

	return code;
}

kern_return_t semaphore_timedwait_signal_trap_impl(
				mach_port_name_t wait_name,
				mach_port_name_t signal_name,
				unsigned int sec,
				clock_res_t nsec)
{
	int code = dserver_rpc_semaphore_timedwait_signal(wait_name, signal_name, sec, nsec);

	if (code < 0) {
		if (code == -LINUX_EINTR) {
			return KERN_ABORTED;
		}
		__simple_printf("semaphore_timedwait_signal failed (internally): %d\n", code);
		__simple_abort();
	}

	return code;
}

kern_return_t clock_sleep_trap_impl(
				mach_port_name_t clock_name,
				sleep_type_t sleep_type,
				int sleep_sec,
				int sleep_nsec,
				mach_timespec_t	*wakeup_time)
{
	UNIMPLEMENTED_TRAP();
	return KERN_FAILURE;
}

kern_return_t _kernelrpc_mach_vm_allocate_trap_impl(
				mach_port_name_t target,
				mach_vm_offset_t *addr,
				mach_vm_size_t size,
				int flags)
{
	if (target != 0 && target != mach_task_self())
	{
		int code = dserver_rpc_mach_vm_allocate(target, addr, size, flags);

		if (code < 0) {
			__simple_printf("mach_vm_allocate failed (internally): %d\n", code);
			__simple_abort();
		}

		return code;
	}
	else
	{
		return _kernelrpc_mach_vm_map_trap_impl(target, addr,
				size, 0, flags, VM_PROT_READ | VM_PROT_WRITE);
	}
}

kern_return_t _kernelrpc_mach_vm_deallocate_trap_impl(
				mach_port_name_t target,
				mach_vm_address_t address,
				mach_vm_size_t size
)
{
	int ret;

	if (target != 0 && target != mach_task_self())
	{
		int code = dserver_rpc_mach_vm_deallocate(target, address, size);

		if (code < 0) {
			__simple_printf("mach_vm_deallocate failed (internally): %d\n", code);
			__simple_abort();
		}

		return code;
	}
	else
	{
		// XNU returns KERN_INVALID_ARGUMENT for address overflow, 
		// and allows NULL address if size is 0.
		if (address + size < address)
		{
			return KERN_INVALID_ARGUMENT;
		}

		if (size == (mach_vm_offset_t)0)
		{
			return KERN_SUCCESS;
		}

		ret = munmap((void*)address, size);

		if (ret == -1)
			return KERN_FAILURE;

		return KERN_SUCCESS;
	}
}

kern_return_t _kernelrpc_mach_vm_protect_trap_impl(
				mach_port_name_t target,
				mach_vm_address_t address,
				mach_vm_size_t size,
				boolean_t set_maximum,
				vm_prot_t new_protection
)
{
	int prot = 0;
	int ret;

	// mach_vm_protect() on a remote process is not trivial to implement
	// in a LKM. We simulate success. At the same time, we allow mach_vm_write()
	// to work even if the target page is R/O (like ptrace(POKE_DATA)).
	if (target != 0 && target != mach_task_self())
		return KERN_SUCCESS;

	if (new_protection & VM_PROT_READ)
		prot |= PROT_READ;
	if (new_protection & VM_PROT_WRITE)
		prot |= PROT_WRITE;
	if (new_protection & VM_PROT_EXECUTE)
		prot |= PROT_EXEC;

	ret = mprotect((void*)address, size, prot);
	if (ret == -1)
		return KERN_FAILURE;

	return KERN_SUCCESS;
}

kern_return_t _kernelrpc_mach_vm_map_trap_impl(
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
		addr = (void*)__linux_mremap(((char*)*address) - 0x1000, 0x1000, 0x1000 + size, 0, NULL);
	else
		addr = mmap((void*)*address, size, prot, posix_flags, -1, 0);
	
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
		
		iaddr = (uintptr_t)mmap((void*)*address, size + boundary, prot, posix_flags, -1, 0);
		if (iaddr == (uintptr_t) MAP_FAILED)
			return KERN_FAILURE;
		
		q = (iaddr + (boundary-1)) / boundary * boundary;
		diff = q - iaddr;
		
		if (diff > 0)
			munmap((void*)iaddr, diff);
		if (boundary - diff > 0)
			munmap((void*) (q + size), boundary - diff);
		
		addr = (void*) q;
	}

	*address = (uintptr_t)addr;
	return KERN_SUCCESS;
}

kern_return_t _kernelrpc_mach_port_allocate_trap_impl(
				mach_port_name_t target,
				mach_port_right_t right,
				mach_port_name_t *name
)
{
	int code = dserver_rpc_mach_port_allocate(target, right, name);

	if (code < 0) {
		__simple_printf("mach_port_allocate failed (internally): %d\n", code);
		__simple_abort();
	}

	return code;
}


kern_return_t _kernelrpc_mach_port_destroy_trap_impl(
				mach_port_name_t target,
				mach_port_name_t name
)
{
	int code = dserver_rpc_mach_port_destruct(target, name, 0, 0);

	if (code < 0) {
		__simple_printf("mach_port_destroy failed (internally): %d\n", code);
		__simple_abort();
	}

	return code;
}

kern_return_t _kernelrpc_mach_port_deallocate_trap_impl(
				mach_port_name_t target,
				mach_port_name_t name
)
{
	int code = dserver_rpc_mach_port_deallocate(target, name);

	if (code < 0) {
		__simple_printf("mach_port_deallocate failed (internally): %d\n", code);
		__simple_abort();
	}

	return code;
}

kern_return_t _kernelrpc_mach_port_mod_refs_trap_impl(
				mach_port_name_t target,
				mach_port_name_t name,
				mach_port_right_t right,
				mach_port_delta_t delta
)
{
	int code = dserver_rpc_mach_port_mod_refs(target, name, right, delta);

	if (code < 0) {
		__simple_printf("mach_port_deallocate failed (internally): %d\n", code);
		__simple_abort();
	}

	return code;
}

kern_return_t _kernelrpc_mach_port_move_member_trap_impl(
				mach_port_name_t target,
				mach_port_name_t member,
				mach_port_name_t after
)
{
	int code = dserver_rpc_mach_port_move_member(target, member, after);

	if (code < 0) {
		__simple_printf("mach_port_move_member failed (internally): %d\n", code);
		__simple_abort();
	}

	return code;
}

kern_return_t _kernelrpc_mach_port_insert_right_trap_impl(
				mach_port_name_t target,
				mach_port_name_t name,
				mach_port_name_t poly,
				mach_msg_type_name_t polyPoly
)
{
	int code = dserver_rpc_mach_port_insert_right(target, name, poly, polyPoly);

	if (code < 0) {
		__simple_printf("mach_port_insert_right failed (internally): %d\n", code);
		__simple_abort();
	}

	return code;
}

kern_return_t _kernelrpc_mach_port_insert_member_trap_impl(
				mach_port_name_t target,
				mach_port_name_t name,
				mach_port_name_t pset
)
{
	int code = dserver_rpc_mach_port_insert_member(target, name, pset);

	if (code < 0) {
		__simple_printf("mach_port_insert_member failed (internally): %d\n", code);
		__simple_abort();
	}

	return code;
}

kern_return_t _kernelrpc_mach_port_extract_member_trap_impl(
				mach_port_name_t target,
				mach_port_name_t name,
				mach_port_name_t pset
)
{
	int code = dserver_rpc_mach_port_extract_member(target, name, pset);

	if (code < 0) {
		__simple_printf("mach_port_extract_member failed (internally): %d\n", code);
		__simple_abort();
	}

	return code;
}

kern_return_t _kernelrpc_mach_port_construct_trap_impl(
				mach_port_name_t target,
				mach_port_options_t *options,
				uint64_t context,
				mach_port_name_t *name
)
{
	int code = dserver_rpc_mach_port_construct(target, options, context, name);

	if (code < 0) {
		__simple_printf("mach_port_construct failed (internally): %d\n", code);
		__simple_abort();
	}

	return code;
}

kern_return_t _kernelrpc_mach_port_destruct_trap_impl(
				mach_port_name_t target,
				mach_port_name_t name,
				mach_port_delta_t srdelta,
				uint64_t guard
)
{
	int code = dserver_rpc_mach_port_destruct(target, name, srdelta, guard);

	if (code < 0) {
		__simple_printf("mach_port_destruct failed (internally): %d\n", code);
		__simple_abort();
	}

	return code;
}

kern_return_t _kernelrpc_mach_port_guard_trap_impl(
				mach_port_name_t target,
				mach_port_name_t name,
				uint64_t guard,
				boolean_t strict
)
{
	int code = dserver_rpc_mach_port_guard(target, name, guard, strict);

	if (code < 0) {
		__simple_printf("mach_port_guard failed (internally): %d\n", code);
		__simple_abort();
	}

	return code;
}

kern_return_t _kernelrpc_mach_port_unguard_trap_impl(
				mach_port_name_t target,
				mach_port_name_t name,
				uint64_t guard
)
{
	int code = dserver_rpc_mach_port_unguard(target, name, guard);

	if (code < 0) {
		__simple_printf("mach_port_unguard failed (internally): %d\n", code);
		__simple_abort();
	}

	return code;
}

kern_return_t thread_get_special_reply_port_impl(void)
{
	unsigned int port_name;
	if (dserver_rpc_thread_get_special_reply_port(&port_name) != 0) {
		port_name = MACH_PORT_NULL;
	}
	return port_name;
};

kern_return_t _kernelrpc_mach_port_request_notification_impl(
	ipc_space_t task,
	mach_port_name_t name,
	mach_msg_id_t msgid,
	mach_port_mscount_t sync,
	mach_port_name_t notify,
	mach_msg_type_name_t notifyPoly,
	mach_port_name_t* previous
)
{
	int code = dserver_rpc_mach_port_request_notification(task, name, msgid, sync, notify, notifyPoly, previous);

	if (code < 0) {
		__simple_printf("mach_port_request_notification failed (internally): %d\n", code);
		__simple_abort();
	}

	return code;
};

kern_return_t _kernelrpc_mach_port_get_attributes_impl(
	mach_port_name_t target,
	mach_port_name_t name,
	mach_port_flavor_t flavor,
	mach_port_info_t port_info_out,
	mach_msg_type_number_t* port_info_outCnt
)
{
	int code = dserver_rpc_mach_port_get_attributes(target, name, flavor, port_info_out, port_info_outCnt);

	if (code < 0) {
		__simple_printf("mach_port_get_attributes failed (internally): %d\n", code);
		__simple_abort();
	}

	return code;
};

kern_return_t _kernelrpc_mach_port_type_impl(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_type_t* ptype
)
{
	int code = dserver_rpc_mach_port_type(task, name, ptype);

	if (code < 0) {
		__simple_printf("mach_port_type failed (internally): %d\n", code);
		__simple_abort();
	}

	return code;
};

kern_return_t macx_swapon_impl(
				uint64_t filename,
				int flags,
				int size,
				int priority)
{
	UNIMPLEMENTED_TRAP();
	return KERN_FAILURE;
}

kern_return_t macx_swapoff_impl(
				uint64_t filename,
				int flags)
{
	UNIMPLEMENTED_TRAP();
	return KERN_FAILURE;
}

kern_return_t macx_triggers_impl(
				int hi_water,
				int low_water,
				int flags,
				mach_port_t alert_port)
{
	UNIMPLEMENTED_TRAP();
	return KERN_FAILURE;
}

kern_return_t macx_backing_store_suspend_impl(
				boolean_t suspend)
{
	UNIMPLEMENTED_TRAP();
	return KERN_FAILURE;
}

kern_return_t macx_backing_store_recovery_impl(
				int pid)
{
	UNIMPLEMENTED_TRAP();
	return KERN_FAILURE;
}

extern void __linux_sched_yield();
boolean_t swtch_pri_impl(int pri)
{
	__linux_sched_yield();
	return 0;
}

boolean_t swtch_impl(void)
{
	__linux_sched_yield();
	return 0;
}

extern int __linux_nanosleep(struct timespec* tv, struct timespec* rem);
kern_return_t syscall_thread_switch_impl(
				mach_port_name_t thread_name,
				int option,
				mach_msg_timeout_t option_time)
{
	struct timespec tv = {
		.tv_sec = 0,
		.tv_nsec = 1000000
	};
	// Sleep for 1ms
	__linux_nanosleep(&tv, &tv);

	// TODO: we could implement this with yield_to() in LKM
	return KERN_SUCCESS;
}

mach_port_name_t task_self_trap_impl(void)
{
	unsigned int port_name;
	if (dserver_rpc_task_self_trap(&port_name) != 0) {
		port_name = MACH_PORT_NULL;
	}
	return port_name;
}

/*
 *	Obsolete interfaces.
 */

kern_return_t task_for_pid_impl(
				mach_port_name_t target_tport,
				int pid,
				mach_port_name_t *t)
{
	int code = dserver_rpc_task_for_pid(target_tport, pid, t);

	if (code < 0) {
		__simple_printf("task_for_pid failed (internally): %d\n", code);
		__simple_abort();
	}

	return code;
}

kern_return_t task_name_for_pid_impl(
				mach_port_name_t target_tport,
				int pid,
				mach_port_name_t *tn)
{
	int code = dserver_rpc_task_name_for_pid(target_tport, pid, tn);

	if (code < 0) {
		__simple_printf("task_name_for_pid failed (internally): %d\n", code);
		__simple_abort();
	}

	return code;
}

kern_return_t pid_for_task_impl(
				mach_port_name_t t,
				int *x)
{
	int code = dserver_rpc_pid_for_task(t, x);

	if (code < 0) {
		__simple_printf("pid_for_task failed (internally): %d\n", code);
		__simple_abort();
	}

	return code;
}

kern_return_t bsdthread_terminate_trap_impl(
				uintptr_t stackaddr,
				unsigned long freesize,
				mach_port_name_t thread,
				mach_port_name_t sem)
{
	UNIMPLEMENTED_TRAP();
	return KERN_FAILURE;
}


boolean_t voucher_mach_msg_set(mach_msg_header_t *msg)
{
	// UNIMPLEMENTED_TRAP();
	return 0;
}

void voucher_mach_msg_revert(voucher_mach_msg_state_t state)
{
	// UNIMPLEMENTED_TRAP();
}

voucher_mach_msg_state_t voucher_mach_msg_adopt(mach_msg_header_t *msg)
{
	// UNIMPLEMENTED_TRAP();
	return NULL;
}

kern_return_t mach_generate_activity_id_impl(mach_port_name_t task, int i, uint64_t* id)
{
	UNIMPLEMENTED_TRAP();
	return KERN_FAILURE;
}

mach_port_name_t mk_timer_create_impl(void)
{
	unsigned int port_name;
	if (dserver_rpc_mk_timer_create(&port_name) < 0) {
		port_name = MACH_PORT_NULL;
	}
	return port_name;
}

kern_return_t mk_timer_destroy_impl(mach_port_name_t name)
{
	int code = dserver_rpc_mk_timer_destroy(name);

	if (code < 0) {
		__simple_printf("mk_timer_destroy failed (internally): %d\n", code);
		__simple_abort();
	}

	return code;
}

kern_return_t mk_timer_arm_impl(mach_port_name_t name, uint64_t expire_time)
{
	int code = dserver_rpc_mk_timer_arm(name, expire_time);

	if (code < 0) {
		__simple_printf("mk_timer_arm failed (internally): %d\n", code);
		__simple_abort();
	}

	return code;
}

kern_return_t mk_timer_cancel_impl(mach_port_name_t name, uint64_t *result_time)
{
	int code = dserver_rpc_mk_timer_cancel(name, result_time);

	if (code < 0) {
		__simple_printf("mk_timer_cancel failed (internally): %d\n", code);
		__simple_abort();
	}

	return code;
}

