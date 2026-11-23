#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "fex_bridge.h"
#include <dlfcn.h>
#include <elf.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

_Static_assert(sizeof(struct elf_calls) == DARLING_FEX_ELFCALL_COUNT * sizeof(void*),
	"DARLING_FEX_ELFCALL_COUNT must match struct elf_calls field count");

static struct elf_calls g_host_elfcalls;
static struct elf_calls g_trampoline_elfcalls;
static uint8_t* g_elfcall_stub_page = NULL;
static pthread_once_t g_elfcall_init_once = PTHREAD_ONCE_INIT;

static pthread_mutex_t g_dlsym_mutex = PTHREAD_MUTEX_INITIALIZER;
static uint8_t* g_dlsym_stub_area = NULL;
static char* g_dlsym_names[DARLING_FEX_DLSYM_MAX_SLOTS];
static pthread_once_t g_dlsym_init_once = PTHREAD_ONCE_INIT;
static void* g_dlsym_targets[DARLING_FEX_DLSYM_MAX_SLOTS];
static uint32_t g_dlsym_count = 0;

static darling_thread_create_callbacks_t g_guest_thread_cbs = NULL;
static fex_fork_lock_fn_t g_fork_lock_hook = NULL;
static fex_fork_unlock_fn_t g_fork_unlock_hook = NULL;

struct fex_deferred_terminate {
	bool pending;
	void* stackaddr;
	unsigned long freesize;
	unsigned long pthobj_size;
};
static __thread struct fex_deferred_terminate t_deferred_terminate = { false, NULL, 0, 0 };

bool fex_elfcalls_has_pending_terminate(void)
{
	return t_deferred_terminate.pending;
}

int fex_elfcalls_flush_deferred_terminate(void)
{
	if (!t_deferred_terminate.pending)
		return 0;

	void* stackaddr = t_deferred_terminate.stackaddr;
	unsigned long freesize = t_deferred_terminate.freesize;
	unsigned long pthobj_size = t_deferred_terminate.pthobj_size;
	t_deferred_terminate.pending = false;

	if (g_host_elfcalls.darling_thread_terminate) {
		return g_host_elfcalls.darling_thread_terminate(stackaddr, freesize, pthobj_size);
	}
	return 0;
}

void fex_elfcalls_set_fork_hooks(fex_fork_lock_fn_t lock_hook, fex_fork_unlock_fn_t unlock_hook)
{
	g_fork_lock_hook = lock_hook;
	g_fork_unlock_hook = unlock_hook;
}

// Each stub is entered with the C calling convention and enters the host with `syscall`. FEX, like the
// hardware, overwrites %rcx with the return address on `syscall`, so the fourth argument is copied
// to %r10 first, which is where the syscall convention keeps it.
#define FEX_STUB_SIZE 16

static void write_x86_64_syscall_stub(uint8_t* dst, uint32_t syscall_nr)
{
	// movq %rcx, %r10 (0x49 0x89 0xca); movl $syscall_nr, %eax (0xb8 + imm32); syscall (0x0f 0x05); ret (0xc3)
	dst[0] = 0x49;
	dst[1] = 0x89;
	dst[2] = 0xca;
	dst[3] = 0xb8;
	memcpy(&dst[4], &syscall_nr, sizeof(syscall_nr));
	dst[8] = 0x0f;
	dst[9] = 0x05;
	dst[10] = 0xc3;
	memset(&dst[11], 0x90, FEX_STUB_SIZE - 11);
}

