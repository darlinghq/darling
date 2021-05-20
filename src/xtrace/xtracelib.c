#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <darling/emulation/simple.h>
#include "xtracelib.h"
#include "mig_trace.h"
#include "tls.h"

// Defined in assembly
extern void darling_mach_syscall_entry_trampoline(void);
extern void darling_mach_syscall_exit_trampoline(void);
extern void darling_bsd_syscall_entry_trampoline(void);
extern void darling_bsd_syscall_exit_trampoline(void);
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
extern struct hook* _darling_bsd_syscall_entry;
extern struct hook* _darling_bsd_syscall_exit;

static void xtrace_setup_mach(void);
static void xtrace_setup_bsd(void);
static void setup_hook(struct hook* hook, void* fnptr);
static void xtrace_setup_options(void);

static int xtrace_ignore = 1;

__attribute__((constructor))
void xtrace_setup()
{
	xtrace_setup_options();
	xtrace_setup_mig_tracing();
	xtrace_setup_mach();
	xtrace_setup_bsd();

	xtrace_ignore = 0;
}

static int xtrace_split_entry_and_exit = 0;
int xtrace_no_color = 0;
int xtrace_kprintf = 0;

static void xtrace_setup_options(void)
{
	if (getenv("XTRACE_SPLIT_ENTRY_AND_EXIT") != NULL)
		xtrace_split_entry_and_exit = 1;
	if (getenv("XTRACE_NO_COLOR") != NULL)
		xtrace_no_color = 1;
	if (getenv("XTRACE_KPRINTF") != NULL)
		xtrace_kprintf = 1;
}


static void setup_hook(struct hook* hook, void* fnptr)
{
	// this hook is (in GAS syntax):
	//   movq $<fnptr value>, %r10
	//   call *%r10
	hook->movabs[0] = 0x49;
	hook->movabs[1] = 0xba;
	hook->call[0] = 0x41;
	hook->call[1] = 0xff;
	hook->call[2] = 0xd2;
	hook->addr = (uintptr_t)fnptr;
}

static void xtrace_setup_mach(void)
{
	uintptr_t area = (uintptr_t)_darling_mach_syscall_entry;
	uintptr_t areaEnd = ((uintptr_t)_darling_mach_syscall_exit) + sizeof(struct hook);

	// __asm__("int3");
	area &= ~(4096-1);
	areaEnd &= ~(4096-1);

	uintptr_t bytes = 4096 + (areaEnd-area);

	mprotect((void*) area, bytes, PROT_READ | PROT_WRITE | PROT_EXEC);

	setup_hook(_darling_mach_syscall_entry, darling_mach_syscall_entry_trampoline);
	setup_hook(_darling_mach_syscall_exit, darling_mach_syscall_exit_trampoline);

	mprotect((void*) area, bytes, PROT_READ | PROT_EXEC);
}

static void xtrace_setup_bsd(void)
{
	uintptr_t area = (uintptr_t)_darling_bsd_syscall_entry;
	uintptr_t areaEnd = ((uintptr_t)_darling_bsd_syscall_exit) + sizeof(struct hook);

	// __asm__("int3");
	area &= ~(4096-1);
	areaEnd &= ~(4096-1);

	uintptr_t bytes = 4096 + (areaEnd-area);

	mprotect((void*) area, bytes, PROT_READ | PROT_WRITE | PROT_EXEC);

	setup_hook(_darling_bsd_syscall_entry, darling_bsd_syscall_entry_trampoline);
	setup_hook(_darling_bsd_syscall_exit, darling_bsd_syscall_exit_trampoline);

	mprotect((void*) area, bytes, PROT_READ | PROT_EXEC);
}

void xtrace_set_gray_color(void)
{
	if (xtrace_no_color)
		return;

	xtrace_printf("\033[37m");
}

void xtrace_reset_color(void)
{
	if (xtrace_no_color)
		return;

	xtrace_printf("\033[0m");
}

void xtrace_start_line(int indent)
{
	xtrace_set_gray_color();

	xtrace_printf("[%d]", sys_thread_selfid());
        for (int i = 0; i < indent + 1; i++)
		xtrace_printf(" ");

	xtrace_reset_color();
}

static void print_call(const struct calldef* defs, const char* type, int nr, int indent, int gray_name)
{
	xtrace_start_line(indent);

	if (gray_name)
		xtrace_set_gray_color();

	if (defs[nr].name != NULL)
		xtrace_printf("%s", defs[nr].name);
	else
		xtrace_printf("%s %d", type, nr);

	// Leaves gray color on!
}


struct nested_call_struct {
	// We're inside this many calls. In other words, we have printed this many
	// call entries without matching exits.
	int current_level;
	// What that value was the last time. if we've just handled an entry or an
	// exit, this will be greater/less than current_level.
	int previous_level;
	// Call numbers, indexed by current level.
	int nrs[64];
};

DEFINE_XTRACE_TLS_VAR(struct nested_call_struct, nested_call);

void handle_generic_entry(const struct calldef* defs, const char* type, int nr, void* args[])
{
	if (xtrace_ignore)
		return;

	if (get_ptr_nested_call()->previous_level < get_ptr_nested_call()->current_level && !xtrace_split_entry_and_exit)
	{
		// We are after an earlier entry without an exit.
		xtrace_printf("\n");
	}

	int indent = 4 * get_ptr_nested_call()->current_level;
	get_ptr_nested_call()->nrs[get_ptr_nested_call()->current_level] = nr;

	print_call(defs, type, nr, indent, 0);

	if (defs[nr].name != NULL)
	{
		char args_buf[4096];
		if (defs[nr].print_args != NULL)
			defs[nr].print_args(args_buf, nr, args);
		else
			strcpy(args_buf, "...");
		xtrace_printf("(%s)", args_buf);
	}
	else
		xtrace_printf("(...)");

	if (xtrace_split_entry_and_exit)
		xtrace_printf("\n");

	get_ptr_nested_call()->previous_level = get_ptr_nested_call()->current_level++;
}


void handle_generic_exit(const struct calldef* defs, const char* type, uintptr_t retval, int force_split)
{
	if (xtrace_ignore)
		return;

	if (get_ptr_nested_call()->previous_level > get_ptr_nested_call()->current_level)
	{
		// We are after an exit, so our call has been split up.
		force_split = 1;
	}
	get_ptr_nested_call()->previous_level = get_ptr_nested_call()->current_level--;
	int nr = get_ptr_nested_call()->nrs[get_ptr_nested_call()->current_level];

	if (xtrace_split_entry_and_exit || force_split)
	{
		int indent = 4 * get_ptr_nested_call()->current_level;
		print_call(defs, type, nr, indent, 1);
		xtrace_printf("()");
	}

	xtrace_set_gray_color();
	xtrace_printf(" -> ");
	xtrace_reset_color();

	if (defs[nr].name != NULL)
	{
		char args_buf[4096];
		if (defs[nr].print_retval != NULL)
			defs[nr].print_retval(args_buf, nr, retval);
		else
			__simple_sprintf(args_buf, "0x%lx\n", retval);
		xtrace_printf("%s\n", args_buf);
	}
	else
		xtrace_printf("0x%lx\n", retval);
}

