#include "os/internal.h"
#include "platform/introspection_private.h"
#include "introspection_internal.h"

#include <mach/mach_error.h>
#include <mach/mach_init.h>
#include <mach/mach_port.h>
#include <mach/mach_vm.h>
#include <mach/task.h>
#include <mach/thread_act.h>

#include <sys/sysctl.h>

#include "libkern/OSAtomic.h"

// Returns the number of thread entries that can be stored in a task page.
static unsigned int
_platform_threads_per_page(void)
{
	// Subtract out the header storage.
	return (int)(vm_page_size / sizeof(struct platform_thread_s)) - 1;
}

// Returns the page-aligned base address for a given pthread structure address.
static mach_vm_size_t
_platform_pthread_addr(mach_vm_address_t addr) {
	return trunc_page(addr); // XXX approximation
}

// Returns the page-aligned size for a given pthread structure address.
static mach_vm_size_t
_platform_pthread_size(mach_vm_address_t addr) {
	return vm_page_size; // XXX approximation
}

static kern_return_t
_platform_thread_deallocate(platform_thread_t thread)
{
	kern_return_t ret;
	if (MACH_PORT_VALID(thread->act)) {
		mach_port_deallocate(mach_task_self(), thread->act);
		thread->act = MACH_PORT_NULL;
	}
	
	if (thread->pthread_addr != 0) {
		ret = mach_vm_deallocate(mach_task_self(),
								 _platform_pthread_addr(thread->pthread_addr),
								 _platform_pthread_size(thread->pthread_addr));
		thread->pthread_addr = 0;
	}
	return ret;
}

static kern_return_t
_platform_task_deallocate(platform_task_t task)
{
	kern_return_t ret;

	if (!task) {
		return KERN_INVALID_TASK;
	}

	task_t port = task->metadata.port;
	if (port != MACH_PORT_NULL) {
		mach_port_deallocate(mach_task_self(), port);
	}

	platform_task_t ptr = task;
	do {
		mach_vm_address_t addr = (mach_vm_address_t)ptr;

		// Deallocate threads.
		int i, start = (ptr == task) ? 1 : 0; // Skip over meta data.
		for (i = start; i < _platform_threads_per_page() - start; ++i) {
			_platform_thread_deallocate(&ptr->threads[i]);
		}

		ptr = ptr->header.next;
		ret = mach_vm_deallocate(mach_task_self(), addr, vm_page_size);
	} while (ptr);

	return ret;
}

extern int __sysctl(int *, unsigned int, void *, size_t *, void *, size_t);

static kern_return_t
_platform_task_query_64_bit(platform_task_t task)
{
	task_flags_info_data_t task_flags_info;
	mach_msg_type_number_t count = TASK_FLAGS_INFO_COUNT;

	kern_return_t ret = task_info(task->metadata.port, TASK_FLAGS_INFO, (task_info_t) &task_flags_info, &count);
	if (ret == KERN_SUCCESS) {
		task->metadata.is_64_bit = (task_flags_info.flags & TF_LP64) ? true : false;
	} else if (ret == KERN_INVALID_ARGUMENT) {
		pid_t pid;
		kern_return_t ret = pid_for_task(task->metadata.port, &pid);
		if (ret != KERN_SUCCESS) return ret;

		struct kinfo_proc info;
		size_t size = sizeof(info);
		int mib[] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, pid };
		unsigned int len = sizeof(mib) / sizeof(*mib);
		int res = __sysctl(mib, len, &info, &size, NULL, 0);
		if (res == 0 && size >= sizeof(info)) {
			task->metadata.is_64_bit = (info.kp_proc.p_flag & P_LP64) != 0;
		}
	}

	return ret;
}

