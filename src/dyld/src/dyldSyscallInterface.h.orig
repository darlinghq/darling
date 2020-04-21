/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2004-2013 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */


#ifndef __DYLD_SYSCALL_HELPERS__
#define __DYLD_SYSCALL_HELPERS__

#include <dirent.h>
#include <mach-o/loader.h>
#include "../dyld_kernel.h"

#if __cplusplus
namespace dyld {
#endif

	//
	// This file contains the table of function pointers the host dyld supplies
	// to the iOS simulator dyld.
	//
	struct SyscallHelpers
	{
		uintptr_t		version;
		int				(*open)(const char* path, int oflag, int extra);
		int				(*close)(int fd);
		ssize_t			(*pread)(int fd, void* buf, size_t nbyte, off_t offset);
		ssize_t			(*write)(int fd, const void* buf, size_t nbyte);
		void*			(*mmap)(void* addr, size_t len, int prot, int flags, int fd, off_t offset);
		int				(*munmap)(void* addr, size_t len);
		int				(*madvise)(void* addr, size_t len, int advice);
		int				(*stat)(const char* path, struct stat* buf);
		int				(*fcntl)(int fildes, int cmd, void* result);
		int				(*ioctl)(int fildes, unsigned long request, void* result);
		int				(*issetugid)(void);
		char*			(*getcwd)(char* buf, size_t size);
		char*			(*realpath)(const char* file_name, char* resolved_name);
		kern_return_t	(*vm_allocate)(vm_map_t target_task, vm_address_t *address, vm_size_t size, int flags);
		kern_return_t	(*vm_deallocate)(vm_map_t target_task, vm_address_t address, vm_size_t size);
		kern_return_t	(*vm_protect)(vm_map_t target_task, vm_address_t address, vm_size_t size, boolean_t max, vm_prot_t prot);
		void			(*vlog)(const char* format, va_list list);
		void			(*vwarn)(const char* format, va_list list);
		int				(*pthread_mutex_lock)(pthread_mutex_t* m);
		int				(*pthread_mutex_unlock)(pthread_mutex_t* m);
		mach_port_t		(*mach_thread_self)(void);
		kern_return_t	(*mach_port_deallocate)(ipc_space_t task, mach_port_name_t name);
		mach_port_name_t(*task_self_trap)(void);
		kern_return_t   (*mach_timebase_info)(mach_timebase_info_t info);
		bool			(*OSAtomicCompareAndSwapPtrBarrier)(void* old, void* nw, void * volatile *value);
		void			(*OSMemoryBarrier)(void);
		void*			(*getProcessInfo)(void); // returns dyld_all_image_infos*;
		int*			(*errnoAddress)();
		uint64_t		(*mach_absolute_time)();
		// Added in version 2
		kern_return_t	(*thread_switch)(mach_port_name_t, int, mach_msg_timeout_t);
		// Added in version 3
		DIR*			(*opendir)(const char* path);
		int 			(*readdir_r)(DIR* dirp, struct dirent* entry, struct dirent **result);
		int 			(*closedir)(DIR* dirp);
		// Added in version 4
		void			(*coresymbolication_load_notifier)(void *connection, uint64_t load_timestamp, const char *image_path, const struct mach_header *mach_header);
		void			(*coresymbolication_unload_notifier)(void *connection, uint64_t unload_timestamp, const char *image_path, const struct mach_header *mach_header);
		// Added in version 5
		int				(*proc_regionfilename)(int pid, uint64_t address, void* buffer, uint32_t buffersize);
		int				(*getpid)();
		kern_return_t	(*mach_port_insert_right)(ipc_space_t task, mach_port_name_t name, mach_port_t poly, mach_msg_type_name_t polyPoly);
		kern_return_t   (*mach_port_allocate)(ipc_space_t, mach_port_right_t, mach_port_name_t*);
		kern_return_t   (*mach_msg)(mach_msg_header_t *, mach_msg_option_t , mach_msg_size_t , mach_msg_size_t , mach_port_name_t , mach_msg_timeout_t , mach_port_name_t);
		// Added in version 6
		void			(*abort_with_payload)(uint32_t reason_namespace, uint64_t reason_code, void* payload, uint32_t payload_size, const char* reason_string, uint64_t reason_flags);
		// Add in version 7
		kern_return_t	(*task_register_dyld_image_infos)(task_t task, dyld_kernel_image_info_array_t dyld_images, mach_msg_type_number_t dyld_imagesCnt);
		kern_return_t	(*task_unregister_dyld_image_infos)(task_t task, dyld_kernel_image_info_array_t dyld_images, mach_msg_type_number_t dyld_imagesCnt);
		kern_return_t	(*task_get_dyld_image_infos)(task_t task, dyld_kernel_image_info_array_t *dyld_images, mach_msg_type_number_t *dyld_imagesCnt);
		kern_return_t	(*task_register_dyld_shared_cache_image_info)(task_t task, dyld_kernel_image_info_t dyld_cache_image, boolean_t no_cache, boolean_t private_cache);
		kern_return_t	(*task_register_dyld_set_dyld_state)(task_t task, uint8_t dyld_state);
		kern_return_t	(*task_register_dyld_get_process_state)(task_t task, dyld_kernel_process_info_t *dyld_process_state);
	};

	extern const struct SyscallHelpers* gSyscallHelpers;


#if __cplusplus
}
#endif

#endif
