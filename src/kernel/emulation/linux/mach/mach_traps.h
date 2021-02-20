#ifndef _MACH_TRAPS_H
#define _MACH_TRAPS_H

#include <mach/mach_traps.h>
#include <mach/vm_statistics.h>
#include <mach/kern_return.h>

typedef struct voucher_mach_msg_state_s *voucher_mach_msg_state_t;

mach_port_name_t mach_reply_port_impl(void);
mach_port_name_t thread_self_trap_impl(void);
mach_port_name_t host_self_trap_impl(void);
mach_msg_return_t mach_msg_trap_impl(
				mach_msg_header_t *msg,
				mach_msg_option_t option,
				mach_msg_size_t send_size,
				mach_msg_size_t rcv_size,
				mach_port_name_t rcv_name,
				mach_msg_timeout_t timeout,
				mach_port_name_t notify);
mach_msg_return_t mach_msg_overwrite_trap_impl(
				mach_msg_header_t *msg,
				mach_msg_option_t option,
				mach_msg_size_t send_size,
				mach_msg_size_t rcv_size,
				mach_port_name_t rcv_name,
				mach_msg_timeout_t timeout,
				mach_port_name_t notify,
				mach_msg_header_t *rcv_msg,
				mach_msg_size_t rcv_limit);
kern_return_t semaphore_signal_trap_impl(
				mach_port_name_t signal_name);
kern_return_t semaphore_signal_all_trap_impl(
				mach_port_name_t signal_name);
kern_return_t semaphore_signal_thread_trap_impl(
				mach_port_name_t signal_name,
				mach_port_name_t thread_name);
kern_return_t semaphore_wait_trap_impl(
				mach_port_name_t wait_name);
kern_return_t semaphore_wait_signal_trap_impl(
				mach_port_name_t wait_name,
				mach_port_name_t signal_name);
kern_return_t semaphore_timedwait_trap_impl(
				mach_port_name_t wait_name,
				unsigned int sec,
				clock_res_t nsec);
kern_return_t semaphore_timedwait_signal_trap_impl(
				mach_port_name_t wait_name,
				mach_port_name_t signal_name,
				unsigned int sec,
				clock_res_t nsec);
kern_return_t clock_sleep_trap_impl(
				mach_port_name_t clock_name,
				sleep_type_t sleep_type,
				int sleep_sec,
				int sleep_nsec,
				mach_timespec_t	*wakeup_time);
kern_return_t _kernelrpc_mach_vm_allocate_trap_impl(
				mach_port_name_t target,
				mach_vm_offset_t *addr,
				mach_vm_size_t size,
				int flags);
