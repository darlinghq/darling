#include "../mach_includes.h"
#include "../mig_includes_pre.h"
#include "../mig/mach_hostServer.h"

kern_return_t host_info
(
	host_t host,
	host_flavor_t flavor,
	host_info_t host_info_out,
	mach_msg_type_number_t *host_info_outCnt
)
{
	return KERN_NOT_SUPPORTED;
}

kern_return_t host_kernel_version
(
	host_t host,
	kernel_version_t kernel_version
)
{
	return KERN_NOT_SUPPORTED;
}

kern_return_t _host_page_size
(
	host_t host,
	vm_size_t *out_page_size
)
{
	return KERN_NOT_SUPPORTED;
}

kern_return_t mach_memory_object_memory_entry
(
	host_t host,
	boolean_t internal,
	vm_size_t size,
	vm_prot_t permission,
	memory_object_t pager,
	mach_port_t *entry_handle
)
{
	return KERN_NOT_SUPPORTED;
}

kern_return_t host_processor_info
(
	host_t host,
	processor_flavor_t flavor,
	natural_t *out_processor_count,
	processor_info_array_t *out_processor_info,
	mach_msg_type_number_t *out_processor_infoCnt
)
{
	return KERN_NOT_SUPPORTED;
}

kern_return_t host_get_io_master
(
	host_t host,
	io_master_t *io_master
)
{
	return KERN_NOT_SUPPORTED;
}

kern_return_t host_get_clock_service
(
	host_t host,
	clock_id_t clock_id,
	clock_serv_t *clock_serv
)
{
	return KERN_NOT_SUPPORTED;
}

kern_return_t kmod_get_info
(
	host_t host,
	kmod_args_t *modules,
	mach_msg_type_number_t *modulesCnt
)
{
	return KERN_NOT_SUPPORTED;
}

kern_return_t host_zone_info
(
	host_priv_t host,
	zone_name_array_t *names,
	mach_msg_type_number_t *namesCnt,
	zone_info_array_t *info,
	mach_msg_type_number_t *infoCnt
)
{
	return KERN_NOT_SUPPORTED;
}

kern_return_t host_virtual_physical_table_info
(
	host_t host,
	hash_info_bucket_array_t *info,
	mach_msg_type_number_t *infoCnt
)
{
	return KERN_NOT_SUPPORTED;
}

kern_return_t processor_set_default
(
	host_t host,
	processor_set_name_t *default_set
)
{
	return KERN_NOT_SUPPORTED;
}

kern_return_t processor_set_create
(
	host_t host,
	processor_set_t *new_set,
	processor_set_name_t *new_name
)
{
	return KERN_NOT_SUPPORTED;
}

kern_return_t mach_memory_object_memory_entry_64
(
	host_t host,
	boolean_t internal,
	memory_object_size_t size,
	vm_prot_t permission,
	memory_object_t pager,
	mach_port_t *entry_handle
)
{
	return KERN_NOT_SUPPORTED;
}

kern_return_t host_statistics
(
	host_t host_priv,
	host_flavor_t flavor,
	host_info_t host_info_out,
	mach_msg_type_number_t *host_info_outCnt
)
{
	return KERN_NOT_SUPPORTED;
}

kern_return_t host_request_notification
(
	host_t host,
	host_flavor_t notify_type,
	mach_port_t notify_port
)
{
	return KERN_NOT_SUPPORTED;
}

kern_return_t host_lockgroup_info
(
	host_t host,
	lockgroup_info_array_t *lockgroup_info,
	mach_msg_type_number_t *lockgroup_infoCnt
)
{
	return KERN_NOT_SUPPORTED;
}

kern_return_t host_statistics64
(
	host_t host_priv,
	host_flavor_t flavor,
	host_info64_t host_info64_out,
	mach_msg_type_number_t *host_info64_outCnt
)
{
	return KERN_NOT_SUPPORTED;
}

kern_return_t mach_zone_info
(
	host_priv_t host,
	mach_zone_name_array_t *names,
	mach_msg_type_number_t *namesCnt,
	mach_zone_info_array_t *info,
	mach_msg_type_number_t *infoCnt
)
{
	return KERN_NOT_SUPPORTED;
}

kern_return_t mach_zone_force_gc
(
	host_t host
)
{
	return KERN_NOT_SUPPORTED;
}

kern_return_t host_create_mach_voucher
(
	host_t host,
	mach_voucher_attr_raw_recipe_array_t recipes,
	mach_msg_type_number_t recipesCnt,
	ipc_voucher_t *voucher
)
{
	return KERN_NOT_SUPPORTED;
}

kern_return_t host_register_mach_voucher_attr_manager
(
	host_t host,
	mach_voucher_attr_manager_t attr_manager,
	mach_voucher_attr_value_handle_t default_value,
	mach_voucher_attr_key_t *new_key,
	ipc_voucher_attr_control_t *new_attr_control
)
{
	return KERN_NOT_SUPPORTED;
}

kern_return_t host_register_well_known_mach_voucher_attr_manager
(
	host_t host,
	mach_voucher_attr_manager_t attr_manager,
	mach_voucher_attr_value_handle_t default_value,
	mach_voucher_attr_key_t key,
	ipc_voucher_attr_control_t *new_attr_control
)
{
	return KERN_NOT_SUPPORTED;
}
