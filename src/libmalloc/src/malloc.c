/*
 * Copyright (c) 1999, 2000, 2003, 2005, 2008, 2012 Apple Inc. All rights reserved.
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

#include "magmallocProvider.h"

#include <_simple.h>
#include <crt_externs.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libkern/OSAtomic.h>
#include <mach/mach_vm.h>
#include <mach/mach_init.h>
#include <mach/thread_switch.h>
#include <mach/vm_map.h>
#include <mach-o/dyld.h>
#include <os/tsd.h>
#include <sys/mman.h>
#include <xlocale.h>
#include <TargetConditionals.h>

#include "malloc.h"
#include "malloc_printf.h"
#include "scalable_malloc.h"
#include "malloc_internal.h"
#include "stack_logging.h"

#if TARGET_OS_EMBEDDED || TARGET_IPHONE_SIMULATOR
// _malloc_printf(ASL_LEVEL_INFO...) on iOS doesn't show up in the Xcode Console log of the device,
// but ASL_LEVEL_NOTICE does.  So raising the log level is helpful.
#undef ASL_LEVEL_INFO
#define ASL_LEVEL_INFO ASL_LEVEL_NOTICE
#endif

#include <CrashReporterClient.h>

#ifdef __LP64__
#define CONFIG_NANOZONE 1
#else
#define CONFIG_NANOZONE 0
#endif

/*
 * MALLOC_ABSOLUTE_MAX_SIZE - There are many instances of addition to a
 * user-specified size_t, which can cause overflow (and subsequent crashes)
 * for values near SIZE_T_MAX.  Rather than add extra "if" checks everywhere
 * this occurs, it is easier to just set an absolute maximum request size,
 * and immediately return an error if the requested size exceeds this maximum.
 * Of course, values less than this absolute max can fail later if the value
 * is still too large for the available memory.  The largest value added
 * seems to be PAGE_SIZE (in the macro round_page()), so to be safe, we set
 * the maximum to be 2 * PAGE_SIZE less than SIZE_T_MAX.
 */
#define MALLOC_ABSOLUTE_MAX_SIZE	(SIZE_T_MAX - (2 * PAGE_SIZE))

#define USE_SLEEP_RATHER_THAN_ABORT	0

typedef void (malloc_logger_t)(uint32_t type, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t result, uint32_t num_hot_frames_to_skip);

extern malloc_logger_t *__syscall_logger;	// use this to set up syscall logging (e.g., vm_allocate, vm_deallocate, mmap, munmap)

extern void __prepare_to_log_stacks(void);

static _malloc_lock_s _malloc_lock = _MALLOC_LOCK_INIT;
#define MALLOC_LOCK()		_malloc_lock_lock(&_malloc_lock)
#define MALLOC_UNLOCK()		_malloc_lock_unlock(&_malloc_lock)

static inline void yield(void) {
	thread_switch(MACH_PORT_NULL, SWITCH_OPTION_DEPRESS, 1);
}

/* The following variables are exported for the benefit of performance tools
 *
 * It should always be safe to first read malloc_num_zones, then read
 * malloc_zones without taking the lock, if only iteration is required and
 * provided that when malloc_destroy_zone is called all prior operations on that
 * zone are complete and no further calls referencing that zone can be made.
 */
unsigned malloc_num_zones = 0;
unsigned malloc_num_zones_allocated = 0;
malloc_zone_t **malloc_zones = 0;
malloc_logger_t *malloc_logger = NULL;

unsigned malloc_debug_flags = 0;

unsigned malloc_check_start = 0; // 0 means don't check
unsigned malloc_check_counter = 0;
unsigned malloc_check_each = 1000;

/* global flag to suppress ASL logging e.g. for syslogd */
int _malloc_no_asl_log = 0;

static int malloc_check_sleep = 100; // default 100 second sleep
static int malloc_check_abort = 0; // default is to sleep, not abort

static int malloc_debug_file = STDERR_FILENO;
static boolean_t _malloc_is_initialized = FALSE;

static const char Malloc_Facility[] = "com.apple.Libsystem.malloc";

/*
 * Counters that coordinate zone destruction (in malloc_zone_unregister) with
 * find_registered_zone (here abbreviated as FRZ).
 */
static int counterAlice = 0, counterBob = 0;
static int *pFRZCounterLive= &counterAlice, *pFRZCounterDrain = &counterBob;

#define MALLOC_LOG_TYPE_ALLOCATE	stack_logging_type_alloc
#define MALLOC_LOG_TYPE_DEALLOCATE	stack_logging_type_dealloc
#define MALLOC_LOG_TYPE_HAS_ZONE	stack_logging_flag_zone
#define MALLOC_LOG_TYPE_CLEARED		stack_logging_flag_cleared

#define DEFAULT_MALLOC_ZONE_STRING "DefaultMallocZone"
#define DEFAULT_PUREGEABLE_ZONE_STRING "DefaultPurgeableMallocZone"

boolean_t malloc_engaged_nano(void);
#if CONFIG_NANOZONE
extern boolean_t _malloc_engaged_nano;
malloc_zone_t *create_nano_zone(size_t initial_size, malloc_zone_t *helper_zone, unsigned debug_flags);
void nano_forked_zone(malloc_zone_t *zone);
#define MALLOC_HELPER_ZONE_STRING "MallocHelperZone"
#endif

/*********	Utilities	************/
__attribute__((visibility("hidden"))) uint64_t malloc_entropy[2] = {0, 0};

void __malloc_init(const char *apple[]);

static int
__entropy_from_kernel(const char *str)
{
	unsigned long long val;
	char tmp[20], *p;
	int idx = 0;

	/* Skip over key to the first value */
	str = strchr(str, '=');
	if (str == NULL)
		return 0;
	str++;

	while (str && idx < sizeof(malloc_entropy)/sizeof(malloc_entropy[0])) {
		strlcpy(tmp, str, 20);
		p = strchr(tmp, ',');
		if (p) *p = '\0';
		val = strtoull_l(tmp, NULL, 0, NULL);
		malloc_entropy[idx] = (uint64_t)val;
		idx++;
		if ((str = strchr(str, ',')) != NULL)
			str++;
	}
	return idx;
}

/* TODO: Remove __malloc_entropy_setup, it was left to avoid rev-lock */
void
__malloc_entropy_setup(const char *apple[]) {
	__malloc_init(apple);
}

/* TODO: Investigate adding _malloc_initialize() into this libSystem initializer */
void
__malloc_init(const char *apple[])
{
	const char **p;
#if CONFIG_NANOZONE
	_malloc_engaged_nano = 0;
	for (p = apple; p && *p; p++) {
		if (0 == strncmp(*p, "MallocNanoZone=1", strlen("MallocNanoZone=1"))) {
			// _malloc_printf(ASL_LEVEL_INFO, "MallocNanoZone=1\n");
			_malloc_engaged_nano = 1;
			break;
		}
	}
#endif

	for (p = apple; p && *p; p++) {
		if (strstr(*p, "malloc_entropy") == *p) {
			int count = __entropy_from_kernel(*p);
			bzero((void*)*p, strlen(*p));

			if (sizeof(malloc_entropy)/sizeof(malloc_entropy[0]) == count) {
				return;
			}
			break;
		}
	}

	malloc_entropy[0] = ((uint64_t)arc4random()) << 32 | ((uint64_t)arc4random());
	malloc_entropy[1] = ((uint64_t)arc4random()) << 32 | ((uint64_t)arc4random());
	return;
}

