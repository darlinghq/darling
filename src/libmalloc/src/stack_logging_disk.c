/*
 * Copyright (c) 2007-2013 Apple Inc. All rights reserved.
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

#include "internal.h"

#pragma mark -
#pragma mark Defines

#if TARGET_OS_EMBEDDED || TARGET_IPHONE_SIMULATOR
// _malloc_printf(ASL_LEVEL_INFO...) on iOS doesn't show up in the Xcode Console log of the device,
// but ASL_LEVEL_NOTICE does.  So raising the log level is helpful.
#undef ASL_LEVEL_INFO
#define ASL_LEVEL_INFO ASL_LEVEL_NOTICE
#endif

#ifdef TEST_DISK_STACK_LOGGING
#define _malloc_printf fprintf
#undef ASL_LEVEL_INFO
#define ASL_LEVEL_INFO stderr
#endif

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
#define INITIAL_MAX_COLLIDE 19
#define DEFAULT_UNIQUING_PAGE_SIZE 256

#pragma mark -
#pragma mark Macros

#define STACK_LOGGING_FLAGS_SHIFT 56
#define STACK_LOGGING_USER_TAG_SHIFT 24
#define STACK_LOGGING_FLAGS(longlongvar) (uint32_t)((uint64_t)(longlongvar) >> STACK_LOGGING_FLAGS_SHIFT)
#define STACK_LOGGING_FLAGS_AND_USER_TAG(longlongvar) \
	(uint32_t)(STACK_LOGGING_FLAGS(longlongvar) | (((uint64_t)(longlongvar)&0x00FF000000000000ull) >> STACK_LOGGING_USER_TAG_SHIFT))

#define STACK_LOGGING_OFFSET_MASK 0x0000FFFFFFFFFFFFull
#define STACK_LOGGING_OFFSET(longlongvar) ((longlongvar)&STACK_LOGGING_OFFSET_MASK)

#define STACK_LOGGING_OFFSET_AND_FLAGS(longlongvar, type_flags)                                                    \
	(((uint64_t)(longlongvar)&STACK_LOGGING_OFFSET_MASK) | ((uint64_t)(type_flags) << STACK_LOGGING_FLAGS_SHIFT) | \
			(((uint64_t)(type_flags)&0xFF000000ull) << STACK_LOGGING_USER_TAG_SHIFT))

#pragma mark -
#pragma mark Types

typedef struct {
	uintptr_t argument;
	uintptr_t address;
	uint64_t offset_and_flags; // top 8 bits are actually the flags!
} stack_logging_index_event;

typedef struct {
	uint32_t argument;
	uint32_t address;
	uint64_t offset_and_flags; // top 8 bits are actually the flags!
} stack_logging_index_event32;

typedef struct {
	uint64_t argument;
	uint64_t address;
	uint64_t offset_and_flags; // top 8 bits are actually the flags!
} stack_logging_index_event64;

// backtrace uniquing table chunks used in client-side stack log reading code,
// in case we can't read the whole table in one mach_vm_read() call.
typedef struct table_chunk_header {
	uint64_t num_nodes_in_chunk;
	uint64_t table_chunk_size;
	mach_vm_address_t *table_chunk;
	struct table_chunk_header *next_table_chunk_header;
} table_chunk_header_t;

#pragma pack(push, 4)
typedef struct backtrace_uniquing_table {
	uint64_t numPages; // number of pages of the table
	uint64_t numNodes;
	uint64_t tableSize;
	uint64_t untouchableNodes;
	mach_vm_address_t table_address;
	int32_t max_collide;
// 'table_address' is just an always 64-bit version of the pointer-sized 'table' field to remotely read;
// it's important that the offset of 'table_address' in the struct does not change between 32 and 64-bit.
#if BACKTRACE_UNIQUING_DEBUG
	uint64_t nodesFull;
	uint64_t backtracesContained;
#endif
	union {
		mach_vm_address_t *table;					 // in "target" process;  allocated using vm_allocate()
		table_chunk_header_t *first_table_chunk_hdr; // in analysis process
	} u;
	uint64_t max_table_size;
	bool in_client_process : 1;
	bool nodes_use_refcount : 1;
	unsigned refcount;
} backtrace_uniquing_table;
#pragma pack(pop)

// for storing/looking up allocations that haven't yet be written to disk; consistent size across 32/64-bit processes.
// It's important that these fields don't change alignment due to the architecture because they may be accessed from an
// analyzing process with a different arch - hence the pragmas.
#pragma pack(push, 4)
typedef struct {
	uint64_t start_index_offset;
	uint32_t next_free_index_buffer_offset;
	char index_buffer[STACK_LOGGING_BLOCK_WRITING_SIZE];
	backtrace_uniquing_table *uniquing_table;
} stack_buffer_shared_memory;
#pragma pack(pop)

// target process address -> record table (for __mach_stack_logging_get_frames)
typedef struct {
	uint64_t address;
	uint64_t index_file_offset;
} remote_index_node;

// for caching index information client-side:
typedef struct {
	size_t cache_size;
	size_t cache_node_capacity;
	uint32_t collision_allowance;
	remote_index_node *table_memory;	 // this can be malloced; it's on the client side.
	stack_buffer_shared_memory *shmem;   // shared memory
	stack_buffer_shared_memory snapshot; // memory snapshot of the remote process' shared memory
	uint32_t last_pre_written_index_size;
	uint64_t last_index_file_offset;
	backtrace_uniquing_table uniquing_table_snapshot; // snapshot of the remote process' uniquing table
	boolean_t lite_mode;
} remote_index_cache;

// for reading stack history information from remote processes:
typedef struct {
	task_t remote_task;
	pid_t remote_pid;
	int32_t task_is_64_bit;
	boolean_t task_uses_lite_mode;
	int32_t in_use_count;
	FILE *index_file_stream;
	uint64_t remote_stack_buffer_shared_memory_address;
	remote_index_cache *cache;
} remote_task_file_streams;

typedef mach_vm_address_t slot_address;
typedef uint64_t slot_parent;
typedef uint64_t slot_refcount;
typedef uint64_t table_slot_index;

#pragma pack(push,16)
typedef struct {
	union {
		struct {
			uint64_t slot0;
			uint64_t slot1;
		} slots;
		
		struct {
			slot_address address:48;
			slot_parent parent:32;
			slot_refcount refcount:48;
		} refcount_slot;
		
		struct {
			slot_address address:64;
			slot_parent parent:64;
		} normal_slot;
	};
	
} table_slot_t;
#pragma pop

_Static_assert(sizeof(table_slot_t) == 16, "table_slot_t must be 128 bits");

#pragma mark -
#pragma mark Constants/Globals

int stack_logging_enable_logging = 0;
int stack_logging_dontcompact = 0;
int stack_logging_finished_init = 0;
int stack_logging_postponed = 0;
int stack_logging_mode = stack_logging_mode_none;

#define MAX_PARENT_NORMAL
#define MAX_PARENT_REFCOUNT

static const slot_parent slot_no_parent_normal =   0xFFFFFFFFFFFFFFFF;	    // 64 bits
static const slot_parent slot_no_parent_refcount = 0xFFFFFFFF;				// 32 bits

static _malloc_lock_s stack_logging_lock = _MALLOC_LOCK_INIT;

// single-thread access variables
static stack_buffer_shared_memory *pre_write_buffers;
static vm_address_t *stack_buffer;
static uintptr_t last_logged_malloc_address = 0;

// Constants to define part of stack logging file path names.
// File names are of the form stack-logs.<pid>.<address>.<progname>.XXXXXX.index
// where <address> is the address of the pre_write_buffers VM region in the target
// process that will need to be mapped into analysis tool processes.
static const char *stack_log_file_base_name = "stack-logs.";
static const char *stack_log_file_suffix = ".index";

static FILE *open_log_file_at_path(char *pathname, remote_task_file_streams *streams);

char *__stack_log_file_path__ = NULL;
static int index_file_descriptor = -1;

// for accessing remote log files
static remote_task_file_streams remote_fds[STACK_LOGGING_MAX_SIMUL_REMOTE_TASKS_INSPECTED];
static uint32_t next_remote_task_fd = 0;
static uint32_t remote_task_fd_count = 0;
static _malloc_lock_s remote_fd_list_lock = _MALLOC_LOCK_INIT;

uint64_t __mach_stack_logging_shared_memory_address = 0;

// activation variables
static int logging_use_compaction = 1; // set this to zero to always disable compaction.

// We set malloc_logger to NULL to disable logging, if we encounter errors
// during file writing
typedef void(malloc_logger_t)(uint32_t type,
		uintptr_t arg1,
		uintptr_t arg2,
		uintptr_t arg3,
		uintptr_t result,
		uint32_t num_hot_frames_to_skip);
extern malloc_logger_t *malloc_logger;

extern malloc_logger_t *__syscall_logger; // use this to set up syscall logging (e.g., vm_allocate, vm_deallocate, mmap, munmap)

#pragma mark -
#pragma mark In-Memory Backtrace Uniquing

static __attribute__((always_inline)) inline void *
sld_allocate_pages(uint64_t memSize)
{
	mach_vm_address_t allocatedMem = 0ull;
	if (mach_vm_allocate(mach_task_self(), &allocatedMem, memSize, VM_FLAGS_ANYWHERE | VM_MAKE_TAG(VM_MEMORY_ANALYSIS_TOOL)) !=
			KERN_SUCCESS) {
		malloc_printf("allocate_pages(): virtual memory exhausted!\n");
	}
	return (void *)(uintptr_t)allocatedMem;
}

static __attribute__((always_inline)) inline int
sld_deallocate_pages(void *memPointer, uint64_t memSize)
{
	return mach_vm_deallocate(mach_task_self(), (mach_vm_address_t)(uintptr_t)memPointer, memSize);
}

static const uint64_t max_table_size_lite = UINT32_MAX;
static const uint64_t max_table_size_normal = UINT64_MAX;

static backtrace_uniquing_table *
__create_uniquing_table(boolean_t lite_mode)
{
	backtrace_uniquing_table *uniquing_table =
			(backtrace_uniquing_table *)sld_allocate_pages((uint64_t)round_page(sizeof(backtrace_uniquing_table)));
	if (!uniquing_table) {
		return NULL;
	}
	bzero(uniquing_table, sizeof(backtrace_uniquing_table));
	uniquing_table->numPages = DEFAULT_UNIQUING_PAGE_SIZE;
	uniquing_table->tableSize = uniquing_table->numPages * vm_page_size;
	uniquing_table->numNodes = ((uniquing_table->tableSize / (sizeof(mach_vm_address_t) * 2)) >> 1) << 1; // make sure it's even.
	uniquing_table->u.table = (mach_vm_address_t *)(uintptr_t)sld_allocate_pages(uniquing_table->tableSize);
	uniquing_table->table_address = (uintptr_t)uniquing_table->u.table;
	uniquing_table->max_collide = INITIAL_MAX_COLLIDE;
	uniquing_table->untouchableNodes = 0;
	uniquing_table->max_table_size = (lite_mode) ? max_table_size_lite : max_table_size_normal;
	uniquing_table->nodes_use_refcount = lite_mode;
	uniquing_table->in_client_process = 0;

#if BACKTRACE_UNIQUING_DEBUG
	malloc_printf("create_uniquing_table(): creating. size: %lldKB == %lldMB, numnodes: %lld (%lld untouchable)\n",
			uniquing_table->tableSize >> 10, uniquing_table->tableSize >> 20, uniquing_table->numNodes,
			uniquing_table->untouchableNodes);
	malloc_printf("create_uniquing_table(): table: %p; end: %p\n", uniquing_table->u.table,
			(void *)((uintptr_t)uniquing_table->u.table + (uintptr_t)uniquing_table->tableSize));
#endif
	return uniquing_table;
}

static void
__destroy_uniquing_table(backtrace_uniquing_table *table)
{
	assert(!table->in_client_process);
	sld_deallocate_pages(table->u.table, table->tableSize);
	sld_deallocate_pages(table, sizeof(backtrace_uniquing_table));
}

static boolean_t
__expand_uniquing_table(backtrace_uniquing_table *uniquing_table)
{
	assert(!uniquing_table->in_client_process);
	mach_vm_address_t *oldTable = uniquing_table->u.table;
	uint64_t oldsize = uniquing_table->tableSize;
	uint64_t oldnumnodes = uniquing_table->numNodes;

	uint64_t newsize = (uniquing_table->numPages << EXPAND_FACTOR) * vm_page_size;
	
	if (newsize > uniquing_table->max_table_size) {
		malloc_printf("no more space in uniquing table\n");
		return false;
	}
	
	uniquing_table->numPages = uniquing_table->numPages << EXPAND_FACTOR;
	uniquing_table->tableSize = uniquing_table->numPages * vm_page_size;
	uniquing_table->numNodes = ((uniquing_table->tableSize / (sizeof(mach_vm_address_t) * 2)) >> 1) << 1; // make sure it's even.
	mach_vm_address_t *newTable = (mach_vm_address_t *)(uintptr_t)sld_allocate_pages(uniquing_table->tableSize);

	uniquing_table->u.table = newTable;
	uniquing_table->table_address = (uintptr_t)uniquing_table->u.table;
	uniquing_table->max_collide = uniquing_table->max_collide + COLLISION_GROWTH_RATE;

	if (mach_vm_copy(mach_task_self(), (mach_vm_address_t)(uintptr_t)oldTable, oldsize, (mach_vm_address_t)(uintptr_t)newTable) !=
			KERN_SUCCESS) {
		malloc_printf("expandUniquingTable(): VMCopyFailed\n");
	}
	uniquing_table->untouchableNodes = oldnumnodes;

#if BACKTRACE_UNIQUING_DEBUG
	malloc_printf(
			"expandUniquingTable(): expanded from nodes full: %lld of: %lld (~%2d%%); to nodes: %lld (inactive = %lld); unique "
			"bts: %lld\n",
			uniquing_table->nodesFull, oldnumnodes, (int)(((uniquing_table->nodesFull * 100.0) / (double)oldnumnodes) + 0.5),
			uniquing_table->numNodes, uniquing_table->untouchableNodes, uniquing_table->backtracesContained);
	malloc_printf("expandUniquingTable(): allocate: %p; end: %p\n", newTable,
			(void *)((uintptr_t)newTable + (uintptr_t)(uniquing_table->tableSize)));
	malloc_printf("expandUniquingTable(): deallocate: %p; end: %p\n", oldTable, (void *)((uintptr_t)oldTable + (uintptr_t)oldsize));
	malloc_printf("expandUniquingTable(): new size = %llu\n", newsize);
#endif

	if (sld_deallocate_pages(oldTable, oldsize) != KERN_SUCCESS) {
		malloc_printf("expandUniquingTable(): mach_vm_deallocate failed. [%p]\n", uniquing_table->u.table);
	}
	
	return true;
}

static void
add_new_slot(table_slot_t *table_slot, mach_vm_address_t address, table_slot_index parent, bool use_refcount, size_t ptr_size)
{
	assert(use_refcount == (ptr_size > 0));
	
	table_slot_t new_slot;
	
	if (use_refcount) {
		new_slot.refcount_slot.address = address;
		new_slot.refcount_slot.parent = parent;
		new_slot.refcount_slot.refcount = ptr_size;
	} else {
		new_slot.normal_slot.address = address;
		new_slot.normal_slot.parent = parent;
	}
	
	*table_slot = new_slot;
}

static void
increment_slot_refcount(table_slot_t *table_slot, size_t ptr_size)
{
	table_slot->refcount_slot.refcount += ptr_size;
}

static int
enter_frames_in_table(backtrace_uniquing_table *uniquing_table, uint64_t *foundIndex, mach_vm_address_t *frames, int32_t count, size_t ptr_size)
{
	assert(!uniquing_table->in_client_process);
	boolean_t use_refcount = (ptr_size > 0);
	
	// The hash values need to be the same size as the addresses (because we use the value -1), for clarity, define a new type
	typedef mach_vm_address_t hash_index_t;
	
	hash_index_t uParent = use_refcount ? slot_no_parent_refcount : slot_no_parent_normal;
	hash_index_t modulus = (uniquing_table->numNodes-uniquing_table->untouchableNodes-1);
	
	int32_t lcopy = count;
	int32_t returnVal = 1;
	hash_index_t hash_multiplier = ((uniquing_table->numNodes - uniquing_table->untouchableNodes)/(uniquing_table->max_collide*2+1));
	
	while (--lcopy >= 0) {
		mach_vm_address_t thisPC = frames[lcopy];
		hash_index_t hash = uniquing_table->untouchableNodes + (((uParent << 4) ^ (thisPC >> 2)) % modulus);
		int32_t collisions = uniquing_table->max_collide;

		while (collisions--) {
			table_slot_t *table_slot = (table_slot_t *) (uniquing_table->u.table + (hash * 2));
			
			if (table_slot->slots.slot0 == 0 && table_slot->slots.slot1 == 0) {
				add_new_slot(table_slot, thisPC, uParent, uniquing_table->nodes_use_refcount, ptr_size);
				uParent = hash;
				break;
			}
			
			slot_address address = use_refcount ? table_slot->refcount_slot.address : table_slot->normal_slot.address;
			slot_parent parent = use_refcount ? table_slot->refcount_slot.parent : table_slot->normal_slot.parent;
			
			if (address == thisPC && parent == uParent) {
				uParent = hash;
				
				if (use_refcount)
					increment_slot_refcount(table_slot, ptr_size);
				
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

	if (returnVal) {
		*foundIndex = uParent;
	}

	return returnVal;
}

#pragma mark -
#pragma mark Disk Stack Logging

// pre-declarations
static void delete_log_files(void);
static int delete_logging_file(char *log_location);
static bool getenv_from_process(pid_t pid, char *env_var_name, char *env_var_value_buf, size_t max_path_len);

#define BASE10 10
#define BASE16 16

static void
append_int(char *filename, uint64_t inputValue, unsigned base, size_t maxLength)
{
	const char *digits = "0123456789abcdef";
	if (base > strlen(digits)) {
		return; // sanity check
	}
	size_t len = strlen(filename);

	uint32_t count = 0;
	uint64_t value = inputValue;
	while (value > 0) {
		value /= base;
		count++;
	}

	if (len + count >= maxLength) {
		return; // don't modify the string if it would violate maxLength
	}
	filename[len + count] = '\0';

	value = inputValue;
	uint32_t i;
	for (i = 0; i < count; i++) {
		filename[len + count - 1 - i] = digits[value % base];
		value /= base;
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
 * Check various logging directory options, in order of preference:
 *
 *      value of MallocStackLoggingDirectory env var if user has set it.  Typically
 *			used on Mac OS X to write to a non-root file system with more free space.
 *
 *      _PATH_TMP - /tmp usually writable for desktop apps and internal iOS apps
 *
 *      value of TMPDIR env var - for sandboxed apps that can't write into /tmp
 *
 *      confstr(_CS_DARWIN_USER_TEMP_DIR, ...) - should be same as TMPDIR if that is set,
 *          but will create it safely if it doesn't yet exist.  (See <rdar://problem/4706096>)
 *
 * Allocating and releasing target buffer is the caller's responsibility.
 */
