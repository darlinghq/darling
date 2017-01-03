/*
 * Copyright (c) 2007-2009 Apple Inc. All rights reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <libkern/OSAtomic.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <sys/sysctl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>
#include <paths.h>
#include <errno.h>
#include "stack_logging.h"
#include "malloc_printf.h"
#include "_simple.h"            // as included by malloc.c, this defines ASL_LEVEL_INFO

#pragma mark -
#pragma mark Defines

#ifdef TEST_DISK_STACK_LOGGING
#define _malloc_printf fprintf
#undef ASL_LEVEL_INFO
#define ASL_LEVEL_INFO stderr
#endif

#define STACK_LOGGING_MAX_STACK_SIZE 512
#define STACK_LOGGING_BLOCK_WRITING_SIZE 8192
#define STACK_LOGGING_MAX_SIMUL_REMOTE_TASKS_INSPECTED 3

#define BACKTRACE_UNIQUING_DEBUG 0

// The expansion factor controls the shifting up of table size. A factor of 1 will double the size upon expanding,
// 2 will quadruple the size, etc. Maintaining a 66% fill in an ideal table requires the collision allowance to
// increase by 3 for every quadrupling of the table size (although this the constant applied to insertion
// performance O(c*n))
#define EXPAND_FACTOR 2
#define COLLISION_GROWTH_RATE 3

// For a uniquing table, the useful node size is slots := floor(table_byte_size / (2 * sizeof(mach_vm_address_t)))
// Some useful numbers for the initial max collision value (desiring 66% fill):
// 16K-23K slots -> 16 collisions
// 24K-31K slots -> 17 collisions
// 32K-47K slots -> 18 collisions
// 48K-79K slots -> 19 collisions
// 80K-96K slots -> 20 collisions
#define INITIAL_MAX_COLLIDE	19
#define DEFAULT_UNIQUING_PAGE_SIZE 256

#pragma mark -
#pragma mark Macros

#define STACK_LOGGING_FLAGS(longlongvar) (uint8_t)((uint64_t)(longlongvar) >> 56)
#define STACK_LOGGING_OFFSET(longlongvar) ((longlongvar) & 0x00FFFFFFFFFFFFFFull)
#define STACK_LOGGING_OFFSET_AND_FLAGS(longlongvar, realshortvar) (((uint64_t)(longlongvar) & 0x00FFFFFFFFFFFFFFull) | ((uint64_t)(realshortvar) << 56))

#pragma mark -
#pragma mark Types

typedef struct {
	uintptr_t	argument;
	uintptr_t	address;
	uint64_t	offset_and_flags; // top 8 bits are actually the flags!
} stack_logging_index_event;

typedef struct {
	uint32_t	argument;
	uint32_t	address;
	uint64_t	offset_and_flags; // top 8 bits are actually the flags!
} stack_logging_index_event32;

typedef struct {
	uint64_t	argument;
	uint64_t	address;
	uint64_t	offset_and_flags; // top 8 bits are actually the flags!
} stack_logging_index_event64;

#pragma pack(push,4)
typedef struct {
    uint64_t							numPages; // number of pages of the table
	uint64_t							numNodes;
	uint64_t							tableSize;
	uint64_t							untouchableNodes;
	mach_vm_address_t					table_address;
	int32_t								max_collide;
	// 'table_address' is just an always 64-bit version of the pointer-sized 'table' field to remotely read;
	// it's important that the offset of 'table_address' in the struct does not change between 32 and 64-bit.
#if BACKTRACE_UNIQUING_DEBUG
	uint64_t nodesFull;
	uint64_t backtracesContained;
#endif
	mach_vm_address_t					*table; // allocated using vm_allocate()
} backtrace_uniquing_table;
#pragma pack(pop)

// for storing/looking up allocations that haven't yet be written to disk; consistent size across 32/64-bit processes.
// It's important that these fields don't change alignment due to the architecture because they may be accessed from an
// analyzing process with a different arch - hence the pragmas.
#pragma pack(push,4)
typedef struct {
	uint64_t			start_index_offset;
	uint32_t			next_free_index_buffer_offset;
	mach_vm_address_t	uniquing_table_address;
	char				index_buffer[STACK_LOGGING_BLOCK_WRITING_SIZE];
	backtrace_uniquing_table	*uniquing_table;
} stack_buffer_shared_memory;
#pragma pack(pop)

// target process address -> record table (for __mach_stack_logging_get_frames)
typedef struct {
	uint64_t		address;
	uint64_t		index_file_offset;
} remote_index_node;

// for caching index information client-side:
typedef struct {
	size_t			cache_size;
	size_t			cache_node_capacity;
	uint32_t		collision_allowance;
	remote_index_node	*table_memory; // this can be malloced; it's on the client side.
	stack_buffer_shared_memory *shmem; // shared memory
	stack_buffer_shared_memory snapshot; // memory snapshot of the remote process' shared memory
	uint32_t		last_pre_written_index_size;
	uint64_t		last_index_file_offset;
	backtrace_uniquing_table uniquing_table; // snapshot of the remote process' uniquing table
} remote_index_cache;

// for reading stack history information from remote processes:
typedef struct {
	task_t			remote_task;
	pid_t			remote_pid;
	int32_t			task_is_64_bit;
	int32_t			in_use_count;
	FILE			*index_file_stream;
	remote_index_cache	*cache;
} remote_task_file_streams;

#pragma mark -
#pragma mark Constants/Globals

static OSSpinLock stack_logging_lock = OS_SPINLOCK_INIT;

// support for multi-threaded forks
extern void __stack_logging_fork_prepare();
extern void __stack_logging_fork_parent();
extern void __stack_logging_fork_child();
extern void __stack_logging_early_finished();

// support for gdb and others checking for stack_logging locks
__private_extern__ boolean_t __stack_logging_locked();

// single-thread access variables
static stack_buffer_shared_memory *pre_write_buffers;
static vm_address_t *stack_buffer;
static uintptr_t last_logged_malloc_address = 0;

// Constants to define stack logging file path names.
// Files will get written as /tmp/stack-logs.<pid>.<progname>.XXXXXX.index
// unless the base directory is specified otherwise with MallocStackLoggingDirectory.
// In this case, a file /tmp/stack-logs.<pid>.<progname>.XXXXXX.link will also be created.
static const char *stack_log_file_base_name = "stack-logs.";
static const char *stack_log_file_suffix = ".index";
static const char *stack_log_link_suffix = ".link";

static void *stack_log_path_buffers = NULL;
static char *stack_log_location = NULL;
static char *stack_log_reference_file = NULL;
char *__stack_log_file_path__ = NULL;
static int index_file_descriptor = -1;

// for accessing remote log files
static remote_task_file_streams remote_fds[STACK_LOGGING_MAX_SIMUL_REMOTE_TASKS_INSPECTED];
static uint32_t next_remote_task_fd = 0;
static uint32_t remote_task_fd_count = 0;
static OSSpinLock remote_fd_list_lock = OS_SPINLOCK_INIT;

// activation variables
static int logging_use_compaction = 1; // set this to zero to always disable compaction.

// We set malloc_logger to NULL to disable logging, if we encounter errors
// during file writing
typedef void (malloc_logger_t)(uint32_t type, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t result, uint32_t num_hot_frames_to_skip);
extern malloc_logger_t *malloc_logger;

#pragma mark -
#pragma mark In-Memory Backtrace Uniquing

static __attribute__((always_inline))
inline void* 
allocate_pages(uint64_t memSize)
{
    mach_vm_address_t allocatedMem = 0ull;
	if (mach_vm_allocate(mach_task_self(), &allocatedMem, memSize, VM_FLAGS_ANYWHERE | VM_MAKE_TAG(VM_MEMORY_ANALYSIS_TOOL)) != KERN_SUCCESS) {
		malloc_printf("allocate_pages(): virtual memory exhaused!\n");
    }
    return (void*)(uintptr_t)allocatedMem;
}

static __attribute__((always_inline))
inline int 
deallocate_pages(void* memPointer, uint64_t memSize)
{
	return mach_vm_deallocate(mach_task_self(), (mach_vm_address_t)(uintptr_t)memPointer, memSize);
}

static backtrace_uniquing_table* 
__create_uniquing_table(void)
{
	backtrace_uniquing_table *uniquing_table = (backtrace_uniquing_table*)allocate_pages((uint64_t)round_page(sizeof(backtrace_uniquing_table)));
	if (!uniquing_table) return NULL;
	bzero(uniquing_table, sizeof(backtrace_uniquing_table));
	uniquing_table->numPages = DEFAULT_UNIQUING_PAGE_SIZE;
	uniquing_table->tableSize = uniquing_table->numPages * vm_page_size;
	uniquing_table->numNodes = ((uniquing_table->tableSize / (sizeof(mach_vm_address_t) * 2)) >> 1) << 1; // make sure it's even.
	uniquing_table->table = (mach_vm_address_t*)(uintptr_t)allocate_pages(uniquing_table->tableSize);
	uniquing_table->table_address = (uintptr_t)uniquing_table->table;
	uniquing_table->max_collide = INITIAL_MAX_COLLIDE;
	uniquing_table->untouchableNodes = 0;
	
#if BACKTRACE_UNIQUING_DEBUG
	malloc_printf("create_uniquing_table(): creating. size: %lldKB == %lldMB, numnodes: %lld (%lld untouchable)\n", uniquing_table->tableSize >> 10, uniquing_table->tableSize >> 20, uniquing_table->numNodes, uniquing_table->untouchableNodes);
	malloc_printf("create_uniquing_table(): table: %p; end: %p\n", uniquing_table->table, (void*)((uintptr_t)uniquing_table->table + (uintptr_t)uniquing_table->tableSize));
#endif
	return uniquing_table;
}

static void
__destroy_uniquing_table(backtrace_uniquing_table* table)
{
	deallocate_pages(table->table, table->tableSize);
	deallocate_pages(table, sizeof(backtrace_uniquing_table));
}

static void 
__expand_uniquing_table(backtrace_uniquing_table *uniquing_table)
{
	mach_vm_address_t *oldTable = uniquing_table->table;
	uint64_t oldsize = uniquing_table->tableSize;
	uint64_t oldnumnodes = uniquing_table->numNodes;
	
	uniquing_table->numPages = uniquing_table->numPages << EXPAND_FACTOR;
	uniquing_table->tableSize = uniquing_table->numPages * vm_page_size;
	uniquing_table->numNodes = ((uniquing_table->tableSize / (sizeof(mach_vm_address_t) * 2)) >> 1) << 1; // make sure it's even.
	mach_vm_address_t *newTable = (mach_vm_address_t*)(uintptr_t)allocate_pages(uniquing_table->tableSize);
	
	uniquing_table->table = newTable;
	uniquing_table->table_address = (uintptr_t)uniquing_table->table;
	uniquing_table->max_collide = uniquing_table->max_collide + COLLISION_GROWTH_RATE;
	
	if (mach_vm_copy(mach_task_self(), (mach_vm_address_t)(uintptr_t)oldTable, oldsize, (mach_vm_address_t)(uintptr_t)newTable) != KERN_SUCCESS) {
		malloc_printf("expandUniquingTable(): VMCopyFailed\n");
	}
	uniquing_table->untouchableNodes = oldnumnodes;
	
#if BACKTRACE_UNIQUING_DEBUG
	malloc_printf("expandUniquingTable(): expanded from nodes full: %lld of: %lld (~%2d%%); to nodes: %lld (inactive = %lld); unique bts: %lld\n", 
				  uniquing_table->nodesFull, oldnumnodes, (int)(((uniquing_table->nodesFull * 100.0) / (double)oldnumnodes) + 0.5),
				  uniquing_table->numNodes, uniquing_table->untouchableNodes, uniquing_table->backtracesContained);
	malloc_printf("expandUniquingTable(): allocate: %p; end: %p\n", newTable, (void*)((uintptr_t)newTable + (uintptr_t)(uniquing_table->tableSize)));
	malloc_printf("expandUniquingTable(): deallocate: %p; end: %p\n", oldTable, (void*)((uintptr_t)oldTable + (uintptr_t)oldsize));
#endif
	
	if (deallocate_pages(oldTable, oldsize) != KERN_SUCCESS) {
		malloc_printf("expandUniquingTable(): mach_vm_deallocate failed. [%p]\n", uniquing_table->table);
	}
}

static int 
__enter_frames_in_table(backtrace_uniquing_table *uniquing_table, uint64_t *foundIndex, mach_vm_address_t *frames, int32_t count)
{	
	// The hash values need to be the same size as the addresses (because we use the value -1), for clarity, define a new type
	typedef mach_vm_address_t hash_index_t;

	mach_vm_address_t thisPC;
	hash_index_t hash, uParent = (hash_index_t)(-1ll), modulus = (uniquing_table->numNodes-uniquing_table->untouchableNodes-1);
	int32_t collisions, lcopy = count, returnVal = 1;
	hash_index_t hash_multiplier = ((uniquing_table->numNodes - uniquing_table->untouchableNodes)/(uniquing_table->max_collide*2+1));
	mach_vm_address_t *node;
	while (--lcopy >= 0) {
        thisPC = frames[lcopy];
		
		// hash = initialHash(uniquing_table, uParent, thisPC);
		hash = uniquing_table->untouchableNodes + (((uParent << 4) ^ (thisPC >> 2)) % modulus);
		collisions = uniquing_table->max_collide;
		
		while (collisions--) {
			node = uniquing_table->table + (hash * 2);
			
			if (*node == 0 && node[1] == 0) { 
				// blank; store this entry!
				// Note that we need to test for both head[0] and head[1] as (0, -1) is a valid entry
				node[0] = thisPC;
				node[1] = uParent;
				uParent = hash;
#if BACKTRACE_UNIQUING_DEBUG
				uniquing_table->nodesFull++;
				if (lcopy == 0) {
					uniquing_table->backtracesContained++;
				}
#endif
				break;
			}
			if (*node == thisPC && node[1] == uParent) {
				// hit! retrieve index and go.
				uParent = hash;
				break;
			}
			
			hash += collisions * hash_multiplier + 1;
			
			if (hash >= uniquing_table->numNodes) {
				hash -= (uniquing_table->numNodes - uniquing_table->untouchableNodes); // wrap around.
			}
		}
		
		if (collisions < 0) {
			returnVal = 0;
			break;
		}
    }
	
	if (returnVal) *foundIndex = uParent;
	
	return returnVal;
}

static void 
__unwind_stack_from_table_index(backtrace_uniquing_table *uniquing_table, uint64_t index_pos, mach_vm_address_t *out_frames_buffer, uint32_t *out_frames_count, uint32_t max_frames)
{	
	mach_vm_address_t *node = uniquing_table->table + (index_pos * 2);
	uint32_t foundFrames = 0;
	if (index_pos < uniquing_table->numNodes) {
		while (foundFrames < max_frames) {
			out_frames_buffer[foundFrames++] = node[0];
			if (node[1] == (mach_vm_address_t)(-1ll)) break;
			node = uniquing_table->table + (node[1] * 2);
		}
	}
	
	*out_frames_count = foundFrames;
}

#pragma mark -
#pragma mark Disk Stack Logging

static void delete_log_files(void);	// pre-declare
static int delete_logging_file(char *log_location);

static void
append_int(char * filename, pid_t pid, size_t maxLength)
{	
	size_t len = strlen(filename);
	
	uint32_t count = 0;
	pid_t value = pid;
	while (value > 0) {
		value /= 10;
		count++;
	}
	
	if (len + count >= maxLength) return; // don't modify the string if it would violate maxLength
	
	filename[len + count] = '\0';
	
	value = pid;
	uint32_t i;
	for (i = 0 ; i < count ; i ++) {
		filename[len + count - 1 - i] = '0' + value % 10;
		value /= 10;
	}
}

/*
 * <rdar://problem/11128080> if we needed to call confstr during init then setting this
 * flag will postpone stack logging until after Libsystem's initialiser has run.
 */