static inline malloc_zone_t * find_registered_zone(const void *, size_t *) __attribute__((always_inline));
static inline malloc_zone_t *
find_registered_zone(const void *ptr, size_t *returned_size) {
	// Returns a zone which contains ptr, else NULL

	if (0 == malloc_num_zones) {
		if (returned_size) *returned_size = 0;
		return NULL;
	}

	// The default zone is registered in malloc_zones[0]. There's no danger that it will ever be unregistered.
	// So don't advance the FRZ counter yet.
	malloc_zone_t *zone = malloc_zones[0];
	size_t size = zone->size(zone, ptr);
	if (size) { // Claimed by this zone?
		if (returned_size) *returned_size = size;
		return zone;
	}

	int *pFRZCounter = pFRZCounterLive; // Capture pointer to the counter of the moment
	__sync_fetch_and_add(pFRZCounter, 1); // Advance this counter -- our thread is in FRZ

	unsigned		index;
	unsigned		limit = malloc_num_zones;
	malloc_zone_t	**zones = &malloc_zones[1];

	for (index = 1; index < limit; ++index, ++zones) {
		zone = *zones;
		size = zone->size(zone, ptr);
		if (size) { // Claimed by this zone?
			if (returned_size) *returned_size = size;
			__sync_fetch_and_sub(pFRZCounter, 1); // our thread is leaving FRZ
			return zone;
		}
	}
	// Unclaimed by any zone.
	if (returned_size) *returned_size = 0;
	__sync_fetch_and_sub(pFRZCounter, 1); // our thread is leaving FRZ
	return NULL;
}

__attribute__((visibility("hidden"))) __attribute__((noinline)) void
malloc_error_break(void) {
	// Provides a non-inlined place for various malloc error procedures to call
	// that will be called after an error message appears.  It does not make
	// sense for developers to call this function, so it is marked
	// hidden to prevent it from becoming API.
	MAGMALLOC_MALLOCERRORBREAK(); // DTrace USDT probe
}

extern boolean_t __stack_logging_locked();

__attribute__((visibility("hidden"))) __attribute__((noinline)) __attribute__((used)) int
malloc_gdb_po_unsafe(void) {
	// In order to implement "po" other data formatters in gdb, the debugger
	// calls functions that call malloc.  The debugger will  only run one thread
	// of the program in this case, so if another thread is holding a zone lock,
	// gdb may deadlock in this case.
	//
	// Iterate over the zones in malloc_zones, and call "trylock" on the zone
	// lock.  If trylock succeeds, unlock it, otherwise return "locked".  Returns
	// 0 == safe, 1 == locked/unsafe.

	if (__stack_logging_locked())
		return 1;

	malloc_zone_t **zones = malloc_zones;
	unsigned i, e = malloc_num_zones;

	for (i = 0; i != e; ++i) {
		malloc_zone_t *zone = zones[i];

		// Version must be >= 5 to look at the new introspection field.
		if (zone->version < 5)
			continue;

		if (zone->introspect->zone_locked && zone->introspect->zone_locked(zone))
			return 1;
	}
	return 0;
}

/*********	Creation and destruction	************/

static void set_flags_from_environment(void);

static void
malloc_zone_register_while_locked(malloc_zone_t *zone) {
	size_t protect_size;
	unsigned i;

	/* scan the list of zones, to see if this zone is already registered.  If
	 * so, print an error message and return. */
	for (i = 0; i != malloc_num_zones; ++i)
		if (zone == malloc_zones[i]) {
			_malloc_printf(ASL_LEVEL_ERR, "Attempted to register zone more than once: %p\n", zone);
			return;
		}

	if (malloc_num_zones == malloc_num_zones_allocated) {
		size_t malloc_zones_size = malloc_num_zones * sizeof(malloc_zone_t *);
		mach_vm_size_t alloc_size = round_page(malloc_zones_size + vm_page_size);
		mach_vm_address_t vm_addr;
		int alloc_flags = VM_FLAGS_ANYWHERE | VM_MAKE_TAG(VM_MEMORY_MALLOC);

		vm_addr = vm_page_size;
		kern_return_t kr = mach_vm_allocate(mach_task_self(), &vm_addr, alloc_size, alloc_flags);
		if (kr) {
			_malloc_printf(ASL_LEVEL_ERR, "malloc_zone_register allocation failed: %d\n", kr);
			return;
		}

		malloc_zone_t **new_zones = (uintptr_t)vm_addr;
		/* If there were previously allocated malloc zones, we need to copy them
		 * out of the previous array and into the new zones array */
		if (malloc_zones) {
			memcpy(new_zones, malloc_zones, malloc_zones_size);
			vm_addr = malloc_zones;
			mach_vm_size_t dealloc_size = round_page(malloc_zones_size);
			mach_vm_deallocate(mach_task_self(), vm_addr, dealloc_size);
		}

		/* Update the malloc_zones pointer, which we leak if it was previously
		 * allocated, and the number of zones allocated */
		protect_size = alloc_size;
		malloc_zones = new_zones;
		malloc_num_zones_allocated = alloc_size / sizeof(malloc_zone_t *);
	} else {
		/* If we don't need to reallocate zones, we need to briefly change the
		 * page protection the malloc zones to allow writes */
		protect_size = malloc_num_zones_allocated * sizeof(malloc_zone_t *);
		mprotect(malloc_zones, protect_size, PROT_READ | PROT_WRITE);
	}
	malloc_zones[malloc_num_zones++] = zone;

	/* Finally, now that the zone is registered, disallow write access to the
	 * malloc_zones array */
	mprotect(malloc_zones, protect_size, PROT_READ);
	//_malloc_printf(ASL_LEVEL_INFO, "Registered malloc_zone %p in malloc_zones %p [%u zones, %u bytes]\n", zone, malloc_zones, malloc_num_zones, protect_size);
}

static void
_malloc_initialize(void) {
	MALLOC_LOCK();
	if (!_malloc_is_initialized) {
		unsigned n;
		malloc_zone_t	*zone;

		_malloc_is_initialized = TRUE;

		set_flags_from_environment(); // will only set flags up to two times
		n = malloc_num_zones;

#if CONFIG_NANOZONE
		malloc_zone_t *helper_zone = create_scalable_zone(0, malloc_debug_flags);
		zone = create_nano_zone(0, helper_zone, malloc_debug_flags);

		if (zone) {
			malloc_zone_register_while_locked(zone);
			malloc_zone_register_while_locked(helper_zone);

			// Must call malloc_set_zone_name() *after* helper and nano are hooked together.
			malloc_set_zone_name(zone, DEFAULT_MALLOC_ZONE_STRING);
			malloc_set_zone_name(helper_zone, MALLOC_HELPER_ZONE_STRING);
		} else {
			zone = helper_zone;
			malloc_zone_register_while_locked(zone);
			malloc_set_zone_name(zone, DEFAULT_MALLOC_ZONE_STRING);
		}
#else
		zone = create_scalable_zone(0, malloc_debug_flags);
		malloc_zone_register_while_locked(zone);
		malloc_set_zone_name(zone, DEFAULT_MALLOC_ZONE_STRING);
#endif

		if (n != 0) { // make the default first, for efficiency
			unsigned protect_size = malloc_num_zones_allocated * sizeof(malloc_zone_t *);
			malloc_zone_t *hold = malloc_zones[0];

			if(hold->zone_name && strcmp(hold->zone_name, DEFAULT_MALLOC_ZONE_STRING)  == 0) {
				malloc_set_zone_name(hold, NULL);
			}

			mprotect(malloc_zones, protect_size, PROT_READ | PROT_WRITE);
			malloc_zones[0] = malloc_zones[n];
			malloc_zones[n] = hold;
			mprotect(malloc_zones, protect_size, PROT_READ);
		}
		// _malloc_printf(ASL_LEVEL_INFO, "%d registered zones\n", malloc_num_zones);
		// _malloc_printf(ASL_LEVEL_INFO, "malloc_zones is at %p; malloc_num_zones is at %p\n", (unsigned)&malloc_zones, (unsigned)&malloc_num_zones);
	}
	MALLOC_UNLOCK();
}

