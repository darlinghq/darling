#include <mach/host_info.h>
#include <mach_debug/zone_info.h>
#include <mach_debug/hash_info.h>
#include <mach_debug/lockgroup_info.h>
#include "../mach_includes.h"
#include "../api.h"
#include <generated/utsrelease.h>
#include <linux/string.h>
#include <linux/cpumask.h>
#include <linux/mm.h>
#include <linux/thread_info.h>
#include <asm/page.h>
#include "../debug.h"
#include "stub.h"
#include "../mig_includes_pre.h"
#include "../mig/mach_hostServer.h"
#include "mach_host.h"

static const char KERNEL_VERSION[] = "Darling Mach (API level " DARLING_MACH_API_VERSION_STR ") on Linux " UTS_RELEASE;

extern darling_mach_port_t* host_port;

struct host_private
{
	darling_mach_port_t* clock_port;
};

static
void __host_free(server_port_t* port)
{
	struct host_private* priv;
	
	priv = (struct host_private*) port->private_data;
	
	ipc_port_put(priv->clock_port);
	kfree(priv);
}

// Because including mig/clockServer.h is problematic
extern mig_subsystem_t clock_subsystem;
void ipc_port_make_clock(darling_mach_port_t* port)
{
	port->is_server_port = true;
	port->server_port.subsystem = (mig_subsystem_t) &clock_subsystem;
	port->server_port.cb_free = NULL;
}

void ipc_port_make_host(darling_mach_port_t* port)
{
	struct host_private* priv;
	
	port->is_server_port = true;
	port->server_port.subsystem = (mig_subsystem_t) &mach_host_subsystem;
	port->server_port.cb_free = __host_free;
	
	priv = (struct host_private*) kmalloc(sizeof(struct host_private),
										GFP_KERNEL);
	
	port->server_port.private_data = priv;
	
	ipc_port_new(&priv->clock_port);
	ipc_port_make_clock(priv->clock_port);
}

static inline
struct host_private* get_host_private(void)
{
	return (struct host_private*) host_port->server_port.private_data;
}

kern_return_t host_info
(
	host_t host,
	host_flavor_t flavor,
	host_info_t host_info_out,
	mach_msg_type_number_t *host_info_outCnt
)
{
	switch (flavor)
	{
		case HOST_BASIC_INFO:
		{
			struct host_basic_info* hinfo;
			struct sysinfo i;

			hinfo = (struct host_basic_info*) host_info_out;

			if (*host_info_outCnt < HOST_BASIC_INFO_OLD_COUNT)
				return KERN_FAILURE;

			si_meminfo(&i);
			hinfo->memory_size = i.totalram;
			hinfo->max_cpus = num_possible_cpus();
			hinfo->avail_cpus = num_online_cpus();
#if defined(__i386__)
			hinfo->cpu_type = CPU_TYPE_I386;
			hinfo->cpu_subtype = CPU_SUBTYPE_I386_ALL;
#elif defined(__x86_64__)
			if (!test_thread_flag(TIF_IA32))
			{
				hinfo->cpu_type = CPU_TYPE_X86_64;
				hinfo->cpu_subtype = CPU_SUBTYPE_X86_64_ALL;
			}
			else
			{
				hinfo->cpu_type = CPU_TYPE_I386;
				hinfo->cpu_subtype = CPU_SUBTYPE_I386_ALL;
			}
#else
			hinfo->cpu_type = 0;
			hinfo->cpu_subtype = 0;
#endif

			if (*host_info_outCnt <= HOST_BASIC_INFO_COUNT)
			{
				// TODO
				*host_info_outCnt = HOST_BASIC_INFO_COUNT;
			}
			else
				*host_info_outCnt = HOST_BASIC_INFO_OLD_COUNT;
			
			return KERN_SUCCESS;
		}
		//case HOST_PROCESSOR_SLOTS:
		//	break;
		case HOST_SCHED_INFO:
			break;
		case HOST_PRIORITY_INFO:
			if (*host_info_outCnt < HOST_PRIORITY_INFO_COUNT)
				return KERN_FAILURE;
			
			// Dummy data - called from pthread_init())
			memset(host_info_out, 0, sizeof(host_priority_info_data_t));
			*host_info_outCnt = HOST_PRIORITY_INFO_COUNT;
			
			return KERN_SUCCESS;
		default:
			return KERN_NOT_SUPPORTED;
	}
	return KERN_NOT_SUPPORTED;
}

kern_return_t host_kernel_version
(
	host_t host,
	kernel_version_t kernel_version
)
{
	strncpy(kernel_version, KERNEL_VERSION, sizeof(KERNEL_VERSION));
	return KERN_SUCCESS;
}

kern_return_t _host_page_size
(
	host_t host,
	vm_size_t *out_page_size
)
{
	*out_page_size = PAGE_SIZE;
	return KERN_SUCCESS;
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
	UNIMPL_MIG_CALL();
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
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t host_get_io_master
(
	host_t host,
	io_master_t *io_master
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t host_get_clock_service
(
	host_t host,
	clock_id_t clock_id,
	clock_serv_t *clock_serv
)
{
	darling_mach_port_t* clock = get_host_private()->clock_port;
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t kmod_get_info
(
	host_t host,
	kmod_args_t *modules,
	mach_msg_type_number_t *modulesCnt
)
{
	UNIMPL_MIG_CALL();
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
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t host_virtual_physical_table_info
(
	host_t host,
	hash_info_bucket_array_t *info,
	mach_msg_type_number_t *infoCnt
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t processor_set_default
(
	host_t host,
	processor_set_name_t *default_set
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t processor_set_create
(
	host_t host,
	processor_set_t *new_set,
	processor_set_name_t *new_name
)
{
	UNIMPL_MIG_CALL();
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
	UNIMPL_MIG_CALL();
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
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t host_request_notification
(
	host_t host,
	host_flavor_t notify_type,
	mach_port_t notify_port
)
{
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t host_lockgroup_info
(
	host_t host,
	lockgroup_info_array_t *lockgroup_info,
	mach_msg_type_number_t *lockgroup_infoCnt
)
{
	UNIMPL_MIG_CALL();
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
	UNIMPL_MIG_CALL();
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
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}

kern_return_t mach_zone_force_gc
(
	host_t host
)
{
	UNIMPL_MIG_CALL();
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
	UNIMPL_MIG_CALL();
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
	UNIMPL_MIG_CALL();
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
	UNIMPL_MIG_CALL();
	return KERN_NOT_SUPPORTED;
}
