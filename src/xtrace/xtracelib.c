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
#include "lock.h"
#include <limits.h>

#include <darling/emulation/ext/for-xtrace.h>
#include <fcntl.h>
#include <signal.h>

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

// whether to use a sigaltstack guard page below the stack
// (this should probably be left on)
#define SIGALTSTACK_GUARD 1

__attribute__((constructor))
void xtrace_setup()
{
	xtrace_setup_options();
	xtrace_setup_mig_tracing();
	xtrace_setup_mach();
	xtrace_setup_bsd();

	// override the default sigaltstack used by libsystem_kernel for the main thread
	// (we need more than the default 8KiB; testing has shown that 16KiB seems to be enough)
	struct bsd_stack custom_altstack = {
		.ss_size = 16 * 1024,
		.ss_flags = 0,
	};

#if SIGALTSTACK_GUARD
	custom_altstack.ss_sp = (void*)mmap(NULL, custom_altstack.ss_size + 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	if (custom_altstack.ss_sp == MAP_FAILED) {
		xtrace_abort("xtrace: failed to allocate larger sigstack for main thread");
	}

	mprotect(custom_altstack.ss_sp, 4096, PROT_NONE);
	custom_altstack.ss_sp = (char*)custom_altstack.ss_sp + 4096;
#else
	custom_altstack.ss_sp = (void*)mmap(NULL, custom_altstack.ss_size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	if (custom_altstack.ss_sp == MAP_FAILED) {
		xtrace_abort("xtrace: failed to allocate larger sigstack for main thread");
	}
#endif

	if (_sigaltstack_for_xtrace(&custom_altstack, NULL) < 0) {
		xtrace_abort("failed to override sigaltstack");
	}

	// and set the size to allocate for future threads
	_sigaltstack_set_default_size_for_xtrace(custom_altstack.ss_size);

	xtrace_ignore = 0;
}

static int xtrace_split_entry_and_exit = 0;
int xtrace_no_color = 0;
int xtrace_kprintf = 0;

static int xtrace_use_logfile = 0;
static int xtrace_use_per_thread_logfile = 0;

static char xtrace_logfile_base[PATH_MAX] = {0};

static xtrace_once_t xtrace_common_logfile_once = XTRACE_ONCE_INITIALIZER;
int xtrace_common_logfile = -1;

DEFINE_XTRACE_TLS_VAR(int, xtrace_per_thread_logfile, -1);

static void xtrace_setup_options(void)
{
	const char* xtrace_log_file = getenv("XTRACE_LOG_FILE");

	if (getenv("XTRACE_SPLIT_ENTRY_AND_EXIT") != NULL)
		xtrace_split_entry_and_exit = 1;
	if (getenv("XTRACE_NO_COLOR") != NULL)
		xtrace_no_color = 1;
	if (getenv("XTRACE_KPRINTF") != NULL)
		xtrace_kprintf = 1;

	if (getenv("XTRACE_LOG_FILE_PER_THREAD") != NULL)
		xtrace_use_per_thread_logfile = 1;

	if (xtrace_log_file != NULL) {
		xtrace_use_logfile = 1;
		strlcpy(xtrace_logfile_base, xtrace_log_file, sizeof(xtrace_logfile_base));
	}
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

	xtrace_log("\033[37m");
}

void xtrace_reset_color(void)
{
	if (xtrace_no_color)
		return;

	xtrace_log("\033[0m");
}

void xtrace_start_line(int indent)
{
	xtrace_set_gray_color();

	xtrace_log("[%d]", sys_thread_selfid());
        for (int i = 0; i < indent + 1; i++)
		xtrace_log(" ");

	xtrace_reset_color();
}

static void print_call(const struct calldef* defs, const char* type, int nr, int indent, int gray_name)
{
	xtrace_start_line(indent);

	if (gray_name)
		xtrace_set_gray_color();

	if (defs[nr].name != NULL)
		xtrace_log("%s", defs[nr].name);
	else
		xtrace_log("%s %d", type, nr);

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

DEFINE_XTRACE_TLS_VAR(struct nested_call_struct, nested_call, (struct nested_call_struct) {0});

void handle_generic_entry(const struct calldef* defs, const char* type, int nr, void* args[])
{
	if (xtrace_ignore)
		return;

	if (get_ptr_nested_call()->previous_level < get_ptr_nested_call()->current_level && !xtrace_split_entry_and_exit)
	{
		// We are after an earlier entry without an exit.
		xtrace_log("\n");
	}

	int indent = 4 * get_ptr_nested_call()->current_level;
	get_ptr_nested_call()->nrs[get_ptr_nested_call()->current_level] = nr;

	print_call(defs, type, nr, indent, 0);

	if (defs[nr].name != NULL && defs[nr].print_args != NULL)
	{
		xtrace_log("(");
		defs[nr].print_args(nr, args);
		xtrace_log(")");
	}
	else
		xtrace_log("(...)");

	if (xtrace_split_entry_and_exit)
		xtrace_log("\n");

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
		xtrace_log("()");
	}

	xtrace_set_gray_color();
	xtrace_log(" -> ");
	xtrace_reset_color();

	if (defs[nr].name != NULL && defs[nr].print_retval != NULL)
	{
		defs[nr].print_retval(nr, retval);
		xtrace_log("\n");
	}
	else
		xtrace_log("0x%lx\n", retval);
}

void xtrace_log(const char* format, ...) {
	va_list args;
	va_start(args, format);
	xtrace_log_v(format, args);
	va_end(args);
};

// TODO: we should add guarded FD support so that we can make FDs like these logfile descriptors guarded.
//       it would also be very useful to guard the special LKM descriptor.

static void xtrace_common_logfile_init(void) {
	xtrace_common_logfile = _open_for_xtrace(xtrace_logfile_base, O_WRONLY | O_APPEND | O_CLOEXEC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
};

static void ensure_logfile(void) {
	bool created = false;
	int fd = -1;

	if (!xtrace_use_logfile) {
		xtrace_abort("xtrace: tried to use logfile when not enabled");
	}

	if (get_xtrace_per_thread_logfile() != -1) {
		return;
	}

	if (xtrace_use_per_thread_logfile) {
		char filename[sizeof(xtrace_logfile_base)];
		char append[32] = {0};

		strlcpy(filename, xtrace_logfile_base, PATH_MAX);

		__simple_snprintf(append, sizeof(append), ".%d", sys_thread_selfid());
		strlcat(filename, append, PATH_MAX);

		fd = _open_for_xtrace(filename, O_WRONLY | O_APPEND | O_CLOEXEC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	} else {
		xtrace_once(&xtrace_common_logfile_once, xtrace_common_logfile_init);
		fd = xtrace_common_logfile;
	}

	if (fd < 0) {
		xtrace_abort("xtrace: failed to open logfile");
	}

	set_xtrace_per_thread_logfile(fd);
};

void xtrace_log_v(const char* format, va_list args) {
	if (xtrace_kprintf) {
		char real_format[512] = "xtrace: ";
		strlcpy(&real_format[0] + (sizeof("xtrace: ") - 1), format, sizeof(real_format) - (sizeof("xtrace: ") - 1));
		__simple_vkprintf(real_format, args);
	} else if (xtrace_use_logfile) {
		char output[512];
		ensure_logfile();
		__simple_vsnprintf(output, sizeof(output), format, args);
		__write_for_xtrace(get_xtrace_per_thread_logfile(), output, __simple_strlen(output));
	} else {
		__simple_vprintf(format, args);
	}
};

void xtrace_error(const char* format, ...) {
	va_list args;
	va_start(args, format);
	xtrace_error_v(format, args);
	va_end(args);
};

void xtrace_error_v(const char* format, va_list args) {
	if (xtrace_kprintf) {
		char real_format[512] = "xtrace: ";
		strlcpy(real_format + (sizeof("xtrace: ") - 1), format, sizeof(real_format) - (sizeof("xtrace: ") - 1));
		__simple_vkprintf(real_format, args);
	} else if (xtrace_use_logfile) {
		char output[512];
		ensure_logfile();
		__simple_vsnprintf(output, sizeof(output), format, args);
		__write_for_xtrace(get_xtrace_per_thread_logfile(), output, __simple_strlen(output));
	} else {
		__simple_vfprintf(STDERR_FILENO, format, args);
	}
};

void xtrace_abort(const char* message) {
	_abort_with_payload_for_xtrace(0, 0, NULL, 0, message, 0);
	__builtin_unreachable();
};