kern_return_t _kernelrpc_mach_vm_deallocate_trap_impl(
				mach_port_name_t target,
				mach_vm_address_t address,
				mach_vm_size_t size
);
kern_return_t _kernelrpc_mach_vm_protect_trap_impl(
				mach_port_name_t target,
				mach_vm_address_t address,
				mach_vm_size_t size,
				boolean_t set_maximum,
				vm_prot_t new_protection
);
kern_return_t _kernelrpc_mach_vm_map_trap_impl(
				mach_port_name_t target,
				mach_vm_offset_t *address,
				mach_vm_size_t size,
				mach_vm_offset_t mask,
				int flags,
				vm_prot_t cur_protection
);
kern_return_t _kernelrpc_mach_port_allocate_trap_impl(
				mach_port_name_t target,
				mach_port_right_t right,
				mach_port_name_t *name
);
kern_return_t _kernelrpc_mach_port_destroy_trap_impl(
				mach_port_name_t target,
				mach_port_name_t name
);
kern_return_t _kernelrpc_mach_port_deallocate_trap_impl(
				mach_port_name_t target,
				mach_port_name_t name
);
kern_return_t _kernelrpc_mach_port_mod_refs_trap_impl(
				mach_port_name_t target,
				mach_port_name_t name,
				mach_port_right_t right,
				mach_port_delta_t delta
);
kern_return_t _kernelrpc_mach_port_move_member_trap_impl(
				mach_port_name_t target,
				mach_port_name_t member,
				mach_port_name_t after
);
kern_return_t _kernelrpc_mach_port_insert_right_trap_impl(
				mach_port_name_t target,
				mach_port_name_t name,
				mach_port_name_t poly,
				mach_msg_type_name_t polyPoly
);
kern_return_t _kernelrpc_mach_port_insert_member_trap_impl(
				mach_port_name_t target,
				mach_port_name_t name,
				mach_port_name_t pset
);
kern_return_t _kernelrpc_mach_port_extract_member_trap_impl(
				mach_port_name_t target,
				mach_port_name_t name,
				mach_port_name_t pset
);
kern_return_t _kernelrpc_mach_port_construct_trap_impl(
				mach_port_name_t target,
				mach_port_options_t *options,
				uint64_t context,
				mach_port_name_t *name
);
kern_return_t _kernelrpc_mach_port_destruct_trap_impl(
				mach_port_name_t target,
				mach_port_name_t name,
				mach_port_delta_t srdelta,
				uint64_t guard
);
kern_return_t _kernelrpc_mach_port_guard_trap_impl(
				mach_port_name_t target,
				mach_port_name_t name,
				uint64_t guard,
				boolean_t strict
);
kern_return_t _kernelrpc_mach_port_unguard_trap_impl(
				mach_port_name_t target,
				mach_port_name_t name,
				uint64_t guard
);
kern_return_t thread_get_special_reply_port_impl(void);
kern_return_t _kernelrpc_mach_port_request_notification_impl(
	ipc_space_t task,
	mach_port_name_t name,
	mach_msg_id_t msgid,
	mach_port_mscount_t sync,
	mach_port_name_t notify,
	mach_msg_type_name_t notifyPoly,
	mach_port_name_t* previous
);
kern_return_t _kernelrpc_mach_port_get_attributes_impl(
	mach_port_name_t target,
	mach_port_name_t name,
	mach_port_flavor_t flavor,
	mach_port_info_t port_info_out,
	mach_msg_type_number_t* port_info_outCnt
);
kern_return_t _kernelrpc_mach_port_type_impl(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_type_t* ptype
);
kern_return_t macx_swapon_impl(
				uint64_t filename,
				int flags,
				int size,
				int priority);
kern_return_t macx_swapoff_impl(
				uint64_t filename,
				int flags);
kern_return_t macx_triggers_impl(
				int hi_water,
				int low_water,
				int flags,
				mach_port_t alert_port);
kern_return_t macx_backing_store_suspend_impl(
				boolean_t suspend);
kern_return_t macx_backing_store_recovery_impl(
				int pid);
boolean_t swtch_pri_impl(int pri);
boolean_t swtch_impl(void);
kern_return_t syscall_thread_switch_impl(
				mach_port_name_t thread_name,
				int option,
				mach_msg_timeout_t option_time);
mach_port_name_t task_self_trap_impl(void);
kern_return_t task_for_pid_impl(
				mach_port_name_t target_tport,
				int pid,
				mach_port_name_t *t);
kern_return_t task_name_for_pid_impl(
				mach_port_name_t target_tport,
				int pid,
				mach_port_name_t *tn);
kern_return_t pid_for_task_impl(
				mach_port_name_t t,
				int *x);
kern_return_t bsdthread_terminate_trap_impl(
				uintptr_t stackaddr,
				unsigned long freesize,
				mach_port_name_t thread,
				mach_port_name_t sem);
boolean_t voucher_mach_msg_set(mach_msg_header_t *msg);
void voucher_mach_msg_revert(voucher_mach_msg_state_t state);
voucher_mach_msg_state_t voucher_mach_msg_adopt(mach_msg_header_t *msg);
kern_return_t mach_generate_activity_id_impl(mach_port_name_t task, int i, uint64_t* id);
mach_port_name_t mk_timer_create_impl(void);
kern_return_t mk_timer_destroy_impl(mach_port_name_t name);
kern_return_t mk_timer_arm_impl(mach_port_name_t name, uint64_t expire_time);
kern_return_t mk_timer_cancel_impl(mach_port_name_t name, uint64_t *result_time);

#endif