static inline malloc_zone_t *inline_malloc_default_zone(void) __attribute__((always_inline));
static inline malloc_zone_t *
inline_malloc_default_zone(void) {
	if (!_malloc_is_initialized) _malloc_initialize();
	// _malloc_printf(ASL_LEVEL_INFO, "In inline_malloc_default_zone with %d %d\n", malloc_num_zones, malloc_has_debug_zone);
	return malloc_zones[0];
}

malloc_zone_t *
malloc_default_zone(void) {
	return inline_malloc_default_zone();
}

static inline malloc_zone_t *inline_malloc_default_scalable_zone(void) __attribute__((always_inline));
static inline malloc_zone_t *
inline_malloc_default_scalable_zone(void) {
	unsigned index;

	if (!_malloc_is_initialized) _malloc_initialize();
	// _malloc_printf(ASL_LEVEL_INFO, "In inline_malloc_default_scalable_zone with %d %d\n", malloc_num_zones, malloc_has_debug_zone);

	MALLOC_LOCK();
#if CONFIG_NANOZONE
	for (index = 0; index < malloc_num_zones; ++index) {
		malloc_zone_t *z = malloc_zones[index];

		if(z->zone_name && strcmp(z->zone_name, MALLOC_HELPER_ZONE_STRING)  == 0) {
			MALLOC_UNLOCK();
			return z;
		}
	}
#endif
	for (index = 0; index < malloc_num_zones; ++index) {
		malloc_zone_t *z = malloc_zones[index];

		if(z->zone_name && strcmp(z->zone_name, DEFAULT_MALLOC_ZONE_STRING)  == 0) {
			MALLOC_UNLOCK();
			return z;
		}
	}
	MALLOC_UNLOCK();

	malloc_printf("*** malloc_default_scalable_zone() failed to find 'DefaultMallocZone'\n");
	return NULL; // FIXME: abort() instead?
}

/*
 * malloc_engaged_nano() is for the benefit of libdispatch, which calls here just once.
 */
boolean_t malloc_engaged_nano(void)
{
#if CONFIG_NANOZONE
	return _malloc_engaged_nano;
#else
	return 0;
#endif
}

malloc_zone_t *
malloc_default_purgeable_zone(void) {
	static malloc_zone_t *dpz;

	if (!dpz) {
		//
		// PR_7288598: Must pass a *scalable* zone (szone) as the helper for create_purgeable_zone().
		// Take care that the zone so obtained is not subject to interposing.
		//
		malloc_zone_t *tmp = create_purgeable_zone(0, inline_malloc_default_scalable_zone(), malloc_debug_flags);
		malloc_zone_register(tmp);
		malloc_set_zone_name(tmp, DEFAULT_PUREGEABLE_ZONE_STRING);
		if (!__sync_bool_compare_and_swap(&dpz, NULL, tmp))
			malloc_destroy_zone(tmp);
	}
	return dpz;
}

static void
set_flags_from_environment(void) {
	const char	*flag;
	int		fd;
	char	**env = * _NSGetEnviron();
	char	**p;
	char	*c;

	if (malloc_debug_file != STDERR_FILENO) {
		close(malloc_debug_file);
		malloc_debug_file = STDERR_FILENO;
	}
#if __LP64__
	malloc_debug_flags = SCALABLE_MALLOC_ABORT_ON_CORRUPTION; // Set always on 64-bit processes
#else
	int libSystemVersion  = NSVersionOfLinkTimeLibrary("System");
	if ((-1 != libSystemVersion) && ((libSystemVersion >> 16) < 126) /* Lion or greater */)
		malloc_debug_flags = 0;
	else
		malloc_debug_flags = SCALABLE_MALLOC_ABORT_ON_CORRUPTION;
#endif
	stack_logging_enable_logging = 0;
	stack_logging_dontcompact = 0;
	malloc_logger = NULL;
	malloc_check_start = 0;
	malloc_check_each = 1000;
	malloc_check_abort = 0;
	malloc_check_sleep = 100;
	/*
	 * Given that all environment variables start with "Malloc" we optimize by scanning quickly
	 * first the environment, therefore avoiding repeated calls to getenv().
	 * If we are setu/gid these flags are ignored to prevent a malicious invoker from changing
	 * our behaviour.
	 */
	for (p = env; (c = *p) != NULL; ++p) {
		if (!strncmp(c, "Malloc", 6)) {
			if (issetugid())
				return;
			break;
		}
	}
	if (c == NULL)
		return;
	flag = getenv("MallocLogFile");
	if (flag) {
		fd = open(flag, O_WRONLY|O_APPEND|O_CREAT, 0644);
		if (fd >= 0) {
			malloc_debug_file = fd;
			fcntl(fd, F_SETFD, 0); // clear close-on-exec flag  XXX why?
		} else {
			malloc_printf("Could not open %s, using stderr\n", flag);
		}
	}
	if (getenv("MallocGuardEdges")) {
		malloc_debug_flags |= SCALABLE_MALLOC_ADD_GUARD_PAGES;
		_malloc_printf(ASL_LEVEL_INFO, "protecting edges\n");
		if (getenv("MallocDoNotProtectPrelude")) {
			malloc_debug_flags |= SCALABLE_MALLOC_DONT_PROTECT_PRELUDE;
			_malloc_printf(ASL_LEVEL_INFO, "... but not protecting prelude guard page\n");
		}
		if (getenv("MallocDoNotProtectPostlude")) {
			malloc_debug_flags |= SCALABLE_MALLOC_DONT_PROTECT_POSTLUDE;
			_malloc_printf(ASL_LEVEL_INFO, "... but not protecting postlude guard page\n");
		}
	}
	flag = getenv("MallocStackLogging");
	if (!flag) {
		flag = getenv("MallocStackLoggingNoCompact");
		stack_logging_dontcompact = 1;
	}
	if (flag) {
		// Set up stack logging as early as possible to catch all ensuing VM allocations,
		// including those from _malloc_printf and malloc zone setup.  Make sure to set
		// __syscall_logger after this, because prepare_to_log_stacks() itself makes VM
		// allocations that we aren't prepared to log yet.
		__prepare_to_log_stacks();
		if (strcmp(flag,"malloc") == 0) {
			malloc_logger = __disk_stack_logging_log_stack;
			_malloc_printf(ASL_LEVEL_INFO, "recording malloc (but not VM allocation) stacks to disk using standard recorder\n");
		} else if (strcmp(flag,"vm") == 0) {
			__syscall_logger = __disk_stack_logging_log_stack;
			_malloc_printf(ASL_LEVEL_INFO, "recording VM allocation (but not malloc) stacks to disk using standard recorder\n");
		} else {
			malloc_logger = __disk_stack_logging_log_stack;
			__syscall_logger = __disk_stack_logging_log_stack;
			_malloc_printf(ASL_LEVEL_INFO, "recording malloc and VM allocation stacks to disk using standard recorder\n");
		}
		stack_logging_enable_logging = 1;
		if (stack_logging_dontcompact) {
			if (malloc_logger == __disk_stack_logging_log_stack) {
				_malloc_printf(ASL_LEVEL_INFO, "stack logging compaction turned off; size of log files on disk can increase rapidly\n");
			} else {
				_malloc_printf(ASL_LEVEL_INFO, "stack logging compaction turned off; VM can increase rapidly\n");
			}
		}
	}
	if (getenv("MallocScribble")) {
		malloc_debug_flags |= SCALABLE_MALLOC_DO_SCRIBBLE;
		_malloc_printf(ASL_LEVEL_INFO, "enabling scribbling to detect mods to free blocks\n");
	}
	if (getenv("MallocErrorAbort")) {
		malloc_debug_flags |= SCALABLE_MALLOC_ABORT_ON_ERROR;
		_malloc_printf(ASL_LEVEL_INFO, "enabling abort() on bad malloc or free\n");
	}
#if CONFIG_NANOZONE
	/* Explicit overrides from the environment */
	if ((flag = getenv("MallocNanoZone"))) {
		if (flag[0] == '1') {
			_malloc_engaged_nano = 1;
		} else if (flag[0] == '0') {
			_malloc_engaged_nano = 0;
		}
	}
#endif /* CONFIG_NANOZONE */

#if __LP64__
	/* initialization above forces SCALABLE_MALLOC_ABORT_ON_CORRUPTION of 64-bit processes */
#else
	flag = getenv("MallocCorruptionAbort");
	if (flag && (flag[0] == '0')) { // Set from an environment variable in 32-bit processes
		malloc_debug_flags &= ~SCALABLE_MALLOC_ABORT_ON_CORRUPTION;
	} else if (flag) {
		malloc_debug_flags |= SCALABLE_MALLOC_ABORT_ON_CORRUPTION;
	}
#endif
	flag = getenv("MallocCheckHeapStart");
	if (flag) {
		malloc_check_start = strtoul(flag, NULL, 0);
		if (malloc_check_start == 0) malloc_check_start = 1;
		if (malloc_check_start == -1) malloc_check_start = 1;
		flag = getenv("MallocCheckHeapEach");
		if (flag) {
			malloc_check_each = strtoul(flag, NULL, 0);
			if (malloc_check_each == 0) malloc_check_each = 1;
			if (malloc_check_each == -1) malloc_check_each = 1;
		}
		_malloc_printf(ASL_LEVEL_INFO, "checks heap after %dth operation and each %d operations\n", malloc_check_start, malloc_check_each);
		flag = getenv("MallocCheckHeapAbort");
		if (flag)
			malloc_check_abort = strtol(flag, NULL, 0);
		if (malloc_check_abort)
			_malloc_printf(ASL_LEVEL_INFO, "will abort on heap corruption\n");
		else {
			flag = getenv("MallocCheckHeapSleep");
			if (flag)
				malloc_check_sleep = strtol(flag, NULL, 0);
			if (malloc_check_sleep > 0)
				_malloc_printf(ASL_LEVEL_INFO, "will sleep for %d seconds on heap corruption\n", malloc_check_sleep);
			else if (malloc_check_sleep < 0)
				_malloc_printf(ASL_LEVEL_INFO, "will sleep once for %d seconds on heap corruption\n", -malloc_check_sleep);
			else
				_malloc_printf(ASL_LEVEL_INFO, "no sleep on heap corruption\n");
		}
	}
	if (getenv("MallocHelp")) {
		_malloc_printf(ASL_LEVEL_INFO,
					   "environment variables that can be set for debug:\n"
					   "- MallocLogFile <f> to create/append messages to file <f> instead of stderr\n"
					   "- MallocGuardEdges to add 2 guard pages for each large block\n"
					   "- MallocDoNotProtectPrelude to disable protection (when previous flag set)\n"
					   "- MallocDoNotProtectPostlude to disable protection (when previous flag set)\n"
					   "- MallocStackLogging to record all stacks.  Tools like leaks can then be applied\n"
					   "- MallocStackLoggingNoCompact to record all stacks.  Needed for malloc_history\n"
					   "- MallocStackLoggingDirectory to set location of stack logs, which can grow large; default is /tmp\n"
					   "- MallocScribble to detect writing on free blocks and missing initializers:\n"
					   "  0x55 is written upon free and 0xaa is written on allocation\n"
					   "- MallocCheckHeapStart <n> to start checking the heap after <n> operations\n"
					   "- MallocCheckHeapEach <s> to repeat the checking of the heap after <s> operations\n"
					   "- MallocCheckHeapSleep <t> to sleep <t> seconds on heap corruption\n"
					   "- MallocCheckHeapAbort <b> to abort on heap corruption if <b> is non-zero\n"
					   "- MallocCorruptionAbort to abort on malloc errors, but not on out of memory for 32-bit processes\n"
					   "  MallocCorruptionAbort is always set on 64-bit processes\n"
					   "- MallocErrorAbort to abort on any malloc error, including out of memory\n"
					   "- MallocHelp - this help!\n");
	}
}