static void
postpone_stack_logging(void)
{
	_malloc_printf(ASL_LEVEL_INFO, "stack logging postponed until after initialization.\n");
	stack_logging_postponed = 1;
}

/*
 * Check various temporary directory options starting with _PATH_TMP and use confstr.
 * Allocating and releasing target buffer is the caller's responsibility.
 */
static bool
get_writeable_temp_dir(char* target)
{
	if (!target) return false;
	if (-1 != access(_PATH_TMP, W_OK)) {
		strlcpy(target, _PATH_TMP, (size_t)PATH_MAX);
		return true;
	}
	if (getenv("TMPDIR") && (-1 != access(getenv("TMPDIR"), W_OK))) {
		strlcpy(target, getenv("TMPDIR"), (size_t)PATH_MAX);
		return true;
	}
	if (stack_logging_finished_init) {
		size_t n = confstr(_CS_DARWIN_USER_TEMP_DIR, target, (size_t) PATH_MAX);
		if ((n > 0) && (n < PATH_MAX)) return true;
		n = confstr(_CS_DARWIN_USER_CACHE_DIR, target, (size_t) PATH_MAX);
		if ((n > 0) && (n < PATH_MAX)) return true;
	} else {
		/* <rdar://problem/11128080> Can't call confstr during init, so postpone
		 logging till after */
		postpone_stack_logging();
	}
	/* No writeable tmp directory found. Maybe shd try /private/var/tmp for device here ... */
	*target = '\0';
	return false;
}

/*
 * If successful, returns path to log file that was created, otherwise NULL.
 *
 * The log could be in one of 3 places (in decreasing order of preference)
 *
 * 1)  value of environment variable MallocStackLoggingDirectory
 * 2)  the temp directory /tmp for desktop apps and internal apps on devices, or
 * 3)  the sandbox location + tmp/ in case of 3rd party apps on the device.
 *
 * For 1 and 3, we create a .link file with the path of the file. We prefer to
 * create this file in /tmp, but if we are unable to (device 3rd party case),
 * we create it in the same location as the .index file and issue a message
 * in syslog asking for it to be copied to /tmp to enable tools.
 *
 */