static bool
get_writeable_logging_directory(char *target)
{
	if (!target) {
		return false;
	}

	char *evn_log_directory = getenv("MallocStackLoggingDirectory");
	if (evn_log_directory) {
		if (-1 != access(evn_log_directory, W_OK)) {
			strlcpy(target, evn_log_directory, (size_t)PATH_MAX);
			return true;
		} else {
			_malloc_printf(ASL_LEVEL_INFO, "MallocStackLoggingDirectory env var set to unwritable path '%s'\n", evn_log_directory);
		}
	}

	if (-1 != access(_PATH_TMP, W_OK)) {
		strlcpy(target, _PATH_TMP, (size_t)PATH_MAX);
		return true;
	}

	evn_log_directory = getenv("TMPDIR");
	if (evn_log_directory && (-1 != access(evn_log_directory, W_OK))) {
		strlcpy(target, evn_log_directory, (size_t)PATH_MAX);
		return true;
	}

	if (stack_logging_finished_init) {
		size_t n = confstr(_CS_DARWIN_USER_TEMP_DIR, target, (size_t)PATH_MAX);
		if ((n > 0) && (n < PATH_MAX)) {
			return true;
		}
	} else {
		/* <rdar://problem/11128080> Can't call confstr during init, so postpone
		 * logging till after */
		postpone_stack_logging();
	}
	*target = '\0';
	return false;
}