malloc_zone_t *
malloc_create_zone(vm_size_t start_size, unsigned flags)
{
	malloc_zone_t	*zone;

	/* start_size doesn't actually appear to be used, but we test anyway. */
	if (start_size > MALLOC_ABSOLUTE_MAX_SIZE) {
		return NULL;
	}
	if (!_malloc_is_initialized) _malloc_initialize();
	zone = create_scalable_zone(start_size, flags | malloc_debug_flags);
	malloc_zone_register(zone);
	return zone;
}

/*
 * For use by CheckFix: establish a new default zone whose behavior is, apart from
 * the use of death-row and per-CPU magazines, that of Leopard.
 */
void
malloc_create_legacy_default_zone(void)
{
	malloc_zone_t	*zone;
	int i;

	if (!_malloc_is_initialized) _malloc_initialize();
	zone = create_legacy_scalable_zone(0, malloc_debug_flags);

	MALLOC_LOCK();
	malloc_zone_register_while_locked(zone);

	//
	// Establish the legacy scalable zone just created as the default zone.
	//
	malloc_zone_t *hold = malloc_zones[0];
	if(hold->zone_name && strcmp(hold->zone_name, DEFAULT_MALLOC_ZONE_STRING)  == 0) {
		malloc_set_zone_name(hold, NULL);
	}
	malloc_set_zone_name(zone, DEFAULT_MALLOC_ZONE_STRING);

	unsigned protect_size = malloc_num_zones_allocated * sizeof(malloc_zone_t *);
	mprotect(malloc_zones, protect_size, PROT_READ | PROT_WRITE);

	// assert(zone == malloc_zones[malloc_num_zones - 1];
	for (i = malloc_num_zones - 1; i > 0; --i) {
		malloc_zones[i] = malloc_zones[i - 1];
	}
	malloc_zones[0] = zone;

	mprotect(malloc_zones, protect_size, PROT_READ);
	MALLOC_UNLOCK();
}

void
malloc_destroy_zone(malloc_zone_t *zone) {
	malloc_set_zone_name(zone, NULL); // Deallocate zone name wherever it may reside PR_7701095
	malloc_zone_unregister(zone);
	zone->destroy(zone);
}

/*********	Block creation and manipulation	************/