kern_return_t
platform_task_attach(platform_task_t *out_task, task_t port)
{
	kern_return_t ret;
	
	// Test some invariants.
	_Static_assert(sizeof(struct platform_task_header_s) == 32, "");
	_Static_assert(sizeof(struct platform_task_metadata_s) == 32, "");
	_Static_assert(sizeof(struct platform_thread_s) == 32, "");
	
	// Allocate storage for the returned task handle.
	mach_vm_address_t addr = 0;
	mach_vm_size_t size = vm_page_size;
	ret = mach_vm_allocate(mach_task_self(), &addr, size, VM_FLAGS_ANYWHERE);
	if (ret != KERN_SUCCESS) return ret;
	
	platform_task_t result = (platform_task_t)addr;

	// Keep a reference to the task port.
	ret = mach_port_mod_refs(mach_task_self(), port, MACH_PORT_RIGHT_SEND, 1);
	if (ret != KERN_SUCCESS) {
		_platform_task_deallocate(result);
		return ret;
	}

	result->header.head = result;
	result->metadata.port = port;

	ret = _platform_task_query_64_bit(result);
	if (ret != KERN_SUCCESS) {
		_platform_task_deallocate(result);
		return ret;
	}

	*out_task = result;

	return ret;
}

kern_return_t
platform_task_detach(platform_task_t task)
{
	kern_return_t ret;
	ret = _platform_task_deallocate(task);
	return ret;
}

bool
platform_task_is_64_bit(platform_task_t task)
{
	return task->metadata.is_64_bit;
}

kern_return_t
platform_task_suspend_threads(platform_task_t task)
{
	return KERN_NOT_SUPPORTED;
}

kern_return_t
platform_task_resume_threads(platform_task_t task)
{
	return KERN_NOT_SUPPORTED;
}

kern_return_t
platform_task_perform(platform_task_t task,
					  mach_vm_address_t func_addr,
					  mach_vm_address_t data_addr)
{
	return KERN_NOT_SUPPORTED;
}

static platform_task_t
_platform_thread_get_task(platform_thread_t thread)
{
	platform_task_t task = (platform_task_t)trunc_page((uintptr_t)thread);
	platform_task_t head = task->header.head;
	if (head) {
		task = head;
	}
	return task;
}

static kern_return_t
_platform_thread_map(platform_task_t task,
					 platform_thread_t thread,
					 mach_vm_address_t thread_handle)
{
	kern_return_t ret;
	vm_prot_t cur_protection, max_protection;
	
	vm_offset_t data = 0;
	mach_vm_size_t wordsize = task->metadata.is_64_bit ? 8 : 4;
	mach_msg_type_number_t size;
	ret = mach_vm_read(_platform_thread_get_task(thread)->metadata.port,
					   thread_handle, // &TSD[0]
					   wordsize,
					   &data,
					   &size);
	if (ret != KERN_SUCCESS) return ret;
	
	mach_vm_address_t pthread_addr = (uintptr_t)*(void **)data; // deref TSD[0]
	mach_vm_deallocate(mach_task_self(), data, size);

	mach_vm_address_t src_addr = _platform_pthread_addr(pthread_addr);
	mach_vm_offset_t offset = pthread_addr - src_addr;
	mach_vm_address_t dst_addr = 0;
	ret = mach_vm_remap(mach_task_self(),
						&dst_addr,
						_platform_pthread_size(pthread_addr),
						0,
						VM_FLAGS_ANYWHERE,
						_platform_thread_get_task(thread)->metadata.port,
						src_addr,
						0, // no copy
						&cur_protection,
						&max_protection,
						VM_INHERIT_NONE);
	if (ret == KERN_SUCCESS) {
		thread->pthread_addr = dst_addr + offset;
	}

	return ret;
}