// Stolen from libc.  Ugly hack because arc4random uses malloc so we can't call Libc's mkstemps.
int getentropy(void *, size_t);
static int
my_mkstemps(char *path, size_t slen)
{
	char *start, *trv, *suffp, *carryp;
	char *pad;
	char carrybuf[MAXPATHLEN];
	int fd;

	for (trv = path; *trv != '\0'; ++trv) {
		;
	}
	trv -= slen;
	suffp = trv;
	--trv;

	/* Fill space with random characters */
	uint8_t randbuf[8];
	unsigned int randbuf_offset = 0;
	getentropy(randbuf, sizeof(randbuf));
	static const char padchar[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	while (trv >= path && *trv == 'X') {
		*trv-- = padchar[randbuf[randbuf_offset++ % sizeof(randbuf)] % sizeof(padchar)];
	}
	start = trv + 1;

	/* save first combination of random characters */
	memcpy(carrybuf, start, suffp - start);

	for (;;) {
		if ((fd = open(path, O_CREAT | O_EXCL | O_RDWR, 0600)) >= 0) {
			return fd;
		}
		if (errno != EEXIST) {
			return -1;
		}

		/* If we have a collision, cycle through the space of filenames */
		for (trv = start, carryp = carrybuf;;) {
			/* have we tried all possible permutations? */
			if (trv == suffp) {
				return -1; /* yes - exit with EEXIST */
			}
			pad = strchr(padchar, *trv);
			if (pad == NULL) {
				/* this should never happen */
				errno = EIO;
				return -1;
			}
			/* increment character */
			*trv = (*++pad == '\0') ? padchar[0] : *pad;
			/* carry to next position? */
			if (*trv == *carryp) {
				/* increment position and loop */
				++trv;
				++carryp;
			} else {
				/* try with new name */
				break;
			}
		}
	}
}

// If successful, returns path to log file that was created, otherwise NULL.
static char *
create_log_file()
{
	pid_t pid = getpid();
	const char *progname = getprogname();
	char *created_log_location = NULL;

	if (__stack_log_file_path__ == NULL) {
		// On first use, allocate space directly from the OS without using malloc
		__stack_log_file_path__ = sld_allocate_pages((uint64_t)round_page(PATH_MAX));
		if (__stack_log_file_path__ == NULL) {
			_malloc_printf(ASL_LEVEL_INFO, "unable to allocate memory for stack log file path\n");
			return NULL;
		}
	}

	if (!get_writeable_logging_directory(__stack_log_file_path__)) {
		if (!stack_logging_postponed) {
			_malloc_printf(ASL_LEVEL_INFO, "No writeable tmp dir\n");
		}
		return NULL;
	}

	// Add the '/' only if it's not already there.  Having multiple '/' characters works
	// but is unsightly when we print these stack log file names out.
	size_t stack_log_len = strlen(__stack_log_file_path__);
	if (__stack_log_file_path__[stack_log_len - 1] != '/') {
		// use strlcat to null-terminate for the next strlcat call, and to check buffer size
		strlcat(__stack_log_file_path__ + stack_log_len, "/", (size_t)PATH_MAX);
	}

	// Append the file name to __stack_log_file_path__ but don't use snprintf since
	// it can cause malloc() calls.
	//
	// The file name is of the form "stack-logs.<pid>.<address>.<progname>.XXXXXX.index"
	// where <address> is the address of the pre_write_buffers VM region in the target
	// process that will need to be mapped into analysis tool processes. We used to just
	// use a shared memory segment for that, but sandbox'ed target apps may not be able
	// to create shared memory segments so including the address of the VM region in the
	// file name is a simple way to communicate the address to analysis tools so the
	// stack log reading code can map in the region with mach_vm_remap().

	strlcat(__stack_log_file_path__, stack_log_file_base_name, (size_t)PATH_MAX);
	append_int(__stack_log_file_path__, pid, BASE10, (size_t)PATH_MAX);
	strlcat(__stack_log_file_path__, ".", (size_t)PATH_MAX);
	append_int(__stack_log_file_path__, (uint64_t)pre_write_buffers, BASE16, (size_t)PATH_MAX);
	if (progname && progname[0] != '\0') {
		strlcat(__stack_log_file_path__, ".", (size_t)PATH_MAX);
		strlcat(__stack_log_file_path__, progname, (size_t)PATH_MAX);
	}
	strlcat(__stack_log_file_path__, ".XXXXXX", (size_t)PATH_MAX);
	strlcat(__stack_log_file_path__, stack_log_file_suffix, (size_t)PATH_MAX);
	
	// Securely create the log file.
	if ((index_file_descriptor = my_mkstemps(__stack_log_file_path__, (int)strlen(stack_log_file_suffix))) != -1) {
		_malloc_printf(ASL_LEVEL_INFO, "stack logs being written into %s\n", __stack_log_file_path__);
		created_log_location = __stack_log_file_path__;
	} else {
		_malloc_printf(ASL_LEVEL_INFO, "unable to create stack logs at %s\n", __stack_log_file_path__);
		__stack_log_file_path__[0] = '\0';
		created_log_location = NULL;
	}

	return created_log_location;
}

// This function may be called from either the target process when exiting, or from either the the target process or
// a stack log analysis process, when reaping orphaned stack log files.
// Returns -1 if the files exist and they couldn't be removed, returns 0 otherwise.
static int
delete_logging_file(char *log_location)
{
	if (log_location == NULL || log_location[0] == '\0') {
		return 0;
	}

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
}

static bool
is_process_running(pid_t pid)
{
	struct kinfo_proc kpt[1];
	size_t size = sizeof(struct kinfo_proc);
	int mib[] = {CTL_KERN, KERN_PROC, KERN_PROC_PID, pid};

	sysctl(mib, 4, kpt, &size, NULL, (size_t)0); // size is either 1 or 0 entries when we ask for a single pid

	return (size == sizeof(struct kinfo_proc));
}

// Stack log files can be quite large and aren't useful after the process that created them no longer exists because
// the stack backtrace uniquing tree was only kept in the process memory, not on disk.  Normally the log files
// should get removed when the process exits, by the delete_log_files() atexit function.  However, there are
// several situations in which that atexit function doesn't get called so the log files remain:
//		- if the process crashes or is force-killed
//		- if the app supported sudden termination, and was terminated through that
//		- if a process such as a shell execs another binary to transform the pid into a different process;
//			the new process will get a new log file but the old one would still be there.
//
// So, reap any stack log files for processes that no longer exist, or for the current process if we find a file
// other than __stack_log_file_path__
//
// This function looks for log files with prefix name "stack-logs.<pid>." underneath <directory>.
// <remaining_path_format> specifies a simple pattern of where stack logs can be down inside <directory>.
// The pattern is essentially a relative path, where a level that start with '<' matches any name, otherwise
// it has to be an exact name match.  See the calling function for examples.
static void
reap_orphaned_log_files_in_hierarchy(char *directory, char *remaining_path_format, pid_t target_pid, remote_task_file_streams *streams)
{
	DIR *dp;
	struct dirent *entry;

	// Ensure that we can access this directory - permissions or sandbox'ing might prevent it.
	if (access(directory, R_OK | W_OK | X_OK) == -1 || (dp = opendir(directory)) == NULL) {
		//_malloc_printf(ASL_LEVEL_INFO, "reaping: no access to %s\n", directory);
		return;
	}

	char pathname[PATH_MAX];
	strlcpy(pathname, directory, (size_t)PATH_MAX);
	size_t pathname_len = strlen(pathname);
	if (pathname[pathname_len - 1] != '/') {
		pathname[pathname_len++] = '/';
	}
	char *suffix = pathname + pathname_len;

	// Recurse down to deeper levels of the temp directory hierarchy if necessary.
	if (remaining_path_format) {
		char *separator = remaining_path_format;
		while (*separator != '/' && *separator != '\0') {
			separator++;
		}
		size_t length_to_match = (*remaining_path_format == '<') ? 0 : separator - remaining_path_format;
		char *next_remaining_path_format = (*separator == '\0') ? NULL : separator + 1;

		while ((entry = readdir(dp)) != NULL) {
			if (entry->d_type == DT_DIR && entry->d_name[0] != '.') {
				if (length_to_match > 0 && strncmp(entry->d_name, remaining_path_format, length_to_match) != 0) {
					continue;
				}
				strlcpy(suffix, entry->d_name, (size_t)PATH_MAX - pathname_len);
				reap_orphaned_log_files_in_hierarchy(pathname, next_remaining_path_format, target_pid, streams);
			}
		}
		closedir(dp);

		return;
	}

	// OK, we found a lowest-level directory matching <remaining_path_format>, and we have access to it.
	// Reap any unnecessary stack log files in here.

	//_malloc_printf(ASL_LEVEL_INFO, "reaping: looking in %s\n", directory);

	// __stack_log_file_path__ may be NULL if this code is running in an analysis tool client process that is not
	// itself running with MallocStackLogging set.
	char *curproc_stack_log_file = __stack_log_file_path__ ? strrchr(__stack_log_file_path__, '/') + 1 : NULL;
	pid_t curpid = getpid();
	size_t prefix_length = strlen(stack_log_file_base_name);

	while ((entry = readdir(dp)) != NULL) {
		if ((entry->d_type == DT_REG || entry->d_type == DT_LNK) &&
				(strncmp(entry->d_name, stack_log_file_base_name, prefix_length) == 0)) {
			long pid = strtol(&entry->d_name[prefix_length], (char **)NULL, 10);
			if (pid == target_pid && streams != NULL) {
				strlcpy(suffix, entry->d_name, (size_t)PATH_MAX - pathname_len);
				open_log_file_at_path(pathname, streams);
			}
			else if (!is_process_running((pid_t)pid) ||
					(pid == curpid && curproc_stack_log_file && strcmp(entry->d_name, curproc_stack_log_file) != 0)) {
				strlcpy(suffix, entry->d_name, (size_t)PATH_MAX - pathname_len);
				if (delete_logging_file(pathname) == 0) {
					if (pid == curpid) {
						_malloc_printf(ASL_LEVEL_INFO, "stack logs deleted from %s\n", pathname);
					} else {
						_malloc_printf(ASL_LEVEL_INFO, "process %ld no longer exists, stack logs deleted from %s\n", pid, pathname);
					}
				}
			}
		}
	}
	closedir(dp);
}

static void
reap_orphaned_log_files(pid_t target_pid, remote_task_file_streams *streams)
{
	reap_orphaned_log_files_in_hierarchy(_PATH_TMP, NULL, target_pid, streams);
	
	char *env_var_names[] = {"TMPDIR", "MallocStackLoggingDirectory"};
	for (unsigned i = 0; i < sizeof(env_var_names) / sizeof(char *); i++) {
		char directory[PATH_MAX];
		bool success = getenv_from_process(target_pid, env_var_names[i], directory, sizeof(directory));
		if (success && strcmp(directory, _PATH_TMP) != 0) {
			reap_orphaned_log_files_in_hierarchy(directory, NULL, target_pid, streams);
		}
	}
	
	// Now reap files left over in any other accessible app-specific temp directories.
	// These could be from sandbox'ed apps.
#if TARGET_OS_EMBEDDED
	char *root_of_temp_directories = "/private/var/mobile/Containers/Data/Application"; // ugh - hard-coding to user name "mobile".
	// Works for all iOS's up to now.
	char *temp_directories_path_format = "<application-UUID>/tmp";
#else
	char *root_of_temp_directories = "/private/var/folders";
	char *temp_directories_path_format = "<xx>/<random>/T";
#endif
	reap_orphaned_log_files_in_hierarchy(root_of_temp_directories, temp_directories_path_format, target_pid, streams);
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
	__syscall_logger = NULL;
	disable_stack_logging_lite();
}

static boolean_t uniquing_table_memory_was_deleted = false;

__attribute__((visibility("hidden"))) boolean_t
__uniquing_table_memory_was_deleted(void)
{
	return uniquing_table_memory_was_deleted;
}

__attribute__((visibility("hidden"))) void
__delete_uniquing_table_memory_while_locked(void)
{
	// Clean out the memory (if not done already)
	if (pre_write_buffers && pre_write_buffers->uniquing_table != NULL) {
		__destroy_uniquing_table(pre_write_buffers->uniquing_table);
		pre_write_buffers->uniquing_table = NULL;
		uniquing_table_memory_was_deleted = true;
	}
	
	// Clear the shared memory address so client tools won't look for the uniquing table memory
	__mach_stack_logging_shared_memory_address = 0;
}

/* A wrapper around write() that will try to reopen the index/stack file and
 * write to it if someone closed it underneath us (e.g. the process we just
 * started decide to close all file descriptors except stin/err/out). Some
 * programs like to do that and calling abort() on them is rude.
 */
static ssize_t
robust_write(int fd, const void *buf, size_t nbyte)
{
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
			int fds_to_close[3] = {0};
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
			if (fds_to_close[0]) {
				close(0);
			}
			if (fds_to_close[1]) {
				close(1);
			}
			if (fds_to_close[2]) {
				close(2);
			}
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
	char *p;

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
			_malloc_printf(
					ASL_LEVEL_INFO, "Unable to write to stack logging file %s (%s)\n", __stack_log_file_path__, strerror(errno));
			disable_stack_logging();
			return;
		}
		p += written;
		remaining -= written;
	}

	pre_write_buffers->start_index_offset += pre_write_buffers->next_free_index_buffer_offset;
	pre_write_buffers->next_free_index_buffer_offset = 0;
}