static char *
create_log_file(void)
{
	pid_t pid = getpid();
	const char *progname = getprogname();
	char *created_log_location = NULL;

	if (stack_log_path_buffers == NULL) {
		/*
		 * on first use, allocate buffers directly from the OS without
		 * using malloc
		 */

		stack_log_path_buffers = allocate_pages((uint64_t)round_page(3*PATH_MAX));
		if (stack_log_path_buffers == NULL) {
			_malloc_printf(ASL_LEVEL_INFO, "unable to allocate memory for path buffers\n");
			return NULL;
		}
		
		stack_log_location = &((char *)stack_log_path_buffers)[0*PATH_MAX];
		stack_log_reference_file = &((char *)stack_log_path_buffers)[1*PATH_MAX];
		__stack_log_file_path__ = &((char *)stack_log_path_buffers)[2*PATH_MAX];
	}
	
	// WARNING! use of snprintf can induce malloc() calls 
	bool use_alternate_location = false;
	char *evn_log_directory = getenv("MallocStackLoggingDirectory");
	size_t stack_log_len;
	if (evn_log_directory && *evn_log_directory) {
		use_alternate_location = true;
		strlcpy(stack_log_location, evn_log_directory, (size_t)PATH_MAX);
	}
	if (!use_alternate_location || (access(stack_log_location, W_OK) == -1)) {
		if (!get_writeable_temp_dir(stack_log_location)) {
			if (!stack_logging_postponed) {
				_malloc_printf(ASL_LEVEL_INFO, "No writeable tmp dir\n");
			}
			return NULL;
		}
		if (0 != strcmp(stack_log_location, _PATH_TMP))
			use_alternate_location = true;
	}
	stack_log_len = strlen(stack_log_location);
	// add the '/' only if it's not already there.
	if (stack_log_location[stack_log_len-1] != '/') {
		strlcat(stack_log_location, "/", (size_t)PATH_MAX);
		++stack_log_len;
	}
	
	strlcpy(__stack_log_file_path__, stack_log_location, (size_t)PATH_MAX);
	
	strlcat(__stack_log_file_path__, stack_log_file_base_name, (size_t)PATH_MAX);
	append_int(__stack_log_file_path__, pid, (size_t)PATH_MAX);
	if (progname && progname[0] != '\0') {
		strlcat(__stack_log_file_path__, ".", (size_t)PATH_MAX);
		strlcat(__stack_log_file_path__, progname, (size_t)PATH_MAX);
	}
	if (!use_alternate_location) strlcat(__stack_log_file_path__, ".XXXXXX", (size_t)PATH_MAX);
	strlcat(__stack_log_file_path__, stack_log_file_suffix, (size_t)PATH_MAX);
	
	// Securely create the log file.
	if ((index_file_descriptor = mkstemps(__stack_log_file_path__, (int)strlen(stack_log_file_suffix))) != -1) {
		_malloc_printf(ASL_LEVEL_INFO, "stack logs being written into %s\n", __stack_log_file_path__);
		created_log_location = __stack_log_file_path__;
	} else {
		_malloc_printf(ASL_LEVEL_INFO, "unable to create stack logs at %s\n", stack_log_location);
		if (use_alternate_location) delete_logging_file(stack_log_reference_file);
		stack_log_reference_file[0] = '\0';
		stack_log_location[0] = '\0';
		__stack_log_file_path__[0] = '\0';
		created_log_location = NULL;
		return created_log_location;
	}
	
	// in the case where the user has specified an alternate location, drop a reference file
	// in /tmp with the suffix 'stack_log_link_suffix' (".link") and save the path of the
	// stack logging file there.
	bool use_alternate_link_location = false;
	if (use_alternate_location) {
		strlcpy(stack_log_reference_file, _PATH_TMP, (size_t)PATH_MAX);
		if (access(stack_log_reference_file, W_OK) == -1) {
			strlcpy(stack_log_reference_file, stack_log_location, (size_t)PATH_MAX);
			use_alternate_link_location = true;
		}
		strlcat(stack_log_reference_file, stack_log_file_base_name, (size_t)PATH_MAX);
		append_int(stack_log_reference_file, pid, (size_t)PATH_MAX);
		if (progname && progname[0] != '\0') {
			strlcat(stack_log_reference_file, ".", (size_t)PATH_MAX);
			strlcat(stack_log_reference_file, progname, (size_t)PATH_MAX);
		}
		if (!use_alternate_link_location)
		strlcat(stack_log_reference_file, ".XXXXXX", (size_t)PATH_MAX);
		strlcat(stack_log_reference_file, ".XXXXXX", (size_t)PATH_MAX);
		strlcat(stack_log_reference_file, stack_log_link_suffix, (size_t)PATH_MAX);
		
		int link_file_descriptor = mkstemps(stack_log_reference_file, (int)strlen(stack_log_link_suffix));
		if (link_file_descriptor == -1) {
			_malloc_printf(ASL_LEVEL_INFO, "unable to create stack reference file %s at %s\n",
				stack_log_reference_file, stack_log_location);
		} else {
			ssize_t written = write(link_file_descriptor, __stack_log_file_path__, strlen(__stack_log_file_path__));
			if (written < (ssize_t)strlen(__stack_log_file_path__)) {
				_malloc_printf(ASL_LEVEL_INFO, "unable to write to stack reference file %s at %s\n",
					stack_log_reference_file, stack_log_location);
			} else {
				const char *description_string = "\n(This is a reference file to the stack logs at the path above.)\n";
				write(link_file_descriptor, description_string, strlen(description_string));
		}
		}
		close(link_file_descriptor);
	}
	if (use_alternate_link_location) {
		_malloc_printf(ASL_LEVEL_INFO, "Please issue: cp %s %s\n", stack_log_reference_file, _PATH_TMP);
	}
	return created_log_location;
}

// Check to see if the log file is actually a reference to another location
static int
log_file_is_reference(char *log_location, char *out_reference_loc_buffer, size_t max_reference_path_size)
{	
	if (log_location == NULL || log_location[0] == '\0') return 0;
	
	size_t log_len = strlen(log_location);
	size_t link_suffix_len = strlen(stack_log_link_suffix);
	if (log_len < link_suffix_len || strncmp(log_location+log_len-link_suffix_len, stack_log_link_suffix, link_suffix_len) != 0) {
		// not a reference file.
		return 0;
	}
	
	if (!out_reference_loc_buffer || max_reference_path_size == 0) return 1;
	
	FILE *reference_file = fopen(log_location, "r");
	if (reference_file == NULL) {
		// if unable to open the file, it may be because another user created it; no need to warn.
		out_reference_loc_buffer[0] = '\0';
		return 1;
	}
	
	char *ret = fgets(out_reference_loc_buffer, (int)max_reference_path_size, reference_file);
	if (!ret) {
		out_reference_loc_buffer[0] = '\0';
		_malloc_printf(ASL_LEVEL_INFO, "unable to read from stack logging reference file at %s\n", log_location);
		return 1;
	} else {
		size_t read_line_len = strlen(out_reference_loc_buffer);
		if (read_line_len >= 1 && out_reference_loc_buffer[read_line_len-1] == '\n') {
			out_reference_loc_buffer[read_line_len-1] = '\0';
		}
	}
	
	fclose(reference_file);
	
	return 1;
}

// This function may be called from either the target process when exiting, or from either the the target process or 
// a stack log analysis process, when reaping orphaned stack log files.
// Returns -1 if the files exist and they couldn't be removed, returns 0 otherwise.
static int
delete_logging_file(char *log_location)
{
	if (log_location == NULL || log_location[0] == '\0') return 0;

	struct stat statbuf;
	if (unlink(log_location) != 0 && stat(log_location, &statbuf) == 0) {
		return -1;
	}
	return 0;
}

// This function will be called from atexit() in the target process.
static void
delete_log_files(void)
{
	if (__stack_log_file_path__ && __stack_log_file_path__[0]) {
		if (delete_logging_file(__stack_log_file_path__) == 0) {
			_malloc_printf(ASL_LEVEL_INFO, "stack logs deleted from %s\n", __stack_log_file_path__);
			__stack_log_file_path__[0] = '\0';
		} else {
			_malloc_printf(ASL_LEVEL_INFO, "unable to delete stack logs from %s\n", __stack_log_file_path__);
		}
	}
	if (stack_log_reference_file && stack_log_reference_file[0]) {
		delete_logging_file(stack_log_reference_file);
	}
}

static bool
is_process_running(pid_t pid)
{
	struct kinfo_proc kpt[1];
	size_t size = sizeof(struct kinfo_proc);
	int mib[] = {CTL_KERN, KERN_PROC, KERN_PROC_PID, pid};

	sysctl(mib, 4, kpt, &size, NULL, (size_t)0); // size is either 1 or 0 entries when we ask for a single pid
    
	return (size==sizeof(struct kinfo_proc));
}

