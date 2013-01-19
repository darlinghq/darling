#ifndef MACH_VM_H
#define MACH_VM_H
#include "task.h"
#include "host.h"
#include <mach/kern_return.h>
#include <mach/vm_attributes.h>
#include <mach/vm_sync.h>
#include <mach/vm_behavior.h>
#include <mach/vm_prot.h>
#include <mach/vm_inherit.h>
#include <mach/vm_region.h>
#include <mach/vm_types.h>
#include <mach/memory_object_types.h>

#ifdef __cplusplus
extern "C"
{
#endif
  
typedef darwin_task_t vm_task_t;

kern_return_t vm_msync(vm_task_t target_task, void* addr, vm_size_t size, vm_sync_t in_flags);
kern_return_t vm_allocate(vm_task_t target_task, void** addr, vm_size_t size, boolean_t anywhere);
kern_return_t vm_deallocate(vm_task_t target_task, void* addr, vm_size_t size);
kern_return_t vm_copy(vm_task_t target_task, const void* source_address, vm_size_t count, void* dest_address);
kern_return_t vm_write(vm_task_t target_task, void* address, const void* data, vm_size_t data_count);
kern_return_t vm_behavior_set (vm_task_t target_task, void* address, vm_size_t size, vm_behavior_t behavior);
kern_return_t vm_inherit (vm_task_t target_task, void* address, vm_size_t size, vm_inherit_t new_inheritance);
kern_return_t vm_machine_attribute(vm_task_t target_task, void* address, vm_size_t size, vm_machine_attribute_t attribute, vm_machine_attribute_val_t value);

kern_return_t vm_map(vm_task_t target_task, void* address, vm_size_t size, void* mask, boolean_t anywhere, memory_object_t memory_object, vm_offset_t offset,
                 boolean_t copy, vm_prot_t cur_protection, vm_prot_t max_protection, vm_inherit_t inheritance);

kern_return_t vm_protect(vm_task_t target_task, void* address, vm_size_t size, boolean_t set_maximum, vm_prot_t new_protection);

kern_return_t vm_read(vm_task_t target_task, const void* address, vm_size_t size, void** data_out, natural_t* data_count);

kern_return_t vm_read_overwrite (vm_task_t target_task, void* address, vm_size_t size, void* data_in, natural_t* data_count);

kern_return_t vm_region(vm_task_t target_task, void* address, vm_size_t size, vm_region_flavor_t flavor, vm_region_info_t info, mach_msg_type_number_t info_count, memory_object_name_t object_name);

kern_return_t vm_remap(vm_task_t target_task, void* target_address, vm_size_t size, void* mask, boolean_t anywhere, mach_port_t source_task, void* source_address, boolean_t copy,
                  vm_prot_t cur_protection, vm_prot_t max_protection, vm_inherit_t inheritance);

kern_return_t vm_wire(host_priv_t host, vm_task_t target_task, void* address, vm_size_t size, vm_prot_t wired_access);
kern_return_t mach_vm_region_recurse(vm_task_t target_task, void *address, vm_size_t *size, natural_t *nesting_depth, void* info, mach_msg_type_number_t *infoCnt);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