__attribute__((visibility("hidden"))) boolean_t
__prepare_to_log_stacks(boolean_t lite_mode)
{
	if (!pre_write_buffers) {
		last_logged_malloc_address = 0ul;
		logging_use_compaction = (stack_logging_dontcompact ? 0 : logging_use_compaction);

		// Create a VM region to hold the pre-write index and stack buffers. The address of this VM region will be
		// encoded into the stack log file name, so that the stack log reading code running in remote analysis
		// processes can find it and map it into the analysis process. This allows remote analysis processes to access
		// these buffers to get logs for even the most recent allocations. The remote process will need to pause this
		// process to assure that the contents of these buffers don't change while being inspected.
		//
		// We used to use shm_open() to create a shared memory region for this, but since this code runs in arbitrary
		// processes that may have sandbox restrictions that don't allow the creation of shared memory regions,
		// we're using this "create a region and put its address in the stack log file name" approach.
		size_t full_shared_mem_size = sizeof(stack_buffer_shared_memory);
		pre_write_buffers = mmap(
				0, full_shared_mem_size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, VM_MAKE_TAG(VM_MEMORY_ANALYSIS_TOOL), 0);
		if (MAP_FAILED == pre_write_buffers) {
			_malloc_printf(ASL_LEVEL_INFO, "error creating VM region for stack logging output buffers\n");
			disable_stack_logging();
			return false;
		}

		// Store and use the buffer offsets in shared memory so that they can be accessed remotely
		pre_write_buffers->start_index_offset = 0ull;
		pre_write_buffers->next_free_index_buffer_offset = 0;

		// create the backtrace uniquing table
		pre_write_buffers->uniquing_table = __create_uniquing_table(lite_mode);
		if (!pre_write_buffers->uniquing_table) {
			_malloc_printf(ASL_LEVEL_INFO, "error while allocating stack uniquing table\n");
			disable_stack_logging();
			return false;
		}

		uint64_t stack_buffer_sz = (uint64_t)round_page(sizeof(vm_address_t) * STACK_LOGGING_MAX_STACK_SIZE);
		stack_buffer = (vm_address_t *)sld_allocate_pages(stack_buffer_sz);
		if (!stack_buffer) {
			_malloc_printf(ASL_LEVEL_INFO, "error while allocating stack trace buffer\n");
			disable_stack_logging();
			return false;
		}

		// lite_mode doesn't use a file
		if (lite_mode) {
			__mach_stack_logging_shared_memory_address = (uint64_t) pre_write_buffers;
		} else {
			// this call ensures that the log files exist; analyzing processes will rely on this assumption.
			if (create_log_file() == NULL) {
				/* postponement support requires cleaning up these structures now */
				__destroy_uniquing_table(pre_write_buffers->uniquing_table);
				sld_deallocate_pages(stack_buffer, stack_buffer_sz);
				stack_buffer = NULL;

				munmap(pre_write_buffers, full_shared_mem_size);
				pre_write_buffers = NULL;

				if (!stack_logging_postponed) {
					disable_stack_logging();
				}
				return false;
			}
		}
	}
	
	return true;
}

__attribute__((visibility("hidden"))) void
__prepare_to_log_stacks_stage2(void)
{
	static int stage2done = 0;

	if (!stage2done) {
		// malloc() can be called by the following, so these need to be done outside the stack_logging_lock but after the buffers
		// have been set up.
		atexit(delete_log_files); // atexit() can call malloc()

		// Reaping orphaned stack log files from dead processes is a nicety, to help
		// reduce wasted disk space.  But we don't *always* have to do it.  Specifically,
		// do not reap orphaned stack log files if the process name is sandboxd or taskgated,
		// or if the MallocStackLoggingNoReaping env var is set to any value other than "no"
		// (case-insensitive) or "0".  This provides multiple ways to fix
		// <rdar://problem/14409213> "processes hang if sandboxd is running with
		// MallocStackLogging enabled", which happened because there were two different
		// places down inside reap_orphaned_log_files() which called sysctl() for KERN_PROCARGS2
		// or KERN_PROC_PID, causing iteration of the process list in the kernel, which takes
		// a lock, which can't happen when processes are in a transitional state.
		bool should_reap = true;
		const char *progname = getprogname();
		if (progname && (strcmp(progname, "sandboxd") == 0 || strcmp(progname, "taskgated") == 0)) {
			should_reap = false;
		}
		if (should_reap) {
			char *noreap = getenv("MallocStackLoggingNoReaping");
			if (noreap && strcasecmp(noreap, "no") != 0 && strcmp(noreap, "0") != 0) {
				should_reap = false;
			}
		}
		if (should_reap) {
			reap_orphaned_log_files(getpid(), NULL); // this calls opendir() which calls malloc()
		}

		stage2done = 1;
	}
}

__attribute__((visibility("hidden"))) void
__malloc_lock_stack_logging()
{
	_malloc_lock_lock(&stack_logging_lock);
}

__attribute__((visibility("hidden"))) void
__malloc_unlock_stack_logging()
{
	_malloc_lock_unlock(&stack_logging_lock);
}

const uint64_t __invalid_stack_id = (uint64_t)(-1ll);

// returns the stack id or invalid_stack_id if any kind of error
// this needs to be done while stack_logging_lock is locked)