static void
internal_check(void) {
	static vm_address_t	*frames = NULL;
	static unsigned	num_frames;
	if (malloc_zone_check(NULL)) {
		if (!frames) vm_allocate(mach_task_self(), (void *)&frames, vm_page_size, 1);
		thread_stack_pcs(frames, vm_page_size/sizeof(vm_address_t) - 1, &num_frames);
	} else {
		_SIMPLE_STRING b = _simple_salloc();
		if (b)
			_simple_sprintf(b, "*** MallocCheckHeap: FAILED check at %dth operation\n", malloc_check_counter-1);
		else
			_malloc_printf(MALLOC_PRINTF_NOLOG, "*** MallocCheckHeap: FAILED check at %dth operation\n", malloc_check_counter-1);
		malloc_printf("*** MallocCheckHeap: FAILED check at %dth operation\n", malloc_check_counter-1);
		if (frames) {
			unsigned	index = 1;
			if (b) {
				_simple_sappend(b, "Stack for last operation where the malloc check succeeded: ");
				while (index < num_frames) _simple_sprintf(b, "%p ", frames[index++]);
				malloc_printf("%s\n(Use 'atos' for a symbolic stack)\n", _simple_string(b));
			} else {
				/*
				 * Should only get here if vm_allocate() can't get a single page of
				 * memory, implying _simple_asl_log() would also fail.  So we just
				 * print to the file descriptor.
				 */
				_malloc_printf(MALLOC_PRINTF_NOLOG, "Stack for last operation where the malloc check succeeded: ");
				while (index < num_frames) _malloc_printf(MALLOC_PRINTF_NOLOG, "%p ", frames[index++]);
				_malloc_printf(MALLOC_PRINTF_NOLOG, "\n(Use 'atos' for a symbolic stack)\n");
			}
		}
		if (malloc_check_each > 1) {
			unsigned	recomm_each = (malloc_check_each > 10) ? malloc_check_each/10 : 1;
			unsigned	recomm_start = (malloc_check_counter > malloc_check_each+1) ? malloc_check_counter-1-malloc_check_each : 1;
			malloc_printf("*** Recommend using 'setenv MallocCheckHeapStart %d; setenv MallocCheckHeapEach %d' to narrow down failure\n", recomm_start, recomm_each);
		}
		if (malloc_check_abort) {
			CRSetCrashLogMessage(b ? _simple_string(b) : "*** MallocCheckHeap: FAILED check");
			abort();
		} else if (b)
			_simple_sfree(b);
		if (malloc_check_sleep > 0) {
			_malloc_printf(ASL_LEVEL_NOTICE, "*** Sleeping for %d seconds to leave time to attach\n",
						   malloc_check_sleep);
			sleep(malloc_check_sleep);
		} else if (malloc_check_sleep < 0) {
			_malloc_printf(ASL_LEVEL_NOTICE, "*** Sleeping once for %d seconds to leave time to attach\n",
						   -malloc_check_sleep);
			sleep(-malloc_check_sleep);
			malloc_check_sleep = 0;
		}
	}
	malloc_check_start += malloc_check_each;
}

void *
malloc_zone_malloc(malloc_zone_t *zone, size_t size) {
	void	*ptr;
	if (malloc_check_start && (malloc_check_counter++ >= malloc_check_start)) {
		internal_check();
	}
	if (size > MALLOC_ABSOLUTE_MAX_SIZE) {
		return NULL;
	}
	ptr = zone->malloc(zone, size);
	if (malloc_logger)
		malloc_logger(MALLOC_LOG_TYPE_ALLOCATE | MALLOC_LOG_TYPE_HAS_ZONE, (uintptr_t)zone, (uintptr_t)size, 0, (uintptr_t)ptr, 0);
	return ptr;
}

void *
malloc_zone_calloc(malloc_zone_t *zone, size_t num_items, size_t size) {
	void	*ptr;
	if (malloc_check_start && (malloc_check_counter++ >= malloc_check_start)) {
		internal_check();
	}
	if (size > MALLOC_ABSOLUTE_MAX_SIZE) {
		return NULL;
	}
	ptr = zone->calloc(zone, num_items, size);
	if (malloc_logger)
		malloc_logger(MALLOC_LOG_TYPE_ALLOCATE | MALLOC_LOG_TYPE_HAS_ZONE | MALLOC_LOG_TYPE_CLEARED, (uintptr_t)zone, (uintptr_t)(num_items * size), 0,
					  (uintptr_t)ptr, 0);
	return ptr;
}

void *
malloc_zone_valloc(malloc_zone_t *zone, size_t size) {
	void	*ptr;
	if (malloc_check_start && (malloc_check_counter++ >= malloc_check_start)) {
		internal_check();
	}
	if (size > MALLOC_ABSOLUTE_MAX_SIZE) {
		return NULL;
	}
	ptr = zone->valloc(zone, size);
	if (malloc_logger)
		malloc_logger(MALLOC_LOG_TYPE_ALLOCATE | MALLOC_LOG_TYPE_HAS_ZONE, (uintptr_t)zone, (uintptr_t)size, 0, (uintptr_t)ptr, 0);
	return ptr;
}

void *
malloc_zone_realloc(malloc_zone_t *zone, void *ptr, size_t size) {
	void	*new_ptr;
	if (malloc_check_start && (malloc_check_counter++ >= malloc_check_start)) {
		internal_check();
	}
	if (size > MALLOC_ABSOLUTE_MAX_SIZE) {
		return NULL;
	}
	new_ptr = zone->realloc(zone, ptr, size);
	if (malloc_logger)
		malloc_logger(MALLOC_LOG_TYPE_ALLOCATE | MALLOC_LOG_TYPE_DEALLOCATE | MALLOC_LOG_TYPE_HAS_ZONE, (uintptr_t)zone, (uintptr_t)ptr, (uintptr_t)size,
					  (uintptr_t)new_ptr, 0);
	return new_ptr;
}

void
malloc_zone_free(malloc_zone_t *zone, void *ptr) {
	if (malloc_logger)
		malloc_logger(MALLOC_LOG_TYPE_DEALLOCATE | MALLOC_LOG_TYPE_HAS_ZONE, (uintptr_t)zone, (uintptr_t)ptr, 0, 0, 0);
	if (malloc_check_start && (malloc_check_counter++ >= malloc_check_start)) {
		internal_check();
	}
	zone->free(zone, ptr);
}

static void
malloc_zone_free_definite_size(malloc_zone_t *zone, void *ptr, size_t size) {
	if (malloc_logger)
		malloc_logger(MALLOC_LOG_TYPE_DEALLOCATE | MALLOC_LOG_TYPE_HAS_ZONE, (uintptr_t)zone, (uintptr_t)ptr, 0, 0, 0);
	if (malloc_check_start && (malloc_check_counter++ >= malloc_check_start)) {
		internal_check();
	}
	zone->free_definite_size(zone, ptr, size);
}

malloc_zone_t *
malloc_zone_from_ptr(const void *ptr) {
	if (!ptr)
		return NULL;
	else
		return find_registered_zone(ptr, NULL);
}

void *
malloc_zone_memalign(malloc_zone_t *zone, size_t alignment, size_t size) {
	void	*ptr;
	if (zone->version < 5) // Version must be >= 5 to look at the new memalign field.
		return NULL;
	if (!(zone->memalign))
		return NULL;
	if (malloc_check_start && (malloc_check_counter++ >= malloc_check_start)) {
		internal_check();
	}
	if (size > MALLOC_ABSOLUTE_MAX_SIZE) {
		return NULL;
	}
	if (alignment < sizeof( void *) ||		// excludes 0 == alignment
		0 != (alignment & (alignment - 1))) {	// relies on sizeof(void *) being a power of two.
		return NULL;
	}
	ptr = zone->memalign(zone, alignment, size);
	if (malloc_logger)
		malloc_logger(MALLOC_LOG_TYPE_ALLOCATE | MALLOC_LOG_TYPE_HAS_ZONE, (uintptr_t)zone, (uintptr_t)size, 0, (uintptr_t)ptr, 0);
	return ptr;
}

/*********	Functions for zone implementors	************/

void
malloc_zone_register(malloc_zone_t *zone) {
	MALLOC_LOCK();
	malloc_zone_register_while_locked(zone);
	MALLOC_UNLOCK();
}

void
malloc_zone_unregister(malloc_zone_t *z) {
	unsigned	index;

	if (malloc_num_zones == 0)
		return;

	MALLOC_LOCK();
	for (index = 0; index < malloc_num_zones; ++index) {
		if (z != malloc_zones[index])
			continue;

		// Modify the page to be allow write access, so that we can update the
		// malloc_zones array.
		size_t protect_size = malloc_num_zones_allocated * sizeof(malloc_zone_t *);
		mprotect(malloc_zones, protect_size, PROT_READ | PROT_WRITE);

		// If we found a match, replace it with the entry at the end of the list, shrink the list,
		// and leave the end of the list intact to avoid racing with find_registered_zone().

		malloc_zones[index] = malloc_zones[malloc_num_zones - 1];
		--malloc_num_zones;

		mprotect(malloc_zones, protect_size, PROT_READ);

		// Exchange the roles of the FRZ counters. The counter that has captured the number of threads presently
		// executing *inside* find_regiatered_zone is swapped with the counter drained to zero last time through.
		// The former is then allowed to drain to zero while this thread yields.
		int *p = pFRZCounterLive;
		pFRZCounterLive = pFRZCounterDrain;
		pFRZCounterDrain = p;
		__sync_synchronize(); // Full memory barrier

		while (0 != *pFRZCounterDrain) { yield(); }

		MALLOC_UNLOCK();

		return;
	}
	MALLOC_UNLOCK();
	malloc_printf("*** malloc_zone_unregister() failed for %p\n", z);
}

