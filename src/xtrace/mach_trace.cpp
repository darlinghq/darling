#include "simple.h"
#include <unistd.h>
#include <pthread.h>
#include "xtracelib.h"

_Thread_local int mach_call_nr = -1;

static void print_kern_return(char* buf, uintptr_t rv);
static void print_port_return(char* buf, uintptr_t rv);
static void print_int_return(char* buf, uintptr_t rv);
static void print_empty(char* buf, void* args[]);

static const struct calldef mach_defs[128] = {
	[10] = { "_kernelrpc_mach_vm_allocate_trap", NULL, print_kern_return },
	[12] = { "_kernelrpc_mach_vm_deallocate_trap", NULL, print_kern_return },
	[14] = { "_kernelrpc_mach_vm_protect_trap", NULL, print_kern_return },
	[15] = { "_kernelrpc_mach_vm_map_trap", NULL, print_kern_return },
	[16] = { "_kernelrpc_mach_port_allocate_trap", NULL, print_kern_return },
	[17] = { "_kernelrpc_mach_port_destroy_trap", NULL, print_kern_return },
	[18] = { "_kernelrpc_mach_port_deallocate_trap", [](char* buf, void* args[]) { __simple_sprintf(buf, "task=%d, name=%d", args[0], args[1]); }, print_kern_return },
	[19] = { "_kernelrpc_mach_port_mod_refs_trap", NULL, print_kern_return },
	[20] = { "_kernelrpc_mach_port_move_member_trap", NULL, print_kern_return },
	[21] = { "_kernelrpc_mach_port_insert_right_trap", NULL, print_kern_return },
	[22] = { "_kernelrpc_mach_port_insert_member_trap", NULL, print_kern_return },
	[23] = { "_kernelrpc_mach_port_extract_member_trap", NULL, print_kern_return },
	[24] = { "_kernelrpc_mach_port_construct_trap", NULL, print_kern_return },
	[25] = { "_kernelrpc_mach_port_destruct_trap", NULL, print_kern_return },
	[26] = { "mach_reply_port", print_empty, print_port_return },
	[27] = { "thread_self_trap", print_empty, print_port_return },
	[28] = { "task_self_trap", print_empty, print_port_return },
	[29] = { "host_self_trap", print_empty, print_port_return },
	[31] = { "mach_msg_trap", NULL, print_kern_return },
	[32] = { "mach_msg_overwrite_trap", NULL, print_kern_return },
	[33] = { "semaphore_signal_trap", NULL, print_kern_return },
	[34] = { "semaphore_signal_all_trap", NULL, print_kern_return },
	[35] = { "semaphore_signal_thread_trap", NULL, print_kern_return },
	[36] = { "semaphore_wait_trap", NULL, print_kern_return },
	[37] = { "semaphore_wait_signal_trap", NULL, print_kern_return },
	[38] = { "semaphore_timedwait_trap", NULL, print_kern_return },
	[39] = { "semaphore_timedwait_signal_trap", NULL, print_kern_return },
	[41] = { "_kernelrpc_mach_port_guard_trap", NULL, print_kern_return },
	[42] = { "_kernelrpc_mach_port_unguard_trap", NULL, print_kern_return },
	[43] = { "mach_generate_activity_id", NULL, print_kern_return },
	[44] = { "task_name_for_pid", NULL, print_kern_return },
	[45] = { "task_for_pid", NULL, print_port_return },
	[46] = { "pid_for_task", NULL, print_int_return },
	[48] = { "macx_swapon", NULL, print_kern_return },
	[49] = { "macx_swapoff", NULL, print_kern_return },
	[51] = { "macx_triggers", NULL, print_kern_return },
	[52] = { "macx_backing_store_suspend", NULL, print_kern_return },
	[53] = { "macx_backing_store_recovery", NULL, print_kern_return },
	[59] = { "swtch_pri", NULL, print_kern_return },
	[60] = { "swtch", NULL, print_kern_return },
	[61] = { "syscall_thread_switch", NULL, print_kern_return },
	[62] = { "clock_sleep_trap", NULL, print_kern_return },
	[89] = { "mach_timebase_info_trap", NULL, print_kern_return },
	[90] = { "mach_wait_until", NULL, print_kern_return },
	[91] = { "mk_timer_create", print_empty, print_port_return },
	[92] = { "mk_timer_destroy", NULL, print_kern_return },
	[93] = { "mk_timer_arm", NULL, print_kern_return },
	[94] = { "mk_timer_cancel", NULL, print_kern_return },
};