// The log files can be quite large and aren't too useful after the process that created them no longer exists.
// Normally they should get removed when the process exits, but if the process crashed the log files might remain.
// So, reap any stack log files for processes that no longer exist.
//
// lf the remove_for_this_pid flag is set, then any log files that already exist for the current process will also be deleted.
// Those log files are probably the result of this process having been exec'ed from another one (without a fork()).
// The remove_for_this_pid flag is only set for a target process (one just starting logging);  a stack logging "client"
// process reaps log files too, but if we're using stack logging on the client process itself, then we don't want to remove
// its own log files.
static void
reap_orphaned_log_files(bool remove_for_this_pid)
{
	DIR *dp;
	struct dirent *entry;
	char prefix_name[PATH_MAX];
	char pathname[PATH_MAX];
	pid_t current_pid = getpid();

	if ((dp = opendir(_PATH_TMP)) == NULL) {
		return;
	}

	strlcpy(prefix_name, stack_log_file_base_name, (size_t)PATH_MAX);
	size_t prefix_length = strlen(prefix_name);

	while ( (entry = readdir(dp)) != NULL ) {
		if ( entry->d_type != DT_DIR && entry->d_type != DT_LNK && ( strncmp( entry->d_name, prefix_name, prefix_length) == 0 ) ) {
			long pid = strtol(&entry->d_name[prefix_length], (char **)NULL, 10);
			if ( (! is_process_running((pid_t)pid)) || (remove_for_this_pid && (pid_t)pid == current_pid) ) {
				strlcpy(pathname, _PATH_TMP, (size_t)PATH_MAX);
				strlcat(pathname, entry->d_name, (size_t)PATH_MAX);
				char reference_file_buffer[PATH_MAX];
				bool pathname_is_ref_file = false;
				if (log_file_is_reference(pathname, reference_file_buffer, (size_t)PATH_MAX) && *reference_file_buffer) {
					pathname_is_ref_file = true;
					if (delete_logging_file(reference_file_buffer) == 0) {
						if (remove_for_this_pid && pid == current_pid) {
							_malloc_printf(ASL_LEVEL_INFO, "stack logs deleted from %s\n", reference_file_buffer);
						} else {
							_malloc_printf(ASL_LEVEL_INFO, "process %ld no longer exists, stack logs deleted from %s\n", pid, reference_file_buffer);
						}
					}
				}
				if (delete_logging_file(pathname) == 0) {
					if (remove_for_this_pid && pid == current_pid) {
						if (!pathname_is_ref_file) _malloc_printf(ASL_LEVEL_INFO, "stack logs deleted from %s\n", pathname);
					} else {
						if (!pathname_is_ref_file) _malloc_printf(ASL_LEVEL_INFO, "process %ld no longer exists, stack logs deleted from %s\n", pid, pathname);
					}
					char shmem_name_string[PATH_MAX];
					strlcpy(shmem_name_string, stack_log_file_base_name, (size_t)PATH_MAX);
					append_int(shmem_name_string, (pid_t)pid, (size_t)PATH_MAX);
					if (pid != current_pid) shm_unlink(shmem_name_string);
				}
			}
		}
	}
	closedir(dp);
}	

/*
 * Since there a many errors that could cause stack logging to get disabled, this is a convenience method
 * for disabling any future logging in this process and for informing the user.
 */
static void
disable_stack_logging(void)
{	
	_malloc_printf(ASL_LEVEL_INFO, "stack logging disabled due to previous errors.\n");
	stack_logging_enable_logging = 0;
	malloc_logger = NULL;
}

/* A wrapper around write() that will try to reopen the index/stack file and
 * write to it if someone closed it underneath us (e.g. the process we just
 * started decide to close all file descriptors except stin/err/out). Some
 * programs like to do that and calling abort() on them is rude.
 */
static ssize_t
robust_write(int fd, const void *buf, size_t nbyte) {
	extern int errno;
	ssize_t written = write(fd, buf, nbyte);
	if (written == -1 && errno == EBADF) {
		char *file_to_reopen = NULL;
		int *fd_to_reset = NULL;

		// descriptor was closed on us. We need to reopen it
		if (fd == index_file_descriptor) {
			file_to_reopen = __stack_log_file_path__;
			fd_to_reset = &index_file_descriptor;
		} else {
			// We don't know about this file. Return (and abort()).
			_malloc_printf(ASL_LEVEL_INFO, "Unknown file descriptor; expecting stack logging index file\n");
			return -1;
		}

		// The file *should* already exist. If not, fail.
		fd = open(file_to_reopen, O_WRONLY | O_APPEND);
		if (fd < 3) {
			// If we somehow got stdin/out/err, we need to relinquish them and
			// get another fd.
			int fds_to_close[3] = { 0 };
			while (fd < 3) {
				if (fd == -1) {
					_malloc_printf(ASL_LEVEL_INFO, "unable to re-open stack logging file %s\n", file_to_reopen);
					delete_log_files();
					return -1;
				}
				fds_to_close[fd] = 1;
				fd = dup(fd);
			}

			// We have an fd we like. Close the ones we opened.
			if (fds_to_close[0]) close(0);
			if (fds_to_close[1]) close(1);
			if (fds_to_close[2]) close(2);
		}

		*fd_to_reset = fd;
		written = write(fd, buf, nbyte);
	}
	return written;
}

static void
flush_data(void)
{
	ssize_t written; // signed size_t
	size_t remaining;
	char * p;
	
	if (index_file_descriptor == -1) {
		if (create_log_file() == NULL) {
			return;
		}
	}
	
	// Write the events before the index so that hopefully the events will be on disk if the index refers to them.
	p = pre_write_buffers->index_buffer;
	remaining = (size_t)pre_write_buffers->next_free_index_buffer_offset;
	while (remaining > 0) {
		written = robust_write(index_file_descriptor, p, remaining);
		if (written == -1) {
			_malloc_printf(ASL_LEVEL_INFO, "Unable to write to stack logging file %s (%s)\n",
				__stack_log_file_path__, strerror(errno));
			disable_stack_logging();
			return;
		}
		p += written;
		remaining -= written;
	}
	
	pre_write_buffers->start_index_offset += pre_write_buffers->next_free_index_buffer_offset;
	pre_write_buffers->next_free_index_buffer_offset = 0;
}

