#include "config.h"
#include "vm.h"
#include "task.h"
#include "mach-stub.h"
#include "trace.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>

static bool memory_readable(const void* ptr, size_t bytes)
{
	int fd = ::open("/dev/null", O_WRONLY);
	if (fd == -1)
		return false;
	
	int r = ::write(fd, ptr, bytes);
	::close(fd);
	
	if (r != bytes)
	{
		if (errno == EFAULT)
			return false;
	}
	return true;
}

static bool memory_writable(void* ptr, size_t bytes)
{
	int fd = ::open("/dev/zero", O_RDONLY);
	if (fd == -1)
		return false;
	
	int r = ::read(fd, ptr, bytes);
	::close(fd);
	
	if (r != bytes)
	{
		if (errno == EFAULT)
			return false;
	}
	return true;
}

kern_return_t vm_msync(vm_task_t target_task, void* addr, vm_size_t size, vm_sync_t in_flags)
{
	TRACE4(target_task, addr, size, in_flags);
	CHECK_TASK_SELF(target_task);
	
	int flags = 0;
	if (in_flags & VM_SYNC_SYNCHRONOUS)
		flags |= MS_SYNC;
	if (in_flags & VM_SYNC_ASYNCHRONOUS)
		flags |= MS_ASYNC;
	if (in_flags & VM_SYNC_INVALIDATE)
		flags |= MS_INVALIDATE;
	
	if (::msync(addr, size, flags) == -1)
		return KERN_INVALID_ADDRESS;
	else
		return KERN_SUCCESS;
}

kern_return_t vm_allocate(vm_task_t target_task, void** addr, vm_size_t size, boolean_t anywhere)
{
	TRACE4(target_task, addr, size, anywhere);
	CHECK_TASK_SELF(target_task);
	
	int flags = MAP_ANONYMOUS|MAP_PRIVATE;
	
	if (!anywhere)
		flags |= MAP_FIXED;
	
	// Give the memory all permissions, as per vm_allocate definition
	*addr = ::mmap(*addr, size, PROT_EXEC|PROT_READ|PROT_WRITE, flags, 0, 0);
	
	if (!*addr)
	{
		if (errno == EINVAL)
			return KERN_INVALID_ADDRESS;
		else
			return KERN_NO_SPACE; // No other return values are permitted
	}
	else
		return KERN_SUCCESS;
}

kern_return_t vm_deallocate(vm_task_t target_task, void* addr, vm_size_t size)
{
	TRACE3(target_task, addr, size);
	CHECK_TASK_SELF(target_task);
	
	if (::munmap(addr, size) == -1)
		return KERN_INVALID_ADDRESS;
	else
		return KERN_SUCCESS;
}

kern_return_t vm_copy(vm_task_t target_task, const void* source_address, vm_size_t count, void* dest_address)
{
	TRACE4(target_task, source_address, count, dest_address);
	CHECK_TASK_SELF(target_task);
	
	if (!memory_readable(source_address, count))
		return KERN_INVALID_ADDRESS;
	if (!memory_writable(dest_address, count))
		return KERN_INVALID_ADDRESS;
	
	::memcpy(dest_address, source_address, count);
	return KERN_SUCCESS;
}

kern_return_t vm_write(vm_task_t target_task, void* address, const void* data, vm_size_t data_count)
{
	TRACE4(target_task, address, data, data_count);
	CHECK_TASK_SELF(target_task);
	
	if (!memory_writable(address, data_count))
		return KERN_INVALID_ADDRESS;
	if (!memory_readable(data, data_count))
		return KERN_INVALID_ADDRESS;
	
	::memcpy(address, data, data_count);
	return KERN_SUCCESS;
}

kern_return_t vm_behavior_set (vm_task_t target_task, void* address, vm_size_t size, vm_behavior_t behavior)
{
	CHECK_TASK_SELF(target_task);
	MACH_STUB();
}