// Add a mach thread to the task's thread array. Updates the existing entry
// with the same unique id if one exists, otherwise allocates a new entry.
// Consumes the reference to the thread act mach port.
static kern_return_t
_platform_task_add_mach_thread(platform_task_t task, thread_act_t act)
{
	int i;
	kern_return_t ret;

	thread_identifier_info_data_t info;
	mach_msg_type_number_t info_count = THREAD_IDENTIFIER_INFO_COUNT;
	ret = thread_info(act,
					  THREAD_IDENTIFIER_INFO,
					  (thread_info_t)&info,
					  &info_count);
	if (ret != KERN_SUCCESS) return ret;

	// Anything older than the previous generation is a candidate for recycling.
	uint32_t gen = task->metadata.gen - 1;

	// Look for an existing slot with this unique ID or the first empty slot.
	platform_thread_t empty = NULL;
	platform_thread_t found = NULL;
	platform_task_t last, ptr = task;
	do {
		int start = (ptr == task) ? 1 : 0; // Skip over meta data.
		for (i = start; i < _platform_threads_per_page() - start; ++i) {
			platform_thread_t thread = &ptr->threads[i];
			if (!empty &&
				thread->refcnt == 0 &&
				(thread->unique_id == 0 || thread->gen < gen)) {
				empty = &ptr->threads[i];
			} else if (task->threads[i].unique_id == info.thread_id) {
				found = &ptr->threads[i];
				break;
			}
		}
		last = ptr;
	} while (!found && (ptr = ptr->header.next));
		
	if (found) {
		mach_port_deallocate(mach_task_self(), found->act);
		found->act = act;
		found->gen = task->metadata.gen;
	} else {
		if (!empty) {
			// Allocate new storage if necessary.
			mach_vm_address_t addr = 0;
			mach_vm_size_t size = vm_page_size;
			ret = mach_vm_allocate(mach_task_self(),
								   &addr,
								   size,
								   VM_FLAGS_ANYWHERE);
			if (ret != KERN_SUCCESS) return ret;
			
			last = last->header.next = (platform_task_t)addr;
			last->header.head = task;
			
			empty = &last->threads[0];
		} else {
			_platform_thread_deallocate(empty);
		}

		empty->act = act; // transfer ownership
		empty->gen = task->metadata.gen;
		empty->unique_id = info.thread_id;
		ret = _platform_thread_map(task, empty, info.thread_handle);
	}

	return ret;
}

kern_return_t
platform_task_update_threads(platform_task_t task)
{
	kern_return_t ret;
	thread_act_array_t array;
	mach_msg_type_number_t array_count;
	ret = task_threads(task->metadata.port, &array, &array_count);
	if (ret != KERN_SUCCESS) return ret;

	++task->metadata.gen;
	task->metadata.cursor = &task->threads[1]; // Reset iteration cursor.

	unsigned int i;
	for (i = 0; i < array_count; ++i) {
		ret = _platform_task_add_mach_thread(task, array[i]);
	}

	mach_vm_size_t array_size = array_count * sizeof(*array);
	mach_vm_deallocate(mach_task_self(), (mach_vm_address_t)array, array_size);

	return ret;
}

platform_thread_t
platform_task_copy_next_thread(platform_task_t task)
{
	int i;
	platform_thread_t result = NULL;
	platform_thread_t cursor = task->metadata.cursor;

	if (cursor == NULL) {
		// End of iteration.
		return NULL;
	}

	uint32_t gen = task->metadata.gen;
	platform_task_t ptr = (platform_task_t)trunc_page((uintptr_t)cursor);

	do {
		if (cursor->gen == gen && cursor->unique_id != 0) {
			result = cursor;
		}

		++cursor;

		if ((uintptr_t)cursor >= ((uintptr_t)ptr + vm_page_size)) {
			ptr = ptr->header.next;
			if (ptr) {
				cursor = &ptr->threads[0];
			} else {
				cursor = NULL;
			}
		}
	} while (!result && cursor);

	task->metadata.cursor = cursor;

	if (result) {
		OSAtomicIncrement32(&result->refcnt);
	}

	return result;
}

platform_thread_id_t
platform_thread_get_unique_id(platform_thread_t thread)
{
	return thread->unique_id;
}