static void
prepare_to_log_stacks(void)
{
	if (!pre_write_buffers) {
		last_logged_malloc_address = 0ul;
		logging_use_compaction = (stack_logging_dontcompact ? 0 : logging_use_compaction);
		
		// Create a shared memory region to hold the pre-write index and stack buffers. This will allow remote analysis processes to access
		// these buffers to get logs for even the most recent allocations. The remote process will need to pause this process to assure that
		// the contents of these buffers don't change while being inspected.
		char shmem_name_string[PATH_MAX];
		strlcpy(shmem_name_string, stack_log_file_base_name, (size_t)PATH_MAX);
		append_int(shmem_name_string, getpid(), (size_t)PATH_MAX);
		
		int shmid = shm_open(shmem_name_string, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
		if (shmid < 0) {
			// Failed to create shared memory region; turn off stack logging.
			_malloc_printf(ASL_LEVEL_INFO, "error while allocating shared memory for disk-based stack logging output buffers\n");
			disable_stack_logging();
			return;
		}
		
		size_t full_shared_mem_size = sizeof(stack_buffer_shared_memory);
		ftruncate(shmid, (off_t)full_shared_mem_size);
		pre_write_buffers = (stack_buffer_shared_memory*)mmap(0, full_shared_mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, shmid, (off_t)0);
		close(shmid);
		
		if (MAP_FAILED == pre_write_buffers) {
			_malloc_printf(ASL_LEVEL_INFO, "error mapping in shared memory for disk-based stack logging output buffers\n");
			disable_stack_logging();
			return;
		}

		// Store and use the buffer offsets in shared memory so that they can be accessed remotely
		pre_write_buffers->start_index_offset = 0ull;
		pre_write_buffers->next_free_index_buffer_offset = 0;
		
		// create the backtrace uniquing table
		pre_write_buffers->uniquing_table = __create_uniquing_table();
		pre_write_buffers->uniquing_table_address = (mach_vm_address_t)(uintptr_t)pre_write_buffers->uniquing_table;
		if (!pre_write_buffers->uniquing_table) {
			_malloc_printf(ASL_LEVEL_INFO, "error while allocating stack uniquing table\n");
			disable_stack_logging();
			return;
		}

		uint64_t stack_buffer_sz = (uint64_t)round_page(sizeof(vm_address_t) * STACK_LOGGING_MAX_STACK_SIZE);
		stack_buffer = (vm_address_t*)allocate_pages(stack_buffer_sz);
		if (!stack_buffer) {
			_malloc_printf(ASL_LEVEL_INFO, "error while allocating stack trace buffer\n");
			disable_stack_logging();
			return;
		}
		
		// malloc() can be called by the following, so these need to be done outside the stack_logging_lock but after the buffers have been set up.
		atexit(delete_log_files);		// atexit() can call malloc()
		reap_orphaned_log_files(true);		// this calls opendir() which calls malloc()

		// this call ensures that the log files exist; analyzing processes will rely on this assumption.
		if (create_log_file() == NULL) {
			/* postponement support requires cleaning up these structures now */
			__destroy_uniquing_table(pre_write_buffers->uniquing_table);
			deallocate_pages(stack_buffer, stack_buffer_sz);
			stack_buffer = NULL;

			munmap(pre_write_buffers, full_shared_mem_size);
			pre_write_buffers = NULL;

			if (!stack_logging_postponed) {
				disable_stack_logging();
			}
			return;
		}
	}
}

void
__disk_stack_logging_log_stack(uint32_t type_flags, uintptr_t zone_ptr, uintptr_t size, uintptr_t ptr_arg, uintptr_t return_val, uint32_t num_hot_to_skip)
{
	if (!stack_logging_enable_logging || stack_logging_postponed) return;
	
	// check incoming data
	if (type_flags & stack_logging_type_alloc && type_flags & stack_logging_type_dealloc) {
		uintptr_t swapper = size;
		size = ptr_arg;
		ptr_arg = swapper;
		if (ptr_arg == return_val) return; // realloc had no effect, skipping
		
		if (ptr_arg == 0) { // realloc(NULL, size) same as malloc(size)
			type_flags ^= stack_logging_type_dealloc;
		} else {
			// realloc(arg1, arg2) -> result is same as free(arg1); malloc(arg2) -> result
			__disk_stack_logging_log_stack(stack_logging_type_dealloc, zone_ptr, ptr_arg, (uintptr_t)0, (uintptr_t)0, num_hot_to_skip + 1);
			__disk_stack_logging_log_stack(stack_logging_type_alloc, zone_ptr, size, (uintptr_t)0, return_val, num_hot_to_skip + 1);
			return;
		}
	}
	if (type_flags & stack_logging_type_dealloc) {
		if (size) {
			ptr_arg = size;
			size = 0;
		} else return; // free(nil)
	}
	if (type_flags & stack_logging_type_alloc && return_val == 0) return; // alloc that failed
	
	type_flags &= 0x7;
	
	// now actually begin
	prepare_to_log_stacks();

	// since there could have been a fatal (to stack logging) error such as the log files not being created, check this variable before continuing
	if (!stack_logging_enable_logging || stack_logging_postponed) return;

	vm_address_t self_thread = (vm_address_t)pthread_self();	// use pthread_self() rather than mach_thread_self() to avoid system call
	
	// lock and enter
	OSSpinLockLock(&stack_logging_lock);
	
	if (!stack_logging_enable_logging) {
		OSSpinLockUnlock(&stack_logging_lock);
		return;
	}
	
	// compaction
	if (last_logged_malloc_address && (type_flags & stack_logging_type_dealloc) && STACK_LOGGING_DISGUISE(ptr_arg) == last_logged_malloc_address) {
		// *waves hand* the last allocation never occurred
		pre_write_buffers->next_free_index_buffer_offset -= (uint32_t)sizeof(stack_logging_index_event);
		last_logged_malloc_address = 0ul;
		
		OSSpinLockUnlock(&stack_logging_lock);
		return;
	}

	// gather stack
	uint32_t count;
	thread_stack_pcs(stack_buffer, STACK_LOGGING_MAX_STACK_SIZE-1, &count); // only gather up to STACK_LOGGING_MAX_STACK_SIZE-1 since we append thread id
	stack_buffer[count++] = self_thread + 1;		// stuffing thread # in the coldest slot.  Add 1 to match what the old stack logging did.
	num_hot_to_skip += 2;
	if (count <= num_hot_to_skip) {
		// Oops!  Didn't get a valid backtrace from thread_stack_pcs().
		OSSpinLockUnlock(&stack_logging_lock);
		return;
	}
		
	// unique stack in memory
	count -= num_hot_to_skip;
#if __LP64__ 
	mach_vm_address_t *frames = (mach_vm_address_t*)stack_buffer + num_hot_to_skip;
#else
	mach_vm_address_t frames[STACK_LOGGING_MAX_STACK_SIZE];
	uint32_t i;
	for (i = 0; i < count; i++) {
		frames[i] = stack_buffer[i+num_hot_to_skip];
	}
#endif

	uint64_t uniqueStackIdentifier = (uint64_t)(-1ll);
	while (!__enter_frames_in_table(pre_write_buffers->uniquing_table, &uniqueStackIdentifier, frames, (int32_t)count)) {
		__expand_uniquing_table(pre_write_buffers->uniquing_table);
    }
	
	stack_logging_index_event current_index;
	if (type_flags & stack_logging_type_alloc) {
		current_index.address = STACK_LOGGING_DISGUISE(return_val);
		current_index.argument = size;
		if (logging_use_compaction) {
			last_logged_malloc_address = current_index.address; // disguised
		}
	} else {
		current_index.address = STACK_LOGGING_DISGUISE(ptr_arg);
		current_index.argument = 0ul;
		last_logged_malloc_address = 0ul;
	}
	current_index.offset_and_flags = STACK_LOGGING_OFFSET_AND_FLAGS(uniqueStackIdentifier, type_flags);

//	the following line is a good debugging tool for logging each allocation event as it happens.
//	malloc_printf("{0x%lx, %lld}\n", STACK_LOGGING_DISGUISE(current_index.address), uniqueStackIdentifier);
		
	// flush the data buffer to disk if necessary
	if (pre_write_buffers->next_free_index_buffer_offset + sizeof(stack_logging_index_event) >= STACK_LOGGING_BLOCK_WRITING_SIZE) {
		flush_data();
	}
	
	// store bytes in buffers
	memcpy(pre_write_buffers->index_buffer+pre_write_buffers->next_free_index_buffer_offset, &current_index, sizeof(stack_logging_index_event));
	pre_write_buffers->next_free_index_buffer_offset += (uint32_t)sizeof(stack_logging_index_event);
	
	OSSpinLockUnlock(&stack_logging_lock);
}

void 
__stack_logging_fork_prepare() {
	OSSpinLockLock(&stack_logging_lock);
}

void
__stack_logging_fork_parent() {
	OSSpinLockUnlock(&stack_logging_lock);
}

void
__stack_logging_fork_child() {
	malloc_logger = NULL;
	stack_logging_enable_logging = 0;
	OSSpinLockUnlock(&stack_logging_lock);
}

void
__stack_logging_early_finished() {
	stack_logging_finished_init = 1;
	stack_logging_postponed = 0;
}

boolean_t
__stack_logging_locked()
{
	bool acquired_lock = OSSpinLockTry(&stack_logging_lock);
	if (acquired_lock) OSSpinLockUnlock(&stack_logging_lock);
	return (acquired_lock ? false : true);
}

#pragma mark -
#pragma mark Remote Stack Log Access

#pragma mark - Design notes:

/*

this first one will look through the index, find the "stack_identifier" (i.e. the offset in the log file), and call the third function listed here.
extern kern_return_t __mach_stack_logging_get_frames(task_t task, mach_vm_address_t address, mach_vm_address_t *stack_frames_buffer, uint32_t max_stack_frames, uint32_t *num_frames);
    //  Gets the last allocation record about address
	
if !address, will load index and iterate through (expensive)
else will load just index, search for stack, and then use third function here to retrieve. (also expensive)
extern kern_return_t __mach_stack_logging_enumerate_records(task_t task, mach_vm_address_t address, void enumerator(mach_stack_logging_record_t, void *), void *context);
    // Applies enumerator to all records involving address sending context as enumerator's second parameter; if !address, applies enumerator to all records 

this function will load the stack file, look for the stack, and follow up to STACK_LOGGING_FORCE_FULL_BACKTRACE_EVERY references to reconstruct.
extern kern_return_t __mach_stack_logging_frames_for_uniqued_stack(task_t task, uint64_t stack_identifier, mach_vm_address_t *stack_frames_buffer, uint32_t max_stack_frames, uint32_t *count);
    // Given a uniqued_stack fills stack_frames_buffer

*/

#pragma mark - caching

__attribute__((always_inline)) static inline size_t 
hash_index(uint64_t address, size_t max_pos) {
	return (size_t)((address >> 2) % (max_pos-1)); // simplicity rules.
}

__attribute__((always_inline)) static inline size_t 
hash_multiplier(size_t capacity, uint32_t allowed_collisions) {
	return (capacity/(allowed_collisions*2+1));
}

__attribute__((always_inline)) static inline size_t 
next_hash(size_t hash, size_t multiplier, size_t capacity, uint32_t collisions) {
	hash += multiplier * collisions;
	if (hash >= capacity) hash -= capacity;
	return hash;
}

static void
transfer_node(remote_index_cache *cache, remote_index_node *old_node)
{
	uint32_t collisions = 0;
	size_t pos = hash_index(old_node->address, cache->cache_node_capacity);
	size_t multiplier = hash_multiplier(cache->cache_node_capacity, cache->collision_allowance);
	do {
		if (cache->table_memory[pos].address == old_node->address) { // hit like this shouldn't happen.
			fprintf(stderr, "impossible collision! two address==address lists! (transfer_node)\n");
			break;
		} else if (cache->table_memory[pos].address == 0) { // empty
			cache->table_memory[pos] = *old_node;
			break;
		} else {
			collisions++;
			pos = next_hash(pos, multiplier, cache->cache_node_capacity, collisions);
		}
	} while (collisions <= cache->collision_allowance);
	
	if (collisions > cache->collision_allowance) {
		fprintf(stderr, "reporting bad hash function! disk stack logging reader %lu bit. (transfer_node)\n", sizeof(void*)*8);
	}
}

static void
expand_cache(remote_index_cache *cache)
{
	// keep old stats
	size_t old_node_capacity = cache->cache_node_capacity;
	remote_index_node *old_table = cache->table_memory;
	
	// double size
	cache->cache_size <<= 2;
	cache->cache_node_capacity <<= 2;
	cache->collision_allowance += 3;
	cache->table_memory = (void*)calloc(cache->cache_node_capacity, sizeof(remote_index_node));
		
	// repopulate (expensive!)
	size_t i;
	for (i = 0; i < old_node_capacity; i++) {
		if (old_table[i].address) {
			transfer_node(cache, &old_table[i]);
		}
	}	
	free(old_table);
//	printf("cache expanded to %0.2f mb (eff: %3.0f%%, capacity: %lu, nodes: %llu, llnodes: %llu)\n", ((float)(cache->cache_size))/(1 << 20), ((float)(cache->cache_node_count)*100.0)/((float)(cache->cache_node_capacity)), cache->cache_node_capacity, cache->cache_node_count, cache->cache_llnode_count);
}

static void
insert_node(remote_index_cache *cache, uint64_t address, uint64_t index_file_offset)
{
	uint32_t collisions = 0;
	size_t pos = hash_index(address, cache->cache_node_capacity);
	size_t multiplier = hash_multiplier(cache->cache_node_capacity, cache->collision_allowance);

	bool inserted = false;
	while (!inserted) {
		if (cache->table_memory[pos].address == 0ull || cache->table_memory[pos].address == address) { // hit or empty
			cache->table_memory[pos].address = address;
			cache->table_memory[pos].index_file_offset = index_file_offset;
			inserted = true;
			break;
		}
		
		collisions++;
		pos = next_hash(pos, multiplier, cache->cache_node_capacity, collisions);

		if (collisions > cache->collision_allowance) {
			expand_cache(cache);
			pos = hash_index(address, cache->cache_node_capacity);
			multiplier = hash_multiplier(cache->cache_node_capacity, cache->collision_allowance);
			collisions = 0;
		}
	}
	
}

static void
update_cache_for_file_streams(remote_task_file_streams *descriptors)
{
	remote_index_cache *cache = descriptors->cache;
	
	// create from scratch if necessary.
	if (!cache) {
		descriptors->cache = cache = (remote_index_cache*)calloc((size_t)1, sizeof(remote_index_cache));
		cache->cache_node_capacity = 1 << 14;
		cache->collision_allowance = 17;
		cache->last_index_file_offset = 0;
		cache->cache_size = cache->cache_node_capacity*sizeof(remote_index_node);
		cache->table_memory = (void*)calloc(cache->cache_node_capacity, sizeof(remote_index_node));
		
		// now map in the shared memory, if possible
		char shmem_name_string[PATH_MAX];
		strlcpy(shmem_name_string, stack_log_file_base_name, (size_t)PATH_MAX);
		append_int(shmem_name_string, descriptors->remote_pid, (size_t)PATH_MAX);
		
		int shmid = shm_open(shmem_name_string, O_RDWR, S_IRUSR | S_IWUSR);
		if (shmid >= 0) {
			cache->shmem = mmap(0, sizeof(stack_buffer_shared_memory), PROT_READ | PROT_WRITE, MAP_SHARED, shmid, (off_t)0);
			close(shmid);
		}
		
		if (shmid < 0 || cache->shmem == MAP_FAILED) {
			// failed to connect to the shared memory region; warn and continue.
			_malloc_printf(ASL_LEVEL_INFO, "warning: unable to connect to remote process' shared memory; allocation histories may not be up-to-date.\n");
		}
	}
	
	// suspend and see how much updating there is to do. there are three scenarios, listed below
	bool update_snapshot = false;
	if (descriptors->remote_task != mach_task_self()) {
		task_suspend(descriptors->remote_task);
	}
		
	struct stat file_statistics;
	fstat(fileno(descriptors->index_file_stream), &file_statistics);
	size_t read_size = (descriptors->task_is_64_bit ? sizeof(stack_logging_index_event64) : sizeof(stack_logging_index_event32));
	uint64_t read_this_update = 0;
	
	// the delta indecies is a complex number; there are three cases:
	// 1. there is no shared memory (or we can't connect); diff the last_index_file_offset from the filesize.
	// 2. the only updates have been in shared memory; disk file didn't change at all. delta_indecies should be zero, scan snapshot only.
	// 3. the updates have flushed to disk, meaning that most likely there is new data on disk that wasn't read from shared memory.
	//    correct delta_indecies for the pre-scanned amount and read the new data from disk and shmem.
	uint64_t delta_indecies = (file_statistics.st_size - cache->last_index_file_offset) / read_size;
	uint32_t last_snapshot_scan_index = 0;
	if (delta_indecies && cache->shmem) { 
		// case 3: add cache scanned to known from disk and recalc
		cache->last_index_file_offset += cache->snapshot.next_free_index_buffer_offset;
		delta_indecies = (file_statistics.st_size - cache->last_index_file_offset) / read_size;
		update_snapshot = true;
	} else if (cache->shmem) {
		// case 2: set the last snapshot scan count so we don't rescan something we've seen.
		last_snapshot_scan_index = cache->snapshot.next_free_index_buffer_offset / (uint32_t)read_size;
	}
	
	// no update necessary for the file; check if need a snapshot.
	if (delta_indecies == 0) {
		if (cache->shmem && !update_snapshot) {
			update_snapshot = (cache->shmem->next_free_index_buffer_offset != cache->snapshot.next_free_index_buffer_offset);
		}
	}
	
	// if a snapshot is necessary, memcpy from remote frozen process' memory 
	// note: there were two ways to do this - spin lock or suspend. suspend allows us to
	// analyze processes even if they were artificially suspended. with a lock, there'd be
	// worry that the target was suspended with the lock taken.
	if (update_snapshot) {
		memcpy(&cache->snapshot, cache->shmem, sizeof(stack_buffer_shared_memory));
		// also need to update our version of the remote uniquing table
		vm_address_t local_uniquing_address = 0ul;
		mach_msg_type_number_t local_uniquing_size = 0;
		mach_vm_size_t desired_size = round_page(sizeof(backtrace_uniquing_table));
		kern_return_t err;
		if ((err = mach_vm_read(descriptors->remote_task, cache->shmem->uniquing_table_address, desired_size, &local_uniquing_address, &local_uniquing_size)) != KERN_SUCCESS 
				|| local_uniquing_size != desired_size) {
			fprintf(stderr, "error while attempting to mach_vm_read remote stack uniquing table (%d): %s\n", err, mach_error_string(err));
		} else {
			// the mach_vm_read was successful, so acquire the uniquing table
			
			// need to re-read the table, so deallocate the current memory
			if (cache->uniquing_table.table) mach_vm_deallocate(mach_task_self(), (mach_vm_address_t)(uintptr_t)(cache->uniquing_table.table), cache->uniquing_table.tableSize);
		
			// the following line gathers the uniquing table structure data, but the actual table memory is invalid since it's a pointer from the
			// remote process. this pointer will be mapped shared in a few lines.
			cache->uniquing_table = *((backtrace_uniquing_table*)local_uniquing_address);
			
			vm_address_t local_table_address = 0ul;
			mach_msg_type_number_t local_table_size = 0;
			
			err = mach_vm_read(descriptors->remote_task, cache->uniquing_table.table_address, cache->uniquing_table.tableSize, &local_table_address, &local_table_size);
			if (err == KERN_SUCCESS) cache->uniquing_table.table = (mach_vm_address_t*)local_table_address;
			else cache->uniquing_table.table = NULL;
			
			mach_vm_deallocate(mach_task_self(), (mach_vm_address_t)local_uniquing_address, (mach_vm_size_t)local_uniquing_size);
		}
	}
	
	// resume
	if (descriptors->remote_task != mach_task_self()) {
		task_resume(descriptors->remote_task);
	}
	
	if (!update_snapshot && delta_indecies == 0) return; // absolutely no updating needed.
	
	FILE *the_index = (descriptors->index_file_stream);
	
	// prepare for the read; target process could be 32 or 64 bit.

	stack_logging_index_event32 *target_32_index = NULL;
	stack_logging_index_event64 *target_64_index = NULL;
	
	// perform the update from the file
	uint32_t i;
	if (delta_indecies) {
		char bufferSpace[4096]; // 4 kb
		target_32_index = (stack_logging_index_event32*)bufferSpace;
		target_64_index = (stack_logging_index_event64*)bufferSpace;
		size_t number_slots = (size_t)(4096/read_size);
	
		size_t read_count = 0;
		if (fseeko(the_index, (off_t)(cache->last_index_file_offset), SEEK_SET)) {
			fprintf(stderr, "error while attempting to cache information from remote stack index file. (update_cache_for_file_streams)\n");
		}
		off_t current_index_position = cache->last_index_file_offset;
		do {
			number_slots = (size_t)MIN(delta_indecies - read_this_update, number_slots);
			read_count = fread(bufferSpace, read_size, number_slots, the_index);
			if (descriptors->task_is_64_bit) {
				for (i = 0; i < read_count; i++) {
					insert_node(cache, STACK_LOGGING_DISGUISE(target_64_index[i].address), (uint64_t)current_index_position);
					read_this_update++;
					current_index_position += read_size;
				}
			} else {
				for (i = 0; i < read_count; i++) {
					insert_node(cache, (mach_vm_address_t)STACK_LOGGING_DISGUISE(target_32_index[i].address), (uint64_t)current_index_position);
					read_this_update++;
					current_index_position += read_size;
				}
			}
		} while (read_count);

		if (read_this_update < delta_indecies) {
			fprintf(stderr, "insufficient data in remote stack index file; expected more records.\n");
		}
		cache->last_index_file_offset += read_this_update * read_size;
	}
	
	if (update_snapshot) {
		target_32_index = (stack_logging_index_event32*)(cache->snapshot.index_buffer);
		target_64_index = (stack_logging_index_event64*)(cache->snapshot.index_buffer);
		
		uint32_t free_snapshot_scan_index = cache->snapshot.next_free_index_buffer_offset / (uint32_t)read_size;
		off_t current_index_position = cache->snapshot.start_index_offset;
		if (descriptors->task_is_64_bit) {
			for (i = last_snapshot_scan_index; i < free_snapshot_scan_index; i++) {
				insert_node(cache, STACK_LOGGING_DISGUISE(target_64_index[i].address), (uint64_t)(current_index_position + (i * read_size))); 
			}
		} else {
			for (i = last_snapshot_scan_index; i < free_snapshot_scan_index; i++) {
				insert_node(cache, (mach_vm_address_t)STACK_LOGGING_DISGUISE(target_32_index[i].address), (uint64_t)(current_index_position + (i * read_size)));
			}
		}
	}
}

static void
destroy_cache_for_file_streams(remote_task_file_streams *descriptors)
{
	if (descriptors->cache->shmem) {
		munmap(descriptors->cache->shmem, sizeof(stack_buffer_shared_memory));
	}
	free(descriptors->cache->table_memory);
	free(descriptors->cache);
	descriptors->cache = NULL;
}

#pragma mark - internal

// In the stack log analysis process, find the stack logging files for target process <pid>
// by scanning the temporary directory for directory entries with names of the form "stack-logs.<pid>."
// If we find such a directory then open the stack logging files in there.
// We might also have been passed the file path if the client first read it from __stack_log_file_path__
// global variable in the target task, as will be needed if the .link cannot be put in /tmp.
static void
open_log_files(pid_t pid, char* file_path, remote_task_file_streams *this_task_streams)
{
	DIR *dp;
	struct dirent *entry;
	char prefix_name[PATH_MAX];
	char pathname[PATH_MAX];

	reap_orphaned_log_files(false);		// reap any left-over log files (for non-existant processes, but not for this analysis process)

	if (file_path != NULL) {
		this_task_streams->index_file_stream = fopen(file_path, "r");
		return;
	}

	if ((dp = opendir(_PATH_TMP)) == NULL) {
		return;
	}

	// It's OK to use snprintf in this routine since it should only be called by the clients
	// of stack logging, and thus calls to malloc are OK.
	snprintf(prefix_name, (size_t)PATH_MAX, "%s%d.", stack_log_file_base_name, pid);	// make sure to use "%s%d." rather than just "%s%d" to match the whole pid
	size_t prefix_length = strlen(prefix_name);

	while ( (entry = readdir(dp)) != NULL ) {
		if ( strncmp( entry->d_name, prefix_name, prefix_length) == 0 ) {
			snprintf(pathname, (size_t)PATH_MAX, "%s%s", _PATH_TMP, entry->d_name);
			char reference_file[PATH_MAX];
			if (log_file_is_reference(pathname, reference_file, (size_t)PATH_MAX)) {
				this_task_streams->index_file_stream = fopen(reference_file, "r");
			} else {
				this_task_streams->index_file_stream = fopen(pathname, "r");
			}
			
			break;
		}
	}
	closedir(dp);
}	

static remote_task_file_streams*
retain_file_streams_for_task(task_t task, char* file_path)
{
	if (task == MACH_PORT_NULL) return NULL;
	
	OSSpinLockLock(&remote_fd_list_lock);
	
	// see if they're already in use
	uint32_t i = 0;
	for (i = 0; i < remote_task_fd_count; i++) {
		if (remote_fds[i].remote_task == task) {
			remote_fds[i].in_use_count++;
			OSSpinLockUnlock(&remote_fd_list_lock);
			return &remote_fds[i];
		}
	}
	
	// open them
	uint32_t failures = 0;
	if (remote_task_fd_count == STACK_LOGGING_MAX_SIMUL_REMOTE_TASKS_INSPECTED) {
		while (remote_fds[next_remote_task_fd].in_use_count > 0) {
			next_remote_task_fd++;
			if (next_remote_task_fd == STACK_LOGGING_MAX_SIMUL_REMOTE_TASKS_INSPECTED) next_remote_task_fd = 0;
			failures++;
			if (failures >= STACK_LOGGING_MAX_SIMUL_REMOTE_TASKS_INSPECTED) {
				OSSpinLockUnlock(&remote_fd_list_lock);
				return NULL;
			}
		}
		fclose(remote_fds[next_remote_task_fd].index_file_stream);
		destroy_cache_for_file_streams(&remote_fds[next_remote_task_fd]);
	}
	
	pid_t pid;
	kern_return_t err = pid_for_task(task, &pid);
	if (err != KERN_SUCCESS) {
		OSSpinLockUnlock(&remote_fd_list_lock);
		return NULL;
	}
	
	remote_task_file_streams *this_task_streams = &remote_fds[next_remote_task_fd];
	
	open_log_files(pid, file_path, this_task_streams);

	// check if opens failed
	if (this_task_streams->index_file_stream == NULL) {
		if (this_task_streams->index_file_stream) fclose(this_task_streams->index_file_stream);
		OSSpinLockUnlock(&remote_fd_list_lock);
		return NULL;
	}
	
	// check if target pid is running 64-bit
	int mib[] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, pid };
	struct kinfo_proc processInfo;
	size_t bufsize = sizeof(processInfo);
	if (sysctl(mib, (unsigned)(sizeof(mib)/sizeof(int)), &processInfo, &bufsize, NULL, (size_t)0) == 0 && bufsize > 0) {
		this_task_streams->task_is_64_bit = processInfo.kp_proc.p_flag & P_LP64;
	} else {
		this_task_streams->task_is_64_bit = 0;
	}
	
	// otherwise set vars and go
	this_task_streams->in_use_count = 1;
	this_task_streams->remote_task = task;
	this_task_streams->remote_pid = pid;
	next_remote_task_fd++;
	if (next_remote_task_fd == STACK_LOGGING_MAX_SIMUL_REMOTE_TASKS_INSPECTED) next_remote_task_fd = 0;
	remote_task_fd_count = MIN(remote_task_fd_count + 1, STACK_LOGGING_MAX_SIMUL_REMOTE_TASKS_INSPECTED);
	
	OSSpinLockUnlock(&remote_fd_list_lock);
	return this_task_streams;
}