static void init_elfcall_stubs(void)
{
	size_t page_sz = (size_t)sysconf(_SC_PAGESIZE);
	g_elfcall_stub_page = mmap(NULL, page_sz, PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (g_elfcall_stub_page == MAP_FAILED) {
		fprintf(stderr, "mldr/fex: failed to mmap elf_calls stub page: %s\n", strerror(errno));
		abort();
	}

	void** table_slots = (void**)&g_trampoline_elfcalls;
	for (uint32_t i = 0; i < DARLING_FEX_ELFCALL_COUNT; i++) {
		uint8_t* stub = g_elfcall_stub_page + (i * FEX_STUB_SIZE);
		write_x86_64_syscall_stub(stub, DARLING_FEX_ELFCALL_SYSCALL_BASE + i);
		table_slots[i] = stub;
	}

	if (mprotect(g_elfcall_stub_page, page_sz, PROT_READ | PROT_EXEC) != 0) {
		fprintf(stderr, "mldr/fex: failed to mprotect elf_calls stub page RX: %s\n", strerror(errno));
		abort();
	}
	__builtin___clear_cache((char*)g_elfcall_stub_page, (char*)g_elfcall_stub_page + page_sz);
}

static void init_dlsym_stubs(void)
{
	size_t page_sz = (size_t)sysconf(_SC_PAGESIZE);
	size_t needed = DARLING_FEX_DLSYM_MAX_SLOTS * FEX_STUB_SIZE;
	size_t area_size = (needed + page_sz - 1) & ~(page_sz - 1);

	g_dlsym_stub_area = mmap(NULL, area_size, PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (g_dlsym_stub_area == MAP_FAILED) {
		g_dlsym_stub_area = NULL;
		fprintf(stderr, "mldr/fex: failed to mmap dlsym stub area: %s\n", strerror(errno));
		abort();
	}

	for (uint32_t i = 0; i < DARLING_FEX_DLSYM_MAX_SLOTS; i++)
		write_x86_64_syscall_stub(g_dlsym_stub_area + (i * FEX_STUB_SIZE), DARLING_FEX_DLSYM_SYSCALL_BASE + i);

	if (mprotect(g_dlsym_stub_area, area_size, PROT_READ | PROT_EXEC) != 0) {
		fprintf(stderr, "mldr/fex: failed to mprotect dlsym stub area RX: %s\n", strerror(errno));
		abort();
	}
	__builtin___clear_cache((char*)g_dlsym_stub_area, (char*)g_dlsym_stub_area + area_size);
}

struct elf_calls* fex_elfcalls_get_trampoline_table(const struct elf_calls* host_calls)
{
	if (host_calls)
		g_host_elfcalls = *host_calls;
	pthread_once(&g_elfcall_init_once, init_elfcall_stubs);
	return &g_trampoline_elfcalls;
}

static unsigned int host_cb_thread_self_trap(void)
{
	darling_thread_create_callbacks_t cbs = __atomic_load_n(&g_guest_thread_cbs, __ATOMIC_ACQUIRE);
	if (!cbs || !cbs->thread_self_trap)
		return 0;
	return (unsigned int)fex_invoke_guest_callback(
		(uintptr_t)cbs->thread_self_trap, 0, 0);
}

static void host_cb_thread_set_tsd_base(void* tsd_base, int unk)
{
	fex_set_current_thread_gs_base((uint64_t)(uintptr_t)tsd_base);
	darling_thread_create_callbacks_t cbs = __atomic_load_n(&g_guest_thread_cbs, __ATOMIC_ACQUIRE);
	if (cbs && cbs->thread_set_tsd_base) {
		fex_invoke_guest_callback(
			(uintptr_t)cbs->thread_set_tsd_base,
			(uint64_t)(uintptr_t)tsd_base,
			(uint64_t)(uint32_t)unk);
	}
}

static void host_cb_rpc_guard(int fd)
{
	darling_thread_create_callbacks_t cbs = __atomic_load_n(&g_guest_thread_cbs, __ATOMIC_ACQUIRE);
	if (cbs && cbs->rpc_guard) {
		fex_invoke_guest_callback(
			(uintptr_t)cbs->rpc_guard,
			(uint64_t)(uint32_t)fd, 0);
	}
}

static void host_cb_rpc_unguard(int fd)
{
	darling_thread_create_callbacks_t cbs = __atomic_load_n(&g_guest_thread_cbs, __ATOMIC_ACQUIRE);
	if (cbs && cbs->rpc_unguard) {
		fex_invoke_guest_callback(
			(uintptr_t)cbs->rpc_unguard,
			(uint64_t)(uint32_t)fd, 0);
	}
}

static const struct darling_thread_create_callbacks g_wrapped_thread_cbs = {
	.thread_self_trap = host_cb_thread_self_trap,
	.thread_set_tsd_base = host_cb_thread_set_tsd_base,
	.rpc_guard = host_cb_rpc_guard,
	.rpc_unguard = host_cb_rpc_unguard,
};

darling_thread_create_callbacks_t fex_wrap_guest_thread_callbacks(darling_thread_create_callbacks_t guest_cbs)
{
	__atomic_store_n(&g_guest_thread_cbs, guest_cbs, __ATOMIC_RELEASE);
	return &g_wrapped_thread_cbs;
}

struct fex_unsupported_callback { const char* name; unsigned mask; const char* reason; };
struct fex_callback_struct { const char* name; unsigned arg; unsigned words; unsigned fields; };
// What the bridge does to the arguments of one host function, decided from its name when it is wrapped
// so that a call does no name lookups.
enum fex_x_handler { FEX_X_HANDLER_NONE, FEX_X_ERROR_HANDLER, FEX_X_IO_ERROR_HANDLER };
// FEX_JPEG_OBJECT: the first argument is a libjpeg object (j_common_ptr), whose error manager the library
// may call into; FEX_JPEG_STD_ERROR: jpeg_std_error, which fills an error manager with host methods.
enum fex_jpeg_rule { FEX_JPEG_NONE, FEX_JPEG_OBJECT, FEX_JPEG_STD_ERROR };
struct fex_callback_rules {
	enum fex_x_handler x_handler;
	unsigned args; // the g_callback_args mask, 0 if the function has no entry
	const struct fex_unsupported_callback* unsupported; // its g_callback_unsupported entry, if any
	const struct fex_callback_struct* callback_struct; // its g_callback_structs entry, if any
	unsigned xim_list; // first argument of its g_xim_value_lists (name, value) list, 0 if none
	bool returns_function; // it returns a host function named by its first argument (g_proc_address_lookups)
	enum fex_jpeg_rule jpeg;
	bool jpeg_method; // one of the methods jpeg_std_error puts in an error manager
	const struct fex_kept_struct* kept_struct; // its g_kept_struct_args layout, if any
	unsigned kept_struct_arg; // which argument points to that struct
	unsigned kept_struct_back; // its g_kept_struct_back_args mask, 0 if none
};
// What a host thunk does to the arguments a host library calls it with, before the guest function runs:
// `struct_args` may point to a host copy made for a kept struct (they get the guest's struct back), and
// `host_fn_args` are host function pointers, which the guest can only call through a bridge stub that
// follows the rules of the host function name `host_fn_rules`.
struct fex_thunk_rules { unsigned struct_args; unsigned host_fn_args; const char* host_fn_rules; };
static void translate_thunk_args(const struct fex_thunk_rules* rules, uint64_t args[6]);
static struct fex_callback_rules g_dlsym_rules[DARLING_FEX_DLSYM_MAX_SLOTS];
static struct fex_callback_rules callback_rules_for(const char* name);
static void* wrap_host_function(void* host_sym, const char* symbol_name, const struct fex_callback_rules* rules);

void* fex_dlsym_wrap_if_func(void* host_sym, const char* symbol_name)
{
	if (!host_sym)
		return NULL;

	Dl_info info;
	const Elf64_Sym* elf_sym = NULL;
	if (dladdr1(host_sym, &info, (void**)&elf_sym, RTLD_DL_SYMENT) != 0 && elf_sym != NULL) {
		if (ELF64_ST_TYPE(elf_sym->st_info) == STT_OBJECT)
			return host_sym;
	}

	const struct fex_callback_rules rules = callback_rules_for(symbol_name);
	return wrap_host_function(host_sym, symbol_name, &rules);
}

// The guest stub of a host function, with the rules a call through it follows (the first rules a host
// function is wrapped with stay its rules).
static void* wrap_host_function(void* host_sym, const char* symbol_name, const struct fex_callback_rules* rules)
{
	pthread_once(&g_dlsym_init_once, init_dlsym_stubs);

	pthread_mutex_lock(&g_dlsym_mutex);

	for (uint32_t i = 0; i < g_dlsym_count; i++) {
		if (g_dlsym_targets[i] == host_sym) {
			void* existing = g_dlsym_stub_area + (i * FEX_STUB_SIZE);
			pthread_mutex_unlock(&g_dlsym_mutex);
			return existing;
		}
	}

	if (g_dlsym_count >= DARLING_FEX_DLSYM_MAX_SLOTS) {
		pthread_mutex_unlock(&g_dlsym_mutex);
		fprintf(stderr, "mldr/fex: dlsym stub table exhausted (%u slots)\n", DARLING_FEX_DLSYM_MAX_SLOTS);
		abort();
	}

	uint32_t slot = g_dlsym_count++;
	g_dlsym_targets[slot] = host_sym;
	g_dlsym_rules[slot] = *rules;
	g_dlsym_names[slot] = symbol_name ? strdup(symbol_name) : NULL;
	if (symbol_name && !g_dlsym_names[slot]) {
		pthread_mutex_unlock(&g_dlsym_mutex);
		fprintf(stderr, "mldr/fex: out of memory recording the name of a host symbol\n");
		abort();
	}
	void* stub = g_dlsym_stub_area + (slot * FEX_STUB_SIZE);

	pthread_mutex_unlock(&g_dlsym_mutex);
	return stub;
}

// A host library calls a function pointer it was given natively, but the guest's functions are x86_64
// code. Xlib's error handlers are the only callbacks the X11 backend hands it directly, so those two
// are replaced by host functions that run the guest handler under FEX.
static uintptr_t g_guest_x_error_handler = 0;
static uintptr_t g_guest_x_io_error_handler = 0;
// The handlers Xlib had before the guest replaced them: host code that the guest may later restore.
static uint64_t g_host_x_error_native = 0;
static uint64_t g_host_x_io_error_native = 0;

static int host_x_error_handler(void* display, void* event)
{
	return (int)fex_invoke_guest_callback(g_guest_x_error_handler, (uint64_t)(uintptr_t)display, (uint64_t)(uintptr_t)event);
}

static int host_x_io_error_handler(void* display)
{
	return (int)fex_invoke_guest_callback(g_guest_x_io_error_handler, (uint64_t)(uintptr_t)display, 0);
}

// Host library functions that take function pointers other than Xlib's error handlers. The guest's
// pointer is replaced by a host thunk bound to that guest function, which runs it under FEX with the
// first six integer arguments it is called with. Arguments after the sixth and floating point
// arguments are not forwarded, and a floating point return value is lost: functions whose callbacks
// need them are listed in g_callback_unsupported instead.
//
// A library may keep a callback for as long as it likes (libwayland until the proxy dies, libtiff until
// TIFFClose), and nothing tells the bridge when it lets go, so a thunk stays bound to its guest function
// for the life of the process. The same guest function always gets the same thunk, so the number of
// thunks is bounded by the number of distinct guest functions handed to host libraries, which is small
// and fixed by the guest's code. The thunks are generated a page at a time, up to FEX_CALLBACK_MAX.
//
// Each thunk is 32 bytes of aarch64 code: "movz x6, #slot; ldr x16, entry; br x16; nop; .quad entry".
// x6 is the seventh integer argument register, which no forwarded argument uses, so the common entry
// receives the slot as an ordinary seventh argument.
#if !defined(__aarch64__)
#error "the host callback thunks are aarch64 code"
#endif

#define FEX_CALLBACK_MAX 8192u
#define FEX_CALLBACK_THUNK_SIZE 32u
_Static_assert(FEX_CALLBACK_MAX <= 0x10000, "a thunk's slot must fit the 16-bit immediate of movz");

static uintptr_t g_callback_guest_fn[FEX_CALLBACK_MAX];
static const struct fex_thunk_rules* g_callback_thunk_rules[FEX_CALLBACK_MAX]; // NULL: arguments pass as is
// Open-addressing index from guest function to slot + 1 (0 = empty), never more than half full.
#define FEX_CALLBACK_INDEX_SIZE (2 * FEX_CALLBACK_MAX)
static uint16_t g_callback_index[FEX_CALLBACK_INDEX_SIZE];
static uint32_t g_callback_count = 0;
static uint32_t g_callback_ready = 0; // thunks [0, ready) are written and executable
static uint8_t* g_callback_area = NULL;
static pthread_mutex_t g_callback_mutex = PTHREAD_MUTEX_INITIALIZER;

static uint64_t host_callback_entry(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4,
	uint64_t a5, uint64_t slot)
{
	uint64_t args[6] = {a0, a1, a2, a3, a4, a5};
	uintptr_t guest_fn = __atomic_load_n(&g_callback_guest_fn[slot], __ATOMIC_ACQUIRE);
	const struct fex_thunk_rules* rules = __atomic_load_n(&g_callback_thunk_rules[slot], __ATOMIC_ACQUIRE);
	if (rules)
		translate_thunk_args(rules, args);
	return fex_invoke_guest_callback_args(guest_fn, args);
}

static void write_aarch64_callback_thunk(uint8_t* dst, uint32_t slot)
{
	const uint32_t code[4] = {
		0xd2800006u | (slot << 5), // movz x6, #slot
		0x58000070u,               // ldr x16, [pc + 12]
		0xd61f0200u,               // br x16
		0xd503201fu,               // nop
	};
	const uint64_t entry = (uint64_t)(uintptr_t)host_callback_entry;
	memcpy(dst, code, sizeof(code));
	memcpy(dst + sizeof(code), &entry, sizeof(entry));
	memset(dst + sizeof(code) + sizeof(entry), 0, FEX_CALLBACK_THUNK_SIZE - sizeof(code) - sizeof(entry));
}

// Makes thunk `slot` executable, writing the page that holds it. Called with g_callback_mutex held.
static void ensure_callback_thunk(uint32_t slot)
{
	size_t page_sz = (size_t)sysconf(_SC_PAGESIZE);
	if (!g_callback_area) {
		// The whole range is reserved up front so that the thunks keep fixed addresses.
		g_callback_area = mmap(NULL, FEX_CALLBACK_MAX * FEX_CALLBACK_THUNK_SIZE, PROT_NONE,
			MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
		if (g_callback_area == MAP_FAILED) {
			fprintf(stderr, "mldr/fex: failed to reserve the host callback thunks: %s\n", strerror(errno));
			abort();
		}
	}
	while (slot >= g_callback_ready) {
		uint8_t* page = g_callback_area + (size_t)g_callback_ready * FEX_CALLBACK_THUNK_SIZE;
		uint32_t per_page = (uint32_t)(page_sz / FEX_CALLBACK_THUNK_SIZE);
		if (mprotect(page, page_sz, PROT_READ | PROT_WRITE) != 0) {
			fprintf(stderr, "mldr/fex: failed to make host callback thunks writable: %s\n", strerror(errno));
			abort();
		}
		for (uint32_t i = 0; i < per_page; i++)
			write_aarch64_callback_thunk(page + i * FEX_CALLBACK_THUNK_SIZE, g_callback_ready + i);
		if (mprotect(page, page_sz, PROT_READ | PROT_EXEC) != 0) {
			fprintf(stderr, "mldr/fex: failed to make host callback thunks executable: %s\n", strerror(errno));
			abort();
		}
		__builtin___clear_cache((char*)page, (char*)page + page_sz);
		g_callback_ready += per_page;
	}
}

// The host function that stands for a guest function pointer (the same one for the same guest function).
static uint64_t host_callback_for(uint64_t guest_fn)
{
	if (guest_fn == 0)
		return 0;
	pthread_mutex_lock(&g_callback_mutex);
	uint32_t h = (uint32_t)((guest_fn * 0x9e3779b97f4a7c15ull) >> 32) % FEX_CALLBACK_INDEX_SIZE;
	while (g_callback_index[h] != 0 && g_callback_guest_fn[g_callback_index[h] - 1] != guest_fn)
		h = (h + 1) % FEX_CALLBACK_INDEX_SIZE;
	uint32_t slot = g_callback_index[h] ? g_callback_index[h] - 1u : g_callback_count;
	if (slot == g_callback_count) {
		if (g_callback_count == FEX_CALLBACK_MAX) {
			pthread_mutex_unlock(&g_callback_mutex);
			fprintf(stderr, "mldr/fex: more than %u distinct guest functions were handed to host libraries\n",
				FEX_CALLBACK_MAX);
			abort();
		}
		ensure_callback_thunk(slot);
		__atomic_store_n(&g_callback_guest_fn[slot], (uintptr_t)guest_fn, __ATOMIC_RELEASE);
		g_callback_index[h] = (uint16_t)(slot + 1);
		g_callback_count++;
	}
	uint64_t host = (uint64_t)(uintptr_t)(g_callback_area + (size_t)slot * FEX_CALLBACK_THUNK_SIZE);
	pthread_mutex_unlock(&g_callback_mutex);
	return host;
}

// Replaces the function pointers marked in the mask by a host function that does nothing.
#define FEX_CALLBACK_IGNORE 0x80000000u

static uint64_t host_callback_ignore(void)
{
	return 0;
}

// Which arguments (bit i = i-th integer argument) of a host function are function pointers.
static const struct { const char* name; unsigned mask; } g_callback_args[] = {
	{ "png_create_read_struct", 0xc }, { "png_create_write_struct", 0xc },
	{ "png_set_error_fn", 0xc }, { "png_set_read_fn", 0x4 }, { "png_set_write_fn", 0xc },
	{ "png_set_progressive_read_fn", 0x1c },
	// TIFFClientOpen(name, mode, clientdata, read, write, seek, close, size, map, unmap)
	{ "TIFFClientOpen", 0x3f8 },
	// TIFFSetErrorHandler and friends take handlers with a va_list parameter, whose layout differs
	// between the guest and the host; they are replaced by a function that ignores the message.
	{ "TIFFSetErrorHandler", FEX_CALLBACK_IGNORE | 0x1 }, { "TIFFSetWarningHandler", FEX_CALLBACK_IGNORE | 0x1 },
	{ "TIFFSetErrorHandlerExt", FEX_CALLBACK_IGNORE | 0x1 }, { "TIFFSetWarningHandlerExt", FEX_CALLBACK_IGNORE | 0x1 },
	// wl_proxy_add_dispatcher(proxy, dispatcher, implementation, data): libwayland-client calls the dispatcher
	// as dispatcher(implementation, proxy, opcode, message, arguments) for every event of the proxy.
	{ "wl_proxy_add_dispatcher", 0x2 },
	// XIfEvent(display, event, predicate, arg) and its siblings call the predicate on each queued event.
	{ "XIfEvent", 0x4 }, { "XCheckIfEvent", 0x4 }, { "XPeekIfEvent", 0x4 },
	// DGifOpen(user_data, read_fn, error): giflib reads the file through read_fn until DGifCloseFile.
	{ "DGifOpen", 0x2 },
};

// Host functions whose callbacks a thunk cannot run: their callbacks take more than six integer
// arguments, or floating point ones. Handing them a guest callback stops the process with a message,
// because the library would otherwise call it with arguments the guest never sees.
static const struct fex_unsupported_callback g_callback_unsupported[] = {
	// GLDEBUGPROC takes seven arguments (source, type, id, severity, length, message, user_param).
	{ "glDebugMessageCallback", 0x1, "its callback takes seven arguments" },
	{ "glDebugMessageCallbackARB", 0x1, "its callback takes seven arguments" },
	{ "glDebugMessageCallbackKHR", 0x1, "its callback takes seven arguments" },
};

// Host functions that take a pointer to a struct holding guest function pointers (bit i of `fields` = the
// i-th 8-byte word of the struct is one). The library reads these structs only while the function
// runs, so it is handed a copy, on the bridge's stack, with the function pointers replaced by thunks;
// the guest's struct is left alone, since guest code may read it or call through it.
#define FEX_CALLBACK_STRUCT_MAX_WORDS 8
_Static_assert(FEX_CALLBACK_STRUCT_MAX_WORDS <= 32, "fields is a 32-bit mask");
static const struct fex_callback_struct g_callback_structs[] = {
	// FT_Outline_Decompose(outline, funcs, user): FT_Outline_Funcs is
	// { move_to, line_to, conic_to, cubic_to, int shift, FT_Pos delta }, six words.
	{ "FT_Outline_Decompose", 1, 6, 0xf },
};

// Xlib input method functions that take a NULL-terminated list of (name, value) pairs from argument
// `first`. The value of a callback name (XNPreeditStartCallback and the others below) is a pointer to
// an XIMCallback { client_data, callback }.
static const struct { const char* name; unsigned first; } g_xim_value_lists[] = {
	{ "XVaCreateNestedList", 1 }, { "XCreateIC", 1 }, { "XSetICValues", 1 }, { "XSetIMValues", 1 },
};
static const char* const g_xim_callback_names[] = {
	"preeditStartCallback", "preeditDoneCallback", "preeditDrawCallback", "preeditCaretCallback",
	"preeditStateNotifyCallback", "statusStartCallback", "statusDoneCallback", "statusDrawCallback",
	"geometryCallback", "destroyCallback", "stringConversionCallback", "r6PreeditCallback",
};

// Host copies of XIMCallback structs. A nested list keeps the pointer it was given until the guest
// frees the list, at a time the bridge does not see, so the copies are never freed; there is one per
// distinct (client_data, callback) pair, as many as a client that creates one input context per window
// registers callbacks. A change the guest makes to its XIMCallback after the call is not seen.
struct fex_xim_callback { uint64_t client_data; uint64_t callback; uint64_t guest_callback; struct fex_xim_callback* next; };
static struct fex_xim_callback* g_xim_callbacks = NULL;
static pthread_mutex_t g_xim_callback_mutex = PTHREAD_MUTEX_INITIALIZER;

static uint64_t host_xim_callback_for(uint64_t guest_struct)
{
	uint64_t guest[2];
	if (guest_struct == 0 || fex_copy_from_guest(guest, (const void*)(uintptr_t)guest_struct, sizeof(guest)) != 0)
		return guest_struct;
	uint64_t thunk = host_callback_for(guest[1]);
	pthread_mutex_lock(&g_xim_callback_mutex);
	struct fex_xim_callback* c = g_xim_callbacks;
	while (c && (c->client_data != guest[0] || c->guest_callback != guest[1]))
		c = c->next;
	if (!c) {
		c = malloc(sizeof(*c));
		if (!c) {
			pthread_mutex_unlock(&g_xim_callback_mutex);
			fprintf(stderr, "mldr/fex: out of memory copying an XIMCallback\n");
			abort();
		}
		*c = (struct fex_xim_callback){ guest[0], thunk, guest[1], g_xim_callbacks };
		g_xim_callbacks = c;
	}
	pthread_mutex_unlock(&g_xim_callback_mutex);
	// XIMCallback is the first two words of the copy.
	return (uint64_t)(uintptr_t)c;
}

// Host functions that return a host function named by their first argument. The guest calls what they
// return, so it gets the bridge's stub for that function, with the callback rules of its name.
static const char* const g_proc_address_lookups[] = {
	// Callers: OpenGL.framework (OpenGL.c), IOSurface (gpubuffer.c) and OpenGLES (EAGLContext.m).
	"eglGetProcAddress",
};

// libjpeg error managers. jpeg_std_error fills the guest's struct jpeg_error_mgr with five host methods,
// error_exit(cinfo), emit_message(cinfo, level), output_message(cinfo), format_message(cinfo, buffer) and
// reset_error_mgr(cinfo), and a guest usually replaces error_exit with its own function, which longjmps
// out instead of returning. Guest code calls these methods through cinfo->err, and so does libjpeg, from
// any function that takes a libjpeg object (whose first field is its err pointer). Both sides read the
// same five words of guest memory, so they hold guest-callable functions (stubs for host methods) while
// guest code runs, and host-callable ones while a libjpeg function runs: a call whose first argument is
// a libjpeg object switches its error manager to the host view, and back when it returns, and a guest
// method runs with the guest view in place.
#define FEX_JPEG_METHODS 5
static const char* const g_jpeg_method_names[FEX_JPEG_METHODS] = {
	"error_exit", "emit_message", "output_message", "format_message", "reset_error_mgr",
};
static const struct fex_callback_rules g_jpeg_method_rules = { .jpeg = FEX_JPEG_OBJECT, .jpeg_method = true };

// An error manager in the host view: the guest's five words, the host words written in their place, and
// the thread whose libjpeg calls it is for. Nested libjpeg calls on one manager share an entry; there is
// one per manager a thread is decoding with, so a few entries suffice. One manager used by two threads at
// once is not supported (the words can hold only one view): the process stops when it is seen.
struct fex_jpeg_host_view {
	uint64_t err;
	uint64_t guest[FEX_JPEG_METHODS];
	uint64_t host[FEX_JPEG_METHODS];
	unsigned depth;
	pid_t owner;
};
#define FEX_JPEG_MAX_HOST_VIEWS 64
static struct fex_jpeg_host_view g_jpeg_host_views[FEX_JPEG_MAX_HOST_VIEWS];
static pthread_mutex_t g_jpeg_mutex = PTHREAD_MUTEX_INITIALIZER;

// The host function behind one of the bridge's guest stubs, or 0 if `fn` is not a stub.
static uint64_t host_function_of_stub(uint64_t fn, struct fex_callback_rules* rules)
{
	uint64_t host = 0;
	pthread_mutex_lock(&g_dlsym_mutex);
	uintptr_t base = (uintptr_t)g_dlsym_stub_area;
	if (base && fn >= base && (fn - base) % FEX_STUB_SIZE == 0 && (fn - base) / FEX_STUB_SIZE < g_dlsym_count) {
		uint32_t slot = (uint32_t)((fn - base) / FEX_STUB_SIZE);
		host = (uint64_t)(uintptr_t)g_dlsym_targets[slot];
		if (rules)
			*rules = g_dlsym_rules[slot];
	}
	pthread_mutex_unlock(&g_dlsym_mutex);
	return host;
}

static uint64_t jpeg_error_mgr_of(uint64_t object)
{
	uint64_t err = 0;
	if (object == 0 || fex_copy_from_guest(&err, (const void*)(uintptr_t)object, sizeof(err)) != 0)
		return 0;
	return err;
}

// Called with g_jpeg_mutex held.
static struct fex_jpeg_host_view* jpeg_host_view_of(uint64_t err)
{
	for (unsigned i = 0; i < FEX_JPEG_MAX_HOST_VIEWS; i++)
		if (g_jpeg_host_views[i].err == err)
			return &g_jpeg_host_views[i];
	return NULL;
}

static void jpeg_host_method_0(uint64_t object);
static void jpeg_host_method_1(uint64_t object, uint64_t arg);
static void jpeg_host_method_2(uint64_t object);
static void jpeg_host_method_3(uint64_t object, uint64_t arg);
static void jpeg_host_method_4(uint64_t object);
static void* const g_jpeg_host_methods[FEX_JPEG_METHODS] = {
	(void*)jpeg_host_method_0, (void*)jpeg_host_method_1, (void*)jpeg_host_method_2,
	(void*)jpeg_host_method_3, (void*)jpeg_host_method_4,
};

// Whether a view is still in place: the manager's words are the host words it wrote. A libjpeg call left
// by another way than a guest method (a signal handler's siglongjmp) leaves a view that is not.
static bool jpeg_host_view_live(const struct fex_jpeg_host_view* v)
{
	uint64_t words[FEX_JPEG_METHODS];
	return fex_copy_from_guest(words, (const void*)(uintptr_t)v->err, sizeof(words)) == 0 &&
		memcmp(words, v->host, sizeof(words)) == 0;
}

// Puts `err` in the host view: a stub becomes its host function, a guest function the host method that
// runs it. `v` is the entry to use, or NULL for a free one. Called with g_jpeg_mutex held.
static void jpeg_set_host_view(struct fex_jpeg_host_view* v, uint64_t err, const uint64_t guest[FEX_JPEG_METHODS],
	unsigned depth)
{
	for (unsigned i = 0; !v && i < FEX_JPEG_MAX_HOST_VIEWS; i++)
		if (g_jpeg_host_views[i].err == 0 || !jpeg_host_view_live(&g_jpeg_host_views[i]))
			v = &g_jpeg_host_views[i];
	if (!v) {
		fprintf(stderr, "mldr/fex: more than %u libjpeg calls at once\n", FEX_JPEG_MAX_HOST_VIEWS);
		abort();
	}
	*v = (struct fex_jpeg_host_view){ .err = err, .depth = depth, .owner = gettid() };
	memcpy(v->guest, guest, sizeof(v->guest));
	for (unsigned m = 0; m < FEX_JPEG_METHODS; m++) {
		uint64_t stub_target = guest[m] ? host_function_of_stub(guest[m], NULL) : 0;
		v->host[m] = !guest[m] ? 0 : stub_target ? stub_target : (uint64_t)(uintptr_t)g_jpeg_host_methods[m];
	}
	fex_copy_to_guest((void*)(uintptr_t)err, v->host, sizeof(v->host));
}

// Called with g_jpeg_mutex held, for a view the calling thread is about to use.
static void jpeg_check_owner(const struct fex_jpeg_host_view* v)
{
	if (v->owner != gettid()) {
		pthread_mutex_unlock(&g_jpeg_mutex);
		fprintf(stderr, "mldr/fex: a libjpeg error manager is used by two threads at once, which the FEX bridge "
			"does not support\n");
		abort();
	}
}

// Switches the error manager of the libjpeg object `object` to the host view; returns the manager to
// switch back, or 0. Memory whose words include no libjpeg method stub is not an error manager (a
// function named jpeg_* whose first argument is something else, an err pointer not set yet) and is left alone.
static uint64_t jpeg_enter_host_view(uint64_t object)
{
	uint64_t err = jpeg_error_mgr_of(object);
	uint64_t guest[FEX_JPEG_METHODS];
	if (err == 0 || fex_copy_from_guest(guest, (const void*)(uintptr_t)err, sizeof(guest)) != 0)
		return 0;
	pthread_mutex_lock(&g_jpeg_mutex);
	struct fex_jpeg_host_view* v = jpeg_host_view_of(err);
	if (v && memcmp(guest, v->host, sizeof(guest)) == 0) {
		jpeg_check_owner(v);
		v->depth++;
	} else {
		bool is_error_mgr = false;
		for (unsigned m = 0; m < FEX_JPEG_METHODS && !is_error_mgr; m++) {
			struct fex_callback_rules rules;
			is_error_mgr = guest[m] && host_function_of_stub(guest[m], &rules) && rules.jpeg_method;
		}
		// A view whose words are gone is stale (see jpeg_host_view_live): this call starts afresh.
		if (!is_error_mgr) {
			if (v)
				v->err = 0;
			err = 0;
		} else {
			jpeg_set_host_view(v, err, guest, 1);
		}
	}
	pthread_mutex_unlock(&g_jpeg_mutex);
	return err;
}

static void jpeg_leave_host_view(uint64_t err)
{
	pthread_mutex_lock(&g_jpeg_mutex);
	struct fex_jpeg_host_view* v = jpeg_host_view_of(err);
	if (v && v->owner == gettid() && --v->depth == 0) {
		fex_copy_to_guest((void*)(uintptr_t)err, v->guest, sizeof(v->guest));
		v->err = 0;
	}
	pthread_mutex_unlock(&g_jpeg_mutex);
}

// libjpeg calls a guest method: run it with the guest view in place, as guest code expects to find it.
static void jpeg_run_guest_method(unsigned method, uint64_t object, uint64_t arg)
{
	uint64_t err = jpeg_error_mgr_of(object);
	pthread_mutex_lock(&g_jpeg_mutex);
	struct fex_jpeg_host_view* v = err ? jpeg_host_view_of(err) : NULL;
	if (!v) {
		pthread_mutex_unlock(&g_jpeg_mutex);
		fprintf(stderr, "mldr/fex: libjpeg called the %s method of an error manager it was not given\n",
			g_jpeg_method_names[method]);
		abort();
	}
	jpeg_check_owner(v);
	const struct fex_jpeg_host_view saved = *v;
	v->err = 0;
	fex_copy_to_guest((void*)(uintptr_t)err, saved.guest, sizeof(saved.guest));
	pthread_mutex_unlock(&g_jpeg_mutex);

	fex_invoke_guest_callback((uintptr_t)saved.guest[method], object, arg);

	// It returned (error_exit longjmps past libjpeg instead, leaving the guest view in place). The guest
	// may have changed its methods meanwhile.
	uint64_t guest[FEX_JPEG_METHODS];
	if (fex_copy_from_guest(guest, (const void*)(uintptr_t)err, sizeof(guest)) != 0)
		memcpy(guest, saved.guest, sizeof(guest));
	pthread_mutex_lock(&g_jpeg_mutex);
	jpeg_set_host_view(NULL, err, guest, saved.depth);
	pthread_mutex_unlock(&g_jpeg_mutex);
}

static void jpeg_host_method_0(uint64_t object) { jpeg_run_guest_method(0, object, 0); }
static void jpeg_host_method_1(uint64_t object, uint64_t arg) { jpeg_run_guest_method(1, object, arg); }
static void jpeg_host_method_2(uint64_t object) { jpeg_run_guest_method(2, object, 0); }
static void jpeg_host_method_3(uint64_t object, uint64_t arg) { jpeg_run_guest_method(3, object, arg); }
static void jpeg_host_method_4(uint64_t object) { jpeg_run_guest_method(4, object, 0); }

// After jpeg_std_error(err): the host methods it stored become guest stubs.
static void jpeg_wrap_std_error(uint64_t err)
{
	uint64_t words[FEX_JPEG_METHODS];
	if (err == 0 || fex_copy_from_guest(words, (const void*)(uintptr_t)err, sizeof(words)) != 0)
		return;
	for (unsigned m = 0; m < FEX_JPEG_METHODS; m++)
		if (words[m] && !host_function_of_stub(words[m], NULL))
			words[m] = (uint64_t)(uintptr_t)wrap_host_function((void*)(uintptr_t)words[m], g_jpeg_method_names[m],
				&g_jpeg_method_rules);
	fex_copy_to_guest((void*)(uintptr_t)err, words, sizeof(words));
}

// libpulse (pulse/*.h): which arguments of each function taking a callback are function pointers. libpulse
// keeps state and notify callbacks until they are replaced and calls operation callbacks when the server
// answers, from the context's main loop. pa_free_cb_t is the free_cb of pa_stream_write.
static const struct { const char* name; unsigned mask; } g_pa_callback_args[] = {
	// pulse/context.h
	{ "pa_context_drain", 0x2 }, { "pa_context_exit_daemon", 0x2 }, { "pa_context_proplist_remove", 0x4 },
	{ "pa_context_proplist_update", 0x8 }, { "pa_context_rttime_new", 0x4 }, { "pa_context_set_default_sink", 0x4 },
	{ "pa_context_set_default_source", 0x4 }, { "pa_context_set_event_callback", 0x2 },
	{ "pa_context_set_name", 0x4 }, { "pa_context_set_state_callback", 0x2 },
	// pulse/subscribe.h
	{ "pa_context_set_subscribe_callback", 0x2 }, { "pa_context_subscribe", 0x4 },
	// pulse/operation.h
	{ "pa_operation_set_state_callback", 0x2 },
	// pulse/stream.h
	{ "pa_stream_cork", 0x4 }, { "pa_stream_drain", 0x2 }, { "pa_stream_flush", 0x2 }, { "pa_stream_prebuf", 0x2 },
	{ "pa_stream_proplist_remove", 0x4 }, { "pa_stream_proplist_update", 0x8 }, { "pa_stream_set_buffer_attr", 0x4 },
	{ "pa_stream_set_buffer_attr_callback", 0x2 }, { "pa_stream_set_event_callback", 0x2 },
	{ "pa_stream_set_latency_update_callback", 0x2 }, { "pa_stream_set_moved_callback", 0x2 },
	{ "pa_stream_set_name", 0x4 }, { "pa_stream_set_overflow_callback", 0x2 }, { "pa_stream_set_read_callback", 0x2 },
	{ "pa_stream_set_started_callback", 0x2 }, { "pa_stream_set_state_callback", 0x2 },
	{ "pa_stream_set_suspended_callback", 0x2 }, { "pa_stream_set_underflow_callback", 0x2 },
	{ "pa_stream_set_write_callback", 0x2 }, { "pa_stream_trigger", 0x2 }, { "pa_stream_update_sample_rate", 0x4 },
	{ "pa_stream_update_timing_info", 0x2 }, { "pa_stream_write", 0x8 }, { "pa_stream_write_ext_free", 0x8 },
	// pulse/introspect.h
	{ "pa_context_add_autoload", 0x20 }, { "pa_context_get_autoload_info_by_index", 0x4 },
	{ "pa_context_get_autoload_info_by_name", 0x8 }, { "pa_context_get_autoload_info_list", 0x2 },
	{ "pa_context_get_card_info_by_index", 0x4 }, { "pa_context_get_card_info_by_name", 0x4 },
	{ "pa_context_get_card_info_list", 0x2 }, { "pa_context_get_client_info", 0x4 },
	{ "pa_context_get_client_info_list", 0x2 }, { "pa_context_get_module_info", 0x4 },
	{ "pa_context_get_module_info_list", 0x2 }, { "pa_context_get_sample_info_by_index", 0x4 },
	{ "pa_context_get_sample_info_by_name", 0x4 }, { "pa_context_get_sample_info_list", 0x2 },
	{ "pa_context_get_server_info", 0x2 }, { "pa_context_get_sink_info_by_index", 0x4 },
	{ "pa_context_get_sink_info_by_name", 0x4 }, { "pa_context_get_sink_info_list", 0x2 },
	{ "pa_context_get_sink_input_info", 0x4 }, { "pa_context_get_sink_input_info_list", 0x2 },
	{ "pa_context_get_source_info_by_index", 0x4 }, { "pa_context_get_source_info_by_name", 0x4 },
	{ "pa_context_get_source_info_list", 0x2 }, { "pa_context_get_source_output_info", 0x4 },
	{ "pa_context_get_source_output_info_list", 0x2 }, { "pa_context_kill_client", 0x4 },
	{ "pa_context_kill_sink_input", 0x4 }, { "pa_context_kill_source_output", 0x4 },
	{ "pa_context_load_module", 0x8 }, { "pa_context_move_sink_input_by_index", 0x8 },
	{ "pa_context_move_sink_input_by_name", 0x8 }, { "pa_context_move_source_output_by_index", 0x8 },
	{ "pa_context_move_source_output_by_name", 0x8 }, { "pa_context_remove_autoload_by_index", 0x4 },
	{ "pa_context_remove_autoload_by_name", 0x8 }, { "pa_context_send_message_to_object", 0x10 },
	{ "pa_context_set_card_profile_by_index", 0x8 }, { "pa_context_set_card_profile_by_name", 0x8 },
	{ "pa_context_set_port_latency_offset", 0x10 }, { "pa_context_set_sink_input_mute", 0x8 },
	{ "pa_context_set_sink_input_volume", 0x8 }, { "pa_context_set_sink_mute_by_index", 0x8 },
	{ "pa_context_set_sink_mute_by_name", 0x8 }, { "pa_context_set_sink_port_by_index", 0x8 },
	{ "pa_context_set_sink_port_by_name", 0x8 }, { "pa_context_set_sink_volume_by_index", 0x8 },
	{ "pa_context_set_sink_volume_by_name", 0x8 }, { "pa_context_set_source_mute_by_index", 0x8 },
	{ "pa_context_set_source_mute_by_name", 0x8 }, { "pa_context_set_source_output_mute", 0x8 },
	{ "pa_context_set_source_output_volume", 0x8 }, { "pa_context_set_source_port_by_index", 0x8 },
	{ "pa_context_set_source_port_by_name", 0x8 }, { "pa_context_set_source_volume_by_index", 0x8 },
	{ "pa_context_set_source_volume_by_name", 0x8 }, { "pa_context_stat", 0x2 },
	{ "pa_context_suspend_sink_by_index", 0x8 }, { "pa_context_suspend_sink_by_name", 0x8 },
	{ "pa_context_suspend_source_by_index", 0x8 }, { "pa_context_suspend_source_by_name", 0x8 },
	{ "pa_context_unload_module", 0x4 },
	// pulse/scache.h
	{ "pa_context_play_sample", 0x10 }, { "pa_context_play_sample_with_proplist", 0x20 },
	{ "pa_context_remove_sample", 0x4 },
};

// Structs of guest function pointers that a host library keeps and calls through after the function that
// was given them returns, as libpulse does with the pa_mainloop_api of pa_context_new for the whole life
// of the context. The library gets a host copy whose function pointers are thunks. The bridge does not see
// the library let go (a context lives as long as the last reference to it, which streams and operations
// also hold), so a copy is kept for the life of the process: one per guest struct, as many as the guest
// has main loops. Each call that hands over the struct rewrites its copy, so a guest struct freed and
// reallocated at the same address gets the new functions. A guest function in two places keeps the thunk
// rules of the first.
#define FEX_KEPT_STRUCT_MAX_WORDS 16
struct fex_kept_struct { unsigned words; unsigned fields; const struct fex_thunk_rules* field_rules; };

// The callbacks libpulse hands to the main loop functions all take the pa_mainloop_api as first argument.
#define FEX_PA_EVENT_CALLBACK "libpulse main loop event callback"
// pa_mainloop_api (pulse/mainloop-api.h) is { userdata, io_new, io_enable, io_free, io_set_destroy, time_new,
// time_restart, time_free, time_set_destroy, defer_new, defer_enable, defer_free, defer_set_destroy, quit }.
static const struct fex_thunk_rules g_pa_mainloop_api_rules[14] = {
	[1] = { 0x1, 0x8, FEX_PA_EVENT_CALLBACK }, // io_new(a, fd, events, cb, userdata)
	[4] = { 0, 0x2, FEX_PA_EVENT_CALLBACK },   // io_set_destroy(e, cb)
	[5] = { 0x1, 0x4, FEX_PA_EVENT_CALLBACK }, // time_new(a, tv, cb, userdata)
	[8] = { 0, 0x2, FEX_PA_EVENT_CALLBACK },   // time_set_destroy(e, cb)
	[9] = { 0x1, 0x2, FEX_PA_EVENT_CALLBACK }, // defer_new(a, cb, userdata)
	[12] = { 0, 0x2, FEX_PA_EVENT_CALLBACK },  // defer_set_destroy(e, cb)
	[13] = { 0x1, 0, NULL },                    // quit(a, retval)
};
static const struct fex_kept_struct g_pa_mainloop_api = { 14, 0x3ffe, g_pa_mainloop_api_rules };
// Host functions that keep the struct their argument `arg` points to.
static const struct { const char* name; unsigned arg; const struct fex_kept_struct* layout; } g_kept_struct_args[] = {
	{ "pa_context_new", 0, &g_pa_mainloop_api }, { "pa_context_new_with_proplist", 0, &g_pa_mainloop_api },
};
// Host functions the guest calls with a kept struct in the arguments of the mask: they get its host copy.
static const struct { const char* name; unsigned mask; } g_kept_struct_back_args[] = {
	{ FEX_PA_EVENT_CALLBACK, 0x1 },
};

struct fex_kept_copy {
	uint64_t guest;
	const struct fex_kept_struct* layout;
	struct fex_kept_copy* next;
	uint64_t words[FEX_KEPT_STRUCT_MAX_WORDS];
};
static struct fex_kept_copy* g_kept_copies = NULL; // pushed under the mutex, read without it, never freed
static pthread_mutex_t g_kept_copy_mutex = PTHREAD_MUTEX_INITIALIZER;

// A thunk for guest_fn that applies `rules` to its arguments.
static uint64_t host_callback_with_rules(uint64_t guest_fn, const struct fex_thunk_rules* rules)
{
	uint64_t host = host_callback_for(guest_fn);
	if (host && (rules->struct_args || rules->host_fn_args)) {
		size_t slot = (size_t)(host - (uint64_t)(uintptr_t)g_callback_area) / FEX_CALLBACK_THUNK_SIZE;
		const struct fex_thunk_rules* none = NULL;
		__atomic_compare_exchange_n(&g_callback_thunk_rules[slot], &none, rules, false, __ATOMIC_RELEASE,
			__ATOMIC_RELAXED);
	}
	return host;
}

static uint64_t kept_copy_for(uint64_t guest, const struct fex_kept_struct* layout)
{
	uint64_t words[FEX_KEPT_STRUCT_MAX_WORDS];
	// A pointer the bridge cannot read is passed on as is: the library then fails on it as it would natively.
	if (guest == 0 || fex_copy_from_guest(words, (const void*)(uintptr_t)guest, layout->words * sizeof(uint64_t)) != 0)
		return guest;
	for (unsigned w = 0; w < layout->words; w++)
		if (layout->fields & (1u << w))
			words[w] = host_callback_with_rules(words[w], &layout->field_rules[w]);
	pthread_mutex_lock(&g_kept_copy_mutex);
	struct fex_kept_copy* c = g_kept_copies;
	while (c && (c->guest != guest || c->layout != layout))
		c = c->next;
	if (!c) {
		c = calloc(1, sizeof(*c));
		if (!c) {
			pthread_mutex_unlock(&g_kept_copy_mutex);
			fprintf(stderr, "mldr/fex: out of memory copying a struct of callbacks\n");
			abort();
		}
		c->guest = guest;
		c->layout = layout;
		c->next = g_kept_copies;
		for (unsigned w = 0; w < layout->words; w++)
			c->words[w] = words[w];
		__atomic_store_n(&g_kept_copies, c, __ATOMIC_RELEASE);
	} else {
		for (unsigned w = 0; w < layout->words; w++)
			__atomic_store_n(&c->words[w], words[w], __ATOMIC_RELAXED);
	}
	pthread_mutex_unlock(&g_kept_copy_mutex);
	return (uint64_t)(uintptr_t)c->words;
}

// The host copy of a guest struct (`value` itself if it has none), and the other way around.
static uint64_t kept_copy_of(uint64_t value)
{
	for (struct fex_kept_copy* c = __atomic_load_n(&g_kept_copies, __ATOMIC_ACQUIRE); c; c = c->next)
		if (c->guest == value)
			return (uint64_t)(uintptr_t)c->words;
	return value;
}

static uint64_t guest_struct_of(uint64_t value)
{
	for (struct fex_kept_copy* c = __atomic_load_n(&g_kept_copies, __ATOMIC_ACQUIRE); c; c = c->next)
		if ((uint64_t)(uintptr_t)c->words == value)
			return c->guest;
	return value;
}

// The guest function behind a host thunk, 0 if `host` is not one.
static uint64_t guest_fn_of_thunk(uint64_t host)
{
	uint64_t guest = 0;
	pthread_mutex_lock(&g_callback_mutex);
	uint64_t base = (uint64_t)(uintptr_t)g_callback_area;
	if (base && host >= base && host < base + (uint64_t)g_callback_count * FEX_CALLBACK_THUNK_SIZE &&
	    (host - base) % FEX_CALLBACK_THUNK_SIZE == 0)
		guest = g_callback_guest_fn[(host - base) / FEX_CALLBACK_THUNK_SIZE];
	pthread_mutex_unlock(&g_callback_mutex);
	return guest;
}

static void translate_thunk_args(const struct fex_thunk_rules* rules, uint64_t args[6])
{
	for (int a = 0; a < 6; a++) {
		if (rules->struct_args & (1u << a))
			args[a] = guest_struct_of(args[a]);
		if ((rules->host_fn_args & (1u << a)) && args[a] != 0) {
			// A thunk the library got from the guest earlier stands for a guest function: the guest calls it directly.
			uint64_t guest = guest_fn_of_thunk(args[a]);
			args[a] = guest ? guest : (uint64_t)(uintptr_t)fex_dlsym_wrap_if_func((void*)(uintptr_t)args[a],
				rules->host_fn_rules);
		}
	}
}

static void kept_struct_rules_for(const char* name, struct fex_callback_rules* r)
{
	for (size_t i = 0; i < sizeof(g_kept_struct_args) / sizeof(g_kept_struct_args[0]); i++) {
		if (strcmp(name, g_kept_struct_args[i].name) != 0)
			continue;
		if (g_kept_struct_args[i].layout->words > FEX_KEPT_STRUCT_MAX_WORDS) {
			fprintf(stderr, "mldr/fex: the kept callback struct of %s is larger than the bridge's copy\n", name);
			abort();
		}
		r->kept_struct = g_kept_struct_args[i].layout;
		r->kept_struct_arg = g_kept_struct_args[i].arg;
	}
	for (size_t i = 0; i < sizeof(g_kept_struct_back_args) / sizeof(g_kept_struct_back_args[0]); i++)
		if (strcmp(name, g_kept_struct_back_args[i].name) == 0)
			r->kept_struct_back = g_kept_struct_back_args[i].mask;
}

static struct fex_callback_rules callback_rules_for(const char* name)
{
	struct fex_callback_rules r = { FEX_X_HANDLER_NONE, 0, NULL, NULL, 0 };
	if (!name)
		return r;
	if (strcmp(name, "XSetErrorHandler") == 0)
		r.x_handler = FEX_X_ERROR_HANDLER;
	else if (strcmp(name, "XSetIOErrorHandler") == 0)
		r.x_handler = FEX_X_IO_ERROR_HANDLER;
	for (size_t i = 0; i < sizeof(g_callback_args) / sizeof(g_callback_args[0]); i++)
		if (strcmp(name, g_callback_args[i].name) == 0)
			r.args = g_callback_args[i].mask;
	for (size_t i = 0; i < sizeof(g_callback_unsupported) / sizeof(g_callback_unsupported[0]); i++)
		if (strcmp(name, g_callback_unsupported[i].name) == 0)
			r.unsupported = &g_callback_unsupported[i];
	for (size_t i = 0; i < sizeof(g_callback_structs) / sizeof(g_callback_structs[0]); i++) {
		if (strcmp(name, g_callback_structs[i].name) != 0)
			continue;
		if (g_callback_structs[i].words > FEX_CALLBACK_STRUCT_MAX_WORDS) {
			fprintf(stderr, "mldr/fex: the callback struct of %s is larger than the bridge's copy\n", name);
			abort();
		}
		r.callback_struct = &g_callback_structs[i];
	}
	for (size_t i = 0; i < sizeof(g_xim_value_lists) / sizeof(g_xim_value_lists[0]); i++)
		if (strcmp(name, g_xim_value_lists[i].name) == 0)
			r.xim_list = g_xim_value_lists[i].first;
	for (size_t i = 0; i < sizeof(g_proc_address_lookups) / sizeof(g_proc_address_lookups[0]); i++)
		if (strcmp(name, g_proc_address_lookups[i]) == 0)
			r.returns_function = true;
	if (strcmp(name, "jpeg_std_error") == 0)
		r.jpeg = FEX_JPEG_STD_ERROR;
	else if (strncmp(name, "jpeg_", 5) == 0)
		r.jpeg = FEX_JPEG_OBJECT;
	for (size_t i = 0; i < sizeof(g_pa_callback_args) / sizeof(g_pa_callback_args[0]); i++)
		if (strcmp(name, g_pa_callback_args[i].name) == 0)
			r.args |= g_pa_callback_args[i].mask;
	kept_struct_rules_for(name, &r);
	return r;
}

// Replaces the guest function pointers among the arguments of a host function (args[i] is its i-th
// integer argument) by host thunks, as its rules say. `struct_copy` must stay valid until the host
// function returns.
static void translate_guest_callbacks(const struct fex_callback_rules* rules, uint64_t* args[], int nargs,
	uint64_t* struct_copy)
{
	bool ignore = (rules->args & FEX_CALLBACK_IGNORE) != 0;
	for (int a = 0; a < nargs && a < 31; a++)
		if (rules->args & (1u << a))
			*args[a] = (*args[a] && ignore) ? (uint64_t)(uintptr_t)host_callback_ignore : host_callback_for(*args[a]);
	const struct fex_unsupported_callback* u = rules->unsupported;
	for (int a = 0; u && a < nargs && a < 31; a++) {
		if ((u->mask & (1u << a)) && *args[a] != 0) {
			fprintf(stderr, "mldr/fex: %s was given a guest callback, which the FEX bridge cannot run: %s\n",
				u->name, u->reason);
			abort();
		}
	}
	const struct fex_callback_struct* cs = rules->callback_struct;
	if (cs) {
		uint64_t* arg = args[cs->arg];
		// A pointer the bridge cannot read is passed on as is: the library then fails on it as it would natively.
		if (*arg != 0 && fex_copy_from_guest(struct_copy, (const void*)(uintptr_t)*arg, cs->words * sizeof(uint64_t)) == 0) {
			for (unsigned w = 0; w < cs->words; w++)
				if (cs->fields & (1u << w))
					struct_copy[w] = host_callback_for(struct_copy[w]);
			*arg = (uint64_t)(uintptr_t)struct_copy;
		}
	}
	if (rules->xim_list) {
		// The host function reads the list with va_arg, past the words the bridge copies if it is longer,
		// so a list whose terminator is not among them is not supported.
		int a = (int)rules->xim_list;
		for (; a < nargs && *args[a] != 0; a += 2) {
			if (a + 1 >= nargs)
				break;
			const char* key = (const char*)(uintptr_t)*args[a];
			for (size_t k = 0; k < sizeof(g_xim_callback_names) / sizeof(g_xim_callback_names[0]); k++) {
				if (strcmp(key, g_xim_callback_names[k]) == 0) {
					*args[a + 1] = host_xim_callback_for(*args[a + 1]);
					break;
				}
			}
		}
		if (a >= nargs || *args[a] != 0) {
			fprintf(stderr, "mldr/fex: an Xlib input method value list longer than %d arguments is not supported\n",
				nargs);
			abort();
		}
	}
	for (int a = 0; rules->kept_struct_back && a < nargs && a < 31; a++)
		if (rules->kept_struct_back & (1u << a))
			*args[a] = kept_copy_of(*args[a]);
	if (rules->kept_struct)
		*args[rules->kept_struct_arg] = kept_copy_for(*args[rules->kept_struct_arg], rules->kept_struct);
}

long fex_dlsym_dispatch(uint32_t slot, struct fex_x86_64_state* state)
{
	if (!state)
		return -EINVAL;

	pthread_mutex_lock(&g_dlsym_mutex);
	void* target = (slot < g_dlsym_count) ? g_dlsym_targets[slot] : NULL;
	const struct fex_callback_rules rules = target ? g_dlsym_rules[slot] : (struct fex_callback_rules){ FEX_X_HANDLER_NONE, 0, NULL, NULL, 0 };
	pthread_mutex_unlock(&g_dlsym_mutex);

	if (!target)
		return -ENOSYS;

	uintptr_t* guest_handler = NULL;
	uint64_t* host_native = NULL;
	uint64_t host_handler = 0;
	if (rules.x_handler == FEX_X_ERROR_HANDLER) {
		guest_handler = &g_guest_x_error_handler;
		host_native = &g_host_x_error_native;
		host_handler = (uint64_t)(uintptr_t)host_x_error_handler;
	} else if (rules.x_handler == FEX_X_IO_ERROR_HANDLER) {
		guest_handler = &g_guest_x_io_error_handler;
		host_native = &g_host_x_io_error_native;
		host_handler = (uint64_t)(uintptr_t)host_x_io_error_handler;
	}
	uint64_t previous_guest_handler = 0;
	if (guest_handler) {
		previous_guest_handler = *guest_handler;
		if (state->rdi && state->rdi == *host_native) {
			// The guest is restoring the handler Xlib had before ours: it is host code, so pass it back as is.
			*guest_handler = 0;
		} else {
			*guest_handler = (uintptr_t)state->rdi;
			state->rdi = state->rdi ? host_handler : 0;
		}
	}

	// The arguments after the sixth are on the guest stack, above the return address. X11 and Wayland
	// functions take up to a dozen (XRRGetOutputProperty has 13), so forward a generous number of
	// words; the host function only reads the ones it has.
	enum { FEX_STACK_ARG_WORDS = 16 };
	uint64_t st[FEX_STACK_ARG_WORDS] = {0};
	if (state->rsp != 0) {
		const uint64_t* sp = (const uint64_t*)(uintptr_t)state->rsp;
		for (int i = 0; i < FEX_STACK_ARG_WORDS; i++)
			st[i] = sp[1 + i];
	}
	// Holds the host copy of a struct of callbacks until the host function returns.
	uint64_t struct_copy[FEX_CALLBACK_STRUCT_MAX_WORDS];
	if (!guest_handler) {
		uint64_t* args[6 + FEX_STACK_ARG_WORDS] = {&state->rdi, &state->rsi, &state->rdx, &state->r10, &state->r8, &state->r9};
		for (int w = 0; w < FEX_STACK_ARG_WORDS; w++)
			args[6 + w] = &st[w];
		translate_guest_callbacks(&rules, args, 6 + FEX_STACK_ARG_WORDS, struct_copy);
	}
	uint64_t s0 = st[0], s1 = st[1];
	const uint64_t jpeg_err = rules.jpeg == FEX_JPEG_OBJECT ? jpeg_enter_host_view(state->rdi) : 0;

#if defined(__aarch64__)
	double v0, v1, v2, v3, v4, v5, v6, v7;
	memcpy(&v0, &state->xmm[0][0], sizeof(double));
	memcpy(&v1, &state->xmm[1][0], sizeof(double));
	memcpy(&v2, &state->xmm[2][0], sizeof(double));
	memcpy(&v3, &state->xmm[3][0], sizeof(double));
	memcpy(&v4, &state->xmm[4][0], sizeof(double));
	memcpy(&v5, &state->xmm[5][0], sizeof(double));
	memcpy(&v6, &state->xmm[6][0], sizeof(double));
	memcpy(&v7, &state->xmm[7][0], sizeof(double));

	register uint64_t x0 asm("x0") = state->rdi;
	register uint64_t x1 asm("x1") = state->rsi;
	register uint64_t x2 asm("x2") = state->rdx;
	register uint64_t x3 asm("x3") = state->r10;
	register uint64_t x4 asm("x4") = state->r8;
	register uint64_t x5 asm("x5") = state->r9;
	register uint64_t x6 asm("x6") = s0;
	register uint64_t x7 asm("x7") = s1;
	register double d0 asm("d0") = v0;
	register double d1 asm("d1") = v1;
	register double d2 asm("d2") = v2;
	register double d3 asm("d3") = v3;
	register double d4 asm("d4") = v4;
	register double d5 asm("d5") = v5;
	register double d6 asm("d6") = v6;
	register double d7 asm("d7") = v7;

	// x6 and x7 carry the first two stack words; the others go on the host stack (14 words, 16-aligned).
	__asm__ volatile(
		"sub sp, sp, #112\n\t"
		"ldp x9, x10, [%[stk], #0]\n\t"
		"stp x9, x10, [sp, #0]\n\t"
		"ldp x9, x10, [%[stk], #16]\n\t"
		"stp x9, x10, [sp, #16]\n\t"
		"ldp x9, x10, [%[stk], #32]\n\t"
		"stp x9, x10, [sp, #32]\n\t"
		"ldp x9, x10, [%[stk], #48]\n\t"
		"stp x9, x10, [sp, #48]\n\t"
		"ldp x9, x10, [%[stk], #64]\n\t"
		"stp x9, x10, [sp, #64]\n\t"
		"ldp x9, x10, [%[stk], #80]\n\t"
		"stp x9, x10, [sp, #80]\n\t"
		"ldp x9, x10, [%[stk], #96]\n\t"
		"stp x9, x10, [sp, #96]\n\t"
		"blr %[fn]\n\t"
		"add sp, sp, #112\n\t"
		: "+r"(x0), "+r"(x1), "+r"(x2), "+r"(x3), "+r"(x4), "+r"(x5), "+r"(x6), "+r"(x7),
		  "+w"(d0), "+w"(d1), "+w"(d2), "+w"(d3), "+w"(d4), "+w"(d5), "+w"(d6), "+w"(d7)
		: [stk] "r"(&st[2]), [fn] "r"(target)
		: "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
		  "x16", "x17", "x18", "x30", "cc", "memory",
		  "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23",
		  "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31"
	);

	if (guest_handler) {
		if (x0 == host_handler)
			x0 = previous_guest_handler;
		else
			*host_native = x0;
	}
	if (jpeg_err)
		jpeg_leave_host_view(jpeg_err);
	if (rules.jpeg == FEX_JPEG_STD_ERROR)
		jpeg_wrap_std_error(state->rdi);
	if (rules.returns_function && x0 != 0)
		x0 = (uint64_t)(uintptr_t)fex_dlsym_wrap_if_func((void*)(uintptr_t)x0, (const char*)(uintptr_t)state->rdi);
	state->rax = x0;
	state->rdx = x1;
	v0 = d0;
	memcpy(&state->xmm[0][0], &v0, sizeof(double));
	long ret = (long)x0;
#else
	typedef uint64_t (*host_fn_t)(uint64_t, uint64_t, uint64_t, uint64_t,
		uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
	uint64_t ret_val = ((host_fn_t)target)(state->rdi, state->rsi, state->rdx, state->r10,
		state->r8, state->r9, s0, s1, st[2], st[3]);
	state->rax = ret_val;
	long ret = (long)ret_val;
#endif

	if (t_deferred_terminate.pending) {
		state->unwind_thread = 1;
	}
	return ret;
}

long fex_elfcalls_dispatch(uint32_t index, struct fex_x86_64_state* state)
{
	if (!state || index >= DARLING_FEX_ELFCALL_COUNT)
		return -ENOSYS;

	uint64_t ret = 0;
	switch (index) {
		case 0: // dlopen
			ret = (uintptr_t)g_host_elfcalls.dlopen((const char*)(uintptr_t)state->rdi);
			break;
		case 1: // dlclose
			ret = (uint64_t)(int64_t)g_host_elfcalls.dlclose((void*)(uintptr_t)state->rdi);
			break;
		case 2: { // dlsym
			void* sym = g_host_elfcalls.dlsym((void*)(uintptr_t)state->rdi, (const char*)(uintptr_t)state->rsi);
			ret = (uintptr_t)fex_dlsym_wrap_if_func(sym, (const char*)(uintptr_t)state->rsi);
			break;
		}
		case 3: // dlerror
			ret = (uintptr_t)g_host_elfcalls.dlerror();
			break;
		case 4: { // darling_thread_create (9 arguments: 6 in rdi..r9, 3 at rsp+8..rsp+24)
			const uint64_t* sp = (const uint64_t*)(uintptr_t)state->rsp;
			uintptr_t arg6 = sp ? (uintptr_t)sp[1] : 0;
			darling_thread_create_callbacks_t guest_cbs =
				sp ? (darling_thread_create_callbacks_t)(uintptr_t)sp[2] : NULL;
			void* dthread = sp ? (void*)(uintptr_t)sp[3] : NULL;
			darling_thread_create_callbacks_t wrapped_cbs =
				guest_cbs ? fex_wrap_guest_thread_callbacks(guest_cbs) : NULL;
			ret = (uintptr_t)g_host_elfcalls.darling_thread_create(
				(unsigned long)state->rdi,
				(unsigned long)state->rsi,
				(void*)(uintptr_t)state->rdx,
				(uintptr_t)state->r10,
				(uintptr_t)state->r8,
				(uintptr_t)state->r9,
				arg6,
				wrapped_cbs,
				dthread);
			break;
		}
		case 5: // darling_thread_terminate (deferred until FEX thread cleanup runs)
			t_deferred_terminate.pending = true;
			t_deferred_terminate.stackaddr = (void*)(uintptr_t)state->rdi;
			t_deferred_terminate.freesize = (unsigned long)state->rsi;
			t_deferred_terminate.pthobj_size = (unsigned long)state->rdx;
			state->unwind_thread = 1;
			ret = 0;
			break;
		case 6: // darling_thread_get_stack
			ret = (uintptr_t)g_host_elfcalls.darling_thread_get_stack();
			break;
		case 7: // dlopen_fatal
			ret = (uintptr_t)g_host_elfcalls.dlopen_fatal((const char*)(uintptr_t)state->rdi);
			break;
		case 8: // dlclose_fatal
			ret = (uint64_t)(int64_t)g_host_elfcalls.dlclose_fatal((void*)(uintptr_t)state->rdi);
			break;
		case 9: { // dlsym_fatal
			void* sym = g_host_elfcalls.dlsym_fatal((void*)(uintptr_t)state->rdi, (const char*)(uintptr_t)state->rsi);
			ret = (uintptr_t)fex_dlsym_wrap_if_func(sym, (const char*)(uintptr_t)state->rsi);
			break;
		}
		case 10: // get_errno
			ret = (uint64_t)(int64_t)g_host_elfcalls.get_errno();
			break;
		case 11: // sem_open
			ret = (uintptr_t)g_host_elfcalls.sem_open(
				(const char*)(uintptr_t)state->rdi,
				(int)state->rsi,
				(unsigned short)state->rdx,
				(unsigned int)state->r10);
			break;
		case 12: // sem_wait
			ret = (uint64_t)(int64_t)g_host_elfcalls.sem_wait((int*)(uintptr_t)state->rdi);
			break;
		case 13: // sem_trywait
			ret = (uint64_t)(int64_t)g_host_elfcalls.sem_trywait((int*)(uintptr_t)state->rdi);
			break;
		case 14: // sem_post
			ret = (uint64_t)(int64_t)g_host_elfcalls.sem_post((int*)(uintptr_t)state->rdi);
			break;
		case 15: // sem_close
			ret = (uint64_t)(int64_t)g_host_elfcalls.sem_close((int*)(uintptr_t)state->rdi);
			break;
		case 16: // sem_unlink
			ret = (uint64_t)(int64_t)g_host_elfcalls.sem_unlink((const char*)(uintptr_t)state->rdi);
			break;
		case 17: // shm_open
			ret = (uint64_t)(int64_t)g_host_elfcalls.shm_open(
				(const char*)(uintptr_t)state->rdi,
				(int)state->rsi,
				(unsigned short)state->rdx);
			break;
		case 18: // shm_unlink
			ret = (uint64_t)(int64_t)g_host_elfcalls.shm_unlink((const char*)(uintptr_t)state->rdi);
			break;
		case 19: // exit
			g_host_elfcalls.exit((int)state->rdi);
			break;
		case 20: // malloc
			ret = (uintptr_t)g_host_elfcalls.malloc((size_t)state->rdi);
			break;
		case 21: // free
			g_host_elfcalls.free((void*)(uintptr_t)state->rdi);
			break;
		case 22: // realloc
			ret = (uintptr_t)g_host_elfcalls.realloc((void*)(uintptr_t)state->rdi, (size_t)state->rsi);
			break;
		case 23: // sysconf
			ret = (uint64_t)g_host_elfcalls.sysconf((int)state->rdi);
			break;
		case 24: // dserver_socket_address
			ret = (uintptr_t)g_host_elfcalls.dserver_socket_address();
			break;
		case 25: // dserver_per_thread_socket
			ret = (uint64_t)(int64_t)g_host_elfcalls.dserver_per_thread_socket();
			break;
		case 26: // dserver_per_thread_socket_refresh
			g_host_elfcalls.dserver_per_thread_socket_refresh();
			break;
		case 27: // dserver_close_socket
			g_host_elfcalls.dserver_close_socket((int)state->rdi);
			break;
		case 28: // dserver_get_process_lifetime_pipe
			ret = (uint64_t)(int64_t)g_host_elfcalls.dserver_get_process_lifetime_pipe();
			break;
		case 29: // dserver_process_lifetime_pipe_refresh
			ret = (uint64_t)(int64_t)g_host_elfcalls.dserver_process_lifetime_pipe_refresh();
			break;
		case 30: // dserver_close_process_lifetime_pipe
			g_host_elfcalls.dserver_close_process_lifetime_pipe((int)state->rdi);
			break;
		case 31: { // native_fork
			pthread_mutex_lock(&g_dlsym_mutex);
			if (g_fork_lock_hook) {
				g_fork_lock_hook();
			}
			int fork_ret = g_host_elfcalls.native_fork ? g_host_elfcalls.native_fork() : -ENOSYS;
			bool is_child = (fork_ret == 0);
			if (is_child)
				fex_signals_after_fork_child();
			if (g_fork_unlock_hook) {
				g_fork_unlock_hook(is_child);
			}
			pthread_mutex_unlock(&g_dlsym_mutex);
			ret = (uint64_t)(int64_t)fork_ret;
			break;
		}
		case 32: // inherited_nofile_limit
			ret = (uint64_t)(int64_t)g_host_elfcalls.inherited_nofile_limit(
				(unsigned int*)(uintptr_t)state->rdi);
			break;
		default:
			return -ENOSYS;
	}

	state->rax = ret;
	if (t_deferred_terminate.pending) {
		state->unwind_thread = 1;
	}
	return (long)ret;
}