kern_return_t vm_inherit (vm_task_t target_task, void* address, vm_size_t size, vm_inherit_t new_inheritance)
{
	CHECK_TASK_SELF(target_task);
	MACH_STUB();
}

kern_return_t vm_machine_attribute(vm_task_t target_task, void* address, vm_size_t size, vm_machine_attribute_t attribute, vm_machine_attribute_val_t value)
{
	CHECK_TASK_SELF(target_task);
	MACH_STUB();
}

kern_return_t vm_map(vm_task_t target_task, void* address, vm_size_t size, void* mask, boolean_t anywhere, memory_object_t memory_object, vm_offset_t offset,
								 boolean_t copy, vm_prot_t cur_protection, vm_prot_t max_protection, vm_inherit_t inheritance)
{
	CHECK_TASK_SELF(target_task);
	MACH_STUB();
}

kern_return_t vm_protect(vm_task_t target_task, void* address, vm_size_t size, boolean_t set_maximum, vm_prot_t new_protection)
{
	TRACE5(target_task, address, size, set_maximum, new_protection);
	CHECK_TASK_SELF(target_task);
	
	// FIXME: Here we ignore set_maximum, not sure what to do with that
	int flags = 0;
	
	if (new_protection & VM_PROT_READ)
		flags |= PROT_READ;
	if (new_protection & VM_PROT_WRITE)
		flags |= PROT_WRITE;
	if (new_protection & VM_PROT_EXECUTE)
		flags |= PROT_EXEC;
	
	if (::mprotect(address, size, flags) == -1)
	{
		if (errno == EACCES)
			return KERN_PROTECTION_FAILURE;
		else if (errno == EINVAL || errno == ENOMEM)
			return KERN_INVALID_ADDRESS;
		else
			return KERN_FAILURE;
	}
	else
		return KERN_SUCCESS;
}

kern_return_t vm_read(vm_task_t target_task, const void* address, vm_size_t size, void** data_out, natural_t* data_count)
{
	TRACE5(target_task, address, size, data_out, data_count);
	CHECK_TASK_SELF(target_task);
	
	if (!memory_readable(address, size))
		return KERN_INVALID_ADDRESS;
	
	kern_return_t ret = vm_allocate(mach_task_self_, data_out, size, true);
	if (ret != KERN_SUCCESS)
		return ret;
	
	::memcpy(*data_out, address, size);
	*data_count = size; // Is this correct?
	
	return KERN_SUCCESS;
}

kern_return_t vm_read_overwrite (vm_task_t target_task, void* address, vm_size_t size, void* data_in, natural_t* data_count)
{
	CHECK_TASK_SELF(target_task);
	MACH_STUB();
}

kern_return_t vm_region(vm_task_t target_task, void* address, vm_size_t size, vm_region_flavor_t flavor, vm_region_info_t info, mach_msg_type_number_t info_count, memory_object_name_t object_name)
{
	CHECK_TASK_SELF(target_task);
	MACH_STUB();
}

kern_return_t vm_remap(vm_task_t target_task, void* target_address, vm_size_t size, void* mask, boolean_t anywhere, mach_port_t source_task, void* source_address, boolean_t copy,
									vm_prot_t cur_protection, vm_prot_t max_protection, vm_inherit_t inheritance)
{
	CHECK_TASK_SELF(target_task);
	MACH_STUB();
}

kern_return_t vm_wire(host_priv_t host, vm_task_t target_task, void* address, vm_size_t size, vm_prot_t wired_access)
{
	CHECK_TASK_SELF(target_task);
	MACH_STUB();
}

kern_return_t mach_vm_region_recurse(vm_task_t target_task, void *address, vm_size_t *size, natural_t *nesting_depth, void* info, mach_msg_type_number_t *infoCnt)
{
	// TODO: browse /proc/$pid/maps
	*infoCnt = 0;
	return KERN_SUCCESS;
}