static void
release_file_streams_for_task(task_t task)
{
	OSSpinLockLock(&remote_fd_list_lock);
	
	// decrement in-use count
	uint32_t i = 0;
	for (i = 0; i < remote_task_fd_count; i++) {
		if (remote_fds[i].remote_task == task) {
			remote_fds[i].in_use_count--;
			break;
		}
	}
	
	OSSpinLockUnlock(&remote_fd_list_lock);
}

#pragma mark - extern

//
// The following is used by client tools like malloc_history and Instruments to pass along the path
// of the index file as read from the target task's __stack_log_file_path__ variable (set in this file)
// Eventually, at a suitable point, this additional argument should just be added to the other APIs below.
//
kern_return_t
__mach_stack_logging_set_file_path(task_t task, char* file_path)
{
	remote_task_file_streams *remote_fd = retain_file_streams_for_task(task, file_path);
	if (remote_fd == NULL) {
		return KERN_FAILURE;
	}
	return KERN_SUCCESS;
}

kern_return_t
__mach_stack_logging_get_frames(task_t task, mach_vm_address_t address, mach_vm_address_t *stack_frames_buffer, uint32_t max_stack_frames, uint32_t *count)
{
	remote_task_file_streams *remote_fd = retain_file_streams_for_task(task, NULL);
	if (remote_fd == NULL) {
		return KERN_FAILURE;
	}
	
	update_cache_for_file_streams(remote_fd);
	
	uint32_t collisions = 0;
	size_t hash = hash_index(address, remote_fd->cache->cache_node_capacity);
	size_t multiplier = hash_multiplier(remote_fd->cache->cache_node_capacity, remote_fd->cache->collision_allowance);
	uint64_t located_file_position = 0;
	
	bool found = false;	
	do {
		if (remote_fd->cache->table_memory[hash].address == address) { // hit!
			located_file_position = remote_fd->cache->table_memory[hash].index_file_offset;
			found = true;
			break;
		} else if (remote_fd->cache->table_memory[hash].address == 0ull) { // failure!
			break;
		}
		
		collisions++;
		hash = next_hash(hash, multiplier, remote_fd->cache->cache_node_capacity, collisions);
		
	} while (collisions <= remote_fd->cache->collision_allowance);
	
	if (found) {
		// prepare for the read; target process could be 32 or 64 bit.
		stack_logging_index_event32 *target_32_index = NULL;
		stack_logging_index_event64 *target_64_index = NULL;
		
		if (located_file_position >= remote_fd->cache->last_index_file_offset) {
			// must be in shared memory
			if (remote_fd->cache->shmem) {
				if (remote_fd->task_is_64_bit) {
					target_64_index = (stack_logging_index_event64*)(remote_fd->cache->snapshot.index_buffer + (located_file_position - remote_fd->cache->snapshot.start_index_offset));
					located_file_position = STACK_LOGGING_OFFSET(target_64_index->offset_and_flags);
				} else {
					target_32_index = (stack_logging_index_event32*)(remote_fd->cache->snapshot.index_buffer + (located_file_position - remote_fd->cache->snapshot.start_index_offset));
					located_file_position = STACK_LOGGING_OFFSET(target_32_index->offset_and_flags);				
				}
			} else {
				found = false;
			}
		
		} else {
			// it's written to disk
			char bufferSpace[128];
			
			size_t read_size = (remote_fd->task_is_64_bit ? sizeof(stack_logging_index_event64) : sizeof(stack_logging_index_event32));
			fseeko(remote_fd->index_file_stream, (off_t)located_file_position, SEEK_SET);
			size_t read_count = fread(bufferSpace, read_size, (size_t)1, remote_fd->index_file_stream);
			if (read_count) {
				if (remote_fd->task_is_64_bit) {
					target_64_index = (stack_logging_index_event64*)bufferSpace;
					located_file_position = STACK_LOGGING_OFFSET(target_64_index->offset_and_flags);
				} else {
					target_32_index = (stack_logging_index_event32*)bufferSpace;
					located_file_position = STACK_LOGGING_OFFSET(target_32_index->offset_and_flags);
				}
			} else {
				found = false;
			}
		}
	}
	
	release_file_streams_for_task(task);
	
	if (!found) {
		return KERN_FAILURE;
	}
	
	return __mach_stack_logging_frames_for_uniqued_stack(task, located_file_position, stack_frames_buffer, max_stack_frames, count);
}