static const char* kern_return_values[] = {
	"KERN_SUCCESS",
	"KERN_INVALID_ADDRESS",
	"KERN_PROTECTION_FAILURE",
	"KERN_NO_SPACE",
	"KERN_INVALID_ARGUMENT",
	"KERN_FAILURE",
	"KERN_RESOURCE_SHORTAGE",
	"KERN_NOT_RECEIVER",
	"KERN_NO_ACCESS",
	"KERN_MEMORY_FAILURE",
	"KERN_MEMORY_ERROR",
	"KERN_ALREADY_IN_SET",
	"KERN_NOT_IN_SET",
	"KERN_NAME_EXISTS",
	"KERN_ABORTED",
	"KERN_INVALID_NAME",
	"KERN_INVALID_TASK",
	"KERN_INVALID_RIGHT",
	"KERN_INVALID_VALUE",
	"KERN_UREFS_OVERFLOW",
	"KERN_INVALID_CAPABILITY",
	"KERN_RIGHT_EXISTS",
	"KERN_INVALID_HOST",
	"KERN_MEMORY_PRESENT",
	"KERN_MEMORY_DATA_MOVED",
	"KERN_MEMORY_RESTART_COPY",
	"KERN_INVALID_PROCESSOR_SET",
	"KERN_POLICY_LIMIT",
	"KERN_INVALID_POLICY",
	"KERN_INVALID_OBJECT",
	"KERN_ALREADY_WAITING",
	"KERN_DEFAULT_SET",
	"KERN_EXCEPTION_PROTECTED",
	"KERN_INVALID_LEDGER",
	"KERN_INVALID_MEMORY_CONTROL",
	"KERN_INVALID_SECURITY",
	"KERN_NOT_DEPRESSED",
	"KERN_TERMINATED",
	"KERN_LOCK_SET_DESTROYED",
	"KERN_LOCK_UNSTABLE",
	"KERN_LOCK_OWNED",
	"KERN_LOCK_OWNED_SELF",
	"KERN_SEMAPHORE_DESTROYED",
	"KERN_RPC_SERVER_TERMINATED",
	"KERN_RPC_TERMINATE_ORPHAN",
	"KERN_RPC_CONTINUE_ORPHAN",
	"KERN_NOT_SUPPORTED",
	"KERN_NODE_DOWN",
	"KERN_NOT_WAITING",
	"KERN_OPERATION_TIMED_OUT",
	"KERN_CODESIGN_ERROR",
	"KERN_POLICY_STATIC",
	"KERN_INSUFFICIENT_BUFFER_SIZE",
};

extern "C"
void darling_mach_syscall_entry_print(int nr, void* args[])
{
	mach_call_nr = nr;
	handle_generic_entry(mach_defs, "mach", mach_call_nr, args);
}

extern "C"
void darling_mach_syscall_exit_print(uintptr_t retval)
{
	handle_generic_exit(mach_defs, "mach", mach_call_nr, retval);
	mach_call_nr = -1;
}

static void print_kern_return(char* buf, uintptr_t rv)
{
	__simple_sprintf(buf, kern_return_values[rv]);
}

static void print_port_return(char* buf, uintptr_t rv)
{
	__simple_sprintf(buf, "port right %d", rv);
}

static void print_int_return(char* buf, uintptr_t rv)
{
	__simple_sprintf(buf, "%d", rv);
}

static void print_empty(char* buf, void* args[])
{
	*buf = 0;
}

