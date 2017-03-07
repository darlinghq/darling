#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include "simple.h"
#include "xtracelib.h"

// Defined in assembly
extern void darling_mach_syscall_entry_trampoline(void);
extern void darling_mach_syscall_exit_trampoline(void);
extern int sys_thread_selfid(void);

#ifdef __x86_64__
struct hook
{
	uint8_t movabs[2];
	uint64_t addr;
	uint8_t call[3];
}
__attribute__((packed));
#endif

// Defined in libsystem_kernel
extern struct hook* _darling_mach_syscall_entry;
extern struct hook* _darling_mach_syscall_exit;

__attribute__((constructor))
void xtrace_setup()
{
	uintptr_t area = (uintptr_t)_darling_mach_syscall_entry;

	// __asm__("int3");
	area &= ~(4096-1);

	mprotect((void*) area, 4096, PROT_READ | PROT_WRITE | PROT_EXEC);

	_darling_mach_syscall_entry->movabs[0] = 0x49;
	_darling_mach_syscall_entry->movabs[1] = 0xbc;
	_darling_mach_syscall_entry->call[0] = 0x41;
	_darling_mach_syscall_entry->call[1] = 0xff;
	_darling_mach_syscall_entry->call[2] = 0xd4;

	_darling_mach_syscall_exit->movabs[0] = 0x49;
	_darling_mach_syscall_exit->movabs[1] = 0xbc;
	_darling_mach_syscall_exit->call[0] = 0x41;
	_darling_mach_syscall_exit->call[1] = 0xff;
	_darling_mach_syscall_exit->call[2] = 0xd4;

	_darling_mach_syscall_entry->addr = (uint64_t)darling_mach_syscall_entry_trampoline;
	_darling_mach_syscall_exit->addr = (uint64_t)darling_mach_syscall_exit_trampoline;

	mprotect((void*) area, 4096, PROT_READ | PROT_EXEC);
}

void handle_generic_entry(const struct calldef* defs, const char* type, int nr, void* args[])
{
	if (defs[nr].name != NULL)
	{
		char args_buf[4096];
		if (defs[nr].print_args != NULL)
			defs[nr].print_args(args_buf, &args[1]); // because args[0] contains %rax
		else
			strcpy(args_buf, "...");
		__simple_printf("[%d] %s (%s)\n", sys_thread_selfid(), defs[nr].name, args_buf);
	}
	else
		__simple_printf("[%d] %s %d (...)\n", sys_thread_selfid(), type, nr);
}


void handle_generic_exit(const struct calldef* defs, const char* type, int nr, uintptr_t retval)
{
	if (defs[nr].name != NULL)
	{
		char args_buf[4096];
		if (defs[nr].print_retval != NULL)
			defs[nr].print_retval(args_buf, retval);
		else
			__simple_sprintf(args_buf, "0x%x\n", retval);
		__simple_printf("[%d]\t%s () -> %s\n", sys_thread_selfid(), defs[nr].name, args_buf);
	}
	else
		__simple_printf("[%d]\t%s %d () -> 0x%x\n", sys_thread_selfid(), type, nr, retval);
}