kern_return_t
__mach_stack_logging_enumerate_records(task_t task, mach_vm_address_t address, void enumerator(mach_stack_logging_record_t, void *), void *context)
{
	remote_task_file_streams *remote_fd = retain_file_streams_for_task(task, NULL);
	if (remote_fd == NULL) {
		return KERN_FAILURE;
	}
	
	bool reading_all_addresses = (address == 0 ? true : false);
	mach_stack_logging_record_t pass_record;
	kern_return_t err = KERN_SUCCESS;
	
	// update (read index file once and only once)
	update_cache_for_file_streams(remote_fd);

	FILE *the_index = (remote_fd->index_file_stream);
	
	// prepare for the read; target process could be 32 or 64 bit.
	char bufferSpace[2048]; // 2 kb
	stack_logging_index_event32 *target_32_index = (stack_logging_index_event32*)bufferSpace;
	stack_logging_index_event64 *target_64_index = (stack_logging_index_event64*)bufferSpace;
	uint32_t target_addr_32 = (uint32_t)STACK_LOGGING_DISGUISE((uint32_t)address);
	uint64_t target_addr_64 = STACK_LOGGING_DISGUISE((uint64_t)address);
	size_t read_size = (remote_fd->task_is_64_bit ? sizeof(stack_logging_index_event64) : sizeof(stack_logging_index_event32));
	size_t number_slots = (size_t)(2048/read_size);
	uint64_t total_slots = remote_fd->cache->last_index_file_offset / read_size;
	
	// perform the search
	size_t read_count = 0;
	int64_t current_file_offset = 0;
	uint32_t i;
	do {
		// at this point, we need to read index events; read them from the file until it's necessary to grab them from the shared memory snapshot
		// and crop file reading to the point where we last scanned
		number_slots = (size_t)MIN(number_slots, total_slots);
		
		// if out of file to read (as of the time we entered this function), try to use shared memory snapshot
		if (number_slots == 0) {
			if (remote_fd->cache->shmem && remote_fd->cache->snapshot.start_index_offset + remote_fd->cache->snapshot.next_free_index_buffer_offset > (uint64_t)current_file_offset) {
				// use shared memory
				target_32_index = (stack_logging_index_event32*)remote_fd->cache->snapshot.index_buffer;
				target_64_index = (stack_logging_index_event64*)remote_fd->cache->snapshot.index_buffer;
				read_count = (uint32_t)(remote_fd->cache->snapshot.start_index_offset + remote_fd->cache->snapshot.next_free_index_buffer_offset - current_file_offset) / read_size;
				current_file_offset += read_count * read_size;
			} else {
				break;
			}
		} else {
			// get and save index (enumerator could modify)
			fseeko(the_index, current_file_offset, SEEK_SET);
			read_count = fread(bufferSpace, read_size, number_slots, the_index);
			current_file_offset = ftello(the_index);
			total_slots -= read_count;
		}
		
		if (remote_fd->task_is_64_bit) {
			for (i = 0; i < read_count; i++) {
				if (reading_all_addresses || target_64_index[i].address == target_addr_64) {
					pass_record.address = STACK_LOGGING_DISGUISE(target_64_index[i].address);
					pass_record.argument = target_64_index[i].argument;
					pass_record.stack_identifier = STACK_LOGGING_OFFSET(target_64_index[i].offset_and_flags);
					pass_record.type_flags = STACK_LOGGING_FLAGS(target_64_index[i].offset_and_flags);
					enumerator(pass_record, context);
				}
			}
		} else {
			for (i = 0; i < read_count; i++) {
				if (reading_all_addresses || target_32_index[i].address == target_addr_32) {
					pass_record.address = STACK_LOGGING_DISGUISE(target_32_index[i].address);
					pass_record.argument = target_32_index[i].argument;
					pass_record.stack_identifier = STACK_LOGGING_OFFSET(target_32_index[i].offset_and_flags);
					pass_record.type_flags = STACK_LOGGING_FLAGS(target_32_index[i].offset_and_flags);
					enumerator(pass_record, context);
				}
			}
		}
	} while (read_count);
	
	release_file_streams_for_task(task);
	return err;
}


