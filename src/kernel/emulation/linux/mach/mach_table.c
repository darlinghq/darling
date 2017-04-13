#include "mach_time.h"
#include "mach_traps.h"
#include <mach/kern_return.h>

__attribute__((visibility("hidden")))
void* __mach_syscall_table[128] = {
	[10] = _kernelrpc_mach_vm_allocate_trap_impl,
	[12] = _kernelrpc_mach_vm_deallocate_trap_impl,
	[14] = _kernelrpc_mach_vm_protect_trap_impl,
	[15] = _kernelrpc_mach_vm_map_trap_impl,
	[16] = _kernelrpc_mach_port_allocate_trap_impl,
	[17] = _kernelrpc_mach_port_destroy_trap_impl,
	[18] = _kernelrpc_mach_port_deallocate_trap_impl,
	[19] = _kernelrpc_mach_port_mod_refs_trap_impl,
	[20] = _kernelrpc_mach_port_move_member_trap_impl,
	[21] = _kernelrpc_mach_port_insert_right_trap_impl,
	[22] = _kernelrpc_mach_port_insert_member_trap_impl,
	[23] = _kernelrpc_mach_port_extract_member_trap_impl,
	[24] = _kernelrpc_mach_port_construct_trap_impl,
	[25] = _kernelrpc_mach_port_destruct_trap_impl,
	[26] = mach_reply_port_impl,
	[27] = thread_self_trap_impl,
	[28] = task_self_trap_impl,
	[29] = host_self_trap_impl,
	[31] = mach_msg_trap_impl,
	[32] = mach_msg_overwrite_trap_impl,
	[33] = semaphore_signal_trap_impl,
	[34] = semaphore_signal_all_trap_impl,
	[35] = semaphore_signal_thread_trap_impl,
	[36] = semaphore_wait_trap_impl,
	[37] = semaphore_wait_signal_trap_impl,
	[38] = semaphore_timedwait_trap_impl,
	[39] = semaphore_timedwait_signal_trap_impl,
	[41] = _kernelrpc_mach_port_guard_trap_impl,
	[42] = _kernelrpc_mach_port_unguard_trap_impl,
	[43] = mach_generate_activity_id_impl,
	[44] = task_name_for_pid_impl,
	[45] = task_for_pid_impl,
	[46] = pid_for_task_impl,
	[48] = macx_swapon_impl,
	[49] = macx_swapoff_impl,
	[51] = macx_triggers_impl,
	[52] = macx_backing_store_suspend_impl,
	[53] = macx_backing_store_recovery_impl,
	[59] = swtch_pri_impl,
	[60] = swtch_impl,
	[61] = syscall_thread_switch_impl,
	[62] = clock_sleep_trap_impl,
	[89] = mach_timebase_info_trap_impl,
	[90] = mach_wait_until_impl,
	[91] = mk_timer_create_impl,
	[92] = mk_timer_destroy_impl,
	[93] = mk_timer_arm_impl,
	[94] = mk_timer_cancel_impl,
};

extern void __simple_printf(const char* format, ...);
long __unknown_mach_syscall(int nr, ...)
{
	__simple_printf("Unimplemented mach trap (%d)\n", nr);
	return KERN_FAILURE;
}

