/*
 * Darling Mach Linux Kernel Module
 * Copyright (C) 2015 Lubos Dolezel
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <mach/mach_types.h>
#include <mach_debug/zone_info.h>
#include "../mach_includes.h"
#include "../debug.h"
#include "../traps.h"
#include "../darling_task.h"
#include "../ipc_port.h"
#include "../ipc_space.h"
#include "../primitives/semaphore.h"
#include "task.h"
#include "stub.h"

extern ipc_namespace_t kernel_namespace;

kern_return_t task_create
(
	task_t target_task,
	ledger_array_t ledgers,
	mach_msg_type_number_t ledgersCnt,
	boolean_t inherit_memory,
	task_t *child_task
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t task_terminate
(
	task_t target_task
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t task_threads
(
	task_t target_task,
	thread_act_array_t *act_list,
	mach_msg_type_number_t *act_listCnt
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t mach_ports_register
(
	task_t target_task,
	mach_port_array_t init_port_set,
	mach_msg_type_number_t init_port_setCnt
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t mach_ports_lookup
(
	task_t target_task,
	mach_port_array_t *init_port_set,
	mach_msg_type_number_t *init_port_setCnt
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t task_info
(
	task_name_t target_task,
	task_flavor_t flavor,
	task_info_t task_info_out,
	mach_msg_type_number_t *task_info_outCnt
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t task_set_info
(
	task_t target_task,
	task_flavor_t flavor,
	task_info_t task_info_in,
	mach_msg_type_number_t task_info_inCnt
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t task_suspend
(
	task_t target_task
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t task_resume
(
	task_t target_task
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t task_get_special_port
(
	task_t task,
	int which_port,
	mach_port_t *special_port
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t task_set_special_port
(
	task_t task,
	int which_port,
	mach_port_t special_port
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t thread_create
(
	task_t parent_task,
	thread_act_t *child_act
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t thread_create_running
(
	task_t parent_task,
	thread_state_flavor_t flavor,
	thread_state_t new_state,
	mach_msg_type_number_t new_stateCnt,
	thread_act_t *child_act
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t task_set_exception_ports
(
	task_t task,
	exception_mask_t exception_mask,
	mach_port_t new_port,
	exception_behavior_t behavior,
	thread_state_flavor_t new_flavor
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t task_get_exception_ports
(
	task_t task,
	exception_mask_t exception_mask,
	exception_mask_array_t masks,
	mach_msg_type_number_t *masksCnt,
	exception_handler_array_t old_handlers,
	exception_behavior_array_t old_behaviors,
	exception_flavor_array_t old_flavors
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t task_swap_exception_ports
(
	task_t task,
	exception_mask_t exception_mask,
	mach_port_t new_port,
	exception_behavior_t behavior,
	thread_state_flavor_t new_flavor,
	exception_mask_array_t masks,
	mach_msg_type_number_t *masksCnt,
	exception_handler_array_t old_handlerss,
	exception_behavior_array_t old_behaviors,
	exception_flavor_array_t old_flavors
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t lock_set_create
(
	task_t task,
	lock_set_t *new_lock_set,
	int n_ulocks,
	int policy
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t lock_set_destroy
(
	task_t task,
	lock_set_t lock_set
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t semaphore_create
(
	task_t task,
	semaphore_t *semaphore,
	int policy,
	int value
)
{
	kern_return_t ret;
	darling_mach_port_t* port;
	mach_task_t* task_self;
	
	task_self = darling_task_get_current();
	
	// TODO: check task
	
	ret = ipc_port_new(&port);
	if (ret != KERN_SUCCESS)
		return ret;
	
	ret = ipc_space_make_send(&kernel_namespace, port, false, semaphore);
	
	if (ret != KERN_SUCCESS)
		return ret;
	
	mach_semaphore_setup(port, value);
	
	return KERN_SUCCESS;
}

kern_return_t semaphore_destroy
(
	task_t task,
	semaphore_t semaphore
)
{
	return _kernelrpc_mach_port_destroy(darling_task_get_current(),
			task, semaphore);
}

kern_return_t task_policy_set
(
	task_t task,
	task_policy_flavor_t flavor,
	task_policy_t policy_info,
	mach_msg_type_number_t policy_infoCnt
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t task_policy_get
(
	task_t task,
	task_policy_flavor_t flavor,
	task_policy_t policy_info,
	mach_msg_type_number_t *policy_infoCnt,
	boolean_t *get_default
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t task_sample
(
	task_t task,
	mach_port_t reply
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t task_policy
(
	task_t task,
	policy_t policy,
	policy_base_t base,
	mach_msg_type_number_t baseCnt,
	boolean_t set_limit,
	boolean_t change
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t task_set_emulation
(
	task_t target_port,
	vm_address_t routine_entry_pt,
	int routine_number
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t task_get_emulation_vector
(
	task_t task,
	int *vector_start,
	emulation_vector_t *emulation_vector,
	mach_msg_type_number_t *emulation_vectorCnt
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t task_set_emulation_vector
(
	task_t task,
	int vector_start,
	emulation_vector_t emulation_vector,
	mach_msg_type_number_t emulation_vectorCnt
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t task_set_ras_pc
(
	task_t target_task,
	vm_address_t basepc,
	vm_address_t boundspc
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t task_zone_info
(
	task_t target_task,
	mach_zone_name_array_t *names,
	mach_msg_type_number_t *namesCnt,
	task_zone_info_array_t *info,
	mach_msg_type_number_t *infoCnt
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t task_assign
(
	task_t task,
	processor_set_t new_set,
	boolean_t assign_threads
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t task_assign_default
(
	task_t task,
	boolean_t assign_threads
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t task_get_assignment
(
	task_t task,
	processor_set_name_t *assigned_set
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t task_set_policy
(
	task_t task,
	processor_set_t pset,
	policy_t policy,
	policy_base_t base,
	mach_msg_type_number_t baseCnt,
	policy_limit_t limit,
	mach_msg_type_number_t limitCnt,
	boolean_t change
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t task_get_state
(
	task_t task,
	thread_state_flavor_t flavor,
	thread_state_t old_state,
	mach_msg_type_number_t *old_stateCnt
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t task_set_state
(
	task_t task,
	thread_state_flavor_t flavor,
	thread_state_t new_state,
	mach_msg_type_number_t new_stateCnt
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t task_set_phys_footprint_limit
(
	task_t task,
	int new_limit,
	int *old_limit
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t task_suspend2
(
	task_t target_task,
	task_suspension_token_t *suspend_token
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t task_resume2
(
	task_suspension_token_t suspend_token
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t task_purgable_info
(
	task_t task,
	task_purgable_info_t *stats
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t task_get_mach_voucher
(
	task_t task,
	mach_voucher_selector_t which,
	ipc_voucher_t *voucher
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t task_set_mach_voucher
(
	task_t task,
	ipc_voucher_t voucher
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t task_swap_mach_voucher
(
	task_t task,
	ipc_voucher_t new_voucher,
	ipc_voucher_t *old_voucher
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}