kern_return_t
__mach_stack_logging_frames_for_uniqued_stack(task_t task, uint64_t stack_identifier, mach_vm_address_t *stack_frames_buffer, uint32_t max_stack_frames, uint32_t *count)
{
	remote_task_file_streams *remote_fd = retain_file_streams_for_task(task, NULL);
	if (remote_fd == NULL) return KERN_FAILURE;

	__unwind_stack_from_table_index(&remote_fd->cache->uniquing_table, stack_identifier, stack_frames_buffer, count, max_stack_frames);	
	
	release_file_streams_for_task(task);
	
	if (*count) return KERN_SUCCESS;
	else return KERN_FAILURE;
}


#ifdef TEST_DISK_STACK_LOGGING

// cc -o stack_logging_disk stack_logging_disk.c -DTEST_DISK_STACK_LOGGING

#include <sys/wait.h>

int
main()
{
  int status;
  int i;
  size_t total_globals = 0ul;

  fprintf(stderr, "master test process is %d\n", getpid());
  fprintf(stderr, "sizeof pre_write_buffers: %lu\n", sizeof(pre_write_buffers)); total_globals += sizeof(pre_write_buffers);
  fprintf(stderr, "sizeof stack_buffer: %lu\n", sizeof(stack_buffer)); total_globals += sizeof(stack_buffer);
  fprintf(stderr, "sizeof last_logged_malloc_address: %lu\n", sizeof(last_logged_malloc_address)); total_globals += sizeof(last_logged_malloc_address);
  fprintf(stderr, "sizeof stack_log_file_base_name: %lu\n", sizeof(stack_log_file_base_name)); total_globals += sizeof(stack_log_file_base_name);
  fprintf(stderr, "sizeof stack_log_file_suffix: %lu\n", sizeof(stack_log_file_suffix)); total_globals += sizeof(stack_log_file_suffix);
  fprintf(stderr, "sizeof stack_log_link_suffix: %lu\n", sizeof(stack_log_link_suffix)); total_globals += sizeof(stack_log_link_suffix);
  fprintf(stderr, "sizeof stack_log_location: %lu\n", (size_t)PATH_MAX); total_globals += (size_t)PATH_MAX;
  fprintf(stderr, "sizeof stack_log_reference_file: %lu\n", (size_t)PATH_MAX); total_globals += (size_t)PATH_MAX;
  fprintf(stderr, "sizeof __stack_log_file_path__ (index_file_path): %lu\n", (size_t)PATH_MAX); total_globals += (size_t)PATH_MAX;
  fprintf(stderr, "sizeof index_file_descriptor: %lu\n", sizeof(index_file_descriptor)); total_globals += sizeof(index_file_descriptor);
  fprintf(stderr, "sizeof remote_fds: %lu\n", sizeof(remote_fds)); total_globals += sizeof(remote_fds);
  fprintf(stderr, "sizeof next_remote_task_fd: %lu\n", sizeof(next_remote_task_fd)); total_globals += sizeof(next_remote_task_fd);
  fprintf(stderr, "sizeof remote_task_fd_count: %lu\n", sizeof(remote_task_fd_count)); total_globals += sizeof(remote_task_fd_count);
  fprintf(stderr, "sizeof remote_fd_list_lock: %lu\n", sizeof(remote_fd_list_lock)); total_globals += sizeof(remote_fd_list_lock);
  fprintf(stderr, "sizeof logging_use_compaction: %lu\n", sizeof(logging_use_compaction)); total_globals += sizeof(logging_use_compaction);

  fprintf(stderr, "size of all global data: %lu\n", total_globals);

  create_log_file();

  // create a few child processes and exit them cleanly so their logs should get cleaned up
  fprintf(stderr, "\ncreating child processes and exiting cleanly\n");
  for (i = 0; i < 3; i++) {
    if (fork() == 0) {
      fprintf(stderr, "\nin child processes %d\n", getpid());
      create_log_file();
      fprintf(stderr, "exiting child processes %d\n", getpid());
      exit(1);
    }
    wait(&status);
  }

  // create a few child processes and abruptly _exit them, leaving their logs around
  fprintf(stderr, "\ncreating child processes and exiting abruptly, leaving logs around\n");
  for (i = 0; i < 3; i++) {
    if (fork() == 0) {
      fprintf(stderr, "\nin child processes %d\n", getpid());
      create_log_file();
      fprintf(stderr, "exiting child processes %d\n", getpid());
      _exit(1);
    }
    wait(&status);
  }

  // this should reap any remaining logs
  fprintf(stderr, "\nexiting master test process %d\n", getpid());
  delete_log_files();
  return 0;
}

#endif