__attribute__((visibility("hidden"))) uint64_t
__enter_stack_into_table_while_locked(vm_address_t self_thread, uint32_t num_hot_to_skip, boolean_t add_thread_id, size_t ptr_size)
{
	// gather stack
	uint32_t count;
	thread_stack_pcs(stack_buffer, STACK_LOGGING_MAX_STACK_SIZE - 1, &count); // only gather up to STACK_LOGGING_MAX_STACK_SIZE-1 since we append thread id
	
	if (add_thread_id) {
		stack_buffer[count++] = self_thread + 1;   // stuffing thread # in the coldest slot. Add 1 to match what the old stack logging did.
	}
	
	// skip stack frames after the malloc call
	num_hot_to_skip += 4;
	
	if (count <= num_hot_to_skip) {
		// Oops!  Didn't get a valid backtrace from thread_stack_pcs().
		return __invalid_stack_id;
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
	
	uint64_t uniqueStackIdentifier = __invalid_stack_id;
	
	while (!enter_frames_in_table(pre_write_buffers->uniquing_table, &uniqueStackIdentifier, frames, count, ptr_size)) {
		if (!__expand_uniquing_table(pre_write_buffers->uniquing_table))
			return __invalid_stack_id;
	}
	
	return uniqueStackIdentifier;
}

static void
decrement_ref_count(table_slot_t *table_slot, size_t ptr_size)
{
	if (table_slot->refcount_slot.refcount > 0) {
		table_slot->refcount_slot.refcount -= ptr_size;
		
		if (table_slot->refcount_slot.refcount == 0) {
			table_slot->slots.slot0 = table_slot->slots.slot1 = 0;
		}
	}
}

__attribute__((visibility("hidden"))) void
__decrement_table_slot_refcount(uint64_t stack_id, size_t ptr_size)
{
	__malloc_lock_stack_logging();

	// see if msl lite was disabled behind our backs
	if (!is_stack_logging_lite_enabled()) {
		__malloc_unlock_stack_logging();
		return;
	}

	backtrace_uniquing_table *uniquing_table = pre_write_buffers->uniquing_table;
	
	assert(uniquing_table->nodes_use_refcount);
	assert(!uniquing_table->in_client_process);

	slot_parent parent = stack_id;
	slot_parent prev_parent = __invalid_stack_id;
	
	do {
		if (parent == prev_parent) {
			malloc_printf("circular parent reference in __decrement_table_slot_refcount\n");
			break;
		}
		
		prev_parent = parent;
		
		table_slot_t *table_slot = (table_slot_t *) (uniquing_table->u.table + (parent * 2));
		
		parent = table_slot->refcount_slot.parent;
		decrement_ref_count(table_slot, ptr_size);
	} while (parent != slot_no_parent_refcount);
	
	__malloc_unlock_stack_logging();
}

void
__disk_stack_logging_log_stack(uint32_t type_flags,
		uintptr_t zone_ptr,
		uintptr_t arg2,
		uintptr_t arg3,
		uintptr_t return_val,
		uint32_t num_hot_to_skip)
{
	if (!stack_logging_enable_logging || stack_logging_postponed) {
		return;
	}

	uintptr_t size;
	uintptr_t ptr_arg;

	// check incoming data
	if (type_flags & stack_logging_type_alloc && type_flags & stack_logging_type_dealloc) {
		size = arg3;
		ptr_arg = arg2; // the original pointer
		if (ptr_arg == return_val) {
			return; // realloc had no effect, skipping
		}
		if (ptr_arg == 0) { // realloc(NULL, size) same as malloc(size)
			type_flags ^= stack_logging_type_dealloc;
		} else {
			// realloc(arg1, arg2) -> result is same as free(arg1); malloc(arg2) -> result
			__disk_stack_logging_log_stack(
					stack_logging_type_dealloc, zone_ptr, ptr_arg, (uintptr_t)0, (uintptr_t)0, num_hot_to_skip + 1);
			__disk_stack_logging_log_stack(stack_logging_type_alloc, zone_ptr, size, (uintptr_t)0, return_val, num_hot_to_skip + 1);
			return;
		}
	}
	if (type_flags & stack_logging_type_dealloc || type_flags & stack_logging_type_vm_deallocate) {
		// For VM deallocations we need to know the size, since they don't always match the
		// VM allocations.  It would be nice if arg2 was the size, for consistency with alloc and
		// realloc events.  However we can't easily make that change because all projects
		// (malloc.c, GC auto_zone, and gmalloc) have historically put the pointer in arg2 and 0 as
		// the size in arg3.  We'd need to change all those projects in lockstep, which isn't worth
		// the trouble.
		ptr_arg = arg2;
		size = arg3;
		if (ptr_arg == 0) {
			return; // free(nil)
		}
	}
	if (type_flags & stack_logging_type_alloc || type_flags & stack_logging_type_vm_allocate) {
		if (return_val == 0) {
			return; // alloc that failed
		}
		size = arg2;
	}

	if (type_flags & stack_logging_type_vm_allocate || type_flags & stack_logging_type_vm_deallocate) {
		mach_port_t targetTask = (mach_port_t)zone_ptr;
		// For now, ignore "injections" of VM into other tasks.
		if (targetTask != mach_task_self()) {
			return;
		}
	}

	type_flags &= stack_logging_valid_type_flags;

	vm_address_t self_thread = (vm_address_t)_os_tsd_get_direct(__TSD_THREAD_SELF);
	static vm_address_t thread_doing_logging = 0;

	if (thread_doing_logging == self_thread) {
		// Prevent a thread from deadlocking against itself if vm_allocate() or malloc()
		// is called below here, from __prepare_to_log_stacks() or _prepare_to_log_stacks_stage2(),
		// or if we are logging an event and need to call __expand_uniquing_table() which calls
		// vm_allocate() to grow stack logging data structures.  Any such "administrative"
		// vm_allocate or malloc calls would attempt to recursively log those events.
		return;
	}

	// lock and enter
	_malloc_lock_lock(&stack_logging_lock);

	thread_doing_logging = self_thread; // for preventing deadlock'ing on stack logging on a single thread

	// now actually begin
	__prepare_to_log_stacks(false);

	// since there could have been a fatal (to stack logging) error such as the log files not being created, check these variables
	// before continuing
	if (!stack_logging_enable_logging || stack_logging_postponed) {
		thread_doing_logging = 0;
		_malloc_lock_unlock(&stack_logging_lock);
		return;
	}

	if (type_flags & stack_logging_type_alloc) {
		// Only do this second stage of setup when we first record a malloc (as opposed to a VM allocation),
		// to ensure that the malloc zone has already been created as is necessary for this.
		__prepare_to_log_stacks_stage2();
	}

	// compaction
	if (last_logged_malloc_address && (type_flags & stack_logging_type_dealloc) &&
			STACK_LOGGING_DISGUISE(ptr_arg) == last_logged_malloc_address) {
		// *waves hand* the last allocation never occurred
		pre_write_buffers->next_free_index_buffer_offset -= (uint32_t)sizeof(stack_logging_index_event);
		last_logged_malloc_address = 0ul;

		thread_doing_logging = 0;
		_malloc_lock_unlock(&stack_logging_lock);
		return;
	}

	uint64_t uniqueStackIdentifier = __enter_stack_into_table_while_locked(self_thread, num_hot_to_skip, true, 0);
	
	if (uniqueStackIdentifier == __invalid_stack_id) {
		thread_doing_logging = 0;
		_malloc_lock_unlock(&stack_logging_lock);
		return;
	}

	stack_logging_index_event current_index;
	if (type_flags & stack_logging_type_alloc || type_flags & stack_logging_type_vm_allocate) {
		current_index.address = STACK_LOGGING_DISGUISE(return_val);
		current_index.argument = size;
		if (logging_use_compaction) {
			last_logged_malloc_address = current_index.address; // disguised
		}
	} else {
		current_index.address = STACK_LOGGING_DISGUISE(ptr_arg);
		current_index.argument = size;
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
	memcpy(pre_write_buffers->index_buffer + pre_write_buffers->next_free_index_buffer_offset, &current_index,
			sizeof(stack_logging_index_event));
	pre_write_buffers->next_free_index_buffer_offset += (uint32_t)sizeof(stack_logging_index_event);

	thread_doing_logging = 0;
	_malloc_lock_unlock(&stack_logging_lock);
}

void
__stack_logging_fork_prepare(void)
{
	_malloc_lock_lock(&stack_logging_lock);
}

void
__stack_logging_fork_parent(void)
{
	_malloc_lock_unlock(&stack_logging_lock);
}

void
__stack_logging_fork_child(void)
{
	malloc_logger = NULL;
	stack_logging_enable_logging = 0;
	_malloc_lock_init(&stack_logging_lock);
}

void
__stack_logging_early_finished(void)
{
	stack_logging_finished_init = 1;
	stack_logging_postponed = 0;
}

// support for gdb and others checking for stack_logging locks
__attribute__((visibility("hidden"))) boolean_t
__stack_logging_locked(void)
{
	bool acquired_lock = _malloc_lock_trylock(&stack_logging_lock);
	if (acquired_lock) {
		_malloc_lock_unlock(&stack_logging_lock);
	}
	return (acquired_lock ? false : true);
}

#pragma mark -
#pragma mark Remote Stack Log Access

#pragma mark - Design notes:

/*
 *
 * this first one will look through the index, find the "stack_identifier" (i.e. the offset in the log file), and call the third
 * function listed here.
 * extern kern_return_t __mach_stack_logging_get_frames(task_t task, mach_vm_address_t address, mach_vm_address_t
 * stack_frames_buffer, uint32_t max_stack_frames, uint32_t *num_frames);
 * //  Gets the last allocation record about address
 *
 * if !address, will load index and iterate through (expensive)
 * else will load just index, search for stack, and then use third function here to retrieve. (also expensive)
 * extern kern_return_t __mach_stack_logging_enumerate_records(task_t task, mach_vm_address_t address, void
 * enumerator(mach_stack_logging_record_t, void *), void *context);
 * // Applies enumerator to all records involving address sending context as enumerator's second parameter; if !address, applies
 * enumerator to all records
 *
 * this function will load the stack file, look for the stack, and follow up to STACK_LOGGING_FORCE_FULL_BACKTRACE_EVERY references
 * to
 * reconstruct.
 * extern kern_return_t __mach_stack_logging_frames_for_uniqued_stack(task_t task, uint64_t stack_identifier, mach_vm_address_t
 * stack_frames_buffer, uint32_t max_stack_frames, uint32_t *count);
 * // Given a uniqued_stack fills stack_frames_buffer
 *
 */

#pragma mark -
#pragma mark Backtrace Uniquing Table Reading and Lookup

// This is client-side code to get a stack log from a uniquing_table.
static void
free_uniquing_table_chunks(backtrace_uniquing_table *uniquing_table)
{
	table_chunk_header_t *table_chunk_header = uniquing_table->u.first_table_chunk_hdr;
	assert(uniquing_table->in_client_process);
	while (table_chunk_header) {
		mach_vm_deallocate(mach_task_self(), (mach_vm_address_t)(uintptr_t)(table_chunk_header->table_chunk),
				table_chunk_header->table_chunk_size);
		table_chunk_header_t *next = table_chunk_header->next_table_chunk_header;
		free(table_chunk_header);
		table_chunk_header = next;
	}
}

static kern_return_t
read_uniquing_table_from_task(task_t remote_task, backtrace_uniquing_table *uniquing_table)
{
	assert(uniquing_table->in_client_process);
	mach_vm_address_t next_address_to_read = uniquing_table->table_address;
	uint64_t remaining_size_to_read = uniquing_table->tableSize;
	const mach_vm_size_t two_gigabytes =
			2ull * 1024 * 1024 * 1024; // attempting to read 4 GB in one call fails, so try a max of 2 GB
	table_chunk_header_t **table_chunk_hdr_ptr = &(uniquing_table->u.first_table_chunk_hdr);
	*table_chunk_hdr_ptr = NULL;

	while (remaining_size_to_read > 0ull) {
		vm_address_t local_table_chunk_address = 0ul;
		mach_msg_type_number_t local_table_chunk_size = 0;

		mach_vm_size_t next_size_to_read = (remaining_size_to_read > two_gigabytes) ? two_gigabytes : remaining_size_to_read;
		while (1) {
			kern_return_t err = mach_vm_read(
					remote_task, next_address_to_read, next_size_to_read, &local_table_chunk_address, &local_table_chunk_size);
			if (err == KERN_SUCCESS) {
				*table_chunk_hdr_ptr = malloc(sizeof(table_chunk_header_t));
				table_chunk_header_t *table_chunk_hdr = *table_chunk_hdr_ptr;
				table_chunk_hdr->num_nodes_in_chunk = local_table_chunk_size / (sizeof(mach_vm_address_t) * 2);
				;
				table_chunk_hdr->table_chunk = (mach_vm_address_t *)local_table_chunk_address;
				table_chunk_hdr->table_chunk_size = local_table_chunk_size;
				table_chunk_hdr->next_table_chunk_header = NULL;				   // initialize it, in case it is the last chunk
				table_chunk_hdr_ptr = &(table_chunk_hdr->next_table_chunk_header); // set up to assign next chunk to this

				next_address_to_read += local_table_chunk_size;
				remaining_size_to_read -= local_table_chunk_size;
				// fprintf(stderr, "requested %#qx, got %#x of %#qx at %p from backtrace uniquing table of target process\n",
				// next_size_to_read, local_table_chunk_size, uniquing_table->tableSize, table_chunk_hdr);
				break;
			} else {
				// fprintf(stderr, "requested %#qx, failed\n", next_size_to_read);
				next_size_to_read /= 2;
				if (next_size_to_read <= 1024 * 1024) {
					// We couldn't even map one megabyte?  Let's call that an error...
					free_uniquing_table_chunks(uniquing_table);
					return err;
				}
			}
		}
	}
	return KERN_SUCCESS;
}

static mach_vm_address_t *
get_node_from_uniquing_table(backtrace_uniquing_table *uniquing_table, uint64_t index_pos)
{
	assert(uniquing_table->in_client_process);
	table_chunk_header_t *table_chunk_hdr = uniquing_table->u.first_table_chunk_hdr;
	uint64_t start_node_of_chunk = 0;
	while (table_chunk_hdr && index_pos > start_node_of_chunk + table_chunk_hdr->num_nodes_in_chunk) {
		table_chunk_hdr = table_chunk_hdr->next_table_chunk_header;
		if (table_chunk_hdr) {
			start_node_of_chunk += table_chunk_hdr->num_nodes_in_chunk;
		}
	}
	
	// Handle case where someone passes an invalid stack id
	// <rdar://problem/25337823> get_node_from_uniquing_table should be more tolerant
	if (!table_chunk_hdr) {
		return NULL;
	}

	uint64_t index_in_chunk = index_pos - start_node_of_chunk;
	mach_vm_address_t *node = table_chunk_hdr->table_chunk + (index_in_chunk * 2);
	return node;
}

static void
unwind_stack_from_table_index(backtrace_uniquing_table *uniquing_table,
		uint64_t index_pos,
		mach_vm_address_t *out_frames_buffer,
		uint32_t *out_frames_count,
		uint32_t max_frames,
		boolean_t use_refcount)
{
	mach_vm_address_t *node = get_node_from_uniquing_table(uniquing_table, index_pos);
	uint32_t foundFrames = 0;
	slot_parent end_parent = use_refcount ? slot_no_parent_refcount : slot_no_parent_normal;
	
	if (node && index_pos < uniquing_table->numNodes) {
		while (foundFrames < max_frames) {
			table_slot_t *table_slot = (table_slot_t *) (node);
			
			slot_address address = use_refcount ? table_slot->refcount_slot.address : table_slot->normal_slot.address;
			
			out_frames_buffer[foundFrames++] = address;
			
			if (use_refcount && table_slot->refcount_slot.refcount == 0) {
				break;
			}
			
			slot_parent parent = use_refcount ? table_slot->refcount_slot.parent : table_slot->normal_slot.parent;
			
			if (parent == end_parent) {
				break;
			}
			
			node = get_node_from_uniquing_table(uniquing_table, parent);
		}
	}

	*out_frames_count = foundFrames;
}

#pragma mark - caching

__attribute__((always_inline)) static inline size_t
hash_index(uint64_t address, size_t max_pos)
{
	return (size_t)((address >> 2) % (max_pos - 1)); // simplicity rules.
}

__attribute__((always_inline)) static inline size_t
hash_multiplier(size_t capacity, uint32_t allowed_collisions)
{
	return (capacity / (allowed_collisions * 2 + 1));
}

__attribute__((always_inline)) static inline size_t
next_hash(size_t hash, size_t multiplier, size_t capacity, uint32_t collisions)
{
	hash += multiplier * collisions;
	if (hash >= capacity) {
		hash -= capacity;
	}
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
		fprintf(stderr, "reporting bad hash function! disk stack logging reader %lu bit. (transfer_node)\n", sizeof(void *) * 8);
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
	cache->table_memory = (void *)calloc(cache->cache_node_capacity, sizeof(remote_index_node));

	// repopulate (expensive!)
	size_t i;
	for (i = 0; i < old_node_capacity; i++) {
		if (old_table[i].address) {
			transfer_node(cache, &old_table[i]);
		}
	}
	free(old_table);
	//	printf("cache expanded to %0.2f mb (eff: %3.0f%%, capacity: %lu, nodes: %llu, llnodes: %llu)\n",
	//((float)(cache->cache_size))/(1 << 20), ((float)(cache->cache_node_count)*100.0)/((float)(cache->cache_node_capacity)),
	// cache->cache_node_capacity, cache->cache_node_count, cache->cache_llnode_count);
}

static void
insert_node(remote_index_cache *cache, uint64_t address, uint64_t index_file_offset)
{
	uint32_t collisions = 0;
	size_t pos = hash_index(address, cache->cache_node_capacity);
	size_t multiplier = hash_multiplier(cache->cache_node_capacity, cache->collision_allowance);

	while (1) {
		if (cache->table_memory[pos].address == 0ull || cache->table_memory[pos].address == address) { // hit or empty
			cache->table_memory[pos].address = address;
			cache->table_memory[pos].index_file_offset = index_file_offset;
			// Inserted it!  Break out of the loop.
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

// Kudos to Daniel Delwood for this function.  This is called in an analysis tool process
// to share a VM region from a target process, without the target process needing to explicitly
// share the region itself via shm_open().  The VM_FLAGS_RETURN_DATA_ADDR flag is necessary
// for iOS in case the target process uses a different VM page size than the analysis tool process.
static mach_vm_address_t
map_shared_memory_from_task(task_t sourceTask, mach_vm_address_t sourceAddress, mach_vm_size_t sourceSize)
{
#if TARGET_OS_EMBEDDED
	int mapRequestFlags = VM_FLAGS_ANYWHERE | VM_FLAGS_RETURN_DATA_ADDR;
	mach_vm_address_t mapRequestAddress = sourceAddress;
	mach_vm_size_t mapRequestSize = sourceSize;
#else
	// Sadly, VM_FLAGS_RETURN_DATA_ADDR isn't available to us; align everything manually.
	int mapRequestFlags = VM_FLAGS_ANYWHERE;
	mach_vm_address_t mapRequestAddress = trunc_page(sourceAddress);
	mach_vm_size_t mapRequestSize = round_page(sourceAddress + sourceSize) - mapRequestAddress;
#endif
	mach_vm_address_t mappedAddress = 0;
	vm_prot_t outCurrentProt = VM_PROT_NONE;
	vm_prot_t outMaxProt = VM_PROT_NONE;
	kern_return_t err = mach_vm_remap(mach_task_self(), &mappedAddress, mapRequestSize, 0, mapRequestFlags, sourceTask,
			mapRequestAddress, false, &outCurrentProt, &outMaxProt, VM_INHERIT_NONE);
	if (err != KERN_SUCCESS) {
		return 0;
	}
	return mappedAddress + (sourceAddress - mapRequestAddress);
}

static kern_return_t
update_cache_for_file_streams(remote_task_file_streams *descriptors)
{
	remote_index_cache *cache = descriptors->cache;

	// create from scratch if necessary.
	if (!cache) {
		descriptors->cache = cache = (remote_index_cache *)calloc((size_t)1, sizeof(remote_index_cache));
		cache->cache_node_capacity = 1 << 14;
		cache->collision_allowance = 17;
		cache->last_index_file_offset = 0;
		cache->cache_size = cache->cache_node_capacity * sizeof(remote_index_node);
		cache->table_memory = (void *)calloc(cache->cache_node_capacity, sizeof(remote_index_node));

		cache->shmem = (stack_buffer_shared_memory *)map_shared_memory_from_task(descriptors->remote_task,
				descriptors->remote_stack_buffer_shared_memory_address, sizeof(stack_buffer_shared_memory));
		if (!cache->shmem) {
			// failed to connect to the shared memory region; warn and continue.
			_malloc_printf(ASL_LEVEL_INFO,
					"warning: unable to map shared memory from %llx in target process %d; no stack backtraces will be available.\n",
					descriptors->remote_stack_buffer_shared_memory_address, descriptors->remote_pid);
		}
		cache->lite_mode = descriptors->task_uses_lite_mode;
	}

	// suspend and see how much updating there is to do. there are three scenarios, listed below
	bool update_snapshot = false;
	if (descriptors->remote_task != mach_task_self()) {
		task_suspend(descriptors->remote_task);
	}

	struct stat file_statistics;
	
	if (descriptors->index_file_stream) {
		fstat(fileno(descriptors->index_file_stream), &file_statistics);
	} else {
		file_statistics.st_size = 0;
	}
		
	size_t read_size = (descriptors->task_is_64_bit ? sizeof(stack_logging_index_event64) : sizeof(stack_logging_index_event32));
	uint64_t read_this_update = 0;

	// the delta indecies is a complex number; there are three cases:
	// 1. there is no shared memory (or we can't connect); diff the last_index_file_offset from the filesize.
	// 2. the only updates have been in shared memory; disk file didn't change at all. delta_indecies should be zero, scan snapshot
	// only.
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

	// need to update the snapshot if in lite mode and haven't yet read the uniquing table
	if (descriptors->task_uses_lite_mode && cache->uniquing_table_snapshot.numPages == 0) {
		update_snapshot = true;
	}
	
	// if a snapshot is necessary, memcpy from remote frozen process' memory
	// note: there were two ways to do this - spin lock or suspend. suspend allows us to
	// analyze processes even if they were artificially suspended. with a lock, there'd be
	// worry that the target was suspended with the lock taken.
	kern_return_t err = KERN_SUCCESS;
	if (update_snapshot) {
		memcpy(&cache->snapshot, cache->shmem, sizeof(stack_buffer_shared_memory));
		// also need to update our version of the remote uniquing table
		vm_address_t local_uniquing_address = 0ul;
		mach_msg_type_number_t local_uniquing_size = 0;
		mach_vm_size_t desired_size = round_page(sizeof(backtrace_uniquing_table));
		if ((err = mach_vm_read(descriptors->remote_task, (mach_vm_address_t)cache->shmem->uniquing_table, desired_size,
					 &local_uniquing_address, &local_uniquing_size)) != KERN_SUCCESS ||
				local_uniquing_size != desired_size) {
			fprintf(stderr, "error while attempting to mach_vm_read remote stack uniquing table (%d): %s\n", err,
					mach_error_string(err));
		} else {
			// the mach_vm_read was successful, so acquire the uniquing table

			// need to re-read the table, so deallocate the current memory
			cache->uniquing_table_snapshot.in_client_process = true;
			free_uniquing_table_chunks(&cache->uniquing_table_snapshot);

			// The following line copies the uniquing table structure data, but the actual uniquing table memory is invalid
			// since it's a pointer from the remote process.
			cache->uniquing_table_snapshot = *((backtrace_uniquing_table *)local_uniquing_address);
			cache->uniquing_table_snapshot.nodes_use_refcount = cache->lite_mode;
			cache->uniquing_table_snapshot.u.first_table_chunk_hdr = NULL;
			cache->uniquing_table_snapshot.in_client_process = true;


			// Read the uniquing table memory from the target process.
			err = read_uniquing_table_from_task(descriptors->remote_task, &(cache->uniquing_table_snapshot));
			if (err) {
				fprintf(stderr, "error while attempting to mach_vm_read remote stack uniquing table contents (%d): %s\n", err,
						mach_error_string(err));
			}
			// Check the error status below, after further deallocating and resuming the target task.

			mach_vm_deallocate(mach_task_self(), (mach_vm_address_t)local_uniquing_address, (mach_vm_size_t)local_uniquing_size);
		}
	}

	// resume
	if (descriptors->remote_task != mach_task_self()) {
		task_resume(descriptors->remote_task);
	}

	if (err != KERN_SUCCESS) {
		// To Do:  further clean up allocated resources, and also try to prevent printing numerous identical "out of memory" errors
		// (maybe we should abort?).
		return err;
	}

	if (!update_snapshot && delta_indecies == 0) {
		return KERN_SUCCESS; // absolutely no updating needed.
	}
	FILE *the_index = (descriptors->index_file_stream);

	// prepare for the read; target process could be 32 or 64 bit.

	stack_logging_index_event32 *target_32_index = NULL;
	stack_logging_index_event64 *target_64_index = NULL;

	// perform the update from the file
	uint32_t i;
	if (delta_indecies) {
		char bufferSpace[4096]; // 4 kb
		target_32_index = (stack_logging_index_event32 *)bufferSpace;
		target_64_index = (stack_logging_index_event64 *)bufferSpace;
		size_t number_slots = (size_t)(4096 / read_size);

		size_t read_count = 0;
		if (fseeko(the_index, (off_t)(cache->last_index_file_offset), SEEK_SET)) {
			fprintf(stderr,
					"error while attempting to cache information from remote stack index file. (update_cache_for_file_streams)\n");
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
					insert_node(cache, (mach_vm_address_t)STACK_LOGGING_DISGUISE(target_32_index[i].address),
							(uint64_t)current_index_position);
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
		target_32_index = (stack_logging_index_event32 *)(cache->snapshot.index_buffer);
		target_64_index = (stack_logging_index_event64 *)(cache->snapshot.index_buffer);

		uint32_t free_snapshot_scan_index = cache->snapshot.next_free_index_buffer_offset / (uint32_t)read_size;
		off_t current_index_position = cache->snapshot.start_index_offset;
		if (descriptors->task_is_64_bit) {
			for (i = last_snapshot_scan_index; i < free_snapshot_scan_index; i++) {
				insert_node(cache, STACK_LOGGING_DISGUISE(target_64_index[i].address),
						(uint64_t)(current_index_position + (i * read_size)));
			}
		} else {
			for (i = last_snapshot_scan_index; i < free_snapshot_scan_index; i++) {
				insert_node(cache, (mach_vm_address_t)STACK_LOGGING_DISGUISE(target_32_index[i].address),
						(uint64_t)(current_index_position + (i * read_size)));
			}
		}
	}

	return KERN_SUCCESS;
}

static void
destroy_cache_for_file_streams(remote_task_file_streams *descriptors)
{
	if (!descriptors->cache) {
		return;
	}
	if (descriptors->cache->shmem) {
		munmap(descriptors->cache->shmem, sizeof(stack_buffer_shared_memory));
	}
	free(descriptors->cache->table_memory);
	free_uniquing_table_chunks(&descriptors->cache->uniquing_table_snapshot);
	free(descriptors->cache);
	descriptors->cache = NULL;
}

#pragma mark - internal

static FILE *
open_log_file_at_path(char *pathname, remote_task_file_streams *streams) {
	FILE *file = fopen(pathname, "r");
	if (!file) {
		return NULL;
	}
	
	char *log_file_name = strrchr(pathname, '/');
	char *p = log_file_name;
	
	// File names are of the form stack-logs.<pid>.<address>.<progname>.XXXXXX.index
	if (p) p = strchr(p, '.');			// skip past "stack-logs"
	if (p) p = strchr(p + 1, '.');		// skip past ".<pid>"
	if (p) p++;							// skip past '.'
		
	if (!p) {
		return NULL;
	}
	
	char *shared_memory_address_string = p;
	
	// The hex address of the remote_index_cache in the target process
	// is given in the stack log file name, following the pid and a period.
	streams->remote_stack_buffer_shared_memory_address = strtoll(shared_memory_address_string, NULL, 16);
	streams->index_file_stream = file;
	
	return file;
}

// In the stack log analysis process, find the stack logging file for target process <pid>
// by scanning the given directory for entries with names of the form "stack-logs.<pid>.*.index"
// If we find such an entry then open that stack logging file.
static FILE *
open_log_file_from_directory(pid_t pid, char *directory, remote_task_file_streams *streams)
{
	DIR *dp;
	struct dirent *entry;
	char prefix_and_pid[PATH_MAX];
	char pathname[PATH_MAX];
	FILE *file = NULL;

	// Check for access permissions in case we're sandbox'ed.
	if (access(directory, R_OK | X_OK) == 0 && (dp = opendir(directory)) != NULL) {
		// It's OK to use snprintf in this routine since it should only be called by the clients
		// of stack logging, and thus calls to malloc are OK.
		snprintf(prefix_and_pid, (size_t)PATH_MAX, "%s%d.", stack_log_file_base_name,
				pid); // make sure to use "%s%d." rather than just "%s%d" to match the whole pid
		size_t prefix_and_pid_length = strlen(prefix_and_pid);

		while ((entry = readdir(dp)) != NULL) {
			if (strncmp(entry->d_name, prefix_and_pid, prefix_and_pid_length) == 0) {
				snprintf(pathname, (size_t)PATH_MAX, "%s/%s", directory, entry->d_name);
				file = open_log_file_at_path(pathname, streams);
				break;
			}
		}
		closedir(dp);
	}

	return file;
}

// Read the launch data of the target process from the kernel to find the
// value of the environment variable named env_var_name.  Since this function
// uses alloca() to temporarily allocate space for data copied from the kernel,
// and we don't want to malloc space so that this can be called from malloc stack
// logging code in the target process as well, we copy the result into the
// env_var_value_buf of length max_path_len supplied by the caller.
static bool
getenv_from_process(pid_t pid, char *env_var_name, char *env_var_value_buf, size_t buf_length)
{
	env_var_value_buf[0] = '\0';

	// Just call getenv() if pid is the current process, partly to avoid the sysctl()
	// call which can cause system deadlock (<rdar://problem/14409213> "processes hang
	// if sandboxd is running with MallocStackLogging enabled").  But it probably
	// doesn't completely fix that since there is another sysctl() call in is_process_running()
	// when checking to see if the process corresponding to an existing stack log file
	// is still running.
	if (pid == getpid()) {
		char *env_var_value = getenv(env_var_name);
		if (!env_var_value) {
			return false;
		} else {
			strlcpy(env_var_value_buf, env_var_value, buf_length);
			return true;
		}
	}

	int mib[3];
	size_t argbufSize = 0; // Must initialize this to 0 so this works when compiled for x86_64.

	// First get the maximum arguments size, to determine the necessary buffer size.
	mib[0] = CTL_KERN;
	mib[1] = KERN_ARGMAX;

	size_t size = sizeof(argbufSize);
	int ret = sysctl(mib, 2, &argbufSize, &size, NULL, 0);
	if (ret != 0) {
		return false;
	}

	mib[0] = CTL_KERN;
	mib[1] = KERN_PROCARGS2; // The older KERN_PROCARGS is deprecated.
	mib[2] = pid;

	char *argbuf = (char *)alloca(argbufSize);
	ret = sysctl(mib, 3, argbuf, &argbufSize, (void *)NULL, 0);
	if (ret != 0) {
		return false;
	}
	argbuf[argbufSize - 1] = '\0'; // make sure the buffer is null-terminated
	char *p = argbuf;
	char *endp = &argbuf[argbufSize];

	// Skip over argc, which is always 4 bytes long (int-sized), even in 64-bit architectures.
	int argumentCount = *((int *)argbuf);
	p += sizeof(argumentCount);

	// Skip over arguments, using the argumentCount read from the start of argbuf.
	argumentCount++; // increment argumentCount to also skip saved exec path, which comes first
	for (int argumentNum = 0; argumentNum < argumentCount && p < endp; argumentNum++) {
		while (p < endp && *p != '\0')
			p++;
		while (p < endp && *p == '\0')
			p++; // saved exec path sometimes has multiple nul's
	}

	size_t env_var_name_length = strlen(env_var_name);

	// Examine environment variables.
	while ((p + env_var_name_length + 1) < endp && *p != '\0') {
		if (strncmp(p, env_var_name, env_var_name_length) == 0 && p[env_var_name_length] == '=') {
			p += env_var_name_length + 1;
			strlcpy(env_var_value_buf, p, buf_length);
			//_malloc_printf(ASL_LEVEL_INFO, "found env var %s='%s'\n", env_var_name, env_var_value_buf);
			return true;
		}
		while (p < endp && *p != '\0')
			p++;
		p++;
	}
	return false;
}

static FILE *
open_log_file(pid_t target_pid, remote_task_file_streams *streams)
{
	static bool already_reaped = false;
	if (!already_reaped) {
		// reap any left-over log files (for non-existent processes, but not for this analysis process)
		reap_orphaned_log_files(target_pid, streams);
		already_reaped = true;
	}
	
	if (streams->index_file_stream !=  NULL) { // reap_orphaned_log_files opened the file
		return streams->index_file_stream;
	}
	
	// Since we're searching for the log file here, not creating it, we can search in any order we want.
	// So look at MallocStackLoggingDirectory last since that is almost never set.
	FILE *file = open_log_file_from_directory(target_pid, _PATH_TMP, streams);
	if (!file) {
		char *env_var_names[] = {"TMPDIR", "MallocStackLoggingDirectory"};
		for (unsigned i = 0; i < sizeof(env_var_names) / sizeof(char *); i++) {
			char directory[PATH_MAX];
			bool success = getenv_from_process(target_pid, env_var_names[i], directory, sizeof(directory));
			if (success) {
				file = open_log_file_from_directory(target_pid, directory, streams);
				if (file) {
					break;
				}
			}
		}
	}
	return file;
}

// shared_memory_address is non-zero when in lite mode and this is called for the first time on a task
static remote_task_file_streams *
retain_file_streams_for_task(task_t task, vm_address_t shared_memory_address)
{
	if (task == MACH_PORT_NULL) {
		return NULL;
	}

	_malloc_lock_lock(&remote_fd_list_lock);

	// see if they're already in use
	uint32_t i = 0;
	for (i = 0; i < remote_task_fd_count; i++) {
		if (remote_fds[i].remote_task == task) {
			remote_fds[i].in_use_count++;
			_malloc_lock_unlock(&remote_fd_list_lock);
			return &remote_fds[i];
		}
	}

	// open them
	uint32_t failures = 0;
	if (remote_task_fd_count == STACK_LOGGING_MAX_SIMUL_REMOTE_TASKS_INSPECTED) {
		while (remote_fds[next_remote_task_fd].in_use_count > 0) {
			next_remote_task_fd++;
			if (next_remote_task_fd == STACK_LOGGING_MAX_SIMUL_REMOTE_TASKS_INSPECTED) {
				next_remote_task_fd = 0;
			}
			failures++;
			if (failures >= STACK_LOGGING_MAX_SIMUL_REMOTE_TASKS_INSPECTED) {
				_malloc_lock_unlock(&remote_fd_list_lock);
				return NULL;
			}
		}
		fclose(remote_fds[next_remote_task_fd].index_file_stream);
		destroy_cache_for_file_streams(&remote_fds[next_remote_task_fd]);
	}

	pid_t pid;
	kern_return_t err = pid_for_task(task, &pid);
	if (err != KERN_SUCCESS) {
		_malloc_lock_unlock(&remote_fd_list_lock);
		return NULL;
	}

	remote_task_file_streams *this_task_streams = &remote_fds[next_remote_task_fd];

	if (shared_memory_address != 0) {
		this_task_streams->remote_stack_buffer_shared_memory_address = shared_memory_address;
		this_task_streams->task_uses_lite_mode = true;
	} else {
		open_log_file(pid, this_task_streams);
		
		if (this_task_streams->index_file_stream == NULL) {
			_malloc_lock_unlock(&remote_fd_list_lock);
			return NULL;
		}
	}

	// check if target pid is running 64-bit
	int mib[] = {CTL_KERN, KERN_PROC, KERN_PROC_PID, pid};
	struct kinfo_proc processInfo;
	size_t bufsize = sizeof(processInfo);
	if (sysctl(mib, (unsigned)(sizeof(mib) / sizeof(int)), &processInfo, &bufsize, NULL, (size_t)0) == 0 && bufsize > 0) {
		this_task_streams->task_is_64_bit = processInfo.kp_proc.p_flag & P_LP64;
	} else {
		this_task_streams->task_is_64_bit = 0;
	}

	// otherwise set vars and go
	this_task_streams->in_use_count = 1;
	this_task_streams->remote_task = task;
	this_task_streams->remote_pid = pid;
	next_remote_task_fd++;
	if (next_remote_task_fd == STACK_LOGGING_MAX_SIMUL_REMOTE_TASKS_INSPECTED) {
		next_remote_task_fd = 0;
	}
	remote_task_fd_count = MIN(remote_task_fd_count + 1, STACK_LOGGING_MAX_SIMUL_REMOTE_TASKS_INSPECTED);

	_malloc_lock_unlock(&remote_fd_list_lock);
	return this_task_streams;
}

static void
release_file_streams_for_task(task_t task)
{
	_malloc_lock_lock(&remote_fd_list_lock);

	// decrement in-use count
	uint32_t i = 0;
	for (i = 0; i < remote_task_fd_count; i++) {
		if (remote_fds[i].remote_task == task) {
			remote_fds[i].in_use_count--;
			break;
		}
	}

	_malloc_lock_unlock(&remote_fd_list_lock);
}

#pragma mark - extern

kern_return_t
__mach_stack_logging_start_reading(task_t task, vm_address_t shared_memory_address, boolean_t *uses_lite_mode)
{
	remote_task_file_streams *remote_fd = retain_file_streams_for_task(task, shared_memory_address);
	if (remote_fd == NULL) {
		return KERN_FAILURE;
	}
	
	*uses_lite_mode = remote_fd->task_uses_lite_mode;
	
	return KERN_SUCCESS;
}

kern_return_t
__mach_stack_logging_stop_reading(task_t task)
{
	kern_return_t err = KERN_SUCCESS;
	
	release_file_streams_for_task(task);

	_malloc_lock_lock(&remote_fd_list_lock);
	
	for (uint32_t i = 0; i < remote_task_fd_count; i++) {
		if (remote_fds[i].remote_task == task) {
			if (remote_fds[i].in_use_count > 0) {
				// Hmm... the client is in the middle of a stack log reading call?
				err = KERN_FAILURE;
			} else {
				// remote_fds[i].in_use_count is 0 so don't decrement it!
				fclose(remote_fds[i].index_file_stream);
				destroy_cache_for_file_streams(&remote_fds[i]);
			}
			break;
		}
	}
	
	_malloc_lock_unlock(&remote_fd_list_lock);
	
	return err;
}

// This function is no longer used.  It was a hack that required an analysis tool process
// to read the target tasks's __stack_log_file_path__ variable then pass the value of
// that to this function.  This is now handled automatically all within this file, by
// having the stack log reading code read the environment variables of the target process.
// This function should be removed once no clients are calling it.
kern_return_t
__mach_stack_logging_set_file_path(task_t task, char *file_path)
{
	return KERN_SUCCESS;
}

kern_return_t
__mach_stack_logging_get_frames(task_t task,
		mach_vm_address_t address,
		mach_vm_address_t *stack_frames_buffer,
		uint32_t max_stack_frames,
		uint32_t *count)
{
	remote_task_file_streams *remote_fd = retain_file_streams_for_task(task, 0);
	if (remote_fd == NULL) {
		return KERN_FAILURE;
	}

	kern_return_t err = update_cache_for_file_streams(remote_fd);
	if (err != KERN_SUCCESS) {
		release_file_streams_for_task(task);
		return err;
	}

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
					target_64_index = (stack_logging_index_event64 *)(remote_fd->cache->snapshot.index_buffer +
																	  (located_file_position -
																			  remote_fd->cache->snapshot.start_index_offset));
					located_file_position = STACK_LOGGING_OFFSET(target_64_index->offset_and_flags);
				} else {
					target_32_index = (stack_logging_index_event32 *)(remote_fd->cache->snapshot.index_buffer +
																	  (located_file_position -
																			  remote_fd->cache->snapshot.start_index_offset));
					located_file_position = STACK_LOGGING_OFFSET(target_32_index->offset_and_flags);
				}
			} else {
				found = false;
			}

		} else {
			// it's written to disk
			char bufferSpace[128];

			size_t read_size =
					(remote_fd->task_is_64_bit ? sizeof(stack_logging_index_event64) : sizeof(stack_logging_index_event32));
			fseeko(remote_fd->index_file_stream, (off_t)located_file_position, SEEK_SET);
			size_t read_count = fread(bufferSpace, read_size, (size_t)1, remote_fd->index_file_stream);
			if (read_count) {
				if (remote_fd->task_is_64_bit) {
					target_64_index = (stack_logging_index_event64 *)bufferSpace;
					located_file_position = STACK_LOGGING_OFFSET(target_64_index->offset_and_flags);
				} else {
					target_32_index = (stack_logging_index_event32 *)bufferSpace;
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
__mach_stack_logging_enumerate_records(task_t task,
		mach_vm_address_t address,
		void enumerator(mach_stack_logging_record_t, void *),
		void *context)
{
	remote_task_file_streams *remote_fd = retain_file_streams_for_task(task, 0);
	if (remote_fd == NULL) {
		return KERN_FAILURE;
	}

	bool reading_all_addresses = (address == 0 ? true : false);
	mach_stack_logging_record_t pass_record;
	kern_return_t err = KERN_SUCCESS;

	// update (read index file once and only once)
	err = update_cache_for_file_streams(remote_fd);
	if (err != KERN_SUCCESS) {
		release_file_streams_for_task(task);
		return err;
	}

	FILE *the_index = (remote_fd->index_file_stream);

	// prepare for the read; target process could be 32 or 64 bit.
	char bufferSpace[2048]; // 2 kb
	stack_logging_index_event32 *target_32_index = (stack_logging_index_event32 *)bufferSpace;
	stack_logging_index_event64 *target_64_index = (stack_logging_index_event64 *)bufferSpace;
	uint32_t target_addr_32 = (uint32_t)STACK_LOGGING_DISGUISE((uint32_t)address);
	uint64_t target_addr_64 = STACK_LOGGING_DISGUISE((uint64_t)address);
	size_t read_size = (remote_fd->task_is_64_bit ? sizeof(stack_logging_index_event64) : sizeof(stack_logging_index_event32));
	size_t number_slots = (size_t)(2048 / read_size);
	uint64_t total_slots = remote_fd->cache->last_index_file_offset / read_size;

	// perform the search
	size_t read_count = 0;
	int64_t current_file_offset = 0;
	uint32_t i;
	do {
		// at this point, we need to read index events; read them from the file until it's necessary to grab them from the shared
		// memory snapshot
		// and crop file reading to the point where we last scanned
		number_slots = (size_t)MIN(number_slots, total_slots);

		// if out of file to read (as of the time we entered this function), try to use shared memory snapshot
		if (number_slots == 0) {
			if (remote_fd->cache->shmem &&
					remote_fd->cache->snapshot.start_index_offset + remote_fd->cache->snapshot.next_free_index_buffer_offset >
							(uint64_t)current_file_offset) {
				// use shared memory
				target_32_index = (stack_logging_index_event32 *)remote_fd->cache->snapshot.index_buffer;
				target_64_index = (stack_logging_index_event64 *)remote_fd->cache->snapshot.index_buffer;
				read_count = (uint32_t)(remote_fd->cache->snapshot.start_index_offset +
										remote_fd->cache->snapshot.next_free_index_buffer_offset - current_file_offset) /
							 read_size;
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
					pass_record.type_flags = STACK_LOGGING_FLAGS_AND_USER_TAG(target_64_index[i].offset_and_flags);
					enumerator(pass_record, context);
				}
			}
		} else {
			for (i = 0; i < read_count; i++) {
				if (reading_all_addresses || target_32_index[i].address == target_addr_32) {
					pass_record.address = STACK_LOGGING_DISGUISE(target_32_index[i].address);
					pass_record.argument = target_32_index[i].argument;
					pass_record.stack_identifier = STACK_LOGGING_OFFSET(target_32_index[i].offset_and_flags);
					pass_record.type_flags = STACK_LOGGING_FLAGS_AND_USER_TAG(target_32_index[i].offset_and_flags);
					enumerator(pass_record, context);
				}
			}
		}
	} while (read_count);

	release_file_streams_for_task(task);
	return err;
}

kern_return_t
__mach_stack_logging_frames_for_uniqued_stack(task_t task,
		uint64_t stack_identifier,
		mach_vm_address_t *stack_frames_buffer,
		uint32_t max_stack_frames,
		uint32_t *count)
{
	remote_task_file_streams *remote_fd = retain_file_streams_for_task(task, 0);
	if (remote_fd == NULL) {
		return KERN_FAILURE;
	}
	
	// ensure that the uniquing table snapshot is valid
	kern_return_t err = update_cache_for_file_streams(remote_fd);
	if (err != KERN_SUCCESS) {
		release_file_streams_for_task(task);
		return err;
	}
	
	bool lite_mode = remote_fd->cache->lite_mode;
	
	unwind_stack_from_table_index(&remote_fd->cache->uniquing_table_snapshot, stack_identifier, stack_frames_buffer, count, max_stack_frames, lite_mode);

	release_file_streams_for_task(task);

	if (*count) {
		return KERN_SUCCESS;
	} else {
		return KERN_FAILURE;
	}
}

kern_return_t
__attribute__((visibility("default")))
__mach_stack_logging_uniquing_table_read_stack(struct backtrace_uniquing_table *uniquing_table,
											   uint64_t stackid,
											   mach_vm_address_t *out_frames_buffer,
											   uint32_t *out_frames_count,
											   uint32_t max_frames)
{
	unwind_stack_from_table_index(uniquing_table, stackid, out_frames_buffer, out_frames_count, max_frames, uniquing_table->
								  nodes_use_refcount);
	return *out_frames_count ? KERN_SUCCESS : KERN_FAILURE;
}


struct backtrace_uniquing_table *
__mach_stack_logging_copy_uniquing_table(task_t task)
{
	remote_task_file_streams *remote_fd = retain_file_streams_for_task(task, 0);
	if (remote_fd == NULL) {
		return NULL;
	}

	// ensure that the uniquing table snapshot is valid
	kern_return_t err = update_cache_for_file_streams(remote_fd);
	if (err != KERN_SUCCESS || remote_fds->cache->uniquing_table_snapshot.numPages == 0) {
		release_file_streams_for_task(task);
		return NULL;
	}

	/* Steal the uniqing table snapshot.   A new snapshot will be taken next time someone calls
	 * update_cache_for_file_streams
	 */
	backtrace_uniquing_table *table = malloc(sizeof(backtrace_uniquing_table));
	memcpy(table, &remote_fds->cache->uniquing_table_snapshot, sizeof(backtrace_uniquing_table));
	bzero(&remote_fds->cache->uniquing_table_snapshot, sizeof(backtrace_uniquing_table));
	remote_fds->cache->uniquing_table_snapshot.in_client_process = true;

	table->refcount = 1;

	release_file_streams_for_task(task);

	return table;
}


void
__mach_stack_logging_uniquing_table_release(struct backtrace_uniquing_table *table)
{
	if (!table) {
		return;
	}
	assert(table->refcount > 0);
	table->refcount--;
	if (table->refcount == 0) {
		free_uniquing_table_chunks(table);
		free(table);
	}
}

void
__mach_stack_logging_uniquing_table_retain(struct backtrace_uniquing_table *table)
{
	assert(table->refcount > 0);
	table->refcount++;
}

static const size_t uniquingTableDataAlign = 16 * 1024;
static const size_t uniquingTableHeaderLength = 16;

static inline size_t roundUp(size_t x, size_t alignment)
{
	return x + (-x % alignment);
}

size_t
__mach_stack_logging_uniquing_table_sizeof(struct backtrace_uniquing_table *table)
{
	size_t size = 0;
	size += uniquingTableHeaderLength; //header
	size += sizeof(backtrace_uniquing_table);
	size = roundUp(size, uniquingTableDataAlign);
	assert(table->in_client_process);
	table_chunk_header_t *table_chunk_header = table->u.first_table_chunk_hdr;
	while (table_chunk_header) {
		size += 2 * sizeof(mach_vm_address_t) * table_chunk_header->num_nodes_in_chunk;
		table_chunk_header = table_chunk_header->next_table_chunk_header;
	}
	return size;
}


void *
__mach_stack_logging_uniquing_table_serialize(struct backtrace_uniquing_table *table, mach_vm_size_t *size)
{
	*size = __mach_stack_logging_uniquing_table_sizeof(table);

	mach_vm_address_t buffer_address = 0;
	kern_return_t kr = mach_vm_allocate(mach_task_self(), &buffer_address, *size, VM_FLAGS_ANYWHERE);
	if (kr != KERN_SUCCESS) {
		*size = 0;
		return NULL;
	}

	void *buffer = (void*)buffer_address;

	uint8_t *p = buffer;

	memcpy(p, "MslUniquingTable", uniquingTableHeaderLength);
	p += uniquingTableHeaderLength;

	memcpy(p, table, sizeof(backtrace_uniquing_table));
	p += sizeof(backtrace_uniquing_table);

	p = ((uint8_t*)buffer) + roundUp(p - (uint8_t*)buffer, uniquingTableDataAlign);

	table_chunk_header_t *table_chunk_header = table->u.first_table_chunk_hdr;
	uint64_t num_nodes = 0;
	while (table_chunk_header) {
		num_nodes += table_chunk_header->num_nodes_in_chunk;
		size_t chunk_size = 2 * sizeof(mach_vm_address_t) * table_chunk_header->num_nodes_in_chunk;
		kr = mach_vm_copy(mach_task_self(), (mach_vm_address_t)table_chunk_header->table_chunk, chunk_size, (vm_address_t)p);
		if (kr != KERN_SUCCESS) {
			memcpy(p, table_chunk_header->table_chunk, chunk_size);
		}
		p += chunk_size;
		table_chunk_header = table_chunk_header->next_table_chunk_header;
	}

	assert(num_nodes == table->numNodes);

	return buffer;
}

struct backtrace_uniquing_table *
__mach_stack_logging_uniquing_table_copy_from_serialized(void *buffer, size_t size)
{
	if (size < uniquingTableHeaderLength + sizeof(backtrace_uniquing_table)) {
		return NULL;
	}
	uint8_t *p = buffer;

	if (strncmp(buffer, "MslUniquingTable", uniquingTableHeaderLength) != 0) {
		return NULL;
	}
	p += uniquingTableHeaderLength;

	backtrace_uniquing_table *table = malloc(sizeof(backtrace_uniquing_table));
	memcpy(table, p, sizeof(backtrace_uniquing_table));
	p += sizeof(backtrace_uniquing_table);

	p = ((uint8_t*)buffer) + roundUp(p - (uint8_t*)buffer, uniquingTableDataAlign);

	table->u.first_table_chunk_hdr = malloc(sizeof(table_chunk_header_t));
	table->refcount = 1;

	mach_vm_size_t chunkSize = 2 * table->numNodes * sizeof(mach_vm_address_t);

	mach_vm_address_t chunkAddr = 0;

	if (roundUp(uniquingTableHeaderLength + sizeof(backtrace_uniquing_table), uniquingTableDataAlign) + chunkSize < size ) {
		goto fail;
	}

	kern_return_t kr = mach_vm_allocate(mach_task_self(), &chunkAddr, chunkSize, VM_FLAGS_ANYWHERE | VM_MAKE_TAG(VM_MEMORY_ANALYSIS_TOOL));
	if (kr != KERN_SUCCESS) {
		goto fail;
	}

	table->u.first_table_chunk_hdr->num_nodes_in_chunk = table->numNodes;
	table->u.first_table_chunk_hdr->table_chunk_size = chunkSize;
	table->u.first_table_chunk_hdr->table_chunk = (mach_vm_address_t*) chunkAddr;
	table->u.first_table_chunk_hdr->next_table_chunk_header = NULL;

	kr = mach_vm_copy(mach_task_self(), (mach_vm_address_t)p, chunkSize, (mach_vm_address_t) table->u.first_table_chunk_hdr->table_chunk);
	if (kr != KERN_SUCCESS) {
		goto fail;
	}

	return table;

fail:
	if (table) {
		if (table->u.first_table_chunk_hdr) {
			free(table->u.first_table_chunk_hdr);
		}
		free(table);
	}
	if (chunkAddr) {
		mach_vm_deallocate(mach_task_self(), chunkAddr, chunkSize);
	}
	return NULL;
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
	fprintf(stderr, "sizeof pre_write_buffers: %lu\n", sizeof(pre_write_buffers));
	total_globals += sizeof(pre_write_buffers);
	fprintf(stderr, "sizeof stack_buffer: %lu\n", sizeof(stack_buffer));
	total_globals += sizeof(stack_buffer);
	fprintf(stderr, "sizeof last_logged_malloc_address: %lu\n", sizeof(last_logged_malloc_address));
	total_globals += sizeof(last_logged_malloc_address);
	fprintf(stderr, "sizeof stack_log_file_base_name: %lu\n", sizeof(stack_log_file_base_name));
	total_globals += sizeof(stack_log_file_base_name);
	fprintf(stderr, "sizeof stack_log_file_suffix: %lu\n", sizeof(stack_log_file_suffix));
	total_globals += sizeof(stack_log_file_suffix);
	fprintf(stderr, "sizeof __stack_log_file_path__ (index_file_path): %lu\n", (size_t)PATH_MAX);
	total_globals += (size_t)PATH_MAX;
	fprintf(stderr, "sizeof index_file_descriptor: %lu\n", sizeof(index_file_descriptor));
	total_globals += sizeof(index_file_descriptor);
	fprintf(stderr, "sizeof remote_fds: %lu\n", sizeof(remote_fds));
	total_globals += sizeof(remote_fds);
	fprintf(stderr, "sizeof next_remote_task_fd: %lu\n", sizeof(next_remote_task_fd));
	total_globals += sizeof(next_remote_task_fd);
	fprintf(stderr, "sizeof remote_task_fd_count: %lu\n", sizeof(remote_task_fd_count));
	total_globals += sizeof(remote_task_fd_count);
	fprintf(stderr, "sizeof remote_fd_list_lock: %lu\n", sizeof(remote_fd_list_lock));
	total_globals += sizeof(remote_fd_list_lock);
	fprintf(stderr, "sizeof logging_use_compaction: %lu\n", sizeof(logging_use_compaction));
	total_globals += sizeof(logging_use_compaction);

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

/* vim: set noet:ts=4:sw=4:cindent: */
