#include "lkm.h"
#include "../../external/lkm/api.h"
#include "../signal/sigexc.h"
#include "../base.h"
#include "../linux-syscalls/linux.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/resource.h>
#include "../../../libsyscall/wrappers/_libkernel_init.h"
#include "../simple.h"
#include "../misc/ioctl.h"
#include <elfcalls.h>
#include "../guarded/table.h"
#include "../elfcalls_wrapper.h"

extern int sys_open(const char*, int, int);
extern int close_internal(int);
extern int sys_write(int, const void*, int);
extern int sys_kill(int, int);
extern int sys_getrlimit(int, struct rlimit*);
extern int sys_dup2(int, int);
extern int sys_fcntl(int, int, int);
extern _libkernel_functions_t _libkernel_functions;

VISIBLE
struct elf_calls* _elfcalls;

static bool use_per_thread_driver_fd = false;

extern void _xtrace_postfork_child(void);

void mach_driver_init(const char** applep)
{
#ifdef VARIANT_DYLD
	if (applep != NULL)
	{
		int i;
		for (i = 0; applep[i] != NULL; i++)
		{
			if (strncmp(applep[i], "elf_calls=", 10) == 0)
			{
				uintptr_t table = (uintptr_t) __simple_atoi16(applep[i] + 10, NULL);
				_elfcalls = (struct elf_calls*) table;
			}
		}
	}
#else
	// ask for elfcalls already set up by dyld
	void* (*p2)(void);
	_libkernel_functions->dyld_func_lookup("__dyld_get_elfcalls", (void**)&p2);

	_elfcalls = p2();

	if (applep) {
		// this is not a fork; guard the main thread's RPC FD we get from mldr
		// (in the child after a fork, sys_fork already takes care of this)
		guard_entry_options_t options;
		options.close = _elfcalls->dserver_close_socket;
		guard_table_add(__dserver_per_thread_socket(), guard_flag_prevent_close | guard_flag_close_on_fork, &options);
		int lifetime_pipe = __dserver_get_process_lifetime_pipe();
		if (lifetime_pipe != -1) {
			options.close = _elfcalls->dserver_close_process_lifetime_pipe;
			guard_table_add(lifetime_pipe, guard_flag_prevent_close | guard_flag_close_on_fork, &options);
		}
	}
#endif

	// mach_driver_init() gets called at several points in application's lifetime:
	// 1) When dyld loads
	// 2) When libc initializes
	// 3) After forking

	// this was previously done when the LKM was in-use.
	// now with per-thread dserver FDs, this is more complicated to do.
	// TODO: find a suitable way of doing this.
#if 0
#ifdef VARIANT_DYLD
	struct rlimit lim;
	if (sys_getrlimit(RLIMIT_NOFILE, &lim) == 0 && driver_fd != lim.rlim_cur)
	{
		// sys_getrlimit intentionally reports a limit lower by 1
		// so that our fd remains "hidden" to applications.
		// It also means rlim_cur is not above the limit
		// in the following statement.
		int d = sys_dup2(driver_fd, lim.rlim_cur);
		close_internal(driver_fd);

		driver_fd = d;
	}
#endif
#endif

	if (!applep) {
		// we're being called in the child after a fork; let xtrace know about it
		_xtrace_postfork_child();
	}
}

__attribute__((visibility("default")))
int lkm_call(int call_nr, void* arg)
{
	__simple_printf("Something called the old LKM API (nr = %d)\n", call_nr);
	__builtin_unreachable();
}

__attribute__((visibility("default")))
int lkm_call_raw(int call_nr, void* arg)
{
	__simple_printf("Something called the old LKM API (nr = %d)\n", call_nr);
	__builtin_unreachable();
}

VISIBLE
int mach_driver_get_fd(void) {
	return __dserver_per_thread_socket();
};

VISIBLE
void* elfcalls_get_pointer(void) {
	return _elfcalls;
};