void
malloc_set_zone_name(malloc_zone_t *z, const char *name) {
	char	*newName;

	mprotect(z, sizeof(malloc_zone_t), PROT_READ | PROT_WRITE);
	if (z->zone_name) {
		free((char *)z->zone_name);
		z->zone_name = NULL;
	}
	if (name) {
		size_t buflen = strlen(name) + 1;
		newName = malloc_zone_malloc(z, buflen);
		if (newName) {
			strlcpy(newName, name, buflen);
			z->zone_name = (const char *)newName;
		} else {
			z->zone_name = NULL;
		}
	}
	mprotect(z, sizeof(malloc_zone_t), PROT_READ);
}

const char *
malloc_get_zone_name(malloc_zone_t *zone) {
	return zone->zone_name;
}

/*
 * XXX malloc_printf now uses _simple_*printf.  It only deals with a
 * subset of printf format specifiers, but it doesn't call malloc.
 */

__attribute__((visibility("hidden")))
void
_malloc_vprintf(int flags, const char *format, va_list ap)
{
	_SIMPLE_STRING b;

	if (_malloc_no_asl_log || (flags & MALLOC_PRINTF_NOLOG) || (b = _simple_salloc()) == NULL) {
		if (!(flags & MALLOC_PRINTF_NOPREFIX)) {
			void *self = _os_tsd_get_direct(__TSD_THREAD_SELF);
			_simple_dprintf(malloc_debug_file, "%s(%d,%p) malloc: ", getprogname(), getpid(), self);
		}
		_simple_vdprintf(malloc_debug_file, format, ap);
		return;
	}
	if (!(flags & MALLOC_PRINTF_NOPREFIX)) {
		void *self = _os_tsd_get_direct(__TSD_THREAD_SELF);
		_simple_sprintf(b, "%s(%d,%p) malloc: ", getprogname(), getpid(), self);
	}
	_simple_vsprintf(b, format, ap);
	_simple_put(b, malloc_debug_file);
	_simple_asl_log(flags & MALLOC_PRINTF_LEVEL_MASK, Malloc_Facility, _simple_string(b));
	_simple_sfree(b);
}

__attribute__((visibility("hidden")))
void
_malloc_printf(int flags, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	_malloc_vprintf(flags, format, ap);
	va_end(ap);
}

void
malloc_printf(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	_malloc_vprintf(ASL_LEVEL_ERR, format, ap);
	va_end(ap);
}

/*********	Generic ANSI callouts	************/

void *
malloc(size_t size) {
	void	*retval;
	retval = malloc_zone_malloc(inline_malloc_default_zone(), size);
	if (retval == NULL) {
		errno = ENOMEM;
	}
	return retval;
}

void *
calloc(size_t num_items, size_t size) {
	void	*retval;
	retval = malloc_zone_calloc(inline_malloc_default_zone(), num_items, size);
	if (retval == NULL) {
		errno = ENOMEM;
	}
	return retval;
}

void
free(void *ptr) {
	malloc_zone_t	*zone;
	size_t		size;
	if (!ptr)
		return;
	zone = find_registered_zone(ptr, &size);
	if (!zone) {
		malloc_printf("*** error for object %p: pointer being freed was not allocated\n"
					  "*** set a breakpoint in malloc_error_break to debug\n", ptr);
		malloc_error_break();
		if ((malloc_debug_flags & (SCALABLE_MALLOC_ABORT_ON_CORRUPTION|SCALABLE_MALLOC_ABORT_ON_ERROR))) {
			_SIMPLE_STRING b = _simple_salloc();
			if (b) {
				_simple_sprintf(b, "*** error for object %p: pointer being freed was not allocated\n", ptr);
				CRSetCrashLogMessage(_simple_string(b));
			} else {
				CRSetCrashLogMessage("*** error: pointer being freed was not allocated\n");
			}
			abort();
		}
	} else if (zone->version >= 6 && zone->free_definite_size)
		malloc_zone_free_definite_size(zone, ptr, size);
	else
		malloc_zone_free(zone, ptr);
}

void *
realloc(void *in_ptr, size_t new_size) {
	void		*retval = NULL;
	void		*old_ptr;
	malloc_zone_t	*zone;

	// SUSv3: "If size is 0 and ptr is not a null pointer, the object
	// pointed to is freed. If the space cannot be allocated, the object
	// shall remain unchanged."  Also "If size is 0, either a null pointer
	// or a unique pointer that can be successfully passed to free() shall
	// be returned."  We choose to allocate a minimum size object by calling
	// malloc_zone_malloc with zero size, which matches "If ptr is a null
	// pointer, realloc() shall be equivalent to malloc() for the specified
	// size."  So we only free the original memory if the allocation succeeds.
	old_ptr = (new_size == 0) ? NULL : in_ptr;
	if (!old_ptr) {
		retval = malloc_zone_malloc(inline_malloc_default_zone(), new_size);
	} else {
		zone = find_registered_zone(old_ptr, NULL);
		if (!zone) {
			malloc_printf("*** error for object %p: pointer being realloc'd was not allocated\n"
						  "*** set a breakpoint in malloc_error_break to debug\n", old_ptr);
			malloc_error_break();
			if ((malloc_debug_flags & (SCALABLE_MALLOC_ABORT_ON_CORRUPTION|SCALABLE_MALLOC_ABORT_ON_ERROR))) {
				_SIMPLE_STRING b = _simple_salloc();
				if (b) {
					_simple_sprintf(b, "*** error for object %p: pointer being realloc'd was not allocated\n", old_ptr);
					CRSetCrashLogMessage(_simple_string(b));
				} else {
					CRSetCrashLogMessage("*** error: pointer being realloc'd was not allocated\n");
				}
				abort();
			}
		} else {
			retval = malloc_zone_realloc(zone, old_ptr, new_size);
		}
	}
	if (retval == NULL) {
		errno = ENOMEM;
	} else if (new_size == 0) {
		free(in_ptr);
	}
	return retval;
}

void *
valloc(size_t size) {
	void	*retval;
	malloc_zone_t	*zone = inline_malloc_default_zone();
	retval = malloc_zone_valloc(zone, size);
	if (retval == NULL) {
		errno = ENOMEM;
	}
	return retval;
}

extern void
vfree(void *ptr) {
	free(ptr);
}

size_t
malloc_size(const void *ptr) {
	size_t	size = 0;

	if (!ptr)
		return size;

	(void)find_registered_zone(ptr, &size);
	return size;
}

size_t
malloc_good_size (size_t size) {
	malloc_zone_t	*zone = inline_malloc_default_zone();
	return zone->introspect->good_size(zone, size);
}

/*
 * The posix_memalign() function shall allocate size bytes aligned on a boundary specified by alignment,
 * and shall return a pointer to the allocated memory in memptr.
 * The value of alignment shall be a multiple of sizeof( void *), that is also a power of two.
 * Upon successful completion, the value pointed to by memptr shall be a multiple of alignment.
 *
 * Upon successful completion, posix_memalign() shall return zero; otherwise,
 * an error number shall be returned to indicate the error.
 *
 * The posix_memalign() function shall fail if:
 * EINVAL
 *	The value of the alignment parameter is not a power of two multiple of sizeof( void *).
 * ENOMEM
 *	There is insufficient memory available with the requested alignment.
 */