void
platform_thread_release(platform_thread_t thread)
{
	int32_t refcnt = OSAtomicDecrement32(&thread->refcnt);
	if (refcnt < 0) {
		__LIBPLATFORM_CLIENT_CRASH__(refcnt, "Over-release of platform thread object");
	}
}

kern_return_t
platform_thread_abort_safely(platform_thread_t thread)
{
	kern_return_t ret;
	ret = thread_abort_safely(thread->act);
	return ret;
}

kern_return_t
platform_thread_suspend(platform_thread_t thread)
{
	kern_return_t ret;
	ret = thread_suspend(thread->act);
	return ret;
}

kern_return_t
platform_thread_resume(platform_thread_t thread)
{
	kern_return_t ret;
	ret = thread_resume(thread->act);
	return ret;
}

kern_return_t
platform_thread_info(platform_thread_t thread,
					 thread_flavor_t flavor,
					 void *info,
					 size_t *size)
{
	kern_return_t ret;
	mach_msg_type_number_t count = (int)*size / sizeof(natural_t);
	ret = thread_info(thread->act, flavor, info, &count);
	*size = count * sizeof(natural_t);
	return ret;
}

kern_return_t
platform_thread_get_state(platform_thread_t thread,
						  thread_state_flavor_t flavor,
						  void *state,
						  size_t *size)
{
	kern_return_t ret;
	mach_msg_type_number_t count = (int)*size / (int)sizeof(natural_t);
	ret = thread_get_state(thread->act, flavor, state, &count);
	*size = count * sizeof(natural_t);
	return ret;
}

kern_return_t
platform_thread_set_state(platform_thread_t thread,
						  thread_state_flavor_t flavor,
						  const void *state,
						  size_t size)
{
	kern_return_t ret;
	mach_msg_type_number_t count = (int)size / (int)sizeof(natural_t);
	ret = thread_set_state(thread->act, flavor, (thread_state_t)state, count);
	return ret;
}

kern_return_t
platform_thread_perform(platform_thread_t thread,
						mach_vm_address_t func_addr,
						mach_vm_address_t data_addr)
{
	return KERN_NOT_SUPPORTED;
}

const void *
platform_thread_get_pthread(platform_thread_t thread)
{
	return (const void *) thread->pthread_addr;
}

#ifdef MAIN

// cc -DMAIN -I../../include/platform introspection.c

#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	kern_return_t ret;
	
	task_t port = MACH_PORT_NULL;
	ret = task_for_pid(mach_task_self(), getppid(), &port);
	if (ret != KERN_SUCCESS) {
		mach_error("task_for_pid", ret);
		return 1;
	}
	
	platform_task_t task = NULL;
	ret = platform_task_attach(&task, port);
	if (ret != KERN_SUCCESS) {
		mach_error("platform_task_attach", ret);
		return 1;
	}
	
	printf("Task is %s.\n", platform_task_is_64_bit(task) ? "64-bit" : "32-bit");
	
	int i;
	for (i = 0; i < 3; ++i) {
		ret = platform_task_update_threads(task);
		if (ret != KERN_SUCCESS) {
			mach_error("platform_task_update_threads", ret);
			return 1;
		}
		
		platform_thread_t thread;
		while ((thread = platform_task_copy_next_thread(task))) {
			printf("thread = { .unique_id = 0x%llx, pthread_addr = 0x%llx }\n",
				   thread->unique_id,
				   thread->pthread_addr);
			printf("pthread = { .sig = %lx, .unique_id = 0x%llx }\n",
				   *(unsigned long *)thread->pthread_addr,
				   *(uint64_t *)((uintptr_t)thread->pthread_addr + 32));
			
			platform_thread_release(thread);
		}

		sleep(3);
	}
	
	ret = platform_task_detach(task);
	if (ret != KERN_SUCCESS) {
		mach_error("platform_task_detach", ret);
		return 1;
	}
	
	return 0;
}
#endif