int
posix_memalign(void **memptr, size_t alignment, size_t size)
{
	void	*retval;

	/* POSIX is silent on NULL == memptr !?! */

	retval = malloc_zone_memalign(inline_malloc_default_zone(), alignment, size);
	if (retval == NULL) {
		// To avoid testing the alignment constraints redundantly, we'll rely on the
		// test made in malloc_zone_memalign to vet each request. Only if that test fails
		// and returns NULL, do we arrive here to detect the bogus alignment and give the
		// required EINVAL return.
		if (alignment < sizeof( void *) ||		// excludes 0 == alignment
			0 != (alignment & (alignment - 1))) {	// relies on sizeof(void *) being a power of two.
			return EINVAL;
		}
		return ENOMEM;
	} else {
		*memptr = retval; // Set iff allocation succeeded
		return 0;
	}
}

static malloc_zone_t *
find_registered_purgeable_zone(void *ptr) {
	if (!ptr)
		return NULL;

	/*
	 * Look for a zone which contains ptr.  If that zone does not have the purgeable malloc flag
	 * set, or the allocation is too small, do nothing.  Otherwise, set the allocation volatile.
	 * FIXME: for performance reasons, we should probably keep a separate list of purgeable zones
	 * and only search those.
	 */
	size_t size = 0;
	malloc_zone_t *zone = find_registered_zone(ptr, &size);

	/* FIXME: would really like a zone->introspect->flags->purgeable check, but haven't determined
	 * binary compatibility impact of changing the introspect struct yet. */
	if (!zone)
		return NULL;

	/* Check to make sure pointer is page aligned and size is multiple of page size */
	if ((size < vm_page_size) || ((size % vm_page_size) != 0))
		return NULL;

	return zone;
}

void
malloc_make_purgeable(void *ptr) {
	malloc_zone_t *zone = find_registered_purgeable_zone(ptr);
	if (!zone)
		return;

	int state = VM_PURGABLE_VOLATILE;
	vm_purgable_control(mach_task_self(), (vm_address_t)ptr, VM_PURGABLE_SET_STATE, &state);
	return;
}

/* Returns true if ptr is valid.  Ignore the return value from vm_purgeable_control and only report
 * state. */
int
malloc_make_nonpurgeable(void *ptr) {
	malloc_zone_t *zone = find_registered_purgeable_zone(ptr);
	if (!zone)
		return 0;

	int state = VM_PURGABLE_NONVOLATILE;
	vm_purgable_control(mach_task_self(), (vm_address_t)ptr, VM_PURGABLE_SET_STATE, &state);

	if (state == VM_PURGABLE_EMPTY)
		return EFAULT;

	return 0;
}

size_t malloc_zone_pressure_relief(malloc_zone_t *zone, size_t goal)
{
	if (!zone) {
		unsigned index = 0;
		size_t total = 0;

		// Take lock to defend against malloc_destroy_zone()
		MALLOC_LOCK();
		while (index < malloc_num_zones) {
			zone = malloc_zones[index++];
			if (zone->version < 8)
				continue;
			if (NULL == zone->pressure_relief)
				continue;
			if (0 == goal) /* Greedy */
				total += zone->pressure_relief(zone, 0);
			else if (goal > total)
				total += zone->pressure_relief(zone, goal - total);
			else /* total >= goal */
				break;
		}
		MALLOC_UNLOCK();
		return total;
	} else {
		// Assumes zone is not destroyed for the duration of this call
		if (zone->version < 8)
			return 0;
		if (NULL == zone->pressure_relief)
			return 0;
		return zone->pressure_relief(zone, goal);
	}
}

/*********	Batch methods	************/

unsigned
malloc_zone_batch_malloc(malloc_zone_t *zone, size_t size, void **results, unsigned num_requested) {
	unsigned	(*batch_malloc)(malloc_zone_t *, size_t, void **, unsigned) = zone-> batch_malloc;
	if (! batch_malloc)
		return 0;
	if (malloc_check_start && (malloc_check_counter++ >= malloc_check_start)) {
		internal_check();
	}
	unsigned	batched = batch_malloc(zone, size, results, num_requested);
	if (malloc_logger) {
		unsigned	index = 0;
		while (index < batched) {
			malloc_logger(MALLOC_LOG_TYPE_ALLOCATE | MALLOC_LOG_TYPE_HAS_ZONE, (uintptr_t)zone, (uintptr_t)size, 0, (uintptr_t)results[index], 0);
			index++;
		}
	}
	return batched;
}

void
malloc_zone_batch_free(malloc_zone_t *zone, void **to_be_freed, unsigned num) {
	if (malloc_check_start && (malloc_check_counter++ >= malloc_check_start)) {
		internal_check();
	}
	if (malloc_logger) {
		unsigned	index = 0;
		while (index < num) {
			malloc_logger(MALLOC_LOG_TYPE_DEALLOCATE | MALLOC_LOG_TYPE_HAS_ZONE, (uintptr_t)zone, (uintptr_t)to_be_freed[index], 0, 0, 0);
			index++;
		}
	}
	void	(*batch_free)(malloc_zone_t *, void **, unsigned) = zone-> batch_free;
	if (batch_free) {
		batch_free(zone, to_be_freed, num);
	} else {
		void 	(*free_fun)(malloc_zone_t *, void *) = zone->free;
		while (num--) {
			void	*ptr = *to_be_freed++;
			free_fun(zone, ptr);
		}
	}
}

/*********	Functions for performance tools	************/

static kern_return_t
_malloc_default_reader(task_t task, vm_address_t address, vm_size_t size, void **ptr) {
	*ptr = (void *)address;
	return 0;
}

kern_return_t
malloc_get_all_zones(task_t task, memory_reader_t reader, vm_address_t **addresses, unsigned *count) {
	// Note that the 2 following addresses are not correct if the address of the target is different from your own.  This notably occurs if the address of System.framework is slid (e.g. different than at B & I )
	vm_address_t	remote_malloc_zones = (vm_address_t)&malloc_zones;
	vm_address_t	remote_malloc_num_zones = (vm_address_t)&malloc_num_zones;
	kern_return_t	err;
	vm_address_t	zones_address;
	vm_address_t	*zones_address_ref;
	unsigned		num_zones;
	unsigned		*num_zones_ref;
	if (!reader) reader = _malloc_default_reader;
	// printf("Read malloc_zones at address %p should be %p\n", &malloc_zones, malloc_zones);
	err = reader(task, remote_malloc_zones, sizeof(void *), (void **)&zones_address_ref);
	// printf("Read malloc_zones[%p]=%p\n", remote_malloc_zones, *zones_address_ref);
	if (err) {
		malloc_printf("*** malloc_get_all_zones: error reading zones_address at %p\n", (unsigned)remote_malloc_zones);
		return err;
	}
	zones_address = *zones_address_ref;
	// printf("Reading num_zones at address %p\n", remote_malloc_num_zones);
	err = reader(task, remote_malloc_num_zones, sizeof(unsigned), (void **)&num_zones_ref);
	if (err) {
		malloc_printf("*** malloc_get_all_zones: error reading num_zones at %p\n", (unsigned)remote_malloc_num_zones);
		return err;
	}
	num_zones = *num_zones_ref;
	// printf("Read malloc_num_zones[%p]=%d\n", remote_malloc_num_zones, num_zones);
	*count = num_zones;
	// printf("malloc_get_all_zones succesfully found %d zones\n", num_zones);
	err = reader(task, zones_address, sizeof(malloc_zone_t *) * num_zones, (void **)addresses);
	if (err) {
		malloc_printf("*** malloc_get_all_zones: error reading zones at %p\n", &zones_address);
		return err;
	}
	// printf("malloc_get_all_zones succesfully read %d zones\n", num_zones);
	return err;
}

/*********	Debug helpers	************/

void
malloc_zone_print_ptr_info(void *ptr) {
	malloc_zone_t	*zone;
	if (!ptr) return;
	zone = malloc_zone_from_ptr(ptr);
	if (zone) {
		printf("ptr %p in registered zone %p\n", ptr, zone);
	} else {
		printf("ptr %p not in heap\n", ptr);
	}
}

boolean_t
malloc_zone_check(malloc_zone_t *zone) {
	boolean_t	ok = 1;
	if (!zone) {
		unsigned	index = 0;
		while (index < malloc_num_zones) {
			zone = malloc_zones[index++];
			if (!zone->introspect->check(zone)) ok = 0;
		}
	} else {
		ok = zone->introspect->check(zone);
	}
	return ok;
}

void
malloc_zone_print(malloc_zone_t *zone, boolean_t verbose) {
	if (!zone) {
		unsigned	index = 0;
		while (index < malloc_num_zones) {
			zone = malloc_zones[index++];
			zone->introspect->print(zone, verbose);
		}
	} else {
		zone->introspect->print(zone, verbose);
	}
}

void
malloc_zone_statistics(malloc_zone_t *zone, malloc_statistics_t *stats) {
	if (!zone) {
		memset(stats, 0, sizeof(*stats));
		unsigned	index = 0;
		while (index < malloc_num_zones) {
			zone = malloc_zones[index++];
			malloc_statistics_t	this_stats;
			zone->introspect->statistics(zone, &this_stats);
			stats->blocks_in_use += this_stats.blocks_in_use;
			stats->size_in_use += this_stats.size_in_use;
			stats->max_size_in_use += this_stats.max_size_in_use;
			stats->size_allocated += this_stats.size_allocated;
		}
	} else {
		zone->introspect->statistics(zone, stats);
	}
}

void
malloc_zone_log(malloc_zone_t *zone, void *address) {
	if (!zone) {
		unsigned	index = 0;
		while (index < malloc_num_zones) {
			zone = malloc_zones[index++];
			zone->introspect->log(zone, address);
		}
	} else {
		zone->introspect->log(zone, address);
	}
}

/*********	Misc other entry points	************/

static void
DefaultMallocError(int x) {
#if USE_SLEEP_RATHER_THAN_ABORT
	malloc_printf("*** error %d\n", x);
	sleep(3600);
#else
	_SIMPLE_STRING b = _simple_salloc();
	if (b) {
		_simple_sprintf(b, "*** error %d", x);
		malloc_printf("%s\n", _simple_string(b));
		CRSetCrashLogMessage(_simple_string(b));
	} else {
		_malloc_printf(MALLOC_PRINTF_NOLOG, "*** error %d", x);
		CRSetCrashLogMessage("*** DefaultMallocError called");
	}
	abort();
#endif
}

void (*
	  malloc_error(void (*func)(int)))(int) {
	return DefaultMallocError;
}

/* Stack logging fork-handling prototypes */
extern void __stack_logging_fork_prepare();
extern void __stack_logging_fork_parent();
extern void __stack_logging_fork_child();
extern void __stack_logging_early_finished();

static void
_malloc_lock_all(void (*callout)(void)) {
	unsigned index = 0;
	MALLOC_LOCK();
	while (index < malloc_num_zones) {
		malloc_zone_t *zone = malloc_zones[index++];
		zone->introspect->force_lock(zone);
	}
	callout();
}

static void
_malloc_unlock_all(void (*callout)(void)) {
	unsigned index = 0;
	callout();
	while (index < malloc_num_zones) {
		malloc_zone_t *zone = malloc_zones[index++];
		zone->introspect->force_unlock(zone);
	}
	MALLOC_UNLOCK();
}

// Called prior to fork() to guarantee that malloc is not in any critical
// sections during the fork(); prevent any locks from being held by non-
// surviving threads after the fork.
void
_malloc_fork_prepare(void) {
	return _malloc_lock_all(&__stack_logging_fork_prepare);
}

// Called in the parent process after fork() to resume normal operation.
void
_malloc_fork_parent(void) {
	return _malloc_unlock_all(&__stack_logging_fork_parent);
}

// Called in the child process after fork() to resume normal operation.
void
_malloc_fork_child(void) {
#if CONFIG_NANOZONE
	if (_malloc_is_initialized && _malloc_engaged_nano)
		nano_forked_zone(inline_malloc_default_zone());
#endif
	return _malloc_unlock_all(&__stack_logging_fork_child);
}

/*
 * A Glibc-like mstats() interface.
 *
 * Note that this interface really isn't very good, as it doesn't understand
 * that we may have multiple allocators running at once.  We just massage
 * the result from malloc_zone_statistics in any case.
 */
struct mstats
mstats(void)
{
	malloc_statistics_t s;
	struct mstats m;

	malloc_zone_statistics(NULL, &s);
	m.bytes_total = s.size_allocated;
	m.chunks_used = s.blocks_in_use;
	m.bytes_used = s.size_in_use;
	m.chunks_free = 0;
	m.bytes_free = m.bytes_total - m.bytes_used;	/* isn't this somewhat obvious? */

	return(m);
}

boolean_t
malloc_zone_enable_discharge_checking(malloc_zone_t *zone)
{
	if (zone->version < 7) // Version must be >= 7 to look at the new discharge checking fields.
		return FALSE;
	if (NULL == zone->introspect->enable_discharge_checking)
		return FALSE;
	return zone->introspect->enable_discharge_checking(zone);
}

void
malloc_zone_disable_discharge_checking(malloc_zone_t *zone)
{
	if (zone->version < 7) // Version must be >= 7 to look at the new discharge checking fields.
		return;
	if (NULL == zone->introspect->disable_discharge_checking)
		return;
	zone->introspect->disable_discharge_checking(zone);
}

void
malloc_zone_discharge(malloc_zone_t *zone, void *memory)
{
	if (NULL == zone)
		zone = malloc_zone_from_ptr(memory);
	if (NULL == zone)
		return;
	if (zone->version < 7) // Version must be >= 7 to look at the new discharge checking fields.
		return;
	if (NULL == zone->introspect->discharge)
		return;
	zone->introspect->discharge(zone, memory);
}

void
malloc_zone_enumerate_discharged_pointers(malloc_zone_t *zone, void (^report_discharged)(void *memory, void *info))
{
	if (!zone) {
		unsigned	index = 0;
		while (index < malloc_num_zones) {
			zone = malloc_zones[index++];
			if (zone->version < 7)
				continue;
			if (NULL == zone->introspect->enumerate_discharged_pointers)
				continue;
			zone->introspect->enumerate_discharged_pointers(zone, report_discharged);
		}
	} else {
		if (zone->version < 7)
			return;
		if (NULL == zone->introspect->enumerate_discharged_pointers)
			return;
		zone->introspect->enumerate_discharged_pointers(zone, report_discharged);
	}
}

/*****************	OBSOLETE ENTRY POINTS	********************/

#if PHASE_OUT_OLD_MALLOC
#error PHASE OUT THE FOLLOWING FUNCTIONS
#endif

void
set_malloc_singlethreaded(boolean_t single) {
	static boolean_t warned = 0;
	if (!warned) {
#if PHASE_OUT_OLD_MALLOC
		malloc_printf("*** OBSOLETE: set_malloc_singlethreaded(%d)\n", single);
#endif
		warned = 1;
	}
}

void
malloc_singlethreaded(void) {
	static boolean_t warned = 0;
	if (!warned) {
		malloc_printf("*** OBSOLETE: malloc_singlethreaded()\n");
		warned = 1;
	}
}

int
malloc_debug(int level) {
	malloc_printf("*** OBSOLETE: malloc_debug()\n");
	return 0;
}

/* vim: set noet:ts=4:sw=4:cindent: */
